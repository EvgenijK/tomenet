#include "endpoint-run.h"
#include "../../src/common/pack.h"
#include <SDL3/SDL.h>
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int listener;
    int contacts;
    bool first_request_exact, corrected_request_exact, closed;
    unsigned char login_start[8];
    const char *draft, *corrected, *reason;
    bool cancel_recovery, retry_disabled, disconnect_after_submit, exclusive;
} Peer;

static bool exact(int socket, void *out, size_t size)
{
    unsigned char *bytes = out;
    while (size) {
        ssize_t count = recv(socket, bytes, size, 0);
        if (count <= 0) return false;
        bytes += count;
        size -= (size_t)count;
    }
    return true;
}

static bool field(int socket, char *out, size_t capacity)
{
    size_t at = 0;
    do {
        if (at + 1 >= capacity || !exact(socket, out + at, 1)) return false;
    } while (out[at++]);
    return true;
}

static void put16(unsigned char **at, uint16_t value)
{
    *(*at)++ = (unsigned char)(value >> 8);
    *(*at)++ = (unsigned char)value;
}

static void put32(unsigned char **at, uint32_t value)
{
    *(*at)++ = (unsigned char)(value >> 24);
    *(*at)++ = (unsigned char)(value >> 16);
    *(*at)++ = (unsigned char)(value >> 8);
    *(*at)++ = (unsigned char)value;
}

static void push_key(SDL_Keycode key, SDL_Keymod mod)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = key;
    event.key.mod = mod;
    if (key == SDLK_N) event.key.scancode = SDL_SCANCODE_N;
    else if (key == SDLK_BACKSPACE) event.key.scancode = SDL_SCANCODE_BACKSPACE;
    else if (key == SDLK_R) event.key.scancode = SDL_SCANCODE_R;
    else if (key == SDLK_END) event.key.scancode = SDL_SCANCODE_END;
    else event.key.scancode = SDL_SCANCODE_RETURN;
    assert(SDL_PushEvent(&event));
}

static void push_text(const char *text)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = text;
    assert(SDL_PushEvent(&event));
}

static void contact_ready(Peer *peer, int socket)
{
    unsigned char fixed[40];
    char text[80];
    assert(exact(socket, fixed, 4) && field(socket, text, sizeof(text)) &&
           exact(socket, fixed + 4, 3) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && exact(socket, fixed + 7, 26));

    unsigned char contact[34], *at = contact;
    *at++ = 255;
    *at++ = 0;
    put32(&at, 0);
    put32(&at, 2);
    const uint32_t version[6] = {4, 9, 4, 0, 0, 0};
    for (size_t i = 0; i < 6; ++i) put32(&at, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == (ssize_t)sizeof(contact));

    unsigned char verify;
    assert(exact(socket, &verify, 1) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && field(socket, text, sizeof(text)));
    static const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == (ssize_t)sizeof(verified));
    unsigned char setup[13];
    at = setup;
    put32(&at, 0);
    put16(&at, 20);
    *at++ = 0;
    *at++ = 0;
    *at++ = 0;
    put32(&at, 13);
    assert(send(socket, setup, sizeof(setup), 0) == (ssize_t)sizeof(setup));

    unsigned char login_start[8];
    assert(exact(socket, login_start, sizeof(login_start)));
    assert(login_start[0] == PKT_LOGIN && login_start[1] == 0 &&
           login_start[2] == 0xf4 && login_start[3] == 0x43);
    if (peer->contacts == 1)
        memcpy(peer->login_start, login_start, sizeof(login_start));
    else
        assert(!memcmp(login_start, peer->login_start, sizeof(login_start)));
}

static void send_overview(int socket)
{
    unsigned char response[64], *at = response;
    *at++ = PKT_SERVERDETAILS;
    put32(&at, 2u | (1u << 8) | (1u << 16));
    put32(&at, 0x11223344);
    put32(&at, 0x55667788);
    put32(&at, 0xc0);
    *at++ = PKT_LOGIN;
    put16(&at, 0);
    *at++ = 0xff;
    *at++ = 'W';
    *at++ = 0;
    memcpy(at, "Hero", 5);
    at += 5;
    put16(&at, 42);
    put16(&at, 0);
    put16(&at, 0);
    memcpy(at, "at home", 8);
    at += 8;
    *at++ = PKT_LOGIN;
    put16(&at, 0);
    *at++ = 0;
    *at++ = 0;
    put16(&at, 0);
    put16(&at, 0);
    put16(&at, 0);
    *at++ = 0;
    assert(send(socket, response, (size_t)(at - response), 0) ==
           (ssize_t)(at - response));
}

static int peer_run(void *opaque)
{
    Peer *peer = opaque;
    int first = accept(peer->listener, NULL, NULL);
    assert(first >= 0);
    struct timeval timeout = {.tv_sec = 8};
    assert(!setsockopt(first, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));
    ++peer->contacts;
    contact_ready(peer, first);
    send_overview(first);
    SDL_Delay(180);
    push_key(peer->exclusive ? SDLK_E : SDLK_N, SDL_KMOD_SHIFT);
    push_text(peer->draft);
    push_key(SDLK_RETURN, 0);
    unsigned char request_type;
    char request[80];
    assert(exact(first, &request_type, 1) && field(first, request, sizeof(request)));
    peer->first_request_exact = request_type == PKT_LOGIN &&
        !strcmp(request, peer->draft);
    if (peer->disconnect_after_submit) {
        close(first);
        peer->closed = true;
        SDL_Delay(120);
        push_key(SDLK_ESCAPE, 0);
        return 0;
    }
    assert(send(first, (unsigned char[]){PKT_QUIT}, 1, 0) == 1);
    assert(send(first, peer->reason, strlen(peer->reason) + 1, 0) ==
           (ssize_t)strlen(peer->reason) + 1);
    shutdown(first, SHUT_WR);
    if (peer->retry_disabled) {
        unsigned char extra;
        peer->closed = recv(first, &extra, 1, 0) == 0;
        close(first);
        return 0;
    }
    SDL_Delay(120);
    push_key(SDLK_R, 0);
    /* These events belong to the closed generation and must not edit/submit
     * the recovered draft in the replacement generation. */
    push_text("Stale");
    push_key(SDLK_RETURN, 0);
    unsigned char extra;
    assert(recv(first, &extra, 1, 0) == 0);
    close(first);

    int second = accept(peer->listener, NULL, NULL);
    assert(second >= 0);
    assert(!setsockopt(second, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));
    ++peer->contacts;
    contact_ready(peer, second);
    send_overview(second);
    SDL_Delay(180);
    if (peer->cancel_recovery) {
        push_key(SDLK_ESCAPE, 0);
        SDL_Delay(80);
        push_key(SDLK_Q, 0);
        peer->closed = recv(second, &extra, 1, 0) == 0;
        close(second);
        return 0;
    }
    /* Recovery must reopen the editor with the rejected draft, not an empty one. */
    push_key(SDLK_END, 0);
    push_key(SDLK_BACKSPACE, 0);
    push_text("a");
    push_key(SDLK_RETURN, 0);
    assert(exact(second, &request_type, 1) && field(second, request, sizeof(request)));
    peer->corrected_request_exact = request_type == PKT_LOGIN &&
        !strcmp(request, peer->corrected);
    const unsigned char accepted = 0;
    assert(send(second, &accepted, 1, 0) == 1);
    SDL_Delay(100);
    push_key(SDLK_Q, 0);
    peer->closed = recv(second, &extra, 1, 0) == 0;
    close(second);
    return 0;
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0)};
    assert(peer.listener >= 0);
    if (!strcmp(argv[3], "forbidden") || !strcmp(argv[3], "exclusive-forbidden") ||
        !strcmp(argv[3], "forbidden-exit") ||
        !strcmp(argv[3], "disconnect")) {
        peer.draft = "Forbidden";
        peer.corrected = "Forbiddea";
        peer.reason = "Forbidden character name. Please choose a different name.";
        peer.retry_disabled = !strcmp(argv[3], "forbidden-exit");
        peer.disconnect_after_submit = !strcmp(argv[3], "disconnect");
        peer.exclusive = !strcmp(argv[3], "exclusive-forbidden");
    } else if (!strcmp(argv[3], "duplicate")) {
        peer.draft = "Duplicate";
        peer.corrected = "Duplicata";
        peer.reason = "Character name already logged in or currently logging in.";
        peer.cancel_recovery = true;
    } else {
        assert(!strcmp(argv[3], "unowned"));
        peer.draft = "Unowned";
        peer.corrected = "Unownea";
        peer.reason = "Name already in use by another player.";
    }
    int reuse = 1;
    assert(!setsockopt(peer.listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 2));
    socklen_t address_size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &address_size));
    SDL_Thread *worker = SDL_CreateThread(peer_run, "sv-b022-rejection-peer", &peer);
    assert(worker);

    SvEndpointOutcome outcome = {0};
    int result = sv_endpoint_run((SvEndpointOptions){
        .root = argv[1], .library = argv[2], .server = "127.0.0.1",
        .width = 1024, .height = 768, .windowed = 1,
        .port = ntohs(address.sin_port), .account = "Test", .password = "pw",
        .real_name = "PLAYER", .outcome = &outcome,
        .retry_login_disabled = peer.retry_disabled
    });
    SDL_WaitThread(worker, NULL);
    close(peer.listener);

    if (peer.disconnect_after_submit) {
        assert(result == 1 && peer.contacts == 1 && peer.first_request_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_DISCONNECTED && outcome.name_submitted);
        assert(!outcome.name_rejected && !outcome.name_recovery_started &&
               !outcome.prior_generation);
        assert(outcome.generation == outcome.name_entry_generation);
        assert(outcome.character_count == 1 && outcome.authenticated);
        assert(!strcmp(outcome.selected_character, peer.draft));
        return 0;
    }
    if (peer.retry_disabled) {
        assert(result == 1 && peer.contacts == 1 && peer.first_request_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_FAILED && outcome.name_rejected);
        assert(!outcome.name_recovery_started && !outcome.prior_generation);
        assert(outcome.generation == outcome.name_rejected_generation);
        assert(outcome.character_count == 1 && outcome.authenticated);
        assert(!strcmp(outcome.name_rejection_reason, peer.reason));
        return 0;
    }
    assert(result == 0 && peer.contacts == 2 && peer.first_request_exact && peer.closed);
    if (peer.cancel_recovery)
        assert(outcome.phase == SV_PREGAME_OVERVIEW &&
               outcome.ordinary_name_cancelled && !peer.corrected_request_exact);
    else
        assert(outcome.phase == SV_PREGAME_BIRTH && peer.corrected_request_exact &&
               !strcmp(outcome.selected_character, peer.corrected));
    assert(outcome.name_rejected);
    assert(outcome.name_recovery_started && outcome.name_recovery_draft_preserved);
    assert(outcome.name_rejected_generation == outcome.prior_generation);
    assert(outcome.name_recovery_generation == outcome.generation);
    assert(outcome.prior_generation && outcome.prior_generation != outcome.generation);
    assert(outcome.character_count == 1 && outcome.server_flags[0] ==
           (2u | (1u << 8) | (1u << 16)));
    assert(outcome.dedicated_creation == peer.exclusive);
    assert(outcome.exclusive_name_started == peer.exclusive);
    assert(outcome.ordinary_name_started != peer.exclusive);
    assert(outcome.dedicated_mode_flags == (peer.exclusive ? 0xc0u : 0));
    assert(!strcmp(outcome.name_rejection_reason, peer.reason));
    return 0;
}

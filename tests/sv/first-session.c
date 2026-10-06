#include "endpoint-run.h"
#include "../../src/common/pack.h"
#include <SDL3/SDL.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef enum { PEER_SUCCESS, PEER_DISCONNECT, PEER_QUIT, PEER_RETRY } PeerMode;
typedef struct {
    int listener;
    PeerMode mode;
    bool contact_exact, verify_exact, login_exact, ping_exact, choice_exact, closed;
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

static void push_event(Uint32 type, SDL_Keycode key)
{
    SDL_Event event = {0};
    event.type = type;
    if (type == SDL_EVENT_KEY_DOWN) event.key.key = key;
    assert(SDL_PushEvent(&event));
}

static void push_text(const char *text)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = text;
    assert(SDL_PushEvent(&event));
}

static int credential_run(void *unused)
{
    (void)unused;
    for (int i = 0; i < 200 && !(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO); ++i)
        SDL_Delay(5);
    SDL_Delay(100);
    push_text("Test");
    push_event(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
    push_text("pw");
    push_event(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
    return 0;
}

static int peer_run(void *opaque)
{
    Peer *peer = opaque;
    int attempt = 0;
retry:
    ;
    int socket = accept(peer->listener, NULL, NULL);
    assert(socket >= 0);
    struct timeval timeout = {.tv_sec = 8};
    assert(!setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));

    unsigned char number[40];
    char real[80], account[80], host[80], password[80];
    assert(exact(socket, number, 4) && field(socket, real, sizeof(real)) &&
           exact(socket, number + 4, 3) && field(socket, account, sizeof(account)) &&
           field(socket, host, sizeof(host)) && exact(socket, number + 7, 26));
    peer->contact_exact = !strcmp(real, "PLAYER") && !strcmp(account, "Test") &&
                          !strcmp(host, "localhost") && number[0] == 0 && number[1] == 0 &&
                          number[2] == 0x30 && number[3] == 0x39;

    unsigned char contact[34], *out = contact;
    *out++ = 255; *out++ = 0; put32(&out, 0); put32(&out, 2);
    const uint32_t version[6] = {4, 9, 4, 0, 0, 0};
    for (size_t i = 0; i < 6; ++i) put32(&out, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == (ssize_t)sizeof(contact));

    unsigned char verify_type;
    assert(exact(socket, &verify_type, 1) && field(socket, real, sizeof(real)) &&
           field(socket, account, sizeof(account)) && field(socket, password, sizeof(password)));
    peer->verify_exact = verify_type == 1 && !strcmp(real, "PLAYER") &&
                         !strcmp(account, "Test") && !strcmp(password, "Z]");
    static const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == (ssize_t)sizeof(verified));

    static const unsigned char motd[] = "Managed peer MOTD";
    unsigned char setup[128]; out = setup;
    put32(&out, sizeof(motd) - 1); put16(&out, 20);
    *out++ = 0; *out++ = 0; *out++ = 0; put32(&out, 13);
    memcpy(out, motd, sizeof(motd) - 1); out += sizeof(motd) - 1;
    size_t setup_size = (size_t)(out - setup);
    for (size_t at = 0; at < setup_size; at += 3) {
        size_t count = setup_size - at < 3 ? setup_size - at : 3;
        assert(send(socket, setup + at, count, 0) == (ssize_t)count);
        SDL_Delay(1);
    }

    unsigned char login_start[8];
    assert(exact(socket, login_start, sizeof(login_start)));
    peer->login_exact = login_start[0] == PKT_LOGIN && login_start[1] == 0 &&
                        login_start[2] == 0xf4 && login_start[3] == 0x43;
    if (peer->mode == PEER_RETRY && !attempt) {
        static const unsigned char rejected[] = {PKT_QUIT, 'W','r','o','n','g',' ','p','a','s','s','w','o','r','d',0};
        assert(send(socket, rejected, sizeof(rejected), 0) == (ssize_t)sizeof(rejected));
        close(socket);
        SDL_Delay(200);
        push_event(SDL_EVENT_KEY_DOWN, SDLK_R);
        ++attempt;
        goto retry;
    }

    unsigned char conversation[256]; out = conversation;
    *out++ = PKT_SERVERDETAILS;
    put32(&out, 8); put32(&out, 0); put32(&out, 0); put32(&out, 0);
    *out++ = PKT_KEEPALIVE;
    unsigned char *ping = out;
    *out++ = PKT_PING; *out++ = 0;
    put32(&out, 7); put32(&out, 8); put32(&out, 9); *out++ = 0;
    *out++ = PKT_LOGIN; put16(&out, 1);
    *out++ = 0xff; *out++ = 'W'; *out++ = 0;
    memcpy(out, "Hero", 5); out += 5;
    put16(&out, 42); put16(&out, 0); put16(&out, 0);
    memcpy(out, "at home", 8); out += 8;
    *out++ = PKT_LOGIN; put16(&out, 0); *out++ = 0; *out++ = 0;
    put16(&out, 0); put16(&out, 0); put16(&out, 0); *out++ = 0;
    size_t conversation_size = (size_t)(out - conversation);
    assert(send(socket, conversation, conversation_size, 0) == (ssize_t)conversation_size);

    unsigned char echo[15];
    assert(exact(socket, echo, sizeof(echo)));
    peer->ping_exact = !memcmp(echo, ping, sizeof(echo)) && echo[1] == 1;
    /* pong is the only changed byte. */
    peer->ping_exact = peer->ping_exact ||
        (echo[0] == ping[0] && echo[1] == 1 && !memcmp(echo + 2, ping + 2, 13));

    if (peer->mode == PEER_QUIT) {
        SDL_Delay(150);
        push_event(SDL_EVENT_WINDOW_RESIZED, 0);
        push_event(SDL_EVENT_KEY_DOWN, SDLK_Q);
        unsigned char byte;
        peer->closed = recv(socket, &byte, 1, 0) == 0;
        close(socket);
        return 0;
    }

    unsigned char choice_type;
    char choice[80];
    assert(exact(socket, &choice_type, 1) && field(socket, choice, sizeof(choice)));
    peer->choice_exact = choice_type == PKT_LOGIN && !strcmp(choice, "Hero");
    if (peer->mode == PEER_DISCONNECT) {
        close(socket);
        peer->closed = true;
        SDL_Delay(150);
        push_event(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE);
        return 0;
    }

    const unsigned char accepted = 0;
    assert(send(socket, &accepted, 1, 0) == 1);
    SDL_Delay(150);
    push_event(SDL_EVENT_WINDOW_RESIZED, 0);
    push_event(SDL_EVENT_WINDOW_FOCUS_LOST, 0);
    push_event(SDL_EVENT_WINDOW_FOCUS_GAINED, 0);
    push_event(SDL_EVENT_KEY_DOWN, SDLK_SPACE);
    SDL_Delay(150);
    push_event(SDL_EVENT_QUIT, 0);
    unsigned char byte;
    peer->closed = recv(socket, &byte, 1, 0) == 0;
    close(socket);
    return 0;
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0), .mode = PEER_SUCCESS};
    assert(peer.listener >= 0);
    if (!strcmp(argv[3], "disconnect")) peer.mode = PEER_DISCONNECT;
    else if (!strcmp(argv[3], "quit")) peer.mode = PEER_QUIT;
    else if (!strcmp(argv[3], "retry")) peer.mode = PEER_RETRY;
    else assert(!strcmp(argv[3], "success"));
    int reuse = 1;
    assert(!setsockopt(peer.listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 1));
    socklen_t address_size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &address_size));
    SDL_Thread *worker = SDL_CreateThread(peer_run, "sv-b020-peer", &peer);
    assert(worker);
    SDL_Thread *credentials = peer.mode == PEER_RETRY ? NULL :
        SDL_CreateThread(credential_run, "sv-b020-credentials", NULL);
    assert(peer.mode == PEER_RETRY || credentials);

    SvEndpointOutcome outcome = {0};
    SvEndpointOptions options = {.root = argv[1], .library = argv[2],
        .server = "127.0.0.1", .width = 1024, .height = 768, .windowed = 1,
        .port = ntohs(address.sin_port),
        .real_name = "PLAYER", .character = peer.mode == PEER_QUIT ? NULL : "hero",
        .outcome = &outcome};
    if (peer.mode == PEER_RETRY) {
        options.account = "Test";
        options.password = "pw";
    }
    int result = sv_endpoint_run(options);
    if (credentials) SDL_WaitThread(credentials, NULL);
    SDL_WaitThread(worker, NULL);
    close(peer.listener);
    assert(peer.contact_exact && peer.verify_exact && peer.login_exact && peer.ping_exact);
    if (peer.mode == PEER_SUCCESS || peer.mode == PEER_RETRY) {
        assert(result == 0 && peer.choice_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_LIVE_HANDOFF);
        assert(!strcmp(outcome.selected_character, "Hero"));
    } else if (peer.mode == PEER_DISCONNECT) {
        assert(result == 1 && peer.choice_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_DISCONNECTED);
    } else {
        assert(result == 0 && !peer.choice_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_OVERVIEW);
    }
    puts("SV-B-020 managed-peer production flow passed");
    return 0;
}

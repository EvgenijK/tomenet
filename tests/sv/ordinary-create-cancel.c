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

typedef enum { ORDINARY_AVAILABLE, ORDINARY_FIRST_RUN, ORDINARY_FULL } PeerMode;

typedef struct {
    int listener;
    PeerMode mode;
    bool contact_exact, verify_exact, login_exact, no_character_request, closed;
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

static void push_key(SDL_Keycode key)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = key;
    if (key == SDLK_N) event.key.mod = SDL_KMOD_SHIFT;
    assert(SDL_PushEvent(&event));
}

static void send_row(unsigned char **at, int16_t mode, const char *name)
{
    *(*at)++ = PKT_LOGIN;
    put16(at, (uint16_t)mode);
    *(*at)++ = 0xff;
    *(*at)++ = 'W';
    *(*at)++ = 0;
    size_t length = strlen(name) + 1;
    memcpy(*at, name, length);
    *at += length;
    put16(at, 10);
    put16(at, 0);
    put16(at, 0);
    memcpy(*at, "Home", 5);
    *at += 5;
}

static void send_overview(int socket, PeerMode mode)
{
    unsigned char response[256], *at = response;
    *at++ = PKT_SERVERDETAILS;
    /* Two ordinary slots plus one IDDC and one PvP dedicated slot. */
    put32(&at, 2u | (1u << 8) | (1u << 16));
    put32(&at, 0);
    put32(&at, 0);
    put32(&at, 0xc0);
    if (mode != ORDINARY_FIRST_RUN) {
        send_row(&at, 0x40, "Dive");
        send_row(&at, 0x80, "Duel");
        send_row(&at, 0, "Hero");
        if (mode == ORDINARY_FULL) send_row(&at, 0, "Mage");
    }
    send_row(&at, 0, "");
    assert(send(socket, response, (size_t)(at - response), 0) ==
           (ssize_t)(at - response));
}

static int peer_run(void *opaque)
{
    Peer *peer = opaque;
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
                          !strcmp(host, "localhost");

    unsigned char contact[34], *at = contact;
    *at++ = 255;
    *at++ = 0;
    put32(&at, 0);
    put32(&at, 2);
    const uint32_t version[6] = {4, 9, 4, 0, 0, 0};
    for (size_t i = 0; i < 6; ++i) put32(&at, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == (ssize_t)sizeof(contact));

    unsigned char verify_type;
    assert(exact(socket, &verify_type, 1) && field(socket, real, sizeof(real)) &&
           field(socket, account, sizeof(account)) && field(socket, password, sizeof(password)));
    peer->verify_exact = verify_type == PKT_VERIFY && !strcmp(real, "PLAYER") &&
                         !strcmp(account, "Test") && !strcmp(password, "Z]");
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
    peer->login_exact = login_start[0] == PKT_LOGIN && login_start[1] == 0 &&
                        login_start[2] == 0xf4 && login_start[3] == 0x43;
    send_overview(socket, peer->mode);

    SDL_Delay(200);
    push_key(SDLK_N);
    SDL_Delay(100);
    if (peer->mode != ORDINARY_FULL) {
        push_key(SDLK_ESCAPE);
        SDL_Delay(100);
    }
    push_key(SDLK_Q);

    unsigned char byte;
    ssize_t received = recv(socket, &byte, 1, 0);
    peer->no_character_request = received == 0;
    peer->closed = received == 0;
    close(socket);
    return 0;
}

static void run_case(const char *profile, const char *library, PeerMode mode)
{
    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0), .mode = mode};
    assert(peer.listener >= 0);
    int reuse = 1;
    assert(!setsockopt(peer.listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 1));
    socklen_t address_size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &address_size));
    SDL_Thread *worker = SDL_CreateThread(peer_run, "sv-b022-ordinary-peer", &peer);
    assert(worker);

    SvEndpointOutcome outcome = {0};
    int result = sv_endpoint_run((SvEndpointOptions){
        .root = profile, .library = library, .server = "127.0.0.1",
        .width = 1024, .height = 768, .windowed = 1,
        .port = ntohs(address.sin_port), .account = "Test", .password = "pw",
        .real_name = "PLAYER", .outcome = &outcome
    });
    SDL_WaitThread(worker, NULL);
    close(peer.listener);

    assert(result == 0 && peer.contact_exact && peer.verify_exact && peer.login_exact);
    assert(peer.no_character_request && peer.closed);
    assert(outcome.phase == SV_PREGAME_OVERVIEW && outcome.authenticated);
    assert(!outcome.selected_character[0]);
    if (mode != ORDINARY_FULL) {
        assert(outcome.ordinary_name_started && outcome.ordinary_name_cancelled);
        assert(outcome.name_entry_generation == outcome.generation);
        assert(outcome.name_cancel_generation == outcome.generation);
        assert(outcome.name_entry_overview_revision == outcome.revision);
        assert(outcome.name_cancel_overview_revision == outcome.revision);
    } else {
        assert(!outcome.ordinary_name_started && !outcome.ordinary_name_cancelled);
    }
}

int main(int argc, char **argv)
{
    assert(argc == 3);
    run_case(argv[1], argv[2], ORDINARY_AVAILABLE);
    run_case(argv[1], argv[2], ORDINARY_FIRST_RUN);
    run_case(argv[1], argv[2], ORDINARY_FULL);
    return 0;
}

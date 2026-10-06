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

typedef enum {
    PEER_CONTACT_REJECT,
    PEER_BAD_PASSWORD,
    PEER_SERVER_REJECT,
    PEER_LOGIN_REJECT,
    PEER_MALFORMED,
    PEER_DISCONNECT,
    PEER_RETRY
} PeerMode;

typedef struct {
    int listener;
    PeerMode mode;
    int contacts, logins;
    bool closed;
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
    assert(SDL_PushEvent(&event));
}

static void push_text(const char *value)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = value;
    assert(SDL_PushEvent(&event));
}

static int accept_contact(Peer *peer, char account[80])
{
    int socket = accept(peer->listener, NULL, NULL);
    assert(socket >= 0);
    struct timeval timeout = {.tv_sec = 8};
    assert(!setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));
    unsigned char fixed[33];
    char real[80], host[80];
    assert(exact(socket, fixed, 4) && field(socket, real, sizeof(real)) &&
           exact(socket, fixed + 4, 3) && field(socket, account, 80) &&
           field(socket, host, sizeof(host)) && exact(socket, fixed + 7, 26));
    assert(!strcmp(real, "PLAYER") && !strcmp(host, "localhost"));
    ++peer->contacts;
    return socket;
}

static void contact_ready(Peer *peer, int socket)
{
    unsigned char response[34], *at = response;
    *at++ = 255; *at++ = 0; put32(&at, 0); put32(&at, 0x12);
    const uint32_t version[6] = {4,9,4,0,0,0};
    for (size_t i = 0; i < 6; ++i) put32(&at, version[i]);
    assert(send(socket, response, sizeof(response), 0) == (ssize_t)sizeof(response));

    unsigned char verify;
    char real[80], account[80], password[80];
    assert(exact(socket, &verify, 1) && field(socket, real, sizeof(real)) &&
           field(socket, account, sizeof(account)) && field(socket, password, sizeof(password)));
    assert(verify == PKT_VERIFY);
    static const unsigned char verified[] = {2,1,122,6, 0,0,0x30,0x39};
    assert(send(socket, verified, sizeof(verified), 0) == (ssize_t)sizeof(verified));
    unsigned char setup[13], *setup_at = setup;
    put32(&setup_at, 0); put16(&setup_at, 20);
    *setup_at++ = 0; *setup_at++ = 0; *setup_at++ = 0; put32(&setup_at, 13);
    assert(send(socket, setup, sizeof(setup), 0) == (ssize_t)sizeof(setup));
    unsigned char login[8];
    assert(exact(socket, login, sizeof(login)) && login[0] == PKT_LOGIN);
    ++peer->logins;
}

static void send_empty_overview(int socket)
{
    unsigned char response[29], *at = response;
    *at++ = PKT_SERVERDETAILS;
    put32(&at, 0x10203040); put32(&at, 3); put32(&at, 2); put32(&at, 1);
    *at++ = PKT_LOGIN; put16(&at, 0);
    *at++ = 0; *at++ = 0; put16(&at, 0); put16(&at, 0); put16(&at, 0); *at++ = 0;
    assert(at == response + sizeof(response));
    assert(send(socket, response, sizeof(response), 0) == (ssize_t)sizeof(response));
}

static void wait_closed(Peer *peer, int socket)
{
    unsigned char byte;
    peer->closed = recv(socket, &byte, 1, 0) == 0;
    close(socket);
}

static int peer_run(void *opaque)
{
    Peer *peer = opaque;
    char account[80];
    int socket = accept_contact(peer, account);
    assert(!strcmp(account, "Invalid"));
    if (peer->mode == PEER_CONTACT_REJECT || peer->mode == PEER_BAD_PASSWORD ||
        peer->mode == PEER_SERVER_REJECT) {
        unsigned char rejected[] = {255, E_LETTER, 0,0,0,0, 0,0,0,0};
        if (peer->mode == PEER_BAD_PASSWORD) rejected[1] = E_PASSWORD;
        else if (peer->mode == PEER_SERVER_REJECT) rejected[1] = E_GAME_FULL;
        assert(send(socket, rejected, sizeof(rejected) - 1, 0) ==
               (ssize_t)sizeof(rejected) - 1);
        SDL_Delay(20);
        assert(send(socket, rejected + sizeof(rejected) - 1, 1, 0) == 1);
        wait_closed(peer, socket);
        push_key(SDLK_ESCAPE);
        return 0;
    }
    contact_ready(peer, socket);
    if (peer->mode == PEER_LOGIN_REJECT || peer->mode == PEER_RETRY) {
        static const unsigned char flags[] = {
            PKT_SERVERDETAILS, 0x10,0x20,0x30,0x40, 0,0,0,3,
            0,0,0,2, 0,0,0,1
        };
        static const unsigned char rejected[] = {
            PKT_QUIT, 'A','c','c','o','u','n','t',' ','a','l','r','e','a','d','y',' ','e','x','i','s','t','s','.',0
        };
        assert(send(socket, flags, sizeof(flags), 0) == (ssize_t)sizeof(flags));
        for (size_t i = 0; i < sizeof(rejected); ++i)
            assert(send(socket, rejected + i, 1, 0) == 1);
        wait_closed(peer, socket);
        if (peer->mode == PEER_LOGIN_REJECT) {
            push_key(SDLK_ESCAPE);
            return 0;
        }
        push_key(SDLK_R);
        socket = accept_contact(peer, account);
        assert(!strcmp(account, "Invalid"));
        contact_ready(peer, socket);
        send_empty_overview(socket);
        SDL_Delay(100);
        push_key(SDLK_Q);
        wait_closed(peer, socket);
        return 0;
    }
    if (peer->mode == PEER_MALFORMED) {
        static const unsigned char malformed[] = {
            PKT_SERVERDETAILS, 0x10,0x20,0x30,0x40, 0,0,0,3,
            0,0,0,2, 0,0,0,1, 0xfe
        };
        assert(send(socket, malformed, sizeof(malformed), 0) == (ssize_t)sizeof(malformed));
    } else {
        static const unsigned char partial[] = {
            PKT_SERVERDETAILS, 0x10,0x20,0x30,0x40, 0,0,0
        };
        assert(send(socket, partial, sizeof(partial), 0) == (ssize_t)sizeof(partial));
        close(socket);
        peer->closed = true;
        SDL_Delay(150);
        push_key(SDLK_ESCAPE);
        return 0;
    }
    wait_closed(peer, socket);
    push_key(SDLK_ESCAPE);
    return 0;
}

static int cancel_credentials(void *opaque)
{
    bool from_password = *(bool *)opaque;
    for (int i = 0; i < 200 && !(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO); ++i)
        SDL_Delay(5);
    SDL_Delay(100);
    if (from_password) {
        push_text("Invalid");
        push_key(SDLK_RETURN);
        push_text("secret");
        push_key(SDLK_ESCAPE);
    }
    push_key(SDLK_ESCAPE);
    return 0;
}

static void assert_no_overview(const SvEndpointOutcome *outcome)
{
    assert(!outcome->authenticated && !outcome->character_count && !outcome->creation_flags);
    assert(!outcome->server_flags[0] && !outcome->server_flags[1] &&
           !outcome->server_flags[2] && !outcome->server_flags[3]);
    assert(!outcome->credential_save_started && !outcome->credential_saved);
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    bool cancel_password = !strcmp(argv[3], "cancel-password");
    bool cancel_account = !strcmp(argv[3], "cancel-account");
    if (cancel_password || cancel_account) {
        SDL_Thread *input = SDL_CreateThread(cancel_credentials, "sv-account-cancel",
                                              &cancel_password);
        assert(input);
        SvEndpointOutcome outcome = {0};
        int result = sv_endpoint_run((SvEndpointOptions){
            .root = argv[1], .library = argv[2], .server = "127.0.0.1",
            .width = 1024, .height = 768, .windowed = 1,
            .port = 1, .real_name = "PLAYER", .outcome = &outcome
        });
        SDL_WaitThread(input, NULL);
        assert(result == 0 && !outcome.generation);
        assert_no_overview(&outcome);
        return 0;
    }

    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0)};
    assert(peer.listener >= 0);
    if (!strcmp(argv[3], "contact-reject")) peer.mode = PEER_CONTACT_REJECT;
    else if (!strcmp(argv[3], "bad-password")) peer.mode = PEER_BAD_PASSWORD;
    else if (!strcmp(argv[3], "server-reject")) peer.mode = PEER_SERVER_REJECT;
    else if (!strcmp(argv[3], "login-reject")) peer.mode = PEER_LOGIN_REJECT;
    else if (!strcmp(argv[3], "malformed")) peer.mode = PEER_MALFORMED;
    else if (!strcmp(argv[3], "disconnect")) peer.mode = PEER_DISCONNECT;
    else { assert(!strcmp(argv[3], "retry")); peer.mode = PEER_RETRY; }
    int reuse = 1;
    assert(!setsockopt(peer.listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 2));
    socklen_t address_size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &address_size));
    SDL_Thread *server = SDL_CreateThread(peer_run, "sv-account-failure-peer", &peer);
    assert(server);

    SvEndpointOutcome outcome = {0};
    int result = sv_endpoint_run((SvEndpointOptions){
        .root = argv[1], .library = argv[2], .server = "127.0.0.1",
        .width = 1024, .height = 768, .windowed = 1,
        .port = ntohs(address.sin_port), .real_name = "PLAYER",
        .account = "invalid", .password = "secret", .outcome = &outcome
    });
    SDL_WaitThread(server, NULL);
    close(peer.listener);
    assert(peer.contacts == (peer.mode == PEER_RETRY ? 2 : 1));
    assert(peer.logins == (peer.mode == PEER_CONTACT_REJECT ||
                           peer.mode == PEER_BAD_PASSWORD ||
                           peer.mode == PEER_SERVER_REJECT ? 0 :
                           peer.mode == PEER_RETRY ? 2 : 1));
    assert(peer.closed);
    if (peer.mode == PEER_RETRY) {
        assert(result == 0 && outcome.phase == SV_PREGAME_OVERVIEW);
        assert(outcome.authenticated && !outcome.character_count);
        assert(outcome.creation_flags == 0x10 && outcome.server_flags[0] == 0x10203040);
        assert(outcome.prior_generation && outcome.prior_generation != outcome.generation);
        assert(!outcome.reason[0]);
    } else {
        assert(result == 1);
        assert(outcome.phase == (peer.mode == PEER_CONTACT_REJECT ||
                                 peer.mode == PEER_BAD_PASSWORD ||
                                 peer.mode == PEER_SERVER_REJECT ||
                                 peer.mode == PEER_LOGIN_REJECT ?
                                 SV_PREGAME_FAILED : SV_PREGAME_DISCONNECTED));
        if (peer.mode == PEER_CONTACT_REJECT)
            assert(!strcmp(outcome.reason, "Server rejected contact: invalid account name."));
        else if (peer.mode == PEER_BAD_PASSWORD)
            assert(!strcmp(outcome.reason, "Server rejected contact: invalid password."));
        else if (peer.mode == PEER_SERVER_REJECT)
            assert(!strcmp(outcome.reason, "Server rejected contact: game is full."));
        else if (peer.mode == PEER_LOGIN_REJECT)
            assert(!strcmp(outcome.reason, "Account already exists."));
        else if (peer.mode == PEER_MALFORMED)
            assert(!strcmp(outcome.reason, "Session closed: packet decode failed"));
        else assert(strstr(outcome.reason, "closed the connection"));
        assert_no_overview(&outcome);
    }
    return 0;
}

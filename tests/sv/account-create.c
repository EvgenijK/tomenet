#include "endpoint-run.h"
#include "credential/vault.h"
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

struct SvVaultRequest { bool store; uint64_t generation; int ordinal; };

static SDL_AtomicInt server_response_complete;
static SDL_AtomicInt store_after_confirmation;
static SDL_AtomicInt exact_store_identity;
static SDL_AtomicInt exact_store_secret;
static SDL_AtomicInt store_cancelled;
static SDL_AtomicInt store_calls;
static SDL_AtomicInt stale_completion_freed;
static uint16_t expected_port;
static SvVaultResult store_result = SV_VAULT_SAVED;
static bool retry_mode;
static SvVaultRequest *detached_request;
static uint64_t first_store_generation, second_store_generation;

bool sv_vault_key(char out[SV_VAULT_KEY_CAPACITY], const void *server,
                  size_t server_size, uint16_t port, const void *account,
                  size_t account_size)
{
    static const char key[] = "tomenet-sv/v1/account-create-check";
    bool exact = server_size == 9 && !memcmp(server, "127.0.0.1", 9) &&
        port == expected_port && account_size == 13 &&
        !memcmp(account, "UnusedAccount", 13);
    SDL_SetAtomicInt(&exact_store_identity, exact);
    memcpy(out, key, sizeof(key));
    return true;
}

SvVaultRequest *sv_vault_lookup(const char *key, uint64_t generation)
{
    (void)key;
    (void)generation;
    return SDL_calloc(1, sizeof(SvVaultRequest));
}

SvVaultRequest *sv_vault_store(const char *key, uint64_t generation,
                               const void *secret, size_t secret_size)
{
    static const unsigned char expected[] = {'s','e','c','r','e','t'};
    (void)key;
    SvVaultRequest *request = SDL_calloc(1, sizeof(*request));
    assert(request);
    request->store = true;
    request->generation = generation;
    request->ordinal = SDL_AddAtomicInt(&store_calls, 1) + 1;
    if (request->ordinal == 1) first_store_generation = generation;
    else if (request->ordinal == 2) second_store_generation = generation;
    SDL_SetAtomicInt(&store_after_confirmation,
                     SDL_GetAtomicInt(&server_response_complete));
    SDL_SetAtomicInt(&exact_store_secret,
                     secret_size == sizeof(expected) &&
                     !memcmp(secret, expected, sizeof(expected)));
    return request;
}

SvVaultResult sv_vault_poll(SvVaultRequest **slot, uint64_t generation,
                            void *secret, size_t capacity, size_t *size)
{
    (void)secret;
    (void)capacity;
    if (size) *size = 0;
    assert(slot && *slot);
    assert(generation == (*slot)->generation);
    if (retry_mode && (*slot)->ordinal == 2 && detached_request) {
        SDL_free(detached_request);
        detached_request = NULL;
        SDL_SetAtomicInt(&stale_completion_freed, 1);
    }
    SvVaultResult result = (*slot)->store ?
        (retry_mode ? ((*slot)->ordinal == 1 ? SV_VAULT_PENDING : SV_VAULT_SAVED) :
         store_result) : SV_VAULT_MISSING;
    if (result == SV_VAULT_PENDING) return result;
    SDL_free(*slot);
    *slot = NULL;
    return result;
}

void sv_vault_cancel(SvVaultRequest **slot)
{
    if (!slot || !*slot) return;
    if ((*slot)->store) SDL_SetAtomicInt(&store_cancelled, 1);
    if (retry_mode && (*slot)->ordinal == 1) detached_request = *slot;
    else SDL_free(*slot);
    *slot = NULL;
}

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

static void push_text(const char *text)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = text;
    assert(SDL_PushEvent(&event));
}

static int enter_new_credentials(void *unused)
{
    (void)unused;
    for (int i = 0; i < 200 && !(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO); ++i)
        SDL_Delay(5);
    SDL_Delay(100);
    push_text("unusedAccount");
    push_key(SDLK_RETURN);
    push_text("secret");
    push_key(SDLK_RETURN);
    return 0;
}

typedef struct {
    int listener, attempts;
    bool retry, contact_exact, verify_exact, login_exact, closed;
} Peer;

static void peer_connection(Peer *peer, int socket, bool retry)
{
    struct timeval timeout = {.tv_sec = 8};
    assert(!setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));
    int prior_stores = SDL_GetAtomicInt(&store_calls);

    unsigned char prefix[7], suffix[26];
    char real[80], account[80], host[80], password[80];
    assert(exact(socket, prefix, 4) && field(socket, real, sizeof(real)) &&
           exact(socket, prefix + 4, 3) && field(socket, account, sizeof(account)) &&
           field(socket, host, sizeof(host)) && exact(socket, suffix, sizeof(suffix)));
    peer->contact_exact &= !memcmp(prefix, "\0\0\x30\x39\0\0\xff", 7) &&
        !strcmp(real, "PLAYER") && !strcmp(account, "UnusedAccount") &&
        !strcmp(host, "localhost") && suffix[0] == 0xff && suffix[1] == 0xff;

    unsigned char contact[34], *out = contact;
    *out++ = 255; *out++ = 0; put32(&out, 0); put32(&out, 0x12);
    const uint32_t version[6] = {4,9,4,0,0,0};
    for (size_t i = 0; i < 6; ++i) put32(&out, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == (ssize_t)sizeof(contact));

    unsigned char verify_type;
    assert(exact(socket, &verify_type, 1) && field(socket, real, sizeof(real)) &&
           field(socket, account, sizeof(account)) && field(socket, password, sizeof(password)));
    static const unsigned char encoded[] = {'Y','O','I','X','O','^',0};
    peer->verify_exact &= verify_type == PKT_VERIFY && !strcmp(real, "PLAYER") &&
        !strcmp(account, "UnusedAccount") && !memcmp(password, encoded, sizeof(encoded));

    static const unsigned char verified[] = {2,1,122,6, 0,0,0x30,0x39};
    assert(send(socket, verified, sizeof(verified), 0) == (ssize_t)sizeof(verified));
    unsigned char setup[13], *setup_at = setup;
    put32(&setup_at, 0); put16(&setup_at, 20);
    *setup_at++ = 0; *setup_at++ = 0; *setup_at++ = 0; put32(&setup_at, 13);
    assert(send(socket, setup, sizeof(setup), 0) == (ssize_t)sizeof(setup));

    unsigned char login[8];
    assert(exact(socket, login, sizeof(login)));
    peer->login_exact &= login[0] == PKT_LOGIN && login[1] == 0 &&
        login[2] == 0xf4 && login[3] == 0x43;

    unsigned char response[29], *response_at = response;
    *response_at++ = PKT_SERVERDETAILS;
    put32(&response_at, 0x10203040); put32(&response_at, 3);
    put32(&response_at, 2); put32(&response_at, 1);
    *response_at++ = PKT_LOGIN; put16(&response_at, 0);
    *response_at++ = 0; *response_at++ = 0;
    put16(&response_at, 0); put16(&response_at, 0); put16(&response_at, 0);
    *response_at++ = 0;
    assert(response_at == response + sizeof(response));
    assert(send(socket, response, sizeof(response) - 1, 0) == (ssize_t)sizeof(response) - 1);
    SDL_Delay(150);
    assert(SDL_GetAtomicInt(&store_calls) == prior_stores);
    SDL_SetAtomicInt(&server_response_complete, 1);
    assert(send(socket, response + sizeof(response) - 1, 1, 0) == 1);
    for (int i = 0; i < 200 && SDL_GetAtomicInt(&store_calls) == prior_stores; ++i)
        SDL_Delay(5);
    assert(SDL_GetAtomicInt(&store_calls) == prior_stores + 1);
    if (retry) {
        close(socket);
        SDL_Delay(150);
        push_key(SDLK_R);
        return;
    }
    push_key(SDLK_Q);
    unsigned char byte;
    peer->closed = recv(socket, &byte, 1, 0) == 0;
    close(socket);
}

static int peer_run(void *opaque)
{
    Peer *peer = opaque;
    int count = peer->retry ? 2 : 1;
    for (int attempt = 0; attempt < count; ++attempt) {
        int socket = accept(peer->listener, NULL, NULL);
        assert(socket >= 0);
        peer_connection(peer, socket, peer->retry && attempt == 0);
        ++peer->attempts;
    }
    return 0;
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    if (!strcmp(argv[3], "unavailable")) store_result = SV_VAULT_UNAVAILABLE;
    else if (!strcmp(argv[3], "locked")) store_result = SV_VAULT_LOCKED;
    else if (!strcmp(argv[3], "refused")) store_result = SV_VAULT_REFUSED;
    else if (!strcmp(argv[3], "invalid")) store_result = SV_VAULT_INVALID;
    else if (!strcmp(argv[3], "error")) store_result = SV_VAULT_ERROR;
    else if (!strcmp(argv[3], "pending")) store_result = SV_VAULT_PENDING;
    else if (!strcmp(argv[3], "retry")) retry_mode = true;
    else assert(!strcmp(argv[3], "saved"));
    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0), .retry = retry_mode,
                 .contact_exact = true, .verify_exact = true, .login_exact = true};
    assert(peer.listener >= 0);
    int reuse = 1;
    assert(!setsockopt(peer.listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 1));
    socklen_t address_size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &address_size));
    expected_port = ntohs(address.sin_port);
    SDL_Thread *server = SDL_CreateThread(peer_run, "sv-account-create-peer", &peer);
    SDL_Thread *credentials = retry_mode ? NULL :
        SDL_CreateThread(enter_new_credentials, "sv-account-create-input", NULL);
    assert(server && (retry_mode || credentials));

    SvEndpointOutcome outcome = {0};
    SvEndpointOptions options = {
        .root = argv[1], .library = argv[2], .server = "127.0.0.1",
        .width = 1024, .height = 768, .windowed = 1,
        .port = expected_port, .real_name = "PLAYER", .outcome = &outcome
    };
    if (retry_mode) {
        options.account = "UnusedAccount";
        options.password = "secret";
    }
    int result = sv_endpoint_run(options);
    if (credentials) SDL_WaitThread(credentials, NULL);
    SDL_WaitThread(server, NULL);
    close(peer.listener);
    assert(result == 0 && peer.contact_exact && peer.verify_exact &&
           peer.login_exact && peer.closed);
    assert(peer.attempts == (retry_mode ? 2 : 1));
    assert(outcome.phase == SV_PREGAME_OVERVIEW && outcome.authenticated);
    assert(!outcome.character_count && outcome.creation_flags == 0x10);
    assert(outcome.server_flags[0] == 0x10203040 && outcome.server_flags[1] == 3 &&
           outcome.server_flags[2] == 2 && outcome.server_flags[3] == 1);
    assert(outcome.credential_save_started);
    if (store_result == SV_VAULT_SAVED || retry_mode) {
        assert(outcome.credential_saved);
        assert(outcome.credential_save == SV_CREDENTIAL_SAVE_SAVED);
    } else if (store_result != SV_VAULT_PENDING) {
        assert(!outcome.credential_saved);
        assert(outcome.credential_save == SV_CREDENTIAL_SAVE_SESSION_ONLY);
    } else {
        assert(!outcome.credential_saved);
        assert(outcome.credential_save == SV_CREDENTIAL_SAVE_SESSION_ONLY);
        assert(SDL_GetAtomicInt(&store_cancelled));
    }
    if (retry_mode) {
        assert(SDL_GetAtomicInt(&store_calls) == 2);
        assert(SDL_GetAtomicInt(&store_cancelled));
        assert(SDL_GetAtomicInt(&stale_completion_freed));
        assert(first_store_generation && second_store_generation &&
               first_store_generation != second_store_generation);
    }
    assert(SDL_GetAtomicInt(&store_after_confirmation));
    assert(SDL_GetAtomicInt(&exact_store_identity));
    assert(SDL_GetAtomicInt(&exact_store_secret));
    assert(!detached_request);
    return 0;
}

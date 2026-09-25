#include "credential/vault.h"
#include <SDL3/SDL.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <wincred.h>
#else
#include <libsecret/secret.h>
#endif

struct SvVaultRequest {
    SDL_Mutex *mutex;
    SDL_Thread *thread;
    uint64_t generation;
    bool store, done, abandoned;
    SvVaultResult result;
    char key[SV_VAULT_KEY_CAPACITY];
    unsigned char secret[SV_VAULT_SECRET_CAPACITY];
    size_t secret_size;
};

static void wipe(void *data, size_t size)
{
    volatile unsigned char *p = data;
    while (size--) *p++ = 0;
}

static void destroy(SvVaultRequest *request)
{
    if (!request) return;
    wipe(request->secret, sizeof(request->secret));
    SDL_DestroyMutex(request->mutex);
    SDL_free(request);
}

bool sv_vault_key(char out[SV_VAULT_KEY_CAPACITY], const void *server,
                  size_t server_size, uint16_t port, const void *account,
                  size_t account_size)
{
    static const char prefix[] = "tomenet-sv/v1/";
    static const char digits[] = "0123456789abcdef";
    if (!out || !server || !account || !server_size || !account_size || !port ||
        server_size > UINT16_MAX || account_size > UINT16_MAX) return false;
    size_t bytes = 2 + server_size + 2 + 2 + account_size;
    if (bytes > (SV_VAULT_KEY_CAPACITY - sizeof(prefix)) / 2) return false;
    unsigned char tuple[256];
    if (bytes > sizeof(tuple)) return false;
    size_t at = 0;
#define PUT16(value) do { tuple[at++] = (unsigned char)((value) >> 8); \
                          tuple[at++] = (unsigned char)(value); } while (0)
    PUT16(server_size);
    memcpy(tuple + at, server, server_size); at += server_size;
    PUT16(port);
    PUT16(account_size);
    memcpy(tuple + at, account, account_size); at += account_size;
#undef PUT16
    memcpy(out, prefix, sizeof(prefix) - 1);
    size_t output = sizeof(prefix) - 1;
    for (size_t i = 0; i < at; ++i) {
        out[output++] = digits[tuple[i] >> 4];
        out[output++] = digits[tuple[i] & 15];
    }
    out[output] = 0;
    return true;
}

#ifdef _WIN32
static wchar_t *wide_key(const char *key)
{
    size_t count = strlen(key);
    if (count > CRED_MAX_GENERIC_TARGET_NAME_LENGTH) return NULL;
    wchar_t *wide = SDL_malloc((count + 1) * sizeof(*wide));
    if (!wide) return NULL;
    for (size_t i = 0; i <= count; ++i) wide[i] = (unsigned char)key[i];
    return wide;
}
static SvVaultResult provider(SvVaultRequest *request)
{
    wchar_t *key = wide_key(request->key);
    if (!key) return SV_VAULT_INVALID;
    SvVaultResult result = SV_VAULT_UNAVAILABLE;
    if (request->store) {
        CREDENTIALW credential = {0};
        credential.Type = CRED_TYPE_GENERIC;
        credential.TargetName = key;
        credential.CredentialBlobSize = (DWORD)request->secret_size;
        credential.CredentialBlob = request->secret;
        credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
        credential.UserName = L"TomeNET SV";
        if (CredWriteW(&credential, 0)) result = SV_VAULT_SAVED;
    } else {
        PCREDENTIALW found = NULL;
        if (CredReadW(key, CRED_TYPE_GENERIC, 0, &found)) {
            if (found->CredentialBlobSize && found->CredentialBlobSize <= sizeof(request->secret)) {
                request->secret_size = found->CredentialBlobSize;
                memcpy(request->secret, found->CredentialBlob, request->secret_size);
                result = SV_VAULT_FOUND;
            } else result = SV_VAULT_INVALID;
            CredFree(found);
        } else if (GetLastError() == ERROR_NOT_FOUND) result = SV_VAULT_MISSING;
    }
    SDL_free(key);
    return result;
}
#else
static const SecretSchema schema = {
    .name = "tomenet-sv/v1", .flags = SECRET_SCHEMA_NONE,
    .attributes = {{"identity", SECRET_SCHEMA_ATTRIBUTE_STRING}}
};
static SvVaultResult provider(SvVaultRequest *request)
{
    GError *error = NULL;
    SvVaultResult result = SV_VAULT_UNAVAILABLE;
    if (request->store) {
        SecretValue *value = secret_value_new((const char *)request->secret,
                                              request->secret_size, "application/octet-stream");
        if (value) {
            if (secret_password_store_binary_sync(&schema, SECRET_COLLECTION_DEFAULT,
                    "TomeNET SV account", value, NULL, &error,
                    "identity", request->key, NULL)) result = SV_VAULT_SAVED;
            secret_value_unref(value);
        }
    } else {
        SecretValue *value = secret_password_lookup_binary_sync(&schema, NULL, &error,
                                            "identity", request->key, NULL);
        if (value) {
            gsize length = 0;
            const char *data = secret_value_get(value, &length);
            if (length && length <= sizeof(request->secret)) {
                request->secret_size = length;
                memcpy(request->secret, data, length);
                result = SV_VAULT_FOUND;
            } else result = SV_VAULT_INVALID;
            secret_value_unref(value);
        } else if (!error) result = SV_VAULT_MISSING;
    }
    if (error) g_error_free(error);
    return result;
}
#endif

static int worker(void *opaque)
{
    SvVaultRequest *request = opaque;
    SvVaultResult result = provider(request);
    SDL_LockMutex(request->mutex);
    request->result = result;
    request->done = true;
    bool abandoned = request->abandoned;
    SDL_UnlockMutex(request->mutex);
    if (abandoned) destroy(request);
    return 0;
}

static SvVaultRequest *start(const char *key, uint64_t generation,
                             bool store, const void *secret, size_t size)
{
    if (!key || strlen(key) >= SV_VAULT_KEY_CAPACITY ||
        strncmp(key, "tomenet-sv/v1/", 14) ||
        (store && (!secret || !size || size > SV_VAULT_SECRET_CAPACITY)))
        return NULL;
    SvVaultRequest *request = SDL_calloc(1, sizeof(*request));
    if (!request) return NULL;
    request->mutex = SDL_CreateMutex();
    if (!request->mutex) { destroy(request); return NULL; }
    memcpy(request->key, key, strlen(key) + 1);
    request->generation = generation;
    request->store = store;
    if (store) { memcpy(request->secret, secret, size); request->secret_size = size; }
    request->thread = SDL_CreateThread(worker, "sv-credential", request);
    if (!request->thread) { destroy(request); return NULL; }
    SDL_DetachThread(request->thread);
    return request;
}

SvVaultRequest *sv_vault_lookup(const char *key, uint64_t generation)
{ return start(key, generation, false, NULL, 0); }
SvVaultRequest *sv_vault_store(const char *key, uint64_t generation,
                               const void *secret, size_t size)
{ return start(key, generation, true, secret, size); }

SvVaultResult sv_vault_poll(SvVaultRequest **slot, uint64_t generation,
                            void *secret, size_t capacity, size_t *size)
{
    if (size) *size = 0;
    if (!slot || !*slot) return SV_VAULT_INVALID;
    SvVaultRequest *request = *slot;
    SDL_LockMutex(request->mutex);
    bool done = request->done;
    SvVaultResult result = request->result;
    if (generation != request->generation) result = SV_VAULT_INVALID;
    if (done && result == SV_VAULT_FOUND) {
        if (!secret || capacity < request->secret_size) result = SV_VAULT_INVALID;
        else {
            memcpy(secret, request->secret, request->secret_size);
            if (size) *size = request->secret_size;
        }
    }
    if (!done) request->abandoned = generation != request->generation;
    SDL_UnlockMutex(request->mutex);
    if (!done && generation == request->generation) return SV_VAULT_PENDING;
    *slot = NULL;
    if (done) destroy(request);
    return result;
}

void sv_vault_cancel(SvVaultRequest **slot)
{
    if (!slot || !*slot) return;
    SvVaultRequest *request = *slot;
    *slot = NULL;
    SDL_LockMutex(request->mutex);
    bool done = request->done;
    if (!done) request->abandoned = true;
    SDL_UnlockMutex(request->mutex);
    if (done) destroy(request);
}

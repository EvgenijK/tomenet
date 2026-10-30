#include "credential/vault.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsecret/secret.h>

/* External provider mutations deliberately bypass SV; every observation below
 * still goes through the production asynchronous vault. Only the wrapper's
 * isolated Secret Service may execute the provider branch. */
static const SecretSchema schema = {
    .name = "tomenet-sv/v1", .flags = SECRET_SCHEMA_NONE,
    .attributes = {{"identity", SECRET_SCHEMA_ATTRIBUTE_STRING}}
};

static SvVaultResult finish(SvVaultRequest **request, uint64_t generation,
                            void *secret, size_t capacity, size_t *size)
{
    for (int i = 0; i < 500; ++i) {
        SvVaultResult result = sv_vault_poll(request, generation, secret, capacity, size);
        if (result != SV_VAULT_PENDING) return result;
        SDL_Delay(10);
    }
    sv_vault_cancel(request);
    return SV_VAULT_PENDING;
}

int main(int argc, char **argv)
{
    assert(SDL_Init(0));
    char first[SV_VAULT_KEY_CAPACITY], other[SV_VAULT_KEY_CAPACITY];
    assert(sv_vault_key(first, "a", 1, 18348, "BC", 2));
    assert(sv_vault_key(other, "aB", 2, 18348, "C", 1));
    assert(strcmp(first, other));
    assert(sv_vault_key(other, "a", 1, 18348, "bc", 2));
    assert(strcmp(first, other));
    assert(sv_vault_key(other, "A", 1, 18348, "BC", 2));
    assert(strcmp(first, other));
    assert(sv_vault_key(other, "a", 1, 18349, "BC", 2));
    assert(strcmp(first, other));
    assert(!sv_vault_key(other, "a", 1, 0, "BC", 2));
    char long_server[300] = {0};
    memset(long_server, 'x', sizeof(long_server));
    assert(!sv_vault_key(other, long_server, sizeof(long_server), 18348, "BC", 2));
    assert(!strcmp(first, "tomenet-sv/v1/00016147ac00024243"));
    if (argc == 2 && !strcmp(argv[1], "unavailable")) {
        SvVaultRequest *request = sv_vault_lookup(first, 42);
        assert(request);
        unsigned char secret[SV_VAULT_SECRET_CAPACITY];
        size_t size = 123;
        assert(finish(&request, 42, secret, sizeof(secret), &size) == SV_VAULT_UNAVAILABLE);
        assert(!request && size == 0);
        request = sv_vault_lookup(first, 43);
        assert(request);
        assert(sv_vault_poll(&request, 44, secret, sizeof(secret), &size) == SV_VAULT_INVALID);
        assert(!request && size == 0);
        SDL_Delay(100);
    }
    if (argc == 2 && !strcmp(argv[1], "provider")) {
        const char *isolated = getenv("SV_VAULT_ISOLATED_PROVIDER");
        assert(isolated && !strcmp(isolated, "1"));
        static const unsigned char original[] = "SV_B005_SECRET_NEVER_PRINT\0\xff";
        SvVaultRequest *request = sv_vault_store(first, 50, original, sizeof(original) - 1);
        assert(request);
        assert(finish(&request, 50, NULL, 0, NULL) == SV_VAULT_SAVED);
        request = sv_vault_lookup(first, 51);
        assert(request);
        unsigned char restored[SV_VAULT_SECRET_CAPACITY] = {0};
        size_t size = 0;
        assert(finish(&request, 51, restored, sizeof(restored), &size) == SV_VAULT_FOUND);
        assert(size == sizeof(original) - 1 && !memcmp(original, restored, size));
        assert(sv_vault_key(other, "a", 1, 18348, "bc", 2));
        request = sv_vault_lookup(other, 52);
        assert(request);
        assert(finish(&request, 52, restored, sizeof(restored), &size) == SV_VAULT_MISSING);

        GError *error = NULL;
        static const unsigned char replacement[] = {'n', 0, 0xfe};
        SecretValue *value = secret_value_new((const char *)replacement,
                                              sizeof(replacement), "application/octet-stream");
        assert(value);
        assert(secret_password_store_binary_sync(&schema, SECRET_COLLECTION_DEFAULT,
                    "SV isolated replacement", value, NULL, &error,
                    "identity", first, NULL));
        assert(!error);
        secret_value_unref(value);
        request = sv_vault_lookup(first, 53);
        assert(request);
        assert(finish(&request, 53, restored, sizeof(restored), &size) == SV_VAULT_FOUND);
        assert(size == sizeof(replacement) && !memcmp(restored, replacement, size));

        assert(secret_password_clear_sync(&schema, NULL, &error, "identity", first, NULL));
        assert(!error);
        request = sv_vault_lookup(first, 54);
        assert(request);
        assert(finish(&request, 54, restored, sizeof(restored), &size) == SV_VAULT_MISSING);
        assert(size == 0);

        request = sv_vault_store(first, 55, original, sizeof(original) - 1);
        assert(request);
        assert(finish(&request, 55, NULL, 0, NULL) == SV_VAULT_SAVED);
        SecretService *service = secret_service_get_sync(SECRET_SERVICE_OPEN_SESSION, NULL, &error);
        assert(service && !error);
        SecretCollection *collection = secret_collection_for_alias_sync(
                service, SECRET_COLLECTION_DEFAULT, SECRET_COLLECTION_NONE, NULL, &error);
        assert(collection && !error);
        GList *objects = g_list_append(NULL, collection), *locked = NULL;
        assert(secret_service_lock_sync(service, objects, NULL, &locked, &error) == 1);
        assert(!error && locked);
        g_list_free_full(locked, g_object_unref);
        g_list_free(objects);
        memset(restored, 0, sizeof(restored));
        request = sv_vault_lookup(first, 56);
        assert(request);
        SvVaultResult locked_result = finish(&request, 56, restored, sizeof(restored), &size);
        assert(locked_result == SV_VAULT_MISSING || locked_result == SV_VAULT_UNAVAILABLE ||
               locked_result == SV_VAULT_PENDING);
        assert(!request && size == 0 && restored[0] == 0);
        request = sv_vault_store(first, 57, replacement, sizeof(replacement));
        assert(request);
        SvVaultResult locked_store = finish(&request, 57, NULL, 0, NULL);
        assert(locked_store == SV_VAULT_UNAVAILABLE || locked_store == SV_VAULT_PENDING);
        assert(!request);
        printf("SV actual provider replacement/deletion passed; locked lookup=%d store=%d\n",
               (int)locked_result, (int)locked_store);
        g_object_unref(collection);
        g_object_unref(service);
    }
    SDL_Quit();
    puts("SV vault production identity/provider checks passed");
    return 0;
}

#include "credential/vault.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

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
    }
    SDL_Quit();
    puts("SV vault production identity/provider checks passed");
    return 0;
}

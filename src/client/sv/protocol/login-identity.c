#ifndef _WIN32
#define _XOPEN_SOURCE 700
#endif
#include "protocol/login-identity.h"
#include "../../../common/md5.h"
#include <SDL3/SDL.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void update_decimal(MD5_CTX *ctx, int value, bool separator)
{
    char text[64];
    int count = snprintf(text, sizeof(text), "%d", value);
    if (count > 0 && count < (int)sizeof(text))
        MD5Update(ctx, (const unsigned char *)text, (unsigned)count);
    if (separator) MD5Update(ctx, (const unsigned char *)"\0", 1);
}

bool sv_login_identity(const char *profile_root, unsigned char iaddr[6])
{
    if (!profile_root || !iaddr) return false;
#ifdef _WIN32
    char *canonical = _fullpath(NULL, profile_root, PATH_MAX);
#else
    char *canonical = realpath(profile_root, NULL);
#endif
    if (!canonical) return false;
    const char *base = SDL_GetBasePath();
    if (!base || !*base) { free(canonical); return false; }
    char separator = base[strlen(base) - 1];
    size_t length = strlen(canonical);
    if (length > UINT_MAX - 2) { free(canonical); return false; }
    char *path = malloc(length + 2);
    if (!path) { free(canonical); return false; }
    memcpy(path, canonical, length);
    if (!length || canonical[length - 1] != separator) path[length++] = separator;
    path[length] = 0;
    free(canonical);

    MD5_CTX ctx;
    unsigned char digest[16];
    MD5Init(&ctx);
    MD5Update(&ctx, (const unsigned char *)"TomenetSDL3Fingerprint", 22);
    MD5Update(&ctx, (const unsigned char *)"\0", 1);
    MD5Update(&ctx, (const unsigned char *)path, (unsigned)length);
    MD5Update(&ctx, (const unsigned char *)"\0", 1);
    free(path);
    update_decimal(&ctx, SDL_GetNumLogicalCPUCores(), true);
    update_decimal(&ctx, SDL_GetSystemRAM(), true);
    update_decimal(&ctx, SDL_GetCPUCacheLineSize(), true);
    update_decimal(&ctx, SDL_BYTEORDER, false);
    MD5Final(digest, &ctx);
    iaddr[0] = 0xf4;
    iaddr[1] = 0x43;
    memcpy(iaddr + 2, digest, 4);
    volatile unsigned char *secret = digest;
    for (size_t i = 0; i < sizeof(digest); ++i) secret[i] = 0;
    return true;
}

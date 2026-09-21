/* Temporary bootstrap for the synthetic-only Stage A executable.
 * Replace this bootstrap with real session startup when live transport arrives. */
#include "synthetic.h"
#include <stdio.h>
#include <string.h>

static const char marker[] = "TomeNET SV isolated synthetic profile v1\n";

static bool path(char *out, size_t size, const char *base, const char *suffix)
{
    if (SDL_snprintf(out, size, "%s/%s", base, suffix) >= (int)size)
        return SDL_SetError("SV path too long");
    return true;
}

bool sv_synthetic_profile(const char *root)
{
    SDL_PathInfo info;
    char name[4096];
    if (!(root[0] == '/' || (SDL_strlen(root) >= 3 && root[1] == ':' &&
          (root[2] == '/' || root[2] == '\\'))))
        return SDL_SetError("Synthetic profile root must be absolute");
    if (!path(name, sizeof(name), root, ".sv-synthetic-profile")) return false;
    if (SDL_GetPathInfo(root, &info)) {
        size_t size = 0;
        char *data = SDL_LoadFile(name, &size);
        bool valid = data && size == sizeof(marker) - 1 && !memcmp(data, marker, size);
        SDL_free(data);
        if (!valid) return SDL_SetError("Refusing existing unmarked profile root; choose a new directory");
    } else {
        if (!SDL_CreateDirectory(root) || !SDL_SaveFile(name, marker, sizeof(marker) - 1)) return false;
    }
    /* Same SDL3 U identity/override; SV settings belong to U/sv, never U CFG. */
    if (SDL_setenv_unsafe("TOMENET_SDL3_USER_PATH", root, 1)) return false;
    return path(name, sizeof(name), root, "sv") && SDL_CreateDirectory(name);
}

static int synthetic_alert(void *context, SvAlert alert)
{
    (void)context;
    printf("SV synthetic alert=%d (no audio device)\n", (int)alert);
    return 1;
}
SvAlertSink sv_synthetic_alert_sink(void)
{
    return (SvAlertSink){NULL, synthetic_alert};
}
bool sv_synthetic_start(SvApp *app)
{
    const int version[6] = {4, 7, 0, 2, 0, 2};
    const unsigned char hp[] = {24, 0x27, 0x74, 0x27, 0x42, 1,
        46,255,'w','E','c','h','o',0,46,255,'w','E','c','h','o',0};
    if (sv_app_open(app, version) != SV_OK ||
        sv_app_set_alerts(app, (SvAlertOptions){1,1}, (SvAttention){0}) != SV_OK ||
        sv_app_receive(app, sv_app_view(app).generation, hp, sizeof(hp)) != SV_OK)
        return false;
    puts("SV HP peer queued server_version=4.7.0.2.0.2 maximum=100 current=50 boosted=1 bar=1 drain=1");
    return true;
}

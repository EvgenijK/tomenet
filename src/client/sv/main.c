/* Stage A ticket 01: runnable native shell, explicitly not a game session. */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char marker[] = "TomeNET SV isolated synthetic profile v1\n";

static bool path(char *out, size_t size, const char *base, const char *suffix)
{
    if (SDL_snprintf(out, size, "%s/%s", base, suffix) >= (int)size)
        return SDL_SetError("SV path too long");
    return true;
}

static bool isolated_profile(const char *root)
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

static bool rectangle(SDL_Renderer *renderer, float scale, float x, float y, float w, float h)
{
    SDL_FRect rect = {SDL_roundf(x * scale), SDL_roundf(y * scale),
                     SDL_roundf(w * scale), SDL_roundf(h * scale)};
    return SDL_SetRenderDrawColor(renderer, 29, 37, 46, 255) && SDL_RenderFillRect(renderer, &rect);
}

static bool frame(SDL_Window *window, SDL_Renderer *renderer, SvFont *font)
{
    int w, h;
    float scale = SDL_GetWindowDisplayScale(window);
    if (scale <= 0 || !SDL_GetRenderOutputSize(renderer, &w, &h)) return false;
    float logical_w = w / scale;
    const SDL_Color title = {225, 232, 237, 255}, text = {180, 196, 208, 255};
    if (!SDL_SetRenderDrawColor(renderer, 15, 21, 28, 255) || !SDL_RenderClear(renderer)) return false;
    if (!rectangle(renderer, scale, 24, 90, logical_w - 48, 144) ||
        !rectangle(renderer, scale, 24, 254, logical_w - 48, 144)) return false;
    const char *lines[] = {
        "TomeNET SV - synthetic shell",
        "Native surfaces",
        "Status, messages and requests: later tickets.",
        "No server connection or gameplay is active.",
        "Isolated profile. Fullscreen default. UI scale 100%.",
        "One SDL window. Close or press Escape to exit."
    };
    const int ys[] = {30, 112, 155, 190, 278, 330};
    for (unsigned i = 0; i < SDL_arraysize(lines); ++i)
        if (!sv_font_draw(font, renderer, lines[i], (int)SDL_roundf(40 * scale),
                          (int)SDL_roundf(ys[i] * scale), scale, i == 0 ? title : text)) return false;
    return SDL_RenderPresent(renderer);
}

static void usage(void)
{
    puts("TomeNET SV Stage A shell (no live session)\n"
         "  --synthetic --profile-root ABSOLUTE-NEW-DIRECTORY\n"
         "  [--library PATH] [--fixture-window WIDTHxHEIGHT] [--frames N]\n"
         "Product defaults: desktop fullscreen, UI scale 100%, Cascadia Mono.\n"
         "TOMENET_PATH selects library resources, otherwise adjacent lib/.\n"
         "TOMENET_SDL3_USER_PATH may select an already marked synthetic root.\n"
         "SDL_RENDER_DRIVER=software explicitly forces the software renderer.");
}

int main(int argc, char **argv)
{
    const char *root = NULL, *library = NULL;
    char adjacent[4096];
    bool synthetic = false, windowed = false, quit = false;
    int width = 1024, height = 768, frames = 0, submitted = 0, result = 1;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SvFont *font = NULL;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help")) { usage(); return 0; }
        if (!strcmp(argv[i], "--synthetic")) { synthetic = true; continue; }
        if (i + 1 >= argc) { usage(); return 2; }
        if (!strcmp(argv[i], "--profile-root")) root = argv[++i];
        else if (!strcmp(argv[i], "--library")) library = argv[++i];
        else if (!strcmp(argv[i], "--fixture-window")) {
            char extra;
            if (sscanf(argv[++i], "%dx%d%c", &width, &height, &extra) != 2 ||
                width < 1024 || height < 768 || width > 8192 || height > 8192) return 2;
            windowed = true;
        } else if (!strcmp(argv[i], "--frames")) {
            char *end;
            long n = strtol(argv[++i], &end, 10);
            if (*end || n < 1 || n > 10000) return 2;
            frames = (int)n;
        } else { usage(); return 2; }
    }
    if (!synthetic) { fprintf(stderr, "SV requires explicit --synthetic; live client is not implemented\n"); return 2; }
    if (!root) root = SDL_getenv("TOMENET_SDL3_USER_PATH");
    if (!root || !*root) { fprintf(stderr, "SV requires an isolated --profile-root; personal SDL3 profile is never opened by this scenario\n"); return 2; }
    if (!isolated_profile(root)) goto done;
    if (!library) library = SDL_getenv("TOMENET_PATH");
    if (!library || !*library) {
        const char *base = SDL_GetBasePath();
        if (!base || !path(adjacent, sizeof(adjacent), base, "lib")) goto done;
        library = adjacent;
    }
    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) goto done;
    font = sv_font_open(root, library);
    if (!font) goto done;
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (!windowed) flags |= SDL_WINDOW_FULLSCREEN;
    window = SDL_CreateWindow("TomeNET SV - SYNTHETIC", width, height, flags);
    if (!window) goto done;
    float window_scale = SDL_GetWindowDisplayScale(window) / SDL_GetWindowPixelDensity(window);
    if (window_scale <= 0 || !SDL_SetWindowMinimumSize(window,
            (int)SDL_ceilf(1024 * window_scale), (int)SDL_ceilf(768 * window_scale))) goto done;
    if (windowed && !SDL_SetWindowSize(window, (int)SDL_roundf(width * window_scale),
                                      (int)SDL_roundf(height * window_scale))) goto done;
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) goto done;
    int count = 0;
    SDL_Window **windows = SDL_GetWindows(&count);
    SDL_free(windows);
    if (count != 1) { SDL_SetError("SV requires exactly one system window"); goto done; }
    int pw, ph;
    if (!SDL_GetRenderOutputSize(renderer, &pw, &ph)) goto done;
    printf("SV startup build=%s synthetic=true windows=%d video=%s renderer=%s fullscreen=%s ui_scale=100 output=%dx%d display_scale=%.3f\n",
           SV_BUILD_ID, count, SDL_GetCurrentVideoDriver(), SDL_GetRendererName(renderer),
           SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN ? "true" : "false", pw, ph, SDL_GetWindowDisplayScale(window));
    printf("SV profile identity=TomenetGame/tomenet override=%s settings=%s/sv/tomenet.cfg writes=none\n", root, root);
    printf("SV text requested=CascadiaMono-Regular.ttf effective=%s profile=sv-shell-ascii-v1 fallback_routes=0\n", sv_font_resource(font));
    fflush(stdout);
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) quit = true;
        }
        if (quit) break;
        if (!frame(window, renderer, font)) goto done;
        ++submitted;
        if (frames && submitted >= frames) break;
        SDL_Delay(16);
    }
    if (frames && submitted < frames) { SDL_SetError("Smoke closed before requested frame count"); goto done; }
    printf("SV exit submitted_frames=%d fallback_routes=0\n", submitted);
    result = 0;
done:
    if (result) fprintf(stderr, "SV startup/render failed: %s; no terminal fallback\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    sv_font_close(font);
    TTF_Quit();
    SDL_Quit();
    return result;
}

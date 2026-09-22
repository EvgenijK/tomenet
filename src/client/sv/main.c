/* Stage A ticket 01: runnable native shell, explicitly not a game session. */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "ui/font.h"
#include "session/session.h"
#include "ui/status.h"
#include "ui/ui.h"
#include "hp-scenario.h"
#include "message-scenario.h"
#include "request-scenario.h"
#include "lifecycle-scenario.h"
#include "input/native-input.h"
#include "geometry-scenario.h"
#include "timing-scenario.h"
#include "arch-scenario.h"
#include "native-frame.h"
#include "synthetic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(void)
{
    puts("TomeNET SV Stage A shell (no live session)\n"
         "  --synthetic --profile-root ABSOLUTE-NEW-DIRECTORY\n"
         "  [--library PATH] [--fixture-window WIDTHxHEIGHT] [--frames N] [--hp-check] [--arch-check] [--message-check] [--request-check] [--lifecycle-check] [--geometry-check] [--timing-check] [--timing-delay]\n"
         "Product defaults: desktop fullscreen, UI scale 100%, Cascadia Mono.\n"
         "TOMENET_PATH selects library resources, otherwise adjacent lib/.\n"
         "TOMENET_SDL3_USER_PATH may select an already marked synthetic root.\n"
         "SDL_RENDER_DRIVER=software explicitly forces the software renderer.");
}

int main(int argc, char **argv)
{
    const char *root = NULL, *library = NULL;
    char adjacent[4096];
    bool hp_check = false, arch_check = false, message_check = false, request_check = false;
    bool lifecycle_check = false, geometry_check = false, timing_check = false, timing_delay = false;
    SvApp *app = NULL;
    bool synthetic = false, windowed = false, quit = false;
    int width = 1024, height = 768, frames = 0, submitted = 0, result = 1;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SvFont *font = NULL;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help")) { usage(); return 0; }
        if (!strcmp(argv[i], "--synthetic")) { synthetic = true; continue; }
        if (!strcmp(argv[i], "--arch-check")) { arch_check = true; continue; }
        if (!strcmp(argv[i], "--request-check")) { request_check = true; continue; }
        if (!strcmp(argv[i], "--lifecycle-check")) { lifecycle_check = true; continue; }
        if (!strcmp(argv[i], "--message-check")) { message_check = true; continue; }
        if (!strcmp(argv[i], "--timing-check")) { timing_check = true; continue; }
        if (!strcmp(argv[i], "--timing-delay")) { timing_delay = true; continue; }
        if (!strcmp(argv[i], "--geometry-check")) { geometry_check = true; continue; }
        if (!strcmp(argv[i], "--hp-check")) { hp_check = true; continue; }
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
    if (timing_delay && !timing_check) return 2;
    if (!synthetic) { fprintf(stderr, "SV requires explicit --synthetic; live client is not implemented\n"); return 2; }
    if (!root) root = SDL_getenv("TOMENET_SDL3_USER_PATH");
    if (!root || !*root) { fprintf(stderr, "SV requires an isolated --profile-root; personal SDL3 profile is never opened by this scenario\n"); return 2; }
    if (!sv_synthetic_profile(root)) goto done;
    if (!library) library = SDL_getenv("TOMENET_PATH");
    if (!library || !*library) {
        const char *base = SDL_GetBasePath();
        if (!base) goto done;
        int length = SDL_snprintf(adjacent, sizeof(adjacent), "%s/lib", base);
        if (length < 0 || (size_t)length >= sizeof(adjacent)) {
            SDL_SetError("SV installation library path too long");
            goto done;
        }
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
    app = sv_app_create(sv_synthetic_alert_sink());
    if (!app) goto done;
    SvUi ui = {.window = window, .renderer = renderer, .font = font, .font_revision = 1};
    if (timing_check && !sv_timing_scenario(app, &ui, timing_delay)) goto done;
    if (geometry_check && !sv_geometry_scenario(app, &ui)) goto done;
    if (lifecycle_check && !sv_lifecycle_scenario(app, &ui)) goto done;
    if (request_check && !sv_request_scenario(app, &ui)) goto done;
    if (message_check && !sv_message_scenario(app, &ui)) goto done;
    if (hp_check) {
        if (!sv_hp_scenario(app, sv_scenario_frame, &ui)) goto done;
    }
    if (arch_check) {
        if (!sv_arch_scenario(app, sv_scenario_frame, &ui)) goto done;
    } else if (!sv_synthetic_start(app)) goto done;
    if (!SDL_StartTextInput(window)) goto done;
    SvNativeInput input;
    sv_native_input_begin(&input, app);
    fflush(stdout);
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (sv_ui_event(&ui, &event)) continue;
            if (sv_native_input(&input, app, &event)) continue;
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) quit = true;
        }
        if (quit) break;
        /* A whole-input budget yields to UI even with buffered network backlog. */
        SvStep step = sv_app_step(app, 16);
        (void)sv_app_dispatch_input(app, 16);
        if (step.result != SV_OK && step.result != SV_WAITING && step.result != SV_CLOSED)
            fprintf(stderr, "SV session: %s\n", sv_result_text(step.result));
        /* Explicit Stage A consumer: model/feed publication is the only
         * implemented message effect. Acknowledgement is never a draw action. */
        SvMessage delivered;
        while (sv_app_take_message(app, sv_app_view(app).generation, &delivered) == SV_OK) {}
        if (!sv_ui_submit(&ui, sv_app_view(app))) goto done;
        ++submitted;
        if (frames && submitted >= frames) break;
        SDL_Delay(16);
    }
    if (frames && submitted < frames) { SDL_SetError("Smoke closed before requested frame count"); goto done; }
    printf("SV exit submitted_frames=%d session_active=%d fallback_routes=0\n", submitted, sv_app_view(app).active);
    result = 0;
done:
    if (result) fprintf(stderr, "SV startup/render failed: %s; no terminal fallback\n", SDL_GetError());
    sv_app_destroy(app);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    sv_font_close(font);
    TTF_Quit();
    SDL_Quit();
    return result;
}

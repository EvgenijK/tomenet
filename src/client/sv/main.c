/* Stage A ticket 01: runnable native shell, explicitly not a game session. */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <ft2build.h>
#include FT_FREETYPE_H
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
#include "endpoint-run.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { SvUi *ui; SvRuntimeScenario scenario; } ScenarioContext;
static int run_scenario(SvApp *app, void *context)
{
    ScenarioContext *check = context;
    switch (check->scenario) {
    case SV_SCENARIO_HP: return sv_hp_scenario(app, sv_scenario_frame, check->ui);
    case SV_SCENARIO_MESSAGE: return sv_message_scenario(app, check->ui);
    case SV_SCENARIO_REQUEST: return sv_request_scenario(app, check->ui);
    case SV_SCENARIO_LIFECYCLE: return sv_lifecycle_scenario(app, check->ui);
    case SV_SCENARIO_GEOMETRY: return sv_geometry_scenario(app, check->ui);
    case SV_SCENARIO_TIMING: return sv_timing_scenario(app, check->ui, 0);
    case SV_SCENARIO_TIMING_DELAYED: return sv_timing_scenario(app, check->ui, 1);
    case SV_SCENARIO_ARCH: return sv_arch_scenario(app, sv_scenario_frame, check->ui);
    default: return 0;
    }
}
static int checked_scenario(SvApp *app, SvUi *ui, SvRuntimeScenario scenario)
{
    ScenarioContext context = {ui, scenario};
    SvRuntimeCheck check = sv_app_check_run(app, scenario, run_scenario, &context);
    const char *path = SDL_getenv("SV_RUNTIME_REPORT");
    if (path && *path) {
        FILE *stream = fopen(path, "a");
        if (!stream) return SDL_SetError("Cannot open runtime report");
        int written = sv_runtime_write(stream, check);
        if (fclose(stream) || !written) return SDL_SetError("Cannot write runtime report");
    }
    /* Human-readable only: the producer consumes the dedicated JSONL artifact. */
    printf("SV measured scenario=%d completed=%d fallback_routes=%u\n",
        (int)scenario, check.completed, (unsigned)check.fallback_entries);
    return check.completed && !check.fallback_entries;
}

static void usage(void)
{
    puts("TomeNET SV native shell\n"
         "  --endpoint [--server HOST[:PORT]] [--port PORT] [--metaserver HOST[:PORT]] [--server-list PATH] [--library PATH]\n"
         "  [--account NAME --password-stdin] (contact through standard input)\n"
         "  --synthetic --profile-root ABSOLUTE-NEW-DIRECTORY\n"
         "  [--library PATH] [--fixture-window WIDTHxHEIGHT] [--frames N] [--review] [--hp-check] [--arch-check] [--message-check] [--request-check] [--lifecycle-check] [--geometry-check] [--timing-check] [--timing-delay]\n"
         "Manual --review: F5 restarts synthetic session, F6 rebuilds surfaces, m expands to Y.\n"
         "Product defaults: desktop fullscreen, UI scale 100%, Cascadia Mono.\n"
         "TOMENET_PATH selects library resources, otherwise adjacent lib/.\n"
         "TOMENET_SDL3_USER_PATH may select an already marked synthetic root.\n"
         "SDL_RENDER_DRIVER=software explicitly forces the software renderer.");
}

int main(int argc, char **argv)
{
    const char *root = NULL, *library = NULL;
    const char *server = NULL, *server_list = NULL, *metaserver = NULL;
    const char *account = NULL, *real_name = NULL;
    bool password_stdin = false;
    char contact_password[80] = {0};
    char adjacent[4096];
    bool hp_check = false, arch_check = false, message_check = false, request_check = false;
    bool lifecycle_check = false, geometry_check = false, timing_check = false, timing_delay = false;
    SvApp *app = NULL;
    bool synthetic = false, endpoint_mode = false, windowed = false, quit = false;
    bool review = false;
    int width = 1024, height = 768, frames = 0, submitted = 0, result = 1;
    unsigned port = 18348;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SvFont *font = NULL;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help")) { usage(); return 0; }
        if (!strcmp(argv[i], "--synthetic")) { synthetic = true; continue; }
        if (!strcmp(argv[i], "--endpoint")) { endpoint_mode = true; continue; }
        if (!strcmp(argv[i], "--password-stdin")) { password_stdin = true; continue; }
        if (!strcmp(argv[i], "--review")) { review = true; continue; }
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
        else if (!strcmp(argv[i], "--server")) server = argv[++i];
        else if (!strcmp(argv[i], "--server-list")) server_list = argv[++i];
        else if (!strcmp(argv[i], "--metaserver")) metaserver = argv[++i];
        else if (!strcmp(argv[i], "--account")) account = argv[++i];
        else if (!strcmp(argv[i], "--real-name")) real_name = argv[++i];
        else if (!strcmp(argv[i], "--port")) {
            char *end;
            unsigned long value = strtoul(argv[++i],&end,10);
            if (*end || value < 1 || value > 65535) return 2;
            port = (unsigned)value;
        }
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
    if (endpoint_mode) {
        if (synthetic || review || hp_check || arch_check || message_check || request_check ||
            lifecycle_check || geometry_check || timing_check || timing_delay) return 2;
        if (!!account != password_stdin) return 2;
        if (password_stdin) {
            if (!fgets(contact_password, sizeof(contact_password), stdin)) return 2;
            size_t count = strlen(contact_password);
            while (count && (contact_password[count - 1] == '\n' || contact_password[count - 1] == '\r'))
                contact_password[--count] = 0;
            if (!count || count >= 16) return 2;
        }
        int contact_result = sv_endpoint_run((SvEndpointOptions){.root = root, .library = library,
            .server = server, .server_list = server_list, .width = width, .height = height,
            .frames = frames, .windowed = windowed, .port = port, .metaserver = metaserver,
            .account = account, .password = password_stdin ? contact_password : NULL,
            .real_name = real_name});
        volatile char *secret = contact_password;
        for (size_t i = 0; i < sizeof(contact_password); ++i) secret[i] = 0;
        return contact_result;
    }
    if (server || server_list || metaserver || port != 18348 || account || password_stdin || real_name) return 2;
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
    float review_scale = 0;
    int pw, ph;
    if (!SDL_GetRenderOutputSize(renderer, &pw, &ph)) goto done;
    printf("SV startup build=%s synthetic=true windows=%d video=%s renderer=%s fullscreen=%s ui_scale=100 output=%dx%d display_scale=%.3f\n",
           SV_BUILD_ID, count, SDL_GetCurrentVideoDriver(), SDL_GetRendererName(renderer),
           SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN ? "true" : "false", pw, ph, SDL_GetWindowDisplayScale(window));
    FT_Library version_library;
    int ft_major, ft_minor, ft_patch;
    if (FT_Init_FreeType(&version_library)) {
        SDL_SetError("Cannot query direct FreeType runtime");
        goto done;
    }
    FT_Library_Version(version_library, &ft_major, &ft_minor, &ft_patch);
    FT_Done_FreeType(version_library);
    printf("SV runtime architecture=%s pointer_bits=%u sdl=%d sdl_ttf=%d freetype=%d.%d.%d\n",
           sizeof(void *) == 4 ? "i686" : "amd64", (unsigned)(sizeof(void *) * 8),
           SDL_GetVersion(), TTF_Version(), ft_major, ft_minor, ft_patch);
    printf("SV profile identity=TomenetGame/tomenet override=%s settings=%s/sv/tomenet.cfg writes=none\n", root, root);
    printf("SV text requested=CascadiaMono-Regular.ttf effective=%s profile=sv-shell-ascii-v1\n", sv_font_resource(font));
    app = sv_app_create(sv_synthetic_alert_sink());
    if (!app) goto done;
    SvUi ui = {.window = window, .renderer = renderer, .font = font, .font_revision = 1};
    if (!sv_ui_start(&ui, sv_app_view(app))) goto done;
    if (timing_check && !checked_scenario(app, &ui, timing_delay ? SV_SCENARIO_TIMING_DELAYED : SV_SCENARIO_TIMING)) goto done;
    if (geometry_check && !checked_scenario(app, &ui, SV_SCENARIO_GEOMETRY)) goto done;
    if (lifecycle_check && !checked_scenario(app, &ui, SV_SCENARIO_LIFECYCLE)) goto done;
    if (request_check && !checked_scenario(app, &ui, SV_SCENARIO_REQUEST)) goto done;
    if (message_check && !checked_scenario(app, &ui, SV_SCENARIO_MESSAGE)) goto done;
    if (hp_check) {
        if (!checked_scenario(app, &ui, SV_SCENARIO_HP)) goto done;
    }
    if (arch_check) {
        if (!checked_scenario(app, &ui, SV_SCENARIO_ARCH)) goto done;
    } else if (!sv_synthetic_start(app)) goto done;
    if (review && sv_app_bind_macro(app, 'm', 'Y', SV_MACRO_NORMAL) != SV_OK) goto done;
    if (!SDL_StartTextInput(window)) goto done;
    SvNativeInput input;
    sv_native_input_begin(&input, app);
    fflush(stdout);
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            /* Manual fixture controls supply lifecycle inputs, never replies. */
            if (review && event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&
                    (event.key.key == SDLK_F5 || event.key.key == SDLK_F6)) {
                if (event.key.key == SDLK_F5) {
                    if (!sv_synthetic_start(app)) goto done;
                    sv_native_input_begin(&input, app);
                    puts("SV review session restarted");
                } else puts("SV review surfaces rebuilt");
                sv_ui_rebuild(&ui);
                continue;
            }
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
        if (review) {
            int w, h;
            if (!SDL_GetRenderOutputSize(renderer, &w, &h)) goto done;
            float scale = sv_ui_scale(&ui);
            if (!submitted || w != pw || h != ph || scale != review_scale) {
                printf("SV review build=%s video=%s renderer=%s output=%dx%d logical=%.3fx%.3f display_scale=%.3f\n",
                    SV_BUILD_ID, SDL_GetCurrentVideoDriver(), SDL_GetRendererName(renderer),
                    w, h, w / scale, h / scale, scale);
                pw = w; ph = h; review_scale = scale;
            }
            unsigned char bytes[32];
            SvOutput reply = sv_app_take_output(app, sv_app_view(app).generation, bytes, sizeof(bytes));
            if (reply.result == SV_OK) {
                printf("SV review serialized reply=");
                for (size_t i = 0; i < reply.size; ++i) printf("%02x", bytes[i]);
                puts("");
            }
            fflush(stdout);
        }
        ++submitted;
        if (frames && submitted >= frames) break;
        SDL_Delay(16);
    }
    if (frames && submitted < frames) { SDL_SetError("Smoke closed before requested frame count"); goto done; }
    printf("SV exit submitted_frames=%d session_active=%d\n", submitted, sv_app_view(app).active);
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

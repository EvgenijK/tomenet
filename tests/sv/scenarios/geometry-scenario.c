#include "geometry-scenario.h"
#include "native-frame.h"
#include "native-input.h"
#include <stdio.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Geometry scenario failed line %d: %s (%s)\n", __LINE__, #test, SDL_GetError()); \
    return 0; } } while (0)
int sv_geometry_scenario(SvApp *app, SvUi *ui)
{
    const int version[] = {4,7,0,2,0,2};
    const struct { int w, h; float scale; } cases[] = {
        {1024,768,1}, {1920,1080,1}, {1920,1080,2}, {1024,768,1.25f}, {1024,768,1.5f}
    };
    const unsigned char request[] = {184,0,0,0,7,'K','e','y','?',0};
    CHECK(sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    CHECK(sv_app_receive(app, gen, request, sizeof(request)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    SvAppView initial = sv_app_view(app);
    SvNativeInput input;
    sv_native_input_begin(&input, app);
    for (unsigned i = 0; i < SDL_arraysize(cases); ++i) {
        ui->fixture_scale = cases[i].scale;
        /* Virtual output target: no compositor DPI/monitor claim. The same
         * native layout/raster path draws at final size without framebuffer scaling. */
        SDL_Texture *target = SDL_CreateTexture(ui->renderer, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET, (int)(cases[i].w * cases[i].scale),
            (int)(cases[i].h * cases[i].scale));
        CHECK(target && SDL_SetRenderTarget(ui->renderer, target));
        sv_ui_rebuild(ui);
        const unsigned char updates[] = {24,0,100,0,(unsigned char)(40 + i),0,46,'E',0,46,'E',0};
        CHECK(sv_app_receive(app, gen, updates, sizeof(updates)) == SV_OK);
        CHECK(sv_app_step(app, 16).processed == 3);
        SvAppView view = sv_app_view(app);
        CHECK(view.request.pending && view.request.sequence == initial.request.sequence);
        CHECK(view.context == SV_CONTEXT_KEY_REQUEST && view.status.current == (int)(40 + i));
        CHECK(!memcmp(view.request.prompt, initial.request.prompt, SV_REQUEST_BYTES));
        SvMessage first, second;
        CHECK(sv_app_take_message(app, gen, &first) == SV_OK);
        CHECK(sv_app_take_message(app, gen, &second) == SV_OK);
        CHECK(first.sequence + 1 == second.sequence);
        int w, h;
        CHECK(SDL_GetCurrentRenderOutputSize(ui->renderer, &w, &h));
        CHECK(w == (int)(cases[i].w * cases[i].scale) && h == (int)(cases[i].h * cases[i].scale));
        CHECK(sv_scenario_draw(ui, view));
        CHECK(ui->logical_width == cases[i].w);
        printf("SV geometry logical=%dx%d output=%dx%d scale=%.2f dpi=virtual\n",
               cases[i].w, cases[i].h, w, h, cases[i].scale);
        CHECK(SDL_SetRenderTarget(ui->renderer, NULL));
        SDL_DestroyTexture(target);
    }
    SDL_Event key = {0};
    key.type = SDL_EVENT_TEXT_INPUT; key.text.text = "Y";
    key.text.timestamp = SDL_GetTicksNS(); key.text.windowID = SDL_GetWindowID(ui->window);
    CHECK(sv_native_input(&input, app, &key));
    CHECK(sv_app_dispatch_input(app, 16).dispatched == 1);
    unsigned char bytes[16];
    const unsigned char expected[] = {184,0,0,0,7,'Y'};
    SvOutput output = sv_app_take_output(app, gen, bytes, sizeof(bytes));
    CHECK(output.result == SV_OK && output.size == sizeof(expected) && !memcmp(bytes, expected, sizeof(expected)));
    CHECK(!sv_app_view(app).request.pending);
    ui->fixture_scale = 0;
    sv_ui_rebuild(ui);
    CHECK(sv_scenario_frame(ui, sv_app_view(app)));
    CHECK(sv_app_take_output(app, gen, bytes, sizeof(bytes)).result == SV_WAITING);
    CHECK(sv_app_close(app) == SV_OK);
    ui->fixture_scale = 0;
    sv_ui_rebuild(ui);
    puts("SV geometry checks passed cases=5 input=keyboard hit_bounds=not-applicable dpi=virtual");
    return 1;
}

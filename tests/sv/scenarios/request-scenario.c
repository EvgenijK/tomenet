#include "request-scenario.h"
#include "input/native-input.h"
#include "native-frame.h"
#include "hp-scenario.h"
#include <stdio.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Request scenario failed at line %d: %s\n", __LINE__, #test); \
    return 0; } } while (0)
int sv_request_scenario(SvApp *app, SvUi *ui)
{
    const unsigned char packet[] = {184,0x12,0x34,0x56,0x78,'C','h','o','o','s','e',' ','a',' ','k','e','y',':',0};
    unsigned cases = 0;
    for (int layout = 1; layout <= 2; ++layout) {
        int version[] = {4,7,0,2,0,layout};
        for (size_t split = 0; split <= sizeof(packet); ++split) {
            CHECK(sv_app_open(app, version) == SV_OK);
            SvNativeInput native;
            sv_native_input_begin(&native, app);
            uint64_t gen = sv_app_view(app).generation;
            CHECK(sv_app_receive(app, gen, packet, split) == SV_OK);
            CHECK(sv_app_step(app, 16).processed == (split == sizeof(packet) ? 1 : 0));
            if (split < sizeof(packet)) CHECK(!sv_app_view(app).request.pending);
            CHECK(sv_app_receive(app, gen, packet + split, sizeof(packet) - split) == SV_OK);
            sv_app_step(app, 16);
            SvAppView view = sv_app_view(app);
            CHECK(view.request.pending && view.request.id == 0x12345678);
            CHECK(!strcmp((char *)view.request.prompt, "Choose a key:"));
            CHECK(sv_scenario_frame(ui, view));
            const unsigned char hp[] = {24,0,100,0,37,0};
            CHECK(sv_app_receive(app, gen, hp, layout == 1 ? 5 : 6) == SV_OK);
            CHECK(sv_app_step(app, 16).processed == 1);
            view = sv_app_view(app);
            CHECK(view.status.current == 37 && view.context == SV_CONTEXT_KEY_REQUEST);
            SvUi rebuilt = {.window = ui->window, .renderer = ui->renderer,
                            .font = ui->font, .font_revision = ui->font_revision};
            CHECK(sv_scenario_frame(&rebuilt, view));
            unsigned char output[32];
            CHECK(sv_app_take_output(app, gen, output, sizeof(output)).result == SV_WAITING);
            unsigned char key = split % 3 == 0 ? 'Y' : 0;
            if (split % 3 == 2) {
                const unsigned char abort[] = {188,188};
                CHECK(sv_app_receive(app, gen, abort, sizeof(abort)) == SV_OK);
                CHECK(sv_app_step(app, 16).processed == 2);
            } else {
                SDL_Event input = {0};
                if (key) { input.type = SDL_EVENT_TEXT_INPUT; input.text.text = "Y"; }
                else { input.type = SDL_EVENT_KEY_DOWN; input.key.key = SDLK_ESCAPE; }
                CHECK(SDL_PushEvent(&input));
                SDL_Event event;
                bool handled = false;
                while (SDL_PollEvent(&event)) if (sv_native_input(&native, app, &event)) handled = true;
                CHECK(handled);
                CHECK(sv_app_dispatch_input(app, 16).dispatched == 1);
            }
            const unsigned char expected[] = {184,0x12,0x34,0x56,0x78,key};
            SvOutput reply = sv_app_take_output(app, gen, output, sizeof(output));
            CHECK(reply.result == SV_OK && reply.size == sizeof(expected));
            CHECK(!memcmp(output, expected, sizeof(expected)));
            view = sv_app_view(app);
            CHECK(!view.request.pending && !view.request.prompt[0] && view.context == SV_CONTEXT_GAME);
            CHECK(sv_scenario_frame(&rebuilt, view));
            CHECK(sv_app_take_output(app, gen, output, sizeof(output)).result == SV_WAITING);
            ++cases;
        }
    }
    CHECK(sv_app_close(app) == SV_OK);
    printf("SV request checks passed cases=%u submitted_frames=%u fallback_routes=0\n", cases, cases * 3);
    return 1;
}

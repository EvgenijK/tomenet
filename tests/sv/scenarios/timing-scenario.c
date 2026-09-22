#include "timing-scenario.h"
#include "timing.h"
#include "native-input.h"
#include <stdio.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Timing scenario failed line %d: %s (%s)\n", __LINE__, #test, SDL_GetError()); \
    (void)sv_app_observe(app, (SvPresentationObserver){0}); return 0; } } while (0)
int sv_timing_scenario(SvApp *app, SvUi *ui, int delayed)
{
    const int version[] = {4,7,0,2,0,2};
    const unsigned char request[] = {184,0,0,0,7,'K','e','y','?',0};
    const unsigned char hp[] = {24,0,100,0,50,0};
    const unsigned char updates[] = {24,0,100,0,10,0,46,'E',0,46,'E',0,46,253,'C',0};
    SvTiming timing = {0};
    CHECK(sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    /* Renderer/window initialization is outside decode/input latency. */
    CHECK(sv_ui_submit(ui, sv_app_view(app)));
    CHECK(sv_app_observe(app, sv_timing_observer(&timing)) == SV_OK);
    CHECK(sv_app_receive(app, gen, request, sizeof(request)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    CHECK(sv_timing_submit(&timing, ui, sv_app_view(app)));
    CHECK(timing.samples == 1 && timing.count == 0);
    SvAppView previous = sv_app_view(app);
    /* An incomplete packet starts no display deadline. */
    CHECK(sv_app_receive(app, gen, hp, 3) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 0 && timing.count == 0);
    CHECK(sv_app_receive(app, gen, hp + 3, sizeof(hp) - 3) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    if (delayed) {
        /* Negative control: an old frame must not acknowledge a newer update.
         * Delay BEFORE the second HP proves coalescing cannot reset the deadline. */
        CHECK(sv_timing_submit(&timing, ui, previous));
        CHECK(timing.samples == 1 && timing.count == 1);
        SDL_Delay(65);
    }
    CHECK(sv_app_receive(app, gen, updates, sizeof(updates)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 4);
    CHECK(sv_app_view(app).status.current == 10 && sv_app_view(app).request.pending);
    CHECK(sv_timing_submit(&timing, ui, sv_app_view(app)));
    CHECK(timing.samples == 5 && timing.count == 0);
    SvMessage first, second, chat;
    CHECK(sv_app_take_message(app, gen, &first) == SV_OK);
    CHECK(sv_app_take_message(app, gen, &second) == SV_OK);
    CHECK(sv_app_take_message(app, gen, &chat) == SV_OK);
    CHECK(first.sequence + 1 == second.sequence && first.bytes[0] == 'E' && second.bytes[0] == 'E');
    SvNativeInput input;
    sv_native_input_begin(&input, app);
    SDL_Event key = {0};
    key.type = SDL_EVENT_TEXT_INPUT; key.text.text = "Y";
    key.text.timestamp = SDL_GetTicksNS(); key.text.windowID = SDL_GetWindowID(ui->window);
    CHECK(sv_native_input(&input, app, &key));
    CHECK(sv_app_dispatch_input(app, 16).dispatched == 1);
    CHECK(!sv_app_view(app).request.pending);
    CHECK(sv_timing_submit(&timing, ui, sv_app_view(app)));
    CHECK(timing.samples == 6 && timing.count == 0);
    unsigned char bytes[16];
    const unsigned char expected[] = {184,0,0,0,7,'Y'};
    SvOutput output = sv_app_take_output(app, gen, bytes, sizeof(bytes));
    CHECK(output.result == SV_OK && output.size == sizeof(expected) && !memcmp(bytes, expected, sizeof(expected)));
    /* Cancellation is an urgent outcome even though it starts with local input. */
    CHECK(sv_app_receive(app, gen, request, sizeof(request)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    CHECK(sv_timing_submit(&timing, ui, sv_app_view(app)));
    key.type = SDL_EVENT_KEY_DOWN; key.key.key = SDLK_ESCAPE;
    key.key.repeat = false; key.key.mod = 0; key.key.timestamp = SDL_GetTicksNS();
    CHECK(sv_native_input(&input, app, &key));
    if (delayed) SDL_Delay(25);
    CHECK(sv_app_dispatch_input(app, 16).dispatched == 1);
    CHECK(sv_timing_submit(&timing, ui, sv_app_view(app)));
    CHECK(timing.samples == 8 && timing.count == 0);
    output = sv_app_take_output(app, gen, bytes, sizeof(bytes));
    const unsigned char cancelled[] = {184,0,0,0,7,0};
    CHECK(output.result == SV_OK && output.size == sizeof(cancelled) && !memcmp(bytes, cancelled, sizeof(cancelled)));
    CHECK(sv_app_observe(app, (SvPresentationObserver){0}) == SV_OK);
    CHECK(sv_app_close(app) == SV_OK);
    printf("SV timing result samples=%u violations=%u invalid=%u background=unexercised delayed=%d\n",
           timing.samples, timing.violations, timing.invalid, delayed);
    if (timing.violations || timing.invalid) return SDL_SetError("Submission timing gate failed");
    return 1;
}

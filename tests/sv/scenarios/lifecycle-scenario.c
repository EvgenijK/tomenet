#include "lifecycle-scenario.h"
#include "input/native-input.h"
#include "native-frame.h"
#include <stdio.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Lifecycle scenario failed at line %d: %s (%s)\n", __LINE__, #test, SDL_GetError()); \
    return 0; } } while (0)
static void pump(SvApp *app, SvUi *ui, SvNativeInput *input)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (!sv_ui_event(ui, &event)) (void)sv_native_input(input, app, &event);
    }
}
int sv_lifecycle_scenario(SvApp *app, SvUi *ui)
{
    const int version[] = {4,7,0,2,0,2};
    const unsigned char request[] = {184,0,0,0,7,'K','e','y','?',0};
    const Uint32 events[] = {SDL_EVENT_WINDOW_RESIZED, SDL_EVENT_WINDOW_FOCUS_LOST,
        SDL_EVENT_WINDOW_MINIMIZED, SDL_EVENT_WINDOW_RESTORED,
        SDL_EVENT_WINDOW_FOCUS_GAINED, SDL_EVENT_RENDER_TARGETS_RESET};
    CHECK(sv_app_open(app, version) == SV_OK);
    SvNativeInput input;
    sv_native_input_begin(&input, app);
    CHECK(sv_app_bind_macro(app, 'm', 'Y', SV_MACRO_NORMAL) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    CHECK(sv_app_receive(app, gen, request, sizeof(request)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    SvAppView initial = sv_app_view(app);
    CHECK(sv_scenario_frame(ui, initial));
    SDL_Event text = {0};
    text.type = SDL_EVENT_TEXT_INPUT; text.text.text = "m";
    text.text.windowID = SDL_GetWindowID(ui->window);
    CHECK(SDL_PushEvent(&text));
    pump(app, ui, &input);
    CHECK(sv_app_dispatch_input(app, 0).pending == 1);
    unsigned char output[32];
    for (unsigned i = 0; i < SDL_arraysize(events); ++i) {
        if (events[i] == SDL_EVENT_WINDOW_RESIZED) {
            int w, h;
            CHECK(SDL_GetWindowSize(ui->window, &w, &h));
            CHECK(SDL_SetWindowSize(ui->window, w + 64, h + 48));
            CHECK(SDL_SyncWindow(ui->window));
        } else if (events[i] == SDL_EVENT_WINDOW_MINIMIZED) {
            CHECK(SDL_MinimizeWindow(ui->window));
        } else if (events[i] == SDL_EVENT_WINDOW_RESTORED) {
            CHECK(SDL_RestoreWindow(ui->window));
            CHECK(SDL_SyncWindow(ui->window));
        }
        /* Deterministic lifecycle delivery, including focus changes controlled
         * by the desktop in an actual session. Same production event handler. */
        SDL_Event event = {0}; event.type = events[i];
        event.window.windowID = SDL_GetWindowID(ui->window);
        CHECK(SDL_PushEvent(&event));
        pump(app, ui, &input);
        const unsigned char updates[] = {24,0,100,0,(unsigned char)(30 + i),0,46,'E',0,46,'E',0};
        CHECK(sv_app_receive(app, gen, updates, sizeof(updates)) == SV_OK);
        CHECK(sv_app_step(app, 16).processed == 3);
        SvAppView view = sv_app_view(app);
        CHECK(view.request.sequence == initial.request.sequence && view.request.id == 7);
        CHECK(view.context == SV_CONTEXT_KEY_REQUEST && view.status.current == (int)(30 + i));
        CHECK(!memcmp(view.request.prompt, initial.request.prompt, SV_REQUEST_BYTES));
        CHECK(sv_app_dispatch_input(app, 0).pending == 1);
        CHECK(sv_app_take_output(app, gen, output, sizeof(output)).result == SV_WAITING);
        SvMessage first, second;
        CHECK(sv_app_take_message(app, gen, &first) == SV_OK);
        CHECK(sv_app_take_message(app, gen, &second) == SV_OK);
        CHECK(first.sequence + 1 == second.sequence && first.bytes[0] == 'E' && second.bytes[0] == 'E');
        CHECK(sv_scenario_frame(ui, view));
    }
    CHECK(sv_app_dispatch_input(app, 16).dispatched == 1);
    const unsigned char expected[] = {184,0,0,0,7,'Y'};
    SvOutput reply = sv_app_take_output(app, gen, output, sizeof(output));
    CHECK(reply.result == SV_OK && reply.size == sizeof(expected));
    CHECK(!memcmp(output, expected, sizeof(expected)));
    CHECK(sv_app_view(app).context == SV_CONTEXT_GAME && !sv_app_view(app).request.pending);
    sv_ui_rebuild(ui);
    CHECK(sv_scenario_frame(ui, sv_app_view(app)));
    CHECK(!ui->prompt.length);
    CHECK(sv_app_take_output(app, gen, output, sizeof(output)).result == SV_WAITING);

    /* Teardown with accepted input, undelivered messages, and an SDL event. */
    CHECK(sv_app_receive(app, gen, request, sizeof(request)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    SvAppView old = sv_app_view(app);
    CHECK(sv_app_accept_key(app, gen, old.request.sequence, 'm') == SV_OK);
    const unsigned char message[] = {46,'O','l','d',0};
    CHECK(sv_app_receive(app, gen, message, sizeof(message)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    text.common.timestamp = SDL_GetTicksNS();
    CHECK(SDL_PushEvent(&text));
    CHECK(sv_app_close(app) == SV_OK && sv_app_open(app, version) == SV_OK);
    sv_native_input_begin(&input, app);
    uint64_t fresh = sv_app_view(app).generation;
    CHECK(sv_app_receive(app, fresh, request, sizeof(request)) == SV_OK);
    CHECK(sv_app_step(app, 16).processed == 1);
    pump(app, ui, &input);
    CHECK(sv_app_dispatch_input(app, 16).dispatched == 0);
    CHECK(sv_app_accept_key(app, gen, old.request.sequence, 'x') == SV_STALE);
    CHECK(sv_app_take_output(app, fresh, output, sizeof(output)).result == SV_WAITING);
    SvMessage gone;
    CHECK(sv_app_take_message(app, fresh, &gone) == SV_WAITING);
    CHECK(sv_app_view(app).request.pending && !sv_app_view(app).messages.count);
    sv_ui_rebuild(ui);
    CHECK(sv_scenario_frame(ui, sv_app_view(app)));
    /* A fresh SDL key reaches the new owner, exactly once. */
    text.common.timestamp = 0;
    CHECK(SDL_PushEvent(&text));
    pump(app, ui, &input);
    CHECK(sv_app_dispatch_input(app, 16).dispatched == 1);
    reply = sv_app_take_output(app, fresh, output, sizeof(output));
    CHECK(reply.result == SV_OK && reply.size == sizeof(expected) && !memcmp(output, expected, sizeof(expected)));
    CHECK(sv_scenario_frame(ui, sv_app_view(app)));
    CHECK(sv_app_take_output(app, fresh, output, sizeof(output)).result == SV_WAITING);
    CHECK(sv_app_close(app) == SV_OK);
    puts("SV lifecycle checks passed transitions=6 submitted_frames=10 replies=2 fallback_routes=0");
    return 1;
}

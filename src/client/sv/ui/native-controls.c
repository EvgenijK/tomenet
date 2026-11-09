#include "ui/native-controls.h"

static bool submit(void *context, SvAppView view, bool pause)
{
    SvNativeControls *controls = context;
    if (!controls->draw(controls->context, view) || !SDL_RenderPresent(controls->renderer))
        return false;
    uint64_t now_ns = SDL_GetTicksNS();
    if (pause) sv_native_input_pause(controls->input, view, now_ns);
    else {
        /* due_ms reflects baseline option/count policy; translate its optional
         * one-ms pacing to a fresh monotonic deadline after actual submission. */
        uint64_t frame_ms = controls->frame_ms;
        controls->due_ns = now_ns + (view.flush_due_ms > frame_ms ? 1000000 : 0);
    }
    return true;
}
SvResult sv_native_controls_bind(SvNativeControls *controls, SvApp *app)
{
    if (!controls || !controls->renderer || !controls->input || !controls->draw)
        return SV_INVALID;
    sv_native_input_begin(controls->input, app);
    controls->generation = sv_app_view(app).generation;
    controls->due_ns = 0;
    controls->batch = false;
    return sv_app_set_control_sink(app, (SvControlSink){controls, submit, NULL});
}
SvStep sv_native_controls_step(SvNativeControls *controls, SvApp *app, size_t budget)
{
    SvStep total = {0, SV_PROTOCOL_CAPACITY - sv_app_receive_capacity(app), SV_OK};
    SvAppView view = sv_app_view(app);
    if (view.generation != controls->generation) {
        sv_native_input_begin(controls->input, app);
        controls->generation = view.generation;
        controls->due_ns = 0;
        controls->batch = false;
    }
    if (!view.active) { controls->due_ns = 0; controls->batch = false;
        total.pending_bytes = 0; total.result = SV_CLOSED; return total; }
    if (SDL_GetTicksNS() < controls->due_ns) { total.result = SV_WAITING; return total; }
    if (!controls->batch) {
        controls->frame_ms = SDL_GetTicks();
        SvResult result = sv_app_frame(app, view.generation, controls->frame_ms);
        if (result != SV_OK) { total.result = result; return total; }
        controls->batch = true;
        controls->remaining = budget;
    }
    while (total.processed < budget && controls->remaining) {
        SvStep one = sv_app_step(app, 1);
        total.processed += one.processed;
        controls->remaining -= one.processed;
        total.pending_bytes = one.pending_bytes;
        total.result = one.result;
        if (one.result != SV_OK || !one.processed || SDL_GetTicksNS() < controls->due_ns) break;
    }
    if (!controls->remaining || total.result == SV_WAITING ||
        (!total.pending_bytes && total.result != SV_BACKPRESSURE)) controls->batch = false;
    return total;
}

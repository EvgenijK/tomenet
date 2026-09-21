#include "app.h"
#include "protocol.h"
#include <stdlib.h>
struct SvApp {
    SvSession *session;
    SvProtocol *protocol;
    SvAppView view;
    SvAlertSink sink;
    SvAlertOptions options;
    SvAttention attention;
    int busy;
};
static void release_session(SvApp *app)
{
    sv_protocol_destroy(app->protocol); app->protocol = NULL;
    sv_session_destroy(app->session); app->session = NULL;
    app->view.active = 0;
}
static void fail_session(SvApp *app, SvResult reason)
{
    release_session(app); app->view.reason = reason;
}
SvApp *sv_app_create(SvAlertSink sink)
{
    SvApp *app = calloc(1, sizeof(*app));
    if (app) { app->sink = sink; app->view.reason = SV_CLOSED; }
    return app;
}
SvResult sv_app_destroy(SvApp *app)
{
    if (!app) return SV_OK;
    if (app->busy) return SV_BUSY;
    release_session(app); free(app);
    return SV_OK;
}
SvResult sv_app_close(SvApp *app)
{
    if (app->busy) return SV_BUSY;
    fail_session(app, SV_CLOSED); return SV_OK;
}
SvResult sv_app_open(SvApp *app, const int version[6])
{
    if (app->busy) return SV_BUSY;
    if (!version || version[0] <= 0) return SV_INVALID;
    if (app->view.generation == UINT64_MAX) return SV_INVALID;
    release_session(app);
    ++app->view.generation;
    app->view.status = (SvStatus){0};
    app->view.executor_failed = 0;
    app->protocol = sv_protocol_create(version);
    app->session = sv_session_create();
    if (!app->protocol || !app->session) { fail_session(app, SV_NO_MEMORY); return SV_NO_MEMORY; }
    app->view.active = 1; app->view.reason = SV_OK;
    return SV_OK;
}
SvResult sv_app_set_alerts(SvApp *app, SvAlertOptions options, SvAttention attention)
{
    if (app->busy) return SV_BUSY;
    app->options = options; app->attention = attention;
    return SV_OK;
}
SvAppView sv_app_view(const SvApp *app) { return app->view; }
static SvResult accepts(const SvApp *app, uint64_t generation)
{
    if (app->busy) return SV_BUSY;
    if (generation != app->view.generation) return SV_STALE;
    if (!app->view.active) return SV_CLOSED;
    return SV_OK;
}
SvResult sv_app_receive(SvApp *app, uint64_t generation, const void *bytes, size_t size)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    result = sv_protocol_receive(app->protocol, bytes, size);
    if (result == SV_INPUT_OVERFLOW) fail_session(app, result);
    return result;
}
SvResult sv_app_receive_owned(SvApp *app, SvOwnedBytes *input)
{
    if (!input || !input->release) return SV_INVALID;
    SvResult result = sv_app_receive(app, input->generation, input->bytes, input->size);
    if (result != SV_BACKPRESSURE && result != SV_BUSY) {
        app->busy = 1;
        input->release(input->owner);
        *input = (SvOwnedBytes){0};
        app->busy = 0;
    }
    return result;
}
size_t sv_app_receive_capacity(const SvApp *app)
{
    return app->view.active ? sv_protocol_capacity(app->protocol) : 0;
}
SvStep sv_app_step(SvApp *app, size_t budget)
{
    SvStep step = {0, 0, SV_OK};
    if (app->busy) { step.result = SV_BUSY; return step; }
    if (!app->view.active) { step.result = SV_CLOSED; return step; }
    app->busy = 1;
    while (step.processed < budget) {
        SvHpUpdate hp;
        step.result = sv_protocol_next(app->protocol, &hp);
        if (step.result == SV_WAITING) break;
        if (step.result != SV_OK && step.result != SV_RECOVERED) {
            fail_session(app, step.result); break;
        }
        ++step.processed;
        if (step.result == SV_RECOVERED) break;
        SvStatusChange change = sv_session_apply_hp(app->session, hp);
        app->view.status = change.after;
        SvAlertEffects effects = sv_alerts_evaluate(change, app->options, app->attention);
        if (!sv_alerts_deliver(app->sink, effects)) app->view.executor_failed = 1;
    }
    step.pending_bytes = app->protocol ? sv_protocol_pending(app->protocol) : 0;
    app->busy = 0;
    return step;
}
SvOutput sv_app_take_output(SvApp *app, uint64_t generation, void *bytes, size_t capacity)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return (SvOutput){result, 0};
    return sv_protocol_output(app->protocol, bytes, capacity);
}

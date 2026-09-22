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
    SvInputRouter input;
    SvInputBindings bindings;
};
static void release_session(SvApp *app)
{
    sv_protocol_destroy(app->protocol); app->protocol = NULL;
    sv_session_destroy(app->session); app->session = NULL;
    app->input = (SvInputRouter){0};
    app->view.request = (SvKeyRequest){0};
    app->view.context = SV_CONTEXT_GAME;
    app->view.active = 0;
    app->view.messages = (SvMessages){0};
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
static SvResult deliver_reply(SvApp *app, SvKeyReply reply)
{
    SvResult result = sv_protocol_key_reply(app->protocol, reply.id, reply.key);
    if (result != SV_OK) { fail_session(app, result); return result; }
    return sv_session_complete_request(app->session, reply.sequence);
}
static SvResult refresh_request(SvApp *app)
{
    SvKeyReply reply;
    app->view.request = sv_session_request(app->session);
    SvResult result = sv_input_sync(&app->input, app->view.request, &reply);
    if (result == SV_OK) {
        result = deliver_reply(app, reply);
        if (result != SV_OK) return result;
        app->view.request = sv_session_request(app->session);
        (void)sv_input_sync(&app->input, app->view.request, &reply);
    } else if (result != SV_WAITING) return result;
    app->view.context = app->input.context;
    return SV_OK;
}
SvResult sv_app_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    SvKeyReply reply;
    result = sv_input_key(&app->input, sv_session_request(app->session), sequence, key, &reply);
    if (result != SV_OK) return result;
    result = deliver_reply(app, reply);
    return result == SV_OK ? refresh_request(app) : result;
}
SvResult sv_app_bind_macro(SvApp *app, unsigned char trigger, unsigned char action, SvMacroKind kind)
{
    if (app->busy) return SV_BUSY;
    return sv_input_bind(&app->bindings, trigger, action, kind);
}
SvResult sv_app_accept_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    result = sv_input_accept(&app->input, &app->bindings, sv_session_request(app->session), sequence, key);
    if (result == SV_KEY_OVERFLOW) fail_session(app, result);
    return result;
}
SvInputStep sv_app_dispatch_input(SvApp *app, size_t budget)
{
    SvInputStep step = {0, 0, app->input.count, accepts(app, app->view.generation)};
    if (step.result != SV_OK) return step;
    app->busy = 1;
    while (step.dispatched + step.stale < budget) {
        SvKeyReply reply;
        SvResult result = sv_input_next(&app->input, sv_session_request(app->session), &reply);
        if (result == SV_WAITING) break;
        if (result == SV_STALE) { ++step.stale; continue; }
        if (result == SV_OK) result = deliver_reply(app, reply);
        if (result == SV_OK) result = refresh_request(app);
        if (result != SV_OK) { step.result = result; break; }
        ++step.dispatched;
    }
    step.pending = app->input.count;
    app->busy = 0;
    return step;
}
SvStep sv_app_step(SvApp *app, size_t budget)
{
    SvStep step = {0, 0, SV_OK};
    if (app->busy) { step.result = SV_BUSY; return step; }
    if (!app->view.active) { step.result = SV_CLOSED; return step; }
    app->busy = 1;
    while (step.processed < budget) {
        SvChange decoded;
        step.result = sv_protocol_next(app->protocol, &decoded);
        if (step.result == SV_WAITING) break;
        if (step.result != SV_OK && step.result != SV_RECOVERED) {
            fail_session(app, step.result); break;
        }
        ++step.processed;
        if (step.result == SV_RECOVERED) break;
        SvSessionChange change = sv_session_apply(app->session, &decoded);
        step.result = change.result;
        if (step.result != SV_OK) { fail_session(app, step.result); break; }
        step.result = refresh_request(app);
        if (step.result != SV_OK) break;
        app->view.status = change.status.after;
        app->view.messages = sv_session_messages(app->session);
        SvAlertEffects effects = sv_alerts_evaluate(change.status, app->options, app->attention);
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

SvResult sv_app_take_message(SvApp *app, uint64_t generation, SvMessage *message)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    return sv_session_take_message(app->session, message);
}

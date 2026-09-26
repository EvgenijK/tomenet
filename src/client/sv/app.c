#include "app.h"
#include "protocol/protocol.h"
#include "../../common/pack.h"
#include <stdlib.h>
#include <string.h>
struct SvApp {
    SvSession *session;
    SvProtocol *protocol;
    SvAppView view;
    SvAlertSink sink;
    SvAlertOptions options;
    SvFlushPolicy flush;
    SvAttention attention;
    int busy;
    SvRuntimeCheck *checks[8];
    unsigned check_depth;
    int check_failed[8];
    SvInputRouter input;
    SvInputBindings bindings;
    SvMacroSet *macros;
    SvMacroRunner *macro_runner;
    uint64_t macro_now_ms;
    uint64_t macro_confirmation_owner;
    SvCommandRouter command;
    SvPresentationObserver observer;
    uint64_t input_started_ns, input_sequence;
};
static SvResult accepts(const SvApp *app, uint64_t generation);
static uint64_t now(SvApp *app)
{
    return app->observer.now ? app->observer.now(app->observer.context) : 0;
}
static void publish(SvApp *app, SvPresentationOrigin origin, uint64_t started,
                    uint64_t occurrence, int interactive)
{
    ++app->view.revision;
    if (app->observer.changed) {
        int busy = app->busy;
        app->busy = 1;
        app->observer.changed(app->observer.context, (SvPresentationEvent){
            origin, app->view.generation, app->view.revision, started, occurrence, interactive});
        app->busy = busy;
    }
}
SvResult sv_app_observe(SvApp *app, SvPresentationObserver observer)
{
    if (app->busy) return SV_BUSY;
    if (!!observer.now != !!observer.changed) return SV_INVALID;
    app->observer = observer;
    return SV_OK;
}
static void release_session(SvApp *app)
{
    sv_protocol_destroy(app->protocol); app->protocol = NULL;
    sv_session_destroy(app->session); app->session = NULL;
    app->input = (SvInputRouter){0};
    if (app->macro_runner) sv_macros_reset(app->macro_runner);
    app->macro_now_ms = 0;
    app->macro_confirmation_owner = 0;
    app->command.bypass = app->command.control = app->command.chat = false;
    app->command.direction_pending = SV_COMMAND_NONE;
    memset(app->command.text, 0, sizeof(app->command.text));
    app->command.text_size = 0;
    app->input_started_ns = app->input_sequence = 0;
    app->view.request = (SvKeyRequest){0};
    app->view.context = SV_CONTEXT_GAME;
    app->view.active = 0;
    app->view.messages = (SvMessages){0};
    app->view.ping = (SvPingTelemetry){0};
    memset(app->view.server_flags, 0, sizeof(app->view.server_flags));
    app->view.paused = 0;
    app->view.flush_due_ms = 0;
    app->flush.count = 0;
    app->flush.frame_ms = 0;
}
static void fail_session(SvApp *app, SvResult reason)
{
    uint64_t started = now(app);
    release_session(app); app->view.reason = reason;
    publish(app, SV_PRESENT_LIFECYCLE, started, 0, 0);
}
SvApp *sv_app_create(SvAlertSink sink)
{
    SvApp *app = calloc(1, sizeof(*app));
    if (app) {
        app->macros = calloc(1, sizeof(*app->macros));
        app->macro_runner = calloc(1, sizeof(*app->macro_runner));
        if (!app->macros || !app->macro_runner) {
            free(app->macros); free(app->macro_runner); free(app); return NULL;
        }
        app->sink = sink; app->view.reason = SV_CLOSED;
    }
    return app;
}
SvResult sv_app_destroy(SvApp *app)
{
    if (!app) return SV_OK;
    if (app->busy || app->check_depth) return SV_BUSY;
    release_session(app); free(app->macros); free(app->macro_runner); free(app);
    return SV_OK;
}
SvResult sv_app_close(SvApp *app)
{
    return sv_app_close_reason(app, SV_CLOSED);
}
SvResult sv_app_close_reason(SvApp *app, SvResult reason)
{
    if (app->busy) return SV_BUSY;
    if (reason == SV_OK || reason == SV_WAITING || reason == SV_BACKPRESSURE) return SV_INVALID;
    fail_session(app, reason); return SV_OK;
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
    app->view.ping = sv_session_ping(app->session);
    return SV_OK;
}
SvResult sv_app_set_alerts(SvApp *app, SvAlertOptions options, SvAttention attention)
{
    if (app->busy) return SV_BUSY;
    app->options = options; app->attention = attention;
    return SV_OK;
}
SvResult sv_app_set_flush_options(SvApp *app, SvFlushOptions options)
{
    if (app->busy) return SV_BUSY;
    app->flush.options = options;
    return SV_OK;
}
SvResult sv_app_frame(SvApp *app, uint64_t generation, uint64_t now_ms)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    sv_flush_frame(&app->flush, now_ms);
    return SV_OK;
}
SvAppView sv_app_view(const SvApp *app) { return app->view; }
SvResult sv_app_set_character_setup(SvApp *app, uint64_t generation,
                                    const SvContactSetup *setup)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    result = sv_session_set_character_setup(app->session, setup);
    if (result == SV_OK) publish(app, SV_PRESENT_CONTROL, now(app), 0, 0);
    return result;
}
const SvContactSetup *sv_app_character_setup(const SvApp *app, uint64_t generation)
{
    return app && app->view.active && app->view.generation == generation ?
        sv_session_character_setup(app->session) : NULL;
}
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
    if (result != SV_OK) {
        if (result != SV_BACKPRESSURE) fail_session(app, result);
        return result;
    }
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
    uint64_t started = now(app);
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (app->view.paused) return SV_BUSY;
    SvKeyReply reply;
    result = sv_input_key(&app->input, sv_session_request(app->session), sequence, key, &reply);
    if (result != SV_OK) return result;
    result = deliver_reply(app, reply);
    if (result == SV_OK) result = refresh_request(app);
    if (result == SV_OK) publish(app, SV_PRESENT_INPUT, started, sequence, !reply.cancelled);
    return result;
}
SvResult sv_app_raw_key(SvApp *app, uint64_t generation, unsigned char key)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (app->view.paused || app->view.context != SV_CONTEXT_GAME || app->view.request.pending) return SV_BUSY;
    if (!key || key == 27 || key == '-' || key == ' ' || key == 13 || key == 10) return SV_INVALID;
    result = sv_protocol_raw_key(app->protocol, key);
    if (result == SV_OUTPUT_OVERFLOW) fail_session(app, result);
    return result;
}
SvResult sv_app_command_mode(SvApp *app, bool roguelike)
{
    if (!app) return SV_INVALID;
    if (app->busy) return SV_BUSY;
    app->command.roguelike = roguelike;
    return SV_OK;
}
SvResult sv_app_keymap_record(SvApp *app, unsigned char key,
                              unsigned char command, unsigned char direction)
{
    if (!app) return SV_INVALID;
    if (app->busy) return SV_BUSY;
    return sv_command_override(&app->command, key, command, direction);
}
bool sv_app_macro_idle(const SvApp *app)
{
    return app && !app->macro_runner->active &&
        !app->macro_runner->fresh_count && !app->macro_runner->ready_count &&
        !app->macro_runner->waiting;
}
SvResult sv_app_install_input_profile(SvApp *app, const SvMacroSet *macros,
    bool roguelike, const unsigned char commands[128],
    const unsigned char directions[128])
{
    if (!app || !macros || !commands || !directions || macros->count > SV_MACRO_LIMIT)
        return SV_INVALID;
    if (app->busy || !sv_app_macro_idle(app)) return SV_BUSY;
    for (size_t i = 0; i < macros->count; ++i) {
        const SvMacroDefinition *entry = &macros->definitions[i];
        if (!entry->trigger_size || entry->trigger_size > SV_MACRO_TRIGGER ||
            entry->action_size > SV_MACRO_ACTION ||
            entry->kind < SV_MACRO_NORMAL || entry->kind > SV_MACRO_COMMAND ||
            memchr(entry->trigger, 0, entry->trigger_size) ||
            memchr(entry->action, 0, entry->action_size)) return SV_INVALID;
    }
    SvCommandRouter next = app->command;
    next.roguelike = roguelike;
    memset(next.override, 0, sizeof(next.override));
    for (size_t i = 1; i < 128; ++i) {
        if (!commands[i]) continue;
        SvResult result = sv_command_override(&next, (unsigned char)i,
                                               commands[i], directions[i]);
        if (result != SV_OK) return result;
    }
    *app->macros = *macros;
    app->command = next;
    return SV_OK;
}
static SvResult command_key(SvApp *app, uint64_t generation, unsigned char key)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (app->view.paused || app->view.context != SV_CONTEXT_GAME || app->view.request.pending)
        return SV_BUSY;
    return sv_command_dispatch(&app->command, app->protocol, key);
}
SvResult sv_app_keepalive(SvApp *app, uint64_t generation,
                          uint64_t now_ms, uint64_t last_sent_ms)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (now_ms < last_sent_ms || now_ms - last_sent_ms < 2000 ||
        sv_protocol_output_capacity(app->protocol) != SV_PROTOCOL_CAPACITY) return SV_WAITING;
    return sv_protocol_keepalive(app->protocol);
}
SvResult sv_app_set_ping_clock(SvApp *app, SvPingClock clock)
{
    if (app->busy) return SV_BUSY;
    if (!app->view.active) return SV_CLOSED;
    return sv_protocol_set_ping_clock(app->protocol, clock);
}
SvResult sv_app_send_ping(SvApp *app, uint64_t generation)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    result = sv_protocol_send_ping(app->protocol);
    if (result == SV_OK) {
        sv_session_ping_sent(app->session);
        app->view.ping = sv_session_ping(app->session);
    }
    return result;
}
SvResult sv_app_ack_pause(SvApp *app, uint64_t generation, uint64_t sequence)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    result = sv_input_ack_pause(&app->input, sequence);
    if (result != SV_OK) return result;
    app->view.paused = app->input.paused;
    publish(app, SV_PRESENT_INPUT, now(app), 0, 1);
    return SV_OK;
}
SvResult sv_app_bind_macro(SvApp *app, unsigned char trigger, unsigned char action, SvMacroKind kind)
{
    if (app->busy) return SV_BUSY;
    return sv_input_bind(&app->bindings, trigger, action, kind);
}
SvResult sv_app_define_macro(SvApp *app, const unsigned char *trigger, size_t trigger_size,
                             const unsigned char *action, size_t action_size, SvMacroKind kind)
{
    if (!app) return SV_INVALID;
    if (app->busy || app->macro_runner->active) return SV_BUSY;
    return sv_macros_define(app->macros, trigger, trigger_size, action, action_size, kind);
}
SvResult sv_app_delete_macro(SvApp *app, const unsigned char *trigger, size_t trigger_size)
{
    if (!app) return SV_INVALID;
    if (app->busy || app->macro_runner->active) return SV_BUSY;
    return sv_macros_delete(app->macros, trigger, trigger_size);
}
SvResult sv_app_queue_macro_action(SvApp *app, const unsigned char *action, size_t size)
{
    if (!app) return SV_INVALID;
    if (app->busy) return SV_BUSY;
    return sv_macros_queue_action(app->macro_runner, action, size);
}
SvResult sv_app_macro_frame(SvApp *app, uint64_t generation, uint64_t now_ms, size_t budget)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    app->macro_now_ms = now_ms;
    if (app->macro_confirmation_owner) {
        unsigned char command;
        result = sv_app_take_confirmation(app, generation,
                                          app->macro_confirmation_owner, &command);
        if (result == SV_OK) app->macro_runner->confirmed = true;
        else if (result != SV_WAITING) return result;
        if (!app->macro_runner->waiting || app->macro_runner->confirmed ||
            app->macro_runner->semaphore || now_ms >= app->macro_runner->wait_deadline_ms) {
            result = sv_app_end_confirmation(app, generation, app->macro_confirmation_owner);
            if (result != SV_OK) return result;
            app->macro_confirmation_owner = 0;
        }
    }
    result = sv_macros_pump(app->macros, app->macro_runner,
        !app->view.request.pending, app->command.chat, false, true, now_ms);
    if (result == SV_INPUT_OVERFLOW) fail_session(app, result);
    if (result != SV_OK && result != SV_WAITING) return result;
    if (app->macro_runner->waiting && !app->macro_confirmation_owner) {
        result = sv_app_begin_confirmation(app, generation, &app->macro_confirmation_owner);
        if (result != SV_OK) return result;
    }
    for (size_t i = 0; i < budget; ++i) {
        unsigned char key;
        if (sv_macros_peek(app->macro_runner, &key) != SV_OK) break;
        if (app->view.request.pending) {
            if (app->input.count) break;
            result = sv_input_enqueue_resolved(&app->input, app->view.request,
                                                app->view.request.sequence, key);
            if (result != SV_OK) return result;
        } else {
            result = command_key(app, generation, key);
            if (result == SV_BACKPRESSURE) return result;
            if (result == SV_INVALID) fail_session(app, result);
            if (result != SV_OK) return result;
        }
        sv_macros_consume(app->macro_runner);
        if (app->view.request.pending) break;
    }
    return SV_OK;
}
bool sv_app_macro_extended_waiting(const SvApp *app, uint64_t generation)
{
    return app && app->view.active && app->view.generation == generation &&
           app->macro_runner->waiting && app->macro_runner->extended_wait;
}
static bool macro_path(const SvApp *app)
{
    return app->macros->count || app->macro_runner->direct_pending ||
           app->macro_runner->active || app->macro_runner->fresh_count ||
           app->macro_runner->ready_count;
}
SvResult sv_app_bind_physical(SvApp *app, const unsigned char *bytes, size_t size,
                              unsigned char action, SvMacroKind kind)
{
    if (app->busy) return SV_BUSY;
    return sv_input_bind_physical(&app->bindings, bytes, size, action, kind);
}
SvResult sv_app_physical(SvApp *app, uint64_t generation, const unsigned char *bytes, size_t size)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (app->view.paused) {
        unsigned char acknowledgement;
        result = sv_input_physical(&app->bindings, bytes, size, false, &acknowledgement);
        return result == SV_OK ? sv_app_ack_pause(app, generation,
            app->view.pause_sequence) : result;
    }
    unsigned char key;
    bool prompt = app->view.request.pending;
    result = sv_input_physical(&app->bindings, bytes, size, prompt, &key);
    if (result != SV_OK) {
        if (result != SV_WAITING || !app->macros->count) return result;
        bool mapped = false;
        for (size_t i = 0; i < app->macros->count; ++i) {
            const SvMacroDefinition *entry = &app->macros->definitions[i];
            if (entry->trigger_size == size &&
                (!prompt || entry->kind != SV_MACRO_COMMAND) &&
                (!app->command.chat || entry->kind == SV_MACRO_NORMAL) &&
                !memcmp(entry->trigger, bytes, size)) {
                mapped = true; break;
            }
        }
        if (!mapped) return SV_WAITING;
        result = sv_macros_feed(app->macros, app->macro_runner, bytes, size,
            !prompt, app->command.chat, false, true, app->macro_now_ms);
        if (result == SV_INPUT_OVERFLOW) fail_session(app, result);
        if (result != SV_OK && result != SV_WAITING) return result;
        return sv_app_macro_frame(app, generation, app->macro_now_ms, 16);
    }
    if (macro_path(app)) {
        result = sv_macros_feed(app->macros, app->macro_runner, &key, 1,
            !prompt, app->command.chat, false, true, app->macro_now_ms);
        if (result == SV_INPUT_OVERFLOW) fail_session(app, result);
        if (result != SV_OK && result != SV_WAITING) return result;
        return sv_app_macro_frame(app, generation, app->macro_now_ms, 16);
    }
    if (prompt) {
        /* The request owner receives exactly one resolved byte; a macro's
         * action is not matched again as a fresh trigger. */
        return sv_app_key(app, generation, app->view.request.sequence, key);
    }
    return command_key(app, generation, key);
}
SvResult sv_app_accept_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key)
{
    uint64_t started = now(app);
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (app->view.paused) return SV_BUSY;
    if (macro_path(app)) {
        if (!app->view.request.pending || app->view.request.sequence != sequence) return SV_STALE;
        result = sv_macros_feed(app->macros, app->macro_runner, &key, 1,
            false, false, false, true, app->macro_now_ms);
        if (result == SV_INPUT_OVERFLOW) fail_session(app, result);
        if (result != SV_OK && result != SV_WAITING) return result;
        return sv_app_macro_frame(app, generation, app->macro_now_ms, 16);
    }
    result = sv_input_accept(&app->input, &app->bindings, sv_session_request(app->session), sequence, key);
    if (result == SV_OK && app->input_sequence != sequence) {
        app->input_sequence = sequence;
        app->input_started_ns = started;
    }
    if (result == SV_KEY_OVERFLOW) fail_session(app, result);
    return result;
}
SvInputStep sv_app_dispatch_input(SvApp *app, size_t budget)
{
    SvInputStep step = {0, 0, app->input.count, accepts(app, app->view.generation)};
    if (step.result != SV_OK) return step;
    app->busy = 1;
    while (step.dispatched + step.stale < budget) {
        if (app->input.count && sv_protocol_output_capacity(app->protocol) < 6) {
            step.result = SV_BACKPRESSURE; break;
        }
        SvKeyReply reply;
        SvResult result = sv_input_next(&app->input, sv_session_request(app->session), &reply);
        if (result == SV_WAITING) break;
        if (result == SV_STALE) { ++step.stale; continue; }
        if (result == SV_OK) result = deliver_reply(app, reply);
        if (result == SV_OK) result = refresh_request(app);
        if (result != SV_OK) { step.result = result; break; }
        publish(app, SV_PRESENT_INPUT, app->input_started_ns, reply.sequence, !reply.cancelled);
        app->input_sequence = app->input_started_ns = 0;
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
    if (app->view.request.aborted) {
        step.result = refresh_request(app);
        if (step.result != SV_OK) {
            step.pending_bytes = sv_protocol_pending(app->protocol);
            app->busy = 0;
            return step;
        }
    }
    while (step.processed < budget) {
        SvChange decoded;
        step.result = sv_protocol_next(app->protocol, &decoded);
        if (step.result == SV_WAITING) break;
        if (step.result == SV_BACKPRESSURE) break;
        if (step.result != SV_OK && step.result != SV_RECOVERED) {
            fail_session(app, step.result); break;
        }
        ++step.processed;
        if (step.result == SV_RECOVERED) break;
        uint64_t started = now(app); /* Complete decode, before apply/effects/UI. */
        SvSessionChange change = sv_session_apply(app->session, &decoded);
        step.result = change.result;
        if (step.result != SV_OK) { fail_session(app, step.result); break; }
        if (decoded.kind == SV_CHANGE_CONFIRM) {
            step.result = sv_input_confirm(&app->input, decoded.confirmed_command);
            if (step.result == SV_EVENT_OVERFLOW) { fail_session(app, step.result); break; }
            step.result = SV_OK; /* Unowned confirms are discarded, as before macro wait. */
        }
        if (decoded.kind == SV_CHANGE_NOOP) continue;
        if (decoded.kind == SV_CHANGE_KEY_REQUEST && app->macro_runner->waiting)
            app->macro_runner->semaphore = true;
        if (decoded.kind == SV_CHANGE_PAUSE) {
            app->view.pause_sequence = sv_input_pause(&app->input);
            app->view.paused = app->input.paused;
        } else if (decoded.kind == SV_CHANGE_FLUSH) {
            ++app->view.flush_sequence;
            app->view.flush_due_ms = sv_flush_request(&app->flush);
        }
        step.result = refresh_request(app);
        if (step.result != SV_OK) break;
        app->view.status = change.status.after;
        app->view.messages = sv_session_messages(app->session);
        app->view.ping = sv_session_ping(app->session);
        SvControlState controls = sv_session_controls(app->session);
        memcpy(app->view.server_flags, controls.server_flags, sizeof(controls.server_flags));
        SvAlertEffects effects = sv_alerts_evaluate(change.status, app->options, app->attention);
        if (!sv_alerts_deliver(app->sink, effects)) app->view.executor_failed = 1;
        SvPresentationOrigin origin = decoded.kind == SV_CHANGE_HP ? SV_PRESENT_HP :
            decoded.kind == SV_CHANGE_MESSAGE ? SV_PRESENT_MESSAGE :
            decoded.kind == SV_CHANGE_PING ? SV_PRESENT_PING :
            decoded.kind == SV_CHANGE_KEY_REQUEST || decoded.kind == SV_CHANGE_REQUEST_ABORT ?
                SV_PRESENT_REQUEST : SV_PRESENT_CONTROL;
        publish(app, origin, started,
            change.message_occurrence, change.message_chat);
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
SvResult sv_app_local_message(SvApp *app, uint64_t generation,
                              const unsigned char *bytes, size_t size)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (!bytes || !size || size >= SV_MESSAGE_BYTES || memchr(bytes, 0, size)) return SV_INVALID;
    SvChange change = {.kind = SV_CHANGE_MESSAGE};
    memcpy(change.message.bytes, bytes, size);
    change.message.length = size;
    SvSessionChange applied = sv_session_apply(app->session, &change);
    if (applied.result != SV_OK) return applied.result;
    app->view.messages = sv_session_messages(app->session);
    publish(app, SV_PRESENT_MESSAGE, now(app), applied.message_occurrence,
            applied.message_chat);
    return SV_OK;
}
SvResult sv_app_begin_confirmation(SvApp *app, uint64_t generation, uint64_t *owner)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    return sv_input_begin_confirmation(&app->input, owner);
}
SvResult sv_app_take_confirmation(SvApp *app, uint64_t generation,
                                  uint64_t owner, unsigned char *command)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    return sv_input_take_confirmation(&app->input, owner, command);
}
SvResult sv_app_end_confirmation(SvApp *app, uint64_t generation, uint64_t owner)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    return sv_input_end_confirmation(&app->input, owner);
}

SvRuntimeCheck sv_app_check_run(SvApp *app, SvRuntimeScenario scenario,
                              int (*run)(SvApp *, void *), void *context)
{
    SvRuntimeCheck check = {scenario, 0, 0};
    if (app->busy || !run || scenario < 0 || scenario >= SV_SCENARIO_COUNT ||
        app->check_depth == 8) {
        if (app->check_depth) app->check_failed[app->check_depth - 1] = 1;
        return check;
    }
    unsigned depth = app->check_depth++;
    app->checks[depth] = &check;
    app->check_failed[depth] = 0;
    int success = run(app, context);
    check.completed = success && !app->check_failed[depth];
    --app->check_depth;
    app->checks[depth] = NULL;
    if (!check.completed && depth) app->check_failed[depth - 1] = 1;
    return check;
}
SvResult sv_app_terminal_fallback(SvApp *app)
{
    for (unsigned i = 0; i < app->check_depth; ++i) {
        if (app->checks[i]->fallback_entries == UINT32_MAX) app->check_failed[i] = 1;
        else ++app->checks[i]->fallback_entries;
    }
    return SV_INVALID; /* No terminal adapter is linked or permitted. */
}
int sv_runtime_write(FILE *stream, SvRuntimeCheck check)
{
    static const char *names[] = {"hp", "message", "request", "lifecycle", "geometry",
        "timing", "timing-delayed", "arch"};
    if (!stream || check.scenario < 0 || check.scenario >= SV_SCENARIO_COUNT) return 0;
    if (fprintf(stream, "{\"scenario\":\"scenario.stage-a.%s\",\"completed\":%s,"
        "\"fallbackEntries\":%u,\"routes\":[", names[check.scenario],
        check.completed ? "true" : "false", (unsigned)check.fallback_entries) < 0) return 0;
    if (check.fallback_entries && fprintf(stream,
        "{\"routeId\":\"route.terminal-handoff\",\"reason\":\"future-flow\",\"count\":%u}",
        (unsigned)check.fallback_entries) < 0) return 0;
    return fputs("]}\n", stream) >= 0 && fflush(stream) == 0;
}

#include "app.h"
#include "protocol/protocol.h"
#include <stdlib.h>
#include <string.h>
typedef struct SvAppOperation {
    struct SvAppOperation *parent;
    void (*fallback)(void *);
    void *fallback_context;
    int failed;
} SvAppOperation;
struct SvApp {
    SvSession *session;
    SvProtocol *protocol;
    SvAppView view;
    SvAlertSink sink;
    SvAlertOptions options;
    SvFlushPolicy flush;
    SvAttention attention;
    int busy;
    SvAppOperation *operation;
    unsigned operation_depth;
    SvInputRouter input;
    SvInputBindings bindings;
    SvKeymap keymap;
    SvQuantity quantity;
    SvCommand command;
    bool command_pending, master_commands;
    SvPresentationObserver observer;
    SvMessageSink message_sink;
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
SvResult sv_app_set_message_sink(SvApp *app, SvMessageSink sink)
{
    if (app->busy) return SV_BUSY;
    if (app->message_sink.reset) app->message_sink.reset(app->message_sink.context);
    app->message_sink = sink;
    return SV_OK;
}
static void release_session(SvApp *app)
{
    if (app->message_sink.reset) app->message_sink.reset(app->message_sink.context);
    sv_protocol_destroy(app->protocol); app->protocol = NULL;
    sv_session_destroy(app->session); app->session = NULL;
    app->input = (SvInputRouter){0};
    app->keymap.prefix = app->keymap.last_direction = 0;
    sv_quantity_reset(&app->quantity);
    app->command_pending = app->master_commands = false;
    app->command = (SvCommand){0};
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
        app->sink = sink;
        app->view.reason = SV_CLOSED;
        sv_keymap_init(&app->keymap, false);
    }
    return app;
}
SvResult sv_app_destroy(SvApp *app)
{
    if (!app) return SV_OK;
    if (app->busy || app->operation_depth) return SV_BUSY;
    release_session(app); free(app);
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
SvAppView sv_app_view(const SvApp *app)
{
    SvAppView view = app->view;
    view.quantity_active = app->quantity.active;
    view.quantity_completed = app->quantity.completed;
    return view;
}
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
SvResult sv_app_keymap_preferences(SvApp *app, const SvPreferences *preferences,
                                   size_t *cursor, bool rebuild)
{
    if (app->busy) return SV_BUSY;
    if (!preferences || !cursor) return SV_INVALID;
    sv_keymap_preferences(&app->keymap, preferences, cursor);
    if (rebuild) sv_keymap_rebuild(&app->keymap);
    return SV_OK;
}
SvResult sv_app_set_master_commands(SvApp *app, uint64_t generation, bool enabled)
{
    SvResult result = accepts(app, generation);
    if (result == SV_OK) app->master_commands = enabled;
    return result;
}
SvResult sv_app_take_command(SvApp *app, uint64_t generation, SvCommand *command)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (!command) return SV_INVALID;
    if (!app->command_pending) return SV_WAITING;
    *command = app->command;
    app->command_pending = false;
    return SV_OK;
}
SvResult sv_app_begin_quantity(SvApp *app, uint64_t generation,
                               int32_t predef, int32_t maximum, bool all_shortcut)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (app->quantity.active || app->quantity.completed || app->command_pending || app->view.paused ||
        app->view.request.pending || app->view.context != SV_CONTEXT_GAME) return SV_BUSY;
    sv_quantity_begin(&app->quantity, predef, maximum, all_shortcut);
    app->keymap.prefix = 0;
    app->input.context = app->view.context = SV_CONTEXT_QUANTITY;
    return SV_OK;
}
SvQuantity sv_app_quantity_view(const SvApp *app, uint64_t generation)
{
    return app->view.active && app->view.generation == generation ?
        app->quantity : (SvQuantity){0};
}
SvResult sv_app_quantity_text(SvApp *app, uint64_t generation, const char *text)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (!text) return SV_INVALID;
    if (!app->quantity.active || app->view.paused || app->view.request.pending) return SV_BUSY;
    SvTextResult inserted = sv_quantity_text(&app->quantity, text);
    return inserted == SV_TEXT_OK ? SV_OK :
        inserted == SV_TEXT_TRUNCATED ? SV_INPUT_OVERFLOW : SV_INVALID;
}
SvResult sv_app_quantity_edit(SvApp *app, uint64_t generation, SvEditKey key)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (!app->quantity.active || app->view.paused || app->view.request.pending) return SV_BUSY;
    sv_quantity_edit(&app->quantity, key);
    return SV_OK;
}
SvResult sv_app_take_quantity(SvApp *app, uint64_t generation, int32_t *value)
{
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (!value) return SV_INVALID;
    if (app->view.request.pending) return SV_BUSY;
    if (!sv_quantity_take(&app->quantity, value)) return SV_WAITING;
    app->input.context = app->view.context = SV_CONTEXT_GAME;
    return SV_OK;
}
SvResult sv_app_set_reporting_audio(SvApp *app, uint64_t generation,
                                    const SvSettingsAudioSnapshot *audio)
{
    SvResult result = accepts(app, generation);
    return result == SV_OK ? sv_protocol_set_reporting_audio(app->protocol, audio) : result;
}
SvResult sv_app_send_font(SvApp *app, uint64_t generation)
{
    SvResult result = accepts(app, generation);
    return result == SV_OK ? sv_protocol_send_font(app->protocol) : result;
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
    if (app->view.paused || app->view.context != SV_CONTEXT_GAME || app->view.request.pending ||
        app->quantity.active || app->quantity.completed) return SV_BUSY;
    if (!key || key == 27 || key == '-' || key == ' ' || key == 13) return SV_INVALID;
    result = sv_protocol_raw_key(app->protocol, key);
    if (result == SV_OUTPUT_OVERFLOW) fail_session(app, result);
    return result;
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
SvResult sv_app_bind_physical(SvApp *app, const unsigned char *bytes, size_t size,
                              unsigned char action, SvMacroKind kind)
{
    if (app->busy) return SV_BUSY;
    return sv_input_bind_physical(&app->bindings, bytes, size, action, kind);
}
/* Baseline c-cmd.c:process_command ownership, not its implementations. Both
 * sound/no-sound builds own Ctrl-C/N/U locally; RETRY_LOGIN changes the quit
 * implementation, not its classification. Later executors take these results. */
static bool local_command(const SvApp *app, unsigned char command)
{
    if (command == ']') return app->master_commands;
    if (command && strchr(" )+;.,gMLsSR<>ocBDied$wtxkK{}HjqrauzFEAfvbGmU*(lIC~|'@#?:P_pWVQ=\"%&h/!", command))
        return true;
    switch (command) {
    case 7: case 15: case 16: case 17: case 18: case 19: case 20:
    case 9: case 21: case 3: case 14:
        return true;
    default: return false;
    }
}
static SvResult gameplay_key(SvApp *app, uint64_t generation, unsigned char key)
{
    if (app->quantity.active) {
        sv_quantity_key(&app->quantity, key);
        return SV_OK;
    }
    if (app->quantity.completed || app->command_pending) return SV_BACKPRESSURE;
    SvKeymap before = app->keymap;
    SvCommand command;
    SvResult result = sv_keymap_key(&app->keymap, key, &command);
    if (result != SV_OK) return result;
    if (command.command == 27 || command.command == '-' || command.command == 13)
        return SV_OK;
    if (local_command(app, command.command)) {
        app->command = command;
        app->command_pending = true;
        return SV_OK;
    }
    result = sv_app_raw_key(app, generation, command.command);
    if (result == SV_BACKPRESSURE) app->keymap = before;
    return result;
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
    result = !prompt && (app->quantity.active || app->quantity.completed) ?
        sv_input_physical_field(&app->bindings, bytes, size, &key) :
        sv_input_physical(&app->bindings, bytes, size, prompt, &key);
    if (result != SV_OK) return result;
    /* inkey normalizes literals after expansion. C053 owns macro WAIT96;
     * the single-byte binding API rejects it as an action. */
    if (key == '`') key = 27;
    if (prompt) {
        /* The request owner receives exactly one resolved byte; a macro's
         * action is not matched again as a fresh trigger. */
        return sv_app_key(app, generation, app->view.request.sequence, key);
    }
    return gameplay_key(app, generation, key);
}
SvResult sv_app_accept_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key)
{
    uint64_t started = now(app);
    SvResult result = accepts(app, generation);
    if (result != SV_OK) return result;
    if (app->view.paused) return SV_BUSY;
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
        if (decoded.kind == SV_CHANGE_MESSAGE && app->message_sink.deliver) {
            SvMessage message = decoded.message;
            message.sequence = change.message_occurrence;
            if (!app->message_sink.deliver(app->message_sink.context,
                                           app->view.generation, &message))
                app->view.executor_failed = 1; /* Optional file effect, keep playing. */
        }
        if (decoded.kind == SV_CHANGE_CONFIRM) {
            step.result = sv_input_confirm(&app->input, decoded.confirmed_command);
            if (step.result == SV_EVENT_OVERFLOW) { fail_session(app, step.result); break; }
            step.result = SV_OK; /* Unowned confirms are discarded, as before macro wait. */
        }
        if (decoded.kind == SV_CHANGE_NOOP) continue;
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

int sv_app_run(SvApp *app, int (*run)(SvApp *, void *), void *context,
               void (*fallback)(void *), void *fallback_context)
{
    if (app->busy || !run || app->operation_depth == 8) {
        if (app->operation) app->operation->failed = 1;
        return 0;
    }
    SvAppOperation operation = {app->operation, fallback, fallback_context, 0};
    app->operation = &operation;
    ++app->operation_depth;
    int success = run(app, context) && !operation.failed;
    --app->operation_depth;
    app->operation = operation.parent;
    if (!success && app->operation) app->operation->failed = 1;
    return success;
}
SvResult sv_app_terminal_fallback(SvApp *app)
{
    int busy = app->busy;
    app->busy = 1;
    for (SvAppOperation *operation = app->operation; operation;
         operation = operation->parent) {
        if (operation->fallback) operation->fallback(operation->fallback_context);
    }
    app->busy = busy;
    return SV_INVALID; /* No terminal adapter is linked or permitted. */
}

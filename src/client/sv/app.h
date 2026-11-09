#ifndef SV_APP_H
#define SV_APP_H
#include "session/alerts.h"
#include "settings/audio.h"
#include "result.h"
#include "input/input.h"
#include "input/keymap.h"
#include "input/quantity.h"
#include "protocol/protocol.h"
#include "ui/flush-policy.h"
typedef struct SvApp SvApp;
typedef struct {
    uint64_t generation;
    uint64_t revision; /* Coherent presentation revision, including local outcomes. */
    SvStatus status;
    SvMessages messages;
    SvPingTelemetry ping;
    SvKeyRequest request;
    SvInputContext context;
    uint32_t server_flags[4];
    uint64_t pause_sequence, flush_sequence;
    uint64_t flush_due_ms;
    int paused;
    int active, executor_failed;
    int quantity_active, quantity_completed;
    SvResult reason;
} SvAppView;
/* Required control effects are delivered in packet order, before decoding the
 * next packet. submit must draw and present the supplied coherent view without
 * re-entering app. resume notifies the caller to schedule its deferred incoming
 * work after returning; it must not re-enter app either. Each acknowledged
 * occurrence notifies resume; another queued PAUSE may still own input. */
typedef struct {
    void *context;
    bool (*submit)(void *context, SvAppView view, bool pause);
    void (*resume)(void *context, uint64_t generation, uint64_t pause_sequence);
} SvControlSink;
SvResult sv_app_set_control_sink(SvApp *app, SvControlSink sink);
typedef enum { SV_PRESENT_HP, SV_PRESENT_MESSAGE, SV_PRESENT_REQUEST,
               SV_PRESENT_INPUT, SV_PRESENT_LIFECYCLE, SV_PRESENT_PING,
               SV_PRESENT_CONTROL } SvPresentationOrigin;
typedef struct {
    SvPresentationOrigin origin;
    uint64_t generation, revision, started_ns, occurrence;
    int interactive;
} SvPresentationEvent;
/* Optional owner-thread observation. Callbacks must not re-enter the app.
 * Clock is monotonic nanoseconds; native measurement uses SDL_GetTicksNS. */
typedef struct {
    void *context;
    uint64_t (*now)(void *context);
    void (*changed)(void *context, SvPresentationEvent event);
} SvPresentationObserver;
SvResult sv_app_observe(SvApp *app, SvPresentationObserver observer);
/* Explicit once-per-applied-message effect delivery, independent of recall/UI.
 * Callbacks run on the owner thread and must not re-enter the app. The sink is
 * borrowed until replaced/destroyed; reset cancels its session-local chain. */
typedef struct {
    void *context;
    int (*deliver)(void *context, uint64_t generation, const SvMessage *message);
    void (*reset)(void *context);
} SvMessageSink;
SvResult sv_app_set_message_sink(SvApp *app, SvMessageSink sink);
SvResult sv_app_set_reporting_audio(SvApp *app, uint64_t generation,
                                    const SvSettingsAudioSnapshot *audio);
SvResult sv_app_send_font(SvApp *app, uint64_t generation);
typedef struct { size_t processed, pending_bytes; SvResult result; } SvStep;
/* All entry points run on the owner thread. Views are copies. */
SvApp *sv_app_create(SvAlertSink sink);
SvResult sv_app_destroy(SvApp *app);
SvResult sv_app_open(SvApp *app, const int version[6]);
SvResult sv_app_close(SvApp *app);
SvResult sv_app_close_reason(SvApp *app, SvResult reason);
SvResult sv_app_set_alerts(SvApp *app, SvAlertOptions options, SvAttention attention);
SvResult sv_app_set_flush_options(SvApp *app, SvFlushOptions options);
SvResult sv_app_frame(SvApp *app, uint64_t generation, uint64_t now_ms);
SvAppView sv_app_view(const SvApp *app);
SvResult sv_app_set_character_setup(SvApp *app, uint64_t generation,
                                    const SvContactSetup *setup);
/* Borrowed for this generation; the session owns the server-supplied data. */
const SvContactSetup *sv_app_character_setup(const SvApp *app, uint64_t generation);
SvResult sv_app_receive(SvApp *app, uint64_t generation, const void *bytes, size_t size);
/* Completion from an external producer, delivered on the owner thread.
 * BACKPRESSURE/BUSY retain input for retry. All other outcomes consume and zero it.
 * release frees storage only; it must not re-enter the application. */
typedef struct {
    uint64_t generation;
    const void *bytes;
    size_t size;
    void *owner;
    void (*release)(void *owner);
} SvOwnedBytes;
SvResult sv_app_receive_owned(SvApp *app, SvOwnedBytes *input);
SvStep sv_app_step(SvApp *app, size_t budget);
size_t sv_app_receive_capacity(const SvApp *app);
SvOutput sv_app_take_output(SvApp *app, uint64_t generation, void *bytes, size_t capacity);
/* Explicit event consumer, independent of UI lifetime. Copies and acknowledges one occurrence. */
SvResult sv_app_take_message(SvApp *app, uint64_t generation, SvMessage *message);
/* A macro interaction registers as the owner before waiting; stale owners cannot drain it. */
SvResult sv_app_begin_confirmation(SvApp *app, uint64_t generation, uint64_t *owner);
SvResult sv_app_take_confirmation(SvApp *app, uint64_t generation,
                                  uint64_t owner, unsigned char *command);
SvResult sv_app_end_confirmation(SvApp *app, uint64_t generation, uint64_t owner);
SvResult sv_app_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key);
/* Profile owner supplies its ordered preference cursor explicitly. Initial
 * loading follows init; manual reload applies occurrences then rebuilds. */
SvResult sv_app_keymap_preferences(SvApp *app, const SvPreferences *preferences,
                                   size_t *cursor, bool rebuild);
SvResult sv_app_set_master_commands(SvApp *app, uint64_t generation, bool enabled);
/* A resolved local command remains owned by this generation until its semantic
 * executor takes it. Raw fallback never substitutes for a missing executor. */
SvResult sv_app_take_command(SvApp *app, uint64_t generation, SvCommand *command);
SvResult sv_app_begin_quantity(SvApp *app, uint64_t generation,
                               int32_t predef, int32_t maximum, bool all_shortcut);
SvResult sv_app_quantity_text(SvApp *app, uint64_t generation, const char *text);
SvResult sv_app_quantity_edit(SvApp *app, uint64_t generation, SvEditKey key);
SvResult sv_app_take_quantity(SvApp *app, uint64_t generation, int32_t *value);
/* Copy for the current interaction's presentation; empty for stale callers. */
SvQuantity sv_app_quantity_view(const SvApp *app, uint64_t generation);
/* Caller invokes this only for an unhandled resolved gameplay command. Reserved
 * no-op keys and the local space action cannot become server packets. LF is
 * allowed here for baseline raw bypass; normal keymap LF resolves to CR. */
SvResult sv_app_raw_key(SvApp *app, uint64_t generation, unsigned char key);
/* Caller supplies monotonic milliseconds and last successful socket send. */
SvResult sv_app_keepalive(SvApp *app, uint64_t generation,
                          uint64_t now_ms, uint64_t last_sent_ms);
SvResult sv_app_set_ping_clock(SvApp *app, SvPingClock clock);
SvResult sv_app_send_ping(SvApp *app, uint64_t generation);
SvResult sv_app_ack_pause(SvApp *app, uint64_t generation, uint64_t sequence);
/* Bindings outlive a session. Accepted input belongs to its generation/request.
 * Matching occurs once at acceptance, dispatch is bounded and independent of UI. */
SvResult sv_app_bind_macro(SvApp *app, unsigned char trigger, unsigned char action, SvMacroKind kind);
SvResult sv_app_bind_physical(SvApp *app, const unsigned char *bytes, size_t size,
                              unsigned char action, SvMacroKind kind);
SvResult sv_app_physical(SvApp *app, uint64_t generation, const unsigned char *bytes, size_t size);
SvResult sv_app_accept_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key);
typedef struct { size_t dispatched, stale, pending; SvResult result; } SvInputStep;
SvInputStep sv_app_dispatch_input(SvApp *app, size_t budget);
/* Synchronous owner-thread operation. Active calls protect app lifetime; failed
 * nested calls fail their parent. At most eight operations may be active.
 * Optional fallback callbacks observe each denied handoff during their scope;
 * they must not re-enter the app. */
int sv_app_run(SvApp *app, int (*run)(SvApp *, void *), void *context,
               void (*fallback)(void *), void *fallback_context);
/* Sole terminal handoff boundary. Records BEFORE any future adapter dispatch.
 * Native-only SV denies every attempt, including stale-generation attempts. */
SvResult sv_app_terminal_fallback(SvApp *app);
#endif

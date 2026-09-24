#ifndef SV_APP_H
#define SV_APP_H
#include "session/alerts.h"
#include "result.h"
#include "input/input.h"
#include "diagnostics/runtime.h"
#include "protocol/protocol.h"
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
    SvResult reason;
} SvAppView;
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
typedef struct { size_t processed, pending_bytes; SvResult result; } SvStep;
typedef struct { int disable_flush, thin_down_flush; } SvFlushOptions;
/* All entry points run on the owner thread. Views are copies. */
SvApp *sv_app_create(SvAlertSink sink);
SvResult sv_app_destroy(SvApp *app);
SvResult sv_app_open(SvApp *app, const int version[6]);
SvResult sv_app_close(SvApp *app);
SvResult sv_app_set_alerts(SvApp *app, SvAlertOptions options, SvAttention attention);
SvResult sv_app_set_flush_options(SvApp *app, SvFlushOptions options);
SvResult sv_app_frame(SvApp *app, uint64_t generation, uint64_t now_ms);
SvAppView sv_app_view(const SvApp *app);
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
SvResult sv_app_take_confirmation(SvApp *app, uint64_t generation, unsigned char *command);
SvResult sv_app_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key);
/* Caller invokes this only for an unmapped gameplay command. Reserved local
 * no-op keys are rejected and cannot become server packets. */
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
/* Synchronous scenario scope: only its checked return can complete it. Nested
 * failures poison the parent. Session teardown never clears the collector.
 * Callbacks verify required events/replies before returning success; they must
 * not destroy the app. There is deliberately no async completion API. */
SvRuntimeCheck sv_app_check_run(SvApp *app, SvRuntimeScenario scenario,
                              int (*run)(SvApp *, void *), void *context);
/* Sole terminal handoff boundary. Records BEFORE any future adapter dispatch.
 * Native-only SV denies every attempt, including stale-generation attempts. */
SvResult sv_app_terminal_fallback(SvApp *app);
#endif

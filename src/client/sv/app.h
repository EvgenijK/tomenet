#ifndef SV_APP_H
#define SV_APP_H
#include "alerts.h"
#include "result.h"
#include "input.h"
typedef struct SvApp SvApp;
typedef struct {
    uint64_t generation;
    SvStatus status;
    SvMessages messages;
    SvKeyRequest request;
    SvInputContext context;
    int active, executor_failed;
    SvResult reason;
} SvAppView;
typedef struct { size_t processed, pending_bytes; SvResult result; } SvStep;
/* All entry points run on the owner thread. Views are copies. */
SvApp *sv_app_create(SvAlertSink sink);
SvResult sv_app_destroy(SvApp *app);
SvResult sv_app_open(SvApp *app, const int version[6]);
SvResult sv_app_close(SvApp *app);
SvResult sv_app_set_alerts(SvApp *app, SvAlertOptions options, SvAttention attention);
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
SvResult sv_app_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key);
/* Bindings outlive a session. Accepted input belongs to its generation/request.
 * Matching occurs once at acceptance, dispatch is bounded and independent of UI. */
SvResult sv_app_bind_macro(SvApp *app, unsigned char trigger, unsigned char action, SvMacroKind kind);
SvResult sv_app_accept_key(SvApp *app, uint64_t generation, uint64_t sequence, unsigned char key);
typedef struct { size_t dispatched, stale, pending; SvResult result; } SvInputStep;
SvInputStep sv_app_dispatch_input(SvApp *app, size_t budget);
#endif

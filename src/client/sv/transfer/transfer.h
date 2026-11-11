#ifndef SV_TRANSFER_H
#define SV_TRANSFER_H
#include "result.h"
#include "lua/reload.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SV_TRANSFER_FRAME_CAPACITY 1030
#define SV_TRANSFER_OUTPUT_CAPACITY (32 * 1024)
typedef struct SvTransfer SvTransfer;
typedef void (*SvTransferDiagnostic)(void *context, const char *message);
typedef struct {
    uint64_t generation;
    size_t active, accepted_init, completed_end, reloads;
    size_t failures, runtime_failures, guide_failures;
    bool has_completion;
    SvLuaCompletion completion;
} SvTransferSummary;
/* library is the baseline ANGBAND_DIR root, not the profile/shared user root.
 * Lua is borrowed; caller begins it with real features before dispatch. */
SvTransfer *sv_transfer_create(const char *library);
void sv_transfer_destroy(SvTransfer *);
/* Recipient runs synchronously on main thread and must not reenter. */
void sv_transfer_set_diagnostic(SvTransfer *, SvTransferDiagnostic, void *context);
SvResult sv_transfer_begin(SvTransfer *, uint64_t generation, const int version[6],
                           SvLuaReload *lua, bool no_updates);
void sv_transfer_close(SvTransfer *, uint64_t generation);
/* One frame. WAITING/BACKPRESSURE consumes zero; OK consumes one complete frame.
 * Includes PKT_FILE byte. Non-OK fatal frames consume zero. Own bounded output
 * reserves replies before any file/Lua effect. */
SvResult sv_transfer_receive(SvTransfer *, uint64_t generation,
                             const void *bytes, size_t available, size_t *consumed);
/* Sender progress must be called on ticks even without incoming packets. */
SvResult sv_transfer_pump(SvTransfer *, uint64_t generation);
/* NULL filename cancels the request without effects; maximum 30 filename bytes. */
SvResult sv_transfer_upload(SvTransfer *, uint64_t generation, const char *filename);
/* Atomic queue drain: OUTPUT_TOO_SMALL leaves bytes pending, size is required. */
SvOutput sv_transfer_take_output(SvTransfer *, void *bytes, size_t capacity);
size_t sv_transfer_output_size(const SvTransfer *);
SvTransferSummary sv_transfer_summary(const SvTransfer *);
#endif

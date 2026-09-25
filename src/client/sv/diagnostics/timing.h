#ifndef SV_TIMING_H
#define SV_TIMING_H
#include "ui/ui.h"
#define SV_TIMING_PENDING 128
/* Bounded diagnostics only: identifiers/timestamps, never prompt/message payloads. */
typedef struct {
    SvPresentationEvent event;
    uint64_t first_revision;
    unsigned coalesced;
} SvTimingEntry;
typedef struct {
    SvTimingEntry pending[SV_TIMING_PENDING];
    size_t count;
    unsigned samples, violations, invalid, frames;
} SvTiming;
SvPresentationObserver sv_timing_observer(SvTiming *timing);
/* Same native draw/present path as normal shell. No readback in timed interval.
 * Records only successfully submitted snapshots; failures retain pending work. */
bool sv_timing_submit(SvTiming *timing, SvUi *ui, SvAppView view);
#endif

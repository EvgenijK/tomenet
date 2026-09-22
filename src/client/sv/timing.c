#include "timing.h"
#include <stdio.h>
static uint64_t clock_ns(void *context)
{
    (void)context;
    return SDL_GetTicksNS();
}
static void changed(void *context, SvPresentationEvent event)
{
    SvTiming *timing = context;
    /* Latest projections coalesce, but never renew the original deadline.
     * Message occurrences retain separate evidence and deadlines. */
    if (event.origin != SV_PRESENT_MESSAGE) {
        for (size_t i = 0; i < timing->count; ++i) {
            SvTimingEntry *entry = &timing->pending[i];
            if (entry->event.generation == event.generation && entry->event.origin == event.origin) {
                uint64_t first = entry->event.started_ns;
                int interactive = entry->event.interactive && event.interactive;
                entry->event = event;
                entry->event.started_ns = first;
                entry->event.interactive = interactive;
                ++entry->coalesced;
                return;
            }
        }
    }
    if (timing->count == SV_TIMING_PENDING) {
        ++timing->invalid;
        fprintf(stderr, "SV timing invalid=capacity generation=%llu revision=%llu\n",
                (unsigned long long)event.generation, (unsigned long long)event.revision);
        return;
    }
    timing->pending[timing->count++] = (SvTimingEntry){event, event.revision, 1};
}
SvPresentationObserver sv_timing_observer(SvTiming *timing)
{
    return (SvPresentationObserver){timing, clock_ns, changed};
}
bool sv_timing_submit(SvTiming *timing, SvUi *ui, SvAppView view)
{
    if (SDL_GetRenderTarget(ui->renderer)) return SDL_SetError("Timing requires the native window output");
    if (!sv_ui_submit(ui, view)) return false;
    uint64_t submitted = SDL_GetTicksNS();
    ++timing->frames;
    size_t remaining = 0;
    for (size_t i = 0; i < timing->count; ++i) {
        SvTimingEntry entry = timing->pending[i];
        SvPresentationEvent event = entry.event;
        if (event.generation != view.generation) {
            ++timing->invalid;
            fprintf(stderr, "SV timing invalid=unsubmitted-generation revision=%llu\n",
                    (unsigned long long)event.revision);
            continue;
        }
        if (event.revision > view.revision) { timing->pending[remaining++] = entry; continue; }
        if (event.origin == SV_PRESENT_MESSAGE) {
            int visible = !event.occurrence && !view.messages.count;
            for (size_t row = 0; row < view.messages.count; ++row)
                if (view.messages.lines[row].sequence == event.occurrence) visible = 1;
            if (!visible) {
                ++timing->invalid;
                fprintf(stderr, "SV timing invalid=message-not-in-frame occurrence=%llu revision=%llu\n",
                        (unsigned long long)event.occurrence, (unsigned long long)event.revision);
                continue;
            }
        }
        uint64_t elapsed = submitted - event.started_ns;
        unsigned budget = event.interactive ? 50 : 20;
        int violation = elapsed > (uint64_t)budget * 1000000;
        ++timing->samples;
        timing->violations += violation;
        printf("SV timing clock=SDL_GetTicksNS frame=%u generation=%llu first_revision=%llu revision=%llu submitted_revision=%llu origin=%d occurrence=%llu class=%s elapsed_ms=%.3f budget_ms=%u coalesced=%u violation=%s\n",
               timing->frames, (unsigned long long)event.generation,
               (unsigned long long)entry.first_revision, (unsigned long long)event.revision,
               (unsigned long long)view.revision, event.origin, (unsigned long long)event.occurrence,
               event.interactive ? "interactive" : "urgent", elapsed / 1000000.0,
               budget, entry.coalesced, violation ? "true" : "false");
    }
    timing->count = remaining;
    return true;
}

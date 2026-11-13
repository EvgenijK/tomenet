#include "ui/status.h"
#include <stdio.h>
void sv_status_text(SvStatus status, char *text, size_t capacity)
{
    if (!status.available) { snprintf(text, capacity, "HP awaiting server update"); return; }
    snprintf(text, capacity, "HP %d / %d%s%s%s", status.current, status.maximum,
             status.boosted ? "  boosted" : "", status.bar ? "  bar" : "",
             status.drain ? "  drain" : "");
}

unsigned sv_status_prepare(SvStatusCache *cache, SvAppView view, SvPresentationKey key)
{
    unsigned changed = 0;
    if (!cache->valid || cache->generation != view.generation || cache->revision != view.status.revision) {
        cache->generation = view.generation;
        cache->revision = view.status.revision;
        cache->status = view.status;
        sv_status_text(view.status, cache->text, sizeof(cache->text));
        changed |= SV_TEXT_CHANGED;
    }
    if (!cache->valid || cache->key.width != key.width || cache->key.height != key.height ||
        cache->key.scale != key.scale || cache->key.font_revision != key.font_revision) {
        cache->key = key;
        changed |= SV_LAYOUT_CHANGED;
    }
    cache->valid = 1;
    return changed;
}

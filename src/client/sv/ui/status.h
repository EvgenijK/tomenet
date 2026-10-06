#ifndef SV_STATUS_H
#define SV_STATUS_H
#include "app.h"
void sv_status_text(SvStatus status, char *text, size_t capacity);
typedef struct {
    int width, height;
    float scale;
    uint64_t font_revision;
} SvPresentationKey;
typedef struct {
    uint64_t generation, revision;
    int valid;
    SvPresentationKey key;
    SvStatus status;
    char text[160];
} SvStatusCache;
enum { SV_TEXT_CHANGED = 1, SV_LAYOUT_CHANGED = 2 };
/* Zero initialize per application lifetime. Returns required presentation work.
 * Owns copies only; geometry/font changes don't reformat semantic status text. */
unsigned sv_status_prepare(SvStatusCache *cache, SvAppView view, SvPresentationKey key);
#endif

#ifndef SV_UI_H
#define SV_UI_H
#include "font.h"
#include "status.h"
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SvFont *font;
    uint64_t font_revision;
    SvStatusCache status;
    float logical_width;
} SvUi;
/* Draw only. The shell owns submission, allowing test-only readback before present.
 * Replace font => increment font_revision; reset this object for a new App lifetime. */
bool sv_ui_draw(SvUi *ui, SvAppView view);
#endif

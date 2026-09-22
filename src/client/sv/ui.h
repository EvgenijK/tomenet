#ifndef SV_UI_H
#define SV_UI_H
#include "font.h"
#include "status.h"
#include "message-text.h"
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SvFont *font;
    uint64_t font_revision;
    SvStatusCache status;
    SvMessageCache messages;
    int message_cell;
    size_t message_lengths[SV_MESSAGE_LINES];
    float logical_width;
    /* Synthetic virtual-DPI fixture only; zero uses the actual display scale. */
    float fixture_scale;
    uint64_t request_generation, request_sequence;
    SvMessageText prompt;
} SvUi;
/* Draw only. The shell owns submission, allowing test-only readback before present.
 * Replace font => increment font_revision; reset this object for a new App lifetime. */
bool sv_ui_draw(SvUi *ui, SvAppView view);
bool sv_ui_submit(SvUi *ui, SvAppView view);
float sv_ui_scale(const SvUi *ui);
/* Discard derived surface state; resources and semantic state have other owners. */
void sv_ui_rebuild(SvUi *ui);
bool sv_ui_event(SvUi *ui, const SDL_Event *event);
#endif

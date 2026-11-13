#ifndef SV_FONT_EDITOR_H
#define SV_FONT_EDITOR_H
#include "ui/font-selection.h"
#include "ui/resource-preparation.h"
#include "settings/map-font.h"
#include "settings/graphics.h"
#include "session/map-effects.h"
typedef struct SvFontEditor SvFontEditor;
SvFontEditor *sv_font_editor_create(SvFontSelection *,const char *user,const char *library,
    const SvSettingsConfig *,const SvMapFont *);
void sv_font_editor_destroy(SvFontEditor *);
/* Optional production composition view of the active stack. Borrowed owners
 * outlive this editor; the resource chooser remains independent of live C013. */
void sv_font_editor_rendering(SvFontEditor *,SvGraphics *const *,SvMapEffects *);
typedef enum { SV_EDITOR_RESOURCE_PREVIEW,SV_EDITOR_RESOURCE_CANCEL,
    SV_EDITOR_RESOURCE_SAVED } SvFontEditorResourceAction;
typedef bool (*SvFontEditorResourcePrepare)(void *,const SvSettingsConfig *,
    SvFontRole,SvFontEditorResourceAction);
void sv_font_editor_resource_prepare(SvFontEditor *,SvFontEditorResourcePrepare,void *);
/* F6 opens a nonblocking resource preview. Events are consumed only while open;
 * network/lifecycle polling continues in the native caller. */
bool sv_font_editor_event(SvFontEditor *,const SDL_Event *);
bool sv_font_editor_open(const SvFontEditor *);
SvFontRole sv_font_editor_role(const SvFontEditor *);
bool sv_font_editor_draw(SvFontEditor *,SDL_Renderer *);
#endif

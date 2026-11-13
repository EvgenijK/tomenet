#ifndef SV_FONT_SELECTION_H
#define SV_FONT_SELECTION_H
#include "ui/font.h"
#include "settings/config.h"

typedef enum { SV_FONT_TEXT, SV_FONT_MAP } SvFontRole;
typedef enum {
    SV_FONT_SELECTION_OK, SV_FONT_SELECTION_INVALID, SV_FONT_SELECTION_MISSING,
    SV_FONT_SELECTION_IO, SV_FONT_SELECTION_LOAD, SV_FONT_SELECTION_PROFILE,
    SV_FONT_SELECTION_APPLY, SV_FONT_SELECTION_CONFLICT, SV_FONT_SELECTION_MEMORY
} SvFontSelectionResult;
typedef struct SvFontSelection SvFontSelection;
/* Called after complete loading/validation, before the infallible active swap.
 * A false result must leave the consumer's active state unchanged. */
typedef bool (*SvFontSelectionPrepare)(void *, SvFontRole, SvFont *);
SvFontSelection *sv_font_selection_create(const char *user_root, const char *library,
                                          const SvSettingsConfig *config);
void sv_font_selection_destroy(SvFontSelection *);
void sv_font_selection_prepare(SvFontSelection *, SvFontSelectionPrepare, void *);
SvFontSelectionResult sv_font_selection_preview(SvFontSelection *, SvFontRole,
                                                const char *font, const char *profile);
SvFontSelectionResult sv_font_selection_cancel(SvFontSelection *, SvFontRole);
SvFontSelectionResult sv_font_selection_save(SvFontSelection *, SvFontRole);
SvFont *sv_font_selection_font(const SvFontSelection *, SvFontRole);
const char *sv_font_selection_requested(const SvFontSelection *, SvFontRole);
const char *sv_font_selection_profile(const SvFontSelection *, SvFontRole);
const char *sv_font_selection_error(const SvFontSelection *);
bool sv_font_selection_dirty(const SvFontSelection *, SvFontRole);
unsigned sv_font_selection_generation(const SvFontSelection *, SvFontRole);
bool sv_font_selection_fallback(const SvFontSelection *, SvFontRole);
#endif

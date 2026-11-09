#ifndef SV_FONT_H
#define SV_FONT_H
#include <SDL3/SDL.h>
#include <stdint.h>
#include "files/files.h"

typedef struct SvFont SvFont;
typedef struct {
    int width, height, ascent;
    uint32_t first_id, last_id, default_id;
    unsigned glyph_count;
    bool has_default, pcf;
} SvFontMetrics;
typedef struct {
    uint32_t requested_id, rendered_id;
    bool present, used_default, replacement;
} SvFontGlyphInfo;
/* Built-ins: sv-pcf-encoded-v1 and sv-ttf-ascii-v1. An explicit profile file
 * starts with SVFONT1 and contains ID Unicode-scalar pairs (decimal or 0x...).
 * A native PCF profile file starts with SVPCF1, followed only by comments.
 * Profiles describe display correspondence, never a network charset. */
SvFont *sv_font_open_profile_file(const char *resolved_path, const char *profile);
const char *sv_font_profile(const SvFont *font);
bool sv_font_metrics(SvFont *font, SvFontMetrics *metrics);
bool sv_font_is_pcf(const SvFont *font);
bool sv_font_has_glyph(SvFont *font, uint32_t id);
/* Numeric IDs include zero/control bytes. ID 2 is the baseline solid cell;
 * IDs above 255 belong to graphics and are rejected. Returned surface is owned. */
SDL_Surface *sv_font_render_glyph(SvFont *font, uint32_t id, int width, int height,
                                 SDL_Color color, SvFontGlyphInfo *info);
/* Validate the actual resource's encoded glyphs through the production renderer. */
bool sv_font_validate(SvFont *font, unsigned *checked_glyphs);
typedef struct { unsigned discovered, valid, invalid, deferred_fon; } SvFontInventory;
typedef void (*SvFontInventoryCallback)(void *context,const SvFilePath *path,
    const SvFontMetrics *metrics,bool valid,unsigned checked_glyphs);
/* Explicit basic corpus validation. Concrete directory ownership is retained;
 * bundled resources are examined even when an overlay shadows them. FON is a
 * deferred Windows resource, not a successful Linux PCF/TTF validation. */
bool sv_font_validate_directory(const char *root,SvFileOwner owner,
    SvFontInventory *inventory,SvFontInventoryCallback callback,void *context);
/* Scoped Stage A UI profile: ASCII text, no game-byte charset inference. */
SvFont *sv_font_open(const char *user_root, const char *library);
SvFont *sv_font_open_selected(const char *user_root, const char *library, const char *requested);
/* Reopen an already resolved effective resource without another fallback search. */
SvFont *sv_font_open_file(const char *resolved_path);
SDL_Surface *sv_font_render_surface(SvFont *font, const char *text, float points,
                                    float output_scale, SDL_Color color, bool bold);
bool sv_font_measure(SvFont *font, const char *text, float points, float output_scale,
                     int *width, int *height, bool bold);
void sv_font_close(SvFont *font);
const char *sv_font_resource(const SvFont *font);
bool sv_font_draw(SvFont *font, SDL_Renderer *renderer, const char *text,
                  int x, int y, float output_scale, SDL_Color color);
int sv_font_cell_width(SvFont *font, float output_scale);
#endif

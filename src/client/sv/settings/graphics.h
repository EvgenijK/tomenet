#ifndef SV_GRAPHICS_H
#define SV_GRAPHICS_H
#include "settings/config.h"
#include "settings/map-font.h"
#include "ui/resource-preparation.h"
typedef struct SvGraphics SvGraphics;
/* Preferences must be a fresh font base. Success publishes graphics PRFs into
 * that candidate; failure leaves it unchanged. Active owner is caller-owned. */
SvGraphics *sv_graphics_prepare(const char *user,const char *library,
    const SvSettingsConfig *,SvPreferences *,const SvPreferenceContext *,
    int width,int height,char *error,size_t error_size);
/* Rebuild policy/size from active immutable sources; never reloads PRFs,
 * emits load effects or changes active on failure. Returned owner independent. */
SvGraphics *sv_graphics_reprepare(const SvGraphics *active,SvResourceFilter filter,
    int outline,int width,int height,char *error,size_t error_size);
void sv_graphics_destroy(SvGraphics *);
/* Call before renderer destruction or SDL device reset; CPU resources survive. */
void sv_graphics_reset_renderer(SvGraphics *);
void sv_graphics_cache_policy(SvGraphics *,bool disable_tile_cache,bool disable_tileset_cache);
bool sv_graphics_persistent_cache(const SvGraphics *);
int sv_graphics_mode(const SvGraphics *);
const char *sv_graphics_name(const SvGraphics *);
uint64_t sv_graphics_generation(const SvGraphics *);
const SvPreparedResource *sv_graphics_tile(SvGraphics *,uint32_t glyph,int width,int height);
/* Raw-picture extents are scaled from authored tile units; ordinary tile masks
 * never apply. The returned object is borrowed until another lookup/size change. */
const SvPreparedResource *sv_graphics_raw(SvGraphics *,unsigned id,int width,int height);
int sv_graphics_draw(SvGraphics *,SDL_Renderer *,uint32_t,const SDL_FRect *,SDL_Color,SDL_Color);
int sv_graphics_draw_raw(SvGraphics *,SDL_Renderer *,unsigned,int cell_width,int cell_height,
    float x,float y);
typedef struct {
    SvMapFont *map;
    const SvSettingsConfig *config;
    SvGraphics **active;
    int width,height;
    char error[256];
} SvGraphicsBootstrap;
/* Startup-only adapter: font -> graphics -> caller's own OPT. Graphics failure
 * retains font-only stack and requested config; diagnostic is never hidden. */
SvPreferenceResult sv_graphics_bootstrap(SvPreferences *,const SvPreferenceContext *,void *);
#endif

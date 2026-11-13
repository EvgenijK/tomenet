#ifndef SV_MAP_RENDERER_H
#define SV_MAP_RENDERER_H
#include "session/map.h"
#include "ui/font.h"
#include "ui/resource-preparation.h"
typedef struct {
    SDL_Rect bounds; /* Output pixels. One transform for every layer and hit. */
    int columns,rows,cell_width,cell_height;
    float output_scale;
} SvMapGeometry;
/* area is in logical UI units; output_scale maps these to output pixels. */
bool sv_map_geometry(SvMapGeometry *,SDL_FRect area,float output_scale,int rows,int font_width,int font_height);
bool sv_map_hit(const SvMapGeometry *,float logical_x,float logical_y,int *x,int *y);
SDL_FRect sv_map_cell_rect(const SvMapGeometry *,int x,int y);
typedef struct {
    void *context;
    /* 1 drawn, 0 font path, -1 failed; graphics owner retains actual resources. */
    int (*visual)(void *,SDL_Renderer *,const SvMapVisual *,const SDL_FRect *,SDL_Color);
    SDL_Color (*colour)(void *,unsigned char attribute,int x,int y,bool terrain);
    /* Read-only overlay projection. False means no effect at this cell. */
    bool (*overlay)(void *,int x,int y,SvMapVisual *);
} SvMapRenderProvider;
typedef struct SvMapRenderer SvMapRenderer;
SvMapRenderer *sv_map_renderer_create(void);
void sv_map_renderer_destroy(SvMapRenderer *);
/* Call before SDL renderer destruction/recreation, including device reset. */
void sv_map_renderer_reset(SvMapRenderer *);
/* Font is borrowed. Generation must change on font/profile reload. No frame
 * submission, semantic mutation, tick, PRF processing or network send here. */
bool sv_map_renderer_draw(SvMapRenderer *,SDL_Renderer *,const SvMap *,const SvMapGeometry *,
    SvFont *,uint64_t font_generation,SvResourceFilter pcf_filter,const uint32_t palette[16],
    const SvMapRenderProvider *);
#endif

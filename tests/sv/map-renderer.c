#include "session/map.h"
#include "ui/map-renderer.h"
#include <assert.h>
#include <stdio.h>
/* A decoded overlay value supplied to the production compositor, not an
 * alternate effects engine. Weather/effect timing belongs to its own owner. */
static bool overlay(void *context,int x,int y,SvMapVisual *out)
{
    if(x!=4 || y!=3)return false;
    *out=*(SvMapVisual *)context;return out->present;
}
static Uint32 pixel(SDL_Renderer *renderer,int x,int y)
{
    SDL_Rect rect={x,y,1,1};SDL_Surface *surface=SDL_RenderReadPixels(renderer,&rect);
    assert(surface);Uint8 r,g,b,a;
    assert(SDL_ReadSurfacePixel(surface,0,0,&r,&g,&b,&a));
    SDL_DestroySurface(surface);return ((Uint32)r<<16)|((Uint32)g<<8)|b;
}
static bool ink(SDL_Renderer *renderer,SDL_FRect rect)
{
    SDL_Rect bounds={(int)rect.x,(int)rect.y,(int)rect.w,(int)rect.h};
    SDL_Surface *surface=SDL_RenderReadPixels(renderer,&bounds);assert(surface);
    bool found=false;
    for(int y=0;y<surface->h;++y)for(int x=0;x<surface->w;++x) {
        Uint8 r,g,b,a;assert(SDL_ReadSurfacePixel(surface,x,y,&r,&g,&b,&a));
        if(r || g || b)found=true;
    }
    SDL_DestroySurface(surface);return found;
}
static uint64_t frame(SDL_Renderer *renderer,const SDL_Rect *bounds)
{
    SDL_Surface *surface=SDL_RenderReadPixels(renderer,bounds);assert(surface);
    uint64_t hash=UINT64_C(1469598103934665603);
    int bytes=surface->w*(int)SDL_BYTESPERPIXEL(surface->format);
    for(int y=0;y<surface->h;++y)for(int x=0;x<bytes;++x) {
        hash^=((unsigned char *)surface->pixels)[y*surface->pitch+x];hash*=UINT64_C(1099511628211);
    }
    SDL_DestroySurface(surface);return hash;
}
int main(void)
{
    assert(SDL_Init(SDL_INIT_VIDEO));
    SDL_Surface *surface=SDL_CreateSurface(1600,1200,SDL_PIXELFORMAT_RGBA32);assert(surface);
    SDL_Renderer *renderer=SDL_CreateSoftwareRenderer(surface);assert(renderer);
    SvFont *font=sv_font_open_file("lib/xtra/font/16x24x.pcf");assert(font);
    SvMap *map=sv_map_create(22,7);assert(map);
    SvMapRenderer *owner=sv_map_renderer_create();assert(owner);
    SvMapGeometry g;
    assert(sv_map_geometry(&g,(SDL_FRect){11,13,1000,600},1.25f,22,16,24));
    assert(g.bounds.w==66*g.cell_width && g.bounds.h==22*g.cell_height);
    int x=-1,y=-1;SDL_FRect cell=sv_map_cell_rect(&g,4,3);
    assert(sv_map_hit(&g,(cell.x+.5f)/1.25f,(cell.y+.5f)/1.25f,&x,&y) && x==4 && y==3);
    assert(!sv_map_hit(&g,(g.bounds.x+g.bounds.w)/1.25f,(cell.y+.5f)/1.25f,&x,&y));
    assert(!sv_map_geometry(&g,(SDL_FRect){0,0,10,10},1,22,16,24));
    assert(sv_map_geometry(&g,(SDL_FRect){11,13,1000,600},1.25f,22,16,24));
    uint32_t palette[16]={0,0xff0000,0x00ff00,0x0000ff,0xffffff};
    SvMapCell update={.terrain={.glyph=2,.attribute=1,.present=true,.solid=true},
        .foreground={.glyph=2,.attribute=2,.present=true,.solid=true}};
    assert(sv_map_apply(map,7,4,3,7,&update));
    uint64_t revision=sv_map_revision(map);
    assert(!sv_map_apply(map,6,4,3,SV_MAP_TERRAIN,&update));
    assert(!sv_map_apply(map,7,66,0,7,&update));
    assert(sv_map_revision(map)==revision);
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0x00ff00);
    SvMapVisual effect={.glyph=' ',.attribute=4,.present=true,.overlay_mode=SV_MAP_REPLACE_FOREGROUND};
    SvMapRenderProvider provider={.context=&effect,.overlay=overlay};
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,&provider));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0xff0000);
    effect.overlay_mode=SV_MAP_REPLACE_ALL;
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,&provider));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0);
    update.terrain.attribute=3;
    assert(sv_map_apply(map,7,4,3,SV_MAP_TERRAIN,&update));
    effect.overlay_mode=SV_MAP_REPLACE_FOREGROUND;
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,&provider));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0x0000ff);
    effect.present=false;
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0x00ff00);
    update.foreground.present=false;
    assert(sv_map_apply(map,7,4,3,SV_MAP_FOREGROUND,&update));
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0x0000ff);
    uint64_t underlying=frame(renderer,&g.bounds);
    assert(sv_map_set_indicator(map,7,SV_MAP_TARGET,4,3,true));
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))!=0x0000ff);
    uint64_t target=frame(renderer,&g.bounds);
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(frame(renderer,&g.bounds)==target); /* Repaint cannot accumulate shadow alpha. */
    assert(sv_map_set_indicator(map,7,SV_MAP_TARGET,4,3,false));
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0x0000ff);
    assert(frame(renderer,&g.bounds)==underlying); /* Glow removal restores every neighbour. */
    assert(sv_map_apply(map,7,5,3,SV_MAP_TERRAIN,&update));
    assert(sv_map_set_indicator(map,7,SV_MAP_HOVER,4,3,true));
    assert(sv_map_wipe(map,7,4,3,1,1));
    assert(!sv_map_indicator(map,SV_MAP_HOVER).visible);
    assert(sv_map_cell(map,5,3)->terrain.present);
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(pixel(renderer,(int)(cell.x+cell.w/2),(int)(cell.y+cell.h/2))==0);
    update.terrain=(SvMapVisual){.glyph='@',.attribute=4,.present=true};
    assert(sv_map_apply(map,7,4,3,SV_MAP_TERRAIN,&update));
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(ink(renderer,cell));
    update.terrain.glyph=0x10000; /* No low-byte truncation; font default/replacement. */
    assert(sv_map_apply(map,7,4,3,SV_MAP_TERRAIN,&update));
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,1,SV_RESOURCE_NEAREST,palette,NULL));
    assert(ink(renderer,cell));
    assert(sv_map_reset(map,44,8));
    assert(!sv_map_cell(map,4,3)->terrain.present);
    assert(!sv_map_apply(map,7,4,3,SV_MAP_TERRAIN,&update));
    assert(sv_map_geometry(&g,(SDL_FRect){0,0,1000,768},1.5f,44,16,24));
    assert(g.bounds.w<=1500 && g.bounds.h<=1152 && g.rows==44);
    assert(sv_map_apply(map,8,65,43,SV_MAP_TERRAIN,&update));
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,2,SV_RESOURCE_LINEAR,palette,NULL));
    assert(ink(renderer,sv_map_cell_rect(&g,65,43)));
    sv_map_renderer_reset(owner); /* Device recreation retains semantic state. */
    assert(sv_map_renderer_draw(owner,renderer,map,&g,font,2,SV_RESOURCE_LINEAR,palette,NULL));
    assert(sv_map_cell(map,65,43)->terrain.glyph==0x10000);
    sv_map_renderer_destroy(owner);sv_map_destroy(map);sv_font_close(font);
    SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
    puts("map-renderer: production layers/glyphs/wipe/indicators/normal-big DPI/hit/reset PASS");
    return 0;
}

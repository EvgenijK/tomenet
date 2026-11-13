#include "ui/map-renderer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
struct SvMapRenderer {
    SDL_Renderer *renderer;
    SvFont *font;
    uint64_t generation;
    int width,height;
    SvResourceFilter filter;
    SDL_Texture *glyphs[257]; /* Numeric byte glyphs + unavailable graphics fallback. */
};
bool sv_map_geometry(SvMapGeometry *out,SDL_FRect area,float scale,int rows,int fw,int fh)
{
    if(!out || (rows!=22 && rows!=44) || fw<1 || fh<1 || !isfinite(scale) || scale<=0 ||
       !isfinite(area.x) || !isfinite(area.y) || !isfinite(area.w) || !isfinite(area.h) ||
       area.w<=0 || area.h<=0)return false;
    double left=ceil((double)area.x*scale),top=ceil((double)area.y*scale);
    double right=floor(((double)area.x+area.w)*scale),bottom=floor(((double)area.y+area.h)*scale);
    if(fabs(left)>1000000 || fabs(top)>1000000 || fabs(right)>1000000 || fabs(bottom)>1000000)return false;
    int width=(int)(right-left),height=(int)(bottom-top);
    int maxw=width/SV_MAP_COLUMNS,maxh=height/rows;
    if(maxw<1 || maxh<1)return false;
    double factor=fmin((double)maxw/fw,(double)maxh/fh);
    int cw=(int)floor(fw*factor+.5),ch=(int)floor(fh*factor+.5);
    if(cw<1)cw=1;
    if(ch<1)ch=1;
    if(cw>maxw)cw=maxw;
    if(ch>maxh)ch=maxh;
    if(cw>4096 || ch>4096)return false;
    *out=(SvMapGeometry){.bounds={(int)left+(width-cw*66)/2,(int)top+(height-ch*rows)/2,cw*66,ch*rows},
        .columns=66,.rows=rows,.cell_width=cw,.cell_height=ch,.output_scale=scale};
    return true;
}
static bool valid_geometry(const SvMapGeometry *g)
{
    return g && g->columns==66 && (g->rows==22 || g->rows==44) &&
        g->cell_width>0 && g->cell_width<=4096 && g->cell_height>0 && g->cell_height<=4096 &&
        g->bounds.x>=-1000000 && g->bounds.x<=1000000 &&
        g->bounds.y>=-1000000 && g->bounds.y<=1000000 &&
        g->bounds.w==g->columns*g->cell_width && g->bounds.h==g->rows*g->cell_height &&
        isfinite(g->output_scale) && g->output_scale>0;
}
bool sv_map_hit(const SvMapGeometry *g,float lx,float ly,int *x,int *y)
{
    if(!valid_geometry(g) || !isfinite(lx) || !isfinite(ly))return false;
    float left=g->bounds.x/g->output_scale,top=g->bounds.y/g->output_scale;
    float right=(g->bounds.x+g->bounds.w)/g->output_scale;
    float bottom=(g->bounds.y+g->bounds.h)/g->output_scale;
    if(lx<left || ly<top || lx>=right || ly>=bottom)return false;
    double px=(double)lx*g->output_scale-g->bounds.x,py=(double)ly*g->output_scale-g->bounds.y;
    /* The public input is float logical coordinates. Round-trip boundary values
     * can land just outside the pixel interval after multiplication. */
    px=fmax(0,fmin(px,g->bounds.w-0.000001));
    py=fmax(0,fmin(py,g->bounds.h-0.000001));
    if(x)*x=(int)(px/g->cell_width);
    if(y)*y=(int)(py/g->cell_height);
    return true;
}
SDL_FRect sv_map_cell_rect(const SvMapGeometry *g,int x,int y)
{
    if(!valid_geometry(g) || x<0 || x>=g->columns || y<0 || y>=g->rows)return (SDL_FRect){0};
    return (SDL_FRect){g->bounds.x+x*g->cell_width,g->bounds.y+y*g->cell_height,g->cell_width,g->cell_height};
}
SvMapRenderer *sv_map_renderer_create(void){return calloc(1,sizeof(SvMapRenderer));}
void sv_map_renderer_reset(SvMapRenderer *owner)
{
    if(!owner)return;
    for(unsigned i=0;i<257;++i)SDL_DestroyTexture(owner->glyphs[i]);
    memset(owner,0,sizeof(*owner));
}
void sv_map_renderer_destroy(SvMapRenderer *owner){sv_map_renderer_reset(owner);free(owner);}
static SDL_Color rgb(uint32_t v){return (SDL_Color){v>>16,v>>8,v,255};}
static bool fill(SDL_Renderer *r,const SDL_FRect *rect,SDL_Color color)
{return SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a) && SDL_RenderFillRect(r,rect);}
static SDL_Surface *replacement(int width,int height)
{
    SDL_Surface *surface=SDL_CreateSurface(width,height,SDL_PIXELFORMAT_RGBA32);
    if(!surface)return NULL;
    SDL_FillSurfaceRect(surface,NULL,SDL_MapSurfaceRGBA(surface,0,0,0,0));
    Uint32 ink=SDL_MapSurfaceRGBA(surface,255,255,255,255);
    SDL_Rect edges[4]={{0,0,width,1},{0,height-1,width,1},{0,0,1,height},{width-1,0,1,height}};
    SDL_FillSurfaceRects(surface,edges,4,ink);return surface;
}
static SDL_Texture *glyph(SvMapRenderer *owner,uint32_t id)
{
    unsigned slot=id>255?256:id;
    if(owner->glyphs[slot])return owner->glyphs[slot];
    SvFontMetrics metrics;
    if(!sv_font_metrics(owner->font,&metrics))return NULL;
    SDL_Surface *surface=NULL;
    const SDL_Color white={255,255,255,255};
    if(id>255 && !metrics.has_default) {
        /* Unavailable graphics identity is one visible cell, never a truncated
         * byte or a guessed Unicode mapping. */
        surface=replacement(owner->width,owner->height);
    } else {
        uint32_t numeric=id>255?metrics.default_id:id;
        surface=sv_font_render_glyph(owner->font,numeric,
            metrics.pcf?metrics.width:owner->width,metrics.pcf?metrics.height:owner->height,white,NULL);
        if(surface && metrics.pcf) {
            SvPreparedResource prepared={0};
            SvResourceResult result=sv_resource_prepare_coverage(surface,owner->width,owner->height,owner->filter,&prepared);
            SDL_DestroySurface(surface);surface=NULL;
            if(result==SV_RESOURCE_OK)surface=sv_resource_composite(&prepared,white,(SDL_Color){0,0,0,255});
            sv_resource_prepared_dispose(&prepared);
        }
    }
    if(!surface)return NULL;
    if(id>255) {
        bool visible=false;
        for(int y=0;y<surface->h && !visible;++y)for(int x=0;x<surface->w;++x) {
            Uint8 r,g,b,a;
            if(SDL_ReadSurfacePixel(surface,x,y,&r,&g,&b,&a) && a) {visible=true;break;}
        }
        if(!visible) {SDL_DestroySurface(surface);surface=replacement(owner->width,owner->height);}
        if(!surface)return NULL;
    }
    SDL_Texture *texture=SDL_CreateTextureFromSurface(owner->renderer,surface);
    SDL_DestroySurface(surface);
    if(texture)SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
    owner->glyphs[slot]=texture;return texture;
}
static bool visual(SvMapRenderer *owner,const SvMapVisual *v,const SDL_FRect *rect,SDL_Color color,
                   const SvMapRenderProvider *provider)
{
    if(!v->present)return true;
    if(v->solid)return fill(owner->renderer,rect,color);
    if(provider && provider->visual) {
        int result=provider->visual(provider->context,owner->renderer,v,rect,color);
        if(result)return result>0;
    }
    SDL_Texture *texture=glyph(owner,v->glyph);
    return texture && SDL_SetTextureColorMod(texture,color.r,color.g,color.b) &&
        SDL_RenderTexture(owner->renderer,texture,NULL,rect);
}
static SDL_Color colour(const SvMapRenderProvider *provider,const uint32_t *palette,
    unsigned char attr,int x,int y,bool terrain)
{
    if(provider && provider->colour)return provider->colour(provider->context,attr,x,y,terrain);
    return palette?rgb(palette[attr%16]):(SDL_Color){255,255,255,255};
}
static bool stroke(SDL_Renderer *renderer,SDL_FRect rect,SDL_Color colour)
{
    if(rect.w<=0 || rect.h<=0)return true;
    return SDL_SetRenderDrawColor(renderer,colour.r,colour.g,colour.b,colour.a) &&
        SDL_RenderRect(renderer,&rect);
}
static int radius(const SvMapGeometry *g,float logical)
{
    /* A shadow cannot contribute beyond the complete map clip. Bounding the
     * loop also avoids an unbounded conversion for invalid display metadata. */
    return (int)fminf(fmaxf(g->bounds.w,g->bounds.h),ceilf(logical*g->output_scale));
}
static bool outer_glow(SDL_Renderer *renderer,SDL_FRect rect,int pixels,SDL_Color colour)
{
    for(int i=pixels;i>0;--i) {
        float strength=1-(float)i/(pixels+1);
        SDL_Color faded=colour;faded.a=(Uint8)(colour.a*strength*strength);
        SDL_FRect ring={rect.x-i,rect.y-i,rect.w+2*i,rect.h+2*i};
        if(!stroke(renderer,ring,faded))return false;
    }
    return true;
}
static bool indicator(SDL_Renderer *r,const SvMapGeometry *g,SvMapIndicator v,bool target)
{
    if(!v.visible)return true;
    SDL_FRect cell=sv_map_cell_rect(g,v.x,v.y);
    if(cell.w<=0)return false;
    if(target) {
        /* HTML windows.css .throw-target-cell: independent contrast outline,
         * cyan outer glow and white inset glow. These final-pixel gradient
         * rings approximate the soft CSS shadows; CSS pixels are not an oracle. */
        if(!outer_glow(r,cell,radius(g,11),(SDL_Color){32,236,242,96}))return false;
        int contrast=radius(g,1);
        for(int i=contrast;i>0;--i) {
            SDL_FRect ring={cell.x-i,cell.y-i,cell.w+2*i,cell.h+2*i};
            if(!stroke(r,ring,(SDL_Color){0,26,27,255}))return false;
        }
        if(!fill(r,&cell,(SDL_Color){24,239,244,46}))return false;
        int inset=radius(g,7);
        for(int i=0;i<inset && cell.w>2*i && cell.h>2*i;++i) {
            float strength=1-(float)i/(inset+1);
            SDL_FRect ring={cell.x+i,cell.y+i,cell.w-2*i,cell.h-2*i};
            if(!stroke(r,ring,(SDL_Color){255,255,255,(Uint8)(102*strength*strength)}))return false;
        }
        SDL_SetRenderDrawColor(r,79,252,255,255);
        int border=(int)fminf(fminf(cell.w,cell.h),fmaxf(1,roundf(2*g->output_scale)));
        for(int i=0;i<border && cell.w>2*i && cell.h>2*i;++i) {
            SDL_FRect inner={cell.x+i,cell.y+i,cell.w-2*i,cell.h-2*i};
            if(!SDL_RenderRect(r,&inner))return false;
        }
        /* Same HTML crosshair role, clipped to the complete map bounds. */
        float stroke=fmaxf(1,roundf(g->output_scale*2));
        SDL_FRect vertical={cell.x+cell.w/2-stroke/2,cell.y-7*g->output_scale,stroke,cell.h+14*g->output_scale};
        SDL_FRect horizontal={cell.x-6*g->output_scale,cell.y+cell.h/2-stroke/2,cell.w+12*g->output_scale,stroke};
        if(!outer_glow(r,vertical,radius(g,5),(SDL_Color){40,255,255,96}) ||
           !outer_glow(r,horizontal,radius(g,5),(SDL_Color){40,255,255,96}))return false;
        return fill(r,&vertical,(SDL_Color){159,255,255,255}) && fill(r,&horizontal,(SDL_Color){159,255,255,255});
    }
    SDL_SetRenderDrawColor(r,89,246,239,230);
    return SDL_RenderRect(r,&cell);
}
bool sv_map_renderer_draw(SvMapRenderer *owner,SDL_Renderer *renderer,const SvMap *map,const SvMapGeometry *g,
    SvFont *font,uint64_t generation,SvResourceFilter filter,const uint32_t palette[16],const SvMapRenderProvider *provider)
{
    if(!owner || !renderer || !map || !font || !valid_geometry(g) || sv_map_rows(map)!=g->rows)return false;
    if(owner->renderer!=renderer || owner->font!=font || owner->generation!=generation ||
       owner->width!=g->cell_width || owner->height!=g->cell_height || owner->filter!=filter) {
        sv_map_renderer_reset(owner);owner->renderer=renderer;owner->font=font;owner->generation=generation;
        owner->width=g->cell_width;owner->height=g->cell_height;owner->filter=filter;
    }
    float sx=1,sy=1;SDL_GetRenderScale(renderer,&sx,&sy);
    SDL_Rect oldclip;bool clipped=SDL_RenderClipEnabled(renderer);
    SDL_GetRenderClipRect(renderer,&oldclip);
    SDL_BlendMode blend;SDL_GetRenderDrawBlendMode(renderer,&blend);
    Uint8 r,gc,b,a;SDL_GetRenderDrawColor(renderer,&r,&gc,&b,&a);
    bool ok=SDL_SetRenderScale(renderer,1,1) && SDL_SetRenderClipRect(renderer,&g->bounds) &&
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    for(int y=0;ok && y<g->rows;++y)for(int x=0;ok && x<g->columns;++x) {
        const SvMapCell *cell=sv_map_cell(map,x,y);SDL_FRect rect=sv_map_cell_rect(g,x,y);
        ok=fill(renderer,&rect,rgb(cell->background_rgb)) &&
            visual(owner,&cell->terrain,&rect,colour(provider,palette,cell->terrain.attribute,x,y,true),provider) &&
            visual(owner,&cell->foreground,&rect,colour(provider,palette,cell->foreground.attribute,x,y,false),provider);
    }
    if(provider && provider->overlay)for(int y=0;ok && y<g->rows;++y)for(int x=0;ok && x<g->columns;++x) {
        SvMapVisual effect={0};
        if(provider->overlay(provider->context,x,y,&effect)) {
            SDL_FRect rect=sv_map_cell_rect(g,x,y);
            const SvMapCell *cell=sv_map_cell(map,x,y);
            if(effect.overlay_mode!=SV_MAP_OVER)ok=fill(renderer,&rect,rgb(cell->background_rgb));
            if(ok && effect.overlay_mode==SV_MAP_REPLACE_FOREGROUND)
                ok=visual(owner,&cell->terrain,&rect,colour(provider,palette,cell->terrain.attribute,x,y,true),provider);
            if(ok)ok=visual(owner,&effect,&rect,colour(provider,palette,effect.attribute,x,y,false),provider);
        }
    }
    /* Self state is retained for its future approved HTML visual; the rejected
     * historical A/B/C sketches do not become a new native self design. */
    SvMapIndicator target=sv_map_indicator(map,SV_MAP_TARGET);
    if(ok && !target.visible)ok=indicator(renderer,g,sv_map_indicator(map,SV_MAP_HOVER),false);
    if(ok)ok=indicator(renderer,g,target,true);
    SDL_SetRenderDrawColor(renderer,r,gc,b,a);SDL_SetRenderDrawBlendMode(renderer,blend);
    SDL_SetRenderScale(renderer,sx,sy);SDL_SetRenderClipRect(renderer,clipped?&oldclip:NULL);
    return ok;
}

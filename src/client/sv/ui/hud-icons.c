#include "ui/hud-icons.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_OUTLINE_H
#include FT_GLYPH_H
#include <stdint.h>
#include <limits.h>
#include <math.h>

typedef struct { char operation; unsigned char closed; float coordinates[6]; } SvIconCommand;
typedef struct { const SvIconCommand *commands; size_t count; } SvIconPrimitive;
typedef struct { unsigned first, count; float stroke, width, height; } SvIconGeometry;
#include "ui/hud-icons-data.h"

/* Icon outlines are immutable compiled SVG source, rasterized only at their
 * final physical size/placement. The texture is then copied 1:1, not downscaled. */
#define ICON_CACHE_ENTRIES 192
#define ICON_MAX_DIMENSION 1024
#define ICON_MAX_SIGMA 64

typedef struct {
    SvHudIcon icon;
    int width64, height64, x64, y64, sigma64;
    int width, height, padding;
    uint64_t generation;
    SDL_Texture *texture;
} SvIconRaster;
typedef struct {
    SvHudIcon icon;
    int width64, height64, x64, y64, width, height, padding;
    SDL_Color foreground;
    SvHudIconShadow shadows[2];
    size_t count;
    uint64_t generation;
    SDL_Texture *texture;
} SvIconFilter;
typedef struct {
    FT_Library library;
    FT_Stroker stroker;
    unsigned next;
    SvIconRaster entries[ICON_CACHE_ENTRIES];
    unsigned next_filter;
    SvIconFilter filters[64];
} SvIconCache;
typedef struct { unsigned char *alpha; int width, height; } SvIconCoverage;

static void SDLCALL cache_free(void *userdata, void *value)
{
    (void)userdata;
    SvIconCache *cache=value;
    if(cache->stroker)FT_Stroker_Done(cache->stroker);
    if(cache->library)FT_Done_FreeType(cache->library);
    /* The renderer owns its textures; only eviction destroys them explicitly. */
    SDL_free(cache);
}
static SvIconCache *icon_cache(SDL_Renderer *renderer)
{
    SDL_PropertiesID properties=SDL_GetRendererProperties(renderer);
    const char *name="TomeNET.SV.HUD.final_size_svg_rasters";
    if(!properties)return NULL;
    SvIconCache *cache=SDL_GetPointerProperty(properties,name,NULL);
    if(cache)return cache;
    cache=SDL_calloc(1,sizeof(*cache));if(!cache)return NULL;
    if(FT_Init_FreeType(&cache->library)||FT_Stroker_New(cache->library,&cache->stroker)){
        cache_free(NULL,cache);return NULL;
    }
    /* SDL invokes cleanup itself if setting the property fails. */
    if(!SDL_SetPointerPropertyWithCleanup(properties,name,cache,cache_free,NULL))return NULL;
    return cache;
}
static FT_Vector vector(float x,float y,float scale,float offset_x,float offset_y,int height)
{
    return (FT_Vector){(FT_Pos)SDL_roundf((offset_x+x*scale)*64),
        (FT_Pos)SDL_roundf((height-offset_y-y*scale)*64)};
}
static bool raster_geometry(SvIconCache *cache,SvHudIcon icon,int width64,int height64,
    int x64,int y64,int padding,SvIconCoverage *coverage)
{
    const SvIconGeometry *geometry=&sv_icon_geometry[icon];
    float width=width64/64.f,height=height64/64.f;
    float scale=SDL_min(width/geometry->width,height/geometry->height);
    float offset_x=padding+x64/64.f+(width-geometry->width*scale)/2;
    float offset_y=padding+y64/64.f+(height-geometry->height*scale)/2;
    FT_Stroker_Set(cache->stroker,(FT_Fixed)SDL_roundf(geometry->stroke*scale*32),
        FT_STROKER_LINECAP_ROUND,FT_STROKER_LINEJOIN_ROUND,1<<16);
    for(unsigned i=0;i<geometry->count;++i){
        const SvIconPrimitive *primitive=&sv_icon_primitives[geometry->first+i];
        FT_Stroker_Rewind(cache->stroker);
        for(size_t j=0;j<primitive->count;++j){
            const SvIconCommand *command=&primitive->commands[j];
            const float *p=command->coordinates;
            FT_Vector a=vector(p[0],p[1],scale,offset_x,offset_y,coverage->height);
            FT_Error error=0;
            switch(command->operation){
            case 'M':error=FT_Stroker_BeginSubPath(cache->stroker,&a,!command->closed);break;
            case 'L':error=FT_Stroker_LineTo(cache->stroker,&a);break;
            case 'C':{
                FT_Vector b=vector(p[2],p[3],scale,offset_x,offset_y,coverage->height);
                FT_Vector c=vector(p[4],p[5],scale,offset_x,offset_y,coverage->height);
                error=FT_Stroker_CubicTo(cache->stroker,&a,&b,&c);break;
            }
            case 'E':error=FT_Stroker_EndSubPath(cache->stroker);break;
            default:return false;
            }
            if(error)return false;
        }
        FT_UInt points,contours;
        if(FT_Stroker_GetCounts(cache->stroker,&points,&contours)||points>SHRT_MAX||contours>SHRT_MAX)return false;
        FT_Glyph glyph=NULL;
        if(FT_New_Glyph(cache->library,FT_GLYPH_FORMAT_OUTLINE,&glyph))return false;
        FT_Outline *outline=&((FT_OutlineGlyph)glyph)->outline;
        if(FT_Outline_New(cache->library,points,(FT_Int)contours,outline)){FT_Done_Glyph(glyph);return false;}
        outline->n_points=0;outline->n_contours=0;
        FT_Stroker_Export(cache->stroker,outline);
        /* A single SVG path may contain intersecting/duplicate stroke contours
         * (e.g. the food bowl rim). The smooth glyph renderer honors OVERLAP
         * and computes target-pixel coverage without doubling shared edges. */
        outline->flags|=FT_OUTLINE_OVERLAP;
        if(FT_Glyph_To_Bitmap(&glyph,FT_RENDER_MODE_NORMAL,NULL,true)){FT_Done_Glyph(glyph);return false;}
        FT_BitmapGlyph bitmap_glyph=(FT_BitmapGlyph)glyph;FT_Bitmap *bitmap=&bitmap_glyph->bitmap;
        if(bitmap->pixel_mode!=FT_PIXEL_MODE_GRAY&&bitmap->width){FT_Done_Glyph(glyph);return false;}
        for(unsigned row=0;row<bitmap->rows;++row){
            int y=coverage->height-bitmap_glyph->top+(int)row;if(y<0||y>=coverage->height)continue;
            const unsigned char *source=bitmap->buffer+(bitmap->pitch>=0?(int)row:((int)bitmap->rows-1-(int)row))*SDL_abs(bitmap->pitch);
            for(unsigned column=0;column<bitmap->width;++column){int x=bitmap_glyph->left+(int)column;if(x<0||x>=coverage->width)continue;
                unsigned old=coverage->alpha[y*coverage->width+x],alpha=source[column];
                coverage->alpha[y*coverage->width+x]=(unsigned char)(alpha+(old*(255-alpha)+127)/255);
            }
        }
        FT_Done_Glyph(glyph);
    }
    return true;
}
static bool gaussian_blur(SvIconCoverage *coverage,float sigma)
{
    if(sigma<=0)return true;
    int radius=(int)SDL_ceilf(3*sigma),size=2*radius+1;
    float *kernel=SDL_malloc((size_t)size*sizeof(*kernel));
    size_t pixels=(size_t)coverage->width*coverage->height;
    float *temporary=SDL_malloc(pixels*sizeof(*temporary));
    if(!kernel||!temporary){SDL_free(kernel);SDL_free(temporary);return false;}
    float sum=0;for(int i=-radius;i<=radius;++i){kernel[i+radius]=SDL_expf(-i*i/(2*sigma*sigma));sum+=kernel[i+radius];}
    for(int i=0;i<size;++i)kernel[i]/=sum;
    for(int y=0;y<coverage->height;++y)for(int x=0;x<coverage->width;++x){float value=0;
        for(int k=-radius;k<=radius;++k)if(x+k>=0&&x+k<coverage->width)value+=coverage->alpha[y*coverage->width+x+k]*kernel[k+radius];
        temporary[y*coverage->width+x]=value;
    }
    for(int y=0;y<coverage->height;++y)for(int x=0;x<coverage->width;++x){float value=0;
        for(int k=-radius;k<=radius;++k)if(y+k>=0&&y+k<coverage->height)value+=temporary[(y+k)*coverage->width+x]*kernel[k+radius];
        coverage->alpha[y*coverage->width+x]=(unsigned char)SDL_clamp((int)SDL_roundf(value),0,255);
    }
    SDL_free(kernel);SDL_free(temporary);return true;
}
static SvIconRaster *icon_raster(SvIconCache *cache,SDL_Renderer *renderer,SvHudIcon icon,
    int width64,int height64,int x64,int y64,int sigma64)
{
    for(size_t i=0;i<SDL_arraysize(cache->entries);++i){SvIconRaster *entry=&cache->entries[i];
        if(entry->texture&&entry->icon==icon&&entry->width64==width64&&entry->height64==height64&&
            entry->x64==x64&&entry->y64==y64&&entry->sigma64==sigma64&&entry->generation==SV_HUD_ICON_SOURCE_GENERATION)return entry;
    }
    int padding=sigma64?(int)SDL_ceilf(3*sigma64/64.f):0;
    int width=(width64+x64+63)/64+2*padding,height=(height64+y64+63)/64+2*padding;
    SvIconCoverage coverage={SDL_calloc((size_t)width*height,1),width,height};if(!coverage.alpha)return NULL;
    if(!raster_geometry(cache,icon,width64,height64,x64,y64,padding,&coverage)||
        !gaussian_blur(&coverage,sigma64/64.f)){SDL_free(coverage.alpha);return NULL;}
    SDL_Surface *surface=SDL_CreateSurface(width,height,SDL_PIXELFORMAT_RGBA32);
    if(!surface){SDL_free(coverage.alpha);return NULL;}
    for(int y=0;y<height;++y){unsigned char *pixel=(unsigned char*)surface->pixels+y*surface->pitch;
        for(int x=0;x<width;++x){pixel[x*4]=pixel[x*4+1]=pixel[x*4+2]=255;pixel[x*4+3]=coverage.alpha[y*width+x];}}
    SDL_free(coverage.alpha);
    SDL_Texture *texture=SDL_CreateTextureFromSurface(renderer,surface);SDL_DestroySurface(surface);
    if(!texture)return NULL;
    if(!SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST)||!SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND)){
        SDL_DestroyTexture(texture);return NULL;
    }
    SvIconRaster *entry=&cache->entries[cache->next++%SDL_arraysize(cache->entries)];
    if(entry->texture)SDL_DestroyTexture(entry->texture);
    *entry=(SvIconRaster){icon,width64,height64,x64,y64,sigma64,width,height,padding,SV_HUD_ICON_SOURCE_GENERATION,texture};
    return entry;
}
static bool draw_effect(SDL_Renderer *renderer,SvHudIcon icon,SDL_FRect destination,
    SDL_Color colour,float sigma,float offset_x,float offset_y)
{
    if(!renderer||icon<0||icon>=SV_HUD_ICON_COUNT||!isfinite(destination.x)||!isfinite(destination.y)||
        !isfinite(destination.w)||!isfinite(destination.h)||destination.w<=0||destination.h<=0||
        destination.w>ICON_MAX_DIMENSION||destination.h>ICON_MAX_DIMENSION||!isfinite(sigma)||sigma<0||sigma>ICON_MAX_SIGMA||
        !isfinite(offset_x)||!isfinite(offset_y))return false;
    if(!colour.a)return true;
    float x=destination.x+offset_x,y=destination.y+offset_y;
    if(x<-1073741824.f||x>1073741824.f||y<-1073741824.f||y>1073741824.f)return false;
    int pixel_x=(int)SDL_floorf(x),pixel_y=(int)SDL_floorf(y);
    int x64=(int)SDL_roundf((x-pixel_x)*64),y64=(int)SDL_roundf((y-pixel_y)*64);
    if(x64==64){++pixel_x;x64=0;}if(y64==64){++pixel_y;y64=0;}
    int width64=(int)SDL_roundf(destination.w*64),height64=(int)SDL_roundf(destination.h*64);
    if(width64<1||height64<1)return true;
    SvIconCache *cache=icon_cache(renderer);if(!cache)return false;
    SvIconRaster *raster=icon_raster(cache,renderer,icon,width64,height64,x64,y64,(int)SDL_roundf(sigma*64));if(!raster)return false;
    SDL_FRect target={(float)(pixel_x-raster->padding),(float)(pixel_y-raster->padding),(float)raster->width,(float)raster->height};
    return SDL_SetTextureColorMod(raster->texture,colour.r,colour.g,colour.b)&&
        SDL_SetTextureAlphaMod(raster->texture,colour.a)&&SDL_RenderTexture(renderer,raster->texture,NULL,&target);
}
bool sv_hud_icon_draw(SDL_Renderer *renderer,SvHudIcon icon,SDL_FRect destination,SDL_Color colour)
{ return draw_effect(renderer,icon,destination,colour,0,0,0); }
bool sv_hud_icon_shadow_draw(SDL_Renderer *renderer,SvHudIcon icon,SDL_FRect destination,
    SDL_Color colour,float sigma_pixels,float offset_x_pixels,float offset_y_pixels)
{ return draw_effect(renderer,icon,destination,colour,sigma_pixels,offset_x_pixels,offset_y_pixels); }

static bool same_colour(SDL_Color a,SDL_Color b)
{ return a.r==b.r&&a.g==b.g&&a.b==b.b&&a.a==b.a; }
static bool shifted_kernel(float sigma,float offset,float **out,int *first,int *last)
{
    int radius=sigma>0?(int)SDL_ceilf(3*sigma):1;
    *first=(int)SDL_floorf(offset)-radius;*last=(int)SDL_ceilf(offset)+radius;
    int count=*last-*first+1;float *kernel=SDL_malloc((size_t)count*sizeof(*kernel));
    if(!kernel)return false;
    float sum=0;for(int i=*first;i<=*last;++i){float d=i-offset;
        float weight=sigma>0?SDL_expf(-d*d/(2*sigma*sigma)):SDL_max(0,1-SDL_fabsf(d));
        kernel[i-*first]=weight;sum+=weight;
    }
    for(int i=0;i<count;++i)kernel[i]/=sum;
    *out=kernel;return true;
}
static bool shifted_blur(const float *alpha,float *result,int width,int height,
    const SvHudIconShadow *shadow)
{
    float *kx=NULL,*ky=NULL;int xfirst,xlast,yfirst,ylast;
    float *temporary=SDL_malloc((size_t)width*height*sizeof(*temporary));
    if(!temporary||!shifted_kernel(shadow->sigma_pixels,shadow->offset_x_pixels,&kx,&xfirst,&xlast)||
        !shifted_kernel(shadow->sigma_pixels,shadow->offset_y_pixels,&ky,&yfirst,&ylast)){
        SDL_free(temporary);SDL_free(kx);SDL_free(ky);return false;
    }
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){float value=0;
        for(int k=xfirst;k<=xlast;++k)if(x-k>=0&&x-k<width)value+=alpha[y*width+x-k]*kx[k-xfirst];
        temporary[y*width+x]=value;
    }
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){float value=0;
        for(int k=yfirst;k<=ylast;++k)if(y-k>=0&&y-k<height)value+=temporary[(y-k)*width+x]*ky[k-yfirst];
        result[y*width+x]=value;
    }
    SDL_free(temporary);SDL_free(kx);SDL_free(ky);return true;
}
static bool filtered_texture(SvIconCache *cache,SDL_Renderer *renderer,SvIconFilter *key)
{
    size_t pixels=(size_t)key->width*key->height;
    SvIconCoverage coverage={SDL_calloc(pixels,1),key->width,key->height};
    float *image=SDL_calloc(pixels*4,sizeof(*image));
    float *alpha=SDL_malloc(pixels*sizeof(*alpha)),*shadow_alpha=SDL_malloc(pixels*sizeof(*shadow_alpha));
    if(!coverage.alpha||!image||!alpha||!shadow_alpha)goto failed;
    if(!raster_geometry(cache,key->icon,key->width64,key->height64,key->x64,key->y64,key->padding,&coverage))goto failed;
    for(size_t i=0;i<pixels;++i){float a=coverage.alpha[i]/255.f*key->foreground.a/255.f;
        image[i*4]=key->foreground.r/255.f*a;image[i*4+1]=key->foreground.g/255.f*a;
        image[i*4+2]=key->foreground.b/255.f*a;image[i*4+3]=a;
    }
    for(size_t stage=0;stage<key->count;++stage){
        const SvHudIconShadow *shadow=&key->shadows[stage];
        for(size_t i=0;i<pixels;++i)alpha[i]=image[i*4+3];
        if(!shifted_blur(alpha,shadow_alpha,key->width,key->height,shadow))goto failed;
        for(size_t i=0;i<pixels;++i){float a=SDL_clamp(shadow_alpha[i],0,1)*shadow->colour.a/255.f;
            float behind=a*(1-image[i*4+3]);
            image[i*4]+=shadow->colour.r/255.f*behind;
            image[i*4+1]+=shadow->colour.g/255.f*behind;
            image[i*4+2]+=shadow->colour.b/255.f*behind;
            image[i*4+3]+=behind;
        }
    }
    SDL_Surface *surface=SDL_CreateSurface(key->width,key->height,SDL_PIXELFORMAT_RGBA32);
    if(!surface)goto failed;
    for(int y=0;y<key->height;++y){unsigned char *pixel=(unsigned char*)surface->pixels+y*surface->pitch;
        for(int x=0;x<key->width;++x){size_t i=(size_t)y*key->width+x;float a=image[i*4+3];
            for(int channel=0;channel<3;++channel)pixel[x*4+channel]=a>0?(unsigned char)SDL_clamp((int)SDL_roundf(image[i*4+channel]/a*255),0,255):0;
            pixel[x*4+3]=(unsigned char)SDL_clamp((int)SDL_roundf(a*255),0,255);
        }
    }
    key->texture=SDL_CreateTextureFromSurface(renderer,surface);SDL_DestroySurface(surface);
    SDL_free(coverage.alpha);SDL_free(image);SDL_free(alpha);SDL_free(shadow_alpha);
    if(!key->texture)return false;
    if(!SDL_SetTextureScaleMode(key->texture,SDL_SCALEMODE_NEAREST)||!SDL_SetTextureBlendMode(key->texture,SDL_BLENDMODE_BLEND)){
        SDL_DestroyTexture(key->texture);key->texture=NULL;return false;
    }
    return true;
failed:
    SDL_free(coverage.alpha);SDL_free(image);SDL_free(alpha);SDL_free(shadow_alpha);return false;
}
bool sv_hud_icon_filter_draw(SDL_Renderer *renderer,SvHudIcon icon,SDL_FRect destination,
    SDL_Color foreground,const SvHudIconShadow *shadows,size_t count)
{
    if(!count)return sv_hud_icon_draw(renderer,icon,destination,foreground);
    if(!renderer||icon<0||icon>=SV_HUD_ICON_COUNT||count>2||!shadows||
        !isfinite(destination.x)||!isfinite(destination.y)||!isfinite(destination.w)||!isfinite(destination.h)||
        destination.w<=0||destination.h<=0||destination.w>ICON_MAX_DIMENSION||destination.h>ICON_MAX_DIMENSION||
        destination.x< -1073741824.f||destination.x>1073741824.f||destination.y< -1073741824.f||destination.y>1073741824.f)return false;
    int pixel_x=(int)SDL_floorf(destination.x),pixel_y=(int)SDL_floorf(destination.y);
    SvIconFilter key={0};key.icon=icon;key.foreground=foreground;key.count=count;key.generation=SV_HUD_ICON_SOURCE_GENERATION;
    key.width64=(int)SDL_roundf(destination.w*64);key.height64=(int)SDL_roundf(destination.h*64);
    if(key.width64<1||key.height64<1)return true;
    key.x64=(int)SDL_roundf((destination.x-pixel_x)*64);key.y64=(int)SDL_roundf((destination.y-pixel_y)*64);
    if(key.x64==64){++pixel_x;key.x64=0;}if(key.y64==64){++pixel_y;key.y64=0;}
    float reach=0;
    for(size_t i=0;i<count;++i){const SvHudIconShadow *shadow=&shadows[i];
        if(!isfinite(shadow->sigma_pixels)||!isfinite(shadow->offset_x_pixels)||!isfinite(shadow->offset_y_pixels)||
            shadow->sigma_pixels<0||shadow->sigma_pixels>ICON_MAX_SIGMA||
            SDL_fabsf(shadow->offset_x_pixels)>64||SDL_fabsf(shadow->offset_y_pixels)>64)return false;
        key.shadows[i]=*shadow;reach+=3*shadow->sigma_pixels+SDL_max(SDL_fabsf(shadow->offset_x_pixels),SDL_fabsf(shadow->offset_y_pixels));
    }
    key.padding=(int)SDL_ceilf(reach);key.width=(key.width64+key.x64+63)/64+2*key.padding;key.height=(key.height64+key.y64+63)/64+2*key.padding;
    SvIconCache *cache=icon_cache(renderer);if(!cache)return false;
    SvIconFilter *found=NULL;
    for(size_t i=0;i<SDL_arraysize(cache->filters);++i){SvIconFilter *entry=&cache->filters[i];
        if(!entry->texture||entry->icon!=icon||entry->width64!=key.width64||entry->height64!=key.height64||
            entry->x64!=key.x64||entry->y64!=key.y64||entry->count!=count||entry->generation!=key.generation||
            !same_colour(entry->foreground,foreground))continue;
        bool equal=true;for(size_t j=0;j<count;++j)if(!same_colour(entry->shadows[j].colour,shadows[j].colour)||
            entry->shadows[j].sigma_pixels!=shadows[j].sigma_pixels||entry->shadows[j].offset_x_pixels!=shadows[j].offset_x_pixels||
            entry->shadows[j].offset_y_pixels!=shadows[j].offset_y_pixels)equal=false;
        if(equal){found=entry;break;}
    }
    if(!found){
        if(!filtered_texture(cache,renderer,&key))return false;
        found=&cache->filters[cache->next_filter++%SDL_arraysize(cache->filters)];
        if(found->texture)SDL_DestroyTexture(found->texture);*found=key;
    }
    SDL_FRect target={(float)(pixel_x-found->padding),(float)(pixel_y-found->padding),(float)found->width,(float)found->height};
    return SDL_RenderTexture(renderer,found->texture,NULL,&target);
}

#include "settings/graphics.h"
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { SHEETS=11, GLYPHS=65536, RAW=257, CACHE=1024 };
typedef struct { SDL_Surface *image; SvFilePath source; SvResourceMasks masks; } Sheet;
typedef struct { bool present; int sheet; SDL_Rect rectangle; } Raw;
typedef struct {
    bool used,raw; uint32_t id; int width,height; SvPreparedResource prepared;
    SDL_Texture *textures[3];
} Entry;
struct SvGraphics {
    int mode,tile_width,tile_height;
    char name[SV_SETTINGS_TEXT_MAX];
    Sheet sheets[SHEETS];
    signed char subsets[GLYPHS];
    bool mapped[GLYPHS];
    Raw raw[RAW];
    Entry cache[CACHE]; unsigned next;
    size_t cache_bytes;
    SDL_Renderer *renderer;
    bool disable_tile_cache,disable_tileset_cache;
    SvResourcePreparation preparation;
    uint64_t generation;
};
static uint64_t hash(uint64_t h,const void *data,size_t size)
{
    const unsigned char *p=data;
    while(size--) {h^=*p++;h*=UINT64_C(1099511628211);}return h;
}
static void error_text(char *error,size_t size,const char *text)
{ if(error && size)SDL_strlcpy(error,text,size); }
static void dispose_entry(Entry *e)
{
    for(unsigned i=0;i<3;++i){SDL_DestroyTexture(e->textures[i]);e->textures[i]=NULL;}
    sv_resource_prepared_dispose(&e->prepared);e->used=false;
}
void sv_graphics_reset_renderer(SvGraphics *g)
{
    if(!g)return;
    for(unsigned i=0;i<CACHE;++i)for(unsigned j=0;j<3;++j) {
        SDL_DestroyTexture(g->cache[i].textures[j]);g->cache[i].textures[j]=NULL;
    }
    g->renderer=NULL;
}
void sv_graphics_cache_policy(SvGraphics *g,bool tile,bool tileset)
{if(g){g->disable_tile_cache=tile;g->disable_tileset_cache=tileset;}}
bool sv_graphics_persistent_cache(const SvGraphics *g){(void)g;return false;}
void sv_graphics_destroy(SvGraphics *g)
{
    if(!g)return;
    for(unsigned i=0;i<CACHE;++i)dispose_entry(&g->cache[i]);
    for(unsigned i=0;i<SHEETS;++i)SDL_DestroySurface(g->sheets[i].image);
    free(g);
}
int sv_graphics_mode(const SvGraphics *g){return g?g->mode:0;}
const char *sv_graphics_name(const SvGraphics *g){return g&&g->mode?g->name:"NO_GRAPHICS";}
uint64_t sv_graphics_generation(const SvGraphics *g){return g?g->generation:0;}
static bool image(SvGraphics *g,int index,const char *user,const char *library,const char *name)
{
    char relative[SV_FILE_PATH];
    if(snprintf(relative,sizeof(relative),"xtra/graphics/%s",name)>=(int)sizeof(relative))return false;
    Sheet *s=&g->sheets[index];
    if(sv_file_resource(&s->source,user,library,relative)!=SV_FILE_OK)return false;
    SDL_Surface *loaded=sv_resource_load_image(s->source.path,NULL);
    if(!loaded)return false;
    s->image=SDL_ConvertSurface(loaded,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(loaded);
    if(!s->image || s->image->w<g->tile_width || s->image->h<g->tile_height ||
       s->image->w%g->tile_width || s->image->h%g->tile_height)return false;
    for(int y=0;y<s->image->h;++y)g->generation=hash(g->generation,
        (unsigned char *)s->image->pixels+y*s->image->pitch,(size_t)s->image->w*4);
    g->generation=hash(g->generation,s->source.path,strlen(s->source.path));return true;
}
static void discover(const char *root,const char *base,const int enabled[10],char names[10][SV_FILE_PATH])
{
    SvFilePath path;if(!sv_file_path(&path,root,"xtra/graphics",SV_FILE_SHARED))return;
    DIR *dir=opendir(path.path);if(!dir)return;
    struct dirent *entry;size_t length=strlen(base);
    while((entry=readdir(dir))) {
        const char *name=entry->d_name;size_t n=strlen(name);
        if(n<length+8 || strncmp(name,base,length) || name[length]!='#' ||
           strcmp(name+n-4,".bmp"))continue;
        char *end;long subset=strtol(name+length+1,&end,10);
        if(end==name+length+1 || *end!='_' || subset<0 || subset>=10 ||
           !enabled[subset] || names[subset][0])continue;
        SDL_strlcpy(names[subset],name,SV_FILE_PATH);
    }closedir(dir);
}
static SvPreferenceStatus load_preferences(SvPreferences *p,const char *user,const char *library,
    const char *stem,SvPreferenceContext *ctx)
{
    char name[SV_FILE_PATH],relative[SV_FILE_PATH];SvFilePath source;
    if(snprintf(name,sizeof(name),"graphics-%s.prf",stem)>=(int)sizeof(name) ||
       snprintf(relative,sizeof(relative),"user/%s",name)>=(int)sizeof(relative))return SV_PREF_FATAL;
    SvFileResult found=sv_file_resource(&source,user,library,relative);
    if(found==SV_FILE_MISSING)return SV_PREF_UNAVAILABLE;
    if(found!=SV_FILE_OK)return SV_PREF_FATAL;
    return sv_preferences_load(p,SV_OPTION_SHARED,name,SV_PREF_OPTIONAL,ctx).status;
}
static bool tables(SvGraphics *g,const SvPreferences *p)
{
    for(unsigned i=0;i<GLYPHS;++i)g->subsets[i]=(signed char)sv_preferences_glyph_subtileset(p,i);
    for(int sheet=0;sheet<SHEETS;++sheet) {
        g->sheets[sheet].masks=sheet?g->sheets[0].masks:sv_resource_default_masks(g->mode==2);
        for(size_t i=0;i<sv_preferences_mapping_count(p);++i) {
            const SvPreferenceMapping *m=sv_preferences_mapping(p,i);
            if(m->subtileset!=sheet-1)continue;
            if(m->family=='m') {
                SvResourceMasks *mask=&g->sheets[sheet].masks;uint32_t rgb=(uint32_t)m->fields[0]>>8;
                if(m->index==0){mask->background=true;mask->background_rgb=rgb;}
                if(m->index==1){mask->foreground=true;mask->foreground_rgb=rgb;}
                if(m->index==2){mask->outline=g->mode==2;mask->outline_rgb=rgb;}
            }
            if(m->family=='I' && m->index<RAW && g->sheets[sheet].image) {
                for(unsigned j=0;j<4;++j)if(m->fields[j]<0 || m->fields[j]>INT_MAX)return false;
                SDL_Rect r={(int)m->fields[0],(int)m->fields[1],(int)m->fields[2],(int)m->fields[3]};
                /* A zero extent disables a definition. */
                if(!r.w || !r.h){g->raw[m->index].present=false;continue;}
                if(r.x>g->sheets[sheet].image->w-r.w || r.y>g->sheets[sheet].image->h-r.h)return false;
                g->raw[m->index]=(Raw){true,sheet,r};
            }
        }
        const SvResourceMasks *mask=&g->sheets[sheet].masks;
        uint32_t key[]={mask->background,mask->foreground,mask->outline,
            mask->background_rgb,mask->foreground_rgb,mask->outline_rgb};
        g->generation=hash(g->generation,key,sizeof(key));
    }
    g->generation=hash(g->generation,g->subsets,sizeof(g->subsets));
    for(unsigned i=0;i<RAW;++i)if(g->raw[i].present) {
        int key[]={(int)i,g->raw[i].sheet,g->raw[i].rectangle.x,g->raw[i].rectangle.y,
            g->raw[i].rectangle.w,g->raw[i].rectangle.h};
        g->generation=hash(g->generation,key,sizeof(key));
    }
    return true;
}
static const SvPreparedResource *prepared(SvGraphics *g,bool raw,uint32_t id,int width,int height)
{
    if(!g || !g->mode || width<=0 || height<=0 || width>8192 || height>8192)return NULL;
    int sheet=0;SDL_Rect rectangle;
    if(raw) {
        if(id>=RAW || !g->raw[id].present)return NULL;
        sheet=g->raw[id].sheet;rectangle=g->raw[id].rectangle;
        int64_t w=(int64_t)rectangle.w*width/g->tile_width;
        int64_t h=(int64_t)rectangle.h*height/g->tile_height;
        if(w<1 || h<1 || w>8192 || h>8192)return NULL;
        width=(int)w;height=(int)h;
    } else {
        if(id<256 || id>=GLYPHS)return NULL;
        int subset=g->subsets[id];
        if(subset>=0 && subset<10 && g->sheets[subset+1].image)sheet=subset+1;
        SDL_Surface *image=g->sheets[sheet].image;if(!image)return NULL;
        unsigned columns=(unsigned)(image->w/g->tile_width),tile=id-256;
        if(tile>=columns*(unsigned)(image->h/g->tile_height))return NULL;
        rectangle=(SDL_Rect){(int)(tile%columns)*g->tile_width,(int)(tile/columns)*g->tile_height,
            g->tile_width,g->tile_height};
    }
    for(unsigned i=0;i<CACHE;++i) {
        Entry *e=&g->cache[i];
        if(!g->disable_tile_cache && e->used && e->raw==raw && e->id==id && e->width==width && e->height==height)return &e->prepared;
    }
    size_t needed=(size_t)width*(size_t)height*12u;
    if(needed>64u*1024u*1024u)return NULL;
    if(g->cache_bytes+needed>64u*1024u*1024u) {
        for(unsigned i=0;i<CACHE;++i) {
            dispose_entry(&g->cache[i]);
        }
        g->cache_bytes=0;g->next=0;
    }
    Entry *e=&g->cache[g->next++%CACHE];SvPreparedResource candidate={0};
    SvResourceResult result=raw?sv_resource_prepare_raw(g->sheets[sheet].image,&rectangle,width,height,
        &g->preparation,&candidate):sv_resource_prepare_tile(g->sheets[sheet].image,&rectangle,width,height,
        &g->sheets[sheet].masks,&g->preparation,&candidate);
    if(result!=SV_RESOURCE_OK)return NULL;
    if(e->used)g->cache_bytes-=(size_t)e->width*(size_t)e->height*12u;
    dispose_entry(e);g->cache_bytes+=needed;
    *e=(Entry){.used=true,.raw=raw,.id=id,.width=width,.height=height,.prepared=candidate};return &e->prepared;
}
const SvPreparedResource *sv_graphics_tile(SvGraphics *g,uint32_t id,int w,int h){return prepared(g,false,id,w,h);}
const SvPreparedResource *sv_graphics_raw(SvGraphics *g,unsigned id,int w,int h){return prepared(g,true,id,w,h);}
static bool validate_preparation(SvGraphics *g,int width,int height)
{
    if(width<1 || height<1 || width>8192 || height>8192)return false;
    if(!g->mode)return true;
    for(unsigned i=256;i<GLYPHS;++i)
        if(g->mapped[i] && !sv_graphics_tile(g,i,width,height))return false;
    for(unsigned i=0;i<RAW;++i)
        if(g->raw[i].present && !sv_graphics_raw(g,i,width,height))return false;
    return sv_graphics_tile(g,256,width,height)!=NULL;
}
static void policy_generation(SvGraphics *g,int width,int height)
{
    g->generation=hash(g->preparation.source_generation,&g->preparation.profile_generation,sizeof(uint64_t));
    int key[]={g->mode,(int)g->preparation.filter,g->preparation.outline_radius,width,height};
    g->generation=hash(g->generation,key,sizeof(key));
}
SvGraphics *sv_graphics_reprepare(const SvGraphics *active,SvResourceFilter filter,
    int outline,int width,int height,char *error,size_t error_size)
{
    error_text(error,error_size,"");
    if(!active || filter<SV_RESOURCE_NEAREST || filter>SV_RESOURCE_PIXELART || outline< -1 || outline>6) {
        error_text(error,error_size,"Invalid graphics preparation policy; previous stack retained");return NULL;
    }
    SvGraphics *g=malloc(sizeof(*g));if(!g)return NULL;
    *g=*active;memset(g->cache,0,sizeof(g->cache));g->next=0;g->cache_bytes=0;g->renderer=NULL;
    for(unsigned i=0;i<SHEETS;++i)g->sheets[i].image=NULL;
    for(unsigned i=0;i<SHEETS;++i)if(active->sheets[i].image) {
        g->sheets[i].image=SDL_DuplicateSurface(active->sheets[i].image);
        if(!g->sheets[i].image)goto failure;
    }
    g->preparation.filter=filter;g->preparation.outline_radius=outline;
    policy_generation(g,width,height);
    if(validate_preparation(g,width,height))return g;
failure:
    error_text(error,error_size,"Graphics reprepare failed; previous stack retained");
    sv_graphics_destroy(g);return NULL;
}
SvGraphics *sv_graphics_prepare(const char *user,const char *library,const SvSettingsConfig *config,
    SvPreferences *preferences,const SvPreferenceContext *context,int width,int height,char *error,size_t error_size)
{
    error_text(error,error_size,"");
    if(!user || !library || !config || !preferences || width<1 || height<1)return NULL;
    SvGraphics *g=calloc(1,sizeof(*g));if(!g)return NULL;
    memset(g->subsets,-1,sizeof(g->subsets));
    g->mode=config->graphics;g->generation=UINT64_C(1469598103934665603);
    SDL_strlcpy(g->name,config->graphic_tiles,sizeof(g->name));
    if(!g->mode) {
        g->preparation=(SvResourcePreparation){g->generation,0,
            sv_resource_filter(config->graphics_filter,SV_RESOURCE_LINEAR),config->graphics_force_outline};
        policy_generation(g,width,height);
        if(validate_preparation(g,width,height))return g;
        goto failure;
    }
    SvPreferences *candidate=sv_preferences_clone(preferences,true);
    if(!candidate)goto failure;
    sv_preferences_reset_graphics(candidate);
    char *end;long w=strtol(g->name,&end,10);
    if(end==g->name || *end!='x' || w<1 || w>8192)goto failed_candidate;
    const char *hs=end+1;long h=strtol(hs,&end,10);
    if(end==hs || h<1 || h>8192 || g->mode<1 || g->mode>2)goto failed_candidate;
    g->tile_width=(int)w;g->tile_height=(int)h;
    char base[SV_FILE_PATH];
    if(snprintf(base,sizeof(base),"%s.bmp",g->name)>=(int)sizeof(base) || !image(g,0,user,library,base))goto failed_candidate;
    char names[10][SV_FILE_PATH]={{0}};
    discover(user,g->name,config->graphic_subsets,names);discover(library,g->name,config->graphic_subsets,names);
    for(int i=0;i<10;++i)if(names[i][0] && !image(g,i+1,user,library,names[i]))goto failed_candidate;
    SvPreferenceContext ctx=context?*context:sv_preferences_default_context();
    ctx.graphics_enabled=true;ctx.graphics_build=true;ctx.sdl3_build=true;ctx.character_offset=256;ctx.subtileset=-1;
    SvPreferenceStatus status=load_preferences(candidate,user,library,g->name,&ctx);
    if(status==SV_PREF_FATAL)goto failed_candidate;
    if(status==SV_PREF_UNAVAILABLE)error_text(error,error_size,"Graphics PRF missing; using sheet defaults");
    else if(status==SV_PREF_PARTIAL)error_text(error,error_size,"Graphics PRF partially loaded; inspect preference diagnostics");
    if(status!=SV_PREF_UNAVAILABLE)for(int i=0;i<10;++i)if(names[i][0]) {
        char *dot=strrchr(names[i],'.');if(dot)*dot=0;
        ctx.subtileset=i;
        if(load_preferences(candidate,user,library,names[i],&ctx)==SV_PREF_FATAL)goto failed_candidate;
    }
    if(!tables(g,candidate))goto failed_candidate;
    uint64_t profile=UINT64_C(1469598103934665603);
    for(size_t i=0;i<sv_preferences_mapping_count(candidate);++i) {
        const SvPreferenceMapping *m=sv_preferences_mapping(candidate,i);
        uint32_t key[]={(unsigned char)m->family,m->index,(uint32_t)m->subtileset,m->attribute,m->character};
        profile=hash(profile,key,sizeof(key));profile=hash(profile,m->fields,sizeof(m->fields));
        profile=hash(profile,m->source.path,strlen(m->source.path));
    }
    for(size_t i=0;i<sv_preferences_record_count(candidate);++i) {
        const SvPreferenceRecord *record=sv_preferences_record(candidate,i);
        profile=hash(profile,record->source.path,strlen(record->source.path));
        profile=hash(profile,record->text,strlen(record->text));
    }
    g->preparation=(SvResourcePreparation){g->generation,profile,
        sv_resource_filter(config->graphics_filter,SV_RESOURCE_LINEAR),config->graphics_force_outline};
    policy_generation(g,width,height);
    for(size_t i=0;i<sv_preferences_mapping_count(candidate);++i) {
        const SvPreferenceMapping *m=sv_preferences_mapping(candidate,i);
        if((m->family=='R'||m->family=='K'||m->family=='F'||m->family=='U') && m->character<GLYPHS)
            g->mapped[m->character]=true;
    }
    if(!validate_preparation(g,width,height))goto failed_candidate;
    sv_preferences_swap(preferences,candidate);sv_preferences_destroy(candidate);return g;
failed_candidate:
    sv_preferences_destroy(candidate);
failure:
    error_text(error,error_size,"Graphics candidate could not load or prepare; previous stack retained");
    sv_graphics_destroy(g);return NULL;
}
static int draw(SvGraphics *g,SDL_Renderer *r,const SvPreparedResource *p,const SDL_FRect *rect,SDL_Color fg,SDL_Color outline)
{
    if(!p)return 0;
    if(g->renderer!=r){sv_graphics_reset_renderer(g);g->renderer=r;}
    Entry *e=NULL;
    for(unsigned i=0;i<CACHE;++i)if(&g->cache[i].prepared==p){e=&g->cache[i];break;}
    if(!e)return -1;
    SDL_Surface *layers[]={p->authored,p->recolour,p->outline};
    SDL_Color colors[]={{255,255,255,255},fg,outline};
    for(unsigned i=0;i<3;++i)if(layers[i]) {
        if(!e->textures[i]) {
            if(!SDL_SetSurfaceColorMod(layers[i],255,255,255) || !SDL_SetSurfaceAlphaMod(layers[i],255))return -1;
            e->textures[i]=SDL_CreateTextureFromSurface(r,layers[i]);
            if(!e->textures[i] || !SDL_SetTextureScaleMode(e->textures[i],SDL_SCALEMODE_NEAREST) ||
               !SDL_SetTextureBlendMode(e->textures[i],SDL_BLENDMODE_BLEND))return -1;
        }
        if(!SDL_SetTextureColorMod(e->textures[i],colors[i].r,colors[i].g,colors[i].b) ||
           !SDL_SetTextureAlphaMod(e->textures[i],colors[i].a) ||
           !SDL_RenderTexture(r,e->textures[i],NULL,rect))return -1;
    }
    return 1;
}
int sv_graphics_draw(SvGraphics *g,SDL_Renderer *r,uint32_t id,const SDL_FRect *rect,SDL_Color fg,SDL_Color outline)
{
    if(!r || !rect)return -1;
    const SvPreparedResource *p=sv_graphics_tile(g,id,(int)rect->w,(int)rect->h);
    if(!p)return id<256 || !g || !g->mode?0:-1;
    return draw(g,r,p,rect,fg,outline);
}
int sv_graphics_draw_raw(SvGraphics *g,SDL_Renderer *r,unsigned id,int w,int h,float x,float y)
{
    const SvPreparedResource *p=sv_graphics_raw(g,id,w,h);if(!p)return 0;
    SDL_FRect rectangle={x,y,(float)p->authored->w,(float)p->authored->h};
    return draw(g,r,p,&rectangle,(SDL_Color){255,255,255,255},(SDL_Color){0,0,0,255});
}
SvPreferenceResult sv_graphics_bootstrap(SvPreferences *preferences,const SvPreferenceContext *context,void *opaque)
{
    SvGraphicsBootstrap *b=opaque;
    if(!b || !b->map || !b->config || !b->active)return (SvPreferenceResult){SV_PREF_FATAL,0,1};
    SvPreferenceResult result=sv_map_font_bootstrap(preferences,context,b->map);
    if(result.status==SV_PREF_FATAL || result.status==SV_PREF_UNAVAILABLE)return result;
    size_t before=sv_preferences_diagnostic_count(preferences);
    SvGraphics *candidate=sv_graphics_prepare(b->map->user,b->map->library,b->config,
        preferences,context,b->width,b->height,b->error,sizeof(b->error));
    if(!candidate) {
        fprintf(stderr,"SV graphics: %s; startup font-only fallback\n",b->error);
        SvSettingsConfig disabled=*b->config;disabled.graphics=0;
        candidate=sv_graphics_prepare(b->map->user,b->map->library,&disabled,preferences,
            context,b->width,b->height,NULL,0);
        result.status=SV_PREF_PARTIAL;++result.diagnostics;
    }
    if(!candidate)return (SvPreferenceResult){SV_PREF_FATAL,0,1};
    size_t diagnostics=sv_preferences_diagnostic_count(preferences)-before;
    if(diagnostics){result.status=SV_PREF_PARTIAL;result.diagnostics+=(unsigned)diagnostics;}
    if(*b->error && sv_graphics_mode(candidate)) {
        fprintf(stderr,"SV graphics: %s\n",b->error);
        result.status=SV_PREF_PARTIAL;
    }
    sv_graphics_destroy(*b->active);*b->active=candidate;
    sv_map_font_definitions(&b->map->definitions,preferences);return result;
}

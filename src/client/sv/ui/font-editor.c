#include "ui/font-editor.h"
#include "ui/map-renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct SvFontEditor {
    SvFontSelection *selection;
    const SvMapFont *map;
    SvSettingsConfig opening,edited,disk;
    SvFilePath destination;
    char user[SV_FILE_PATH],library[SV_FILE_PATH],fields[2][1024],message[256];
    bool open;int field;SvFontRole role;
    SDL_Renderer *renderer;
    SDL_Texture *glyphs[256],*tile,*raw;
    SDL_Surface *source;
    int cw,ch,source_w,source_h,source_x,source_y;
    unsigned generation;
    SvFontRole cached_role;
    SvResourceFilter cached_filter,cached_graphics;
    int cached_outline;
    SvGraphics *const *graphics;
    SvMapEffects *effects;
    SvMap *composition;
    SvMapRenderer *map_renderer;
    SvMapGeometry geometry;
    uint64_t map_revision,graphics_generation;
    SvFontEditorResourcePrepare resource_prepare;
    void *resource_context;
};
static void clear_cache(SvFontEditor *e)
{
    for (unsigned i=0;i<256;++i) { SDL_DestroyTexture(e->glyphs[i]);e->glyphs[i]=NULL; }
    SDL_DestroyTexture(e->tile);SDL_DestroyTexture(e->raw);e->tile=e->raw=NULL;
    e->generation=0;
    sv_map_renderer_reset(e->map_renderer);
    if(e->graphics)sv_graphics_reset_renderer(*e->graphics);
}
SvFontEditor *sv_font_editor_create(SvFontSelection *s,const char *user,const char *library,
    const SvSettingsConfig *config,const SvMapFont *map)
{
    if (!s || !user || !library || strlen(user)>=SV_FILE_PATH || strlen(library)>=SV_FILE_PATH) return NULL;
    SvFontEditor *e=calloc(1,sizeof(*e));if(!e)return NULL;
    e->selection=s;e->map=map;e->opening=*config;e->edited=*config;
    e->opening.records=e->opening.records_tail=e->edited.records=e->edited.records_tail=NULL;
    strcpy(e->user,user);strcpy(e->library,library);
    if(!sv_file_path(&e->destination,user,"sv/tomenet.cfg",SV_FILE_PROFILE)){free(e);return NULL;}
    sv_settings_config_defaults(&e->disk);
    sv_settings_config_load(&e->disk,e->destination.path);
    sv_settings_config_dispose(&e->disk);
    char relative[4096];
    if (config->graphics && snprintf(relative,sizeof(relative),"xtra/graphics/%s.bmp",config->graphic_tiles)<(int)sizeof(relative)) {
        SvFilePath path;
        if(sv_file_resource(&path,user,library,relative)==SV_FILE_OK)
            e->source=sv_resource_load_image(path.path,NULL);
        if(e->source && e->source->format!=SDL_PIXELFORMAT_RGBA32) {
            SDL_Surface *rgba=SDL_ConvertSurface(e->source,SDL_PIXELFORMAT_RGBA32);
            SDL_DestroySurface(e->source);e->source=rgba;
        }
        if(e->source && sscanf(config->graphic_tiles,"%dx%d",&e->source_w,&e->source_h)!=2) {
            SDL_DestroySurface(e->source);e->source=NULL;
        }
        /* Select a nonempty authored cell, avoiding the blank atlas sentinel. */
        if(e->source && e->source_w>0 && e->source_h>0) {
            SvResourceMasks masks=sv_resource_default_masks(config->graphics==2);
            bool found=false;
            for(int y=0;y+e->source_h<=e->source->h && !found;y+=e->source_h)
                for(int x=0;x+e->source_w<=e->source->w && !found;x+=e->source_w) {
                    uint32_t first=0;unsigned covered=0;bool distinct=false,visible=false;
                    for(int py=y;py<y+e->source_h;++py)
                        for(int px=x;px<x+e->source_w;++px) {
                            const unsigned char *p=(const unsigned char *)e->source->pixels+py*e->source->pitch+px*4;
                            uint32_t rgb=(uint32_t)p[0]<<16|(uint32_t)p[1]<<8|p[2];
                            if(p[3] && rgb!=masks.background_rgb && rgb!=masks.outline_rgb) {
                                if(!covered)first=rgb;
                                else if(rgb!=first)distinct=true;
                                if(rgb!=0x1d211c && rgb!=0x3e3d00 &&
                                    (p[0]>100 || p[1]>100 || p[2]>100))visible=true;
                                ++covered;
                            }
                        }
                    if(distinct && visible && covered>=8) {e->source_x=x;e->source_y=y;found=true;}
                }
        }
        if(!e->source) {
            strcpy(e->message,"Selected tiles unavailable: font-only map. Requested tiles/enabled retained.");
            fprintf(stderr,"SV resources: %s\n",e->message);
        }
    }
    return e;
}
void sv_font_editor_destroy(SvFontEditor *e)
{ if(e){clear_cache(e);sv_map_renderer_destroy(e->map_renderer);sv_map_destroy(e->composition);SDL_DestroySurface(e->source);free(e);} }
void sv_font_editor_rendering(SvFontEditor *e,SvGraphics *const *graphics,SvMapEffects *effects)
{
    if(!e)return;
    e->graphics=graphics;e->effects=effects;
    if(!e->composition)e->composition=sv_map_create(!strcmp(e->edited.layout,"wide")?44:22,1);
    if(!e->map_renderer)e->map_renderer=sv_map_renderer_create();
    e->map_revision=0;e->graphics_generation=0;
}
void sv_font_editor_resource_prepare(SvFontEditor *e,SvFontEditorResourcePrepare prepare,void *context)
{if(e){e->resource_prepare=prepare;e->resource_context=context;}}
static bool resource_change(SvFontEditor *e,const SvSettingsConfig *config,SvFontEditorResourceAction action)
{
    if(!e->resource_prepare || e->resource_prepare(e->resource_context,config,e->role,action))return true;
    SDL_strlcpy(e->message,SDL_GetError(),sizeof(e->message));return false;
}
bool sv_font_editor_open(const SvFontEditor *e){return e && e->open;}
SvFontRole sv_font_editor_role(const SvFontEditor *e){return e?e->role:SV_FONT_TEXT;}
static void fields(SvFontEditor *e)
{
    SDL_strlcpy(e->fields[0],sv_font_selection_requested(e->selection,e->role),sizeof(e->fields[0]));
    SDL_strlcpy(e->fields[1],sv_font_selection_profile(e->selection,e->role),sizeof(e->fields[1]));
}
static void cycle(char field[16])
{
    SvResourceFilter current=sv_resource_filter(field,SV_RESOURCE_NEAREST);
    SDL_strlcpy(field,sv_resource_filter_name((SvResourceFilter)((current+1)%3)),16);
}
bool sv_font_editor_event(SvFontEditor *e,const SDL_Event *event)
{
    if(!e || !event)return false;
    if(event->type==SDL_EVENT_RENDER_DEVICE_RESET){clear_cache(e);return e->open;}
    if(!e->open) {
        if(event->type!=SDL_EVENT_KEY_DOWN || event->key.repeat || event->key.key!=SDLK_F6)return false;
        e->open=true;fields(e);SDL_StartTextInput(SDL_GetKeyboardFocus());return true;
    }
    if(e->composition && e->role==SV_FONT_MAP &&
       (event->type==SDL_EVENT_MOUSE_MOTION || event->type==SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event->type==SDL_EVENT_WINDOW_MOUSE_LEAVE || event->type==SDL_EVENT_WINDOW_FOCUS_LOST)) {
        SDL_Window *window=SDL_GetKeyboardFocus();
        float density=window?SDL_GetWindowPixelDensity(window):1;
        float x=0,y=0;
        if(event->type==SDL_EVENT_MOUSE_MOTION){x=event->motion.x;y=event->motion.y;}
        else if(event->type==SDL_EVENT_MOUSE_BUTTON_DOWN){x=event->button.x;y=event->button.y;}
        int cx=0,cy=0;
        bool visible=event->type!=SDL_EVENT_WINDOW_MOUSE_LEAVE && event->type!=SDL_EVENT_WINDOW_FOCUS_LOST &&
            e->geometry.output_scale>0 && sv_map_hit(&e->geometry,x*density/e->geometry.output_scale,
                y*density/e->geometry.output_scale,&cx,&cy);
        if(event->type==SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if(event->button.button==SDL_BUTTON_LEFT)
                sv_map_set_indicator(e->composition,1,SV_MAP_TARGET,cx,cy,visible);
            else if(event->button.button==SDL_BUTTON_RIGHT)
                sv_map_set_indicator(e->composition,1,SV_MAP_TARGET,0,0,false);
        } else {
            sv_map_set_indicator(e->composition,1,SV_MAP_HOVER,cx,cy,visible);
            if(event->type==SDL_EVENT_WINDOW_FOCUS_LOST)
                sv_map_set_indicator(e->composition,1,SV_MAP_TARGET,0,0,false);
        }
        return true;
    }
    if(event->type==SDL_EVENT_TEXT_INPUT) {
        size_t have=strlen(e->fields[e->field]),add=strlen(event->text.text);
        if(add<sizeof(e->fields[0])-have)memcpy(e->fields[e->field]+have,event->text.text,add+1);
        return true;
    }
    if(event->type!=SDL_EVENT_KEY_DOWN)return event->type==SDL_EVENT_KEY_UP || event->type==SDL_EVENT_TEXT_EDITING;
    if(event->key.repeat && event->key.key!=SDLK_BACKSPACE)return true;
    if(event->key.key==SDLK_ESCAPE) {
        SvFontSelectionResult a=sv_font_selection_cancel(e->selection,SV_FONT_TEXT);
        SvFontSelectionResult b=sv_font_selection_cancel(e->selection,SV_FONT_MAP);
        if(a!=SV_FONT_SELECTION_OK || b!=SV_FONT_SELECTION_OK) {
            SDL_strlcpy(e->message,sv_font_selection_error(e->selection),sizeof(e->message));return true;
        }
        if(!resource_change(e,&e->opening,SV_EDITOR_RESOURCE_CANCEL))return true;
        e->edited=e->opening;clear_cache(e);e->open=false;
        SDL_StopTextInput(SDL_GetKeyboardFocus());return true;
    }
    if(event->key.key==SDLK_F7){e->role=e->role==SV_FONT_TEXT?SV_FONT_MAP:SV_FONT_TEXT;fields(e);return true;}
    if(event->key.key==SDLK_TAB){e->field^=1;return true;}
    if(event->key.key==SDLK_BACKSPACE){
        size_t size=strlen(e->fields[e->field]);
        if(size){do{--size;}while(size && ((unsigned char)e->fields[e->field][size]&0xc0)==0x80);e->fields[e->field][size]=0;}
        return true;
    }
    if(event->key.key==SDLK_F8 || event->key.key==SDLK_F9 || event->key.key==SDLK_F10) {
        SvSettingsConfig candidate=e->edited;
        if(event->key.key==SDLK_F8)cycle(candidate.pcf_filter);
        if(event->key.key==SDLK_F9)cycle(candidate.graphics_filter);
        if(event->key.key==SDLK_F10)candidate.graphics_force_outline=candidate.graphics_force_outline>=6?-1:candidate.graphics_force_outline+1;
        if(resource_change(e,&candidate,SV_EDITOR_RESOURCE_PREVIEW)){e->edited=candidate;clear_cache(e);}
        return true;
    }
    if(event->key.key==SDLK_RETURN || event->key.key==SDLK_KP_ENTER) {
        SvFontSelectionResult result=sv_font_selection_preview(e->selection,e->role,e->fields[0],e->fields[1]);
        SDL_strlcpy(e->message,result==SV_FONT_SELECTION_OK?"Preview active; Ctrl+S saves, Esc cancels.":sv_font_selection_error(e->selection),sizeof(e->message));
        clear_cache(e);return true;
    }
    if(event->key.key==SDLK_S && (event->key.mod&SDL_KMOD_CTRL)) {
        SvFontSelectionResult result=sv_font_selection_save(e->selection,e->role);
        if(result==SV_FONT_SELECTION_OK) {
            unsigned saved=0,conflicts=0;
            unsigned mask=SV_CONFIG_PCF_FILTER|SV_CONFIG_GRAPHICS_FILTER|SV_CONFIG_OUTLINE;
            SvFileResult written=sv_settings_config_save_resources(&e->destination,&e->disk,&e->edited,mask,&saved,&conflicts);
            if(saved&SV_CONFIG_PCF_FILTER){strcpy(e->disk.pcf_filter,e->edited.pcf_filter);strcpy(e->opening.pcf_filter,e->edited.pcf_filter);}
            if(saved&SV_CONFIG_GRAPHICS_FILTER){strcpy(e->disk.graphics_filter,e->edited.graphics_filter);strcpy(e->opening.graphics_filter,e->edited.graphics_filter);}
            if(saved&SV_CONFIG_OUTLINE){e->disk.graphics_force_outline=e->edited.graphics_force_outline;e->opening.graphics_force_outline=e->edited.graphics_force_outline;}
            if(written==SV_FILE_OK) {
                strcpy(e->opening.pcf_filter,e->edited.pcf_filter);strcpy(e->opening.graphics_filter,e->edited.graphics_filter);
                e->opening.graphics_force_outline=e->edited.graphics_force_outline;
                strcpy(e->message,"Saved selected role and resource filters.");
            } else strcpy(e->message,conflicts?"External filter edit retained; preview remains unsaved.":"Filter Save failed; preview remains active and unsaved.");
            if(!resource_change(e,&e->opening,SV_EDITOR_RESOURCE_SAVED))return true;
        } else SDL_strlcpy(e->message,sv_font_selection_error(e->selection),sizeof(e->message));
        return true;
    }
    return true;
}
static SDL_Texture *texture(SDL_Renderer *renderer,SDL_Surface *surface)
{
    if(!surface)return NULL;
    SDL_Texture *t=SDL_CreateTextureFromSurface(renderer,surface);
    SDL_DestroySurface(surface);return t;
}
static bool prepare(SvFontEditor *e,SDL_Renderer *renderer,int width,int height)
{
    SvFont *font=sv_font_selection_font(e->selection,e->role);
    SvResourceFilter pcf=sv_resource_filter(e->edited.pcf_filter,SV_RESOURCE_NEAREST);
    SvResourceFilter gfx=sv_resource_filter(e->edited.graphics_filter,SV_RESOURCE_LINEAR);
    unsigned generation=sv_font_selection_generation(e->selection,e->role);
    if(e->cached_role==e->role && e->generation==generation && e->cw==width && e->ch==height && e->renderer==renderer &&
        e->cached_filter==pcf && e->cached_graphics==gfx && e->cached_outline==e->edited.graphics_force_outline)return true;
    clear_cache(e);e->renderer=renderer;e->cw=width;e->ch=height;
    SvFontMetrics metrics;
    if(!sv_font_metrics(font,&metrics))return false;
    SDL_Color white={255,255,255,255};
    for(unsigned i=0;i<256;++i) {
        SDL_Surface *surface=sv_font_render_glyph(font,i,metrics.pcf?metrics.width:width,
            metrics.pcf?metrics.height:height,white,NULL);
        if(!surface)return false;
        if(metrics.pcf) {
            SvPreparedResource prepared={0};
            SvResourceResult result=sv_resource_prepare_coverage(surface,width,height,pcf,&prepared);
            SDL_DestroySurface(surface);
            if(result!=SV_RESOURCE_OK)return false;
            surface=sv_resource_composite(&prepared,white,(SDL_Color){0,0,0,255});
            sv_resource_prepared_dispose(&prepared);
        }
        e->glyphs[i]=texture(renderer,surface);if(!e->glyphs[i])return false;
    }
    if(e->role==SV_FONT_MAP && e->source && e->source_w>0 && e->source_h>0) {
        SvPreparedResource prepared={0};SvResourceMasks masks=sv_resource_default_masks(e->edited.graphics==2);
        SvResourcePreparation policy={.filter=gfx,.outline_radius=e->edited.graphics_force_outline};
        SDL_Rect cell={e->source_x,e->source_y,e->source_w,e->source_h};
        if(sv_resource_prepare_tile(e->source,&cell,width,height,&masks,&policy,&prepared)!=SV_RESOURCE_OK)return false;
        e->tile=texture(renderer,sv_resource_composite(&prepared,white,(SDL_Color){0,0,0,255}));
        sv_resource_prepared_dispose(&prepared);
        if(sv_resource_prepare_raw(e->source,&cell,width*2,height*2,&policy,&prepared)!=SV_RESOURCE_OK)return false;
        e->raw=texture(renderer,sv_resource_composite(&prepared,white,(SDL_Color){0,0,0,255}));
        sv_resource_prepared_dispose(&prepared);
    }
    e->generation=generation;e->cached_role=e->role;e->cached_filter=pcf;e->cached_graphics=gfx;e->cached_outline=e->edited.graphics_force_outline;
    return true;
}
static int map_visual(void *context,SDL_Renderer *renderer,const SvMapVisual *visual,
    const SDL_FRect *rect,SDL_Color colour)
{
    SvFontEditor *e=context;
    return sv_graphics_draw(e->graphics?*e->graphics:NULL,renderer,visual->glyph,rect,
        colour,(SDL_Color){0,0,0,255});
}
static SDL_Color map_colour(void *context,unsigned char attribute,int x,int y,bool terrain)
{
    (void)terrain;
    SvFontEditor *e=context;
    SvMapColour c=sv_map_effects_colour(e->effects,attribute,x,y,SV_EFFECT_MAP);
    return (SDL_Color){c.r,c.g,c.b,255};
}
static bool map_overlay(void *context,int x,int y,SvMapVisual *visual)
{
    SvFontEditor *e=context;SvEffectVisual effect;
    if(!sv_map_effects_overlay(e->effects,x,y,&effect))return false;
    *visual=(SvMapVisual){.glyph=effect.glyph,.attribute=effect.attribute,.present=true,
        .overlay_mode=(SvMapOverlayMode)effect.mode};
    return true;
}
static bool composition(SvFontEditor *e,SDL_Renderer *renderer,SDL_FRect area,float scale)
{
    if(!e->graphics || !e->map_renderer || !e->composition || !e->map)return true;
    uint64_t graphics_generation=sv_graphics_generation(*e->graphics);
    if(e->map_revision!=e->map->revision || e->graphics_generation!=graphics_generation) {
        sv_map_reset(e->composition,sv_map_rows(e->composition),1);
        /* Resource projection, not invented live-world state: font IDs and the
         * actual prepared F/R/K definitions share the production compositor. */
        for(unsigned i=0;i<512;++i) {
            SvVisualDefinition d=e->map->definitions.features[i];
            SvMapCell cell={.background_rgb=0x10141c,
                .terrain={.glyph=d.character,.attribute=d.attribute,.present=d.character!=0}};
            if(i<256)cell.foreground=(SvMapVisual){.glyph=i,.attribute=1,.present=true};
            if(!sv_map_apply(e->composition,1,(int)(i%66),(int)(i/66),7,&cell))return false;
        }
        unsigned position=528;
        for(unsigned i=0;i<1280 && position<(unsigned)(66*sv_map_rows(e->composition));++i) {
            const SvVisualDefinition definitions[2]={e->map->definitions.monsters[i],e->map->definitions.kinds[i]};
            for(unsigned j=0;j<2 && position<(unsigned)(66*sv_map_rows(e->composition));++j) {
                if(!definitions[j].character)continue;
                SvMapCell cell={.background_rgb=0x10141c,.foreground={.glyph=definitions[j].character,
                    .attribute=definitions[j].attribute,.present=true}};
                if(!sv_map_apply(e->composition,1,(int)(position%66),(int)(position/66),7,&cell))return false;
                ++position;
            }
        }
        e->map_revision=e->map->revision;e->graphics_generation=graphics_generation;
        if(e->effects) {
            /* Baseline KIDX_PO_RAIN/W1/W2/E1/E2/SNOW/SAND definitions. */
            uint32_t rain[5];
            for(unsigned i=0;i<5;++i)rain[i]=e->map->definitions.kinds[822+i].character;
            sv_map_effects_weather_glyphs(e->effects,rain,e->map->definitions.kinds[827].character,
                e->map->definitions.kinds[828].character);
        }
    }
    SvFont *font=sv_font_selection_font(e->selection,SV_FONT_MAP);SvFontMetrics metrics;
    if(!sv_font_metrics(font,&metrics))return false;
    if(!sv_map_geometry(&e->geometry,area,scale,sv_map_rows(e->composition),metrics.width,metrics.height)) {
        e->geometry=(SvMapGeometry){0};return true;
    }
    SvMapRenderProvider provider={.context=e,.visual=map_visual,
        .colour=e->effects?map_colour:NULL,.overlay=e->effects?map_overlay:NULL};
    return sv_map_renderer_draw(e->map_renderer,renderer,e->composition,&e->geometry,font,
        sv_font_selection_generation(e->selection,SV_FONT_MAP),
        sv_resource_filter(e->edited.pcf_filter,SV_RESOURCE_NEAREST),e->edited.palette,&provider);
}
bool sv_font_editor_draw(SvFontEditor *e,SDL_Renderer *renderer)
{
    if(!e || !e->open)return false;
    SvFont *text=sv_font_selection_font(e->selection,SV_FONT_TEXT);
    int w,h;if(!SDL_GetCurrentRenderOutputSize(renderer,&w,&h))return false;
    float scale=SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer));if(scale<=0)scale=1;
    SDL_SetRenderDrawColor(renderer,16,20,28,255);SDL_RenderClear(renderer);
    const SDL_Color ink={235,225,195,255};char line[2300];
    const char *role=e->role==SV_FONT_TEXT?"Text":"Map";
    snprintf(line,sizeof(line),"Fonts: %s   F7 role | Tab field | Enter preview | Ctrl+S Save | Esc Cancel",role);
    if(!sv_font_draw(text,renderer,line,16,12,scale,ink))return false;
    snprintf(line,sizeof(line),"%c Font: %s",e->field==0?'>':' ',e->fields[0]);sv_font_draw(text,renderer,line,16,42,scale,ink);
    snprintf(line,sizeof(line),"%c Profile: %s",e->field==1?'>':' ',e->fields[1]);sv_font_draw(text,renderer,line,16,72,scale,ink);
    snprintf(line,sizeof(line),"Effective: %s",sv_font_resource(sv_font_selection_font(e->selection,e->role)));
    sv_font_draw(text,renderer,line,16,102,scale,ink);
    snprintf(line,sizeof(line),"F8 PCF: %s | F9 graphics: %s | F10 outline: %d%s",
        e->edited.pcf_filter,e->edited.graphics_filter,e->edited.graphics_force_outline,
        !strcmp(e->edited.pcf_filter,"pixelart") || !strcmp(e->edited.graphics_filter,"pixelart")?" (effective Nearest on CPU)":"");
    sv_font_draw(text,renderer,line,16,132,scale,ink);
    sv_font_draw(text,renderer,e->message,16,162,scale,ink);
    sv_font_draw(text,renderer,e->role==SV_FONT_TEXT?"Text font glyph preview":
        "Map glyphs and current font-PRF definitions; tile / raw resource preview",16,192,scale,ink);
    SvFontMetrics metrics;
    if(!sv_font_metrics(sv_font_selection_font(e->selection,e->role),&metrics))return false;
    int ch=(h-260)/16;if(ch<1)ch=1;
    int cw=ch*metrics.width/metrics.height;if(cw<1)cw=1;
    if(cw*20>w-32){cw=(w-32)/20;if(cw<1)cw=1;ch=cw*metrics.height/metrics.width;if(ch<1)ch=1;}
    if(!prepare(e,renderer,cw,ch))return false;
    for(unsigned i=0;i<256;++i) {
        SDL_FRect dst={16+(i%16)*cw,230+(i/16)*ch,cw,ch};
        if(!SDL_RenderTexture(renderer,e->glyphs[i],NULL,&dst))return false;
    }
    if(e->role==SV_FONT_MAP && e->map)for(unsigned i=0;i<16;++i) {
        unsigned id=e->map->definitions.features[i].character;
        SDL_FRect dst={16+17*cw,230+i*ch,cw,ch};
        if(id && id<256) {
            uint32_t rgb=e->edited.palette[e->map->definitions.features[i].attribute%16];
            SDL_SetTextureColorMod(e->glyphs[id],(unsigned char)(rgb>>16),
                (unsigned char)(rgb>>8),(unsigned char)rgb);
            bool drawn=SDL_RenderTexture(renderer,e->glyphs[id],NULL,&dst);
            SDL_SetTextureColorMod(e->glyphs[id],255,255,255);
            if(!drawn)return false;
        }
    }
    SDL_FRect tile={16+19*cw,230,cw,ch},raw={16+19*cw,230+2*ch,2*cw,2*ch};
    if(e->tile)SDL_RenderTexture(renderer,e->tile,NULL,&tile);
    if(e->raw)SDL_RenderTexture(renderer,e->raw,NULL,&raw);
    if(e->role==SV_FONT_MAP && e->graphics) {
        int left=16+22*cw;
        if(!sv_font_draw(text,renderer,"Active graphics stack | hover / click target",left,206,scale,ink))return false;
        SDL_FRect area={left/scale,230/scale,(w-left-16)/scale,(h-246)/scale};
        if(!composition(e,renderer,area,scale))return false;
    }
    return SDL_RenderPresent(renderer);
}

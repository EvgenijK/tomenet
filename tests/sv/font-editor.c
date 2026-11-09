#define _POSIX_C_SOURCE 200809L
#include "ui/font-editor.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct { SvMapFont map; SvPreferences *preferences; unsigned changes; } MapOwner;
static bool apply_map(void *context,SvFontRole role,SvFont *font)
{
    MapOwner *owner=context;
    if(role!=SV_FONT_MAP)return true;
    SvMapFont candidate=owner->map;
    SvPreferences *preferences=sv_map_font_reload(&candidate,owner->preferences,sv_font_resource(font),NULL);
    if(!preferences)return false;
    sv_preferences_destroy(owner->preferences);
    owner->preferences=preferences;owner->map=candidate;owner->changes++;
    return true;
}
static void key(SvFontEditor *editor,SDL_Keycode keycode,SDL_Keymod modifier)
{
    SDL_Event event={.type=SDL_EVENT_KEY_DOWN};
    event.key.key=keycode;event.key.mod=modifier;
    assert(sv_font_editor_event(editor,&event));
}
static void replace_font(SvFontEditor *editor,const char *name)
{
    for(unsigned i=0;i<SV_SETTINGS_TEXT_MAX;++i)key(editor,SDLK_BACKSPACE,0);
    SDL_Event event={.type=SDL_EVENT_TEXT_INPUT};
    event.text.text=name;
    assert(sv_font_editor_event(editor,&event));
    key(editor,SDLK_RETURN,0);
}
static void draw(SvFontEditor *editor,SDL_Renderer *renderer)
{
    assert(sv_font_editor_draw(editor,renderer));
    SDL_Surface *pixels=SDL_RenderReadPixels(renderer,NULL);
    assert(pixels && pixels->w==1200 && pixels->h==900);
    SDL_DestroySurface(pixels);
}
/* Observe the actual preview only, excluding labels/field values. This is a
 * change assertion, not a fixed pixel oracle for either font or backend. */
static uint64_t preview_pixels(SDL_Renderer *renderer)
{
    SDL_Rect region={16,230,300,300};
    SDL_Surface *capture=SDL_RenderReadPixels(renderer,&region);assert(capture);
    SDL_Surface *rgba=SDL_ConvertSurface(capture,SDL_PIXELFORMAT_RGBA32);assert(rgba);
    SDL_DestroySurface(capture);
    uint64_t hash=UINT64_C(14695981039346656037);
    for(int y=0;y<rgba->h;++y)for(int x=0;x<rgba->w*4;++x) {
        hash^=((const unsigned char *)rgba->pixels)[y*rgba->pitch+x];
        hash*=UINT64_C(1099511628211);
    }
    SDL_DestroySurface(rgba);return hash;
}
int main(int argc,char **argv)
{
    const char *library=argc>1?argv[1]:"lib";
    char user[]="/tmp/sv-c006-editor-XXXXXX",cfg[4096],directory[4096];
    assert(mkdtemp(user));
    assert(SDL_Init(SDL_INIT_VIDEO) && TTF_Init());
    SDL_Window *window=SDL_CreateWindow("SV production font editor",1200,900,0);
    SDL_Renderer *renderer=SDL_CreateRenderer(window,"software");
    assert(window && renderer);
    SvSettingsConfig config;sv_settings_config_defaults(&config);
    SvFontSelection *selection=sv_font_selection_create(user,library,&config);assert(selection);
    MapOwner owner={0};
    assert(sv_map_font_init(&owner.map,user,library,
        sv_font_resource(sv_font_selection_font(selection,SV_FONT_MAP))));
    owner.preferences=sv_preferences_create(user,library,NULL);assert(owner.preferences);
    assert(sv_preferences_bootstrap(owner.preferences,NULL,sv_map_font_bootstrap,&owner.map).status!=SV_PREF_FATAL);
    sv_map_font_definitions(&owner.map.definitions,owner.preferences);
    sv_font_selection_prepare(selection,apply_map,&owner);
    SvFontEditor *editor=sv_font_editor_create(selection,user,library,&config,&owner.map);assert(editor);
    assert(!sv_font_editor_open(editor));
    SvFont *initial_text=sv_font_selection_font(selection,SV_FONT_TEXT);
    SvFont *initial_map=sv_font_selection_font(selection,SV_FONT_MAP);
    key(editor,SDLK_F6,0);draw(editor,renderer);
    uint64_t text_before=preview_pixels(renderer);
    replace_font(editor,"9x15tg.pcf");draw(editor,renderer);
    assert(sv_font_selection_font(selection,SV_FONT_TEXT)!=initial_text && !owner.changes);
    assert(sv_font_selection_font(selection,SV_FONT_MAP)==initial_map);
    assert(preview_pixels(renderer)!=text_before);
    key(editor,SDLK_ESCAPE,0);
    assert(!sv_font_editor_open(editor) && sv_font_selection_font(selection,SV_FONT_TEXT)==initial_text);
    key(editor,SDLK_F6,0);draw(editor,renderer);
    assert(preview_pixels(renderer)==text_before);
    key(editor,SDLK_F7,0);draw(editor,renderer);
    uint64_t map_before=preview_pixels(renderer);
    replace_font(editor,"9x15tg.pcf");draw(editor,renderer);
    assert(owner.changes==1 && sv_font_selection_font(selection,SV_FONT_MAP)!=initial_map);
    assert(preview_pixels(renderer)!=map_before);
    assert(sv_font_selection_font(selection,SV_FONT_TEXT)==initial_text);
    key(editor,SDLK_ESCAPE,0);
    assert(owner.changes==2 && sv_font_selection_font(selection,SV_FONT_MAP)==initial_map);
    /* Closed reset invalidates retained textures; reopen rebuilds actual output. */
    SDL_Event reset={.type=SDL_EVENT_RENDER_DEVICE_RESET};
    assert(!sv_font_editor_event(editor,&reset));
    key(editor,SDLK_F6,0);draw(editor,renderer);
    replace_font(editor,"12x24.pcf");
    key(editor,SDLK_F8,0);key(editor,SDLK_F9,0);key(editor,SDLK_F10,0);
    draw(editor,renderer);
    assert(sv_font_editor_event(editor,&reset));draw(editor,renderer);
    key(editor,SDLK_S,SDL_KMOD_CTRL);
    snprintf(cfg,sizeof(cfg),"%s/sv/tomenet.cfg",user);
    assert(sv_settings_config_load(&config,cfg)==SV_SETTINGS_CONFIG_OK);
    assert(!strcmp(config.map_font,"12x24.pcf") && !strcmp(config.pcf_filter,"linear") &&
        !strcmp(config.graphics_filter,"pixelart") && config.graphics_force_outline==0);
    key(editor,SDLK_ESCAPE,0);
    assert(!sv_font_selection_dirty(selection,SV_FONT_MAP));
    key(editor,SDLK_F6,0);draw(editor,renderer);
    assert(!mkdir(".scratch/sv-c006",0700) || errno==EEXIST);
    SDL_Surface *capture=SDL_RenderReadPixels(renderer,NULL);assert(capture);
    assert(SDL_SaveBMP(capture,".scratch/sv-c006/font-editor.bmp"));
    SDL_DestroySurface(capture);
    /* C007 attaches the real compositor to the existing production editor.
     * This is a resource view, not packet-derived live map evidence. */
    SvPreferences *graphics_preferences=sv_map_font_reload(&owner.map,owner.preferences,
        sv_font_resource(sv_font_selection_font(selection,SV_FONT_MAP)),NULL);
    assert(graphics_preferences);
    char error[256];
    SvGraphics *graphics=sv_graphics_prepare(user,library,&config,graphics_preferences,NULL,
        12,24,error,sizeof(error));assert(graphics && sv_graphics_mode(graphics)==1);
    sv_map_font_definitions(&owner.map.definitions,graphics_preferences);
    SvMapEffects *effects=sv_map_effects_create(1,66,!strcmp(config.layout,"wide")?44:22,1);assert(effects);
    sv_font_editor_rendering(editor,&graphics,effects);
    draw(editor,renderer);
    SDL_Surface *before=SDL_RenderReadPixels(renderer,NULL);assert(before);
    SDL_Event target={.type=SDL_EVENT_MOUSE_BUTTON_DOWN};
    target.button.button=SDL_BUTTON_LEFT;target.button.x=700;target.button.y=550;
    assert(sv_font_editor_event(editor,&target));draw(editor,renderer);
    SDL_Surface *after=SDL_RenderReadPixels(renderer,NULL);assert(after);
    SDL_Surface *rgba_before=SDL_ConvertSurface(before,SDL_PIXELFORMAT_RGBA32);
    SDL_Surface *rgba_after=SDL_ConvertSurface(after,SDL_PIXELFORMAT_RGBA32);
    assert(rgba_before && rgba_after && rgba_before->pitch==rgba_after->pitch);
    assert(memcmp(rgba_before->pixels,rgba_after->pixels,
        (size_t)rgba_before->pitch*rgba_before->h));
    assert(!mkdir(".scratch/sv-c007",0700) || errno==EEXIST);
    assert(SDL_SaveBMP(after,".scratch/sv-c007/native-map.bmp"));
    SDL_DestroySurface(before);SDL_DestroySurface(after);
    SDL_DestroySurface(rgba_before);SDL_DestroySurface(rgba_after);
    assert(sv_font_editor_event(editor,&reset));draw(editor,renderer);
    key(editor,SDLK_ESCAPE,0);
    sv_font_editor_destroy(editor);
    sv_graphics_destroy(graphics);sv_preferences_destroy(graphics_preferences);sv_map_effects_destroy(effects);
    strcpy(config.graphic_tiles,"missing-c006-sheet");
    editor=sv_font_editor_create(selection,user,library,&config,&owner.map);assert(editor);
    key(editor,SDLK_F6,0);draw(editor,renderer);
    assert(!strcmp(config.graphic_tiles,"missing-c006-sheet") && config.graphics==1);
    key(editor,SDLK_ESCAPE,0);sv_font_editor_destroy(editor);
    sv_font_selection_destroy(selection);
    sv_preferences_destroy(owner.preferences);sv_settings_config_dispose(&config);
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);TTF_Quit();SDL_Quit();
    snprintf(directory,sizeof(directory),"%s/sv",user);
    assert(!unlink(cfg) && !rmdir(directory) && !rmdir(user));
    puts("SV font editor production events/dummy software draw/reset/Save: PASS");
    return 0;
}

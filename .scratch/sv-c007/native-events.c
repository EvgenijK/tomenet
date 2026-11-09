#define _POSIX_C_SOURCE 200809L
#include "login/native-run.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int frame;
static void key(SDL_Keycode value,SDL_Keymod mod)
{SDL_Event e={.type=SDL_EVENT_KEY_DOWN};e.key.key=value;e.key.mod=mod;assert(SDL_PushEvent(&e));}
static bool poll_source(void *context,SvEndpoint *endpoint,bool *failed)
{
    (void)context;(void)endpoint;*failed=false;
    switch(frame++) {
    case 0:key(SDLK_F6,0);break;
    case 1:key(SDLK_F7,0);break;
    case 2: {
        for(int i=0;i<1024;++i)key(SDLK_BACKSPACE,0);
        SDL_Event e={.type=SDL_EVENT_TEXT_INPUT};e.text.text="9x15tg.pcf";assert(SDL_PushEvent(&e));
        key(SDLK_RETURN,0);break;
    }
    case 3:key(SDLK_F9,0);break;
    case 4:key(SDLK_F10,0);break;
    case 5:key(SDLK_S,SDL_KMOD_CTRL);break;
    case 6:key(SDLK_F9,0);break;
    case 7:key(SDLK_ESCAPE,0);break;
    case 8:key(SDLK_F6,0);break;
    case 9:key(SDLK_F9,0);break;
    case 10:key(SDLK_F7,0);break;
    case 11:key(SDLK_S,SDL_KMOD_CTRL);break;
    case 12:key(SDLK_ESCAPE,0);break;
    case 13:key(SDLK_F6,0);break;
    case 14:key(SDLK_F7,0);break;
    case 15: {SDL_Event e={.type=SDL_EVENT_RENDER_DEVICE_RESET};assert(SDL_PushEvent(&e));break;}
    case 16: {
        int count=0;SDL_Window **windows=SDL_GetWindows(&count);assert(windows&&count==1);
        SDL_Renderer *renderer=SDL_GetRenderer(windows[0]);assert(renderer);
        SDL_Surface *s=SDL_RenderReadPixels(renderer,NULL);assert(s);
        assert(SDL_SaveBMP(s,".scratch/sv-c007/native-events.bmp"));
        SDL_DestroySurface(s);SDL_free(windows);break;
    }
    case 17:key(SDLK_ESCAPE,0);break;
    }
    return false;
}
int main(void)
{
    const char *root=".scratch/sv-c007/native-events-profile";
    SvEndpointChoice choice={0};
    int result=sv_endpoint_run((SvEndpointOptions){.root=root,.library="lib",.frames=20,
        .windowed=1,.width=1200,.height=900,.source_poll=poll_source,.selected_endpoint=&choice});
    assert(!result && frame==20);
    SvSettingsConfig config;sv_settings_config_defaults(&config);
    assert(sv_settings_config_load(&config,".scratch/sv-c007/native-events-profile/sv/tomenet.cfg")==SV_SETTINGS_CONFIG_OK);
    assert(!strcmp(config.map_font,"9x15tg.pcf") && !strcmp(config.graphics_filter,"nearest") && config.graphics_force_outline==0);
    sv_settings_config_dispose(&config);
    puts("native rendering events: actual coupled font/filter preview, Save/Cancel and device reset PASS");
    return 0;
}

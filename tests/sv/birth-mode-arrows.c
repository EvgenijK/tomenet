/* Mode arrows follow the displayed two-column grid without confirming it. */
#include "input/native-birth.h"
#include <assert.h>
#include <stdio.h>
static void key(SvBirth *b,SvBirthViewport *v,SDL_Keycode code)
{
    SDL_Event e={.type=SDL_EVENT_KEY_DOWN};e.key.key=code;
    assert(sv_native_birth_event(b,1,0,v,&e)==SV_BIRTH_INPUT_HANDLED);
}
int main(void)
{
    assert(SDL_Init(SDL_INIT_EVENTS));
    SvContactSetup setup={.class_count=1,.race_count=1,.body_step_available=true};
    setup.races[0].choice=1;
    uint32_t flags[4]={0};int16_t stats[6]={0};SvBirth b;
    assert(sv_birth_begin(&b,1,&setup,flags,false,0,false,SV_BIRTH_SEX_MALE,
        false,0,false,0,false,0,false,SV_BIRTH_BODY_NORMAL,false,stats,false,SV_BIRTH_MODE_NORMAL)==SV_OK);
    for(unsigned i=0;i<4;++i)sv_birth_command(&b,1,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM});
    assert(b.owner==SV_BIRTH_STATS);
    for(unsigned i=0;i<6;++i)sv_birth_command(&b,1,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM});
    assert(b.owner==SV_BIRTH_MODE && b.legal_mode_count>=6);
    SvBirthViewport v={.logical_width=1440,.logical_height=900};
    sv_birth_viewport_sync(&v,&b,900);
    b.highlighted_mode=b.legal_modes[0];
    key(&b,&v,SDLK_DOWN);assert(b.highlighted_mode==b.legal_modes[2]);
    key(&b,&v,SDLK_RIGHT);assert(b.highlighted_mode==b.legal_modes[3]);
    key(&b,&v,SDLK_UP);assert(b.highlighted_mode==b.legal_modes[1]);
    key(&b,&v,SDLK_LEFT);assert(b.highlighted_mode==b.legal_modes[0]);
    key(&b,&v,SDLK_LEFT);assert(b.highlighted_mode==b.legal_modes[1]);
    key(&b,&v,SDLK_RIGHT);assert(b.highlighted_mode==b.legal_modes[0]);
    unsigned last=((b.legal_mode_count-1)/2)*2;
    key(&b,&v,SDLK_UP);assert(b.highlighted_mode==b.legal_modes[last]);
    key(&b,&v,SDLK_DOWN);assert(b.highlighted_mode==b.legal_modes[0]);
    key(&b,&v,SDLK_KP_2);assert(b.highlighted_mode==b.legal_modes[2]);
    key(&b,&v,SDLK_KP_6);assert(b.highlighted_mode==b.legal_modes[3]);
    assert(b.owner==SV_BIRTH_MODE && !b.mode_confirmed);
    const SDL_Keycode letters[]={SDLK_N,SDLK_G,SDLK_S,SDLK_E,SDLK_H,SDLK_P};
    const SvBirthMode modes[]={SV_BIRTH_MODE_NORMAL,SV_BIRTH_MODE_NO_GHOST,
        SV_BIRTH_MODE_SOLOIST,SV_BIRTH_MODE_EVERLASTING,SV_BIRTH_MODE_HELLISH,SV_BIRTH_MODE_PVP};
    for(unsigned i=0;i<sizeof(letters)/sizeof(letters[0]);++i) {
        SvBirth selected=b;SvBirthViewport viewport=v;
        SDL_Event e={.type=SDL_EVENT_KEY_DOWN};e.key.key=letters[i];
        if(letters[i]==SDLK_H)e.key.mod=SDL_KMOD_SHIFT;
        assert(sv_native_birth_event(&selected,1,0,&viewport,&e)==SV_BIRTH_INPUT_CONFIRMED);
        assert(selected.owner==SV_BIRTH_READY && selected.mode_confirmed && selected.mode_ready);
        assert(selected.chosen_mode==modes[i]);
    }
    SDL_Quit();puts("mode four-direction grid navigation, wrap and keypad PASS");
}

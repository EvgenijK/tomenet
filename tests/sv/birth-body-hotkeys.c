#include "login/creation/birth.h"
/* Letter selection commits exactly the body step through the native adapter. */
#include "login/creation/native-birth.h"
#include <assert.h>
#include <stdio.h>

static SvContactSetup setup = {.class_count=1,.race_count=1,.body_step_available=true};
static SvBirth begin(bool first_run)
{
    setup.races[0].choice=1;
    uint32_t flags[4]={0};int16_t stats[6]={0};SvBirth birth;
    assert(sv_birth_begin(&birth,1,&setup,flags,first_run,0,false,SV_BIRTH_SEX_MALE,
        false,0,false,0,false,0,false,SV_BIRTH_BODY_NORMAL,false,stats,false,SV_BIRTH_MODE_NORMAL)==SV_OK);
    sv_birth_command(&birth,1,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM});
    sv_birth_command(&birth,1,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM});
    sv_birth_command(&birth,1,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM});
    assert(birth.owner==SV_BIRTH_BODY);
    return birth;
}
static void key(SvBirth *birth,SDL_Keycode code,bool repeat)
{
    SDL_Event event={.type=SDL_EVENT_KEY_DOWN};event.key.key=code;event.key.repeat=repeat;
    sv_native_birth_event(birth,1,0,NULL,&event);
}
int main(void)
{
    assert(SDL_Init(SDL_INIT_EVENTS));
    const SDL_Keycode keys[]={SDLK_N,SDLK_F};
    for(unsigned i=0;i<2;++i) {
        SvBirth birth=begin(false);
        key(&birth,keys[i],true);assert(birth.owner==SV_BIRTH_BODY);
        key(&birth,keys[i],false);
        fprintf(stderr,"body %c: owner=%d confirmed=%d\n",(char)keys[i],birth.owner,birth.body_confirmed);
        assert(birth.owner==SV_BIRTH_STATS && birth.body_confirmed);
        assert(birth.chosen_body==(i?SV_BIRTH_BODY_FRUIT_BAT:SV_BIRTH_BODY_NORMAL));
        assert(!birth.stats_confirmed);
    }
    SvBirth birth=begin(true);
    key(&birth,SDLK_F,false);assert(birth.owner==SV_BIRTH_BODY && !birth.body_confirmed);
    key(&birth,SDLK_N,false);assert(birth.owner==SV_BIRTH_STATS && birth.body_confirmed);
    SDL_Quit();puts("body hotkeys PASS");
}

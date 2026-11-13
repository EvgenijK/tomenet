#include "login/creation/birth.h"
/* Point-buy repeats exercise the production adapter, timer and cancellation. */
#include "login/creation/native-birth.h"
#include "login/ui/login-layout.h"
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
static void mouse(SvBirth *b,SvBirthViewport *v,Uint32 type,SvLoginRect button)
{
 SDL_Event e={.type=type};e.button.button=SDL_BUTTON_LEFT;
 e.button.x=button.x+button.w/2;e.button.y=button.y+button.h/2;
 sv_native_birth_event(b,1,0,v,&e);
}
int main(void)
{
 assert(SDL_Init(SDL_INIT_EVENTS));setup.creation_flags=1;
 SvBirth birth=begin(false);key(&birth,SDLK_N,false);
 assert(birth.owner==SV_BIRTH_STATS && birth.stats_point_buy);
 int before=birth.stats[0];
 key(&birth,SDLK_RIGHT,false);key(&birth,SDLK_RIGHT,true);
 assert(birth.stats[0]==before+2);
 key(&birth,SDLK_MINUS,true);assert(birth.stats[0]==before+1);
 key(&birth,SDLK_RETURN,true);assert(!birth.stats_confirmation_pending && birth.owner==SV_BIRTH_STATS);
 key(&birth,SDLK_DOWN,true);assert(birth.highlighted_stat==1);
 SvBirthViewport v={.logical_width=1440,.logical_height=900};
 sv_birth_viewport_sync(&v,&birth,900);
 SvBirthLayout l=sv_birth_layout(1440,900,SV_BIRTH_STATS);
 SvLoginRect row=sv_birth_choice_rect(l,1,0),plus=sv_birth_stat_plus(row);
 before=birth.stats[1];mouse(&birth,&v,SDL_EVENT_MOUSE_BUTTON_DOWN,plus);
 assert(birth.stats[1]==before+1 && v.held_direction==1);
 uint64_t due=v.repeat_at_ms;
 sv_native_birth_tick(&birth,1,&v,due-1);assert(birth.stats[1]==before+1);
 sv_native_birth_tick(&birth,1,&v,due);assert(birth.stats[1]==before+2);
 sv_native_birth_tick(&birth,1,&v,due+1);assert(birth.stats[1]==before+2);
 sv_native_birth_tick(&birth,1,&v,due+65);assert(birth.stats[1]==before+3);
 mouse(&birth,&v,SDL_EVENT_MOUSE_BUTTON_UP,plus);
 sv_native_birth_tick(&birth,1,&v,due+1000);assert(birth.stats[1]==before+3 && !v.held_direction);
 mouse(&birth,&v,SDL_EVENT_MOUSE_BUTTON_DOWN,sv_birth_stat_minus(row));
 before=birth.stats[1];due=v.repeat_at_ms;
 sv_native_birth_tick(&birth,1,&v,due);assert(birth.stats[1]==before-1);
 SDL_Event lost={.type=SDL_EVENT_WINDOW_FOCUS_LOST};
 sv_native_birth_event(&birth,1,0,&v,&lost);assert(!v.held_direction);
 before=birth.stats[1];sv_native_birth_tick(&birth,1,&v,due+1000);assert(birth.stats[1]==before);
 mouse(&birth,&v,SDL_EVENT_MOUSE_BUTTON_DOWN,plus);due=v.repeat_at_ms;
 before=birth.stats[1];sv_native_birth_tick(&birth,2,&v,due);assert(birth.stats[1]==before && !v.held_direction);
 mouse(&birth,&v,SDL_EVENT_MOUSE_BUTTON_DOWN,plus);due=v.repeat_at_ms;
 SDL_Event motion={.type=SDL_EVENT_MOUSE_MOTION};
 sv_native_birth_event(&birth,1,0,&v,&motion);assert(!v.held_direction);
 /* Unused-point confirmation consumes one deliberate gesture, never repeats. */
 key(&birth,SDLK_RETURN,false);assert(birth.stats_confirmation_pending);
 key(&birth,SDLK_RIGHT,true);assert(birth.stats_confirmation_pending);
 key(&birth,SDLK_RETURN,true);assert(birth.owner==SV_BIRTH_STATS);
 SDL_Quit();puts("stats keyboard/mouse hold, cadence, release/focus/generation guards PASS");
}

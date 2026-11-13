/* Test observation/SDL gestures only. Based on the accepted birth-complete.c
 * rendered-view driver; no mutation of production birth state. Include after
 * push_text declaration. Set native_birth_driver.enabled before endpoint run. */
#include "login/ui/endpoint-scene.h"
#include "login/login-view.h"
#include "login/creation/birth-interaction.h"
#include <string.h>
static struct {
    bool enabled, name_started, name_submitted, mode_confirmed, motd_acknowledged, handoff_rendered;
    bool point_buy_seen, points_confirmation_started, points_confirmation_sent;
    unsigned submitted_owners, stat_step;
    bool stat_selected;
    unsigned rendered_owners;
} native_birth_driver;
static void (*native_birth_capture)(SDL_Renderer *,SvBirthOwner);

static void native_birth_key(SDL_Keycode key,SDL_Keymod modifiers)
{
    SDL_Event event={.type=SDL_EVENT_KEY_DOWN};
    event.key.key=key;event.key.mod=modifiers;
    assert(SDL_PushEvent(&event));
}

bool __real_sv_endpoint_draw(SDL_Renderer *,SvFont *,const SvEndpoint *,
    const SvEndpointView *,SvEndpointRenderMetrics *);
bool __wrap_sv_endpoint_draw(SDL_Renderer *renderer,SvFont *font,const SvEndpoint *endpoint,
    const SvEndpointView *view,SvEndpointRenderMetrics *metrics)
{
    SDL_ClearError();
    bool drawn=__real_sv_endpoint_draw(renderer,font,endpoint,view,metrics);
    if(!drawn)fprintf(stderr,"endpoint draw failed: motd=%d birth=%d error=%s\n",
        view->login_view && view->login_view->motd!=NULL,view->birth_view!=NULL,SDL_GetError());
    if(!drawn || !native_birth_driver.enabled)return drawn;
    const SvLoginView *login=view->login_view;
    if(login && login->live_handoff && native_birth_driver.motd_acknowledged && view->preparation_identity) {
        assert(!view->birth_view && !view->birth_viewport);
        assert(view->contact_status && strstr(view->contact_status,"Born"));
        native_birth_driver.handoff_rendered=true;
        if(native_birth_capture)native_birth_capture(renderer,SV_BIRTH_READY);
    }
    if(login && login->motd && !native_birth_driver.motd_acknowledged) {
        if(native_birth_capture)native_birth_capture(renderer,SV_BIRTH_NONE);
        native_birth_driver.motd_acknowledged=true;native_birth_key(SDLK_RETURN,0);
    }
    if(login && login->overview && !native_birth_driver.name_started) {
        native_birth_driver.name_started=true;
        native_birth_key(SDLK_N,SDL_KMOD_SHIFT);
    } else if(login && login->ordinary_name_entry && !native_birth_driver.name_submitted) {
        native_birth_driver.name_submitted=true;
        push_text("Born");native_birth_key(SDLK_RETURN,0);
    }
    const SvBirthView *birth=view->birth_view;
    if(!birth)return drawn;
    if(native_birth_capture && birth->owner!=SV_BIRTH_READY)
        native_birth_capture(renderer,birth->owner);
    native_birth_driver.rendered_owners|=1u<<birth->owner;
    SDL_Keycode key=SDLK_UNKNOWN;
    if(birth->owner==SV_BIRTH_MODE) {
        assert(birth->stats_confirmed && (birth->stats_point_buy || birth->stats_order_count==6));
        if(!native_birth_driver.mode_confirmed) {
            native_birth_driver.mode_confirmed=true;key=SDLK_RETURN;
        }
    } else if(birth->owner==SV_BIRTH_STATS) {
        if(birth->stats_point_buy) {
            native_birth_driver.point_buy_seen=true;
            if(!native_birth_driver.points_confirmation_started) {
                native_birth_driver.points_confirmation_started=true;key=SDLK_ESCAPE;
            } else if(birth->stats_confirmation_pending && !native_birth_driver.points_confirmation_sent) {
                native_birth_driver.points_confirmation_sent=true;key=SDLK_ESCAPE;
            }
            if(key!=SDLK_UNKNOWN)native_birth_key(key,0);
            return drawn;
        }
        if(birth->stats_order_count>native_birth_driver.stat_step) {
            assert(birth->stats_order_count==native_birth_driver.stat_step+1);
            native_birth_driver.stat_step++;native_birth_driver.stat_selected=false;
        }
        if(!native_birth_driver.stat_selected) {
            key=SDLK_A+native_birth_driver.stat_step;
            native_birth_driver.stat_selected=true;
        } else if(birth->highlighted_stat==(int)native_birth_driver.stat_step &&
            !(native_birth_driver.submitted_owners&(1u<<(16+native_birth_driver.stat_step)))) {
            native_birth_driver.submitted_owners|=1u<<(16+native_birth_driver.stat_step);
            key=SDLK_RETURN;
        }
    } else if(!(native_birth_driver.submitted_owners&(1u<<birth->owner))) {
        switch(birth->owner) {
        case SV_BIRTH_SEX: case SV_BIRTH_CLASS: case SV_BIRTH_RACE:
        case SV_BIRTH_TRAIT: case SV_BIRTH_BODY:key=SDLK_RETURN;break;
        default:break;
        }
        native_birth_driver.submitted_owners|=1u<<birth->owner;
    }
    if(key!=SDLK_UNKNOWN)native_birth_key(key,0);
    return drawn;
}

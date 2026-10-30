#include "input/native-login.h"
#include "ui/login-layout.h"

bool sv_native_login_command(const SDL_Event *event, SvLoginCommand *command)
{
    if (!event || !command) return false;
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat ||
        !event->key.key) return false;
    SDL_Keycode key = event->key.key;
    if (key == SDLK_Q) *command = (SvLoginCommand){SV_LOGIN_COMMAND_QUIT, 0};
    else if (key == SDLK_N && (event->key.mod & (SDL_KMOD_SHIFT | SDL_KMOD_CAPS)))
        *command = (SvLoginCommand){SV_LOGIN_COMMAND_CREATE_ORDINARY, 0};
    else if (key == SDLK_E && (event->key.mod & (SDL_KMOD_SHIFT | SDL_KMOD_CAPS)))
        *command = (SvLoginCommand){SV_LOGIN_COMMAND_CREATE_EXCLUSIVE, 0};
    else if (key == SDLK_ESCAPE)
        *command = (SvLoginCommand){SV_LOGIN_COMMAND_CANCEL, 0};
    else if (key >= SDLK_A && key <= SDLK_Z)
        *command = (SvLoginCommand){SV_LOGIN_COMMAND_CHOOSE,
                                    (size_t)(key - SDLK_A)};
    else *command = (SvLoginCommand){SV_LOGIN_COMMAND_ACK_MOTD, 0};
    return true;
}

void sv_native_login_clear_previous_keys(SvLoginTransition transition)
{
    if (transition != SV_LOGIN_TRANSITION_NONE)
        SDL_FlushEvents(SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);
}

void sv_character_viewport_sync(SvCharacterViewport *v,SvCharacterLayout c,size_t count,size_t total)
{
    c=sv_character_content_layout(c,count);
    if(v->selected>=count)v->selected=count?count-1:0;
    size_t maximum=total;
    float used=0;
    while(maximum) {
        float step=maximum-1<count?35:47;
        if(used+step-7>c.list.h)break;
        used+=step;--maximum;
    }
    if(v->first>maximum)v->first=maximum;
    c.first=v->first;
    c=sv_character_content_layout(c,count);
    if(count && v->list_height!=c.list.h) {
        if(v->selected<v->first)v->first=v->selected;
        if(v->selected>=v->first+c.rows)v->first=v->selected-c.rows+1;
    }
    c.first=v->first;
    c=sv_character_content_layout(c,count);
    v->rows=c.rows;
    v->list_height=c.list.h;
}
bool sv_native_character_command(const SDL_Event *event,SvCharacterLayout c,
    SvCharacterViewport *v,size_t count,size_t total,SvLoginCommand *command)
{
    sv_character_viewport_sync(v,c,count,total);
    c.first=v->first;
    c=sv_character_content_layout(c,count);
    if(event->type==SDL_EVENT_MOUSE_WHEEL) {
        size_t maximum=total;
        float used=0;
        while(maximum) {
            float step=maximum-1<count?35:47;
            if(used+step-7>c.list.h)break;
            used+=step;--maximum;
        }
        if(event->wheel.y<0 && v->first<maximum)++v->first;
        if(event->wheel.y>0 && v->first)--v->first;
        return false;
    }
    if(event->type==SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button==SDL_BUTTON_LEFT) {
        float x=event->button.x,y=event->button.y;
        if(sv_login_contains(c.list,x,y)) {
            for(size_t slot=c.first;slot<count && slot<c.first+c.rows;++slot) {
                if(!sv_login_contains(sv_character_row(c,slot),x,y))continue;
                v->selected=slot;v->focus=0;
                if(event->button.clicks>=2) {*command=(SvLoginCommand){SV_LOGIN_COMMAND_CHOOSE,slot};return true;}
                break;
            }
            return false;
        }
        if(sv_login_contains(c.ordinary,x,y)) {v->focus=1;*command=(SvLoginCommand){SV_LOGIN_COMMAND_CREATE_ORDINARY,0};return true;}
        if(sv_login_contains(c.exclusive,x,y)) {v->focus=2;*command=(SvLoginCommand){SV_LOGIN_COMMAND_CREATE_EXCLUSIVE,0};return true;}
        if(sv_login_contains(c.enter,x,y) && count) {*command=(SvLoginCommand){SV_LOGIN_COMMAND_CHOOSE,v->selected};return true;}
        if(sv_login_contains(c.quit,x,y)) {*command=(SvLoginCommand){SV_LOGIN_COMMAND_CANCEL,0};return true;}
        return false;
    }
    if(event->type==SDL_EVENT_KEY_DOWN && !event->key.repeat) {
        SDL_Keycode key=event->key.key;
        if(key==SDLK_TAB) {v->focus=(v->focus+((event->key.mod&SDL_KMOD_SHIFT)?4:1))%5;return false;}
        if(key==SDLK_UP || key==SDLK_DOWN || key==SDLK_HOME || key==SDLK_END) {
            if(key==SDLK_UP && v->selected)--v->selected;
            if(key==SDLK_DOWN && v->selected+1<count)++v->selected;
            if(key==SDLK_HOME)v->selected=0;
            if(key==SDLK_END && count)v->selected=count-1;
            v->focus=0;
            if(v->selected<v->first)v->first=v->selected;
            if(v->selected>=v->first+c.rows)v->first=v->selected-c.rows+1;
            return false;
        }
        if(key==SDLK_RETURN || key==SDLK_KP_ENTER) {
            SvLoginCommandKind kinds[]={SV_LOGIN_COMMAND_CHOOSE,SV_LOGIN_COMMAND_CREATE_ORDINARY,
                SV_LOGIN_COMMAND_CREATE_EXCLUSIVE,SV_LOGIN_COMMAND_CHOOSE,SV_LOGIN_COMMAND_CANCEL};
            *command=(SvLoginCommand){kinds[v->focus],v->selected};return true;
        }
    }
    return sv_native_login_command(event,command);
}

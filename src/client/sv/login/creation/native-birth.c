#include "login/creation/native-birth.h"
#include "login/creation/birth.h"
#include "login/ui/pregame-events.h"
#include "login/ui/login-layout.h"

static bool letter_key(SDL_Keycode key)
{
    return key >= SDLK_A && key <= SDLK_Z;
}

static bool letter_modifiers(const SDL_KeyboardEvent *event)
{
    return !(event->mod & (SDL_KMOD_LEVEL5 | SDL_KMOD_CTRL | SDL_KMOD_ALT |
                           SDL_KMOD_GUI | SDL_KMOD_MODE));
}

static bool symbol_modifiers(const SDL_KeyboardEvent *event)
{
    return !(event->mod & (SDL_KMOD_CTRL | SDL_KMOD_LALT | SDL_KMOD_GUI));
}

static bool uppercase_letter(const SDL_KeyboardEvent *event, SDL_Keycode key)
{
    bool shifted = (event->mod & SDL_KMOD_SHIFT) != 0;
    bool caps_locked = (event->mod & SDL_KMOD_CAPS) != 0;
    return letter_key(key) && shifted != caps_locked;
}

static bool lowercase_letter(const SDL_KeyboardEvent *event, SDL_Keycode key)
{
    return letter_key(key) && letter_modifiers(event) &&
        !uppercase_letter(event, key);
}

static SDL_Keycode layout_key(const SDL_KeyboardEvent *event)
{
    SDL_Keycode translated = SDL_GetKeyFromScancode(
        event->scancode, event->mod, false);
    return translated == SDLK_UNKNOWN ? event->key : translated;
}

const char *sv_birth_mode_title(SvBirthMode mode)
{
    switch(mode) {
    case SV_BIRTH_MODE_NORMAL:return "Normal";
    case SV_BIRTH_MODE_NO_GHOST:return "No Ghost";
    case SV_BIRTH_MODE_SOLOIST:return "Soloist";
    case SV_BIRTH_MODE_HELLISH:return "Hellish";
    case SV_BIRTH_MODE_EVERLASTING:return "Everlasting";
    case SV_BIRTH_MODE_PVP:return "PvP";
    case SV_BIRTH_MODE_FRUIT_BAT:return "Fruit bat";
    case SV_BIRTH_MODE_IDDC:return "IDDC";
    case SV_BIRTH_MODE_IDDC_SOLOIST:return "IDDC Soloist";
    case SV_BIRTH_MODE_IDDC_HELLISH:return "Hellish IDDC";
    case SV_BIRTH_MODE_DEDICATED_PVP:return "Dedicated PvP";
    default:return "--";
    }
}
unsigned sv_birth_choice_count(const SvBirthView *v)
{
    switch(v->owner) {
    case SV_BIRTH_SEX:return 2;
    case SV_BIRTH_CLASS:return v->visible_class_count;
    case SV_BIRTH_RACE:return v->setup?v->setup->race_count:0;
    case SV_BIRTH_TRAIT:return v->legal_trait_count;
    case SV_BIRTH_BODY:return v->legal_body_count;
    case SV_BIRTH_STATS:return v->legal_stat_count;
    case SV_BIRTH_MODE:return v->legal_mode_count;
    default:return 0;
    }
}
int sv_birth_choice_id(const SvBirthView *v,unsigned p)
{
    if(p>=sv_birth_choice_count(v))return -1;
    switch(v->owner) {
    case SV_BIRTH_SEX:case SV_BIRTH_RACE:return (int)p;
    case SV_BIRTH_CLASS:return v->visible_classes[p];
    case SV_BIRTH_TRAIT:return v->legal_traits[p];
    case SV_BIRTH_BODY:return v->legal_bodies[p];
    case SV_BIRTH_STATS:return v->legal_stats[p];
    case SV_BIRTH_MODE:return v->legal_modes[p];
    default:return -1;
    }
}
int sv_birth_highlight(const SvBirthView *v)
{
    switch(v->owner) {
    case SV_BIRTH_SEX:return v->highlighted_sex;
    case SV_BIRTH_CLASS:return v->highlighted_class;
    case SV_BIRTH_RACE:return v->highlighted_race;
    case SV_BIRTH_TRAIT:return v->highlighted_trait;
    case SV_BIRTH_BODY:return v->highlighted_body;
    case SV_BIRTH_STATS:return v->highlighted_stat;
    case SV_BIRTH_MODE:return v->highlighted_mode;
    default:return -1;
    }
}
bool sv_birth_choice_legal(const SvBirthView *v,unsigned p)
{
    int id=sv_birth_choice_id(v,p);
    if(id<0)return false;
    if(v->owner==SV_BIRTH_CLASS) {
        for(unsigned i=0;i<v->legal_class_count;++i)if(v->legal_classes[i]==id)return true;
        return false;
    }
    if(v->owner==SV_BIRTH_RACE) {
        for(unsigned i=0;i<v->legal_race_count;++i)if(v->legal_races[i]==id)return true;
        return false;
    }
    return true;
}
const char *sv_birth_choice_title(const SvBirthView *v,unsigned p)
{
    static const char *stats[]={"Strength","Intelligence","Wisdom","Dexterity","Constitution","Charisma"};
    int id=sv_birth_choice_id(v,p);if(id<0)return "--";
    switch(v->owner) {
    case SV_BIRTH_SEX:return id==SV_BIRTH_SEX_MALE?"Male":"Female";
    case SV_BIRTH_CLASS:return v->setup->classes[id].title;
    case SV_BIRTH_RACE:return v->setup->races[id].title;
    case SV_BIRTH_TRAIT:return v->setup->traits[id].title;
    case SV_BIRTH_BODY:return id==SV_BIRTH_BODY_FRUIT_BAT?"Fruit bat":"Normal body";
    case SV_BIRTH_STATS:return stats[id];
    case SV_BIRTH_MODE:return sv_birth_mode_title((SvBirthMode)id);
    default:return "--";
    }
}
bool sv_birth_action_available(const SvBirthView *v,SvBirthCommandKind kind)
{
    if(!v || v->owner<SV_BIRTH_SEX || v->owner>SV_BIRTH_MODE)return false;
    if(kind==SV_BIRTH_BACK)return true;
    if(kind==SV_BIRTH_RANDOM)return v->owner!=SV_BIRTH_STATS || !v->stats_point_buy;
    if(kind==SV_BIRTH_DNA || kind==SV_BIRTH_AUTOMATIC_DNA) {
        if(!v->dna_available)return false;
        switch(v->owner) {
        case SV_BIRTH_SEX:return v->dna_available;
        case SV_BIRTH_CLASS:return v->dna_class_available;
        case SV_BIRTH_RACE:return v->dna_race_available;
        case SV_BIRTH_TRAIT:return v->dna_trait_available;
        case SV_BIRTH_BODY:return v->dna_body_available;
        case SV_BIRTH_STATS:return v->stats_point_buy && v->dna_stats_available;
        case SV_BIRTH_MODE:return v->dna_mode_available;
        default:return false;
        }
    }
    return true;
}
static unsigned choice_count(const SvBirth *b)
{ SvBirthView v;sv_birth_view_prepare(&v,b);return sv_birth_choice_count(&v); }
static int highlighted_position(const SvBirth *b)
{
    SvBirthView v;sv_birth_view_prepare(&v,b);
    for(unsigned i=0;i<sv_birth_choice_count(&v);++i)
        if(sv_birth_choice_id(&v,i)==sv_birth_highlight(&v))return (int)i;
    return -1;
}
void sv_birth_viewport_sync(SvBirthViewport *v,const SvBirth *b,int height)
{
    if(!v || !b)return;
    if(v->logical_width<=0)v->logical_width=1280;
    SvBirthLayout l=sv_birth_layout(v->logical_width,height,b->owner);
    unsigned rows=l.rows*l.columns;
    SvBirthView view;sv_birth_view_prepare(&view,b);
    int identity=sv_birth_highlight(&view);
    bool changed=v->owner!=b->owner || v->rows!=rows || v->columns!=l.columns || v->highlighted_identity!=identity || v->logical_height!=height;
    if(v->owner!=b->owner || v->logical_height!=height ||
       v->highlighted_identity!=identity)sv_native_birth_release(v);
    if(v->owner!=b->owner) {
        v->first=0;v->detail_scroll=0;v->hover=-1;v->focus=0;v->choice_keyboard_focus=false;
    } else if(v->highlighted_identity!=identity)v->detail_scroll=0;
    v->owner=b->owner;v->rows=rows;v->columns=l.columns;v->logical_height=height;
    unsigned count=choice_count(b),total_rows=(count+l.columns-1)/l.columns;
    unsigned maximum=total_rows>l.rows?(total_rows-l.rows)*l.columns:0;
    if(v->first>maximum)v->first=maximum;
    v->first-=v->first%l.columns;
    int position=highlighted_position(b);
    if(changed && position>=0) {
        unsigned row=(unsigned)position/l.columns;
        if((unsigned)position<v->first)v->first=row*l.columns;
        else if((unsigned)position>=v->first+rows)v->first=(row-l.rows+1)*l.columns;
    }
    v->highlighted_identity=identity;
}
static void scroll_viewport(SvBirthViewport *v,const SvBirth *b,int direction)
{
    unsigned count=choice_count(b),columns=v->columns?v->columns:1;
    unsigned total_rows=(count+columns-1)/columns,rows=v->rows/columns;
    unsigned maximum=total_rows>rows?(total_rows-rows)*columns:0;
    if(direction<0 && v->first>=columns)v->first-=columns;
    else if(direction>0 && v->first<maximum)v->first+=columns;
}
static SvBirthCommand select_position(const SvBirth *b,unsigned p)
{
    SvBirthView v;sv_birth_view_prepare(&v,b);int id=sv_birth_choice_id(&v,p);
    switch(b->owner) {
    case SV_BIRTH_SEX:return (SvBirthCommand){.kind=SV_BIRTH_SELECT_SEX,.sex=(SvBirthSex)id};
    case SV_BIRTH_CLASS:return (SvBirthCommand){.kind=SV_BIRTH_SELECT_CLASS,.class_id=id};
    case SV_BIRTH_RACE:return (SvBirthCommand){.kind=SV_BIRTH_SELECT_RACE,.race_id=id};
    case SV_BIRTH_TRAIT:return (SvBirthCommand){.kind=SV_BIRTH_SELECT_TRAIT,.trait_id=id};
    case SV_BIRTH_BODY:return (SvBirthCommand){.kind=SV_BIRTH_SELECT_BODY,.body=(SvBirthBody)id};
    case SV_BIRTH_STATS:return (SvBirthCommand){.kind=SV_BIRTH_SELECT_STAT,.stat=id};
    case SV_BIRTH_MODE:return (SvBirthCommand){.kind=SV_BIRTH_SELECT_MODE,.mode=(SvBirthMode)id};
    default:return (SvBirthCommand){.kind=SV_BIRTH_MOVE_NEXT};
    }
}
static bool selectable_position(const SvBirth *b,unsigned p)
{ SvBirthView v;sv_birth_view_prepare(&v,b);return sv_birth_choice_legal(&v,p); }

static SvBirthInputResult apply(SvBirth *birth, uint64_t generation,
                                SvBirthViewport *viewport,
                                SvBirthCommand command, bool keyboard, bool mouse)
{
    SvBirthOwner previous_owner = birth->owner;
    if(viewport && previous_owner==SV_BIRTH_STATS && keyboard)
        viewport->choice_keyboard_focus=true;
    SvBirthInputResult result = sv_birth_command(birth, generation, command);
    if (birth->owner != previous_owner) sv_pregame_input_boundary();
    if (result != SV_BIRTH_INPUT_IGNORED) {
        birth->keyboard_navigated |= keyboard;
        birth->mouse_navigated |= mouse;
    }
    if (viewport)
        sv_birth_viewport_sync(viewport, birth, viewport->logical_height);
    return result;
}

void sv_native_birth_release(SvBirthViewport *v)
{ if(v){v->held_direction=0;v->repeat_at_ms=0;} }
SvBirthInputResult sv_native_birth_tick(SvBirth *b,uint64_t generation,
                                       SvBirthViewport *v,uint64_t now_ms)
{
    if(!v || !v->held_direction)return SV_BIRTH_INPUT_IGNORED;
    if(!b || generation!=v->held_generation || generation!=b->generation ||
       !b->active || b->quit || b->owner!=SV_BIRTH_STATS || !b->stats_point_buy ||
       b->stats_confirmation_pending || b->highlighted_stat!=v->held_stat) {
        sv_native_birth_release(v);return SV_BIRTH_INPUT_IGNORED;
    }
    if(now_ms<v->repeat_at_ms)return SV_BIRTH_INPUT_IGNORED;
    v->repeat_at_ms=now_ms+65;
    return apply(b,generation,v,(SvBirthCommand){.kind=v->held_direction>0?
        SV_BIRTH_INCREASE_STAT:SV_BIRTH_DECREASE_STAT},false,true);
}
static SvBirthInputResult hold_stat(SvBirth *b,uint64_t generation,SvBirthViewport *v,int direction)
{
    SvBirthInputResult result=apply(b,generation,v,(SvBirthCommand){.kind=direction>0?
        SV_BIRTH_INCREASE_STAT:SV_BIRTH_DECREASE_STAT},false,true);
    if(result==SV_BIRTH_INPUT_HANDLED) {
        v->held_stat=b->highlighted_stat;v->held_direction=direction;
        v->held_generation=generation;v->repeat_at_ms=SDL_GetTicks()+350;
    }
    return result;
}

/* Letter shortcuts commit only the owner that received the gesture. The
 * command may skip unavailable later steps, which must not see another confirm. */
static SvBirthInputResult select_and_confirm(SvBirth *birth, uint64_t generation,
                                             SvBirthViewport *viewport,
                                             SvBirthCommand command)
{
    SvBirthOwner owner=birth->owner;
    SvBirthInputResult result=apply(birth,generation,viewport,command,true,false);
    if(result==SV_BIRTH_INPUT_HANDLED && birth->owner==owner)
        return apply(birth,generation,viewport,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM},true,false);
    return result;
}

SvBirthInputResult sv_native_birth_event(SvBirth *birth, uint64_t generation,
                                         uint64_t since_ns,
                                         SvBirthViewport *viewport,
                                         const SDL_Event *event)
{
    if (!birth || !event) return SV_BIRTH_INPUT_IGNORED;
    if(viewport && (event->type==SDL_EVENT_WINDOW_FOCUS_LOST ||
                   event->type==SDL_EVENT_MOUSE_BUTTON_DOWN))
        viewport->choice_keyboard_focus=false;
    if (generation != birth->generation || !birth->active || birth->quit) {
        sv_native_birth_release(viewport);return SV_BIRTH_INPUT_IGNORED;
    }
    if(event->type==SDL_EVENT_MOUSE_BUTTON_UP || event->type==SDL_EVENT_WINDOW_FOCUS_LOST ||
       event->type==SDL_EVENT_WINDOW_MOUSE_LEAVE || event->type==SDL_EVENT_WINDOW_RESIZED ||
       event->type==SDL_EVENT_KEY_DOWN || event->type==SDL_EVENT_MOUSE_BUTTON_DOWN ||
       event->type==SDL_EVENT_MOUSE_WHEEL)sv_native_birth_release(viewport);
    if (!sv_pregame_keyboard_event(event) &&
        event->common.timestamp && event->common.timestamp < since_ns)
        return SV_BIRTH_INPUT_IGNORED;
    if(event->type==SDL_EVENT_WINDOW_RESIZED && viewport) {
        viewport->logical_width=event->window.data1;
        sv_birth_viewport_sync(viewport,birth,event->window.data2);
        return SV_BIRTH_INPUT_IGNORED;
    }
    if(viewport && (event->type==SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event->type==SDL_EVENT_MOUSE_MOTION || event->type==SDL_EVENT_MOUSE_WHEEL)) {
        SvBirthLayout l=sv_birth_layout(viewport->logical_width,viewport->logical_height,birth->owner);
        float x=event->type==SDL_EVENT_MOUSE_MOTION?event->motion.x:
            event->type==SDL_EVENT_MOUSE_WHEEL?event->wheel.mouse_x:event->button.x;
        float y=event->type==SDL_EVENT_MOUSE_MOTION?event->motion.y:
            event->type==SDL_EVENT_MOUSE_WHEEL?event->wheel.mouse_y:event->button.y;
        if(event->type==SDL_EVENT_MOUSE_WHEEL) {
            float dy=event->wheel.direction==SDL_MOUSEWHEEL_FLIPPED?-event->wheel.y:event->wheel.y;
            unsigned *offset=sv_login_contains(l.summary,x,y)?&viewport->summary_scroll:
                sv_login_contains(l.detail,x,y)?&viewport->detail_scroll:NULL;
            if(offset) {
                unsigned maximum=offset==&viewport->summary_scroll?viewport->summary_max:viewport->detail_max;
                if(dy<0)*offset=*offset+36<maximum?*offset+36:maximum;
                else if(dy>0)*offset=*offset>36?*offset-36:0;
            } else if(sv_login_contains(l.choices,x,y)) {
                if(dy<0)scroll_viewport(viewport,birth,1);
                else if(dy>0)scroll_viewport(viewport,birth,-1);
            } else return SV_BIRTH_INPUT_IGNORED;
            return SV_BIRTH_INPUT_HANDLED;
        }
        if(birth->stats_confirmation_pending && event->type==SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if(event->button.button!=SDL_BUTTON_LEFT)return SV_BIRTH_INPUT_IGNORED;
            if(sv_login_contains(l.warning_confirm,x,y))
                return apply(birth,generation,viewport,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM},false,true);
            if(sv_login_contains(l.warning_cancel,x,y))
                return apply(birth,generation,viewport,(SvBirthCommand){.kind=SV_BIRTH_CANCEL_STATS_CONFIRMATION},false,true);
            return SV_BIRTH_INPUT_IGNORED;
        }
        int hit=-1;
        if(sv_login_contains(l.choices,x,y))
            for(unsigned p=viewport->first;p<choice_count(birth) && p<viewport->first+viewport->rows;++p)
                if(sv_login_contains(sv_birth_choice_rect(l,p,viewport->first),x,y))hit=(int)p;
        if(event->type==SDL_EVENT_MOUSE_MOTION) {
            viewport->hover=hit;
            if(viewport->held_direction) {
                SvLoginRect held=sv_birth_choice_rect(l,(unsigned)viewport->held_stat,viewport->first);
                SvLoginRect button=viewport->held_direction>0?sv_birth_stat_plus(held):sv_birth_stat_minus(held);
                if(!sv_login_contains(button,x,y))sv_native_birth_release(viewport);
            }
            return SV_BIRTH_INPUT_IGNORED;
        }
        if(event->button.button!=SDL_BUTTON_LEFT)return SV_BIRTH_INPUT_IGNORED;
        SvBirthView v;sv_birth_view_prepare(&v,birth);
        SvLoginRect controls[]={l.back,l.quit,l.random,l.dna,l.automatic,l.guide,l.confirm};
        SvBirthCommandKind kinds[]={SV_BIRTH_BACK,SV_BIRTH_QUIT,SV_BIRTH_RANDOM,SV_BIRTH_DNA,SV_BIRTH_AUTOMATIC_DNA,SV_BIRTH_CONFIRM,SV_BIRTH_CONFIRM};
        for(unsigned i=0;i<7;++i)if(sv_login_contains(controls[i],x,y)) {
            viewport->focus=(int)i+1;
            if(i==5)return birth->owner>=SV_BIRTH_CLASS && !(birth->owner==SV_BIRTH_STATS && !birth->stats_point_buy)?SV_BIRTH_INPUT_PLACEHOLDER:SV_BIRTH_INPUT_HANDLED;
            if(!sv_birth_action_available(&v,kinds[i]))return SV_BIRTH_INPUT_IGNORED;
            return apply(birth,generation,viewport,(SvBirthCommand){.kind=kinds[i]},false,true);
        }
        if(hit<0)return SV_BIRTH_INPUT_IGNORED;
        SvLoginRect row=sv_birth_choice_rect(l,(unsigned)hit,viewport->first);
        SvBirthInputResult selected=apply(birth,generation,viewport,select_position(birth,(unsigned)hit),false,true);
        viewport->focus=0;
        if(birth->owner==SV_BIRTH_STATS && birth->stats_point_buy) {
            if(sv_login_contains(sv_birth_stat_minus(row),x,y))
                return !v.stat_can_decrease[sv_birth_choice_id(&v,(unsigned)hit)]?SV_BIRTH_INPUT_IGNORED:hold_stat(birth,generation,viewport,-1);
            if(sv_login_contains(sv_birth_stat_plus(row),x,y))
                return !v.stat_can_increase[sv_birth_choice_id(&v,(unsigned)hit)]?SV_BIRTH_INPUT_IGNORED:hold_stat(birth,generation,viewport,1);
        }
        if(selected!=SV_BIRTH_INPUT_IGNORED && event->button.clicks>=2)
            return apply(birth,generation,viewport,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM},false,true);
        return selected;
    }
    if (event->type != SDL_EVENT_KEY_DOWN)return SV_BIRTH_INPUT_IGNORED;
    SDL_Keycode key = layout_key(&event->key);
    if(event->key.repeat && !(birth->owner==SV_BIRTH_STATS && birth->stats_point_buy &&
       !birth->stats_confirmation_pending && letter_modifiers(&event->key) &&
       (key==SDLK_UP || key==SDLK_DOWN || key==SDLK_LEFT || key==SDLK_RIGHT ||
        key==SDLK_KP_2 || key==SDLK_KP_4 || key==SDLK_KP_6 || key==SDLK_KP_8 ||
        ((key==SDLK_MINUS || key==SDLK_PLUS) && symbol_modifiers(&event->key)))))
        return SV_BIRTH_INPUT_IGNORED;
    if(viewport && birth->stats_confirmation_pending && key==SDLK_TAB) {
        viewport->focus=viewport->focus==1?7:1;
        return SV_BIRTH_INPUT_HANDLED;
    }
    if(viewport && birth->stats_confirmation_pending && (key==SDLK_RETURN || key==SDLK_KP_ENTER))
        return apply(birth,generation,viewport,(SvBirthCommand){.kind=viewport->focus==1?SV_BIRTH_CANCEL_STATS_CONFIRMATION:SV_BIRTH_CONFIRM},true,false);
    if(viewport && key==SDLK_TAB) {
        viewport->focus=viewport->focus==1?7:viewport->focus==7?1:
            (event->key.mod&SDL_KMOD_SHIFT)?7:1;
        return SV_BIRTH_INPUT_HANDLED;
    }
    if(viewport && viewport->focus && (key==SDLK_RETURN || key==SDLK_KP_ENTER)) {
        SvBirthView v;sv_birth_view_prepare(&v,birth);
        SvBirthCommandKind kinds[]={SV_BIRTH_BACK,SV_BIRTH_QUIT,SV_BIRTH_RANDOM,SV_BIRTH_DNA,SV_BIRTH_AUTOMATIC_DNA,SV_BIRTH_CONFIRM,SV_BIRTH_CONFIRM};
        if(viewport->focus==6)return birth->owner>=SV_BIRTH_CLASS && !(birth->owner==SV_BIRTH_STATS && !birth->stats_point_buy)?SV_BIRTH_INPUT_PLACEHOLDER:SV_BIRTH_INPUT_HANDLED;
        SvBirthCommandKind kind=kinds[viewport->focus-1];
        return sv_birth_action_available(&v,kind)?apply(birth,generation,viewport,(SvBirthCommand){.kind=kind},true,false):SV_BIRTH_INPUT_IGNORED;
    }
    if(viewport && (key==SDLK_UP || key==SDLK_DOWN || key==SDLK_LEFT || key==SDLK_RIGHT))viewport->focus=0;
    if (key == SDLK_QUESTION && symbol_modifiers(&event->key)) {
        if (birth->owner == SV_BIRTH_SEX ||
            (birth->owner == SV_BIRTH_STATS && !birth->stats_point_buy))
            return SV_BIRTH_INPUT_HANDLED;
        if (birth->owner >= SV_BIRTH_CLASS && birth->owner <= SV_BIRTH_MODE)
            return SV_BIRTH_INPUT_PLACEHOLDER;
    }
    if(birth->owner==SV_BIRTH_MODE &&
       (key==SDLK_UP || key==SDLK_DOWN || key==SDLK_LEFT || key==SDLK_RIGHT ||
        key==SDLK_KP_8 || key==SDLK_KP_2 || key==SDLK_KP_4 || key==SDLK_KP_6)) {
        if(viewport)sv_birth_viewport_sync(viewport,birth,viewport->logical_height);
        int columns=viewport && viewport->columns?(int)viewport->columns:2;
        int position=highlighted_position(birth),count=(int)choice_count(birth);
        if(position<0 || !count)return SV_BIRTH_INPUT_IGNORED;
        int next=position;
        bool horizontal=key==SDLK_LEFT || key==SDLK_RIGHT || key==SDLK_KP_4 || key==SDLK_KP_6;
        int direction=key==SDLK_UP || key==SDLK_LEFT || key==SDLK_KP_8 || key==SDLK_KP_4?-1:1;
        int rows=(count+columns-1)/columns;
        for(int attempt=0;attempt<count;++attempt) {
            if(horizontal) {
                int start=next/columns*columns,end=SDL_min(start+columns,count)-1;
                next=direction<0?(next>start?next-1:end):(next<end?next+1:start);
            } else {
                int column=next%columns,row=next/columns;
                do { row=(row+direction+rows)%rows; } while(row*columns+column>=count);
                next=row*columns+column;
            }
            if(selectable_position(birth,(unsigned)next))
                return apply(birth,generation,viewport,select_position(birth,(unsigned)next),true,false);
        }
        return SV_BIRTH_INPUT_HANDLED;
    }
    if((birth->owner==SV_BIRTH_CLASS || birth->owner==SV_BIRTH_RACE ||
        birth->owner==SV_BIRTH_TRAIT) &&
       (key==SDLK_UP || key==SDLK_DOWN || key==SDLK_LEFT || key==SDLK_RIGHT ||
        key==SDLK_KP_8 || key==SDLK_KP_2 || key==SDLK_KP_4 || key==SDLK_KP_6)) {
        if(viewport) {
            viewport->focus=0;
            sv_birth_viewport_sync(viewport,birth,viewport->logical_height);
        }
        int columns=viewport && viewport->columns?(int)viewport->columns:1;
        int step=key==SDLK_UP || key==SDLK_KP_8?-columns:
            key==SDLK_DOWN || key==SDLK_KP_2?columns:
            key==SDLK_LEFT || key==SDLK_KP_4?-1:1;
        int position=highlighted_position(birth),count=(int)choice_count(birth);
        if(position<0)return SV_BIRTH_INPUT_IGNORED;
        for(int next=position+step;next>=0 && next<count;next+=step)
            if(selectable_position(birth,(unsigned)next))
                return apply(birth,generation,viewport,select_position(birth,(unsigned)next),true,false);
        return SV_BIRTH_INPUT_HANDLED;
    }
    SvBirthCommand command;
    if (event->key.key == SDLK_Q && ((event->key.mod & SDL_KMOD_CTRL) ||
        (letter_modifiers(&event->key) &&
         uppercase_letter(&event->key, event->key.key))))
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_QUIT}, true, false);
    if (birth->owner == SV_BIRTH_MODE && key >= 'A' && key <= 'Z')
        key += 'a' - 'A';
    if (birth->owner == SV_BIRTH_MODE && letter_key(key) &&
        letter_modifiers(&event->key)) {
        bool uppercase = uppercase_letter(&event->key, key);
        SvBirthMode mode;
        if (birth->dedicated)
            mode = uppercase && key == SDLK_H ? SV_BIRTH_MODE_IDDC_HELLISH :
                !uppercase && key == SDLK_I ? SV_BIRTH_MODE_IDDC :
                !uppercase && key == SDLK_S ? SV_BIRTH_MODE_IDDC_SOLOIST :
                !uppercase && key == SDLK_P ? SV_BIRTH_MODE_DEDICATED_PVP :
                SV_BIRTH_MODE_NONE;
        else
            mode = uppercase && key == SDLK_H ? SV_BIRTH_MODE_HELLISH :
                !uppercase && key == SDLK_N ? SV_BIRTH_MODE_NORMAL :
                !uppercase && key == SDLK_F ? SV_BIRTH_MODE_FRUIT_BAT :
                !uppercase && key == SDLK_G ? SV_BIRTH_MODE_NO_GHOST :
                !uppercase && key == SDLK_S ? SV_BIRTH_MODE_SOLOIST :
                !uppercase && key == SDLK_E ? SV_BIRTH_MODE_EVERLASTING :
                !uppercase && key == SDLK_P ? SV_BIRTH_MODE_PVP :
                SV_BIRTH_MODE_NONE;
        return select_and_confirm(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_MODE, .mode = mode});
    } else if (birth->owner == SV_BIRTH_STATS &&
        lowercase_letter(&event->key, key)) {
        unsigned stat = (unsigned)(key - SDLK_A);
        if (stat >= 6) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_STAT, .stat = (int)stat},
            true, false);
    } else if (birth->owner == SV_BIRTH_BODY &&
        lowercase_letter(&event->key, key)) {
        SvBirthBody body = key == SDLK_N ? SV_BIRTH_BODY_NORMAL :
            key == SDLK_F ? SV_BIRTH_BODY_FRUIT_BAT : SV_BIRTH_BODY_NONE;
        if (body == SV_BIRTH_BODY_NONE) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return select_and_confirm(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_BODY, .body = body});
    } else if (birth->owner == SV_BIRTH_TRAIT &&
        lowercase_letter(&event->key, key)) {
        unsigned position = (unsigned)(key - SDLK_A);
        if (position >= birth->legal_trait_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return select_and_confirm(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_TRAIT,
                .trait_id = birth->legal_traits[position]});
    } else if (birth->owner == SV_BIRTH_RACE &&
        lowercase_letter(&event->key, key)) {
        unsigned id = (unsigned)(key - SDLK_A);
        if (id >= birth->setup->race_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return select_and_confirm(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_RACE,
                .race_id = (int)id});
    } else if (birth->owner == SV_BIRTH_CLASS &&
        lowercase_letter(&event->key, key)) {
        unsigned position = (unsigned)(key - SDLK_A);
        if (position >= birth->visible_class_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return select_and_confirm(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_CLASS,
                .class_id = birth->visible_classes[position]});
    }
    if (viewport && birth->owner >= SV_BIRTH_CLASS &&
        birth->owner <= SV_BIRTH_MODE &&
        (key == SDLK_PAGEUP || key == SDLK_PAGEDOWN) &&
        choice_count(birth)) {
        int current = highlighted_position(birth);
        unsigned count = choice_count(birth);
        unsigned amount = viewport->rows > 1 ? viewport->rows - 1 : 1;
        unsigned target = current < 0 ? 0 : (unsigned)current;
        if (key == SDLK_PAGEUP)
            target = target > amount ? target - amount : 0;
        else if (target + amount < count) target += amount;
        else target = count - 1;
        int direction = key == SDLK_PAGEUP ? -1 : 1;
        while (!selectable_position(birth, target)) {
            if (direction < 0 && target) --target;
            else if (direction > 0 && target + 1 < count) ++target;
            else break;
        }
        return apply(birth, generation, viewport,
                     select_position(birth, target), true, false);
    }
    if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        key == SDLK_ESCAPE)
        command = (SvBirthCommand){.kind = SV_BIRTH_CONFIRM};
    else if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        (key == SDLK_LEFT || key == SDLK_KP_4 ||
         (key == SDLK_MINUS && symbol_modifiers(&event->key))))
        command = (SvBirthCommand){.kind = SV_BIRTH_DECREASE_STAT};
    else if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        (key == SDLK_RIGHT || key == SDLK_KP_6 ||
         (key == SDLK_PLUS && symbol_modifiers(&event->key))))
        command = (SvBirthCommand){.kind = SV_BIRTH_INCREASE_STAT};
    else if (key == SDLK_UP || key == SDLK_LEFT || key == SDLK_KP_8 || key == SDLK_KP_4)
        command = (SvBirthCommand){.kind = SV_BIRTH_MOVE_PREVIOUS};
    else if (key == SDLK_DOWN || key == SDLK_RIGHT || key == SDLK_KP_2 || key == SDLK_KP_6)
        command = (SvBirthCommand){.kind = SV_BIRTH_MOVE_NEXT};
    else if (key == SDLK_M && lowercase_letter(&event->key, key))
        command = (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX,
            .sex = SV_BIRTH_SEX_MALE};
    else if (key == SDLK_F && lowercase_letter(&event->key, key))
        command = (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX,
            .sex = SV_BIRTH_SEX_FEMALE};
    else if ((key == SDLK_ASTERISK || key == SDLK_KP_MULTIPLY) &&
        symbol_modifiers(&event->key))
        command = (SvBirthCommand){.kind = SV_BIRTH_RANDOM};
    else if (key == SDLK_HASH && symbol_modifiers(&event->key))
        command = (SvBirthCommand){.kind = SV_BIRTH_DNA};
    else if (key == SDLK_PERCENT && symbol_modifiers(&event->key))
        command = (SvBirthCommand){.kind = SV_BIRTH_AUTOMATIC_DNA};
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        command = (SvBirthCommand){.kind = SV_BIRTH_CONFIRM};
    else if (key == SDLK_BACKSPACE)
        command = (SvBirthCommand){.kind = SV_BIRTH_BACK};
    else {
        birth->illegal_ignored = true;
        return SV_BIRTH_INPUT_IGNORED;
    }
    if(birth->owner==SV_BIRTH_SEX && command.kind==SV_BIRTH_SELECT_SEX)
        return select_and_confirm(birth,generation,viewport,command);
    return apply(birth, generation, viewport, command, true, false);
}

#include "ui/hud.h"
#define CLIENT_SIDE
#include "../../../common/defines.h"
#include "ui/message-text.h"
#include "ui/hud-icons.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static const SDL_Color ink={209,203,187,255}, muted={130,127,116,255};
static const SDL_Color green={73,212,67,255}, red={233,70,56,255}, gold={223,189,99,255};
static bool option(const SvSettingsOptions *o,const char *name)
{ int i=sv_settings_option_slot(name);return o&&i>=0&&o->values[i]; }
SvHudPreferences sv_hud_preferences(const SvSettingsConfig *c,const SvSettingsOptions *o)
{
    SvHudPreferences p={0};
    p.small=c&&!strcmp(c->layout,"small");p.ui_scale_percent=c?c->ui_scale_percent:100;
    if(p.ui_scale_percent<1)p.ui_scale_percent=100;
    const char *bars[]={"hp_bar","mp_bar","st_bar"};
    const char *huge[]={"hp_huge_bar","mp_huge_bar","st_huge_bar","sn_huge_bar","stun_huge_bar"};
    for(int i=0;i<3;++i)p.bar[i]=option(o,bars[i]);
    for(int i=0;i<5;++i)p.huge[i]=option(o,huge[i]);
    p.linear_stats=option(o,"linear_stats");p.solid=option(o,"solid_bars");p.graphics=option(o,"huge_bars_gfx");p.experience_bar=option(o,"exp_bar");
    return p;
}
static void row(SvHudPresentation *p,const char *label,bool available,const char *fmt,...)
{
    if(p->row_count==SV_HUD_ROWS)return;
    SvHudRow *r=&p->rows[p->row_count++];
    *r=(SvHudRow){0};snprintf(r->label,sizeof(r->label),"%s",label);r->colour=available?ink:muted;
    if(!available){snprintf(r->value,sizeof(r->value),"Awaiting server");return;}
    va_list args;va_start(args,fmt);vsnprintf(r->value,sizeof(r->value),fmt,args);va_end(args);
}
static SDL_Color term_colour(unsigned attribute)
{
    static const SDL_Color palette[16]={{0,0,0,255},{240,240,232,255},{145,150,145,255},{230,144,65,255},
        {197,65,62,255},{69,160,89,255},{82,116,215,255},{159,119,75,255},{92,98,94,255},{245,245,245,255},
        {183,111,211,255},{235,209,92,255},{245,113,100,255},{135,223,131,255},{121,186,239,255},{207,174,120,255}};
    if(attribute==TERM_MULTI)return red;
    return palette[attribute<16?attribute:1];
}
static void colour_last(SvHudPresentation *p,SDL_Color c)
{ if(p->row_count)p->rows[p->row_count-1].colour=c; }
static void source_text(const char *source,char out[SV_STATUS_TEXT])
{
    SvMessage m={0};size_t n=0;while(n<sizeof(m.bytes)-1&&source[n])++n;
    memcpy(m.bytes,source,n);m.length=n;SvMessageText visual=sv_message_text(&m);
    memcpy(out,visual.text,visual.length+1);
}
static void formatted_last(SvHudPresentation *p,const char *source)
{
    if(!p->row_count)return;SvMessage m={0};
    while(m.length<sizeof(m.bytes)-1&&source[m.length]){m.bytes[m.length]=(unsigned char)source[m.length];++m.length;}
    SvMessageText visual=sv_message_text(&m);SvHudRow *r=&p->rows[p->row_count-1];
    memcpy(r->value,visual.text,visual.length+1);memcpy(r->colours,visual.colors,visual.length);r->formatted=true;
}
static void formatted_prefix(SvHudPresentation *p,const char *source)
{
    if(!p->row_count)return;SvHudRow *r=&p->rows[p->row_count-1];
    char value[SV_HUD_ROW_TEXT];memcpy(value,r->value,sizeof(value));
    formatted_last(p,source);size_t prefix=strlen(r->value);
    memset(r->colours+prefix,1,sizeof(r->colours)-prefix);memcpy(r->value,value,sizeof(value));
}
static const char *food_label(int v)
{ return v<PY_FOOD_FAINT?"Starved":v<PY_FOOD_WEAK?"Starved":v<PY_FOOD_ALERT?"Hungry":v<PY_FOOD_FULL?"Fed":v<PY_FOOD_MAX?"Full":"Gorged"; }
static const char *cut_label(int v)
{ return v>=CUT_MORTAL_WOUND?"Mortal wound":v>=200?"Deep gash":v>=100?"Severe cut":v>=50?"Nasty cut":v>=25?"Bad cut":v>=10?"Light cut":v?"Graze":"Clear"; }
static void prepare_status_formats(SvHudPresentation *p,const SvHud *h)
{
    /* The shared weapon slot follows prt_encumberment: easy > heavy > awkward.
     * Later writes win; the shooter slot independently gives heavy > awkward. */
    static const int slots[]={5,9,10,0,3,4,1,0,0,7,6,8,4,11,2};
    static const int order[]={7,3,8,6,14,4,12,5,0,10,9,11,1,2,13};
    for(int i=0;i<12;++i)p->encumbrance_slots[i]=-1;
    if(sv_hud_has(h,SV_HUD_ENCUMBRANCE))for(size_t i=0;i<SDL_arraysize(order);++i){int flag=order[i];if(h->encumbrance[flag])p->encumbrance_slots[slots[flag]]=flag;}
    /* /snbar is server-authoritative and skill gated. A nine-character star
     * bar discloses only its received segments, never a more precise numeric
     * ratio which the server may also send for the huge-bar representation. */
    char label[SV_STATUS_TEXT];source_text(h->sanity.label,label);p->sanity_bar_segments=-1;
    if(sv_hud_has(h,SV_HUD_SANITY)&&strlen(label)==9){int filled=0;bool valid=true,empty=false;
        for(int i=0;i<9;++i){if(label[i]=='-' )empty=true;else if(label[i]=='*'&&!empty)++filled;else valid=false;}
        if(valid)p->sanity_bar_segments=filled;
    }
    const SvHudExperience *e=&h->experience;
    if(e->maximum_level<e->maximum_player_level)SDL_snprintf(p->level_value,sizeof(p->level_value),"%d (%d)",e->level,e->maximum_player_level);
    else SDL_snprintf(p->level_value,sizeof(p->level_value),"%d",e->level);
    p->level_colour=term_colour(e->level<e->maximum_level?TERM_YELLOW:e->level<PY_MAX_PLAYER_LEVEL?TERM_L_GREEN:e->level<PY_MAX_LEVEL?TERM_L_UMBER:TERM_BLUE);
    p->experience_colour=term_colour(e->current<e->maximum?TERM_YELLOW:e->current<PY_MAX_EXP?TERM_L_GREEN:TERM_L_UMBER);
    double upper=e->advance,span=upper-e->previous;
    if(e->level>=PY_MAX_PLAYER_LEVEL||e->advance<=0||span<=0){upper=PY_MAX_EXP;span=upper-e->previous;}
    double current=(double)e->current+(e->fractional?.5:0);
    p->experience_fraction=span>0?SDL_clamp((current-e->previous)/span,0.,1.):current>=upper?1:0;
    /* No fractional bit is supplied for historical max XP. */
    p->experience_maximum_fraction=span>0?SDL_clamp(((double)e->maximum-e->previous)/span,0.,1.):e->maximum>=upper?1:0;
}
static void prepare_rows(SvHudPresentation *p,const SvHud *h)
{
    p->row_count=0;
    const SvHudResource *v[]={&h->hp,&h->mana,&h->stamina};
    const char *labels[]={"Health","Mana","Stamina"};
    for(int i=0;i<3;++i)row(p,labels[i],sv_hud_has(h,(SvHudField)i),"%d / %d%s%s",v[i]->current,v[i]->maximum,v[i]->boosted?"  boosted":"",v[i]->bar?"  bar":"");
    char safe[SV_STATUS_TEXT];source_text(h->sanity.label,safe);
    row(p,"Sanity",sv_hud_has(h,SV_HUD_SANITY),"%s",safe);if(sv_hud_has(h,SV_HUD_SANITY))formatted_last(p,h->sanity.label);if(sv_hud_has(h,SV_HUD_SANITY))colour_last(p,term_colour(h->sanity.attribute));
    if(sv_hud_has(h,SV_HUD_SANITY)&&!h->sanity.numeric_available)row(p,"Sanity values",true,"Not supplied by this server version");
    else row(p,"Sanity values",sv_hud_has(h,SV_HUD_SANITY),"%d / %d",h->sanity.current,h->sanity.maximum);
    if(sv_hud_has(h,SV_HUD_SANITY)&&!h->sanity.damage_available)row(p,"Sanity damage",true,"Not supplied by this server version");
    else row(p,"Sanity damage",sv_hud_has(h,SV_HUD_SANITY),"%d (colour %u)",h->sanity.damage,h->sanity.attribute);
    static const char *stats[]={"Strength","Intelligence","Wisdom","Dexterity","Constitution","Charisma"};
    for(int i=0;i<6;++i){const SvHudStat *s=&h->stats[i];row(p,stats[i],s->available,"%d / %d  base %d  index %d  temp %d%s",s->current,s->maximum,s->base,s->index,s->temporary,s->boosted?" boosted":"");if(s->available)colour_last(p,term_colour(s->current<s->maximum?TERM_YELLOW:s->boosted?TERM_L_BLUE:s->base<118?TERM_L_GREEN:TERM_L_UMBER));}
    row(p,"Armour",sv_hud_has(h,SV_HUD_ARMOUR),"%d %+d%s%s",h->armour.base,h->armour.bonus,h->armour.unknown?" unknown":"",h->armour.boosted?" boosted":"");if(sv_hud_has(h,SV_HUD_ARMOUR))colour_last(p,h->armour.unknown?muted:h->armour.boosted?term_colour(TERM_L_BLUE):green);
    source_text(h->demographics.body,safe);row(p,"Body",sv_hud_has(h,SV_HUD_DEMOGRAPHICS),"%s",safe);
    row(p,"Details",sv_hud_has(h,SV_HUD_DEMOGRAPHICS),"Age %d  Height %d  Weight %d  Social %d",h->demographics.age,h->demographics.height,h->demographics.weight,h->demographics.social_class);
    row(p,"Melee hit / damage",sv_hud_has(h,SV_HUD_COMBAT),"%+d / %+d",h->combat.melee_hit,h->combat.melee_damage);
    row(p,"Ranged hit / damage",sv_hud_has(h,SV_HUD_COMBAT),"%+d / %+d",h->combat.ranged_hit,h->combat.ranged_damage);
    row(p,"Other hit / damage",sv_hud_has(h,SV_HUD_COMBAT),"%+d / %+d",h->combat.hit,h->combat.damage);
    row(p,"Level",sv_hud_has(h,SV_HUD_EXPERIENCE),"%d  max %d  player max %d%s",h->experience.level,h->experience.maximum_level,h->experience.maximum_player_level,h->experience.fractional?" fractional":"");
    row(p,"Experience",sv_hud_has(h,SV_HUD_EXPERIENCE),"%d / %d",h->experience.current,h->experience.maximum);
    row(p,"Exp thresholds",sv_hud_has(h,SV_HUD_EXPERIENCE),"Previous %d  Next %d",h->experience.previous,h->experience.advance);
    row(p,"Gold / bank",sv_hud_has(h,SV_HUD_CURRENCY),"%d / %d",h->currency.carried,h->currency.bank);
    row(p,"Confusion",sv_hud_has(h,SV_HUD_CONFUSION),"%s (%d)",h->confusion?"Confused":"Clear",h->confusion);
    row(p,"Poison",sv_hud_has(h,SV_HUD_POISON),"%s (%d)",h->poison&2?"Diseased":h->poison?"Poisoned":"Clear",h->poison);if(sv_hud_has(h,SV_HUD_POISON)&&h->poison)colour_last(p,term_colour((h->poison&2)||!(h->poison&4)?TERM_ORANGE:TERM_YELLOW));
    row(p,"Study",sv_hud_has(h,SV_HUD_STUDY),"%d",h->study);
    source_text(h->blows.label,safe);
    const char *blows=safe[0]?safe:h->blows.value==255?"Wraith":
#ifdef IND_WRAITH_PROB
        h->blows.value==254?"PbTrav":
#endif
        "Blows / round";
    row(p,"Blows / wraith",sv_hud_has(h,SV_HUD_BLOWS),"%s  %u (colour %u)",blows,h->blows.value,h->blows.attribute);if(sv_hud_has(h,SV_HUD_BLOWS)&&safe[0])formatted_prefix(p,h->blows.label);if(sv_hud_has(h,SV_HUD_BLOWS))colour_last(p,term_colour(h->blows.attribute));
    row(p,"Food",sv_hud_has(h,SV_HUD_FOOD),"%s (%d)",food_label(h->food),h->food);if(sv_hud_has(h,SV_HUD_FOOD))colour_last(p,term_colour(h->food<PY_FOOD_FAINT?TERM_L_RED:h->food<PY_FOOD_WEAK?TERM_ORANGE:h->food<PY_FOOD_ALERT?TERM_YELLOW:h->food<PY_FOOD_MAX?TERM_L_GREEN:TERM_GREEN));
    row(p,"Fear",sv_hud_has(h,SV_HUD_FEAR),"%s (%d)",h->fear?"Afraid":"Clear",h->fear);
    row(p,"Speed",sv_hud_has(h,SV_HUD_SPEED),"%+d%s%s",h->speed,h->speed_boosted?" boosted":"",sv_hud_has(h,SV_HUD_DEPTH)&&h->depth.no_tele?"  No-Tele":"");if(sv_hud_has(h,SV_HUD_SPEED))colour_last(p,term_colour(sv_hud_has(h,SV_HUD_DEPTH)&&h->depth.no_tele?TERM_L_DARK:h->speed_boosted?TERM_L_BLUE:h->speed>0?TERM_L_GREEN:h->speed<0?TERM_L_UMBER:TERM_WHITE));
    row(p,"Cut",sv_hud_has(h,SV_HUD_CUT),"%s (%d)",cut_label(h->cut),h->cut);if(sv_hud_has(h,SV_HUD_CUT)&&h->cut)colour_last(p,term_colour(h->cut>=CUT_MORTAL_WOUND?TERM_L_RED:h->cut>=100?TERM_RED:h->cut>=25?TERM_ORANGE:TERM_YELLOW));
    row(p,"Blind / hallucination",sv_hud_has(h,SV_HUD_BLIND),"%s (%d)",(h->blind&2)&&h->blind!=2?"Blind + hallucinating":h->blind==2?"Hallucinating":h->blind?"Blind":"Clear",h->blind);
    row(p,"Stun",sv_hud_has(h,SV_HUD_STUN),"%s (%d)",h->stun>100?"Knocked out":h->stun>50?"Heavy stun":h->stun?"Stunned":"Clear",h->stun);if(sv_hud_has(h,SV_HUD_STUN)&&h->stun)colour_last(p,term_colour(h->stun>100?TERM_RED:TERM_ORANGE));
    row(p,"Tracked monster",sv_hud_has(h,SV_HUD_MONSTER),h->monster.attribute?"%u / 10 (colour %u)":"Not tracking",h->monster.health,h->monster.attribute);if(sv_hud_has(h,SV_HUD_MONSTER))colour_last(p,h->monster.attribute?term_colour(h->monster.attribute):muted);
    row(p,"AFK",sv_hud_has(h,SV_HUD_AFK),"%s (%d)",h->afk?"Away":"Present",h->afk);
    static const char *enc[]={"Armour burden","Awkward armour","Gloves / helm","Heavy weapon","Heavy shield","Heavy launcher","Icky weapon","Awkward weapon","Two-hand grip","Weight burden","Monk armour","Rogue armour","Awkward launcher","Heavy swim","Heavy tool"};
    for(int i=0;i<15;++i)row(p,enc[i],sv_hud_has(h,SV_HUD_ENCUMBRANCE),"%u",h->encumbrance[i]);
    source_text(h->extra,safe);row(p,"Extra status",sv_hud_has(h,SV_HUD_EXTRA),"%s",safe);if(sv_hud_has(h,SV_HUD_EXTRA))formatted_last(p,h->extra);
    row(p,"Martyr",sv_hud_has(h,SV_HUD_MARTYR),"%s (%d)",h->martyr?"Active":"Clear",h->martyr);
    row(p,"Idle",sv_hud_has(h,SV_HUD_IDLE),"%s (%d)",h->idle?"Idle":"Active",h->idle);
    row(p,"Indicators",sv_hud_has(h,SV_HUD_INDICATORS),"0x%08x",(unsigned)h->indicators);
    static const char *bits[]={"Fire resist","Cold resist","Electric resist","Acid resist","Poison resist","Divine resist","ESP","Melee brand","Regeneration","Dispersion","Charm","Reflect shield","Lightning shield","Counter shield","Fire shield","Ice shield","Plasma shield","Mystic shield","Protection evil","Thunderstorm","Critical force","Probability travel"};
    if(sv_hud_has(h,SV_HUD_INDICATORS)){
        bool shield=false;
        for(int i=0;i<22;++i)if(h->indicators&(UINT32_C(1)<<i)){
            if(i>=11&&i<=17){if(shield)continue;shield=true;}
#ifdef IND_WRAITH_PROB
            if(i==21)continue;
#endif
            row(p,"Active indicator",true,"%s",bits[i]);
        }
    }
}
/* Current prototype CONDITION_DEFINITIONS palette. Only conditions without a
 * wire attribute use this palette; sanity/blows retain server colour roles. */
static SDL_Color css_colour(unsigned rgb)
{ return (SDL_Color){rgb>>16,(rgb>>8)&255,rgb&255,255}; }
static void prepare_conditions(SvHudPresentation *p,const SvHud *h)
{
    static const SvHudField fields[]={SV_HUD_FOOD,SV_HUD_BLIND,SV_HUD_CONFUSION,SV_HUD_FEAR,SV_HUD_POISON,SV_HUD_IDLE,SV_HUD_CUT,SV_HUD_STUN};
    static const SvHudIcon icons[]={SV_HUD_ICON_STATUS_FOOD,SV_HUD_ICON_STATUS_ESP,SV_HUD_ICON_STATUS_CONFUSED,SV_HUD_ICON_STATUS_FEAR,SV_HUD_ICON_STATUS_POISON,SV_HUD_ICON_STATUS_PARALYSIS,SV_HUD_ICON_STATUS_WOUND,SV_HUD_ICON_STATUS_STUN};
    for(int i=0;i<SV_HUD_CONDITION_COUNT;++i)p->conditions[i]=(SvHudCondition){.available=i!=SV_HUD_CONDITION_ACTIVITY&&sv_hud_has(h,fields[i]),.colour={139,141,128,255},.icon=icons[i]};
    SvHudCondition *c=&p->conditions[SV_HUD_CONDITION_FOOD];
    if(c->available&&(h->food<PY_FOOD_ALERT||h->food>=PY_FOOD_FULL)){
        c->active=true;unsigned colour;
        if(h->food<PY_FOOD_FAINT){colour=0xef5445;c->severity=3;}
        else if(h->food<PY_FOOD_WEAK){colour=0xe27e34;c->severity=2;c->critical=true;}
        else if(h->food<PY_FOOD_ALERT)colour=0xd9bd43;
        else if(h->food<PY_FOOD_MAX)colour=0x64c95c;
        else{colour=0x319c4c;c->severity=2;}
        c->colour=css_colour(colour);
    }
    c=&p->conditions[SV_HUD_CONDITION_VISION];
    if(c->available&&h->blind){c->active=true;
        if(h->blind==2){c->icon=SV_HUD_ICON_STATUS_HALLU;c->colour=css_colour(0xe4c641);}
        else{c->icon=SV_HUD_ICON_STATUS_BLIND;c->severity=(h->blind&2)?3:2;c->critical=!(h->blind&2);c->colour=css_colour((h->blind&2)?0xee6246:0xe08137);}}
    c=&p->conditions[SV_HUD_CONDITION_CONFUSION];if(c->available&&h->confusion){c->active=true;c->critical=true;c->severity=2;c->colour=css_colour(0xdf8436);}
    c=&p->conditions[SV_HUD_CONDITION_FEAR];if(c->available&&h->fear){c->active=true;c->critical=true;c->severity=2;c->colour=css_colour(0xdf8436);}
    c=&p->conditions[SV_HUD_CONDITION_POISON];
    if(c->available&&h->poison){c->active=true;
        if(h->poison&2){c->icon=SV_HUD_ICON_STATUS_DISEASE;c->severity=3;c->colour=css_colour(0xee6946);}
        else if(h->poison&4)c->colour=css_colour(0xd5c140);
        else{c->severity=2;c->critical=true;c->colour=css_colour(0xe78035);}}
    /* PKT_IDLE is audio inactivity, not a source of gameplay rest/search or
     * paralysis. The activity slot remains unknown without that gameplay source. */
    c=&p->conditions[SV_HUD_CONDITION_CUT];
    if(c->available&&h->cut){c->active=true;unsigned colour;
        if(h->cut>=CUT_MORTAL_WOUND){colour=0xfa493e;c->severity=4;}
        else if(h->cut>=200){colour=0xe23632;c->severity=3;}
        else if(h->cut>=100){colour=0xdf4539;c->severity=3;}
        else if(h->cut>=50){colour=0xe36b37;c->severity=2;}
        else if(h->cut>=25){colour=0xdf8734;c->severity=2;}
        else if(h->cut>=10){colour=0xe1ba3d;c->severity=1;}
        else{colour=0xdec647;c->severity=1;}c->colour=css_colour(colour);}
    c=&p->conditions[SV_HUD_CONDITION_STUN];
    if(c->available&&h->stun){c->active=true;
        if(h->stun>100){c->severity=4;c->colour=css_colour(0xe5483b);}
        else if(h->stun>50){c->severity=2;c->critical=true;c->colour=css_colour(0xe47731);}
        else{c->severity=1;c->colour=css_colour(0xe4a03a);}}
}
/* Actual fixed block extents mirror the draw order below. Source CSS uses the
 * short three-row icon grid; no guessed framebuffer height is involved. */
static float left_content_height(SvHudPreferences p,bool compact)
{
    bool expanded=!p.small&&(p.huge[0]||p.huge[1]||p.huge[3]);
    int ordinary=3-(expanded&&p.huge[0])-(expanded&&p.huge[1]);
    float top=compact?9:12,sanity=expanded&&p.huge[3]?46:47;
    float resources=ordinary*38-4+(expanded?4:9)*2-(expanded?0:9)+(p.huge[2]?12:0);
    float effects=(compact?20:25)+3*(compact?22:28)+2*(compact?2:3)+(compact?6:9);
    return top+50+41+39+43+132+sanity+resources+39+34+34+effects+(expanded?(compact?104:144):0)+100;
}
bool sv_hud_prepare(SvHudPresentation *p,SvAppView view,SvHudPreferences preferences,int w,int h,float scale,uint64_t font_revision)
{
    if(!p||w<=0||h<=0||scale<=0)return false;
    bool generation=!p->valid||p->generation!=view.generation;
    if(generation){p->character_open=false;p->scroll=0;p->widget_scroll=0;p->pointer_known=false;p->character_page=SV_HUD_CHARACTER_PROFILE;p->summary_page=SV_HUD_CHARACTER_PROFILE;p->dump_notice=false;p->scroll_max=0;p->widget_scroll_max=0;SDL_zero(p->page_scroll);SDL_zero(p->page_widget_scroll);}
    if(generation||p->revision!=view.hud.revision){prepare_rows(p,&view.hud);prepare_status_formats(p,&view.hud);prepare_conditions(p,&view.hud);p->revision=view.hud.revision;}
    if(!p->widths_selected){float initial=SDL_clamp(w/scale/1672.f,.82f,1.18f);p->left_width=(int)SDL_roundf(320*initial);p->right_width=(int)SDL_roundf(481*initial);p->widths_selected=true;}
    if(preferences.left_width>0)p->left_width=preferences.left_width;
    if(preferences.right_width>0)p->right_width=preferences.right_width;
    p->generation=view.generation;p->redraw_revision=view.redraw_revision;
    p->preferences=preferences;p->width=w;p->height=h;p->scale=scale*preferences.ui_scale_percent/100.f;p->font_revision=font_revision;p->valid=true;
    p->left_content_height=left_content_height(preferences,w/scale<=1400);
    p->left_scale=p->scale;
    if(preferences.small){p->left_scale=SDL_min(p->left_scale,(h-8*scale)/p->left_content_height);p->left_scale=SDL_min(p->left_scale,p->left_width*scale/220.f);}
    return true;
}
static bool fill(SDL_Renderer *r,float s,float x,float y,float w,float h,SDL_Color c)
{ SDL_FRect b={x*s,y*s,w*s,h*s};return SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a)&&SDL_RenderFillRect(r,&b); }
static bool text(SDL_Renderer *r,SvFont *f,float s,float x,float y,const char *str,SDL_Color c,float points)
{
    if(!str||!*str)return true;
    if(SDL_RenderClipEnabled(r)){SDL_Rect clip;SDL_GetRenderClipRect(r,&clip);if(y*s>=clip.y+clip.h||(y+points*3)*s<clip.y)return true;}
    SDL_Surface *surface=sv_font_render_surface(f,str,points,s,c,false);if(!surface)return false;
    SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);
    SDL_FRect rect={x*s,y*s,(float)surface->w,(float)surface->h};SDL_DestroySurface(surface);
    if(!texture)return false;bool ok=SDL_RenderTexture(r,texture,NULL,&rect);SDL_DestroyTexture(texture);return ok;
}
/* Bar text has CSS text-shadow, whose blur radius is twice Gaussian sigma.
 * Keep this local to the XP/resource overlays; it is not a second font path. */
typedef struct {
    SvFont *font;uint64_t font_revision;float scale,points,blur;SDL_Color colour;
    char text[128];SDL_Texture *texture;int padding,width,height;
} HudOverlayText;
typedef struct {HudOverlayText entries[24];unsigned next;} HudOverlayCache;
static void SDLCALL overlay_cache_free(void *userdata,void *value)
{ (void)userdata;SDL_free(value); /* Renderer owns all cached textures. */ }
static bool blur_text_alpha(const float *source,float *temporary,float *blurred,int width,int height,float sigma)
{
    int radius=(int)SDL_ceilf(3*sigma);float *kernel=SDL_malloc((2*radius+1)*sizeof(*kernel));if(!kernel)return false;
    float sum=0;for(int i=-radius;i<=radius;++i){float v=SDL_expf(-.5f*i*i/(sigma*sigma));kernel[i+radius]=v;sum+=v;}
    for(int i=0;i<2*radius+1;++i)kernel[i]/=sum;
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){float value=0;for(int k=-radius;k<=radius;++k)if(x+k>=0&&x+k<width)value+=source[(size_t)y*width+x+k]*kernel[k+radius];temporary[(size_t)y*width+x]=value;}
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){float value=0;for(int k=-radius;k<=radius;++k)if(y+k>=0&&y+k<height)value+=temporary[(size_t)(y+k)*width+x]*kernel[k+radius];blurred[(size_t)y*width+x]=value;}
    SDL_free(kernel);return true;
}
static HudOverlayText *overlay_texture(SDL_Renderer *r,SvFont *font,uint64_t font_revision,float scale,float points,SDL_Color colour,float blur,const char *str)
{
    if(strlen(str)>=sizeof(((HudOverlayText*)0)->text))return NULL;
    SDL_PropertiesID properties=SDL_GetRendererProperties(r);const char *key="sv.hud.bar-overlay-text";
    HudOverlayCache *cache=SDL_GetPointerProperty(properties,key,NULL);
    if(!cache){cache=SDL_calloc(1,sizeof(*cache));if(!cache)return NULL;if(!SDL_SetPointerPropertyWithCleanup(properties,key,cache,overlay_cache_free,NULL))return NULL;}
    for(size_t i=0;i<SDL_arraysize(cache->entries);++i){HudOverlayText *entry=&cache->entries[i];if(entry->texture&&entry->font==font&&entry->font_revision==font_revision&&entry->scale==scale&&entry->points==points&&entry->blur==blur&&!SDL_memcmp(&entry->colour,&colour,sizeof(colour))&&!strcmp(entry->text,str))return entry;}
    SDL_Surface *rendered=sv_font_render_surface(font,str,points,scale,colour,false);if(!rendered)return NULL;
    SDL_Surface *foreground=SDL_ConvertSurface(rendered,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(rendered);if(!foreground)return NULL;
    int padding=(int)SDL_ceilf(3*SDL_max(scale,blur*scale/2)+scale)+1;
    int width=foreground->w+2*padding,height=foreground->h+2*padding;size_t count=(size_t)width*height;
    float *source=SDL_calloc(count,sizeof(float)),*temporary=SDL_malloc(count*sizeof(float)),*blurred=SDL_malloc(count*sizeof(float)),*shadow=SDL_calloc(count,sizeof(float));
    SDL_Surface *surface=NULL;SDL_Texture *texture=NULL;
    if(!source||!temporary||!blurred||!shadow)goto done;
    for(int y=0;y<foreground->h;++y){const unsigned char *row=(const unsigned char*)foreground->pixels+y*foreground->pitch;for(int x=0;x<foreground->w;++x)source[(size_t)(y+padding)*width+x+padding]=row[x*4+3]/255.f;}
    /* Text-shadow layers independently sample the same foreground silhouette. */
    for(int pass=0;pass<2;++pass){float sigma=pass?blur*scale/2:scale,offset=pass?0:scale;
        if(!blur_text_alpha(source,temporary,blurred,width,height,sigma))goto done;
        for(int y=0;y<height;++y){float at=y-offset;int y0=(int)SDL_floorf(at);float t=at-y0;for(int x=0;x<width;++x){float a=y0>=0&&y0<height?blurred[(size_t)y0*width+x]*(1-t):0;if(y0+1>=0&&y0+1<height)a+=blurred[(size_t)(y0+1)*width+x]*t;size_t index=(size_t)y*width+x;shadow[index]+=a*(1-shadow[index]);}}
    }
    surface=SDL_CreateSurface(width,height,SDL_PIXELFORMAT_RGBA32);if(!surface)goto done;
    for(int y=0;y<height;++y){unsigned char *row=(unsigned char*)surface->pixels+y*surface->pitch;for(int x=0;x<width;++x){size_t index=(size_t)y*width+x;float a=source[index],out=a+shadow[index]*(1-a);const unsigned char *pixel=NULL;if(x>=padding&&x<padding+foreground->w&&y>=padding&&y<padding+foreground->h)pixel=(const unsigned char*)foreground->pixels+(y-padding)*foreground->pitch+(x-padding)*4;for(int c=0;c<3;++c)row[x*4+c]=pixel&&out>0?(Uint8)SDL_roundf(pixel[c]*a/out):0;row[x*4+3]=(Uint8)SDL_roundf(SDL_clamp(out,0,1)*255);}}
    texture=SDL_CreateTextureFromSurface(r,surface);if(texture&&!SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND)){SDL_DestroyTexture(texture);texture=NULL;}
done:
    SDL_DestroySurface(surface);SDL_DestroySurface(foreground);SDL_free(source);SDL_free(temporary);SDL_free(blurred);SDL_free(shadow);
    if(!texture)return NULL;
    HudOverlayText *entry=&cache->entries[cache->next++%SDL_arraysize(cache->entries)];if(entry->texture)SDL_DestroyTexture(entry->texture);
    *entry=(HudOverlayText){.font=font,.font_revision=font_revision,.scale=scale,.points=points,.blur=blur,.colour=colour,.texture=texture,.padding=padding,.width=width,.height=height};SDL_snprintf(entry->text,sizeof(entry->text),"%s",str);return entry;
}
static bool overlay_text(SDL_Renderer *r,SvFont *font,uint64_t font_revision,float scale,float x,float y,const char *str,SDL_Color colour,float points,float blur)
{
    if(!str||!*str)return true;HudOverlayText *entry=overlay_texture(r,font,font_revision,scale,points,colour,blur,str);if(!entry)return false;
    SDL_FRect target={x*scale-entry->padding,y*scale-entry->padding,entry->width,entry->height};return SDL_RenderTexture(r,entry->texture,NULL,&target);
}
static bool right_overlay_text(SDL_Renderer *r,SvFont *font,uint64_t font_revision,float scale,float right,float y,const char *str,SDL_Color colour,float points,float blur)
{
    int width,height;if(!sv_font_measure(font,str,points,scale,&width,&height,false))return false;return overlay_text(r,font,font_revision,scale,right-width/scale,y,str,colour,points,blur);
}
/* The prototype's forged surface is drawn natively: nested metal/brass rims,
 * blackened insets and enamel meters. These are presentation only. */
static bool rim(SDL_Renderer *r,float s,float x,float y,float w,float h,SDL_Color c)
{ SDL_FRect b={x*s,y*s,w*s,h*s};return SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a)&&SDL_RenderRect(r,&b); }
static bool line(SDL_Renderer *r,float s,float x,float y,float x2,float y2,SDL_Color c)
{ return SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a)&&SDL_RenderLine(r,x*s,y*s,x2*s,y2*s); }
/* CSS colour compositing uses sRGB values, as the prototype's gradients do. */
typedef struct { float r,g,b; } HudRgb;
typedef struct { float at; SDL_Color colour; } HudStop;
static HudRgb rgb(SDL_Color c) { return (HudRgb){c.r,c.g,c.b}; }
static HudRgb over(HudRgb b,SDL_Color c,float opacity)
{ float a=c.a/255.f*opacity;return (HudRgb){b.r+(c.r-b.r)*a,b.g+(c.g-b.g)*a,b.b+(c.b-b.b)*a}; }
static SDL_Color colour_mix(SDL_Color a,SDL_Color b,float t)
{ return (SDL_Color){a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t,a.a+(b.a-a.a)*t}; }
static SDL_Color gradient(const HudStop *stops,size_t n,float t)
{
    if(t<=stops[0].at)return stops[0].colour;
    for(size_t i=1;i<n;++i)if(t<=stops[i].at)return colour_mix(stops[i-1].colour,stops[i].colour,(t-stops[i-1].at)/(stops[i].at-stops[i-1].at));
    return stops[n-1].colour;
}
static float css_angle(float x,float y,float w,float h,float degrees)
{ float a=degrees*.01745329252f,dx=SDL_sinf(a),dy=-SDL_cosf(a);return .5f+((x-w*.5f)*dx+(y-h*.5f)*dy)/(SDL_fabsf(w*dx)+SDL_fabsf(h*dy)); }
static HudRgb forged_pixel(float x,float y,float w,float h)
{
    static const HudStop base[]={{0,{36,41,37,255}},{.42,{23,28,25,255}},{1,{12,16,14,255}}};
    static const HudStop slope[]={{0,{211,222,216,18}},{.18,{211,222,216,0}},{.44,{0,0,0,46}},{.66,{0,0,0,0}},{1,{185,198,192,13}}};
    static const HudStop seams[]={{0,{0,0,0,0}},{.11,{0,0,0,0}},{.114,{220,229,224,15}},{.119,{220,229,224,0}},{.39,{0,0,0,0}},{.395,{0,0,0,41}},{.402,{0,0,0,0}},{.74,{0,0,0,0}},{.744,{204,214,209,11}},{.75,{204,214,209,0}},{1,{0,0,0,0}}};
    HudRgb c=rgb(gradient(base,3,y/h));c=over(c,gradient(slope,5,css_angle(x,y,w,h,107)),1);c=over(c,gradient(seams,11,css_angle(x,y,w,h,91)),1);
    float a=96*.01745329252f,projection=x*SDL_sinf(a)-y*SDL_cosf(a);float stripe=SDL_fmodf(projection,17);
    if(stripe<1)c=over(c,(SDL_Color){220,228,224,5},1);else if(stripe>=8&&stripe<9)c=over(c,(SDL_Color){0,0,0,6},1);
    /* Back-to-front order of the five --forged-surface radial gradients. */
    static const struct{float cx,cy,rx,ry;SDL_Color middle,edge;float middle_at;} spot[]={
        {.57,.46,31,22,{182,193,188,17},{182,193,188,0},0},
        {.74,.87,74,38,{0,0,0,64},{0,0,0,0},0},
        {.36,.71,43,29,{158,171,164,23},{158,171,164,0},0},
        {.84,.28,68,42,{0,0,0,71},{42,48,45,15},.52},
        {.13,.16,54,31,{178,190,184,27},{65,73,69,11},.42}};
    for(size_t i=0;i<SDL_arraysize(spot);++i){float dx=(x-w*spot[i].cx)/spot[i].rx,dy=(y-h*spot[i].cy)/spot[i].ry,t=SDL_sqrtf(dx*dx+dy*dy);
        float end=i==0?.73f:i==1?.76f:i==2?.71f:i==3?.76f:.72f;
        if(t<end){SDL_Color k;if(spot[i].middle_at&&t<spot[i].middle_at)k=colour_mix(spot[i].middle,spot[i].edge,t/spot[i].middle_at);else {float start=spot[i].middle_at;SDL_Color from=start?spot[i].edge:spot[i].middle;SDL_Color transparent=from;transparent.a=0;k=colour_mix(from,transparent,(t-start)/(end-start));}c=over(c,k,1);}}
    /* inset 0 0 27px black, and the left iron edge's broad pale reflection. */
    float d=SDL_min(SDL_min(x,w-x),SDL_min(y,h-y));float shade=.65f*SDL_expf(-d*d/(2*13.5f*13.5f));
    c=over(c,(SDL_Color){0,0,0,255},shade);c=over(c,(SDL_Color){195,207,200,14},SDL_expf(-x*x/(2*35.f*35.f)));
    return c;
}
typedef struct {int width,height;float scale;SDL_Texture *texture;} HudRaster;
typedef struct {HudRaster raster[24];unsigned next;} HudRasterCache;
static void SDLCALL raster_cache_free(void *userdata,void *value)
{ (void)userdata;SDL_free(value); /* SDL_Renderer owns and destroys the cached textures. */ }
static SDL_Texture *forged_texture(SDL_Renderer *r,int width,int height,float scale)
{
    SDL_PropertiesID properties=SDL_GetRendererProperties(r);const char *key="sv.hud.forged-raster-cache";
    HudRasterCache *cache=SDL_GetPointerProperty(properties,key,NULL);
    if(!cache){cache=SDL_calloc(1,sizeof(*cache));if(!cache)return NULL;if(!SDL_SetPointerPropertyWithCleanup(properties,key,cache,raster_cache_free,NULL))return NULL;}
    for(size_t i=0;i<SDL_arraysize(cache->raster);++i){HudRaster *item=&cache->raster[i];if(item->texture&&item->width==width&&item->height==height&&item->scale==scale)return item->texture;}
    SDL_Surface *surface=SDL_CreateSurface(width,height,SDL_PIXELFORMAT_RGBA32);if(!surface)return NULL;
    for(int y=0;y<height;++y){unsigned char *pixel=(unsigned char *)surface->pixels+y*surface->pitch;for(int x=0;x<width;++x){HudRgb c=forged_pixel((x+.5f)/scale,(y+.5f)/scale,width/scale,height/scale);pixel[x*4]=(unsigned char)SDL_clamp(c.r,0,255);pixel[x*4+1]=(unsigned char)SDL_clamp(c.g,0,255);pixel[x*4+2]=(unsigned char)SDL_clamp(c.b,0,255);pixel[x*4+3]=255;}}
    SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);if(!texture)return NULL;
    HudRaster *slot=&cache->raster[cache->next++%SDL_arraysize(cache->raster)];if(slot->texture)SDL_DestroyTexture(slot->texture);*slot=(HudRaster){width,height,scale,texture};
    return texture;
}
static bool forged(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    if(w<=0||h<=0)return true;int pw=(int)SDL_ceilf(w*s),ph=(int)SDL_ceilf(h*s);SDL_Texture *t=forged_texture(r,pw,ph,s);if(!t)return false;
    SDL_FRect target={x*s,y*s,w*s,h*s};if(!SDL_RenderTexture(r,t,NULL,&target))return false;
    return rim(r,s,x,y,w,h,(SDL_Color){93,93,82,255})&&rim(r,s,x+2,y+2,w-4,h-4,(SDL_Color){20,22,17,255})&&rim(r,s,x+3,y+3,w-6,h-6,(SDL_Color){95,87,64,255});
}
static bool card(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    if(w<=0||h<=0)return true;SDL_BlendMode previous;SDL_GetRenderDrawBlendMode(r,&previous);if(!SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND))return false;
    bool ok=fill(r,s,x,y,w,h,(SDL_Color){1,3,2,168});
    /* --forged-inset: horizontal reflection/blackening over the parent surface. */
    for(int i=0;ok&&i<(int)SDL_ceilf(w*s);++i){float t=(float)i/(w*s);SDL_Color shade=t<.24f?(SDL_Color){190,201,195,(unsigned char)(11*(1-t/.24f))}:t<.73f?(SDL_Color){0,0,0,(unsigned char)(59*(t-.24f)/.49f)}:(SDL_Color){0,0,0,(unsigned char)(59*(1-(t-.73f)/.27f))};SDL_FRect band={x*s+i,y*s,1,h*s};ok=SDL_SetRenderDrawColor(r,shade.r,shade.g,shade.b,shade.a)&&SDL_RenderFillRect(r,&band);}
    ok=ok&&line(r,s,x,y,x+w,y,(SDL_Color){220,230,225,15})&&line(r,s,x,y+h-1,x+w,y+h-1,(SDL_Color){0,0,0,209});
    SDL_SetRenderDrawBlendMode(r,previous);return ok&&line(r,s,x,y+h,x+w,y+h,(SDL_Color){92,78,48,255});
}
static bool character_panel(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    SDL_BlendMode old;SDL_GetRenderDrawBlendMode(r,&old);if(!SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND))return false;
    bool ok=fill(r,s,x,y,w,h,(SDL_Color){1,3,2,130});
    for(int i=0;ok&&i<(int)SDL_ceilf(w*s);++i){float t=i/(w*s);SDL_Color c=t<.24f?(SDL_Color){190,201,195,(Uint8)(11*(1-t/.24f))}:t<.73f?(SDL_Color){0,0,0,(Uint8)(59*(t-.24f)/.49f)}:(SDL_Color){0,0,0,(Uint8)(59*(1-(t-.73f)/.27f))};SDL_FRect band={x*s+i,y*s,1,h*s};ok=SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a)&&SDL_RenderFillRect(r,&band);}
    SDL_SetRenderDrawBlendMode(r,old);return ok;
}
static bool block_card(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    if(!card(r,s,x,y,w,h))return false;
    /* .block::after: a 5px square rotated 45deg, right9px/bottom-3px. */
    float cx=x+w-11.5f,cy=y+h-.5f,radius=3.535534f;
    SDL_Vertex vertices[4]={{{cx*s,(cy-radius)*s},{.09f,.08f,.06f,1},{0,0}},{{(cx+radius)*s,cy*s},{.09f,.08f,.06f,1},{0,0}},{{cx*s,(cy+radius)*s},{.09f,.08f,.06f,1},{0,0}},{{(cx-radius)*s,cy*s},{.09f,.08f,.06f,1},{0,0}}};int indices[]={0,1,2,0,2,3};
    if(!SDL_RenderGeometry(r,NULL,vertices,4,indices,6))return false;
    SDL_FPoint points[5]={{cx*s,(cy-radius)*s},{(cx+radius)*s,cy*s},{cx*s,(cy+radius)*s},{(cx-radius)*s,cy*s},{cx*s,(cy-radius)*s}};
    return SDL_SetRenderDrawColor(r,128,105,61,255)&&SDL_RenderLines(r,points,5);
}
static bool level_badge(SDL_Renderer *r,float s,float cx,float cy)
{
    const float radius=21.92031f;int extent=(int)SDL_ceilf(radius*s),pixels=extent*2;SDL_Surface *surface=SDL_CreateSurface(pixels,pixels,SDL_PIXELFORMAT_RGBA32);if(!surface)return false;
    static const HudStop stops[]={{0,{41,37,26,255}},{.54,{18,19,14,255}},{.56,{5,6,5,255}},{1,{5,6,5,255}}};
    for(int y=0;y<pixels;++y){unsigned char *pixel=(unsigned char*)surface->pixels+y*surface->pitch;for(int x=0;x<pixels;++x){float u=(x+.5f-extent)/s,v=(y+.5f-extent)/s,edge=(radius-SDL_fabsf(u)-SDL_fabsf(v))*.70710678f;SDL_Color c=gradient(stops,4,SDL_sqrtf(u*u+v*v)/radius);if(edge<0)c.a=0;else if(edge<1)c=(SDL_Color){182,152,81,255};else if(edge<3)c=(SDL_Color){8,9,6,255};else if(edge<4)c=(SDL_Color){77,66,41,255};else if(edge>=4&&edge<5)c=(SDL_Color){61,53,30,255};pixel[x*4]=c.r;pixel[x*4+1]=c.g;pixel[x*4+2]=c.b;pixel[x*4+3]=c.a;}}
    SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);if(!texture)return false;SDL_FRect target={cx*s-extent,cy*s-extent,(float)pixels,(float)pixels};bool ok=SDL_RenderTexture(r,texture,NULL,&target);SDL_DestroyTexture(texture);return ok;
}
static bool inactive_enemy(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    SDL_BlendMode old;SDL_GetRenderDrawBlendMode(r,&old);if(!SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND))return false;
    bool ok=fill(r,s,x,y,w,h,(SDL_Color){2,3,2,148})&&rim(r,s,x-1,y-1,w+2,h+2,(SDL_Color){16,18,15,148})&&rim(r,s,x,y,w,h,(SDL_Color){37,40,33,148})&&line(r,s,x+1,y+1,x+w-1,y+1,(SDL_Color){0,0,0,148});SDL_SetRenderDrawBlendMode(r,old);return ok;
}
static bool wide_badge(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    SDL_BlendMode old;SDL_GetRenderDrawBlendMode(r,&old);if(!SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND))return false;bool ok=true;int rows=(int)SDL_ceilf(h*s);
    for(int i=0;ok&&i<rows;++i){SDL_Color c=colour_mix((SDL_Color){20,23,19,224},(SDL_Color){2,4,3,209},(float)i/rows);SDL_FRect stripe={x*s,y*s+i,w*s,1};ok=SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a)&&SDL_RenderFillRect(r,&stripe);}
    ok=ok&&rim(r,s,x,y,w,h,(SDL_Color){190,163,99,122})&&rim(r,s,x+1,y+1,w-2,h-2,(SDL_Color){0,0,0,140})&&line(r,s,x+2,y+1,x+w-2,y+1,(SDL_Color){255,255,255,20});SDL_SetRenderDrawBlendMode(r,old);return ok;
}
static bool right_text(SDL_Renderer *r,SvFont *f,float s,float right,float y,const char *v,SDL_Color c,float points)
{ int w,h;if(!*v)return true;if(!sv_font_measure(f,v,points,s,&w,&h,false))return false;return text(r,f,s,right-w/s,y,v,c,points); }
static bool clipped_text(SDL_Renderer *r,SvFont *f,float s,float x,float y,float width,const char *v,SDL_Color c,float points)
{
    SDL_Rect old;bool enabled=SDL_RenderClipEnabled(r);if(enabled)SDL_GetRenderClipRect(r,&old);
    SDL_Rect b={(int)(x*s),(int)(y*s),(int)(width*s),(int)(30*s)};
    if(enabled){SDL_Rect result;if(!SDL_GetRectIntersection(&b,&old,&result))return true;b=result;}
    bool ok=SDL_SetRenderClipRect(r,&b)&&text(r,f,s,x,y,v,c,points);
    return SDL_SetRenderClipRect(r,enabled?&old:NULL)&&ok;
}
/* Roles follow the final Northern Kingdom resource declarations. */
static bool enamel_kind(SDL_Renderer *r,float s,float x,float y,float w,float h,double current,double maximum,int role,bool solid)
{
    SDL_Color edge=role==0?(SDL_Color){141,117,66,255}:role==4?(SDL_Color){132,100,64,255}:(SDL_Color){130,108,67,255};
    if(!rim(r,s,x-2,y-2,w+4,h+4,(SDL_Color){69,71,63,255})||!rim(r,s,x-1,y-1,w+2,h+2,(SDL_Color){23,25,20,255})||!fill(r,s,x,y,w,h,(SDL_Color){3,3,3,255}))return false;
    float fraction=maximum>0?(float)(current/maximum):0;fraction=SDL_clamp(fraction,0,1);
    static const HudStop stops[][3]={
        {{0,{255,255,255,255}},{.57,{220,227,231,255}},{1,{143,153,159,255}}},
        {{0,{201,54,45,255}},{.62,{133,23,17,255}},{1,{75,12,8,255}}},
        {{0,{43,181,210,255}},{.61,{23,111,155,255}},{1,{11,56,88,255}}},
        {{0,{88,184,61,255}},{.62,{44,121,37,255}},{1,{21,63,21,255}}},
        {{0,{185,45,39,255}},{.62,{116,19,15,255}},{1,{62,9,7,255}}},
        {{0,{82,120,255,255}},{.52,{23,75,255,255}},{1,{8,41,216,255}}}};
    SDL_Color highlight=role==1?(SDL_Color){255,198,178,82}:role==2?(SDL_Color){192,246,255,87}:role==3?(SDL_Color){213,255,184,77}:role==4?(SDL_Color){255,194,171,71}:role==5?(SDL_Color){193,220,255,107}:(SDL_Color){255,255,255,168};
    int rows=(int)SDL_ceilf((h-2)*s);
    for(int i=0;i<rows;++i){float t=rows>1?(float)i/(rows-1):0;HudRgb c=rgb(gradient(stops[role],3,t));float highlight_end=role==0||role==5?.38f:.28f;
        if(t<highlight_end)c=over(c,highlight,1-t/highlight_end);c=over(c,(SDL_Color){0,0,0,255},.3f*SDL_expf(-(1-t)*(1-t)*h*h/32.f));
        SDL_FRect band={(x+1)*s,(y+1)*s+i,(w-2)*fraction*s,1};
        if(!SDL_SetRenderDrawColor(r,c.r,c.g,c.b,255)||!SDL_RenderFillRect(r,&band))return false;}
    if(!solid)for(float at=24;at<w*fraction;at+=24)if(!fill(r,s,x+at,y+1,1,h-2,(SDL_Color){12,17,12,255}))return false;
    return rim(r,s,x,y,w,h,edge)&&line(r,s,x+1,y+1,x+w-1,y+1,(SDL_Color){53,49,35,255});
}
static bool icon(SDL_Renderer *r,float s,float x,float y,float size,SvHudIcon id,SDL_Color colour)
{
    bool attr=id<=SV_HUD_ICON_ATTR_CHARISMA,line_icon=id==SV_HUD_ICON_SANITY||id==SV_HUD_ICON_ARMOUR,enc=id>=SV_HUD_ICON_ENC_EASY_WIELD&&id<=SV_HUD_ICON_ENC_HEAVY_SWIM,metric=id==SV_HUD_ICON_STATUS_SPEED||id==SV_HUD_ICON_STATUS_BPR;
    if(attr||line_icon||enc||metric){
        SDL_Color glow=attr?(SDL_Color){194,150,65,41}:colour;
        if(enc)glow.a=(unsigned char)(colour.a*.44f);else if(metric)glow.a=(unsigned char)(colour.a*.35f);
        SvHudIconShadow shadows[]={{{0,0,0,255},s,0,s},{glow,(enc||metric?4:3)*s,0,0}};
        return sv_hud_icon_filter_draw(r,id,(SDL_FRect){x*s,y*s,size*s,size*s},colour,shadows,2);
    }
    return sv_hud_icon_draw(r,id,(SDL_FRect){x*s,y*s,size*s,size*s},colour);
}
static bool right_source_text(SDL_Renderer *r,SvFont *f,float s,float right,float y,float width,const char *source,SDL_Color base,float points)
{
    SvMessage m={0};while(m.length<sizeof(m.bytes)-1&&source[m.length]){m.bytes[m.length]=(unsigned char)source[m.length];++m.length;}SvMessageText visual=sv_message_text(&m);if(!visual.length)return true;
    int full_width,full_height;if(!sv_font_measure(f,visual.text,points,s,&full_width,&full_height,false))return false;float at=SDL_max(right-width,right-full_width/s);SDL_Rect old;bool enabled=SDL_RenderClipEnabled(r);if(enabled)SDL_GetRenderClipRect(r,&old);SDL_Rect clip={(int)((right-width)*s),(int)(y*s),(int)(width*s),(int)SDL_ceilf((full_height/s+2)*s)};if(enabled){SDL_Rect intersection;if(!SDL_GetRectIntersection(&old,&clip,&intersection))return true;clip=intersection;}if(!SDL_SetRenderClipRect(r,&clip))return false;
    bool ok=true;for(size_t a=0;ok&&a<visual.length;){size_t b=a+1;while(b<visual.length&&visual.colors[b]==visual.colors[a])++b;char part[SV_STATUS_TEXT];memcpy(part,visual.text+a,b-a);part[b-a]=0;SDL_Color c=visual.colors[a]==1?base:term_colour(visual.colors[a]);ok=text(r,f,s,at,y,part,c,points);int run_width,run_height;if(ok)ok=sv_font_measure(f,part,points,s,&run_width,&run_height,false);if(ok)at+=run_width/s;a=b;}
    SDL_SetRenderClipRect(r,enabled?&old:NULL);return ok;
}
static bool pair(SDL_Renderer *r,SvFont *f,float s,float x,float y,float w,const char *label,const char *value,SDL_Color c,float points)
{
    if(SDL_RenderClipEnabled(r)){SDL_Rect clip;SDL_GetRenderClipRect(r,&clip);if(y*s>=clip.y+clip.h||(y+points*3)*s<clip.y)return true;}
    int vw,vh;if(!sv_font_measure(f,value,points,s,&vw,&vh,false))return false;
    float label_width=SDL_max(0,w-vw/s-10);
    if(label_width>0&&!clipped_text(r,f,s,x,y,label_width,label,(SDL_Color){255,255,255,255},points))return false;
    if(vw/s>w-label_width-6){float offset=label_width+6;if(!clipped_text(r,f,s,x+offset,y,SDL_max(1,w-offset),value,c,points))return false;}
    else if(!right_text(r,f,s,x+w,y,value,c,points))return false;
    for(float at=0;at<w;at+=2)if(!fill(r,s,x+at,y+points*1.17f,1,1,(SDL_Color){28,29,24,255}))return false;
    return true;
}
static bool tracked_text(SDL_Renderer *r,SvFont *f,float s,float x,float y,const char *label,SDL_Color colour,float points,float tracking)
{
    for(const char *a=label;*a;++a){char ch[2]={*a,0};int w,h;if(!text(r,f,s,x,y,ch,colour,points)||!sv_font_measure(f,ch,points,s,&w,&h,false))return false;x+=w/s+points*tracking;}return true;
}
static bool heading(SDL_Renderer *r,SvFont *f,float s,float x,float y,float w,const char *title,float points)
{ (void)w;return tracked_text(r,f,s,x,y,title,(SDL_Color){199,157,85,255},points,.07f); }
static void integer(char *out,size_t n,int64_t v)
{
    char digits[32];snprintf(digits,sizeof(digits),"%lld",(long long)v);size_t len=strlen(digits),at=0;
    for(size_t i=0;i<len&&at+1<n;++i){out[at++]=digits[i];if(digits[i]!='-'&&i+1<len&&(len-i-1)%3==0&&at+1<n)out[at++]=',';}out[at]=0;
}
/* Both the right widget and Character primary use the HTML's overview columns,
 * combat groups and detail sections. The larger source values remain below. */
static void stat_number(char *out,size_t size,int value,bool linear)
{
    int bonus=value-18;
    if(value<=18)snprintf(out,size,"%d",value);
    else if(bonus>=220)snprintf(out,size,linear?"**":"18/***");
    else if(linear)snprintf(out,size,"%d.%d",18+bonus/10,bonus%10);
    else snprintf(out,size,bonus>=100?"18/%03d":"18/%02d",bonus);
}
static void stat_display_number(char *out,size_t size,int value,bool linear)
{ char number[24];stat_number(number,sizeof(number),value,linear);if(!linear&&value>18)snprintf(out,size,"18 / %s",number+3);else SDL_strlcpy(out,number,size); }
static float character_tab_width(float width)
{ return width<600?SDL_max(48,(width-256)/3):104; }
static void character_select(SvHudPresentation *p,SvHudCharacterPage page)
{
    if(page==p->character_page)return;
    p->page_scroll[p->character_page]=p->scroll;p->page_widget_scroll[p->character_page]=p->widget_scroll;
    p->character_page=page;if(page!=SV_HUD_CHARACTER_RESISTS)p->summary_page=page;
    p->scroll=p->page_scroll[page];p->widget_scroll=p->page_widget_scroll[page];p->scroll_max=0;p->widget_scroll_max=0;p->dump_notice=false;
}
static bool character_resists(SvHudPresentation *p,SDL_Renderer *r,SvFont *f,float x,float y,float width,float height,bool primary,int *scroll)
{
    float s=p->scale,em=primary?12:9,pad=em*.7f,row=em+1,start=y,at=start+pad;
    SDL_Rect clip={(int)(x*s),(int)(y*s),(int)(width*s),(int)(height*s)};if(!SDL_SetRenderClipRect(r,&clip))return false;
    bool ok=true;
#define DRAW(v) do { if(!(v)){ok=false;goto done;} }while(0)
    /* This source has no item/body flag matrix yet. Unknown is deliberately
     * different from '.', which the baseline uses to assert absence. */
    const char *legend[]={"+ resist / flag","* immunity / max","- susceptibility","1-9 bonus","1-9 penalty","E ESP","s slay / sustain","k kill","@ player / body","? unknown"};
    const SDL_Color colours[]={{242,241,231,255},{228,189,77,255},{217,59,52,255},{105,223,99,255},{255,103,91,255},{89,200,239,255},{240,221,67,255},{242,241,231,255},{208,209,202,255},{130,127,116,255}};
    int widths[10],heights[10];float total=0;for(int i=0;i<10;++i){DRAW(sv_font_measure(f,legend[i],em,s,&widths[i],&heights[i],false));total+=widths[i]/s;}
    float legend_gap=SDL_clamp((width-2*pad-total)/9,em*.35f,em*1.1f),lx=x+pad;
    float gap=em*.45f,labelw=em*2.4f+3,cw=(width-2*pad-3*gap)/4,cell=(cw-labelw)/15;
    int glyph_width,glyph_height;DRAW(sv_font_measure(f,"M",em,s,&glyph_width,&glyph_height,false));
    int columns=4;
    /* Keep the wide prototype composition. Only a primary whose actual font
     * no longer fits the source cells reflows the same groups into two rows;
     * font/UI scale and the protected left HUD are never reduced to make room. */
    if(primary&&cell<glyph_width/s+2){float minimum=labelw+15*SDL_max(14,glyph_width/s+2);columns=width-2*pad>=2*minimum+gap?2:1;cw=(width-2*pad-(columns-1)*gap)/columns;cell=(cw-labelw)/15;}
    int group_rows=(4+columns-1)/columns;float group_height=em*1.55f+20*row+26,group_gap=em;
    float legend_height=em*2.05f;
    for(int i=0;i<10;++i){if(lx+widths[i]/s>x+width-pad+.1f){lx=x+pad;legend_height+=em*1.45f;}lx+=widths[i]/s+legend_gap;}
    float content_height=2*pad+legend_height+group_rows*group_height+(group_rows-1)*group_gap;
    int maximum=SDL_max(0,(int)SDL_ceilf(content_height-height));*scroll=SDL_clamp(*scroll,0,maximum);
    if(primary)p->scroll_max=maximum;else p->widget_scroll_max=maximum;
    start=y-*scroll;at=start+pad;lx=x+pad;
    for(int i=0;i<10;++i){if(lx+widths[i]/s>x+width-pad+.1f){lx=x+pad;at+=em*1.45f;}const char *space=SDL_strchr(legend[i],' ');size_t prefix=space?(size_t)(space-legend[i]):1;char mark[8];SDL_memcpy(mark,legend[i],prefix);mark[prefix]=0;int mw,mh;DRAW(text(r,f,s,lx,at,mark,colours[i],em));DRAW(sv_font_measure(f,mark,em,s,&mw,&mh,false));DRAW(text(r,f,s,lx+mw/s,at,legend[i]+prefix,(SDL_Color){132,137,129,255},em));lx+=widths[i]/s+legend_gap;}
    at+=em*2.05f;
    const char *titles[]={"RESISTANCES","ABILITIES","BONUSES","SLAYS / BRANDS"};
    const char *labels[4][19]={{"Fire","Cold","Elec","Acid","Pois","Blnd","Lite","Dark","Soun","Shrd","Nexu","Neth","Conf","Chao","Dise","Watr","Time","Mana","Mind"},{"Fear","Para","HLif","Tele","FFal","Lvtn","Clmb","SInv","Invs","Refl","Wrth","RgHP","RgMP","Food","Vamp","AuID","AMSh","AMFi","Aggr"},{"Spd","Slth","Srch","Infr","Lite","Tunn","Blow","Crit","Shot","Mght","MxHP","MxMP","Luck","STR","INT","WIS","DEX","CON","CHR"},{"Spid","Anim","Orcs","Trol","Gian","Drgn","Demn","Undd","Evil","Dgri","Good","Nonl","Uniq","Fire","Cold","Elec","Acid","Pois","Vorp"}};
    const int slots[]={0,1,2,2,3,4,5,6,7,8,9,10,11,12,13};
    for(int group=0;group<4;++group){float cx=x+pad+(group%columns)*(cw+gap),gy=at+(group/columns)*(group_height+group_gap);DRAW(text(r,f,s,cx,gy,titles[group],(SDL_Color){213,185,92,255},em));float top=gy+em*1.55f;
        DRAW(fill(r,s,cx,top,cw,row,(SDL_Color){8,10,7,255}));DRAW(text(r,f,s,cx,top,"Flag",(SDL_Color){174,176,168,255},em));
        for(int col=0;col<15;++col){char key[2]={(char)(col==14?'@':'a'+col),0};DRAW(text(r,f,s,cx+labelw+col*cell,top,key,(SDL_Color){216,189,101,255},em));}
        for(int j=0;j<19;++j){float yy=top+(j+1)*row;DRAW(text(r,f,s,cx,yy,labels[group][j],(SDL_Color){174,176,168,255},em));for(int col=0;col<15;++col)DRAW(text(r,f,s,cx+labelw+col*cell,yy,"?",(SDL_Color){100,105,98,255},em));}
        float foot=top+20*row;DRAW(fill(r,s,cx,foot,cw,26,(SDL_Color){8,10,7,255}));DRAW(line(r,s,cx,foot,cx+cw,foot,(SDL_Color){85,70,36,255}));DRAW(line(r,s,cx+labelw-1,top,cx+labelw-1,foot+26,(SDL_Color){109,88,46,255}));
        for(int col=0;col<15;++col){float iw=SDL_min(14,cell),ix=cx+labelw+col*cell+(cell-iw)/2;DRAW(sv_hud_icon_shadow_draw(r,(SvHudIcon)(SV_HUD_ICON_SOURCE_WEAPON+slots[col]),(SDL_FRect){ix*s,(foot+2)*s,iw*s,22*s},(SDL_Color){119,123,115,255},2*s,0,0));DRAW(sv_hud_icon_draw(r,(SvHudIcon)(SV_HUD_ICON_SOURCE_WEAPON+slots[col]),(SDL_FRect){ix*s,(foot+2)*s,iw*s,22*s},(SDL_Color){119,123,115,255}));}
    }
done:SDL_SetRenderClipRect(r,NULL);return ok;
#undef DRAW
}
static bool character_content(SvHudPresentation *p,SDL_Renderer *r,SvFont *f,SvAppView view,float x,float y,float width,float height,bool primary,int *scroll)
{
    float s=p->scale;const SvHud *h=&view.hud;
    if(p->character_page==SV_HUD_CHARACTER_RESISTS)return character_resists(p,r,f,x,y,width,height,primary,scroll);
    SDL_Rect clip={(int)(x*s),(int)(y*s),(int)(width*s),(int)(height*s)};
    if(!SDL_SetRenderClipRect(r,&clip))return false;
    float start=y-*scroll,em=primary?12:9,step=em*1.322f,padding=em*1.05f,at=start+em*.45f,gap=em*1.3f;char val[SV_HUD_ROW_TEXT];
#define DRAW(expression) do{if(!(expression))goto failed;}while(0)
    float left_col=(width-2*padding-16)*1.15f/2.15f,right_col=(width-2*padding-16)/2.15f,right=x+padding+left_col+16,overview_top=at;
    DRAW(text(r,f,s,x+padding,at,view.identity.name_available&&view.identity.name[0]?view.identity.name:"Character",(SDL_Color){0,255,255,255},em*1.25f));
    DRAW(text(r,f,s,x+padding,at+em*1.73f,"Level",(SDL_Color){255,255,255,255},em));DRAW(text(r,f,s,x+padding+em*3.6f,at+em*1.73f,sv_hud_has(h,SV_HUD_EXPERIENCE)?p->level_value:"--",p->level_colour,em));
    at+=em*3.45f;
    source_text(h->demographics.body,val);DRAW(pair(r,f,s,x+padding,at,left_col,"Body",sv_hud_has(h,SV_HUD_DEMOGRAPHICS)?val:"--",(SDL_Color){0,255,255,255},em));
    /* Fields outside the status core keep their place but do not acquire
     * invented prototype values. The full Character capability owns them. */
    DRAW(pair(r,f,s,x+padding,at+step,left_col,"Trait","--",muted,em));
    const char *progress_labels[]={"Experience","Max Exp","Exp to Adv."};
    int64_t progress_values[]={h->experience.current,h->experience.maximum,(int64_t)h->experience.advance-h->experience.current};if(progress_values[2]<0)progress_values[2]=0;
    for(int i=0;i<3;++i){integer(val,sizeof(val),progress_values[i]);DRAW(pair(r,f,s,x+padding,at+(i+2)*step,left_col,progress_labels[i],sv_hud_has(h,SV_HUD_EXPERIENCE)?val:"--",(SDL_Color){0,255,0,255},em));}
    DRAW(pair(r,f,s,x+padding,at+5*step,left_col,"Mode","--",muted,em));
    DRAW(pair(r,f,s,x+padding,at+6*step+em*1.08f,left_col,"Status","--",muted,em));
    integer(val,sizeof(val),h->currency.carried);DRAW(pair(r,f,s,x+padding,at+7*step+em*1.08f,left_col,"Gold",sv_hud_has(h,SV_HUD_CURRENCY)?val:"--",(SDL_Color){0,255,0,255},em));
    const char *resource_names[]={"HP","MP","Stamina"};const SvHudResource *resources[]={&h->hp,&h->mana,&h->stamina};
    for(int i=0;i<3;++i){snprintf(val,sizeof(val),"%d / %d",resources[i]->current,resources[i]->maximum);DRAW(pair(r,f,s,right,overview_top+(i==2?3:i)*step,right_col,resource_names[i],sv_hud_has(h,(SvHudField)i)?val:"--",resources[i]->current>=resources[i]->maximum?(SDL_Color){0,255,0,255}:resources[i]->current>resources[i]->maximum/10?gold:red,em));}
    DRAW(pair(r,f,s,right,overview_top+2*step,right_col,"Sanity",p->rows[3].value,p->rows[3].colour,em));
    DRAW(line(r,s,right,overview_top+4*step+em*.35f,right+right_col,overview_top+4*step+em*.35f,(SDL_Color){54,47,30,255}));
    static const char *stat_names[]={"STR","INT","WIS","DEX","CON","CHR"};
    for(int i=0;i<6;++i){stat_display_number(val,sizeof(val),h->stats[i].current,p->preferences.linear_stats);DRAW(pair(r,f,s,right,overview_top+em*6.15f+i*step,right_col,stat_names[i],h->stats[i].available?val:"--",p->rows[6+i].colour,em));}
    float overview_height=em*(3.45f+1.08f+1.f)+8*step;DRAW(line(r,s,right-8,overview_top,right-8,overview_top+overview_height-em*.8f,(SDL_Color){78,68,43,255}));at=overview_top+overview_height;
    DRAW(line(r,s,x,at,x+width,at,(SDL_Color){36,35,27,255}));at+=em*.65f;
    const char *groups[]={"MELEE","RANGED","OTHER"};float cw=(width-2*padding-2*em)/3;
    for(int i=0;i<3;++i){float cx=x+padding+i*(cw+em);DRAW(tracked_text(r,f,s,cx,at,groups[i],(SDL_Color){199,157,85,255},em*.9f,.06f));DRAW(line(r,s,cx,at+em*1.26f,cx+cw,at+em*1.26f,(SDL_Color){54,47,30,255}));
        for(int j=0;j<6;++j){const char *label="",*value="--";SDL_Color colour=muted;
            if(i<2){static const char *labels[2][6]={{"Fighting","Hit","Damage","Blows / Round","","Total AC"},{"Bows / Throw","Hit","Damage","Shots / Round","","Infra-Vision"}};label=labels[i][j];
                if(j==1||j==2){snprintf(val,sizeof(val),"%+d",i==0?(j==1?h->combat.melee_hit:h->combat.melee_damage):(j==1?h->combat.ranged_hit:h->combat.ranged_damage));if(sv_hud_has(h,SV_HUD_COMBAT)){value=val;colour=(SDL_Color){0,255,0,255};}}
                else if(i==0&&j==3&&sv_hud_has(h,SV_HUD_BLOWS)){source_text(h->blows.label,val);if(h->blows.value<254)SDL_snprintf(val,sizeof(val),"%u",h->blows.value);else if(!*val)SDL_snprintf(val,sizeof(val),"%s",h->blows.value==255?"Wraith":"PbTrav");value=val;colour=term_colour(h->blows.attribute);}
                else if(i==0&&j==5&&sv_hud_has(h,SV_HUD_ARMOUR)){snprintf(val,sizeof(val),"%d %+d%s",h->armour.base,h->armour.bonus,h->armour.unknown?" ?":"");if(!h->armour.bonus)snprintf(val,sizeof(val),"%d%s",h->armour.base,h->armour.unknown?" ?":"");value=val;colour=p->rows[12].colour;}}
            else {static const char *labels[]={"Stealth","Disarming","Perception","Magic Device","Saving Throw","Searching"};label=labels[j];}
            if(*label)DRAW(pair(r,f,s,cx,at+em*1.57f+j*step,cw,label,value,colour,em));}
    }
    at+=em*1.57f+6*step+em*.8f;DRAW(line(r,s,x,at,x+width,at,(SDL_Color){36,35,27,255}));
    if(primary){
        at+=em*.65f;
        if(p->character_page==SV_HUD_CHARACTER_PROFILE){
            DRAW(heading(r,f,s,x+padding,at,width-2*padding,"CHARACTER BACKGROUND",em));
            DRAW(text(r,f,s,x+padding,at+em*1.75f,"--",muted,em));at+=em*8.2f;
        }else{
            DRAW(heading(r,f,s,x+padding,at,width-2*padding,"DETAILS",em));at+=em*1.7f;
            const char *names[]={"Sex","Age","Height","Weight","Social Class"};int values[]={0,h->demographics.age,h->demographics.height,h->demographics.weight,h->demographics.social_class};
            int cols=SDL_clamp((int)((width-2*padding+gap)/(116+gap)),1,5);float dw=(width-2*padding-gap*(cols-1))/cols;
            for(int j=0;j<5;++j){SDL_snprintf(val,sizeof(val),"%d",values[j]);DRAW(pair(r,f,s,x+padding+(j%cols)*(dw+gap),at+(j/cols)*step,dw,names[j],j&&sv_hud_has(h,SV_HUD_DEMOGRAPHICS)?val:"--",(SDL_Color){0,255,255,255},em));}
            at+=((5+cols-1)/cols)*step+em*.8f;
        }
        DRAW(line(r,s,x,at,x+width,at,(SDL_Color){36,35,27,255}));
    }
    at+=em*.7f;
    if(sv_hud_has(h,SV_HUD_DEPTH)){source_text(h->depth.location,val);DRAW(text(r,f,s,x+padding,at,val[0]?val:"--",(SDL_Color){157,157,157,255},em));}
    else DRAW(text(r,f,s,x+padding,at,"--",(SDL_Color){157,157,157,255},em));
    at+=em*2.0f;
    {int maximum=(int)(at-start-height+12);if(maximum<0)maximum=0;if(*scroll>maximum)*scroll=maximum;if(primary)p->scroll_max=maximum;else p->widget_scroll_max=maximum;}
    return SDL_SetRenderClipRect(r,NULL);
failed:SDL_SetRenderClipRect(r,NULL);return false;
#undef DRAW
}
static bool coin(SDL_Renderer *r,float s,float x,float y)
{
    /* .gold-icon: 21px circular brass radial gradient, not a generic glyph. */
    static const HudStop stops[]={{0,{255,241,132,255}},{.32,{216,163,31,255}},{.72,{104,65,7,255}},{1,{104,65,7,255}}};
    int pixels=(int)SDL_ceilf(21*s);SDL_Surface *surface=SDL_CreateSurface(pixels,pixels,SDL_PIXELFORMAT_RGBA32);if(!surface)return false;
    for(int yy=0;yy<pixels;++yy){unsigned char *p=(unsigned char*)surface->pixels+surface->pitch*yy;for(int xx=0;xx<pixels;++xx){float u=(xx+.5f)/pixels,v=(yy+.5f)/pixels,d=SDL_sqrtf((u-.5f)*(u-.5f)+(v-.5f)*(v-.5f));SDL_Color c=gradient(stops,4,SDL_sqrtf((u-.35f)*(u-.35f)+(v-.30f)*(v-.30f))/.9f);if(d>.5f)c.a=0;else if(d>.455f)c=(SDL_Color){208,157,35,255};p[xx*4]=c.r;p[xx*4+1]=c.g;p[xx*4+2]=c.b;p[xx*4+3]=c.a;}}
    SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);if(!texture)return false;SDL_FRect target={x*s,y*s,21*s,21*s};bool ok=SDL_RenderTexture(r,texture,NULL,&target);SDL_DestroyTexture(texture);return ok;
}
typedef struct {int width,height,phase,severity;float scale;Uint32 colour;bool active,critical;SDL_Texture *texture;} HudTileRaster;
typedef struct {HudTileRaster raster[96];unsigned next;} HudTileCache;
static bool status_tile(SDL_Renderer *r,float s,float x,float y,float w,float h,SDL_Color colour,bool active,int severity,bool critical,int frame_phase)
{
    const char *key="sv.hud.status-tile-raster-cache";SDL_PropertiesID properties=SDL_GetRendererProperties(r);HudTileCache *cache=SDL_GetPointerProperty(properties,key,NULL);
    if(!cache){cache=SDL_calloc(1,sizeof(*cache));if(!cache)return false;if(!SDL_SetPointerPropertyWithCleanup(properties,key,cache,raster_cache_free,NULL))return false;}
    critical=active&&critical;int phase=critical?frame_phase:0;float pulse=phase/15.f;pulse=pulse*pulse*(3-2*pulse);
    int padding=critical?12:4,pw=(int)SDL_ceilf((w+2*padding)*s),ph=(int)SDL_ceilf((h+2*padding)*s);Uint32 code=((Uint32)colour.r<<24)|((Uint32)colour.g<<16)|((Uint32)colour.b<<8)|colour.a;SDL_Texture *texture=NULL;
    for(size_t i=0;i<SDL_arraysize(cache->raster);++i){HudTileRaster *item=&cache->raster[i];if(item->texture&&item->width==pw&&item->height==ph&&item->phase==phase&&item->severity==severity&&item->scale==s&&item->colour==code&&item->active==active&&item->critical==critical){texture=item->texture;break;}}
    if(!texture){SDL_Surface *surface=SDL_CreateSurface(pw,ph,SDL_PIXELFORMAT_RGBA32);if(!surface)return false;
        float opacity=1; /* Whole-tile CSS opacity is applied after group composition. */
        for(int row=0;row<ph;++row){unsigned char *pixel=(unsigned char*)surface->pixels+row*surface->pitch;for(int column=0;column<pw;++column){float u=(column+.5f)/s-padding,v=(row+.5f)/s-padding,qx=SDL_fabsf(u-w/2)-(w/2-2),qy=SDL_fabsf(v-h/2)-(h/2-2);float distance=SDL_sqrtf(SDL_max(qx,0)*SDL_max(qx,0)+SDL_max(qy,0)*SDL_max(qy,0))+SDL_min(SDL_max(qx,qy),0)-2;
            HudRgb c={0,0,0};float alpha=0;
            if(distance<=0){alpha=opacity;
                if(critical)c=rgb((SDL_Color){16,11,9,255});else c=rgb(colour_mix(active?(SDL_Color){25,28,24,255}:(SDL_Color){18,21,18,255},active?(SDL_Color){7,9,7,255}:(SDL_Color){5,7,5,255},v/h));
                float dx=u-w*.5f,dy=v-h*.35f,radius=SDL_sqrtf((w*.5f)*(w*.5f)+(h*.65f)*(h*.65f)),radial=SDL_sqrtf(dx*dx+dy*dy)/radius,end=critical?.70f:.68f;
                if(active&&radial<end){SDL_Color tint=colour;tint.a=255;c=over(c,tint,(critical?.28f:.17f)*(1-radial/end));}
                if(critical){float diagonal=css_angle(u,v,w,h,135);float stripe=diagonal<.44f?0:diagonal<.45f?(diagonal-.44f)/.01f:diagonal<.49f?1:diagonal<.50f?(1-(diagonal-.49f)/.01f):0;c=over(c,(SDL_Color){0,0,0,133},stripe);}
                if(distance> -1){SDL_Color edge=active?colour_mix(colour,(SDL_Color){36,40,32,255},.64f):(SDL_Color){66,71,64,255};if(critical){edge=colour_mix(colour,(SDL_Color){255,255,255,255},.12f*pulse);alpha=opacity*(.74f+.26f*pulse);}c=rgb(edge);}
                else if(distance> -2)c=rgb(critical?colour_mix((SDL_Color){23,8,6,255},(SDL_Color){37,10,7,255},pulse):(SDL_Color){8,10,8,255});
                else if(v<2)c=over(c,(SDL_Color){255,255,255,18},1);
            }else if(active){float blur=critical?(severity>=4?11:7):3,near=SDL_expf(-distance*distance/(2*blur*blur));alpha=(critical?.42f+.33f*pulse:.23f)*near*opacity;c=rgb(colour);}
            if(critical){float gray=c.r*.2126f+c.g*.7152f+c.b*.0722f,saturation=.92f+.22f*pulse,brightness=.92f+((severity>=4?1.3f:1.2f)-.92f)*pulse;c=(HudRgb){(gray+(c.r-gray)*saturation)*brightness,(gray+(c.g-gray)*saturation)*brightness,(gray+(c.b-gray)*saturation)*brightness};}
            if(!active){float gray=c.r*.2126f+c.g*.7152f+c.b*.0722f;c=(HudRgb){gray*.9664f+c.r*.0336f,gray*.9664f+c.g*.0336f,gray*.9664f+c.b*.0336f};}
            pixel[column*4]=SDL_clamp(c.r,0,255);pixel[column*4+1]=SDL_clamp(c.g,0,255);pixel[column*4+2]=SDL_clamp(c.b,0,255);pixel[column*4+3]=SDL_clamp(alpha*255,0,255);
        }}
        texture=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);if(!texture)return false;SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
        HudTileRaster *slot=&cache->raster[cache->next++%SDL_arraysize(cache->raster)];if(slot->texture)SDL_DestroyTexture(slot->texture);*slot=(HudTileRaster){pw,ph,phase,severity,s,code,active,critical,texture};
    }
    SDL_FRect target={(x-padding)*s,(y-padding)*s,(float)pw,(float)ph};return SDL_RenderTexture(r,texture,NULL,&target);
}
static SDL_Color critical_tint(SDL_Color colour,int severity,int phase)
{
    float pulse=phase/15.f;pulse=pulse*pulse*(3-2*pulse);float brightness=.92f+((severity>=4?1.3f:1.2f)-.92f)*pulse,saturation=.92f+.22f*pulse,gray=colour.r*.2126f+colour.g*.7152f+colour.b*.0722f;
    return (SDL_Color){SDL_clamp((gray+(colour.r-gray)*saturation)*brightness,0,255),SDL_clamp((gray+(colour.g-gray)*saturation)*brightness,0,255),SDL_clamp((gray+(colour.b-gray)*saturation)*brightness,0,255),colour.a};
}
static bool shield_mark(SDL_Renderer *r,float s,float x,float y,int type,SDL_Color c)
{
    /* The source's Unicode shield marks are geometry here, so the selected
     * encoded PCF font never substitutes an unsupported '?' glyph. */
    if(type==0){const float a[][2]={{7,0},{3,1},{1,4},{1,8},{4,10},{8,10},{10,7}};for(size_t i=1;i<SDL_arraysize(a);++i)if(!line(r,s,x+a[i-1][0],y+a[i-1][1],x+a[i][0],y+a[i][1],c))return false;return line(r,s,x+7,y,x+7,y+4,c)&&line(r,s,x+7,y,x+3,y,c);}
    if(type==1)return icon(r,s,x-1,y-1,13,SV_HUD_ICON_STATUS_ELEC,c);
    if(type==2)return line(r,s,x+2,y+2,x+9,y+9,c)&&line(r,s,x+9,y+2,x+2,y+9,c);
    if(type==3)return line(r,s,x,y+6,x+3,y+3,c)&&line(r,s,x+3,y+3,x+7,y+8,c)&&line(r,s,x+7,y+8,x+11,y+5,c);
    if(type==4)return line(r,s,x+5,y,x+5,y+11,c)&&line(r,s,x,y+5,x+11,y+5,c)&&line(r,s,x+1,y+1,x+10,y+10,c)&&line(r,s,x+10,y+1,x+1,y+10,c);
    if(type==5)return line(r,s,x+5,y,x+7,y+4,c)&&line(r,s,x+7,y+4,x+11,y+5,c)&&line(r,s,x+11,y+5,x+7,y+7,c)&&line(r,s,x+7,y+7,x+5,y+11,c)&&line(r,s,x+5,y+11,x+4,y+7,c)&&line(r,s,x+4,y+7,x,y+5,c)&&line(r,s,x,y+5,x+4,y+4,c)&&line(r,s,x+4,y+4,x+5,y,c);
    return line(r,s,x+5,y,x+11,y+5,c)&&line(r,s,x+11,y+5,x+5,y+11,c)&&line(r,s,x+5,y+11,x,y+5,c)&&line(r,s,x,y+5,x+5,y,c);
}
/* CSS opacity belongs to the composed tile, not independently to its
 * background and SVG. A renderer-owned scratch target flattens the group once. */
typedef struct {SDL_Texture *texture;int width,height;} HudTileGroup;
static bool condition_tile(SDL_Renderer *r,float s,float x,float y,float w,float h,SDL_Color colour,bool active,int severity,bool critical,int phase,SvHudIcon glyph,float icon_size,int shield)
{
    SDL_PropertiesID properties=SDL_GetRendererProperties(r);const char *key="sv.hud.tile-group-target";
    HudTileGroup *group=SDL_GetPointerProperty(properties,key,NULL);
    if(!group){group=SDL_calloc(1,sizeof(*group));if(!group)return false;if(!SDL_SetPointerPropertyWithCleanup(properties,key,group,raster_cache_free,NULL))return false;}
    critical=active&&critical;int padding=critical?12:4;
    float origin_x=SDL_floorf((x-padding)*s),origin_y=SDL_floorf((y-padding)*s);
    int width=(int)SDL_ceilf((x+w+padding)*s-origin_x),height=(int)SDL_ceilf((y+h+padding)*s-origin_y);
    if(!group->texture||group->width<width||group->height<height){
        int wanted_w=SDL_max(width,group->width),wanted_h=SDL_max(height,group->height);
        SDL_Texture *texture=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,wanted_w,wanted_h);
        if(!texture)return false;
        if(!SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND_PREMULTIPLIED)){SDL_DestroyTexture(texture);return false;}
        if(group->texture)SDL_DestroyTexture(group->texture);group->texture=texture;group->width=wanted_w;group->height=wanted_h;
    }
    SDL_Texture *previous=SDL_GetRenderTarget(r);SDL_Rect viewport,clip;bool clipped=SDL_RenderClipEnabled(r),viewport_set=SDL_RenderViewportSet(r);SDL_BlendMode blend;
    if(!SDL_GetRenderViewport(r,&viewport)||!SDL_GetRenderClipRect(r,&clip)||!SDL_GetRenderDrawBlendMode(r,&blend))return false;
    bool ok=SDL_SetRenderTarget(r,group->texture)&&SDL_SetRenderViewport(r,NULL)&&SDL_SetRenderClipRect(r,NULL)&&SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND)&&SDL_SetRenderDrawColor(r,0,0,0,0)&&SDL_RenderClear(r);
    float tx=x-origin_x/s,ty=y-origin_y/s;
    if(ok)ok=status_tile(r,s,tx,ty,w,h,colour,active,severity,critical,phase);
    if(!active){float gray=.2126f*colour.r+.7152f*colour.g+.0722f*colour.b;colour=(SDL_Color){gray*.9664f+colour.r*.0336f,gray*.9664f+colour.g*.0336f,gray*.9664f+colour.b*.0336f,255};}
    else if(critical)colour=critical_tint(colour,severity,phase);
    if(ok)ok=icon(r,s,tx+(w-icon_size)/2,ty+(h-icon_size)/2,icon_size,glyph,colour);
    if(ok&&shield>=0){float mark_scale=s*(icon_size==15?.72f:1);ok=shield_mark(r,mark_scale,tx*s/mark_scale+(w*s/mark_scale-11)/2,ty*s/mark_scale+(h*s/mark_scale-11)/2,shield,colour);}
    /* Preserve automatic viewport semantics: restoring its old rectangle as an
     * explicit viewport freezes the window size across subsequent resizes. */
    bool restored=SDL_SetRenderTarget(r,previous)&&SDL_SetRenderViewport(r,viewport_set?&viewport:NULL)&&SDL_SetRenderClipRect(r,clipped?&clip:NULL)&&SDL_SetRenderDrawBlendMode(r,blend);
    if(!ok||!restored)return false;
    float pulse=phase/15.f;pulse=pulse*pulse*(3-2*pulse);float dim=severity>=4?.72f:.78f;
    Uint8 opacity=(Uint8)(255*(!active?.25f:critical?dim+(1-dim)*pulse:1));
    /* Render targets contain premultiplied RGB: modulate both RGB and alpha. */
    SDL_FRect source={0,0,width,height},target={origin_x,origin_y,width,height};
    return SDL_SetTextureColorMod(group->texture,opacity,opacity,opacity)&&SDL_SetTextureAlphaMod(group->texture,opacity)&&SDL_RenderTexture(r,group->texture,&source,&target);
}
static bool wide_ground(SDL_Renderer *r,float s,float x,float y,float w,float h,int kind)
{
    SDL_Color first=kind==0?(SDL_Color){57,5,4,255}:kind==1?(SDL_Color){3,11,37,255}:(SDL_Color){50,16,8,255};
    SDL_Color middle=kind==0?(SDL_Color){118,11,8,255}:kind==1?(SDL_Color){7,25,75,255}:(SDL_Color){101,30,13,255};
    SDL_Color last=kind==0?(SDL_Color){48,3,2,255}:kind==1?(SDL_Color){2,8,31,255}:(SDL_Color){41,11,5,255};HudStop stops[]={{0,first},{.48,middle},{1,last}};
    int cols=(int)SDL_ceilf(w*s);for(int i=0;i<cols;++i){SDL_Color c=gradient(stops,3,(float)i/cols);SDL_FRect band={x*s+i,y*s,1,h*s};if(!SDL_SetRenderDrawColor(r,c.r,c.g,c.b,255)||!SDL_RenderFillRect(r,&band))return false;}return true;
}
static bool wide_reflection(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    SDL_BlendMode old;SDL_GetRenderDrawBlendMode(r,&old);if(!SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND))return false;bool ok=true;
    int cols=(int)SDL_ceilf(w*s);for(int i=0;ok&&i<cols;++i){float t=(float)i/cols;SDL_Color c=t<.22f?(SDL_Color){213,236,255,(unsigned char)(77*(1-t/.22f))}:t>.76f?(SDL_Color){0,0,0,(unsigned char)(46*(t-.76f)/.24f)}:(SDL_Color){0,0,0,0};SDL_FRect band={x*s+i,y*s,1,h*s};ok=SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a)&&SDL_RenderFillRect(r,&band);}SDL_SetRenderDrawBlendMode(r,old);return ok;
}
static bool xp_drained(SDL_Renderer *r,float s,float x,float y,float w,float h)
{
    static const HudStop stops[]={{0,{185,138,40,255}},{.68,{114,80,19,255}},{1,{62,42,11,255}}};int rows=(int)SDL_ceilf(h*s);
    for(int row=0;row<rows;++row){float t=rows>1?(float)row/(rows-1):0;HudRgb c=rgb(gradient(stops,3,t));if(t<.30f)c=over(c,(SDL_Color){255,241,158,71},1-t/.3f);c=over(c,(SDL_Color){0,0,0,77},SDL_expf(-(1-t)*(1-t)*h*h/32));float jag=t<.21f?3-2*t/.21f:t<.43f?1+2*(t-.21f)/.22f:t<.61f?3-3*(t-.43f)/.18f:t<.79f?3*(t-.61f)/.18f:3-2*(t-.79f)/.21f;SDL_FRect band={(x+jag)*s,y*s+row,SDL_max(0,(w-jag)*s),1};if(!SDL_SetRenderDrawColor(r,c.r,c.g,c.b,245)||!SDL_RenderFillRect(r,&band))return false;}
    return true;
}
typedef struct {
    SDL_Texture *texture;
    uint64_t generation,revision,redraw,font_revision;
    SvFont *font;
    int width,height,left_width,right_width,widget_scroll;
    unsigned preferences;
    uint64_t pulse_bucket;
    float scale,left_scale;
} HudBackdrop;
static bool character_backdrop(SvHudPresentation *p,SDL_Renderer *r,SvFont *font,SvAppView view)
{
    SDL_PropertiesID properties=SDL_GetRendererProperties(r);const char *key="sv.hud.character-backdrop";
    const SvHudPreferences *prefs=&p->preferences;unsigned signature=((unsigned)p->character_page<<14)|prefs->small|(prefs->solid<<1)|(prefs->graphics<<2)|(prefs->experience_bar<<3)|(prefs->linear_stats<<4);
    for(int i=0;i<4;++i)signature|=prefs->bar[i]<<(5+i);for(int i=0;i<5;++i)signature|=prefs->huge[i]<<(9+i);
    bool animated=false;for(int i=0;i<SV_HUD_CONDITION_COUNT;++i){const SvHudCondition *condition=&p->conditions[i];if(condition->active&&(condition->critical||condition->severity>=3)){animated=true;break;}}
    /* The native pulse has 16 phases per 1.25s half-cycle. Refresh the blurred
     * backdrop at 10Hz only while such a tile is active; static scenes retain
     * their cache. Animation must not freeze when the primary opens. */
    uint64_t pulse_bucket=animated?SDL_GetTicks()/100:0;
    HudBackdrop *cache=SDL_GetPointerProperty(properties,key,NULL);
    if(!cache){cache=SDL_calloc(1,sizeof(*cache));if(!cache)return false;if(!SDL_SetPointerPropertyWithCleanup(properties,key,cache,raster_cache_free,NULL))return false;}
    if(!cache->texture||cache->generation!=view.generation||cache->revision!=view.revision||cache->redraw!=view.redraw_revision||cache->font!=font||cache->font_revision!=p->font_revision||cache->width!=p->width||cache->height!=p->height||cache->scale!=p->scale||cache->left_scale!=p->left_scale||cache->left_width!=p->left_width||cache->right_width!=p->right_width||cache->widget_scroll!=p->widget_scroll||cache->preferences!=signature||cache->pulse_bucket!=pulse_bucket){
        /* Only the already drawn production scene is sampled. This is a UI
         * effect cache, never a reference bitmap or a second scene renderer. */
        SDL_Surface *read=SDL_RenderReadPixels(r,NULL);if(!read)return false;
        SDL_Surface *surface=SDL_ConvertSurface(read,SDL_PIXELFORMAT_RGBA32);SDL_DestroySurface(read);if(!surface)return false;
        size_t count=(size_t)surface->w*surface->h;
        float *source=SDL_malloc(count*sizeof(float)),*temporary=SDL_malloc(count*sizeof(float)),*blurred=SDL_malloc(count*sizeof(float));
        bool ok=source&&temporary&&blurred;
        for(int channel=0;ok&&channel<3;++channel){
            for(int y=0;y<surface->h;++y){const unsigned char *row=(const unsigned char*)surface->pixels+y*surface->pitch;for(int x=0;x<surface->w;++x)source[(size_t)y*surface->w+x]=row[x*4+channel];}
            ok=blur_text_alpha(source,temporary,blurred,surface->w,surface->h,1.5f*p->scale);
            if(ok)for(int y=0;y<surface->h;++y){unsigned char *row=(unsigned char*)surface->pixels+y*surface->pitch;for(int x=0;x<surface->w;++x)row[x*4+channel]=(Uint8)SDL_roundf(blurred[(size_t)y*surface->w+x]);}
        }
        SDL_free(source);SDL_free(temporary);SDL_free(blurred);
        if(ok)for(int y=0;y<surface->h;++y){unsigned char *row=(unsigned char*)surface->pixels+y*surface->pitch;for(int x=0;x<surface->w;++x){unsigned char *pixel=row+x*4;float gray=.2126f*pixel[0]+.7152f*pixel[1]+.0722f*pixel[2];for(int c=0;c<3;++c){float saturated=gray+(pixel[c]-gray)*.72f;pixel[c]=(Uint8)SDL_roundf(saturated*.24f+(c==1?2:c==2?1:0)*.76f);}pixel[3]=255;}}
        SDL_Texture *texture=ok?SDL_CreateTextureFromSurface(r,surface):NULL;SDL_DestroySurface(surface);if(!texture)return false;
        if(cache->texture)SDL_DestroyTexture(cache->texture);
        *cache=(HudBackdrop){texture,view.generation,view.revision,view.redraw_revision,p->font_revision,font,p->width,p->height,p->left_width,p->right_width,p->widget_scroll,signature,pulse_bucket,p->scale,p->left_scale};
    }
    SDL_FRect target={0,0,p->width,p->height};return SDL_RenderTexture(r,cache->texture,NULL,&target);
}
static bool character_head(SvHudPresentation *p,SDL_Renderer *r,SvFont *font,float s,float x,float y,float width,bool primary)
{
    float height=primary?38:30;
    if(!fill(r,s,x,y,width,height,(SDL_Color){8,11,8,255}))return false;
    SDL_BlendMode old;SDL_GetRenderDrawBlendMode(r,&old);SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);for(int j=0;j<(int)height;++j)if(!fill(r,s,x,y+j,width,1,(SDL_Color){141,112,55,(Uint8)(41*SDL_max(0,1-j/(height*.7f)))})){SDL_SetRenderDrawBlendMode(r,old);return false;}SDL_SetRenderDrawBlendMode(r,old);
    if(!line(r,s,x,y+height-1,x+width,y+height-1,(SDL_Color){141,113,56,255})||!tracked_text(r,font,s,x+13,y+(primary?10:9),"CHARACTER",(SDL_Color){228,196,83,255},primary?13:10,.09f))return false;
    if(!primary){
        const char *labels[]={"PROFILE / SKILLS","RESISTS"};const float widths[]={98,50};float tx=x+width-151;
        for(int i=0;i<2;++i){float tab_width=widths[i];
            if(!line(r,s,tx,y,tx,y+height,(SDL_Color){58,49,27,255}))return false;
            if((p->character_page==SV_HUD_CHARACTER_RESISTS)==(i==1)){SDL_BlendMode blend;SDL_GetRenderDrawBlendMode(r,&blend);SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);bool ok=fill(r,s,tx,y,tab_width,height,(SDL_Color){174,137,62,20});SDL_SetRenderDrawBlendMode(r,blend);if(!ok||!line(r,s,tx,y+height-1,tx+tab_width,y+height-1,(SDL_Color){157,122,53,255}))return false;}
            int tw,th;if(!sv_font_measure(font,labels[i],8,s,&tw,&th,false)||!text(r,font,s,tx+(tab_width-tw/s)/2,y+(height-th/s)/2,labels[i],((p->character_page==SV_HUD_CHARACTER_RESISTS)==(i==1))?(SDL_Color){255,255,0,255}:muted,8))return false;
            tx+=tab_width;
        }
        return true;
    }
    float close=primary?38:0,dump=primary?58:0,tab_width=primary?104:74;
    if(primary)tab_width=character_tab_width(width);
    float start=x+width-close-dump-3*tab_width;
    const char *labels[]={"PROFILE","SKILLS","RESISTS"};
    for(int i=0;i<3;++i){float tx=start+i*tab_width;
        if(!line(r,s,tx,y,tx,y+height,(SDL_Color){58,49,27,255}))return false;
        if(i==(int)p->character_page){SDL_BlendMode blend;SDL_GetRenderDrawBlendMode(r,&blend);SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);bool ok=fill(r,s,tx,y,tab_width,height,(SDL_Color){161,126,48,24});SDL_SetRenderDrawBlendMode(r,blend);if(!ok||!line(r,s,tx,y+height-2,tx+tab_width,y+height-2,(SDL_Color){200,157,64,255}))return false;}
        int tw,th;float points=primary?9:8;if(!sv_font_measure(font,labels[i],points,s,&tw,&th,false)||!text(r,font,s,tx+(tab_width-tw/s)/2,y+(height-th/s)/2,labels[i],i==(int)p->character_page?(SDL_Color){240,202,90,255}:muted,points))return false;
    }
    if(primary){float dx=x+width-close-dump;if(!line(r,s,dx,y,dx,y+height,(SDL_Color){58,49,27,255})||!text(r,font,s,dx+17,y+15,"DUMP",(SDL_Color){88,89,80,255},9)||!line(r,s,x+width-close,y,x+width-close,y+height,(SDL_Color){58,49,27,255}))return false;
        float cx=x+width-close/2,cy=y+height/2;SDL_Color cross={170,169,159,255};if(!line(r,s,cx-4,cy-4,cx+4,cy+4,cross)||!line(r,s,cx+4,cy-4,cx-4,cy+4,cross))return false;}
    return true;
}
bool sv_hud_draw(SvHudPresentation *p,SDL_Renderer *r,SvFont *font,SvAppView view,SvHudPreferences prefs,float display_scale,uint64_t font_revision)
{
    int w,h;if(!SDL_GetCurrentRenderOutputSize(r,&w,&h)||!sv_hud_prepare(p,view,prefs,w,h,display_scale,font_revision))return false;
    bool compact=w/display_scale<=1400,expanded=!prefs.small&&(prefs.huge[0]||prefs.huge[1]||prefs.huge[3]);
    float primary_scale=p->scale,s=p->left_scale;
    p->scale=s;float lw=w/s,lh=h/s,left=p->left_width*display_scale/s,pad=compact?10:14,top=compact?9:12,x=pad+10,width=left-2*x;
    if(!SDL_SetRenderDrawColor(r,2,3,2,255)||!SDL_RenderClear(r))return false;
    if(!view.active)return text(r,font,s,30,30,"Awaiting active session",muted,16);
    const SvHud *v=&view.hud;char val[SV_HUD_ROW_TEXT],other[80],tooltip[160]={0};float tooltip_y=0;
#define DRAW(expression) do{if(!(expression))return false;}while(0)
    DRAW(forged(r,s,0,0,left,lh));DRAW(line(r,s,left-2,0,left-2,lh,(SDL_Color){154,127,73,255}));DRAW(line(r,s,left-4,0,left-4,lh,(SDL_Color){74,80,74,255}));
    float at=top;DRAW(block_card(r,s,pad,at,left-2*pad,50));
    float xp_x=x,xp_w=width,xp_y=at+12.5f;
    DRAW(enamel_kind(r,s,xp_x,xp_y,xp_w,24,prefs.experience_bar?p->experience_fraction:0,1,p->experience_fraction>=.9?5:0,prefs.solid));
    if(prefs.experience_bar&&v->experience.maximum>v->experience.current){float begin=(float)p->experience_fraction,end=(float)p->experience_maximum_fraction;if(end>begin)DRAW(xp_drained(r,s,xp_x+xp_w*begin,xp_y+1,xp_w*(end-begin),22));}
    float dx=xp_x+8.5f,dy=xp_y+12;DRAW(level_badge(r,s,dx,dy));
    snprintf(val,sizeof(val),"%d",v->experience.level);int tw,th;DRAW(sv_font_measure(font,sv_hud_has(v,SV_HUD_EXPERIENCE)?val:"--",12,s,&tw,&th,false));DRAW(text(r,font,s,dx-tw/s/2,dy-th/s/2,sv_hud_has(v,SV_HUD_EXPERIENCE)?val:"--",p->level_colour,12));
    if(sv_hud_has(v,SV_HUD_EXPERIENCE)&&v->experience.maximum_level<v->experience.maximum_player_level){SDL_snprintf(val,sizeof(val),"(%d)",v->experience.maximum_player_level);DRAW(clipped_text(r,font,s,xp_x+31,xp_y+7,SDL_max(0,width-235),val,muted,9));}
    integer(val,sizeof(val),v->experience.current);integer(other,sizeof(other),v->experience.advance);size_t n=strlen(val);snprintf(val+n,sizeof(val)-n," / %s",other);DRAW(right_overlay_text(r,font,font_revision,s,xp_x+xp_w-7,xp_y+6,sv_hud_has(v,SV_HUD_EXPERIENCE)?val:"Awaiting server",v->experience.current<v->experience.maximum?p->experience_colour:(SDL_Color){238,232,215,255},11,3));at+=50;
    DRAW(block_card(r,s,pad,at,left-2*pad,41));DRAW(coin(r,s,x,at+10));DRAW(text(r,font,s,x+33,at+13,"GOLD",(SDL_Color){216,199,157,255},14));integer(val,sizeof(val),v->currency.carried);DRAW(right_text(r,font,s,left-x,at+11,sv_hud_has(v,SV_HUD_CURRENCY)?val:"--",(SDL_Color){76,232,61,255},16));at+=41;
    DRAW(block_card(r,s,pad,at,left-2*pad,39));float enc_x=pad+(compact?7:9),enc_w=left-2*enc_x,slot=enc_w/12;
    DRAW(fill(r,s,enc_x,at+7,enc_w,25,(SDL_Color){1,3,2,255}));DRAW(rim(r,s,enc_x,at+7,enc_w,25,(SDL_Color){94,82,53,255}));
    static const SvHudIcon enc_icons[]={SV_HUD_ICON_ENC_CUMBER_ARMOR,SV_HUD_ICON_ENC_AWKWARD_ARMOR,SV_HUD_ICON_ENC_CUMBER_GLOVE,SV_HUD_ICON_ENC_HEAVY_WIELD,SV_HUD_ICON_ENC_HEAVY_SHIELD,SV_HUD_ICON_ENC_HEAVY_SHOOT,SV_HUD_ICON_ENC_ICKY_WIELD,SV_HUD_ICON_ENC_AWKWARD_WIELD,SV_HUD_ICON_ENC_EASY_WIELD,SV_HUD_ICON_ENC_CUMBER_WEIGHT,SV_HUD_ICON_ENC_MONK_HEAVYARMOR,SV_HUD_ICON_ENC_ROGUE_HEAVYARMOR,SV_HUD_ICON_ENC_AWKWARD_SHOOT,SV_HUD_ICON_ENC_HEAVY_SWIM,SV_HUD_ICON_ENC_HEAVY_TOOL};
    static const SDL_Color enc_colours[]={{167,119,69,255},{166,107,224,255},{174,107,226,255},{227,72,61,255},{227,72,61,255},{227,72,61,255},{223,133,45,255},{225,197,63,255},{84,216,77,255},{239,106,88,255},{225,197,63,255},{67,132,217,255},{225,197,63,255},{85,191,230,255},{227,72,61,255}};
    static const char *enc_labels[]={"Armour burden","Mana-heavy armour","Casting gloves / helm","Heavy weapon","Heavy shield","Heavy shooter","Unsuitable weapon","Awkward weapon","Two-handed bonus","Overloaded pack","Monk armour","Rogue armour","Awkward shooter","Heavy swimming","Heavy tool"};
    bool enc=false;for(int i=0;i<12;++i)if(p->encumbrance_slots[i]>=0)enc=true;
    for(int i=0;i<12;++i){if(i)DRAW(line(r,s,enc_x+i*slot,at+8,enc_x+i*slot,at+31,(SDL_Color){31,30,21,255}));int j=p->encumbrance_slots[i];float size=compact?15:17;if(j>=0)DRAW(icon(r,s,enc_x+i*slot+(slot-size)/2,at+11,size,enc_icons[j],enc_colours[j]));
        if(p->pointer_known&&p->pointer_x>=(enc_x+i*slot)*s&&p->pointer_x<(enc_x+(i+1)*slot)*s&&p->pointer_y>=(at+7)*s&&p->pointer_y<(at+32)*s){snprintf(tooltip,sizeof(tooltip),"%s",!sv_hud_has(v,SV_HUD_ENCUMBRANCE)?"Encumbrance: awaiting server":j<0?"No encumbrance":enc_labels[j]);tooltip_y=at+34;}}
    if(!enc)DRAW(text(r,font,s,enc_x+enc_w*.28f,at+15,sv_hud_has(v,SV_HUD_ENCUMBRANCE)?"No encumbrance":"Awaiting server",(SDL_Color){85,91,85,255},9));at+=39;
    /* Preserve the prototype's Ping block in the composition. The production
     * telemetry owns the value; absent samples never become demo latency. */
    DRAW(block_card(r,s,pad,at,left-2*pad,43));DRAW(text(r,font,s,x,at+13,"PING",(SDL_Color){216,199,157,255},14));
    float ping_x=x+47,ping_w=left-x-ping_x;
    DRAW(fill(r,s,ping_x,at+10.5f,ping_w,21,(SDL_Color){2,4,3,255}));
    DRAW(rim(r,s,ping_x-1,at+9.5f,ping_w+2,23,(SDL_Color){41,44,38,255}));DRAW(rim(r,s,ping_x,at+10.5f,ping_w,21,(SDL_Color){119,99,59,255}));
    if(view.ping.average_count>0&&view.ping.latest_ms>=0){int low=view.ping.latest_ms/100*100;SDL_Color tint=view.ping.latest_ms<100?(SDL_Color){255,255,255,255}:view.ping.latest_ms<200?(SDL_Color){255,178,183,255}:view.ping.latest_ms<400?(SDL_Color){245,197,90,255}:(SDL_Color){233,85,69,255};float fraction=SDL_clamp((low+100)/1000.f,0,1);DRAW(fill(r,s,ping_x+1,at+11.5f,(ping_w-2)*fraction,19,tint));SDL_snprintf(val,sizeof(val),"%d-%d ms",low,low+100);}
    else SDL_strlcpy(val,"--",sizeof(val));
    DRAW(right_overlay_text(r,font,font_revision,s,left-x-7,at+14,val,(SDL_Color){238,233,220,255},12,4));at+=43;
    DRAW(block_card(r,s,pad,at,left-2*pad,132));static const char *stats[]={"Strength","Intelligence","Wisdom","Dexterity","Constitution","Charisma"};static const SDL_Color stat_colours[]={{207,158,81,255},{210,189,121,255},{185,154,233,255},{159,188,120,255},{174,185,182,255},{225,188,78,255}};
    for(int i=0;i<6;++i){float yy=at+9+i*19;DRAW(icon(r,s,x,yy,18,(SvHudIcon)(SV_HUD_ICON_ATTR_STRENGTH+i),stat_colours[i]));DRAW(text(r,font,s,x+27,yy+2,stats[i],(SDL_Color){212,201,178,255},14));stat_display_number(val,sizeof(val),v->stats[i].current,prefs.linear_stats);DRAW(right_text(r,font,s,left-x,yy+2,v->stats[i].available?val:"--",p->rows[6+i].colour,14));}at+=132;
    bool wide_sanity=expanded&&prefs.huge[3];float sanity_height=wide_sanity?39:47;DRAW(block_card(r,s,pad,at,left-2*pad,sanity_height));if(!wide_sanity){DRAW(icon(r,s,x,at+4,18,SV_HUD_ICON_SANITY,(SDL_Color){177,116,228,255}));DRAW(text(r,font,s,x+30,at+6,"Sanity",(SDL_Color){212,201,178,255},14));if(sv_hud_has(v,SV_HUD_SANITY)){
        if(p->sanity_bar_segments>=0){float mw=SDL_min(112,width-90),mx=left-x-mw,my=at+7;DRAW(rim(r,s,mx-1,my-1,mw+2,15,(SDL_Color){28,32,27,255}));DRAW(fill(r,s,mx,my,mw,13,(SDL_Color){2,3,2,255}));DRAW(fill(r,s,mx+1,my+1,(mw-2)*p->sanity_bar_segments/9.f,11,p->rows[3].colour));DRAW(rim(r,s,mx,my,mw,13,(SDL_Color){102,90,59,255}));}
        else DRAW(right_source_text(r,font,s,left-x,at+6,width-90,v->sanity.label,p->rows[3].colour,14));
    }else DRAW(right_text(r,font,s,left-x,at+6,"--",muted,14));}
    float armour_y=at+(wide_sanity?9:24);DRAW(icon(r,s,x,armour_y,18,SV_HUD_ICON_ARMOUR,(SDL_Color){184,194,189,255}));DRAW(text(r,font,s,x+30,armour_y+2,"Armor",(SDL_Color){212,201,178,255},14));snprintf(val,sizeof(val),"%d %+d%s",v->armour.base,v->armour.bonus,v->armour.unknown?" ?":"");if(!v->armour.bonus)snprintf(val,sizeof(val),"%d%s",v->armour.base,v->armour.unknown?" ?":"");DRAW(right_text(r,font,s,left-x,armour_y+2,sv_hud_has(v,SV_HUD_ARMOUR)?val:"--",p->rows[12].colour,14));at+=sanity_height+(wide_sanity?7:0);
    const SvHudResource *resources[]={&v->hp,&v->mana,&v->stamina};const char *names[]={"Health","Mana","Stamina"};
    int ordinary=3-(expanded&&prefs.huge[0])-(expanded&&prefs.huge[1]);float stamina_extra=prefs.huge[2]?12:0,resource_pad=expanded?4:9,resource_height=ordinary*38-4+resource_pad*2-(expanded?0:9)+stamina_extra;DRAW(block_card(r,s,pad,at,left-2*pad,resource_height));int visible=0;
    for(int i=0;i<3;++i){if(expanded&&i<2&&prefs.huge[i])continue;float yy=at+resource_pad+visible++*38;bool available=sv_hud_has(v,(SvHudField)i),bar=prefs.bar[i]||resources[i]->bar||prefs.huge[i];float bar_height=34+(i==2?stamina_extra:0);DRAW(enamel_kind(r,s,x,yy,width,bar_height,available&&bar?resources[i]->current:0,resources[i]->maximum,i+1,prefs.solid));DRAW(overlay_text(r,font,font_revision,s,x+9,yy+(bar_height-16)/2,names[i],(SDL_Color){238,231,214,255},14,4));snprintf(val,sizeof(val),"%d / %d%s",resources[i]->current,resources[i]->maximum,resources[i]->boosted?" +":"");DRAW(right_overlay_text(r,font,font_revision,s,left-x-7,yy+(bar_height-14)/2,available?val:"--",(SDL_Color){238,231,214,255},12,4));}at+=resource_height;
    DRAW(block_card(r,s,pad,at,left-2*pad,39));bool tracking=sv_hud_has(v,SV_HUD_MONSTER)&&v->monster.attribute;if(tracking)DRAW(enamel_kind(r,s,pad+7,at+8,left-2*pad-14,23,v->monster.health,10,4,true));else DRAW(inactive_enemy(r,s,pad+7,at+8,left-2*pad-14,23));at+=39;
    DRAW(block_card(r,s,pad,at,left-2*pad,34));DRAW(icon(r,s,x,at+7,20,SV_HUD_ICON_STATUS_SPEED,term_colour(sv_hud_has(v,SV_HUD_DEPTH)&&v->depth.no_tele?TERM_L_DARK:v->speed_boosted?TERM_L_BLUE:v->speed>0?TERM_L_GREEN:v->speed<0?TERM_L_UMBER:TERM_WHITE)));DRAW(text(r,font,s,x+30,at+10,"Speed",(SDL_Color){201,190,168,255},13));snprintf(val,sizeof(val),"%s %+d%s%s",v->speed>0?"Fast":v->speed<0?"Slow":"Normal",v->speed,v->speed_boosted?" +":"",sv_hud_has(v,SV_HUD_DEPTH)&&v->depth.no_tele?" No-Tele":"");DRAW(right_text(r,font,s,left-x,at+10,sv_hud_has(v,SV_HUD_SPEED)?val:"--",term_colour(sv_hud_has(v,SV_HUD_DEPTH)&&v->depth.no_tele?TERM_L_DARK:v->speed_boosted?TERM_L_BLUE:v->speed>0?TERM_L_GREEN:v->speed<0?TERM_L_UMBER:TERM_WHITE),12));at+=34;
    DRAW(block_card(r,s,pad,at,left-2*pad,34));DRAW(icon(r,s,x,at+7,20,SV_HUD_ICON_STATUS_BPR,term_colour(v->blows.attribute)));DRAW(text(r,font,s,x+30,at+10,"Blows / Round",(SDL_Color){201,190,168,255},13));source_text(v->blows.label,val);if(!*val){if(v->blows.value==255)snprintf(val,sizeof(val),"Wraith");
#ifdef IND_WRAITH_PROB
        else if(v->blows.value==254)snprintf(val,sizeof(val),"PbTrav");
#endif
        else snprintf(val,sizeof(val),"%u BpR",v->blows.value);}if(sv_hud_has(v,SV_HUD_BLOWS)&&v->blows.label[0])DRAW(right_source_text(r,font,s,left-x,at+10,width-135,v->blows.label,term_colour(v->blows.attribute),12));else DRAW(right_text(r,font,s,left-x,at+10,sv_hud_has(v,SV_HUD_BLOWS)?val:"--",term_colour(v->blows.attribute),12));at+=34;
    float tile_height=compact?22:28,tile_gap=compact?2:3,effects_pad=compact?6:8,effects_height=(compact?20:25)+3*tile_height+2*tile_gap+(compact?6:9);DRAW(block_card(r,s,pad,at,left-2*pad,effects_height));DRAW(text(r,font,s,pad+effects_pad,at+(compact?5:7),"STATUS EFFECTS",(SDL_Color){213,183,93,255},compact?10:11));
    DRAW(sv_font_measure(font,"STATUS EFFECTS",compact?10:11,s,&tw,&th,false));
    float rule_x=pad+effects_pad+tw/s+8,rule_w=left-pad-effects_pad-rule_x;
    SDL_BlendMode rule_blend;SDL_GetRenderDrawBlendMode(r,&rule_blend);DRAW(SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND));
    for(int i=0;i<(int)rule_w;++i)DRAW(fill(r,s,rule_x+i,at+(compact?10:13),1,1,(SDL_Color){118,95,52,(Uint8)(255*(1-i/rule_w))}));DRAW(SDL_SetRenderDrawBlendMode(r,rule_blend));at+=compact?20:25;
    const unsigned indicator_bits[]={0,1,2,3,4,5,6,7,8,9,10,19,18,20};
    const SDL_Color indicator_colours[]={{229,75,55,255},{231,237,240,255},{77,131,237,255},{154,161,156,255},{66,200,117,255},{185,109,225,255},{228,224,213,255},{231,197,75,255},{85,217,112,255},{150,159,155,255},{98,187,223,255},{81,135,232,255},{114,216,97,255},{223,135,47,255}};
    const SvHudIcon indicator_icons[]={SV_HUD_ICON_STATUS_FIRE,SV_HUD_ICON_STATUS_COLD,SV_HUD_ICON_STATUS_ELEC,SV_HUD_ICON_STATUS_ACID,SV_HUD_ICON_STATUS_POISON,SV_HUD_ICON_STATUS_MANA,SV_HUD_ICON_STATUS_ESP,SV_HUD_ICON_STATUS_BRAND,SV_HUD_ICON_STATUS_REGEN,SV_HUD_ICON_STATUS_DISPERSION,SV_HUD_ICON_STATUS_CHARM,SV_HUD_ICON_STATUS_THUNDER,SV_HUD_ICON_STATUS_PFE,SV_HUD_ICON_STATUS_CRIT,SV_HUD_ICON_STATUS_MUSHROOM,SV_HUD_ICON_STATUS_SHIELD,SV_HUD_ICON_STATUS_FOOD,SV_HUD_ICON_STATUS_ESP,SV_HUD_ICON_STATUS_CONFUSED,SV_HUD_ICON_STATUS_FEAR,SV_HUD_ICON_STATUS_POISON,SV_HUD_ICON_STATUS_REST,SV_HUD_ICON_STATUS_WOUND,SV_HUD_ICON_STATUS_STUN};
    static const char *tile_names[]={"Resist Fire","Resist Cold","Resist Electricity","Resist Acid","Resist Poison","Divine / Mana Resistance","Temporary ESP","Melee Brand","Regeneration","Dispersion","Charm","Thunderstorm","Protection from Evil","Critical Force","Mycorrhiza: unavailable","Reactive Shield","Food","Vision","Confusion","Fear","Poison / Disease","Activity","Wounds","Stun"};
    float tile_x=pad+effects_pad,tile_width=(left-2*tile_x-7*tile_gap)/8;
    float cycle=(SDL_GetTicks()%2500)/1250.f;if(cycle>1)cycle=2-cycle;int frame_phase=(int)(cycle*15);
    for(int i=0;i<24;++i){bool active=false,critical=false;SDL_Color colour=muted;SvHudIcon glyph=indicator_icons[i];int severity=0;
        if(i<14){active=sv_hud_has(v,SV_HUD_INDICATORS)&&(v->indicators&(UINT32_C(1)<<indicator_bits[i]));colour=indicator_colours[i];}
        else if(i==14){active=false;colour=(SDL_Color){105,189,102,255};}
        else if(i==15){active=sv_hud_has(v,SV_HUD_INDICATORS)&&(v->indicators&UINT32_C(0x3f800));colour=(SDL_Color){139,141,128,255};static const SDL_Color shields[]={{231,237,240,255},{95,145,237,255},{167,119,69,255},{228,75,55,255},{231,237,240,255},{238,114,95,255},{177,110,224,255}};for(int j=0;j<7;++j)if(v->indicators&(UINT32_C(1)<<(11+j))){colour=shields[j];break;}}
        else {const SvHudCondition *condition=&p->conditions[i-16];active=condition->active;colour=condition->colour;glyph=condition->icon;severity=condition->severity;critical=condition->critical;}
        float xx=tile_x+(i%8)*(tile_width+tile_gap),yy=at+(i/8)*(tile_height+tile_gap),icon_size=compact?15:20;
        int shield=-1;if(i==15&&active){shield=0;while(shield<6&&!(v->indicators&(UINT32_C(1)<<(11+shield))))++shield;}
        DRAW(condition_tile(r,s,xx,yy,tile_width,tile_height,colour,active,severity,critical||severity>=3,frame_phase,glyph,icon_size,shield));
        if(p->pointer_known&&p->pointer_x>=xx*s&&p->pointer_x<(xx+tile_width)*s&&p->pointer_y>=yy*s&&p->pointer_y<(yy+tile_height)*s){bool known=i<14||i==15?sv_hud_has(v,SV_HUD_INDICATORS):i==14?false:p->conditions[i-16].available;snprintf(tooltip,sizeof(tooltip),i==14?"%s":"%s: %s",tile_names[i],!known?"awaiting server":active?"active":"inactive");tooltip_y=yy+tile_height+2;}}
    at+=3*tile_height+2*tile_gap+effects_pad;
    if(expanded){int count=(prefs.huge[0]?1:0)+(prefs.huge[1]?1:0)+(prefs.huge[3]?1:0);float yy=at+8,bar_height=SDL_max(compact?96:136,lh-yy-60),cw=(width-8*(count-1))/count;int column=0;
        const int map[]={1,3,0};
        for(int i=0;i<3;++i){int k=map[i];bool enabled=prefs.huge[k];if(!enabled)continue;int current=k==3?v->sanity.current:resources[k]->current,maximum=k==3?v->sanity.maximum:resources[k]->maximum;float xx=x+column++*(cw+8);
            DRAW(wide_ground(r,s,xx,yy,cw,bar_height,k));DRAW(rim(r,s,xx-2,yy-2,cw+4,bar_height+4,(SDL_Color){63,68,61,255}));DRAW(rim(r,s,xx-1,yy-1,cw+2,bar_height+2,(SDL_Color){23,26,22,255}));DRAW(rim(r,s,xx,yy,cw,bar_height,(SDL_Color){113,97,62,255}));
            bool available=k==3?sv_hud_has(v,SV_HUD_SANITY)&&v->sanity.numeric_available:sv_hud_has(v,(SvHudField)k);
            if(available){float fraction=maximum>0?(float)current/maximum:0;fraction=SDL_clamp(fraction,0,1);SDL_Color top_colour=k==0?(SDL_Color){255,73,61,255}:k==1?(SDL_Color){57,140,255,255}:(SDL_Color){125,240,68,255};SDL_Color middle=k==0?(SDL_Color){237,17,13,255}:k==1?(SDL_Color){7,89,239,255}:(SDL_Color){55,197,35,255};SDL_Color bottom=k==0?(SDL_Color){169,7,5,255}:k==1?(SDL_Color){2,45,183,255}:(SDL_Color){19,141,22,255};HudStop stops[]={{0,top_colour},{.54,middle},{1,bottom}};int rows=(int)SDL_ceilf((bar_height-2)*fraction*s);
                for(int band=0;band<rows;++band){float t=rows>1?(float)band/(rows-1):0;SDL_Color tint=gradient(stops,3,t);SDL_FRect stripe={(xx+1)*s,(yy+bar_height-1)*s-rows+band,(cw-2)*s,1};DRAW(SDL_SetRenderDrawColor(r,tint.r,tint.g,tint.b,255)&&SDL_RenderFillRect(r,&stripe));}
                DRAW(wide_reflection(r,s,xx+1,yy+bar_height-1-(bar_height-2)*fraction,cw-2,(bar_height-2)*fraction));
                if(!prefs.solid&&maximum>100)for(int tick=100;tick<maximum;tick+=100){float tick_y=yy+bar_height-(float)tick/maximum*bar_height;DRAW(line(r,s,xx+7,tick_y,xx+cw-7,tick_y,(SDL_Color){48,55,40,255}));}}
            float badge_width=SDL_min(74,cw-8),bx=xx+(cw-badge_width)/2,by=yy+(bar_height-37)/2;
            DRAW(wide_badge(r,s,bx,by,badge_width,37));integer(val,sizeof(val),current);DRAW(sv_font_measure(font,available?val:"--",13,s,&tw,&th,false));DRAW(text(r,font,s,xx+(cw-tw/s)/2,by+5,available?val:"--",(SDL_Color){255,253,243,255},13));integer(other,sizeof(other),maximum);snprintf(val,sizeof(val),"/ %s",other);DRAW(sv_font_measure(font,available?val:"",8,s,&tw,&th,false));DRAW(text(r,font,s,xx+(cw-tw/s)/2,by+24,available?val:"",(SDL_Color){170,169,159,255},8));
            if(prefs.graphics&&enabled)DRAW(rim(r,s,xx-2,yy-2,cw+4,bar_height+4,gold));}
    }
    if(prefs.huge[4]&&sv_hud_has(v,SV_HUD_STUN)&&v->stun)DRAW(rim(r,s,5,5,left-10,lh-10,v->stun>100?red:gold));
    float shortcut_width=(left-22)/3,shortcut_x=7+shortcut_width+4,shortcut_y=lh-49;
    DRAW(card(r,s,1,lh-101,left-2,100));p->shortcut=(SDL_FRect){shortcut_x*s,shortcut_y*s,shortcut_width*s,42*s};
    DRAW(fill(r,s,shortcut_x,shortcut_y,shortcut_width,42,(SDL_Color){11,14,12,255}));DRAW(rim(r,s,shortcut_x,shortcut_y,shortcut_width,42,(SDL_Color){103,85,47,255}));DRAW(icon(r,s,shortcut_x+(shortcut_width-20)/2,shortcut_y+9,20,SV_HUD_ICON_CHARACTER,(SDL_Color){185,183,170,255}));DRAW(text(r,font,s,shortcut_x+shortcut_width-8,shortcut_y+31,"C",(SDL_Color){212,183,88,255},8));
    if(tooltip[0]&&!p->character_open){DRAW(sv_font_measure(font,tooltip,11,s,&tw,&th,false));float ty=tooltip_y;if(ty+30>lh)ty=lh-30;DRAW(forged(r,s,left+6,ty,tw/s+20,28));DRAW(text(r,font,s,left+16,ty+7,tooltip,ink,11));}
    DRAW(rim(r,s,left+3,3,lw-left-7,lh-6,(SDL_Color){74,62,37,255}));
    if(!prefs.small){float rw=p->right_width*display_scale/s,rx=lw-rw,wh=SDL_max(72,SDL_roundf((lh-12)*.30f)),wy=lh-6-wh;
        DRAW(forged(r,s,rx,0,rw,lh));DRAW(card(r,s,rx+6,wy,rw-12,wh));DRAW(rim(r,s,rx+6,wy,rw-12,wh,(SDL_Color){94,80,51,255}));
        DRAW(character_head(p,r,font,s,rx+7,wy+1,rw-14,false));DRAW(character_content(p,r,font,view,rx+7,wy+31,rw-14,wh-32,false,&p->widget_scroll));
        p->widget=(SDL_FRect){(rx+6)*s,wy*s,(rw-12)*s,wh*s};
    }else p->widget=(SDL_FRect){0};
    left=left*s/primary_scale;s=primary_scale;p->scale=s;lw=w/s;lh=h/s;
    if(p->character_open&&!view.request.pending&&!view.paused){float available=lw-left-28,modal_width=SDL_min(918,lw*.54f);if(modal_width>available)modal_width=available;float modal_height=p->character_page==SV_HUD_CHARACTER_RESISTS?SDL_min(387.359375f,lh-28):SDL_clamp(lh*.6578f,300,lh-28),px=SDL_max(left+14,(lw-modal_width)/2),py=(lh-modal_height)/2;p->primary=(SDL_FRect){px,py,modal_width,modal_height};
        DRAW(character_backdrop(p,r,font,view));
        DRAW(forged(r,s,px,py,modal_width,modal_height));DRAW(rim(r,s,px,py,modal_width,modal_height,(SDL_Color){154,123,61,255}));DRAW(rim(r,s,px+2,py+2,modal_width-4,modal_height-4,(SDL_Color){9,12,9,255}));DRAW(rim(r,s,px+3,py+3,modal_width-6,modal_height-6,(SDL_Color){99,105,95,255}));
        DRAW(character_panel(r,s,px+1,py+39,modal_width-2,modal_height-40));DRAW(character_head(p,r,font,s,px+1,py+1,modal_width-2,true));DRAW(character_content(p,r,font,view,px+1,py+39,modal_width-2,modal_height-40,true,&p->scroll));if(p->dump_notice){float nw=SDL_min(440,modal_width),ny=SDL_max(0,py-31);DRAW(forged(r,s,px+modal_width-nw,ny,nw,28));DRAW(text(r,font,s,px+modal_width-nw+9,ny+8,"DUMP unavailable: character export is not implemented",muted,10));}}
    if(view.request.pending){char prompt[SV_STATUS_TEXT],bounded[SV_REQUEST_BYTES+1];memcpy(bounded,view.request.prompt,SV_REQUEST_BYTES);bounded[SV_REQUEST_BYTES]=0;source_text(bounded,prompt);DRAW(forged(r,s,left+24,70,lw-left-36,92));DRAW(text(r,font,s,left+38,82,"SERVER REQUEST",gold,14));DRAW(text(r,font,s,left+38,110,prompt,ink,12));}
#undef DRAW
    return true;
}
bool sv_hud_event(SvHudPresentation *p,SvAppView view,const SDL_Event *event)
{
    if(!p||!p->valid||p->generation!=view.generation||!view.active)return false;
    if(view.context!=SV_CONTEXT_GAME||view.request.pending||view.paused||view.quantity_active)return false;
    if(event->type==SDL_EVENT_MOUSE_MOTION){SDL_Window *window=SDL_GetWindowFromID(event->motion.windowID);float density=window?SDL_GetWindowPixelDensity(window):1;p->pointer_known=true;p->pointer_x=event->motion.x*density;p->pointer_y=event->motion.y*density;return p->character_open;}
    if(event->type==SDL_EVENT_KEY_DOWN&&!event->key.repeat){
        if(event->key.key==SDLK_C&&(event->key.mod&SDL_KMOD_SHIFT)&&!(event->key.mod&(SDL_KMOD_CTRL|SDL_KMOD_ALT|SDL_KMOD_GUI))){p->character_open=true;return true;}
        if(p->character_open&&event->key.key==SDLK_ESCAPE){p->character_open=false;p->dump_notice=false;return true;}
        if(p->character_open&&(event->key.key==SDLK_PAGEDOWN||event->key.key==SDLK_PAGEUP||event->key.key==SDLK_DOWN||event->key.key==SDLK_UP)){
            int delta=event->key.key==SDLK_PAGEDOWN?320:event->key.key==SDLK_PAGEUP?-320:event->key.key==SDLK_DOWN?24:-24;p->scroll=SDL_clamp(p->scroll+delta,0,p->scroll_max);return true;}
    }
    if(event->type==SDL_EVENT_MOUSE_WHEEL&&p->character_open){p->scroll=SDL_clamp(p->scroll-(int)(event->wheel.y*48),0,p->scroll_max);return true;}
    if(event->type==SDL_EVENT_MOUSE_WHEEL&&p->pointer_known&&p->pointer_x>=p->widget.x&&p->pointer_x<p->widget.x+p->widget.w&&p->pointer_y>=p->widget.y&&p->pointer_y<p->widget.y+p->widget.h){p->widget_scroll=SDL_clamp(p->widget_scroll-(int)(event->wheel.y*36),0,p->widget_scroll_max);return true;}
    if(event->type==SDL_EVENT_MOUSE_BUTTON_DOWN&&event->button.button==SDL_BUTTON_LEFT){SDL_Window *window=SDL_GetWindowFromID(event->button.windowID);float density=window?SDL_GetWindowPixelDensity(window):1;float x=event->button.x*density,y=event->button.y*density;SDL_FRect b=p->shortcut;
        if(p->character_open){float px=x/p->scale,py=y/p->scale,rx=p->primary.x+1,ry=p->primary.y+1,rw=p->primary.w-2;
            if(px>=rx&&px<rx+rw&&py>=ry&&py<ry+38){
                if(px>=rx+rw-38){p->character_open=false;p->dump_notice=false;return true;}
                if(px>=rx+rw-96){p->dump_notice=true;return true;}
                float tw=character_tab_width(rw),tx=rx+rw-96-3*tw;
                if(px>=tx){int page=(int)((px-tx)/tw);if(page<SV_HUD_CHARACTER_COUNT)character_select(p,(SvHudCharacterPage)page);return true;}
            }

        }else{
            if(x>=b.x&&x<b.x+b.w&&y>=b.y&&y<b.y+b.h){p->character_open=true;return true;}
            b=p->widget;
            if(x>=b.x&&x<b.x+b.w&&y>=b.y&&y<b.y+30*p->left_scale){float rx=b.x+b.w-152*p->left_scale;
                if(x>=rx){character_select(p,x<rx+98*p->left_scale?p->summary_page:SV_HUD_CHARACTER_RESISTS);return true;}
                p->character_open=true;return true;
            }
        }
    }
    /* A primary owns ordinary input; no UI click becomes a game command. */
    return p->character_open&&(event->type==SDL_EVENT_KEY_DOWN||event->type==SDL_EVENT_TEXT_INPUT||event->type==SDL_EVENT_MOUSE_BUTTON_DOWN||event->type==SDL_EVENT_MOUSE_BUTTON_UP);
}

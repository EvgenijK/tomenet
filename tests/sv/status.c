#include "app.h"
#include "ui/status.h"
#include "ui/hud.h"
#include "status-fixture.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { unsigned char bytes[600]; size_t size; } Packet;
static void byte(Packet *p,unsigned value) { assert(p->size<sizeof(p->bytes));p->bytes[p->size++]=(unsigned char)value; }
static void word(Packet *p,int value) { byte(p,(unsigned)value>>8);byte(p,(unsigned)value); }
static void number(Packet *p,int32_t value) { word(p,(int)((uint32_t)value>>16));word(p,value); }
static void text(Packet *p,const char *value) { do { byte(p,(unsigned char)*value); } while(*value++); }
static SvApp *open_version(const int version[6])
{ SvApp *a=sv_app_create((SvAlertSink){0});assert(a);assert(sv_app_open(a,version)==SV_OK);return a; }
static void feed(SvApp *a,const void *bytes,size_t size,size_t count)
{
    assert(sv_app_receive(a,sv_app_view(a).generation,bytes,size)==SV_OK);
    SvStep step=sv_app_step(a,64);
    if(step.result!=SV_OK && step.result!=SV_WAITING)
        fprintf(stderr,"status fixture failed packet=%u result=%s count=%zu\n",*(const unsigned char*)bytes,sv_result_text(step.result),step.processed);
    assert(step.result==SV_OK || step.result==SV_WAITING);assert(step.processed==count);
}
static void packet(SvApp *a,Packet p) { feed(a,p.bytes,p.size,1); }
static void finish(SvApp *a) { assert(sv_app_destroy(a)==SV_OK); }
static void latest_all_fields(void)
{
    SvApp *a=open_version(sv_status_fixture_version);assert(sv_status_fixture_feed(a)==SV_OK);
    SvAppView v=sv_app_view(a);SvHud h=v.hud;
    assert(h.generation==v.generation && h.revision==27);
    assert(h.available==((UINT64_C(1)<<27)-1));
    assert(h.hp.current==75 && h.hp.maximum==100 && h.hp.boosted && h.hp.bar && h.hp.drain==1);
    assert(h.mana.current==30 && h.mana.maximum==50 && h.mana.bar);
    assert(h.stamina.current==60 && h.stamina.maximum==80 && h.stamina.bar);
    assert(h.sanity.current==40 && h.sanity.maximum==60 && h.sanity.damage==1);
    assert(h.sanity.attribute==13 && !strcmp(h.sanity.label,"Sound") && h.sanity.numeric_available && h.sanity.damage_available);
    assert(h.stats[0].current==108 && h.stats[0].maximum==118 && h.stats[0].index==20);
    assert(h.stats[0].base==110 && h.stats[0].temporary==3 && h.stats[0].boosted && h.stats[0].available);
    assert(!h.stats[1].available);
    assert(h.armour.base==123 && h.armour.bonus==7 && h.armour.unknown && h.armour.boosted);
    assert(h.demographics.height==180 && h.demographics.weight==75 && h.demographics.age==42 && h.demographics.social_class==9 && !strcmp(h.demographics.body,"Player"));
    assert(h.combat.hit==1 && h.combat.damage==2 && h.combat.ranged_hit==3 && h.combat.ranged_damage==4 && h.combat.melee_hit==5 && h.combat.melee_damage==6);
    assert(h.experience.level==20 && h.experience.maximum_level==25 && h.experience.maximum_player_level==23 && h.experience.fractional);
    assert(h.experience.maximum==123456 && h.experience.current==120000 && h.experience.advance==130000 && h.experience.previous==110000);
    assert(h.currency.carried==1234 && h.currency.bank==5678);
    assert(h.confusion==1 && h.poison==3 && h.study==4 && h.blows.value==3 && h.blows.attribute==13 && !strcmp(h.blows.label,"3 BpR"));
    assert(h.food==5000 && h.fear==1 && h.speed==10 && h.speed_boosted && h.cut==75 && h.blind==3 && h.stun==52);
    assert(h.monster.health==6 && h.monster.attribute==12 && h.afk==1);
    for(unsigned i=0;i<15;i++)assert(h.encumbrance[i]==i+1);
    assert(!memcmp(h.extra,(unsigned char[]){255,'r','X',128,0},5));
    assert(h.martyr==1 && h.idle==1 && h.indicators==UINT32_C(0x80000011));
    /* The returned view owns copies, not borrowed pointers into future state. */
    Packet gold={{23},1};number(&gold,99);number(&gold,100);packet(a,gold);
    assert(h.currency.carried==1234 && sv_app_view(a).hud.currency.carried==99);
    finish(a);
}
/* Every C012 receiver waits without publication on a split field and consumes
 * exactly its own frame when adjacent to an independently specified sentinel. */
static void split_and_adjacent(void)
{
    static const unsigned char sentinel[]={23,0,0,0,9,0,0,0,8};
    for(size_t i=0;i<sizeof(sv_status_fixtures)/sizeof(sv_status_fixtures[0]);i++) {
        const SvStatusFixture *f=&sv_status_fixtures[i];SvApp *a=open_version(sv_status_fixture_version);
        size_t split=f->size-1;feed(a,f->bytes,split,0);
        assert(!sv_app_view(a).hud.available && !sv_app_view(a).hud.revision);
        unsigned char tail[32];assert(f->size-split+sizeof(sentinel)<=sizeof(tail));
        memcpy(tail,f->bytes+split,f->size-split);memcpy(tail+f->size-split,sentinel,sizeof(sentinel));
        feed(a,tail,f->size-split+sizeof(sentinel),2);
        SvHud h=sv_app_view(a).hud;assert(h.revision==2 && h.currency.carried==9 && h.currency.bank==8);
        assert(sv_app_step(a,8).processed==0);finish(a);
    }
}
static SvHud checked_branch(const int version[6],Packet p)
{
    SvApp *a=open_version(version);size_t split=p.size/2;
    feed(a,p.bytes,split,0);assert(!sv_app_view(a).hud.available);
    static const unsigned char sentinel[]={23,0,0,0,19,0,0,0,18};
    memcpy(p.bytes+p.size,sentinel,sizeof(sentinel));
    feed(a,p.bytes+split,p.size-split+sizeof(sentinel),2);
    SvHud h=sv_app_view(a).hud;assert(h.currency.carried==19 && h.currency.bank==18);
    finish(a);return h;
}
static void version_defaults(void)
{
    const int old_stats[]={4,7,4,5,9,9},new_stats[]={4,7,4,6,0,0};
    for(int modern=0;modern<2;modern++) {
        Packet p={{28},1};byte(&p,0x11);word(&p,90);word(&p,80);word(&p,9);word(&p,85);if(modern)word(&p,0);
        SvHud h=checked_branch(modern?new_stats:old_stats,p);
        assert(h.stats[1].available && h.stats[1].temporary==0 && h.stats[1].boosted && h.stats[1].base==85);
    }
    const int armour_versions[][6]={{4,7,2,9,9,9},{4,7,3,0,0,0},{4,7,3,1,0,0}};
    const int armour_values[]={123,0x1000+123,30123};
    for(int i=0;i<3;i++) {
        Packet p={{21},1};word(&p,armour_values[i]);word(&p,-7);
        SvHud h=checked_branch(armour_versions[i],p);
        assert(h.armour.base==123 && h.armour.bonus==-7 && h.armour.boosted==(i>0) && h.armour.unknown==(i==2));
    }
    const int exp_versions[][6]={{4,5,6,0,0,1},{4,5,6,0,0,2}};
    for(int i=0;i<2;i++) {
        Packet p={{22},1};word(&p,20);word(&p,25);word(&p,23);number(&p,1000);number(&p,900);number(&p,1100);if(i)number(&p,800);
        SvHud h=checked_branch(exp_versions[i],p);assert(h.experience.previous==(i?800:1100) && !h.experience.fractional);
    }
    const int sanity_versions[][6]={{4,6,1,2,0,0},{4,6,1,2,0,1},{4,8,1,3,0,0}};
    for(int i=0;i<3;i++) {
        Packet p={{130},1};byte(&p,12);text(&p,"\377rMind");if(i)byte(&p,1);if(i==2){word(&p,17);word(&p,50);}
        SvHud h=checked_branch(sanity_versions[i],p);assert(h.sanity.attribute==12 && !strcmp(h.sanity.label,"\377rMind"));
        assert(h.sanity.damage==(i?1:0) && h.sanity.current==(i==2?17:0) && h.sanity.maximum==(i==2?50:0));
        assert(h.sanity.damage_available==(i>0) && h.sanity.numeric_available==(i==2));
    }
    const int enc_versions[][6]={{4,4,2,0,0,0},{4,4,2,0,0,1},{4,8,1,0,0,0}};
    for(int i=0;i<3;i++) {
        Packet p={{164},1};int count=i==0?12:i==1?13:15;
        for(int k=1;k<=count;k++)byte(&p,k);
        SvHud h=checked_branch(enc_versions[i],p);
        for(int k=0;k<11;k++)assert(h.encumbrance[k]==k+1);
        assert(h.encumbrance[11]==(i?12:0));assert(h.encumbrance[12]==(i?13:12));
        assert(h.encumbrance[13]==(i==2?14:0) && h.encumbrance[14]==(i==2?15:0));
    }
    const int blows_versions[][6]={{4,9,1,0,0,0},{4,9,1,0,0,1}};
    for(int i=0;i<2;i++) {
        Packet p={{129,4,13},3};if(i)text(&p,"Wraith");SvHud h=checked_branch(blows_versions[i],p);
        assert(h.blows.value==4 && h.blows.attribute==13 && !strcmp(h.blows.label,i?"Wraith":""));
    }
    const int hp_versions[][6]={{4,7,0,2,0,1},{4,7,0,2,0,2}};
    for(int i=0;i<2;i++) {
        Packet p={{24},1};word(&p,10100);word(&p,9990);if(i)byte(&p,255);
        SvHud h=checked_branch(hp_versions[i],p);assert(h.hp.current==-10 && h.hp.maximum==100 && h.hp.bar && h.hp.boosted && h.hp.drain==(i?255:0));
    }
}
static Packet depth_packet(int branch,bool no_tele)
{
    Packet p={{34},1};word(&p,1);word(&p,2);word(&p,-3);byte(&p,0);
    if(branch==0)word(&p,1);else {byte(&p,5);byte(&p,no_tele?8:13);}
    text(&p,"Dungeon");if(branch>=2)text(&p,"Angband");if(branch>=3)text(&p,"beneath");return p;
}
static void depth_speed_and_semantics(void)
{
    const int versions[][6]={{4,4,1,5,0,0},{4,4,1,6,0,1},{4,5,9,0,0,1},{4,6,1,2,0,1}};
    for(int i=0;i<4;i++) {
        SvHud h=checked_branch(versions[i],depth_packet(i,true));
        assert(h.depth.x==1 && h.depth.y==2 && h.depth.z==-3 && !h.depth.town);
        assert(h.depth.sector_colour==(i?8:13) && h.depth.no_tele==(i!=0));
        assert(!strcmp(h.depth.preposition,i==3?"beneath":"in"));
    }
    SvApp *a=open_version(sv_status_fixture_version);
    Packet speed={{42},1};word(&speed,266);packet(a,speed);packet(a,depth_packet(3,true));
    SvHud h=sv_app_view(a).hud;assert(h.speed==10 && h.speed_boosted && h.depth.no_tele);
    packet(a,depth_packet(3,false));assert(!sv_app_view(a).hud.depth.no_tele);
    Packet negative={{42},1};word(&negative,-10);packet(a,negative);assert(sv_app_view(a).hud.speed==-10 && !sv_app_view(a).hud.speed_boosted);
    Packet simple={{42},1};word(&simple,0);packet(a,depth_packet(3,true));packet(a,simple);
    assert(sv_app_view(a).hud.speed==0 && sv_app_view(a).hud.depth.no_tele);
    /* Encoded states and explicit clearing remain distinct from boolean UI. */
    feed(a,(unsigned char[]){39,2,36,2,45,0,101},7,3);
    h=sv_app_view(a).hud;assert(h.poison==2 && h.blind==2 && h.stun==101);
    feed(a,(unsigned char[]){39,0,36,0,45,0,0,163,0,136,0,138,0},13,6);
    h=sv_app_view(a).hud;assert(!h.poison && !h.blind && !h.stun && !h.afk && !h.martyr && !h.idle);
    finish(a);
}
static const char *row_value(const SvHudPresentation *p,const char *label)
{
    for(size_t i=0;i<p->row_count;i++)if(!strcmp(p->rows[i].label,label))return p->rows[i].value;
    assert(!"Missing production HUD row");return "";
}
static bool row_contains(const SvHudPresentation *p,const char *value)
{
    for(size_t i=0;i<p->row_count;i++)if(strstr(p->rows[i].value,value))return true;
    return false;
}
static void stat_colour_boundaries(void)
{
    SvApp *a=open_version(sv_status_fixture_version);SvHudPresentation p={0};
    const struct { int current,base,temporary;unsigned char r,g,b; } cases[]={
        {118,117,0,135,223,131}, /* Natural cap below 18/100: light green. */
        {118,118,0,207,174,120}, /* Natural cap at 18/100: light umber. */
        {118,118,1,121,186,239}, /* Boosted, without drain: light blue. */
        {117,118,1,235,209,92}   /* Drain takes priority over boost: yellow. */
    };
    for(size_t i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        Packet stat={{28,0},2};word(&stat,118);word(&stat,cases[i].current);
        word(&stat,20);word(&stat,cases[i].base);word(&stat,cases[i].temporary);packet(a,stat);
        assert(sv_hud_prepare(&p,sv_app_view(a),(SvHudPreferences){.ui_scale_percent=100},1932,1208,1,1));
        const SvHudRow *row=NULL;
        for(size_t j=0;j<p.row_count;j++)if(!strcmp(p.rows[j].label,"Strength")){row=&p.rows[j];break;}
        assert(row && row->colour.r==cases[i].r && row->colour.g==cases[i].g && row->colour.b==cases[i].b && row->colour.a==255);
    }
    finish(a);
}
static void status_slot_formats(void)
{
    SvApp *a=open_version(sv_status_fixture_version);SvHudPresentation p={0};
    SvHudPreferences prefs={.ui_scale_percent=100};
    /* The renderer consumes these selected slots for both icons and tooltips.
     * Exercise the conflicting weapon flags through actual packets. */
    const int winners[]={8,3,7};
    for(int i=0;i<3;i++) {
        Packet enc={{164},1};
        for(int flag=0;flag<15;flag++)byte(&enc,flag==7 || (i<2&&flag==3) || (i==0&&flag==8));
        packet(a,enc);assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1932,1208,1,1));
        assert(p.encumbrance_slots[0]==winners[i]);
    }
    /* Server bar segments must not acquire the more precise numeric ratio. */
    const char *sanity_labels[]={"******---","Sound"};
    for(int i=0;i<2;i++) {
        Packet sanity={{130,13},2};text(&sanity,sanity_labels[i]);byte(&sanity,0);word(&sanity,40);word(&sanity,60);
        packet(a,sanity);assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1932,1208,1,1));
        assert(p.sanity_bar_segments==(i?-1:6));
        assert(!strcmp(row_value(&p,"Sanity"),sanity_labels[i]));
    }
    /* A half-XP flag changes the existing fill; the drained level keeps its
     * maximum-player suffix and warning colour in the same existing label. */
    for(int fractional=0;fractional<2;fractional++) {
        Packet xp={{22},1};word(&xp,19+1000*fractional);word(&xp,20);word(&xp,23);
        number(&xp,120);number(&xp,110);number(&xp,120);number(&xp,100);packet(a,xp);
        assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1932,1208,1,1));
        assert(!strcmp(p.level_value,"19 (23)"));
        assert(p.level_colour.r==235 && p.level_colour.g==209 && p.level_colour.b==92);
        assert(p.experience_colour.r==235 && p.experience_colour.g==209 && p.experience_colour.b==92);
        double expected=fractional?.525:.5;
        assert(p.experience_fraction>expected-.0000001 && p.experience_fraction<expected+.0000001);
        assert(p.experience_maximum_fraction==1);
    }
    finish(a);
}
static void baseline_presentation_boundaries(void)
{
    SvApp *a=open_version(sv_status_fixture_version);SvHudPresentation p={0};SvHudPreferences prefs={.small=true,.ui_scale_percent=100};
    const int foods[]={999,1000,1999,2000,2999,3000};
    const char *labels[]={"Starved","Starved","Starved","Hungry","Hungry","Fed"};
    for(unsigned i=0;i<sizeof(foods)/sizeof(foods[0]);i++) {
        Packet food={{35},1};word(&food,foods[i]);packet(a,food);
        assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1024,768,1,1));
        assert(strstr(row_value(&p,"Food"),labels[i]));
    }
    for(int mortal=0;mortal<2;mortal++) {
        Packet cut={{44},1};word(&cut,799+mortal);packet(a,cut);
        assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1024,768,1,1));
        assert(strstr(row_value(&p,"Cut"),mortal?"Mortal wound":"Deep gash"));
    }
    feed(a,(unsigned char[]){36,3,197,0x80,0,0,0x11},7,2);
    assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1024,768,1,1));
    assert(strstr(row_value(&p,"Blind / hallucination"),"Blind + hallucinating"));
    assert(strstr(row_value(&p,"Indicators"),"80000011") && row_contains(&p,"Fire resist") && row_contains(&p,"Poison resist"));
    const SvHudCondition *vision=&p.conditions[SV_HUD_CONDITION_VISION];
    assert(vision->active && vision->icon==SV_HUD_ICON_STATUS_BLIND && vision->severity==3);
    assert(vision->colour.r==0xee && vision->colour.g==0x62 && vision->colour.b==0x46);
    feed(a,(unsigned char[]){138,1,37,1,38,1,39,2},8,4);
    assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1024,768,1,1));
    assert(sv_app_view(a).hud.idle && strstr(row_value(&p,"Idle"),"Idle"));
    assert(!p.conditions[SV_HUD_CONDITION_ACTIVITY].available && !p.conditions[SV_HUD_CONDITION_ACTIVITY].active);
    assert(p.conditions[SV_HUD_CONDITION_ACTIVITY].icon!=SV_HUD_ICON_STATUS_REST);
    const SvHudCondition *confused=&p.conditions[SV_HUD_CONDITION_CONFUSION];
    assert(confused->colour.r==0xdf && confused->colour.g==0x84 && confused->colour.b==0x36 && confused->critical);
    assert(p.conditions[SV_HUD_CONDITION_FEAR].colour.r==0xdf);
    assert(p.conditions[SV_HUD_CONDITION_POISON].colour.r==0xee && p.conditions[SV_HUD_CONDITION_POISON].colour.g==0x69);
    /* Initial defaults come from logical viewport; selected widths survive UI
     * scale, resize, display density and generation changes. */
    assert(p.left_width==262 && p.right_width==394);
    assert(p.left_scale>0 && p.left_scale<=1 && p.left_content_height*p.left_scale<=760.01f);
    SvHudPresentation wide={0};SvHudPreferences wide_prefs={.ui_scale_percent=100};
    assert(sv_hud_prepare(&wide,sv_app_view(a),wide_prefs,1920,1080,1,1));
    assert(wide.left_width==367 && wide.right_width==552);
    assert(sv_hud_prepare(&wide,sv_app_view(a),wide_prefs,2048,1536,2,2));
    assert(wide.left_width==367 && wide.right_width==552);
    wide_prefs.left_width=280;wide_prefs.right_width=420;
    assert(sv_hud_prepare(&wide,sv_app_view(a),wide_prefs,1280,800,1,2));
    assert(wide.left_width==280 && wide.right_width==420);
    /* Read/resize/reprepare doesn't consume retained reactions or change model. */
    uint64_t revision=sv_app_view(a).hud.revision;
    assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1280,800,1.25f,2));
    assert(sv_app_view(a).hud.revision==revision);
    assert(p.left_width==262 && p.right_width==394);
    /* Existing Character entry and its input ownership use production events,
     * including when the right panel is hidden by small layout. */
    SDL_Event event={0};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_C;event.key.mod=SDL_KMOD_SHIFT;
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.character_open);
    /* Repeated Character entry stays open, so its separate SDL text event
     * remains owned by the primary and cannot become a gameplay command. */
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.character_open);
    event=(SDL_Event){0};event.type=SDL_EVENT_TEXT_INPUT;event.text.text="C";
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.character_open);
    /* Renderer reports the actual overflow limit; input must stop there. */
    p.scroll_max=80;
    event=(SDL_Event){0};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_PAGEDOWN;event.key.mod=0;
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.scroll==80);
    event.key.key=SDLK_ESCAPE;
    assert(sv_hud_event(&p,sv_app_view(a),&event) && !p.character_open);
    /* Click the actual wide Character tab positions through production input.
     * Bounds are supplied by the renderer, as for the widget below; geometry
     * and visible content are independently checked in paired runtime frames. */
    SvHudPresentation tabs={0};
    assert(sv_hud_prepare(&tabs,sv_app_view(a),(SvHudPreferences){.ui_scale_percent=100},1932,1208,1,1));
    tabs.primary=(SDL_FRect){507,207,918,795};
    event=(SDL_Event){0};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_C;event.key.mod=SDL_KMOD_SHIFT;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && tabs.character_open);
    assert(tabs.character_page==SV_HUD_CHARACTER_PROFILE);
    event=(SDL_Event){0};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=1172;event.button.y=226;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && tabs.character_page==SV_HUD_CHARACTER_SKILLS);
    event.button.x=1276;
    event.button.y=226;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && tabs.character_page==SV_HUD_CHARACTER_RESISTS);
    tabs.primary=(SDL_FRect){507,410,918,388};
    event.button.x=1357;event.button.y=430;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && tabs.character_page==SV_HUD_CHARACTER_RESISTS);
    assert(tabs.dump_notice);
    event=(SDL_Event){0};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_ESCAPE;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && !tabs.character_open);
    event.key.key=SDLK_C;event.key.mod=SDL_KMOD_SHIFT;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && tabs.character_open);
    assert(tabs.character_page==SV_HUD_CHARACTER_RESISTS);
    event=(SDL_Event){0};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;
    event.button.x=1068;event.button.y=430;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && tabs.character_page==SV_HUD_CHARACTER_PROFILE);
    event=(SDL_Event){0};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_PAGEDOWN;
    assert(sv_hud_event(&tabs,sv_app_view(a),&event) && tabs.scroll==0);
    assert(sv_app_view(a).hud.revision==revision);
    /* Renderer-provided widget bounds route its wheel independently from the
     * primary; opening the primary must also prevent pointer click-through. */
    p.widget=(SDL_FRect){700,450,300,300};
    p.widget_scroll_max=120;
    event=(SDL_Event){0};event.type=SDL_EVENT_MOUSE_MOTION;event.motion.x=720;event.motion.y=500;
    assert(!sv_hud_event(&p,sv_app_view(a),&event));
    int primary_scroll=p.scroll;
    event=(SDL_Event){0};event.type=SDL_EVENT_MOUSE_WHEEL;event.wheel.y=-1;
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.widget_scroll>0 && p.scroll==primary_scroll);
    event=(SDL_Event){0};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;event.button.x=720;event.button.y=460;
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.character_open);
    event.button.button=SDL_BUTTON_RIGHT;
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.character_open);
    event=(SDL_Event){0};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_ESCAPE;
    assert(sv_hud_event(&p,sv_app_view(a),&event) && !p.character_open);
    event.key.key=SDLK_C;event.key.mod=SDL_KMOD_SHIFT;
    uint64_t generation=sv_app_view(a).generation;
    assert(sv_app_begin_quantity(a,generation,1,99,false)==SV_OK);
    assert(!sv_hud_event(&p,sv_app_view(a),&event) && !p.character_open);
    assert(sv_app_quantity_view(a,generation).active);
    assert(sv_app_quantity_edit(a,generation,SV_EDIT_CANCEL)==SV_OK);
    int32_t quantity;assert(sv_app_take_quantity(a,generation,&quantity)==SV_OK);
    feed(a,(unsigned char[]){58},1,1);
    assert(!sv_hud_event(&p,sv_app_view(a),&event) && !p.character_open);
    assert(sv_app_view(a).paused);
    assert(sv_app_ack_pause(a,generation,sv_app_view(a).pause_sequence)==SV_OK);
    assert(sv_hud_event(&p,sv_app_view(a),&event) && p.character_open);
    feed(a,(unsigned char[]){184,0,0,0,7,'?',0},7,1);
    assert(!sv_hud_event(&p,sv_app_view(a),&event) && p.character_open);
    assert(sv_app_view(a).request.pending);
    SvAppView stale_view=sv_app_view(a);
    assert(sv_app_open(a,sv_status_fixture_version)==SV_OK);
    assert(sv_hud_prepare(&p,sv_app_view(a),prefs,1024,768,1,3) && !p.character_open);
    assert(!sv_hud_event(&p,stale_view,&event) && !p.character_open);
    finish(a);
}
static int count_alert(void *context,SvAlert alert)
{ unsigned *counts=context;assert(alert<=SV_LOW_HP_NOTICE);++counts[alert];return 1; }
static void hp_occurrences_and_projection(void)
{
    unsigned counts[3]={0};SvApp *a=sv_app_create((SvAlertSink){counts,count_alert});assert(a);
    assert(sv_app_open(a,sv_status_fixture_version)==SV_OK);
    assert(sv_app_set_alerts(a,(SvAlertOptions){1,1},(SvAttention){1})==SV_OK);
    feed(a,(unsigned char[]){24,0,100,0,10,0},6,1);
    assert(counts[SV_LOW_HP_SOUND]==1 && counts[SV_LOW_HP_NOTICE]==1);
    uint64_t hp_revision=sv_app_view(a).status.revision,hud_revision=sv_app_view(a).hud.revision;
    feed(a,(unsigned char[]){25,0,50,0,30,163,1},7,2);
    assert(sv_app_view(a).status.revision==hp_revision && sv_app_view(a).hud.revision==hud_revision+2);
    SvStatusCache cache={0};SvAppView view=sv_app_view(a);SvPresentationKey key={1024,768,1,1};
    assert(sv_status_prepare(&cache,view,key)&SV_TEXT_CHANGED);
    key.width=1280;key.scale=1.2f;key.font_revision++;
    assert(sv_status_prepare(&cache,view,key)==SV_LAYOUT_CHANGED);
    assert(counts[SV_LOW_HP_SOUND]==1 && counts[SV_LOW_HP_NOTICE]==1);
    feed(a,(unsigned char[]){24,0,100,0,10,0},6,1);
    assert(counts[SV_LOW_HP_SOUND]==2 && counts[SV_LOW_HP_NOTICE]==2);
    /* Exact marker thresholds: equality is not the encoded range. */
    feed(a,(unsigned char[]){24,0x27,0x10,0x13,0x88,0},6,1);
    assert(sv_app_view(a).hud.hp.current==5000 && !sv_app_view(a).hud.hp.boosted && !sv_app_view(a).hud.hp.bar);
    finish(a);
}
static Packet body_packet(const char *body)
{ Packet p={{27},1};word(&p,1);word(&p,2);word(&p,3);word(&p,4);text(&p,body);return p; }
static void effects_reset_and_errors(void)
{
    SvApp *a=open_version(sv_status_fixture_version);uint64_t gen=sv_app_view(a).generation;
    packet(a,body_packet("Player"));packet(a,body_packet("Wolf"));packet(a,body_packet("Wolf"));packet(a,body_packet("Player"));
    feed(a,(unsigned char[]){138,1,138,0},4,2);
    const char *bodies[]={"Player","Wolf","Player"};SvStatusEffect effect;uint64_t previous=0;
    for(unsigned i=0;i<3;i++) {
        assert(sv_app_take_status_effect(a,gen,&effect)==SV_OK);
        assert(effect.kind==SV_STATUS_BODY_EFFECT && !strcmp(effect.body,bodies[i]) && effect.generation==gen && effect.sequence>previous);previous=effect.sequence;
    }
    for(int i=1;i>=0;i--) {
        assert(sv_app_take_status_effect(a,gen,&effect)==SV_OK && effect.kind==SV_STATUS_IDLE_EFFECT && effect.idle==i && effect.sequence>previous);previous=effect.sequence;
    }
    assert(sv_app_take_status_effect(a,gen,&effect)==SV_WAITING);
    for(unsigned i=0;i<SV_STATUS_EFFECT_CAPACITY;i++)feed(a,(unsigned char[]){138,1},2,1);
    assert(sv_app_receive(a,gen,(unsigned char[]){138,0},2)==SV_OK);
    assert(sv_app_step(a,1).result==SV_EVENT_OVERFLOW && !sv_app_view(a).active);
    assert(sv_app_open(a,sv_status_fixture_version)==SV_OK);assert(!sv_app_view(a).hud.available && !sv_app_view(a).hud.revision);
    assert(sv_app_receive(a,gen,(unsigned char[]){138,0},2)==SV_STALE);
    assert(sv_app_take_status_effect(a,gen,&effect)==SV_STALE);
    gen=sv_app_view(a).generation;assert(sv_app_take_status_effect(a,gen,&effect)==SV_WAITING);
    /* Out-of-range stat IDs never publish a partial or indexed update. */
    Packet invalid={{28,6},2};for(unsigned i=0;i<5;i++)word(&invalid,1);
    assert(sv_app_receive(a,gen,invalid.bytes,invalid.size)==SV_OK);
    assert(sv_app_step(a,1).result==SV_DECODE_ERROR && !sv_app_view(a).hud.available);
    assert(sv_app_open(a,sv_status_fixture_version)==SV_OK);gen=sv_app_view(a).generation;
    Packet too_long={{174},1};for(unsigned i=0;i<256;i++)byte(&too_long,'x');byte(&too_long,0);
    assert(sv_app_receive(a,gen,too_long.bytes,too_long.size)==SV_OK);
    assert(sv_app_step(a,1).result==SV_DECODE_ERROR && !sv_app_view(a).hud.available);
    finish(a);
}
int main(void)
{
    latest_all_fields();split_and_adjacent();version_defaults();depth_speed_and_semantics();stat_colour_boundaries();status_slot_formats();baseline_presentation_boundaries();hp_occurrences_and_projection();effects_reset_and_errors();
    puts("SV-C012 production status checks passed: 27 outcomes, wire/default/semantic branches, split+adjacent, HP occurrences, body/idle retention, generation/reset");return 0;
}

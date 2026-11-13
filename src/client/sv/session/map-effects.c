#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "session/map-effects.h"
#define SOUND_QUEUE 16
#define WORLD_WIDTH 198
#define WORLD_HEIGHT 66
#define SKY_ALTITUDE 20
typedef struct { int x,y,destination,type; } Particle;
typedef struct { uint64_t id,expires,order; int x,y; SvEffectVisual visual; bool active; } Transient;
struct SvMapEffects {
    uint64_t generation,revision,now_ms,weather_ms,order,sound_sequence;
    uint32_t seed,rng;
    int columns,rows,version[6];
    bool clock_started,lamp_bright,wind_notice;
    unsigned lamp_fainting;
    unsigned flash_step,lightning_step,flash_level;
    bool flash_suppressed,lightning_suppressed,flash_visible,lightning_visible;
    int lightning_volume,lightning_sound_type;
    SvMapEffectOptions options;
    SvMapColour palette[256];
    SvWeatherUpdate weather;
    int64_t cloud_xfrac[10],cloud_yfrac[10];
    unsigned char rain_colour,snow_colour,sand_colour,sand_glyph;
    uint32_t rain_glyphs[5],snow_glyph,sand_picture;
    Particle particles[SV_WEATHER_PARTICLES];
    size_t particle_count;
    int generation_ticks,wind_ticks,speed_ticks[4],speed[4];
    uint64_t cloud_second;
    int audible_type,audible_wind,quiet_ticks;
    Transient transient[SV_TRANSIENT_EFFECTS];
    SvWeatherSound sounds[SOUND_QUEUE];
    size_t sound_count;
};
static SvMapColour flash_colour(const SvMapEffects *,unsigned,SvMapColour);
typedef struct { const SvMapEffects *e; uint32_t rng; int x,y; SvEffectScope scope; } ColourContext;
static uint32_t random32(uint32_t *state)
{
    uint32_t x=*state;x^=x<<13;x^=x>>17;x^=x<<5;*state=x?x:1;return *state;
}
static int colour_random(ColourContext *c,int n) { return n>0?(int)(random32(&c->rng)%(unsigned)n):0; }
static int colour_randint(ColourContext *c,int n) { return colour_random(c,n)+1; }
static int version_compare(const int *v,const int *gate)
{
    for(int i=0;i<6;++i) if(v[i]!=gate[i]) return v[i]>gate[i]?1:-1;
    return 0;
}
#define is_newer_than(v,a,b,c,d,e,f) (version_compare(v,(const int[]){a,b,c,d,e,f})>0)
#define is_older_than(v,a,b,c,d,e,f) (version_compare(v,(const int[]){a,b,c,d,e,f})<0)
static unsigned char searchlight(ColourContext *c);
static unsigned char shimmer(ColourContext *c);
#include "map-colours.inc"
#undef is_newer_than
#undef is_older_than
static unsigned char shimmer(ColourContext *c)
{
    static const unsigned char colours[]={4,12,1,13,6,8,5};
    return colours[colour_random(c,7)];
}
static unsigned char searchlight(ColourContext *c)
{
    if(c->scope!=SV_EFFECT_MAP) return animated_colour(c,18);
    /* Full viewport coordinates; unlike legacy there is no y -= 22 repeat. */
    double angle=atan2(-(double)(c->y-c->e->rows/2),(double)(c->x-c->e->columns/2))*180.0/3.14159265358979323846;
    if(angle<0) angle+=360;
    unsigned speed=(unsigned)((c->e->now_ms/100*5+(c->e->now_ms/10%10)/2)%360);
    unsigned beam=((unsigned)angle+360-speed)%180/4;
    static const unsigned char colours[]={4,3,11,3,4};
    return beam<5?colours[beam]:1;
}
static const uint32_t default_palette[16]={
    0x000000,0xffffff,0x9d9d9d,0xff8d00,0xb70000,0x009d44,0x0000ff,0x8d6600,
    0x666666,0xcdcdcd,0xaf00ff,0xffff00,0xff3030,0x00ff00,0x00ffff,0xc79d55
};
SvResult sv_map_effects_reset(SvMapEffects *e,uint64_t generation,int columns,int rows,uint32_t seed)
{
    if(!e||columns<1||columns>198||rows<1||rows>66) return SV_INVALID;
    memset(e,0,sizeof(*e));e->generation=generation;e->columns=columns;e->rows=rows;
    e->seed=e->rng=seed?seed:1;e->revision=1;
    const int v[]={4,9,2,1,0,0};memcpy(e->version,v,sizeof(v));
    e->options=(SvMapEffectOptions){.subterm_flicker=true,.palette_animation=true,.use_colour=true};
    e->rain_colour=6;e->snow_colour=1;e->sand_colour=15;e->sand_glyph='+';
    e->speed[1]=e->speed[3]=1;e->speed[2]=3;
    for(unsigned i=0;i<256;++i){uint32_t c=default_palette[i%16];e->palette[i]=(SvMapColour){c>>16,c>>8,c};}
    for(int i=0;i<10;++i)e->weather.clouds[i].x1=-9999;
    return SV_OK;
}
SvMapEffects *sv_map_effects_create(uint64_t g,int columns,int rows,uint32_t seed)
{
    SvMapEffects *e=malloc(sizeof(*e));if(!e)return NULL;
    if(sv_map_effects_reset(e,g,columns,rows,seed)!=SV_OK){free(e);return NULL;}return e;
}
void sv_map_effects_destroy(SvMapEffects *e){free(e);}
void sv_map_effects_set_version(SvMapEffects *e,const int v[6]){if(e&&v)memcpy(e->version,v,sizeof(e->version));}
void sv_map_effects_options(SvMapEffects *e,const SvMapEffectOptions *o)
{if(e&&o){e->options=*o;++e->revision;}}
void sv_map_effects_lamp(SvMapEffects *e,unsigned fainting){if(e)e->lamp_fainting=fainting;}
SvResult sv_map_effects_base_palette(SvMapEffects *e,const uint32_t colours[16])
{
    if(!e||!colours)return SV_INVALID;
    for(int i=0;i<16;++i)if(colours[i]>0xffffff)return SV_INVALID;
    for(int i=0;i<16;++i){uint32_t c=colours[i];e->palette[i]=(SvMapColour){c>>16,c>>8,c};}
    ++e->revision;return SV_OK;
}
SvResult sv_map_effects_palette(SvMapEffects *e,uint64_t g,unsigned char i,unsigned char r,unsigned char green,unsigned char b)
{
    if(!e)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    if(!e->options.palette_animation)return SV_OK;
    /* 127/128 are refresh markers, never palette entries. */
    if(i!=127&&i!=128)e->palette[i]=(SvMapColour){r,green,b};
    ++e->revision;return SV_OK;
}
SvResult sv_map_effects_colouring(SvMapEffects *e,uint64_t g,unsigned char rain,unsigned char snow,
                                bool has_sand,unsigned char sand,unsigned char glyph)
{
    if(!e)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    e->rain_colour=rain;e->snow_colour=snow;
    if(has_sand){e->sand_colour=sand;e->sand_glyph=glyph;}
    ++e->revision;return SV_OK;
}
static void sound_append(SvMapEffects *e,SvWeatherSoundKind kind,int type,int wind,int volume)
{
    e->sounds[e->sound_count++]=(SvWeatherSound){kind,type,wind,volume,e->generation,++e->sound_sequence};
}
SvResult sv_map_effects_weather(SvMapEffects *e,uint64_t g,const SvWeatherUpdate *w)
{
    if(!e||!w)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    if(w->type < -1||w->wind<0||w->wind>4||w->generation_speed<0||w->intensity<0||w->speed<0)
        return SV_DECODE_ERROR;
    /* Failure to retain an observable sound transition commits no model changes. */
    bool stop=w->type==-1,fade=!stop&&w->type%10==0;
    if((stop||fade)&&e->sound_count==SOUND_QUEUE)return SV_BACKPRESSURE;
    SvWeatherCloud previous[10];memcpy(previous,e->weather.clouds,sizeof(previous));
    e->weather=*w;
    for(int i=0;i<10;++i) {
        if(w->clear_clouds)e->weather.clouds[i].x1=-9999;
        else if(!(w->changed_clouds&(1u<<i)))e->weather.clouds[i]=previous[i];
        else e->cloud_xfrac[i]=e->cloud_yfrac[i]=0;
    }
    if(stop||(w->type%10000)/10!=0)e->particle_count=0;
    if(w->type>=10000){e->generation_ticks=e->wind_ticks=0;memset(e->speed_ticks,0,sizeof(e->speed_ticks));}
    int type=w->type%10;if(type>0&&type<=3)e->speed[type]=w->speed;
    if(stop)e->weather.type=0;
    if(stop||fade){sound_append(e,stop?SV_WEATHER_SOUND_STOP:SV_WEATHER_SOUND_FADE,type,w->wind,0);e->audible_type=0;e->wind_notice=false;}
    ++e->revision;return SV_OK;
}
SvMapColour sv_map_effects_colour(const SvMapEffects *e,unsigned char attr,int x,int y,SvEffectScope scope)
{
    if(!e)return (SvMapColour){255,255,255};
    unsigned index=attr;
    if(!e->options.use_colour)index=attr?1:0;
    else if(attr>=64&&attr<80)index=attr-64+16;
    else if(attr>=16) {
        ColourContext c={e,e->seed^(uint32_t)(e->now_ms/100)*0x9e3779b9u^(uint32_t)x*131u^(uint32_t)y*8191u^attr,x,y,scope};
        if(!c.rng)c.rng=1;
        index=animated_colour(&c,attr);
    }
    SvMapColour colour=e->palette[index&255];
    if(e->options.lighter_dark_blue&&(index==6||index==22)&&
       colour.r==0&&colour.g==0&&colour.b==255)
        colour=(SvMapColour){0,51,255};
    return flash_colour(e,index,colour);
}
void sv_map_effects_weather_glyphs(SvMapEffects *e,const uint32_t rain[5],uint32_t snow,uint32_t sand)
{if(e){if(rain)memcpy(e->rain_glyphs,rain,sizeof(e->rain_glyphs));e->snow_glyph=snow;e->sand_picture=sand;}}
SvResult sv_map_effects_start(SvMapEffects *e,uint64_t g,uint64_t id,int x,int y,SvEffectVisual visual,uint64_t expires)
{
    if(!e||!id||x<0||y<0||x>=e->columns||y>=e->rows)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    Transient *slot=NULL;
    for(int i=0;i<SV_TRANSIENT_EFFECTS;++i){if(e->transient[i].active&&e->transient[i].id==id){slot=&e->transient[i];break;}if(!e->transient[i].active&&!slot)slot=&e->transient[i];}
    if(!slot)return SV_BACKPRESSURE;
    *slot=(Transient){id,expires,++e->order,x,y,visual,true};++e->revision;return SV_OK;
}
SvResult sv_map_effects_stop(SvMapEffects *e,uint64_t g,uint64_t id)
{
    if(!e)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    for(int i=0;i<SV_TRANSIENT_EFFECTS;++i)if(e->transient[i].active&&e->transient[i].id==id){e->transient[i].active=false;++e->revision;return SV_OK;}
    return SV_RECOVERED;
}
SvResult sv_map_effects_screenflash(SvMapEffects *e,uint64_t g,bool suppress)
{
    if(!e)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    e->flash_step=1;e->flash_suppressed=suppress;
    ++e->revision;return SV_OK;
}
SvResult sv_map_effects_lightning(SvMapEffects *e,uint64_t g,int volume,int sound_type,
                                bool weather_source,bool suppress)
{
    if(!e||volume<0||volume>100)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    if(weather_source&&e->options.no_weather)return SV_OK;
    e->lightning_step=1;e->lightning_suppressed=suppress;
    e->lightning_volume=volume;e->lightning_sound_type=sound_type;
    ++e->revision;return SV_OK;
}
SvResult sv_map_effects_cancel_flashes(SvMapEffects *e,uint64_t g)
{
    if(!e)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    e->flash_step=e->lightning_step=0;e->flash_visible=e->lightning_visible=false;
    ++e->revision;return SV_OK;
}
static void flash_step(SvMapEffects *e)
{
    /* do_weather processes these before its no_weather early return. A projection
     * is applied to the current palette, so an intervening server palette change
     * is not overwritten by restoring a stale effect backup. */
    if(e->flash_step){
        unsigned step=e->flash_step;
        e->flash_visible=e->options.palette_animation&&!e->flash_suppressed&&step<20;
        e->flash_level=step<20?20-step:0;
        if(++e->flash_step==21)e->flash_step=0;
    }
    if(e->lightning_step){
        unsigned step=e->lightning_step;
        e->lightning_visible=e->options.palette_animation&&!e->options.disable_lightning&&
                              !e->lightning_suppressed&&step<12;
        unsigned delay=e->lightning_volume>=85?1:86-(unsigned)e->lightning_volume;
        if(step==delay)sound_append(e,SV_WEATHER_SOUND_THUNDER,e->lightning_sound_type,0,e->lightning_volume);
        unsigned end=delay>12?delay:12;
        if(++e->lightning_step==end+1)e->lightning_step=0;
    }
}
static SvMapColour flash_colour(const SvMapEffects *e,unsigned index,SvMapColour colour)
{
    if(!e->options.palette_animation)return colour;
    /* ANIM_FULL_PALETTE_FLASH affects both banks, excluding the black entries. */
    if(e->flash_visible&&index<32&&index%16){
        unsigned level=e->flash_level;
        colour.r+=(255-colour.r)*level/19;
        colour.g+=(255-colour.g)*level/19;
        colour.b+=(255-colour.b)*level/19;
    }
    /* ANIM_FULL_PALETTE_LIGHTNING is disabled in baseline: extended bank only. */
    if(e->lightning_visible&&!e->options.disable_lightning){
        switch(index){
        case 18:colour=(SvMapColour){0xcc,0xcc,0xff};break;
        case 20:colour=(SvMapColour){0xff,0x77,0x88};break;
        case 21:colour=(SvMapColour){0x33,0xff,0x33};break;
        case 22:colour=(SvMapColour){0x44,0x66,0xff};break;
        case 23:colour=(SvMapColour){0xad,0x88,0x33};break;
        case 24:colour=(SvMapColour){0x88,0x88,0x88};break;
        case 25:colour=(SvMapColour){0xee,0xee,0xee};break;
        case 29:colour=(SvMapColour){0xbb,0xff,0xbb};break;
        case 31:colour=(SvMapColour){0xf7,0xcd,0x85};break;
        default:break;
        }
    }
    return colour;
}
static int bounded_random(SvMapEffects *e,int n){return n>0?(int)(random32(&e->rng)%(unsigned)n):0;}
static int64_t distance_to(int x,int y,int32_t cx,int32_t cy)
{
    int64_t dx=llabs((int64_t)x-cx),dy=llabs((int64_t)y-cy);
    return dx>dy?dx+dy/2:dy+dx/2;
}
static bool cloud_allows(SvMapEffects *e,int x,int y)
{
    bool any=false,inside=false;
    for(int i=0;i<10;++i){SvWeatherCloud *c=&e->weather.clouds[i];if(c->x1==-9999)continue;any=true;
        int64_t d=distance_to(x,y,c->x1,c->y1)+distance_to(x,y,c->x2,c->y2);
        if(d<=c->distance&&bounded_random(e,100)>=(d-(int64_t)c->distance*3/4)*4)inside=true;}
    return !any||inside;
}
static int32_t saturate32(int64_t x){return x>INT32_MAX?INT32_MAX:x<INT32_MIN?INT32_MIN:(int32_t)x;}
static void weather_step(SvMapEffects *e)
{
    if(e->options.no_weather)return;
    int type=e->weather.type;if(type<0)type=0;
    type%=10000;int pregenerate=type/10+1;type%=10;
    e->weather.type=type;
    if(++e->generation_ticks>=e->weather.generation_speed||pregenerate>1) {
        if(e->generation_ticks>=e->weather.generation_speed)e->generation_ticks=0;else --pregenerate;
        int64_t amount=(int64_t)pregenerate*e->weather.intensity;
        /* Larger requests cannot fit the baseline bounded particle array. Avoid
         * signed overflow on hostile int32 parameters before applying density. */
        if(amount>SV_WEATHER_PARTICLES)amount=SV_WEATHER_PARTICLES+1;
        else amount=amount*(e->options.graphics_mode&&!e->options.ascii_weather?25:15)/10;
        if(type>=1&&type<=3&&amount<=SV_WEATHER_PARTICLES-(int64_t)e->particle_count) {
            for(int64_t i=0;i<amount;++i){int x=bounded_random(e,WORLD_WIDTH-2)+1,y=bounded_random(e,WORLD_HEIGHT-1+SKY_ALTITUDE)-SKY_ALTITUDE;
                if(cloud_allows(e,x,y))e->particles[e->particle_count++]=(Particle){x,y,y+SKY_ALTITUDE,type};}
        }
    }
    int wind=e->weather.wind,shift=0;
    if(wind&&++e->wind_ticks>=(wind+wind%2)/2){e->wind_ticks=0;shift=wind%2?1:-1;}
    bool gravity[4]={false};
    for(int i=1;i<=3;++i)if(++e->speed_ticks[i]>=e->speed[i]){e->speed_ticks[i]=0;gravity[i]=true;}
    for(size_t i=0;i<e->particle_count;){Particle *p=&e->particles[i];
        if(gravity[p->type])++p->y;
        if(p->type==1||!bounded_random(e,wind))p->x+=shift;
        if(p->x<1)p->x=WORLD_WIDTH-2;else if(p->x>WORLD_WIDTH-2)p->x=1;
        bool gone=p->type==1?(p->y>=WORLD_HEIGHT-2||p->y>=p->destination):(p->y>WORLD_HEIGHT-2||p->y>p->destination);
        if(gone){memmove(p,p+1,(e->particle_count-i-1)*sizeof(*p));--e->particle_count;}else ++i;
    }
}
static size_t visible_particles(const SvMapEffects *e)
{
    size_t n=0;
    /* Packet panel origin includes legacy left/top padding; translate once. */
    int64_t px=(int64_t)e->weather.panel_x+13,py=(int64_t)e->weather.panel_y+1;
    for(size_t i=0;i<e->particle_count;++i){const Particle *p=&e->particles[i];if(p->x>=px&&p->x<px+e->columns&&p->y>=py&&p->y<py+e->rows)++n;}
    return n;
}
SvResult sv_map_effects_tick(SvMapEffects *e,uint64_t g,uint64_t now)
{
    if(!e)return SV_INVALID;
    if(g!=e->generation)return SV_STALE;
    if(e->clock_started&&now<e->now_ms)return SV_INVALID;
    bool first=!e->clock_started;
    if(first){e->clock_started=true;e->now_ms=e->weather_ms=now;e->cloud_second=now/1000;}
    uint64_t before=e->now_ms;
    if(!first&&now==before&&now-e->weather_ms<10)return SV_OK;
    if(e->sound_count>SOUND_QUEUE-2)return SV_BACKPRESSURE;
    uint64_t old_weather_ms=e->weather_ms;
    e->now_ms=now;
    for(int i=0;i<SV_TRANSIENT_EFFECTS;++i)if(e->transient[i].active&&e->transient[i].expires&&now>=e->transient[i].expires)e->transient[i].active=false;
    if(now/100!=before/100){int lamp_wind=e->wind_notice&&e->weather.wind?(e->weather.wind+e->weather.wind%2)/2*4-1:20;
        e->lamp_bright=bounded_random(e,lamp_wind)!=0;if(e->lamp_fainting)--e->lamp_fainting;}
    /* Bound work per call, retaining unprocessed time rather than blocking UI. */
    unsigned steps=0;
    while(e->weather_ms<=now&&now-e->weather_ms>=10&&steps++<1000){
        e->weather_ms+=10;
        flash_step(e);
        uint64_t second=e->weather_ms/1000;
        if(e->cloud_second<second){e->cloud_second=second;
            if(!e->options.no_weather)for(int i=0;i<10;++i){
                SvWeatherCloud *c=&e->weather.clouds[i];if(c->x1==-9999)continue;
                e->cloud_xfrac[i]+=c->xm100;e->cloud_yfrac[i]+=c->ym100;
                int64_t dx=e->cloud_xfrac[i]/100,dy=e->cloud_yfrac[i]/100;
                c->x1=saturate32((int64_t)c->x1+dx);c->x2=saturate32((int64_t)c->x2+dx);
                c->y1=saturate32((int64_t)c->y1+dy);c->y2=saturate32((int64_t)c->y2+dy);
                e->cloud_xfrac[i]-=dx*100;e->cloud_yfrac[i]-=dy*100;
            }
        }
        weather_step(e);
    }
    size_t visible=visible_particles(e);
    int type=e->weather.type%10,wind=e->weather.wind;
    if(!e->options.no_weather){
    if(visible>=8)e->wind_notice=true;
    else if(visible<=3)e->wind_notice=false;
    if(visible>=8&&type>0&&(type!=e->audible_type||wind!=e->audible_wind)){
        if(e->sound_count==SOUND_QUEUE)return SV_BACKPRESSURE;
        sound_append(e,SV_WEATHER_SOUND_START,type,wind,100);e->audible_type=type;e->audible_wind=wind;e->quiet_ticks=0;
    }else if(visible<=3&&e->audible_type&&e->weather_ms/10>old_weather_ms/10){
        e->quiet_ticks+=(int)(e->weather_ms/10-old_weather_ms/10);
        if(e->quiet_ticks>=100){if(e->sound_count==SOUND_QUEUE)return SV_BACKPRESSURE;sound_append(e,SV_WEATHER_SOUND_FADE,type,wind,0);e->audible_type=0;e->quiet_ticks=0;}
    }else e->quiet_ticks=0;
    }
    ++e->revision;return SV_OK;
}
bool sv_map_effects_overlay(const SvMapEffects *e,int x,int y,SvEffectVisual *out)
{
    if(!e||!out||x<0||y<0||x>=e->columns||y>=e->rows)return false;
    bool present=false;SvEffectVisual visual={0};
    const Transient *top=NULL;
    for(int i=0;i<SV_TRANSIENT_EFFECTS;++i){const Transient *t=&e->transient[i];if(t->active&&t->x==x&&t->y==y&&(!top||t->order>top->order))top=t;}
    if(top){visual=top->visual;present=true;}
    if(!e->options.no_weather){
        int64_t wx=(int64_t)x+e->weather.panel_x+13,wy=(int64_t)y+e->weather.panel_y+1;
        for(size_t i=0;i<e->particle_count;++i){const Particle *p=&e->particles[i];if(p->x!=wx||p->y!=wy)continue;
            int wind=e->weather.wind;
            uint32_t glyph=p->type==1?e->rain_glyphs[wind]:p->type==2?e->snow_glyph:e->sand_picture;
            if(!e->options.graphics_mode||e->options.ascii_weather)glyph=0;
            bool graphical=glyph!=0;
            if(!glyph)glyph=p->type==1?(wind==0?'|':(wind%2==0?'/':'\\')):p->type==2?'*':e->sand_glyph;
            visual=(SvEffectVisual){glyph,p->type==1?e->rain_colour:p->type==2?e->snow_colour:e->sand_colour,
                    graphical&&e->options.graphics_mode==2&&!e->options.ascii_weather&&!e->options.no2mask_weather,
                    SV_EFFECT_REPLACE_ALL};
            if(visual.two_mask)visual.mode=SV_EFFECT_REPLACE_FOREGROUND;
            present=true;
        }
    }
    if(present)*out=visual;
    return present;
}
SvResult sv_map_effects_sound(SvMapEffects *e,SvWeatherSound *out,bool consume)
{
    if(!e||!out)return SV_INVALID;
    if(!e->sound_count)return SV_WAITING;
    *out=e->sounds[0];if(consume){--e->sound_count;memmove(e->sounds,e->sounds+1,e->sound_count*sizeof(*e->sounds));}return SV_OK;
}
uint64_t sv_map_effects_revision(const SvMapEffects *e){return e?e->revision:0;}
size_t sv_map_effects_particles(const SvMapEffects *e){return e?e->particle_count:0;}
const SvWeatherUpdate *sv_map_effects_weather_state(const SvMapEffects *e){return e?&e->weather:NULL;}

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "protocol/rendering.h"
static bool same(SvMapColour a,SvMapColour b){return a.r==b.r&&a.g==b.g&&a.b==b.b;}
static void put32(unsigned char *b,int32_t value)
{uint32_t u=(uint32_t)value;b[0]=u>>24;b[1]=u>>16;b[2]=u>>8;b[3]=u;}
static size_t weather_packet(unsigned char *b,int type,int count)
{
    b[0]=SV_RENDER_WEATHER;
    int32_t v[]={type,3,1,50,1,-13,-1,count};
    for(int i=0;i<8;++i)put32(b+1+i*4,v[i]);
    return 33;
}
int main(void)
{
    const int version[6]={4,9,2,1,0,0},old[6]={4,7,4,5,0,0};
    SvMapEffects *e=sv_map_effects_create(7,66,44,1);assert(e);
    SvRenderingEvent event={0},sentinel;size_t consumed=999;
    unsigned char bytes[400]={0};
    size_t length=weather_packet(bytes,1,2);
    const int32_t first[]={0,2,3,4,5,120,-100,100};
    const int32_t ignored[]={20,8,9,10,11,12,13,14};
    for(int i=0;i<8;++i){put32(bytes+length+i*4,first[i]);put32(bytes+length+32+i*4,ignored[i]);}
    length+=64;
    memset(&sentinel,0x5a,sizeof(sentinel));event=sentinel;
    for(size_t i=0;i<length;++i){assert(sv_rendering_decode(bytes,i,version,&event,&consumed)==SV_WAITING);assert(consumed==999);assert(memcmp(&event,&sentinel,sizeof(event))==0);}
    bytes[length]=SV_RENDER_PALETTE;bytes[length+1]=6;bytes[length+2]=11;bytes[length+3]=22;bytes[length+4]=33;
    assert(sv_rendering_decode(bytes,length+5,version,&event,&consumed)==SV_OK&&consumed==length);
    assert(event.weather.changed_clouds==1&&event.weather.clouds[0].xm100==-100);
    assert(sv_rendering_apply(e,6,&event)==SV_STALE);
    assert(sv_rendering_apply(e,7,&event)==SV_OK);
    assert(sv_map_effects_weather_state(e)->clouds[0].y2==5);
    assert(sv_rendering_decode(bytes+consumed,5,version,&event,&consumed)==SV_OK&&consumed==5);
    assert(sv_rendering_apply(e,7,&event)==SV_OK);
    assert(same(sv_map_effects_colour(e,6,0,0,SV_EFFECT_MAP),(SvMapColour){11,22,33}));
    unsigned char colouring[]={SV_RENDER_WEATHER_COLOURING,14,9,3,128};
    assert(sv_rendering_decode(colouring,4,version,&event,&consumed)==SV_WAITING);
    assert(sv_rendering_decode(colouring,5,version,&event,&consumed)==SV_OK&&event.colouring.has_sand);
    assert(sv_rendering_apply(e,7,&event)==SV_OK);
    assert(sv_rendering_decode(colouring,5,old,&event,&consumed)==SV_OK&&consumed==3&&!event.colouring.has_sand);
    SvMapEffectOptions options={.use_colour=true,.palette_animation=true,.subterm_flicker=true,.lighter_dark_blue=true};
    sv_map_effects_options(e,&options);
    assert(same(sv_map_effects_colour(e,6,0,0,SV_EFFECT_MAP),(SvMapColour){11,22,33})); /* custom blue wins */
    SvEffectVisual a={.glyph='*',.attribute=4},b={.glyph='!',.attribute=11},out;
    assert(sv_map_effects_start(e,7,1,2,3,a,0)==SV_OK);
    assert(sv_map_effects_start(e,7,2,2,3,b,100)==SV_OK);
    assert(sv_map_effects_overlay(e,2,3,&out)&&out.glyph=='!');
    assert(sv_map_effects_tick(e,7,0)==SV_OK);assert(sv_map_effects_tick(e,7,100)==SV_OK);
    assert(sv_map_effects_overlay(e,2,3,&out)&&out.glyph=='*');
    assert(sv_map_effects_stop(e,7,1)==SV_OK);
    assert(!sv_map_effects_overlay(e,2,3,&out));
    options.no_flicker=true;sv_map_effects_options(e,&options);
    assert(same(sv_map_effects_colour(e,18,2,3,SV_EFFECT_MAP),sv_map_effects_colour(e,4,2,3,SV_EFFECT_MAP)));
    options.no_flicker=false;options.misc_no_flicker=true;sv_map_effects_options(e,&options);
    assert(same(sv_map_effects_colour(e,18,2,3,SV_EFFECT_AUXILIARY),sv_map_effects_colour(e,4,2,3,SV_EFFECT_MAP)));
    for(int attr=16;attr<255;++attr){SvMapColour c=sv_map_effects_colour(e,(unsigned char)attr,2,3,SV_EFFECT_MESSAGE);assert(same(c,sv_map_effects_colour(e,(unsigned char)attr,2,3,SV_EFFECT_MESSAGE)));}
    bool independent=false;
    for(int x=0;x<66;++x)for(int y=0;y<22;++y)if(!same(sv_map_effects_colour(e,63,x,y,SV_EFFECT_MAP),sv_map_effects_colour(e,63,x,y+22,SV_EFFECT_MAP)))independent=true;
    assert(independent); /* full-height searchlight, not repeated lower half */
    /* Clear cloud restrictions, then real production clock generates weather. */
    weather_packet(bytes,211, -1);
    assert(sv_rendering_decode(bytes,33,version,&event,&consumed)==SV_OK);
    assert(sv_rendering_apply(e,7,&event)==SV_OK);
    assert(sv_map_effects_weather_state(e)->clouds[0].x1==-9999);
    assert(sv_map_effects_tick(e,7,300)==SV_OK);
    assert(sv_map_effects_particles(e)>0);
    size_t particle_count=sv_map_effects_particles(e);
    assert(sv_map_effects_tick(e,7,300)==SV_OK&&sv_map_effects_particles(e)==particle_count);
    bool drawn=false;for(int y=0;y<44;++y)for(int x=0;x<66;++x)if(sv_map_effects_overlay(e,x,y,&out)){drawn=true;assert(out.mode==SV_EFFECT_REPLACE_ALL);}
    assert(drawn);
    SvWeatherSound sound;assert(sv_map_effects_sound(e,&sound,false)==SV_OK&&sound.kind==SV_WEATHER_SOUND_START);
    uint64_t sequence=sound.sequence;assert(sv_map_effects_sound(e,&sound,true)==SV_OK&&sound.sequence==sequence);
    assert(sv_map_effects_sound(e,&sound,true)==SV_WAITING);
    /* Halt is committed once; projection/repaint cannot replay a sound. */
    weather_packet(bytes,-1,-1);assert(sv_rendering_decode(bytes,33,version,&event,&consumed)==SV_OK);
    assert(sv_rendering_apply(e,7,&event)==SV_OK&&sv_map_effects_particles(e)==0);
    for(int i=0;i<3;++i)assert(!sv_map_effects_overlay(e,2,3,&out));
    assert(sv_map_effects_sound(e,&sound,true)==SV_OK&&sound.kind==SV_WEATHER_SOUND_STOP);
    assert(sv_map_effects_sound(e,&sound,true)==SV_WAITING);
    assert(sv_map_effects_reset(e,8,66,22,2)==SV_OK);
    options.palette_animation=false;sv_map_effects_options(e,&options);
    uint32_t custom[16]={0};custom[6]=0x123456;
    assert(sv_map_effects_base_palette(e,custom)==SV_OK);
    assert(sv_map_effects_palette(e,8,6,99,98,97)==SV_OK);
    assert(same(sv_map_effects_colour(e,6,0,0,SV_EFFECT_MAP),(SvMapColour){0x12,0x34,0x56}));
    assert(sv_map_effects_stop(e,7,1)==SV_STALE);
    assert(sv_map_effects_sound(e,&sound,true)==SV_WAITING);
    /* Flash/lightning are production palette timelines independent of weather. */
    options.palette_animation=true;options.no_weather=true;sv_map_effects_options(e,&options);
    assert(sv_map_effects_tick(e,8,0)==SV_OK);
    unsigned char flash_packet[]={SV_RENDER_SCREENFLASH};
    assert(sv_rendering_decode(flash_packet,1,version,&event,&consumed)==SV_OK&&consumed==1);
    assert(sv_rendering_apply(e,8,&event)==SV_OK);
    assert(sv_map_effects_tick(e,8,10)==SV_OK);
    assert(same(sv_map_effects_colour(e,6,0,0,SV_EFFECT_MAP),(SvMapColour){255,255,255}));
    assert(same(sv_map_effects_colour(e,70,0,0,SV_EFFECT_MAP),(SvMapColour){255,255,255}));
    assert(same(sv_map_effects_colour(e,0,0,0,SV_EFFECT_MAP),(SvMapColour){0,0,0}));
    assert(sv_map_effects_tick(e,8,20)==SV_OK);
    SvMapColour fading=sv_map_effects_colour(e,6,0,0,SV_EFFECT_MAP);
    assert(fading.r>0x12&&fading.r<255);
    assert(sv_map_effects_palette(e,8,6,20,40,60)==SV_OK);
    assert(sv_map_effects_tick(e,8,200)==SV_OK);
    assert(same(sv_map_effects_colour(e,6,0,0,SV_EFFECT_MAP),(SvMapColour){20,40,60}));
    assert(sv_map_effects_lightning(e,8,100,7,true,false)==SV_OK); /* weather suppressed */
    assert(sv_map_effects_tick(e,8,210)==SV_OK);
    assert(sv_map_effects_sound(e,&sound,true)==SV_WAITING);
    assert(sv_map_effects_lightning(e,8,50,8,false,false)==SV_OK); /* ambient survives no_weather */
    assert(sv_map_effects_tick(e,8,220)==SV_OK);
    assert(same(sv_map_effects_colour(e,70,0,0,SV_EFFECT_MAP),(SvMapColour){0x44,0x66,0xff}));
    assert(same(sv_map_effects_colour(e,6,0,0,SV_EFFECT_MAP),(SvMapColour){20,40,60}));
    assert(sv_map_effects_tick(e,8,330)==SV_OK); /* visual ends after 12 steps */
    assert(same(sv_map_effects_colour(e,70,0,0,SV_EFFECT_MAP),(SvMapColour){0,51,255}));
    assert(sv_map_effects_sound(e,&sound,true)==SV_WAITING);
    assert(sv_map_effects_tick(e,8,570)==SV_OK); /* 101-(50+15) = 36 */
    assert(sv_map_effects_sound(e,&sound,true)==SV_OK&&sound.kind==SV_WEATHER_SOUND_THUNDER&&sound.volume==50&&sound.type==8);
    assert(sv_map_effects_sound(e,&sound,true)==SV_WAITING);
    assert(sv_map_effects_tick(e,8,570)==SV_OK);
    assert(sv_map_effects_sound(e,&sound,true)==SV_WAITING);
    options.disable_lightning=true;sv_map_effects_options(e,&options);
    assert(sv_map_effects_lightning(e,8,100,8,false,false)==SV_OK);
    assert(sv_map_effects_tick(e,8,580)==SV_OK);
    assert(same(sv_map_effects_colour(e,70,0,0,SV_EFFECT_MAP),(SvMapColour){0,51,255}));
    assert(sv_map_effects_sound(e,&sound,true)==SV_OK&&sound.kind==SV_WEATHER_SOUND_THUNDER);
    assert(sv_map_effects_cancel_flashes(e,7)==SV_STALE);
    assert(sv_map_effects_cancel_flashes(e,8)==SV_OK);
    sv_map_effects_destroy(e);puts("map-effects: PASS");return 0;
}

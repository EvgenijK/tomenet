#include <string.h>
#include "protocol/rendering.h"
static int32_t signed32(const unsigned char *b)
{
    uint32_t u=((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];
    return u <= INT32_MAX ? (int32_t)u : -1-(int32_t)(UINT32_MAX-u);
}
static bool sand_version(const int v[6])
{
    const int gate[6]={4,7,4,6,0,0};
    for(int i=0;i<6;++i) if(v[i]!=gate[i]) return v[i]>gate[i];
    return true;
}
SvResult sv_rendering_decode(const void *data,size_t size,const int version[6],
                            SvRenderingEvent *event,size_t *consumed)
{
    if(!event||!consumed||!version||(!data&&size)) return SV_INVALID;
    if(!size) return SV_WAITING;
    const unsigned char *b=data;
    SvRenderingEvent candidate={.type=b[0]};
    size_t needed;
    if(b[0]==SV_RENDER_SCREENFLASH) {
        needed=1;
    } else if(b[0]==SV_RENDER_PALETTE) {
        needed=5;
        if(size<needed) return SV_WAITING;
        candidate.palette.index=b[1]; candidate.palette.r=b[2];
        candidate.palette.g=b[3]; candidate.palette.b=b[4];
    } else if(b[0]==SV_RENDER_WEATHER_COLOURING) {
        candidate.colouring.has_sand=sand_version(version);
        needed=candidate.colouring.has_sand?5:3;
        if(size<needed) return SV_WAITING;
        candidate.colouring.rain=b[1];candidate.colouring.snow=b[2];
        if(needed==5){candidate.colouring.sand=b[3];candidate.colouring.glyph=b[4];}
    } else if(b[0]==SV_RENDER_WEATHER) {
        if(size<33) return SV_WAITING;
        SvWeatherUpdate *w=&candidate.weather;
        w->type=signed32(b+1);w->wind=signed32(b+5);w->generation_speed=signed32(b+9);
        w->intensity=signed32(b+13);w->speed=signed32(b+17);
        w->panel_x=signed32(b+21);w->panel_y=signed32(b+25);w->cloud_count=signed32(b+29);
        /* Legacy negative counts other than -1 carry ten cloud records. */
        uint32_t count=w->cloud_count<0?10:(uint32_t)w->cloud_count;
        if(w->cloud_count==-1) count=0;
        #if SIZE_MAX <= UINT32_MAX
        if(count>(SIZE_MAX-33)/32) return SV_DECODE_ERROR;
#endif
        needed=33+(size_t)count*32;
        if(size<needed) return SV_WAITING;
        w->clear_clouds=w->cloud_count==-1;
        for(uint32_t i=0;i<count;++i) {
            const unsigned char *c=b+33+(size_t)i*32;
            int32_t index=signed32(c);
            /* Forward compatible unknown slots, and unsafe negative slot, are ignored. */
            if(index<0||index>=SV_WEATHER_CLOUDS) continue;
            SvWeatherCloud *cloud=&w->clouds[index];
            cloud->x1=signed32(c+4);cloud->y1=signed32(c+8);
            cloud->x2=signed32(c+12);cloud->y2=signed32(c+16);
            cloud->distance=signed32(c+20);cloud->xm100=signed32(c+24);cloud->ym100=signed32(c+28);
            w->changed_clouds|=(uint16_t)(1u<<index);
        }
    } else return SV_DECODE_ERROR;
    *event=candidate;*consumed=needed;
    return SV_OK;
}
SvResult sv_rendering_apply(SvMapEffects *effects,uint64_t generation,const SvRenderingEvent *event)
{
    if(!event) return SV_INVALID;
    switch(event->type) {
    case SV_RENDER_SCREENFLASH:return sv_map_effects_screenflash(effects,generation,false);
    case SV_RENDER_WEATHER:return sv_map_effects_weather(effects,generation,&event->weather);
    case SV_RENDER_PALETTE:return sv_map_effects_palette(effects,generation,event->palette.index,
                                      event->palette.r,event->palette.g,event->palette.b);
    case SV_RENDER_WEATHER_COLOURING:return sv_map_effects_colouring(effects,generation,
                    event->colouring.rain,event->colouring.snow,event->colouring.has_sand,
                    event->colouring.sand,event->colouring.glyph);
    default:return SV_DECODE_ERROR;
    }
}

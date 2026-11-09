#ifndef SV_PROTOCOL_RENDERING_H
#define SV_PROTOCOL_RENDERING_H
#include "session/map-effects.h"
enum { SV_RENDER_PALETTE=137, SV_RENDER_WEATHER=177, SV_RENDER_WEATHER_COLOURING=199, SV_RENDER_SCREENFLASH=203 };
typedef struct {
    unsigned char type;
    union {
        SvWeatherUpdate weather;
        struct { unsigned char index,r,g,b; } palette;
        struct { unsigned char rain,snow,sand,glyph; bool has_sand; } colouring;
    };
} SvRenderingEvent;
/* Decode exactly one known rendering packet. WAITING/error leaves event and
 * consumed untouched. Caller retains stream remainder and owns backpressure. */
SvResult sv_rendering_decode(const void *bytes,size_t size,const int version[6],
                            SvRenderingEvent *event,size_t *consumed);
SvResult sv_rendering_apply(SvMapEffects *effects,uint64_t generation,const SvRenderingEvent *event);
#endif

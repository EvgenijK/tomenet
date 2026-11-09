#ifndef SV_MAP_EFFECTS_H
#define SV_MAP_EFFECTS_H
#include <stdbool.h>
#include <stdint.h>
#include "result.h"
#define SV_WEATHER_CLOUDS 10
#define SV_WEATHER_PARTICLES 1024
#define SV_TRANSIENT_EFFECTS 64
typedef struct SvMapEffects SvMapEffects;
typedef enum { SV_EFFECT_MAP, SV_EFFECT_AUXILIARY, SV_EFFECT_MESSAGE } SvEffectScope;
typedef struct { unsigned char r,g,b; } SvMapColour;
typedef struct {
    bool no_flicker, subterm_flicker, misc_no_flicker, palette_animation;
    bool use_colour, lighter_dark_blue, no_weather, ascii_weather, no2mask_weather;
    bool disable_lightning;
    int graphics_mode;
} SvMapEffectOptions;
typedef struct {
    int32_t x1,y1,x2,y2,distance,xm100,ym100;
} SvWeatherCloud;
typedef struct {
    int32_t type,wind,generation_speed,intensity,speed,panel_x,panel_y,cloud_count;
    uint16_t changed_clouds;
    bool clear_clouds;
    SvWeatherCloud clouds[SV_WEATHER_CLOUDS];
} SvWeatherUpdate;
typedef struct {
    uint32_t glyph;
    unsigned char attribute;
    bool two_mask;
    enum { SV_EFFECT_OVER, SV_EFFECT_REPLACE_FOREGROUND, SV_EFFECT_REPLACE_ALL } mode;
} SvEffectVisual;
typedef enum { SV_WEATHER_SOUND_NONE, SV_WEATHER_SOUND_START,
               SV_WEATHER_SOUND_FADE, SV_WEATHER_SOUND_STOP, SV_WEATHER_SOUND_THUNDER } SvWeatherSoundKind;
typedef struct {
    SvWeatherSoundKind kind;
    int type, wind, volume;
    uint64_t generation, sequence;
} SvWeatherSound;
SvMapEffects *sv_map_effects_create(uint64_t generation,int columns,int rows,uint32_t seed);
void sv_map_effects_destroy(SvMapEffects *effects);
SvResult sv_map_effects_reset(SvMapEffects *effects,uint64_t generation,int columns,int rows,uint32_t seed);
void sv_map_effects_set_version(SvMapEffects *effects,const int version[6]);
void sv_map_effects_options(SvMapEffects *effects,const SvMapEffectOptions *options);
/* Local CFG custom entries are independent of the server palette-animation switch. */
SvResult sv_map_effects_base_palette(SvMapEffects *effects,const uint32_t colours[16]);
SvResult sv_map_effects_palette(SvMapEffects *effects,uint64_t generation,unsigned char index,
                              unsigned char r,unsigned char g,unsigned char b);
SvResult sv_map_effects_colouring(SvMapEffects *effects,uint64_t generation,unsigned char rain,
                                unsigned char snow,bool has_sand,unsigned char sand,unsigned char sand_glyph);
SvResult sv_map_effects_weather(SvMapEffects *effects,uint64_t generation,const SvWeatherUpdate *update);
/* Clock mutation happens once outside drawing; read-only queries never advance RNG. */
SvResult sv_map_effects_tick(SvMapEffects *effects,uint64_t generation,uint64_t now_ms);
SvMapColour sv_map_effects_colour(const SvMapEffects *effects,unsigned char attribute,
                                 int x,int y,SvEffectScope scope);
/* 10ms source timelines: flash is full palette/20 steps; lightning affects the
 * animated palette bank/12 steps. Thunder follows the baseline volume delay.
 * Weather-source lightning respects no_weather; ambient lightning and flash do
 * not. suppress_visual captures the baseline modal/screen-icky state at trigger.
 * Sound type retains its original semantic category (e.g. weather=7/ambient=8). */
SvResult sv_map_effects_screenflash(SvMapEffects *,uint64_t generation,bool suppress_visual);
SvResult sv_map_effects_lightning(SvMapEffects *,uint64_t generation,int volume,
                                int sound_type,bool weather_source,bool suppress_visual);
SvResult sv_map_effects_cancel_flashes(SvMapEffects *,uint64_t generation);
/* Explicit semantic lamp state, shared by every cell in the frame. */
void sv_map_effects_lamp(SvMapEffects *effects,unsigned fainting);
/* Application supplies actual graphics pseudo-object IDs; zero uses ASCII. */
void sv_map_effects_weather_glyphs(SvMapEffects *effects,const uint32_t rain[5],uint32_t snow,uint32_t sand);
bool sv_map_effects_overlay(const SvMapEffects *effects,int x,int y,SvEffectVisual *visual);
/* Same-cell effects form an ordered stack. Stop/expiry reveals remaining state;
 * no saved terrain/raster is retained. Full capacity is explicit backpressure. */
SvResult sv_map_effects_start(SvMapEffects *effects,uint64_t generation,uint64_t id,
                            int x,int y,SvEffectVisual visual,uint64_t expires_ms);
SvResult sv_map_effects_stop(SvMapEffects *effects,uint64_t generation,uint64_t id);
/* One-shot transition outputs. Peek with consume=false; retry delivery safely. */
SvResult sv_map_effects_sound(SvMapEffects *effects,SvWeatherSound *sound,bool consume);
uint64_t sv_map_effects_revision(const SvMapEffects *effects);
size_t sv_map_effects_particles(const SvMapEffects *effects);
const SvWeatherUpdate *sv_map_effects_weather_state(const SvMapEffects *effects);
#endif

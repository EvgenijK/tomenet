#ifndef SV_SESSION_MAP_H
#define SV_SESSION_MAP_H
#include <stdbool.h>
#include <stdint.h>
#define SV_MAP_COLUMNS 66
#define SV_MAP_NORMAL_ROWS 22
#define SV_MAP_BIG_ROWS 44
typedef enum {
    SV_MAP_OVER,
    SV_MAP_REPLACE_FOREGROUND,
    SV_MAP_REPLACE_ALL
} SvMapOverlayMode;
typedef struct {
    uint32_t glyph;
    uint16_t subset; /* Source diagnostic; glyph-indexed resource owner resolves overrides. */
    unsigned char attribute;
    bool present, solid;
    SvMapOverlayMode overlay_mode; /* Used only by transient overlay projection. */
} SvMapVisual;
typedef struct {
    uint32_t background_rgb;
    SvMapVisual terrain, foreground;
} SvMapCell;
typedef enum { SV_MAP_BACKGROUND=1, SV_MAP_TERRAIN=2, SV_MAP_FOREGROUND=4 } SvMapFields;
typedef enum { SV_MAP_SELF, SV_MAP_TARGET, SV_MAP_HOVER, SV_MAP_INDICATORS } SvMapIndicatorKind;
typedef struct { int x,y; bool visible; } SvMapIndicator;
typedef struct SvMap SvMap;
/* Main-thread semantic owner. Generation belongs to the session, not resources.
 * Decoders commit complete updates here; no wire/version/SDL or Term dependency. */
SvMap *sv_map_create(int rows, uint64_t session_generation);
void sv_map_destroy(SvMap *);
bool sv_map_reset(SvMap *, int rows, uint64_t session_generation);
bool sv_map_apply(SvMap *,uint64_t generation,int x,int y,unsigned fields,const SvMapCell *);
bool sv_map_wipe(SvMap *,uint64_t generation,int x,int y,int width,int height);
bool sv_map_set_indicator(SvMap *,uint64_t generation,SvMapIndicatorKind,int x,int y,bool visible);
const SvMapCell *sv_map_cell(const SvMap *,int x,int y);
SvMapIndicator sv_map_indicator(const SvMap *,SvMapIndicatorKind);
int sv_map_rows(const SvMap *);
uint64_t sv_map_generation(const SvMap *);
uint64_t sv_map_revision(const SvMap *);
#endif

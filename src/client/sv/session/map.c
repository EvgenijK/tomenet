#include "session/map.h"
#include <stdlib.h>
#include <string.h>
struct SvMap {
    SvMapCell cells[SV_MAP_BIG_ROWS][SV_MAP_COLUMNS];
    SvMapIndicator indicators[SV_MAP_INDICATORS];
    int rows;
    uint64_t generation, revision;
};
static bool valid_rows(int rows){return rows==SV_MAP_NORMAL_ROWS || rows==SV_MAP_BIG_ROWS;}
static bool position(const SvMap *map,int x,int y)
{return map && x>=0 && x<SV_MAP_COLUMNS && y>=0 && y<map->rows;}
SvMap *sv_map_create(int rows,uint64_t generation)
{
    if(!valid_rows(rows))return NULL;
    SvMap *map=calloc(1,sizeof(*map));
    if(map)sv_map_reset(map,rows,generation);
    return map;
}
void sv_map_destroy(SvMap *map){free(map);}
bool sv_map_reset(SvMap *map,int rows,uint64_t generation)
{
    if(!map || !valid_rows(rows))return false;
    memset(map->cells,0,sizeof(map->cells));
    memset(map->indicators,0,sizeof(map->indicators));
    map->rows=rows;map->generation=generation;map->revision++;
    return true;
}
bool sv_map_apply(SvMap *map,uint64_t generation,int x,int y,unsigned fields,const SvMapCell *cell)
{
    if(!position(map,x,y) || !cell || generation!=map->generation || !fields || (fields&~7u))return false;
    SvMapCell *out=&map->cells[y][x];
    if(fields&SV_MAP_BACKGROUND)out->background_rgb=cell->background_rgb&0xffffff;
    if(fields&SV_MAP_TERRAIN)out->terrain=cell->terrain;
    if(fields&SV_MAP_FOREGROUND)out->foreground=cell->foreground;
    map->revision++;return true;
}
bool sv_map_wipe(SvMap *map,uint64_t generation,int x,int y,int width,int height)
{
    if(!map || generation!=map->generation || x<0 || y<0 || width<0 || height<0 ||
       x>SV_MAP_COLUMNS || y>map->rows || width>SV_MAP_COLUMNS-x || height>map->rows-y)return false;
    if(!width || !height)return true;
    for(int row=y;row<y+height;++row)memset(&map->cells[row][x],0,(size_t)width*sizeof(SvMapCell));
    for(unsigned i=0;i<SV_MAP_INDICATORS;++i) {
        SvMapIndicator *v=&map->indicators[i];
        if(v->x>=x && v->x<x+width && v->y>=y && v->y<y+height)v->visible=false;
    }
    map->revision++;return true;
}
bool sv_map_set_indicator(SvMap *map,uint64_t generation,SvMapIndicatorKind kind,int x,int y,bool visible)
{
    if(!map || generation!=map->generation || (unsigned)kind>=SV_MAP_INDICATORS ||
       (visible && !position(map,x,y)))return false;
    map->indicators[kind]=(SvMapIndicator){x,y,visible};map->revision++;return true;
}
const SvMapCell *sv_map_cell(const SvMap *map,int x,int y){return position(map,x,y)?&map->cells[y][x]:NULL;}
SvMapIndicator sv_map_indicator(const SvMap *map,SvMapIndicatorKind kind)
{return map && (unsigned)kind<SV_MAP_INDICATORS?map->indicators[kind]:(SvMapIndicator){0};}
int sv_map_rows(const SvMap *map){return map?map->rows:0;}
uint64_t sv_map_generation(const SvMap *map){return map?map->generation:0;}
uint64_t sv_map_revision(const SvMap *map){return map?map->revision:0;}

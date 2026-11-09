#include "settings/map-font.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
bool sv_map_font_init(SvMapFont *map,const char *user,const char *library,const char *effective)
{
    if (!map || !user || !library || !effective || strlen(user)>=sizeof(map->user) ||
        strlen(library)>=sizeof(map->library) || strlen(effective)>=sizeof(map->effective)) return false;
    memset(map,0,sizeof(*map)); strcpy(map->user,user); strcpy(map->library,library);
    const char *name=strrchr(effective,'/');
    strcpy(map->effective,name?name+1:effective); return true;
}
void sv_map_font_definitions(SvVisualDefinitions *out,const SvPreferences *preferences)
{
    memset(out,0,sizeof(*out));
    for (size_t i=0;i<sv_preferences_mapping_count(preferences);++i) {
        const SvPreferenceMapping *m=sv_preferences_mapping(preferences,i);
        SvVisualDefinition *table=NULL; size_t count=0;
        switch(m->family) {
        case 'U':table=out->unknown;count=127;break;
        case 'F':table=out->features;count=512;break;
        case 'K':table=out->kinds;count=1280;break;
        case 'R':table=out->monsters;count=1280;break;
        }
        if (table && m->index<count) table[m->index]=(SvVisualDefinition){m->character,m->attribute};
    }
}
SvPreferenceResult sv_map_font_bootstrap(SvPreferences *preferences,
    const SvPreferenceContext *context,void *opaque)
{
    SvMapFont *map=opaque;
    SvPreferenceContext ctx=context?*context:sv_preferences_default_context();
    ctx.character_offset=0;ctx.graphics_enabled=false;ctx.subtileset=-1;
    sv_preferences_reset_visuals(preferences);
    const char *base=strrchr(map->effective,'/');base=base?base+1:map->effective;
    char stem[1024],relative[SV_FILE_PATH];
    if (strlen(base)>=sizeof(stem)) return (SvPreferenceResult){SV_PREF_UNAVAILABLE,0,0};
    strcpy(stem,base);
    /* SDL3 extensionless PCF selection names address the shipped custom PRFs.
     * Native SV references include .pcf; normalize this resource role only. */
    size_t n=strlen(stem);
    if (n>4 && !strcmp(stem+n-4,".pcf")) stem[n-4]=0;
    if (snprintf(map->source,sizeof(map->source),"font-custom-%s.prf",stem)>=(int)sizeof(map->source) ||
        snprintf(relative,sizeof(relative),"user/%s",map->source)>=(int)sizeof(relative))
        return (SvPreferenceResult){SV_PREF_UNAVAILABLE,0,0};
    SvFileResult found=sv_file_resource(&map->resolved,map->user,map->library,relative);
    if (found==SV_FILE_MISSING) {
        strcpy(map->source,"font-sv.prf");
        found=sv_file_resource(&map->resolved,map->user,map->library,"user/font-sv.prf");
        /* SDL3 has no shipped font-sdl3 PRF. Missing system definitions mean
         * authoritative server glyphs (zero overrides), with an explicit result. */
    }
    map->source_result=found;
    if(found!=SV_FILE_OK && found!=SV_FILE_MISSING) {
        map->status=SV_PREF_UNAVAILABLE;
        return (SvPreferenceResult){SV_PREF_UNAVAILABLE,0,1};
    }
    if(found==SV_FILE_MISSING)
        fprintf(stderr,"SV map preferences: %s missing; using server glyph identities (zero overrides)\n",map->source);
    SvPreferenceResult loaded=sv_preferences_load(preferences,SV_OPTION_SHARED,
        map->source,SV_PREF_OPTIONAL,&ctx);
    map->status=loaded.status;
    sv_map_font_definitions(&map->definitions,preferences);
    ++map->revision;
    return loaded;
}
SvPreferences *sv_map_font_reload(SvMapFont *map,const SvPreferences *base,
    const char *effective,const SvPreferenceContext *context)
{
    if (!map || !base || !effective || strlen(effective)>=sizeof(map->effective)) return NULL;
    SvPreferences *candidate=sv_preferences_clone(base,false);
    if (!candidate) return NULL;
    const char *name=strrchr(effective,'/');
    strcpy(map->effective,name?name+1:effective);
    SvPreferenceResult result=sv_map_font_bootstrap(candidate,context,map);
    if (result.status==SV_PREF_FATAL || result.status==SV_PREF_UNAVAILABLE) {
        sv_preferences_destroy(candidate);return NULL;
    }
    return candidate;
}

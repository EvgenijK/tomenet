#ifndef SV_MAP_FONT_H
#define SV_MAP_FONT_H
#include "settings/preferences.h"
#include "protocol/visuals.h"
typedef struct {
    char user[SV_FILE_PATH],library[SV_FILE_PATH],effective[SV_FILE_PATH];
    char source[SV_FILE_PATH];
    SvFilePath resolved;
    SvFileResult source_result;
    SvVisualDefinitions definitions;
    uint64_t revision;
    SvPreferenceStatus status;
} SvMapFont;
bool sv_map_font_init(SvMapFont *,const char *user,const char *library,const char *effective);
SvPreferenceResult sv_map_font_bootstrap(SvPreferences *,const SvPreferenceContext *,void *);
/* A fresh preference candidate, with effects from this load only. Caller
 * publishes it with the prepared font or discards it whole on failure. */
SvPreferences *sv_map_font_reload(SvMapFont *candidate,const SvPreferences *,
    const char *effective,const SvPreferenceContext *);
void sv_map_font_definitions(SvVisualDefinitions *,const SvPreferences *);
#endif

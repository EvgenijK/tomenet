#include "settings/preferences.h"
#include <assert.h>
#include <stdio.h>
int main(int argc,char **argv) {
 assert(argc==4);
 SvPreferences *p=sv_preferences_create(argv[1],argv[2],NULL); assert(p);
 SvPreferenceResult r=sv_preferences_load(p,SV_OPTION_SHARED,argv[3],SV_PREF_EXPLICIT,NULL);
 assert(r.status==SV_PREF_CLEAN);
 assert(sv_preferences_mapping_count(p)==1272);
 SvPreferences *copy=sv_preferences_clone(p,false); assert(copy);
 assert(sv_preferences_mapping_count(copy)==sv_preferences_mapping_count(p));
 printf("supplied font PRF + fresh clone PASS: files=%u records=%zu mappings=%zu diagnostics=%u\n",r.files,sv_preferences_record_count(p),sv_preferences_mapping_count(p),r.diagnostics);
 sv_preferences_destroy(copy); sv_preferences_destroy(p); return 0;
}

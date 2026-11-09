#include "ui/font-selection.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct SvFontSelection {
    char user[SV_FILE_PATH],library[SV_FILE_PATH],error[256];
    SvFilePath destination;
    SvSettingsConfig opening,draft,disk;
    SvFont *active[2],*original[2];
    unsigned generation[2];
    bool fallback[2],original_fallback[2];
    SvFontSelectionPrepare prepare;
    void *context;
};
static bool role_valid(SvFontRole role) { return role==SV_FONT_TEXT || role==SV_FONT_MAP; }
static char *request(SvSettingsConfig *c,SvFontRole r) { return r==SV_FONT_TEXT?c->text_font:c->map_font; }
static char *profile(SvSettingsConfig *c,SvFontRole r) { return r==SV_FONT_TEXT?c->text_profile:c->map_profile; }
static bool copy(char *out,size_t size,const char *in)
{
    if (!in || strlen(in)>=size || strpbrk(in,"\r\n")) return false;
    strcpy(out,in);return true;
}
static SvFontSelectionResult diagnostic(SvFontSelection *s,SvFontSelectionResult result,const char *detail)
{
    snprintf(s->error,sizeof(s->error),"%s",detail);return result;
}
static SvFontSelectionResult file_result(SvFileResult result)
{
    return result==SV_FILE_MISSING?SV_FONT_SELECTION_MISSING:
        result==SV_FILE_INVALID?SV_FONT_SELECTION_INVALID:SV_FONT_SELECTION_IO;
}
static SvFileResult resolve(SvFontSelection *s,const char *name,bool bundled,SvFilePath *out)
{
    if (!name || !*name) return SV_FILE_INVALID;
    if (name[0]=='/' && !bundled) {
        struct stat st;
        out->owner=SV_FILE_SHARED;
        if (!copy(out->path,sizeof(out->path),name)) return SV_FILE_INVALID;
        if (sv_file_stat(out,&st)) return errno==ENOENT?SV_FILE_MISSING:
            errno==ENOTDIR?SV_FILE_INVALID:SV_FILE_IO;
        return S_ISREG(st.st_mode)?SV_FILE_OK:SV_FILE_INVALID;
    }
    char relative[SV_FILE_PATH];
    int n=snprintf(relative,sizeof(relative),"xtra/font/%s",name);
    if (n<0 || (size_t)n>=sizeof(relative)) return SV_FILE_INVALID;
    if (!bundled) return sv_file_resource(out,s->user,s->library,relative);
    if (!sv_file_path(out,s->library,relative,SV_FILE_BUNDLED)) return SV_FILE_INVALID;
    struct stat st;
    if (sv_file_stat(out,&st)) return errno==ENOENT?SV_FILE_MISSING:
        errno==ENOTDIR?SV_FILE_INVALID:SV_FILE_IO;
    return S_ISREG(st.st_mode)?SV_FILE_OK:SV_FILE_INVALID;
}
static SvFont *load(SvFontSelection *s,const char *name,const char *requested_profile,
                   bool bundled,SvFontSelectionResult *status)
{
    SvFilePath font_path,profile_path;
    char normalized[SV_FILE_PATH];
    const char *base=strrchr(name,'/');base=base?base+1:name;
    if(!strchr(base,'.')) {
        int size=snprintf(normalized,sizeof(normalized),"%s.pcf",name);
        if(size<0 || (size_t)size>=sizeof(normalized)){*status=SV_FONT_SELECTION_INVALID;return NULL;}
        name=normalized;
    }
    SvFileResult resolved=resolve(s,name,bundled,&font_path);
    if (resolved!=SV_FILE_OK) {
        *status=file_result(resolved);return NULL;
    }
    const char *actual_profile=requested_profile;
    if (*requested_profile && strcmp(requested_profile,"sv-pcf-encoded-v1") &&
        strcmp(requested_profile,"sv-ttf-ascii-v1")) {
        resolved=resolve(s,requested_profile,bundled,&profile_path);
        if (resolved!=SV_FILE_OK) { *status=SV_FONT_SELECTION_PROFILE;return NULL; }
        actual_profile=profile_path.path;
    }
    SvFont *font=sv_font_open_profile_file(font_path.path,actual_profile);
    *status=font?SV_FONT_SELECTION_OK:(*requested_profile?SV_FONT_SELECTION_PROFILE:SV_FONT_SELECTION_LOAD);
    return font;
}
SvFontSelection *sv_font_selection_create(const char *user,const char *library,const SvSettingsConfig *config)
{
    if (!config || !user || !library) return NULL;
    SvFontSelection *s=calloc(1,sizeof(*s));
    if (!s) return NULL;
    if (!copy(s->user,sizeof(s->user),user) || !copy(s->library,sizeof(s->library),library) ||
        !sv_file_path(&s->destination,user,"sv/tomenet.cfg",SV_FILE_PROFILE)) goto failed;
    s->opening=*config;s->opening.records=s->opening.records_tail=NULL;
    s->draft=s->opening;
    /* CLI/default active choices need not match the destination at edit-open. */
    sv_settings_config_defaults(&s->disk);
    sv_settings_config_load(&s->disk,s->destination.path);
    sv_settings_config_dispose(&s->disk);
    for (int r=0;r<2;++r) {
        SvFontSelectionResult result;
        s->active[r]=load(s,request(&s->draft,(SvFontRole)r),profile(&s->draft,(SvFontRole)r),false,&result);
        if (!s->active[r]) {
            const char *fallbacks[]={"CascadiaMono-Regular.ttf","16x24x.pcf"};
            unsigned start=r==SV_FONT_TEXT?0:1;
            for (unsigned i=start;i<2 && !s->active[r];++i)
                s->active[r]=load(s,fallbacks[i],"",true,&result);
            if (!s->active[r]) goto failed;
            s->fallback[r]=true;
            snprintf(s->error,sizeof(s->error),"Requested %s font/profile unavailable; using bundled fallback",
                r==SV_FONT_TEXT?"text":"map");
            fprintf(stderr,"SV fonts: %s\n",s->error);
        }
        s->generation[r]=1;
    }
    return s;
failed:
    fprintf(stderr,"SV fonts: required text/map font and bundled fallback unavailable\n");
    sv_font_selection_destroy(s);return NULL;
}
void sv_font_selection_destroy(SvFontSelection *s)
{
    if (!s) return;
    for (int r=0;r<2;++r) {
        sv_font_close(s->active[r]);
        sv_font_close(s->original[r]);
    }
    free(s);
}
void sv_font_selection_prepare(SvFontSelection *s,SvFontSelectionPrepare callback,void *context)
{
    if (!s) return;
    s->prepare=callback;s->context=context;
}
SvFontSelectionResult sv_font_selection_preview(SvFontSelection *s,SvFontRole role,
                                                const char *name,const char *requested_profile)
{
    if (!s || !role_valid(role) || !name || !requested_profile) return SV_FONT_SELECTION_INVALID;
    if (strlen(name)>=SV_SETTINGS_TEXT_MAX || strlen(requested_profile)>=SV_SETTINGS_TEXT_MAX ||
        strpbrk(name,"\r\n") || strpbrk(requested_profile,"\r\n"))
        return diagnostic(s,SV_FONT_SELECTION_INVALID,"Font/profile reference too long or invalid");
    strcpy(request(&s->draft,role),name);
    strcpy(profile(&s->draft,role),requested_profile);
    SvFontSelectionResult result;
    SvFont *candidate=load(s,name,requested_profile,false,&result);
    if (!candidate) return diagnostic(s,result,"Requested font/profile unavailable; previous working selection retained");
    if (s->prepare && !s->prepare(s->context,role,candidate)) {
        sv_font_close(candidate);
        return diagnostic(s,SV_FONT_SELECTION_APPLY,"Resource consumer rejected replacement; previous working selection retained");
    }
    if (!s->original[role]) {
        s->original[role]=s->active[role];s->original_fallback[role]=s->fallback[role];
    }
    else sv_font_close(s->active[role]);
    s->active[role]=candidate;s->generation[role]++;s->fallback[role]=false;
    s->error[0]=0;return SV_FONT_SELECTION_OK;
}
SvFontSelectionResult sv_font_selection_cancel(SvFontSelection *s,SvFontRole role)
{
    if (!s || !role_valid(role)) return SV_FONT_SELECTION_INVALID;
    if (s->original[role]) {
        if (s->prepare && !s->prepare(s->context,role,s->original[role]))
            return diagnostic(s,SV_FONT_SELECTION_APPLY,"Cannot restore previous resource consumer");
        sv_font_close(s->active[role]);s->active[role]=s->original[role];s->original[role]=NULL;
        s->fallback[role]=s->original_fallback[role];
        s->generation[role]++;
    }
    strcpy(request(&s->draft,role),request(&s->opening,role));
    strcpy(profile(&s->draft,role),profile(&s->opening,role));
    s->error[0]=0;return SV_FONT_SELECTION_OK;
}
SvFontSelectionResult sv_font_selection_save(SvFontSelection *s,SvFontRole role)
{
    if (!s || !role_valid(role)) return SV_FONT_SELECTION_INVALID;
    unsigned mask=role==SV_FONT_TEXT?SV_CONFIG_TEXT_FONT|SV_CONFIG_TEXT_PROFILE:SV_CONFIG_MAP_FONT|SV_CONFIG_MAP_PROFILE;
    unsigned saved=0,conflicts=0;
    SvFileResult result=sv_settings_config_save_resources(&s->destination,&s->disk,&s->draft,mask,&saved,&conflicts);
    unsigned font_bit=role==SV_FONT_TEXT?SV_CONFIG_TEXT_FONT:SV_CONFIG_MAP_FONT;
    unsigned profile_bit=role==SV_FONT_TEXT?SV_CONFIG_TEXT_PROFILE:SV_CONFIG_MAP_PROFILE;
    if (saved&font_bit) strcpy(request(&s->disk,role),request(&s->draft,role));
    if (saved&profile_bit) strcpy(profile(&s->disk,role),profile(&s->draft,role));
    if (result!=SV_FILE_OK) return diagnostic(s,result==SV_FILE_CONFLICT?SV_FONT_SELECTION_CONFLICT:SV_FONT_SELECTION_IO,
        conflicts?"External resource edit retained; conflicting preview remains active and unsaved":"Font Save failed; preview remains active and dirty");
    strcpy(request(&s->opening,role),request(&s->draft,role));
    strcpy(profile(&s->opening,role),profile(&s->draft,role));
    if (!sv_font_selection_dirty(s,role)) { sv_font_close(s->original[role]);s->original[role]=NULL; }
    s->error[0]=0;return SV_FONT_SELECTION_OK;
}
SvFont *sv_font_selection_font(const SvFontSelection *s,SvFontRole role)
{ return s && role_valid(role)?s->active[role]:NULL; }
const char *sv_font_selection_requested(const SvFontSelection *s,SvFontRole role)
{ return s && role_valid(role)?request((SvSettingsConfig *)&s->draft,role):""; }
const char *sv_font_selection_profile(const SvFontSelection *s,SvFontRole role)
{ return s && role_valid(role)?profile((SvSettingsConfig *)&s->draft,role):""; }
const char *sv_font_selection_error(const SvFontSelection *s) { return s?s->error:"Font selection unavailable"; }
bool sv_font_selection_dirty(const SvFontSelection *s,SvFontRole role)
{
    return s && role_valid(role) &&
        (strcmp(sv_font_selection_requested(s,role),request((SvSettingsConfig *)&s->opening,role)) ||
         strcmp(sv_font_selection_profile(s,role),profile((SvSettingsConfig *)&s->opening,role)));
}
unsigned sv_font_selection_generation(const SvFontSelection *s,SvFontRole role)
{ return s && role_valid(role)?s->generation[role]:0; }
bool sv_font_selection_fallback(const SvFontSelection *s,SvFontRole role)
{ return s && role_valid(role) && s->fallback[role]; }

#include "settings/config.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const char *key; size_t offset, capacity; unsigned bit; } ResourceField;
#define STR(k,f,b) {k,offsetof(SvSettingsConfig,f),sizeof(((SvSettingsConfig *)0)->f),b}
#define NUM(k,f,b) {k,offsetof(SvSettingsConfig,f),0,b}
static const ResourceField resources[]={
    STR("svTextFont",text_font,SV_CONFIG_TEXT_FONT), STR("svMapFont",map_font,SV_CONFIG_MAP_FONT),
    STR("svTextProfile",text_profile,SV_CONFIG_TEXT_PROFILE), STR("svMapProfile",map_profile,SV_CONFIG_MAP_PROFILE),
    STR("svGraphicsFilter",graphics_filter,SV_CONFIG_GRAPHICS_FILTER), STR("svPcfFilter",pcf_filter,SV_CONFIG_PCF_FILTER),
    NUM("graphicsForceOutline",graphics_force_outline,SV_CONFIG_OUTLINE),
    NUM("graphics",graphics,SV_CONFIG_GRAPHICS), STR("graphic_tiles",graphic_tiles,SV_CONFIG_TILESET)
};
#undef STR
#undef NUM
static bool equal(const SvSettingsConfig *a, const SvSettingsConfig *b, const ResourceField *f)
{
    const char *x=(const char *)a+f->offset,*y=(const char *)b+f->offset;
    return f->capacity?!strcmp(x,y):*(const int *)x==*(const int *)y;
}
static bool valid(const SvSettingsConfig *c,const ResourceField *f)
{
    const char *value=(const char *)c+f->offset;
    if (f->capacity) {
        const char *end=memchr(value,0,f->capacity);
        if (!end || strpbrk(value,"\r\n") || (*value &&
            (*value==' ' || *value=='\t' || end[-1]==' ' || end[-1]=='\t'))) return false;
        if (f->bit==SV_CONFIG_GRAPHICS_FILTER || f->bit==SV_CONFIG_PCF_FILTER)
            return !strcmp(value,"nearest") || !strcmp(value,"linear") || !strcmp(value,"pixelart");
        return true;
    }
    int n=*(const int *)value;
    return f->bit==SV_CONFIG_OUTLINE?n>=-1 && n<=6:n>=0 && n<=2;
}
static bool line_key(const char *line,size_t size,const char *key)
{
    while (size && (*line==' ' || *line=='\t')) { line++;size--; }
    size_t n=strlen(key);
    return size>=n && !memcmp(line,key,n) && (size==n || line[n]==' ' || line[n]=='\t' || line[n]=='\r');
}
SvFileResult sv_settings_config_save_resources(const SvFilePath *target,
    const SvSettingsConfig *opening,const SvSettingsConfig *edited,unsigned mask,
    unsigned *saved,unsigned *conflicts)
{
    SvFilePublication op={0};
    SvSettingsConfig current;
    char *bytes=NULL;
    size_t size=0;
    unsigned changed=0,clash=0,accepted=0,already=0;
    SvFileResult result=SV_FILE_INVALID;
    if (saved) *saved=0;
    if (conflicts) *conflicts=0;
    if (!target || target->owner!=SV_FILE_PROFILE || !opening || !edited ||
        !opening->overwrite_allowed || !edited->overwrite_allowed) return result;
    for (size_t i=0;i<sizeof(resources)/sizeof(resources[0]);++i) {
        const ResourceField *f=&resources[i];
        if (!(mask&f->bit)) continue;
        if (!valid(edited,f)) return SV_FILE_INVALID;
        if (!equal(opening,edited,f)) changed|=f->bit;
    }
    if (!changed) return SV_FILE_OK;
    /* Observe before rereading: commit detects replacement during the read/merge. */
    result=sv_file_publish_begin(&op,target,false);
    if (result!=SV_FILE_OK && result!=SV_FILE_REPLACE_REQUIRED) return result;
    if (op.existed) {
        FILE *in=sv_file_open(target,"rb");
        if (!in) { result=SV_FILE_IO;goto end; }
        bytes=malloc(4*1024*1024+1);
        if (!bytes) { fclose(in);result=SV_FILE_IO;goto end; }
        size=fread(bytes,1,4*1024*1024+1,in);
        bool failed=ferror(in) || size>4*1024*1024;
        if (fclose(in)) failed=true;
        if (failed) { result=SV_FILE_IO;goto end; }
    }
    sv_settings_config_defaults(&current);
    SvSettingsConfigResult parsed=sv_settings_config_parse(&current,bytes,size);
    if (parsed!=SV_SETTINGS_CONFIG_OK || !current.overwrite_allowed) {
        sv_settings_config_dispose(&current);result=SV_FILE_INVALID;goto end;
    }
    for (size_t i=0;i<sizeof(resources)/sizeof(resources[0]);++i) {
        const ResourceField *f=&resources[i];
        if (!(changed&f->bit)) continue;
        if (equal(&current,edited,f)) already|=f->bit;
        else if (!equal(&current,opening,f)) clash|=f->bit;
        else accepted|=f->bit;
    }
    sv_settings_config_dispose(&current);
    if (conflicts) *conflicts=clash;
    /* Resource/profile changes form one selection: never persist half a group. */
    if (clash) { result=SV_FILE_CONFLICT;goto end; }
    if (!accepted) { if (saved) *saved=already; result=clash?SV_FILE_CONFLICT:SV_FILE_OK;goto end; }
    for (size_t pos=0;pos<size;) {
        size_t start=pos;
        while (pos<size && bytes[pos]!='\n') pos++;
        size_t length=pos-start;
        if (pos<size) pos++;
        bool replace=false;
        for (size_t i=0;i<sizeof(resources)/sizeof(resources[0]);++i)
            if ((accepted&resources[i].bit) && line_key(bytes+start,length,resources[i].key)) replace=true;
        if (!replace && sv_file_publish_write(&op,bytes+start,pos-start)!=SV_FILE_OK) { result=SV_FILE_IO;goto end; }
    }
    if (size && bytes[size-1]!='\n' && sv_file_publish_write(&op,"\n",1)!=SV_FILE_OK) { result=SV_FILE_IO;goto end; }
    for (size_t i=0;i<sizeof(resources)/sizeof(resources[0]);++i) {
        const ResourceField *f=&resources[i];
        if (!(accepted&f->bit)) continue;
        char line[SV_SETTINGS_TEXT_MAX+80];
        const char *value=(const char *)edited+f->offset;
        int n=f->capacity?snprintf(line,sizeof(line),"%s %s\n",f->key,value):
            snprintf(line,sizeof(line),"%s %d\n",f->key,*(const int *)value);
        if (n<0 || (size_t)n>=sizeof(line) || sv_file_publish_write(&op,line,(size_t)n)!=SV_FILE_OK) {
            result=SV_FILE_IO;goto end;
        }
    }
    result=sv_file_publish_commit(&op,true);
    if (result==SV_FILE_OK) {
        if (saved) *saved=accepted|already;
        if (clash) result=SV_FILE_CONFLICT;
    }
end:
    sv_file_publish_cancel(&op);
    free(bytes);
    return result;
}

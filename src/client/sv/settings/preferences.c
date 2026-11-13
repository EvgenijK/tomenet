#define _XOPEN_SOURCE 700
#include "settings/preferences.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SV-local semantic constants from common/defines.h; importing the legacy
 * client would pull its terminal, protocol and frontend dependencies here. */
enum { PREF_R_MAX=1280, PREF_K_MAX=1280, PREF_F_MAX=512,
       PREF_GLYPH_MAX=65536, PREF_RAW_MAX=256, PREF_SUBSETS=10, PREF_MASKS=3 };
struct SvPreferences {
    char user_root[SV_FILE_PATH], library_root[SV_FILE_PATH];
    SvSettingsOptions options;
    signed char glyph_subtilesets[PREF_GLYPH_MAX];
    SvPreferenceRecord *records;
    size_t record_count;
    SvPreferenceMapping *mappings;
    size_t mapping_count;
    SvPreferenceKeymapRecord *keymap_records;
    size_t keymap_count;
    SvPreferenceMacro *macros;
    size_t macro_count, macro_capacity;
    SvPreferenceEffect *effects;
    size_t effect_count;
    SvPreferenceDiagnostic *diagnostics;
    size_t diagnostic_count;
    unsigned char *action, *queue;
    size_t action_size, queue_size, allocated;
    unsigned issues;
    bool fatal, saw_options;
    char stack[SV_PREFERENCE_INCLUDE_DEPTH][SV_FILE_PATH];
    unsigned depth;
};
void sv_preferences_swap(SvPreferences *a,SvPreferences *b)
{
    if (a && b) { SvPreferences temporary=*a; *a=*b; *b=temporary; }
}
static void *resize(SvPreferences *p, void *old, size_t old_size, size_t size)
{
    if (size > SV_PREFERENCE_BYTE_LIMIT || p->allocated-old_size > SV_PREFERENCE_BYTE_LIMIT-size) {
        p->fatal=true; return NULL;
    }
    void *next = realloc(old, size ? size : 1);
    if (!next) { p->fatal=true; return NULL; }
    p->allocated = p->allocated-old_size+size;
    return next;
}
static void release(SvPreferences *p, void *ptr, size_t size)
{
    if (ptr) { free(ptr); p->allocated-=size; }
}
static void *copy_bytes(SvPreferences *p, const void *bytes, size_t size)
{
    unsigned char *copy=resize(p,NULL,0,size+1);
    if (!copy) return NULL;
    if (size) memcpy(copy,bytes,size);
    copy[size]=0;
    return copy;
}
static void diagnose(SvPreferences *p, SvPreferenceDiagnosticKind kind,
                     const SvFilePath *source, unsigned line, const char *message,
                     bool issue)
{
    if (issue) { ++p->issues; ++p->options.warnings; }
    /* Even a data-budget failure has an observable result; diagnostics are
     * capped separately so malicious input cannot grow an unbounded log. */
    if (p->diagnostic_count>=256) return;
    SvPreferenceDiagnostic *next=realloc(p->diagnostics,
        (p->diagnostic_count+1)*sizeof(*next));
    if (!next) { p->fatal=true; return; }
    p->diagnostics=next;
    SvPreferenceDiagnostic *d=&next[p->diagnostic_count++];
    memset(d,0,sizeof(*d));
    d->kind=kind; d->line=line; d->message=message;
    if (source) d->source=*source;
}
static bool add_record(SvPreferences *p, SvOptionOwner role,
                       const SvFilePath *source, unsigned line, const char *text)
{
    char *copy=copy_bytes(p,text,strlen(text));
    if (!copy) return false;
    SvPreferenceRecord *next=resize(p,p->records,p->record_count*sizeof(*next),
                                  (p->record_count+1)*sizeof(*next));
    if (!next) { release(p,copy,strlen(text)+1); return false; }
    p->records=next;
    next[p->record_count++]=(SvPreferenceRecord){role,*source,line,copy};
    /* Legacy C002's bounded compatibility view is retained for its callers.
     * Full source records above are authoritative; the view never gates apply. */
    size_t needed=strlen(source->path)+strlen(text)+2;
    if (needed < sizeof(p->options.records)-p->options.record_bytes) {
        int n=snprintf(p->options.records+p->options.record_bytes,
            sizeof(p->options.records)-p->options.record_bytes,"%s\t%s\n",source->path,text);
        if (n>0) p->options.record_bytes+=(size_t)n;
    }
    return true;
}
static bool add_effect(SvPreferences *p, SvPreferenceEffect effect,
                       const void *bytes, size_t size)
{
    unsigned char *copy=copy_bytes(p,bytes,size);
    if (!copy) return false;
    SvPreferenceEffect *next=resize(p,p->effects,p->effect_count*sizeof(*next),
                                  (p->effect_count+1)*sizeof(*next));
    if (!next) { release(p,copy,size+1); return false; }
    p->effects=next; effect.bytes=copy; effect.size=size;
    next[p->effect_count++]=effect;
    return true;
}
static bool add_keymap_record(SvPreferences *p, SvPreferenceKeymapRecord record)
{
    SvPreferenceKeymapRecord *next=resize(p,p->keymap_records,
        p->keymap_count*sizeof(*next),(p->keymap_count+1)*sizeof(*next));
    if (!next) return false;
    p->keymap_records=next;
    next[p->keymap_count++]=record;
    return true;
}
SvPreferenceContext sv_preferences_default_context(void)
{
    return (SvPreferenceContext){.graphics_build=true,.sdl3_build=true,
        .custom_mapping_always=true,.subtileset=-1};
}
SvPreferences *sv_preferences_create(const char *user, const char *library,
                                     const SvSettingsOptions *initial)
{
    if (!user || strlen(user)>=SV_FILE_PATH || (library && strlen(library)>=SV_FILE_PATH)) return NULL;
    SvPreferences *p=calloc(1,sizeof(*p));
    if (!p) return NULL;
    memset(p->glyph_subtilesets,-1,sizeof(p->glyph_subtilesets));
    strcpy(p->user_root,user);
    if (library) strcpy(p->library_root,library);
    if (initial) p->options=*initial;
    else sv_settings_options_defaults(&p->options);
    if (!add_keymap_record(p,(SvPreferenceKeymapRecord){
        .kind=SV_PREF_KEYMAP_INITIAL,.rogue=p->options.values[0]!=0})) {
        sv_preferences_destroy(p); return NULL;
    }
    return p;
}
const SvSettingsOptions *sv_preferences_options(const SvPreferences *p) { return p?&p->options:NULL; }
SvSettingsOptions *sv_preferences_options_mutable(SvPreferences *p) { return p?&p->options:NULL; }
void sv_preferences_clear_effects(SvPreferences *p)
{
    if (!p) return;
    for (size_t i=0;i<p->effect_count;++i)
        release(p,(void *)p->effects[i].bytes,p->effects[i].size+1);
    release(p,p->effects,p->effect_count*sizeof(*p->effects));
    release(p,p->queue,p->queue_size);
    p->effects=NULL; p->effect_count=0; p->queue=NULL; p->queue_size=0;
}
void sv_preferences_destroy(SvPreferences *p)
{
    if (!p) return;
    sv_preferences_clear_effects(p);
    for (size_t i=0;i<p->record_count;++i) free((void *)p->records[i].text);
    for (size_t i=0;i<p->macro_count;++i) {
        free((void *)p->macros[i].trigger); free((void *)p->macros[i].action);
    }
    free(p->records); free(p->mappings); free(p->macros); free(p->diagnostics);
    free(p->keymap_records);
    free(p->action); free(p);
}
size_t sv_preferences_record_count(const SvPreferences *p) { return p?p->record_count:0; }
const SvPreferenceRecord *sv_preferences_record(const SvPreferences *p,size_t i) { return p && i<p->record_count?&p->records[i]:NULL; }
size_t sv_preferences_mapping_count(const SvPreferences *p) { return p?p->mapping_count:0; }
const SvPreferenceMapping *sv_preferences_mapping(const SvPreferences *p,size_t i) { return p && i<p->mapping_count?&p->mappings[i]:NULL; }
const SvPreferenceKeymapRecord *sv_preferences_keymap_record(const SvPreferences *p,size_t i)
{ return p && i<p->keymap_count?&p->keymap_records[i]:NULL; }
size_t sv_preferences_macro_count(const SvPreferences *p) { return p?p->macro_count:0; }
const SvPreferenceMacro *sv_preferences_macro(const SvPreferences *p,size_t i) { return p && i<p->macro_count?&p->macros[i]:NULL; }
size_t sv_preferences_effect_count(const SvPreferences *p) { return p?p->effect_count:0; }
const SvPreferenceEffect *sv_preferences_effect(const SvPreferences *p,size_t i) { return p && i<p->effect_count?&p->effects[i]:NULL; }
size_t sv_preferences_diagnostic_count(const SvPreferences *p) { return p?p->diagnostic_count:0; }
const SvPreferenceDiagnostic *sv_preferences_diagnostic(const SvPreferences *p,size_t i) { return p && i<p->diagnostic_count?&p->diagnostics[i]:NULL; }
const unsigned char *sv_preferences_queue(const SvPreferences *p,size_t *size)
{
    if (size) *size=p?p->queue_size:0;
    return p?p->queue:NULL;
}
int sv_preferences_glyph_subtileset(const SvPreferences *p,unsigned glyph)
{
    return p && glyph<PREF_GLYPH_MAX?p->glyph_subtilesets[glyph]:-1;
}
static bool basename_only(const char *name)
{
    return name && *name && strcmp(name,".") && strcmp(name,"..") &&
        !strchr(name,'/') && !strchr(name,'\\');
}
static bool option_entry(const char *name)
{
    size_t n=strlen(name);
    return !strcmp(name,"options.prf") || (n>=4 && !strcmp(name+n-4,".opt"));
}
static bool path_for(SvPreferences *p,SvFilePath *out,SvOptionOwner role,const char *name)
{
    char relative[SV_FILE_PATH];
    const char *folder=role==SV_OPTION_PROFILE?"sv":"user";
    int n=snprintf(relative,sizeof(relative),"%s/%s",folder,name);
    return n>0 && (size_t)n<sizeof(relative) && sv_file_path(out,
        role==SV_OPTION_BUNDLED?p->library_root:p->user_root,relative,
        role==SV_OPTION_PROFILE?SV_FILE_PROFILE:role==SV_OPTION_BUNDLED?SV_FILE_BUNDLED:SV_FILE_SHARED);
}
static SvFileResult inspect(const SvFilePath *path)
{
    struct stat st;
    if (!sv_file_stat(path,&st)) return S_ISREG(st.st_mode)?SV_FILE_OK:SV_FILE_INVALID;
    return errno==ENOENT?SV_FILE_MISSING:errno==ENOTDIR?SV_FILE_INVALID:SV_FILE_IO;
}
static SvFileResult resolve(SvPreferences *p,SvFilePath *path,SvOptionOwner *role,
                            const char *name,bool custom,bool options_fallback,bool route_options)
{
    if (!name || !*name) return SV_FILE_INVALID;
    if (custom && name[0]=='/') {
        if (strlen(name)>=sizeof(path->path)) return SV_FILE_INVALID;
        strcpy(path->path,name);
        path->owner=*role==SV_OPTION_PROFILE?SV_FILE_PROFILE:
            *role==SV_OPTION_BUNDLED?SV_FILE_BUNDLED:SV_FILE_SHARED;
        return inspect(path);
    }
    if (custom && name[0]=='~' && name[1]=='/') {
        const char *home=getenv("HOME");
        if (!home) return SV_FILE_INVALID;
        if (!sv_file_path(path,home,name+2,*role==SV_OPTION_PROFILE?SV_FILE_PROFILE:
            *role==SV_OPTION_BUNDLED?SV_FILE_BUNDLED:SV_FILE_SHARED)) return SV_FILE_INVALID;
        return inspect(path);
    }
    if (!custom && !basename_only(name)) return SV_FILE_INVALID;
    if (route_options && option_entry(name)) *role=SV_OPTION_PROFILE;
    if ((*role==SV_OPTION_BUNDLED && !*p->library_root) ||
        (*role!=SV_OPTION_BUNDLED && !*p->user_root)) return SV_FILE_MISSING;
    if (!path_for(p,path,*role,name)) return SV_FILE_INVALID;
    SvFileResult result=inspect(path);
    if (result==SV_FILE_MISSING && (*role==SV_OPTION_SHARED || options_fallback)) {
        if (!*p->library_root) return result;
        if (!path_for(p,path,SV_OPTION_BUNDLED,name)) return SV_FILE_INVALID;
        result=inspect(path);
    }
    return result;
}
/* Safe version of the baseline byte decoder. C-string consumers stop at the
 * first decoded NUL; malformed escapes are diagnosed instead of invoking UB. */
static int digit(unsigned char c,unsigned base)
{
    int n=c>='0' && c<='9'?c-'0':c>='a' && c<='f'?c-'a'+10:c>='A' && c<='F'?c-'A'+10:-1;
    return n>=0 && (unsigned)n<base?n:-1;
}
static bool decode(const char *text,unsigned char out[SV_PREFERENCE_MACRO_MAXLEN],size_t *size)
{
    size_t n=0;
    for (size_t i=0;text[i];++i) {
        unsigned char c=(unsigned char)text[i];
        if (c=='^') {
            if (!text[++i]) return false;
            c=(unsigned char)text[i]&037;
        } else if (c=='\\') {
            c=(unsigned char)text[++i];
            if (!c) return false;
            if (c=='x' || (c>='0' && c<='3')) {
                unsigned base=c=='x'?16:8;
                int a=digit((unsigned char)text[i+1],base);
                if (!text[i+1] || !text[i+2]) return false;
                int b=digit((unsigned char)text[i+2],base);
                if (a<0 || b<0) return false;
                c=(unsigned char)((base==16?0:(c-'0')*64)+a*base+b); i+=2;
            } else {
                switch (c) {
                case 'w':c=0; /* set below from baseline constants */ break;
                case 'W':c=0; break;
                case '\\':case '^':break;
                case 's':c=' ';break;
                case 'e':c=27;break;
                case 'b':c='\b';break;
                case 'n':c='\n';break;
                case 'r':c='\r';break;
                case 't':c='\t';break;
                default: /* Baseline silently discards an unknown escape. */ continue;
                }
                if (text[i]=='w') c=96;
                if (text[i]=='W') c=30;
            }
        }
        if (!c) { *size=n; out[n]=0; return true; }
        if (n>=SV_PREFERENCE_MACRO_MAXLEN-1) return false;
        out[n++]=c;
    }
    out[n]=0; *size=n; return true;
}
static bool trigger_allowed(const SvPreferenceContext *ctx,const unsigned char *bytes,size_t n)
{
    return !ctx->exclusive_trigger || !ctx->exclusive_trigger_size ||
        (n==ctx->exclusive_trigger_size && !memcmp(bytes,ctx->exclusive_trigger,n));
}
static bool macro_store(SvPreferences *p,const SvPreferenceContext *ctx,char tag,
                         const char *text,const SvFilePath *source,unsigned line)
{
    unsigned char bytes[SV_PREFERENCE_MACRO_MAXLEN]; size_t n;
    if (!decode(text,bytes,&n)) return false;
    if (tag=='A') {
        unsigned char *copy=copy_bytes(p,bytes,n);
        if (!copy) return false;
        release(p,p->action,p->action?p->action_size+1:0);
        p->action=copy; p->action_size=n; return true;
    }
    if (!trigger_allowed(ctx,bytes,n)) return true;
    /* Before the first A record, baseline macro_add receives NULL and ignores
     * a definition; an explicitly empty A action is still a valid action. */
    if (tag!='D' && !p->action) return true;
    size_t index=0;
    while (index<p->macro_count && (p->macros[index].trigger_size!=n ||
           memcmp(p->macros[index].trigger,bytes,n))) ++index;
    if (tag=='D') {
        if (index==p->macro_count) return true;
        release(p,(void *)p->macros[index].trigger,p->macros[index].trigger_size+1);
        release(p,(void *)p->macros[index].action,p->macros[index].action_size+1);
        memmove(p->macros+index,p->macros+index+1,(p->macro_count-index-1)*sizeof(*p->macros));
        /* The allocation still includes the spare slot until next resize. */
        --p->macro_count;
        memset(p->macros+p->macro_count,0,sizeof(*p->macros));
        return true;
    }
    unsigned char *trigger=copy_bytes(p,bytes,n);
    unsigned char *action=copy_bytes(p,p->action,p->action_size);
    if (!trigger || !action) {
        release(p,trigger,trigger?n+1:0); release(p,action,action?p->action_size+1:0); return false;
    }
    if (index==p->macro_count) {
        if (p->macro_count==p->macro_capacity) {
            SvPreferenceMacro *next=resize(p,p->macros,p->macro_capacity*sizeof(*next),
                                          (p->macro_count+1)*sizeof(*next));
            if (!next) { release(p,trigger,n+1); release(p,action,p->action_size+1); return false; }
            p->macros=next; p->macro_capacity=p->macro_count+1;
        }
        ++p->macro_count;
    } else {
        release(p,(void *)p->macros[index].trigger,p->macros[index].trigger_size+1);
        release(p,(void *)p->macros[index].action,p->macros[index].action_size+1);
    }
    p->macros[index]=(SvPreferenceMacro){
        .kind=tag=='P'?SV_PREF_MACRO_NORMAL:tag=='H'?SV_PREF_MACRO_HYBRID:SV_PREF_MACRO_COMMAND,
        .trigger=trigger,.trigger_size=n,.action=action,.action_size=p->action_size,
        .source=*source,.line=line};
    return true;
}
/* Baseline tokenizer permits quoted delimiter characters and backslash-escaped
 * delimiters; its final field contains the unparsed remainder. */
static unsigned tokenize(char *text,unsigned count,char **fields)
{
    unsigned n=0;
    if (!count) return 0;
    fields[n++]=text;
    for (char *s=text; *s && n<count; ++s) {
        if (*s==':' || *s=='/') { *s=0; fields[n++]=s+1; continue; }
        if (*s=='\'') {
            ++s;
            if (*s=='\\') { if (!s[1]) break; ++s; }
            if (!*s) break;
            ++s;
            if (!*s) break;
            if (*s!='\'') *s='\'';
        }
        if (*s=='\\' && s[1]) ++s;
    }
    return n;
}
static bool number(const char *text,long *value)
{
    /* Keep baseline strtol prefix/empty semantics; reject only unrepresentable
     * values, which otherwise lead to overflow or unsafe indexes. */
    errno=0; *value=strtol(text,NULL,0);
    return errno!=ERANGE;
}
static bool mapping_store(SvPreferences *p,SvPreferenceMapping next,
                           bool set_attr,bool set_char)
{
    size_t i=0;
    while (i<p->mapping_count && (p->mappings[i].family!=next.family ||
           p->mappings[i].index!=next.index ||
           ((next.family=='I' || next.family=='m') && p->mappings[i].subtileset!=next.subtileset))) ++i;
    if (i==p->mapping_count) {
        if (!set_char) { next.character=0; next.subtileset=p->glyph_subtilesets[0]; }
        SvPreferenceMapping *all=resize(p,p->mappings,p->mapping_count*sizeof(*all),
                                       (p->mapping_count+1)*sizeof(*all));
        if (!all) return false;
        p->mappings=all; ++p->mapping_count;
    } else {
        if (!set_attr) next.attribute=p->mappings[i].attribute;
        if (!set_char) {
            next.character=p->mappings[i].character;
            next.subtileset=p->mappings[i].subtileset;
        }
    }
    p->mappings[i]=next;
    return true;
}
static bool solid_wall(unsigned i)
{
    return i==26 || i==28 || (i>=50 && i<=63) || (i>=75 && i<=78) ||
        i==95 || (i>=98 && i<=100) || i==177 || (i>=183 && i<=185) || (i>=188 && i<=194);
}
static bool mapping_record(SvPreferences *p,const SvPreferenceContext *ctx,
    char tag,char *text,const SvFilePath *source,unsigned line,bool *bad_mapping)
{
    SvPreferenceMapping map={.family=tag,.subtileset=ctx->subtileset,.source=*source,.line=line};
    char *fields[5]; long values[5];
    if (tag=='I' && (!ctx->graphics_build || !ctx->graphics_enabled)) return true;
    if (tag=='m' && (!ctx->graphics_build || !ctx->sdl3_build)) return false;
    unsigned count=tag=='I'?5:tag=='m'?2:3;
    if (tokenize(text,count,fields)!=count) return false;
    for (unsigned i=0;i<(tag=='S'?1:count) && !(tag=='m' && i==1);++i)
        if (!number(fields[i],&values[i])) return false;
    if (values[0]<0 || (unsigned long)values[0]>UINT_MAX) return false;
    map.index=(unsigned)values[0];
    if (tag=='m') {
        unsigned index=(unsigned char)values[0];
        if (index>=PREF_MASKS || fields[1][0]!='#') return false;
        unsigned color;
        if (sscanf(fields[1]+1,"%06x",&color)!=1) return false;
        map.index=index; map.fields[0]=(long)(0xffu | (color<<8));
    } else if (tag=='I') {
        if (map.index>PREF_RAW_MAX) {
            diagnose(p,SV_PREF_DIAG_MAPPING,source,line,"Raw picture index exceeds baseline limit; ignored",false);
            return true;
        }
        for (unsigned i=0;i<4;++i) map.fields[i]=values[i+1];
    } else if (tag=='S') {
        /* Preserve the known baseline defect; C005 owns effective keymaps. */
        map.index=(unsigned)values[0]&0x7f;
        map.fields[0]=map.fields[1]=map.index;
        map.fields[2]=map.index>9 || map.index==5?0:map.index;
    } else {
        unsigned max=tag=='R'?PREF_R_MAX:tag=='K'?PREF_K_MAX:tag=='F'?PREF_F_MAX:101;
        if (tag=='U' && map.index>100) return true;
        if (map.index>=max ||
            (ctx->character_offset>0 && values[2]>LONG_MAX-ctx->character_offset) ||
            (ctx->character_offset<0 && values[2]<LONG_MIN-ctx->character_offset)) return false;
        long glyph=values[2]+ctx->character_offset;
        if (glyph<0 || glyph>=PREF_GLYPH_MAX ||
            ((!ctx->graphics_build || !ctx->graphics_enabled) && glyph>255)) return false;
        map.attribute=(unsigned char)values[1]; map.character=(unsigned)glyph;
        bool attr=ctx->custom_mapping_always || values[1]!=0;
        bool chr=ctx->custom_mapping_always || glyph!=0;
        if (tag=='F' && chr && (glyph==2 || glyph==127) && !solid_wall(map.index)) *bad_mapping=true;
        bool stored=mapping_store(p,map,attr,chr);
        if (stored && chr) {
            p->glyph_subtilesets[map.character]=(signed char)map.subtileset;
            for (size_t i=0;i<p->mapping_count;++i) {
                SvPreferenceMapping *other=&p->mappings[i];
                if ((other->family=='R' || other->family=='K' ||
                     other->family=='F' || other->family=='U') && other->character==map.character)
                    other->subtileset=map.subtileset;
            }
        }
        return stored;
    }
    if (!mapping_store(p,map,true,true)) return false;
    return tag!='S' || add_keymap_record(p,(SvPreferenceKeymapRecord){
        .kind=SV_PREF_KEYMAP_S,.key=map.index,.command=map.fields[1],
        .direction=map.fields[2]});
}
static bool message_bytes(const char *text,unsigned char *out,
                          size_t *size,bool message,bool *chat)
{
    size_t n=0; *chat=message && text[0]=='{' && text[1]=='+';
    if (*chat) { out[n++]=253; text+=2; }
    for (size_t i=0;text[i];++i) {
        unsigned char c=(unsigned char)text[i];
        if (c=='{') {
            if (text[i+1]=='{') ++i;
            else c=255;
        }
        out[n++]=c;
    }
    out[n]=0; *size=n; return true;
}
static bool action_record(SvPreferences *p,const SvPreferenceContext *ctx,char tag,
                           const char *text,const SvFilePath *source,unsigned line)
{
    if (tag=='?' && ctx->bodymonster) return true;
    size_t capacity=strlen(text)+2;
    unsigned char *translated=resize(p,NULL,0,capacity);
    if (!translated) return false;
    unsigned char bytes[SV_PREFERENCE_MACRO_MAXLEN];
    size_t size,n; bool chat;
    message_bytes(text,translated,&size,tag=='#',&chat);
    if (tag=='#') {
        bool added=add_effect(p,(SvPreferenceEffect){.kind=SV_PREF_MESSAGE,.chat=chat,
            .in_game=ctx->in_game,.generation=ctx->generation,.source=*source,.line=line},translated,size);
        release(p,translated,capacity); return added;
    }
    bool decoded=decode((const char *)translated,bytes,&n);
    release(p,translated,capacity);
    if (!decoded) return false;
    /* Reserve before publishing an intent; rejected records cannot leave an
     * action intent that never made it into the pending queue. */
    size_t old_size=p->queue_size;
    unsigned char *queue=resize(p,p->queue,old_size,old_size+n+1);
    if (!queue) return false;
    p->queue=queue;
    if (!add_effect(p,(SvPreferenceEffect){.kind=SV_PREF_ACTION,
            .in_game=ctx->in_game,.generation=ctx->generation,.source=*source,.line=line},bytes,n)) {
        /* No further delivery occurs after fatal allocation failure. The
         * allocation remains owned and is released by destroy. */
        return false;
    }
    memmove(queue+n+1,queue,old_size);
    memcpy(queue,bytes,n); queue[n]=29; p->queue_size=old_size+n+1;
    return true;
}
static bool apply_option(SvPreferences *p,const SvPreferenceContext *ctx,
                          const char *name,bool value,const SvFilePath *source,unsigned line)
{
    int slots[2]; size_t count=0;
    unsigned before=p->options.aliases;
    if (!sv_settings_option_apply_record(&p->options,name,value,slots,&count)) return false;
    if (before!=p->options.aliases)
        diagnose(p,SV_PREF_DIAG_ALIAS,source,line,"Converted baseline option alias in memory",false);
    /* Keep public option values as the production adapter; immediate consumers
     * observe slot/value/in_game intents without the parser invoking Term/UI. */
    for (size_t index=0;index<count;++index) {
        int i=slots[index];
        if (i==0 && !add_keymap_record(p,(SvPreferenceKeymapRecord){
                .kind=SV_PREF_KEYMAP_OPTION,.rogue=p->options.values[i]!=0})) return false;
        if (!add_effect(p,(SvPreferenceEffect){.kind=SV_PREF_OPTION,.option_slot=i,
                .option_value=p->options.values[i]!=0,.in_game=ctx->in_game,
                .generation=ctx->generation,.source=*source,.line=line},NULL,0)) return false;
    }
    return true;
}
static SvPreferenceStatus load_file(SvPreferences *,SvOptionOwner,const char *,
    SvPreferencePurpose,const SvPreferenceContext *,bool,bool);
static bool apply_record(SvPreferences *p,SvOptionOwner role,const SvFilePath *source,
    unsigned line,char *text,const SvPreferenceContext *ctx,bool custom,bool *bad_mapping)
{
    if (!*text || isspace((unsigned char)*text) || (text[0]=='#' && text[1]!=':')) return true;
    if (text[1]!=':') return false;
    char tag=*text;
    if (tag=='%') {
        bool fallback=!strcmp(text+2,"options.prf") && source->owner==SV_FILE_BUNDLED;
        SvPreferenceContext included=*ctx;
        included.subtileset=-1; /* Each source derives its own filename subset. */
        SvPreferenceStatus result=load_file(p,role,text+2,SV_PREF_INCLUDE,&included,custom,fallback);
        /* Nested loader already supplied contextual diagnostics. */
        return result!=SV_PREF_FATAL;
    }
    if (ctx->macro_only && tag!='A' && tag!='P' && tag!='H' && tag!='C' && tag!='D') return true;
    /* Baseline caps the encoded record, including tag and colon, after
     * includes/filtering. Keep the original record above and make this
     * otherwise silent effective-prefix boundary visible to callers. */
    if (strlen(text)>=SV_PREFERENCE_MACRO_MAXLEN) {
        text[SV_PREFERENCE_MACRO_MAXLEN-1]=0;
        diagnose(p,SV_PREF_DIAG_LIMIT,source,line,
            "Encoded preference record exceeds baseline prefix; applying prefix only",true);
    }
    switch (tag) {
    case 'X':case 'Y':return apply_option(p,ctx,text+2,tag=='Y',source,line);
    case 'A':case 'P':case 'H':case 'C':case 'D':return macro_store(p,ctx,tag,text+2,source,line);
    case 'R':case 'K':case 'F':case 'U':case 'I':case 'S':case 'm':
        return mapping_record(p,ctx,tag,text+2,source,line,bad_mapping);
    case 'W': { /* Recognized legacy topology, intentionally inert in SV. */
        char *fields[2]; return tokenize(text+2,2,fields)==2;
    }
    case 'E':case 'V':return true;
    case '#':case '!':case '?':return action_record(p,ctx,tag,text+2,source,line);
    default:return false;
    }
}
/* Baseline line reader keeps printable bytes, expands each tab to eight spaces,
 * supports CR, LF, CRLF, and skips other controls. Overflow is explicit rather
 * than silently applying a truncated executable record. */
static int read_line(FILE *file,char **out)
{
    size_t n=0,capacity=256;
    bool any=false,overflow=false;
    char *text=malloc(capacity);
    if (!text) return -3;
    for (;;) {
        int c=fgetc(file);
        if (c==EOF) {
            if (ferror(file)) { free(text); return -2; }
            if (!any) { free(text); return 0; }
            break;
        }
        any=true;
        if (c=='\r') { int next=fgetc(file); if (next!=EOF && next!='\n') ungetc(next,file); break; }
        if (c=='\n') break;
        unsigned count=c=='\t'?8:isprint((unsigned char)c)?1:0;
        if (n+count+1>SV_PREFERENCE_BYTE_LIMIT) { overflow=true; continue; }
        if (n+count+1>capacity) {
            size_t next_capacity=capacity*2;
            if (next_capacity>SV_PREFERENCE_BYTE_LIMIT) next_capacity=SV_PREFERENCE_BYTE_LIMIT;
            char *next=realloc(text,next_capacity);
            if (!next) { free(text); return -3; }
            text=next; capacity=next_capacity;
        }
        while (count--) text[n++]=c=='\t'?' ':(char)c;
    }
    text[n]=0;
    if (overflow) { free(text); return -1; }
    *out=text; return 1;
}
static bool confined(SvPreferences *p,const SvFilePath *source,const char *canonical)
{
    char root[SV_FILE_PATH],resolved[SV_FILE_PATH];
    const char *base=source->owner==SV_FILE_BUNDLED?p->library_root:p->user_root;
    int n=snprintf(root,sizeof(root),"%s/%s",base,source->owner==SV_FILE_PROFILE?"sv":"user");
    if (n<=0 || (size_t)n>=sizeof(root) || !realpath(root,resolved)) return false;
    size_t length=strlen(resolved);
    return !strncmp(canonical,resolved,length) && canonical[length]=='/';
}
static SvPreferenceStatus load_file_content(SvPreferences *p,SvOptionOwner role,const char *name,
    SvPreferencePurpose purpose,const SvPreferenceContext *context,bool custom,bool fallback)
{
    if (p->fatal) return SV_PREF_FATAL;
    SvPreferenceContext ctx=*context;
    if (ctx.subtileset < -1 || ctx.subtileset>=PREF_SUBSETS ||
        ctx.character_offset<0 || ctx.character_offset>=PREF_GLYPH_MAX) {
        diagnose(p,SV_PREF_DIAG_INVALID,NULL,0,"Invalid preference graphics context",true);
        return SV_PREF_PARTIAL;
    }
    unsigned issues=p->issues;
    SvFilePath source={0};
    if (name && !strcmp(name,"options.prf")) p->saw_options=true;
    if (p->depth==SV_PREFERENCE_INCLUDE_DEPTH) {
        diagnose(p,SV_PREF_DIAG_LIMIT,NULL,0,"Preference include depth exceeds limit",true);
        return SV_PREF_PARTIAL;
    }
    SvFileResult found=resolve(p,&source,&role,name,custom,fallback,purpose!=SV_PREF_EXPLICIT);
    if (found!=SV_FILE_OK) {
        if (found==SV_FILE_MISSING && purpose==SV_PREF_OPTIONAL) return SV_PREF_CLEAN;
        diagnose(p,found==SV_FILE_MISSING?SV_PREF_DIAG_MISSING:found==SV_FILE_IO?SV_PREF_DIAG_IO:SV_PREF_DIAG_INVALID,
            &source,0,found==SV_FILE_MISSING?"Could not open included/selected preference file":
            found==SV_FILE_IO?"Could not read preference file":"Invalid preference source or owner escape",true);
        return found==SV_FILE_MISSING || found==SV_FILE_IO?SV_PREF_UNAVAILABLE:SV_PREF_PARTIAL;
    }
    char canonical[SV_FILE_PATH];
    if (!realpath(source.path,canonical) || (!custom && !confined(p,&source,canonical))) {
        diagnose(p,SV_PREF_DIAG_INVALID,&source,0,"Preference source escapes its resolved owner",true);
        return SV_PREF_PARTIAL;
    }
    for (unsigned i=0;i<p->depth;++i) if (!strcmp(p->stack[i],canonical)) {
        diagnose(p,SV_PREF_DIAG_CYCLE,&source,0,"Recursive preference include cycle",true);
        return SV_PREF_PARTIAL;
    }
    strcpy(source.path,canonical);
    FILE *file=sv_file_open(&source,"r");
    if (!file) {
        diagnose(p,SV_PREF_DIAG_IO,&source,0,"Could not open preference file",true);
        return SV_PREF_UNAVAILABLE;
    }
    strcpy(p->stack[p->depth++],canonical); ++p->options.files;
    /* Filename-specific subset selection is part of ordinary/manual loading. */
    const char *base=strrchr(source.path,'/'); base=base?base+1:source.path;
    const char *hash=strchr(base,'#');
    if (!strncmp(base,"graphics-",9) && hash) {
        errno=0; long subset=strtol(hash+1,NULL,10);
        if (!errno && subset>=0 && subset<PREF_SUBSETS) ctx.subtileset=(int)subset;
    }
    bool bad_mapping=false;
    unsigned line=0;
    for (;;) {
        char *text=NULL;
        int read=read_line(file,&text);
        if (!read) break;
        ++line;
        if (read<0) {
            if (read==-3) p->fatal=true;
            diagnose(p,read==-1?SV_PREF_DIAG_LIMIT:SV_PREF_DIAG_IO,&source,line,
                read==-1?"Encoded preference line exceeds retained byte budget":
                read==-3?"Could not allocate preference line":"Could not read preference file",true);
            if (read==-2 || read==-3) break;
            continue;
        }
        if (*text && !add_record(p,role,&source,line,text)) { free(text); break; }
        if (!apply_record(p,role,&source,line,text,&ctx,custom,&bad_mapping)) {
            diagnose(p,p->fatal?SV_PREF_DIAG_LIMIT:SV_PREF_DIAG_INVALID,&source,line,
                p->fatal?"Preference retained data/effect allocation limit":"Error parsing preference record",true);
        }
        free(text);
        if (p->fatal) break;
    }
    if (bad_mapping)
        diagnose(p,SV_PREF_DIAG_MAPPING,&source,0,
            "Mapping assigns non-wall terrain to solid wall symbols (2 or 127)",false);
    fclose(file); --p->depth;
    return p->fatal?SV_PREF_FATAL:p->issues!=issues?SV_PREF_PARTIAL:SV_PREF_CLEAN;
}
static SvPreferenceResult result_since(SvPreferences *p,SvPreferenceStatus status,
                                       unsigned files,size_t diagnostics)
{
    return (SvPreferenceResult){p->fatal?SV_PREF_FATAL:status,
        p->options.files-files,(unsigned)(p->diagnostic_count-diagnostics)};
}
static SvPreferenceStatus load_file(SvPreferences *p,SvOptionOwner role,const char *name,
    SvPreferencePurpose purpose,const SvPreferenceContext *context,bool custom,bool fallback)
{
    SvPreferenceStatus status=load_file_content(p,role,name,purpose,context,custom,fallback);
    if (purpose!=SV_PREF_EXPLICIT && context->arcade_file && name &&
        !strstr(name,context->arcade_file)) {
        SvPreferenceContext tail_context=*context;
        SvPreferenceStatus tail=load_file_content(p,SV_OPTION_SHARED,context->arcade_file,
            SV_PREF_OPTIONAL,&tail_context,false,false);
        if (tail==SV_PREF_FATAL || (tail!=SV_PREF_CLEAN && status==SV_PREF_CLEAN)) status=tail;
    }
    return status;
}
SvPreferenceResult sv_preferences_load(SvPreferences *p,SvOptionOwner role,
    const char *name,SvPreferencePurpose purpose,const SvPreferenceContext *context)
{
    if (!p) return (SvPreferenceResult){SV_PREF_FATAL,0,0};
    SvPreferenceContext ctx=context?*context:sv_preferences_default_context();
    unsigned files=p->options.files; size_t diagnostics=p->diagnostic_count;
    SvPreferenceStatus status=load_file(p,role,name,purpose,&ctx,purpose==SV_PREF_EXPLICIT,false);
    return result_since(p,status,files,diagnostics);
}
static void reset_mappings(SvPreferences *p,bool graphics_only)
{
    if (!p) return;
    size_t kept=0;
    for (size_t i=0;i<p->mapping_count;++i) {
        char family=p->mappings[i].family;
        if (family!='I' && family!='m' &&
            (graphics_only || (family!='R' && family!='K' && family!='F' && family!='U'))) p->mappings[kept++]=p->mappings[i];
    }
    /* Retain capacity/accounting until the next store or destruction. Shrink
     * explicitly so mapping_count remains the allocation size invariant. */
    if (kept!=p->mapping_count) {
        if (!kept) { release(p,p->mappings,p->mapping_count*sizeof(*p->mappings)); p->mappings=NULL; }
        else {
            SvPreferenceMapping *smaller=resize(p,p->mappings,
                p->mapping_count*sizeof(*p->mappings),kept*sizeof(*p->mappings));
            if (smaller) p->mappings=smaller;
            else return;
        }
        p->mapping_count=kept;
    }
    if (!graphics_only) memset(p->glyph_subtilesets,-1,sizeof(p->glyph_subtilesets));
}
void sv_preferences_reset_visuals(SvPreferences *p){reset_mappings(p,false);}
void sv_preferences_reset_graphics(SvPreferences *p){reset_mappings(p,true);}
SvPreferenceResult sv_preferences_bootstrap(SvPreferences *p,
    const SvPreferenceContext *context,SvPreferenceResourceHook resources,void *opaque)
{
    if (!p) return (SvPreferenceResult){SV_PREF_FATAL,0,0};
    SvPreferenceContext ctx=context?*context:sv_preferences_default_context();
    unsigned files=p->options.files,issues=p->issues; size_t diagnostics=p->diagnostic_count;
    load_file(p,SV_OPTION_BUNDLED,"pref.prf",SV_PREF_OPTIONAL,&ctx,false,false);
    if (!p->saw_options) load_file(p,SV_OPTION_PROFILE,"options.prf",SV_PREF_OPTIONAL,&ctx,false,true);
    load_file(p,SV_OPTION_PROFILE,"global.opt",SV_PREF_OPTIONAL,&ctx,false,false);
    /* Preserve explicit SV overrides. Only a missing override selects the
     * shipped SDL3 defaults; invalid/unreadable sources never trigger fallback. */
    SvFilePath system={0}; SvOptionOwner role=SV_OPTION_SHARED;
    SvFileResult found=resolve(p,&system,&role,"pref-sv.prf",false,false,true);
    load_file(p,found==SV_FILE_MISSING?SV_OPTION_BUNDLED:SV_OPTION_SHARED,
        found==SV_FILE_MISSING?"pref-sdl3.prf":"pref-sv.prf",SV_PREF_OPTIONAL,&ctx,false,false);
    if (resources) {
        SvPreferenceResult resource_result=resources(p,&ctx,opaque);
        if (resource_result.status!=SV_PREF_CLEAN) {
            if (resource_result.status==SV_PREF_FATAL) p->fatal=true;
            diagnose(p,SV_PREF_DIAG_IO,NULL,0,"Resource preference stage failed",true);
        }
    }
    load_file(p,SV_OPTION_PROFILE,"global-sv.opt",SV_PREF_OPTIONAL,&ctx,false,false);
    return result_since(p,p->issues!=issues?SV_PREF_PARTIAL:SV_PREF_CLEAN,files,diagnostics);
}
SvPreferences *sv_preferences_clone(const SvPreferences *base,bool copy_effects)
{
    if (!base || base->fatal) return NULL;
    SvPreferences *p=sv_preferences_create(base->user_root,base->library_root,&base->options);
    if (!p) return NULL;
    p->saw_options=base->saw_options;
    /* Replace the create-time INITIAL snapshot with the inherited history. */
    release(p,p->keymap_records,p->keymap_count*sizeof(*p->keymap_records));
    p->keymap_records=NULL;
    p->keymap_count=0;
    for (size_t i=0;i<base->keymap_count && !p->fatal;++i)
        add_keymap_record(p,base->keymap_records[i]);
    memcpy(p->glyph_subtilesets,base->glyph_subtilesets,sizeof(p->glyph_subtilesets));
    if (base->action) { p->action=copy_bytes(p,base->action,base->action_size); p->action_size=base->action_size; }
    for (size_t i=0;i<base->record_count && !p->fatal;++i) {
        const SvPreferenceRecord *record=&base->records[i];
        /* Avoid appending inherited records twice to the compatibility view. */
        size_t before=p->options.record_bytes;
        add_record(p,record->role,&record->source,record->line,record->text);
        p->options.record_bytes=before; p->options.records[before]=0;
    }
    if (base->mapping_count) {
        p->mappings=resize(p,NULL,0,base->mapping_count*sizeof(*p->mappings));
        if (p->mappings) memcpy(p->mappings,base->mappings,base->mapping_count*sizeof(*p->mappings));
        if (p->mappings) p->mapping_count=base->mapping_count;
    }
    for (size_t i=0;i<base->macro_count && !p->fatal;++i) {
        const SvPreferenceMacro *macro=&base->macros[i];
        SvPreferenceMacro *all=resize(p,p->macros,p->macro_capacity*sizeof(*all),(p->macro_count+1)*sizeof(*all));
        if (!all) break;
        p->macros=all; p->macro_capacity=p->macro_count+1;
        SvPreferenceMacro next=*macro;
        next.trigger=copy_bytes(p,macro->trigger,macro->trigger_size);
        next.action=copy_bytes(p,macro->action,macro->action_size);
        if (!next.trigger || !next.action) { free((void *)next.trigger); free((void *)next.action); break; }
        all[p->macro_count++]=next;
    }
    if (copy_effects) {
        for (size_t i=0;i<base->effect_count && !p->fatal;++i)
            add_effect(p,base->effects[i],base->effects[i].bytes,base->effects[i].size);
        if (base->queue_size) {
            p->queue=resize(p,NULL,0,base->queue_size);
            if (p->queue) { memcpy(p->queue,base->queue,base->queue_size); p->queue_size=base->queue_size; }
        }
        for (size_t i=0;i<base->diagnostic_count;++i) {
            const SvPreferenceDiagnostic *d=&base->diagnostics[i];
            diagnose(p,d->kind,&d->source,d->line,d->message,false);
        }
        p->issues=base->issues; p->fatal|=base->fatal;
    }
    if (p->fatal) { sv_preferences_destroy(p); return NULL; }
    return p;
}
SvPreferences *sv_preferences_character(const SvPreferences *base,const char *character,
    const SvPreferenceContext *context,SvPreferenceResult *result)
{
    if (result) *result=(SvPreferenceResult){SV_PREF_FATAL,0,0};
    SvPreferences *p=sv_preferences_clone(base,false);
    if (!p) return NULL;
    char name[SV_FILE_PATH];
    if (!basename_only(character) || snprintf(name,sizeof(name),"%s.opt",character)>=(int)sizeof(name)) {
        diagnose(p,SV_PREF_DIAG_INVALID,NULL,0,"Invalid character preference name",true);
        if (result) *result=(SvPreferenceResult){SV_PREF_PARTIAL,0,1};
        return p;
    }
    SvPreferenceResult loaded=sv_preferences_load(p,SV_OPTION_PROFILE,name,SV_PREF_OPTIONAL,context);
    if (result) *result=loaded;
    return p;
}

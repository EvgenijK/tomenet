#include "config.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SDL3 client CFG spellings; source/default dispositions are recorded in C002. */
typedef enum { NUMBER, STRING, FLAG, CHOICE, SCALE } Kind;
typedef struct {
    const char *key;
    size_t offset, capacity;
    Kind kind;
    int low, high, initial;
    const char *text;
} Field;
#define N(k,f,l,h,d) {k,offsetof(SvSettingsConfig,f),0,NUMBER,l,h,d,NULL}
#define S(k,f,d) {k,offsetof(SvSettingsConfig,f),sizeof(((SvSettingsConfig *)0)->f),STRING,0,0,0,d}
#define F(k,f,d) {k,offsetof(SvSettingsConfig,f),0,FLAG,0,1,d,NULL}
#define C(k,f,d,choices) {k,offsetof(SvSettingsConfig,f),sizeof(((SvSettingsConfig *)0)->f),CHOICE,0,0,0,d "|" choices}
static const Field fields[] = {
    N("svSchemaVersion",schema_version,1,INT_MAX,1),
    S("nick",nick,"PLAYER"), S("name",name,""), S("meta",meta,"meta.tomenet.eu"),
    S("server",server,""), S("realname",realname,"PLAYER"), S("path",path,""),
    N("port",port,1,65535,18348), F("fullauto",fullauto,0), N("fps",fps,0,INT_MAX,100),
    C("svLayout",layout,"wide","small"), C("svWindowMode",window_mode,"fullscreen","window"),
    N("svWindowWidth",window_width,1024,INT_MAX,0), N("svWindowHeight",window_height,768,INT_MAX,0),
    {"svUiScalePercent",offsetof(SvSettingsConfig,ui_scale_percent),0,SCALE,50,200,100,NULL},
    S("svTextFont",text_font,"CascadiaMono-Regular.ttf"), S("svMapFont",map_font,"16x24x.pcf"),
    S("svTextProfile",text_profile,""), S("svMapProfile",map_profile,""),
    C("svGraphicsFilter",graphics_filter,"linear","nearest|pixelart"),
    C("svPcfFilter",pcf_filter,"nearest","linear|pixelart"),
    N("graphics",graphics,0,2,1), N("graphicsForceOutline",graphics_force_outline,-1,6,-1),
    N("graphic_resize_type",graphic_resize_type,0,2,1), S("graphic_tiles",graphic_tiles,"16x24sv"),
    F("lighterDarkBlue",lighter_dark_blue,1), N("windowDecorations",window_decorations,0,1,1),
    N("sound",sound,0,1,1), N("cacheAudio",cache_audio,0,1,1),
    N("audioSampleRate",audio_sample_rate,4000,48000,44100), N("audioChannels",audio_channels,4,32,32),
    N("audioBuffer",audio_buffer,128,8192,1024), S("soundpackFolder",soundpack_folder,"sound"),
    S("musicpackFolder",musicpack_folder,"music"),
    N("soundpackSubset",soundpack_subset,1,9,1), N("musicpackSubset",musicpack_subset,1,9,1),
    N("audioMaster",audio_master,0,1,1), N("audioMusic",audio_music,0,1,1),
    N("audioSound",audio_sound,0,1,1), N("audioWeather",audio_weather,0,1,1),
    N("audioVolumeMaster",volume_master,0,100,70), N("audioVolumeMusic",volume_music,0,100,70),
    N("audioVolumeSound",volume_sound,0,100,70), N("audioVolumeWeather",volume_weather,0,100,70)
};
#undef N
#undef S
#undef F
#undef C

static void field_default(SvSettingsConfig *config, const Field *field)
{
    void *out = (char *)config + field->offset;
    if (field->kind == STRING || field->kind == CHOICE) {
        size_t length = field->kind == CHOICE ? strcspn(field->text,"|") : strlen(field->text);
        memcpy(out, field->text, length);
        ((char *)out)[length] = 0;
    } else *(int *)out = field->initial;
}

void sv_settings_config_defaults(SvSettingsConfig *config)
{
    static const uint32_t palette[16] = {
        0x000000,0xffffff,0x9d9d9d,0xff8d00,0xb70000,0x009d44,0x0000ff,0x8d6600,
        0x666666,0xcdcdcd,0xaf00ff,0xffff00,0xff3030,0x00ff00,0x00ffff,0xc79d55
    };
    size_t i;
    memset(config,0,sizeof(*config));
    for (i=0;i<sizeof(fields)/sizeof(fields[0]);i++) field_default(config,&fields[i]);
    memcpy(config->palette,palette,sizeof(palette));
    config->overwrite_allowed=1;
}

void sv_settings_config_dispose(SvSettingsConfig *config)
{
    SvSettingsConfigRecord *record=config->records;
    while (record) {
        SvSettingsConfigRecord *next=record->next;
        free(record->line);
        free(record);
        record=next;
    }
    config->records=NULL;
    config->records_tail=NULL;
}

static int number(const char *value, int low, int high, int *out)
{
    char *end;
    long result;
    if (!*value) return 0;
    errno=0;
    result=strtol(value,&end,10);
    if (errno || *end || result<low || result>high) return 0;
    *out=(int)result;
    return 1;
}

bool sv_settings_config_server(SvSettingsConfig *config, const char *value)
{
    size_t length;
    const char *colon;
    int port=config->port;
    if (!value || !(length=strlen(value)) || length>=sizeof(config->server) ||
        strpbrk(value," \t\r\n")) return false;
    colon=strchr(value,':');
    if (colon && !strchr(colon+1,':')) {
        if (colon==value || !number(colon+1,1,65535,&port)) return false;
        length=(size_t)(colon-value);
    }
    memcpy(config->server,value,length);
    config->server[length]=0;
    config->port=port;
    return true;
}

static int choice(const char *choices, const char *value)
{
    size_t n=strlen(value);
    while (*choices) {
        size_t length=strcspn(choices,"|");
        if (length==n && !memcmp(choices,value,n)) return 1;
        choices+=length;
        if (*choices) choices++;
    }
    return 0;
}

static int retain(SvSettingsConfig *config, const char *raw, size_t length, int inert)
{
    SvSettingsConfigRecord *record=malloc(sizeof(*record));
    if (!record) return 0;
    record->line=malloc(length+1);
    if (!record->line) { free(record); return 0; }
    memcpy(record->line,raw,length);
    record->line[length]=0;
    record->next=NULL;
    record->inert=inert;
    if (config->records_tail) config->records_tail->next=record;
    else config->records=record;
    config->records_tail=record;
    if (inert) { config->unsupported_records++; config->diagnostics++; }
    return 1;
}

static int inert_key(const char *key)
{
    static const char *prefixes[]={"Term-Main","Term-1","Term-2","Term-3","Term-4","Term-5","Term-6","Term-7","Term-8","Term-9",
        "Mainwindow","Mirrorwindow","Recallwindow","Choicewindow","Term-4window","Term-5window","Term-6window","Term-7window","Term-8window","Term-9window"};
    static const char *suffixes[]={"_Title","_Visible","_X","_Y","_Columns","_Lines","_Font"};
    size_t i,j;
    for (i=0;i<sizeof(prefixes)/sizeof(prefixes[0]);i++) {
        size_t n=strlen(prefixes[i]);
        if (!strncmp(key,prefixes[i],n))
            for (j=0;j<sizeof(suffixes)/sizeof(suffixes[0]);j++)
                if (!strcmp(key+n,suffixes[j])) return 1;
    }
    /* No approved HTML metadata/bounds yet: keep requested records inert. */
    return !strcmp(key,"disableGfxCache") || !strcmp(key,"hintBigmap") || !strcmp(key,"hintSound") ||
        !strcmp(key,"svPanelLeftWidth") || !strcmp(key,"svPanelRightWidth") ||
        !strncmp(key,"svBlockVisible_",15) || !strncmp(key,"svWidgetOrder_",14);
}

static int parse_line(SvSettingsConfig *config, char *line, size_t length, size_t line_number)
{
    char *key=line,*value,*end;
    const Field *field=NULL;
    int malformed=memchr(line,0,length)!=NULL || memchr(line,'\r',length)!=NULL;
    size_t i;
    while (*key==' ' || *key=='\t') key++;
    /* Comments have no active meaning, and must not retain commented passwords. */
    if (!*key || *key=='#') return 1;
    end=key;
    while (*end && *end!=' ' && *end!='\t') end++;
    value=end;
    if (*end) {
        *end++=0;
        while (*end==' ' || *end=='\t') end++;
        value=end;
    }
    end=value+strlen(value);
    while (end>value && (end[-1]==' ' || end[-1]=='\t')) *--end=0;
    if (!strcmp(key,"pass")) return 1;
    for (i=0;i<sizeof(fields)/sizeof(fields[0]);i++)
        if (!strcmp(key,fields[i].key)) { field=&fields[i]; break; }
    if (field) {
        void *out=(char *)config+field->offset;
        int valid=!malformed;
        if (field->kind==STRING || field->kind==CHOICE) {
            valid=valid && strlen(value)<field->capacity &&
                (field->kind!=CHOICE || choice(field->text,value));
            if (valid && !strcmp(field->key,"server")) valid=sv_settings_config_server(config,value);
            else if (valid) strcpy(out,value);
        } else if (field->kind==FLAG) {
            /* Existing standalone flags are enabled by presence, including "flag 0". */
            if (valid) *(int *)out=1;
        } else {
            int parsed;
            valid=valid && number(value,field->low,field->high,&parsed);
            if (valid && field->kind==SCALE && parsed%5) valid=0;
            if (valid) *(int *)out=parsed;
        }
        if (!valid) {
            field_default(config,field);
            config->diagnostics++;
            fprintf(stderr,"SV settings: invalid %s at CFG line %zu; using default\n",field->key,line_number);
        }
        return 1;
    }
    if (!strncmp(key,"graphic_tiles",13) && strlen(key)==14 && key[13]>='0' && key[13]<='9') {
        int n=key[13]-'0', parsed;
        if (!malformed && number(value,0,1,&parsed)) config->graphic_subsets[n]=parsed;
        else {
            config->graphic_subsets[n]=0;
            config->diagnostics++;
            fprintf(stderr,"SV settings: invalid graphic_tiles%d at CFG line %zu; using default\n",n,line_number);
        }
        return 1;
    }
    if (!strncmp(key,"colormap_",9)) {
        int index;
        char canonical[16];
        if (number(key+9,0,15,&index) &&
            snprintf(canonical,sizeof(canonical),"colormap_%d",index)>0 && !strcmp(canonical,key)) {
            const char *hex=value;
            char *tail;
            unsigned long color;
            size_t hex_digits;
            if (*hex=='#') hex++;
            hex_digits=strspn(hex,"0123456789abcdefABCDEF");
            errno=0;
            color=strtoul(hex,&tail,16);
            if (!malformed && strlen(hex)==6 && hex_digits==6 && !*tail && !errno && color<=0xffffff)
                config->palette[index]=(uint32_t)color;
            else {
                SvSettingsConfig defaults;
                sv_settings_config_defaults(&defaults);
                config->palette[index]=defaults.palette[index];
                config->diagnostics++;
                fprintf(stderr,"SV settings: invalid colormap_%d at CFG line %zu; using default\n",index,line_number);
            }
            return 1;
        }
    }
    if (malformed) {
        config->diagnostics++;
        fprintf(stderr,"SV settings: malformed scalar at CFG line %zu; skipped\n",line_number);
        return 1;
    }
    /* Restore the raw delimiter before retaining the compatible record. */
    return 2;
}

SvSettingsConfigResult sv_settings_config_overlay_parse(SvSettingsConfig *config, const void *bytes, size_t size)
{
    const char *input=bytes;
    size_t pos=0,line_number=0;
    if (!bytes && size) {
        config->diagnostics++;
        fprintf(stderr,"SV settings: invalid CFG input buffer; retaining current settings/defaults\n");
        return SV_SETTINGS_CONFIG_INVALID;
    }
    while (pos<size) {
        size_t start=pos,length;
        char *line;
        line_number++;
        int result;
        while (pos<size && input[pos]!='\n') pos++;
        length=pos-start;
        if (pos<size) pos++;
        if (length && input[start+length-1]=='\r') length--;
        if (length>65536) {
            config->diagnostics++;
            fprintf(stderr,"SV settings: oversized CFG line %zu; skipped\n",line_number);
            continue;
        }
        line=malloc(length+1);
        if (!line) return SV_SETTINGS_CONFIG_MEMORY;
        memcpy(line,input+start,length);
        line[length]=0;
        result=parse_line(config,line,length,line_number);
        if (result==2) {
            /* parse_line has tokenized its private copy; raw bytes remain untouched. */
            int inert=inert_key(line+strspn(line," \t"));
            if (!retain(config,input+start,length,inert)) { free(line); return SV_SETTINGS_CONFIG_MEMORY; }
        }
        free(line);
    }
    if (config->schema_version>1) {
        unsigned diagnostics=config->diagnostics+1;
        sv_settings_config_dispose(config);
        sv_settings_config_defaults(config);
        config->overwrite_allowed=0;
        config->diagnostics=diagnostics;
        fprintf(stderr,"SV settings: incompatible future CFG schema; using defaults, overwrite prohibited\n");
        return SV_SETTINGS_CONFIG_FUTURE_SCHEMA;
    }
    return config->diagnostics>config->unsupported_records ? SV_SETTINGS_CONFIG_INVALID : SV_SETTINGS_CONFIG_OK;
}

SvSettingsConfigResult sv_settings_config_overlay_load(SvSettingsConfig *config, const char *resolved_path)
{
    FILE *file;
    char *buffer;
    size_t used;
    SvSettingsConfigResult result;
    file=fopen(resolved_path,"rb");
    if (!file) {
        if (errno==ENOENT) return SV_SETTINGS_CONFIG_MISSING;
        config->diagnostics++;
        fprintf(stderr,"SV settings: cannot open CFG %s; retaining current settings/defaults\n",resolved_path);
        return SV_SETTINGS_CONFIG_IO;
    }
    buffer=malloc(4*1024*1024+1);
    if (!buffer) { fclose(file); return SV_SETTINGS_CONFIG_MEMORY; }
    used=fread(buffer,1,4*1024*1024+1,file);
    if (ferror(file) || used>4*1024*1024) {
        config->diagnostics++;
        fprintf(stderr,"SV settings: cannot read CFG %s (read error or size limit); retaining current settings/defaults\n",resolved_path);
        result=SV_SETTINGS_CONFIG_IO;
    } else result=sv_settings_config_overlay_parse(config,buffer,used);
    free(buffer);
    if (fclose(file)!=0 && result==SV_SETTINGS_CONFIG_OK) {
        config->diagnostics++;
        fprintf(stderr,"SV settings: cannot close CFG %s after reading; settings may be incomplete\n",resolved_path);
        result=SV_SETTINGS_CONFIG_IO;
    }
    return result;
}

SvSettingsConfigResult sv_settings_config_parse(SvSettingsConfig *config, const void *bytes, size_t size)
{
    sv_settings_config_dispose(config);
    sv_settings_config_defaults(config);
    return sv_settings_config_overlay_parse(config,bytes,size);
}

SvSettingsConfigResult sv_settings_config_load(SvSettingsConfig *config, const char *resolved_path)
{
    sv_settings_config_dispose(config);
    sv_settings_config_defaults(config);
    return sv_settings_config_overlay_load(config,resolved_path);
}

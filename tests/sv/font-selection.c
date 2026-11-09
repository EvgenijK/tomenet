#define _POSIX_C_SOURCE 200809L
#include "ui/font-selection.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static unsigned map_changes,text_changes;
static bool apply(void *context,SvFontRole role,SvFont *font)
{
    (void)context;
    assert(font);
    if (role==SV_FONT_MAP) map_changes++;
    else text_changes++;
    return true;
}
static void external(const char *path,const char *bytes)
{
    FILE *file=fopen(path,"wb");
    assert(file && fwrite(bytes,1,strlen(bytes),file)==strlen(bytes) && !fclose(file));
}
int main(int argc,char **argv)
{
    const char *library=argc>1?argv[1]:"lib";
    char root[]="/tmp/sv-c006-selection-XXXXXX",cfg[4096],backup[4096],directory[4096];
    assert(mkdtemp(root));
    snprintf(cfg,sizeof(cfg),"%s/sv/tomenet.cfg",root);
    snprintf(backup,sizeof(backup),"%s/backup",root);
    snprintf(directory,sizeof(directory),"%s/sv",root);
    assert(!mkdir(directory,0700));
    external(cfg,"svTextFont missing.ttf\nfutureKey untouched\nfps 75\n");
    assert(SDL_Init(0) && TTF_Init());
    SvSettingsConfig config;
    sv_settings_config_defaults(&config);
    strcpy(config.text_font,"missing.ttf");
    SvFontSelection *s=sv_font_selection_create(root,library,&config);
    assert(s && sv_font_selection_fallback(s,SV_FONT_TEXT));
    assert(!strcmp(sv_font_selection_requested(s,SV_FONT_TEXT),"missing.ttf"));
    SvFont *map=sv_font_selection_font(s,SV_FONT_MAP),*text=sv_font_selection_font(s,SV_FONT_TEXT);
    sv_font_selection_prepare(s,apply,NULL);
    assert(sv_font_selection_preview(s,SV_FONT_TEXT,"9x15tg.pcf","")==SV_FONT_SELECTION_OK);
    assert(sv_font_selection_dirty(s,SV_FONT_TEXT) && !map_changes && text_changes==1);
    assert(sv_font_selection_font(s,SV_FONT_MAP)==map);
    assert(!access(cfg,F_OK));
    SvFont *preview=sv_font_selection_font(s,SV_FONT_TEXT);
    assert(sv_font_selection_preview(s,SV_FONT_TEXT,"absent.pcf","")!=SV_FONT_SELECTION_OK);
    assert(sv_font_selection_font(s,SV_FONT_TEXT)==preview);
    assert(sv_font_selection_cancel(s,SV_FONT_TEXT)==SV_FONT_SELECTION_OK);
    assert(sv_font_selection_font(s,SV_FONT_TEXT)==text && !sv_font_selection_dirty(s,SV_FONT_TEXT));
    assert(sv_font_selection_fallback(s,SV_FONT_TEXT) && !map_changes);
    /* Disk baseline is independent of the effective bundled startup fallback. */
    external(cfg,"svTextFont missing.ttf\nfutureKey untouched\nfps 75\n");
    assert(sv_font_selection_preview(s,SV_FONT_TEXT,"9x15tg.pcf","")==SV_FONT_SELECTION_OK);
    assert(sv_font_selection_save(s,SV_FONT_TEXT)==SV_FONT_SELECTION_OK);
    assert(!sv_font_selection_dirty(s,SV_FONT_TEXT));
    assert(sv_settings_config_load(&config,cfg)==SV_SETTINGS_CONFIG_OK);
    assert(!strcmp(config.text_font,"9x15tg.pcf") && config.fps==75 && config.records);
    assert(!strcmp(config.records->line,"futureKey untouched"));
    external(cfg,"svTextFont 9x15tg.pcf\nfps 80\nfutureKey external\n");
    assert(sv_font_selection_preview(s,SV_FONT_TEXT,"12x24.pcf","")==SV_FONT_SELECTION_OK);
    assert(sv_font_selection_save(s,SV_FONT_TEXT)==SV_FONT_SELECTION_OK);
    assert(sv_settings_config_load(&config,cfg)==SV_SETTINGS_CONFIG_OK && config.fps==80);
    assert(!strcmp(config.records->line,"futureKey external"));
    external(cfg,"svTextFont 8x16.pcf\nfps 81\n");
    assert(sv_font_selection_preview(s,SV_FONT_TEXT,"9x15tg.pcf","")==SV_FONT_SELECTION_OK);
    preview=sv_font_selection_font(s,SV_FONT_TEXT);
    assert(sv_font_selection_save(s,SV_FONT_TEXT)==SV_FONT_SELECTION_CONFLICT);
    assert(sv_font_selection_dirty(s,SV_FONT_TEXT) && sv_font_selection_font(s,SV_FONT_TEXT)==preview);
    assert(sv_settings_config_load(&config,cfg)==SV_SETTINGS_CONFIG_OK && !strcmp(config.text_font,"8x16.pcf"));
    assert(!rename(cfg,backup) && !mkdir(cfg,0700));
    assert(sv_font_selection_save(s,SV_FONT_TEXT)!=SV_FONT_SELECTION_OK);
    assert(sv_font_selection_dirty(s,SV_FONT_TEXT) && sv_font_selection_font(s,SV_FONT_TEXT)==preview);
    assert(!rmdir(cfg) && !rename(backup,cfg));
    assert(!map_changes);
    sv_font_selection_destroy(s);
    sv_settings_config_dispose(&config);
    /* Explicit Save also persists CLI choices when no CFG existed at opening. */
    assert(!unlink(cfg));
    sv_settings_config_defaults(&config);
    strcpy(config.text_font,"12x24.pcf");
    s=sv_font_selection_create(root,library,&config);
    assert(s && sv_font_selection_save(s,SV_FONT_TEXT)==SV_FONT_SELECTION_OK);
    assert(sv_settings_config_load(&config,cfg)==SV_SETTINGS_CONFIG_OK && !strcmp(config.text_font,"12x24.pcf"));
    sv_font_selection_destroy(s);sv_settings_config_dispose(&config);
    sv_settings_config_defaults(&config);
    strcpy(config.text_font,"12x24");strcpy(config.map_font,"absent.pcf");
    s=sv_font_selection_create(root,library,&config);
    assert(s && sv_font_selection_fallback(s,SV_FONT_MAP));
    assert(strstr(sv_font_resource(sv_font_selection_font(s,SV_FONT_TEXT)),"12x24.pcf"));
    assert(!strcmp(sv_font_selection_requested(s,SV_FONT_TEXT),"12x24"));
    sv_font_selection_destroy(s);
    assert(!sv_font_selection_create(root,"/sv-c006-unavailable-library",&config));
    sv_settings_config_dispose(&config);
    assert(!unlink(cfg) && !rmdir(directory) && !rmdir(root));
    TTF_Quit();SDL_Quit();
    puts("SV font selection/preview/cancel/Save production core: PASS");
    return 0;
}

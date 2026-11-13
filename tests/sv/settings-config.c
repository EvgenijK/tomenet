#define _POSIX_C_SOURCE 200809L
#include "settings/config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static SvSettingsConfigResult parse(SvSettingsConfig *config, const char *text)
{
    return sv_settings_config_parse(config,text,strlen(text));
}

int main(void)
{
    SvSettingsConfig config;
    char root[]="/tmp/sv-c002-cfg-XXXXXX",path[256];
    struct stat status;
    FILE *file;
    const char cfg[]="audioChannels\t4\r\naudioBuffer 128\r\naudioSampleRate 4000\n"
        "sound 0\nsoundpackSubset 9\nmusicpackSubset 1\nfullauto\nlighterDarkBlue 0\n"
        "svLayout small\nsvWindowMode window\nsvUiScalePercent 125\n"
        "svTextFont Custom Font.ttf\nsvMapProfile private.profile\n"
        "graphic_tiles9 1\ngraphics 2\ncolormap_6 #0033ff\n"
        "audioVolumeMusic 0\naudioVolumeMusic 100\nfutureKey exact bytes\n"
        "Term-Main_Font 8x13\ndisableGfxCache 1\npass secret\n#pass hidden\n";
    sv_settings_config_defaults(&config);
    assert(config.port==18348 && config.fps==100 && config.graphics==1);
    assert(config.sound && config.audio_sample_rate==44100 && config.audio_channels==32 && config.audio_buffer==1024);
    assert(config.volume_master==70 && config.volume_music==70 && config.volume_sound==70 && config.volume_weather==70);
    assert(config.audio_master && config.audio_music && config.audio_sound && config.audio_weather);
    assert(!strcmp(config.layout,"wide") && !strcmp(config.window_mode,"fullscreen"));
    assert(!strcmp(config.text_font,"CascadiaMono-Regular.ttf") && !strcmp(config.map_font,"16x24x.pcf"));
    assert(!strcmp(config.graphics_filter,"linear") && !strcmp(config.pcf_filter,"nearest"));
    assert(config.ui_scale_percent==100 && config.window_width==0 && config.window_height==0);
    assert(mkdtemp(root));
    snprintf(path,sizeof(path),"%s/tomenet.cfg",root);
    assert(sv_settings_config_load(&config,path)==SV_SETTINGS_CONFIG_MISSING);
    assert(stat(path,&status)<0 && config.overwrite_allowed);
    assert(parse(&config,cfg)==SV_SETTINGS_CONFIG_OK);
    assert(config.audio_channels==4 && config.audio_buffer==128 && config.audio_sample_rate==4000);
    assert(config.sound==0 && config.soundpack_subset==9 && config.musicpack_subset==1);
    assert(config.fullauto && config.lighter_dark_blue && config.graphics==2 && config.graphic_subsets[9]);
    assert(config.palette[6]==0x0033ff && config.volume_music==100);
    assert(config.ui_scale_percent==125 && !strcmp(config.text_font,"Custom Font.ttf"));
    assert(config.unsupported_records==2 && config.diagnostics==2);
    assert(config.records && !strcmp(config.records->line,"futureKey exact bytes"));
    assert(config.records->next && config.records->next->inert);
    assert(config.records->next->next && !config.records->next->next->next);
    assert(parse(&config,"audioChannels 32\naudioChannels 3\naudioBuffer 8193\naudioSampleRate 48001\n"
        "svUiScalePercent 127\nsvLayout massive\nsoundpackSubset 0\ngraphics 3\nport 65536\n"
        "audioVolumeMaster 101\ngraphic_tiles0 2\ncolormap_6 #oops\n")==SV_SETTINGS_CONFIG_INVALID);
    assert(config.audio_channels==32 && config.audio_buffer==1024 && config.audio_sample_rate==44100);
    assert(config.ui_scale_percent==100 && !strcmp(config.layout,"wide") && config.graphics==1);
    assert(config.port==18348 && config.soundpack_subset==1 && config.volume_master==70);
    assert(config.palette[6]==0x0000ff);
    assert(parse(&config,"sound 0\nsoundpackSubset 2\nsoundpackSubsetExtra 1\n")==SV_SETTINGS_CONFIG_OK);
    assert(!config.sound && config.soundpack_subset==2 && config.records);
    assert(parse(&config,"server first.example:21000\nport 22000\n")==SV_SETTINGS_CONFIG_OK);
    assert(!strcmp(config.server,"first.example") && config.port==22000);
    assert(parse(&config,"port 22000\nserver second.example:21000\n")==SV_SETTINGS_CONFIG_OK);
    assert(!strcmp(config.server,"second.example") && config.port==21000);
    assert(parse(&config,"port 22000\nserver bad.example:bad\n")==SV_SETTINGS_CONFIG_INVALID);
    assert(!config.server[0] && config.port==22000);
    assert(sv_settings_config_server(&config,"2001:db8::1"));
    assert(!strcmp(config.server,"2001:db8::1") && config.port==22000);
    assert(!sv_settings_config_server(&config,"host:65536"));
    assert(!strcmp(config.server,"2001:db8::1") && config.port==22000);
    {
        const char nul[]="audioBuffer 128\0junk\n";
        assert(sv_settings_config_parse(&config,nul,sizeof(nul)-1)==SV_SETTINGS_CONFIG_INVALID);
        assert(config.audio_buffer==1024 && !config.records);
    }
    assert(parse(&config,"svSchemaVersion 2\nsound 0\nfutureKey value\n")==SV_SETTINGS_CONFIG_FUTURE_SCHEMA);
    assert(config.sound && !config.records && !config.overwrite_allowed);
    assert(parse(&config,"svSchemaVersion 1\nsound 0\n")==SV_SETTINGS_CONFIG_OK);
    assert(config.overwrite_allowed && !config.sound);
    assert(sv_settings_config_overlay_parse(&config,"port 20000\n",11)==SV_SETTINGS_CONFIG_OK);
    assert(!config.sound && config.port==20000);
    file=fopen(path,"wb");
    assert(file && fwrite(cfg,1,sizeof(cfg)-1,file)==sizeof(cfg)-1 && !fclose(file));
    assert(sv_settings_config_load(&config,path)==SV_SETTINGS_CONFIG_OK);
    file=fopen(path,"rb");
    assert(file);
    {
        char actual[sizeof(cfg)];
        assert(fread(actual,1,sizeof(actual),file)==sizeof(cfg)-1);
        assert(!memcmp(actual,cfg,sizeof(cfg)-1));
    }
    assert(!fclose(file));
    assert(sv_settings_config_load(&config,root)==SV_SETTINGS_CONFIG_IO);
    assert(config.sound && config.diagnostics);
    assert(!unlink(path) && !rmdir(root));
    sv_settings_config_dispose(&config);
    puts("SV settings CFG core: PASS");
    return 0;
}

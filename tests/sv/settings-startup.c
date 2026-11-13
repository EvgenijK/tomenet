#define _POSIX_C_SOURCE 200809L
#include "settings/startup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    SvSettingsConfig config;
    SvSettingsInvocation invocation;
    char root[]="/tmp/sv-c002-startup-XXXXXX",directory[256],path[256],alternate[256],file_arg[260];
    const char original[]="port 19000\nfps 80\nsound 1\nsvLayout small\n";
    const char extra[]="server file.example:20100\nport 20000\nfps 90\n";
    struct stat status;
    FILE *file;
    assert(mkdtemp(root));
    snprintf(directory,sizeof(directory),"%s/sv",root);
    snprintf(path,sizeof(path),"%s/sv/tomenet.cfg",root);
    snprintf(alternate,sizeof(alternate),"%s/extra.cfg",root);
    snprintf(file_arg,sizeof(file_arg),"-f%s",alternate);
    sv_settings_config_defaults(&config);
    assert(sv_settings_startup_load(&config,root));
    assert(config.port==18348 && stat(directory,&status)<0);
    assert(!mkdir(directory,0700));
    file=fopen(path,"wb");
    assert(file && fwrite(original,1,sizeof(original)-1,file)==sizeof(original)-1 && !fclose(file));
    file=fopen(alternate,"wb");
    assert(file && fwrite(extra,1,sizeof(extra)-1,file)==sizeof(extra)-1 && !fclose(file));
    assert(sv_settings_startup_load(&config,root));
    assert(config.port==19000 && config.fps==80 && !strcmp(config.layout,"small"));
    {
        char *args[]={"sv","-p21000","-F70",file_arg,"-q"};
        assert(sv_settings_startup_arguments(&config,&invocation,5,args));
        assert(config.port==20000 && config.fps==90 && config.sound && invocation.quiet);
        assert(!strcmp(config.layout,"small"));
    }
    {
        char *args[]={"sv",file_arg,"-p21000","-F70"};
        assert(sv_settings_startup_arguments(&config,&invocation,4,args));
        assert(config.port==21000 && config.fps==70 && !invocation.quiet);
    }
    {
        char *args[]={"sv","-nLocalName","-q","-Pcustom-library",file_arg,"-i","-p22000"};
        assert(sv_settings_startup_arguments(&config,&invocation,7,args));
        assert(config.port==22000 && config.fps==100 && !strcmp(config.layout,"wide"));
        assert(!strcmp(config.realname,"LocalName") && !strcmp(invocation.library_override,"custom-library"));
        assert(invocation.reset && !invocation.quiet);
    }
    {
        char *args[]={"sv","-i",file_arg,"--server","saved.example:22001","--account","Account"};
        assert(sv_settings_startup_arguments(&config,&invocation,7,args));
        assert(config.port==22001 && config.fps==90);
        assert(!strcmp(config.server,"saved.example") && !strcmp(config.nick,"Account"));
    }
    {
        char *args[]={"sv","inline.example:23000","-p24000"};
        assert(sv_settings_startup_arguments(&config,&invocation,3,args));
        assert(!strcmp(config.server,"inline.example") && config.port==24000);
        char *reverse[]={"sv","--port","24000","--server","inline.example:23000"};
        assert(sv_settings_startup_arguments(&config,&invocation,5,reverse));
        assert(!strcmp(config.server,"inline.example") && config.port==23000);
    }
    {
        char *args[]={"sv","-Fbad"};
        assert(!sv_settings_startup_arguments(&config,&invocation,2,args));
    }
    {
        char *args[]={"sv","-lAccount","session-secret","login.example:23001","-RHero","-C","-w","-u","-X","-s"};
        assert(sv_settings_startup_arguments(&config,&invocation,10,args));
        assert(!strcmp(config.nick,"Account") && !strcmp(config.server,"login.example") && config.port==23001);
        assert(!strcmp(config.name,"Hero") && invocation.auto_reincarnation);
        assert(invocation.password_present && !strcmp(invocation.password,"session-secret"));
        assert(invocation.protocol_override==1 && invocation.no_weather && invocation.no_lua_updates);
        assert(invocation.quit_no_prompt && invocation.ignore_scaling);
        sv_settings_invocation_clear_password(&invocation);
        assert(!invocation.password_present);
        for (size_t i=0;i<sizeof(invocation.password);++i) assert(!invocation.password[i]);
        char *clear_name[]={"sv","-N"};
        assert(sv_settings_startup_arguments(&config,&invocation,2,clear_name));
        assert(!*config.name);
    }
    {
        char *args[]={"sv","-l","Spaced","private","-v","-V","-x","-e","-t","-T","-c"};
        assert(sv_settings_startup_arguments(&config,&invocation,11,args));
        assert(!strcmp(config.nick,"Spaced") && invocation.password_present);
        assert(invocation.chat_log_mode==3 && invocation.redirect_log && invocation.disable_tileset_cache);
        assert(invocation.disable_tile_cache && invocation.force_cui);
        sv_settings_invocation_clear_password(&invocation);
    }
    {
        char *args[]={"sv","-lAccount","private","-i"};
        assert(sv_settings_startup_arguments(&config,&invocation,4,args));
        assert(!*config.nick && !invocation.password_present && !*invocation.password);
        char *missing[]={"sv","-l"};
        assert(!sv_settings_startup_arguments(&config,&invocation,2,missing) && invocation.help);
        char *conflicting[]={"sv","-lAccount","private","--password-stdin"};
        assert(!sv_settings_startup_arguments(&config,&invocation,4,conflicting));
        assert(!invocation.password_present && !*invocation.password);
        char *stdin_only[]={"sv","--account","Account","--password-stdin"};
        assert(sv_settings_startup_arguments(&config,&invocation,4,stdin_only));
        assert(invocation.password_stdin && !invocation.password_present);
        char *reset_stdin[]={"sv","--password-stdin","-i","--account","Later"};
        assert(sv_settings_startup_arguments(&config,&invocation,5,reset_stdin));
        assert(!invocation.password_stdin && !invocation.password_present);
    }
    {
        const char future[]="svSchemaVersion 2\n";
        assert(sv_settings_config_parse(&config,future,sizeof(future)-1)==SV_SETTINGS_CONFIG_FUTURE_SCHEMA);
        char *args[]={"sv","-i"};
        assert(sv_settings_startup_arguments(&config,&invocation,2,args));
        assert(config.port==18348 && !config.overwrite_allowed);
    }
    /* Startup and CLI reductions leave both owned CFG and explicit -f source byte-identical. */
    file=fopen(path,"rb");
    assert(file);
    {
        char actual[sizeof(original)];
        assert(fread(actual,1,sizeof(actual),file)==sizeof(original)-1);
        assert(!memcmp(actual,original,sizeof(original)-1));
    }
    assert(!fclose(file));
    file=fopen(alternate,"rb");
    assert(file);
    {
        char actual[sizeof(extra)];
        assert(fread(actual,1,sizeof(actual),file)==sizeof(extra)-1);
        assert(!memcmp(actual,extra,sizeof(extra)-1));
    }
    assert(!fclose(file));
    assert(!unlink(path) && !unlink(alternate) && !rmdir(directory) && !rmdir(root));
    sv_settings_config_dispose(&config);
    puts("SV settings startup core: PASS");
    return 0;
}

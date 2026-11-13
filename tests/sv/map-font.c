#define _XOPEN_SOURCE 700
#include "settings/map-font.h"
#include "protocol/login.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void write_file(const char *root, const char *name, const char *text)
{
    char path[1024];
    assert(snprintf(path,sizeof(path),"%s/%s",root,name)<(int)sizeof(path));
    FILE *file=fopen(path,"w");assert(file);
    assert(fputs(text,file)>=0 && fclose(file)==0);
}
static void remove_file(const char *root,const char *name)
{
    char path[1024];assert(snprintf(path,sizeof(path),"%s/%s",root,name)<(int)sizeof(path));
    assert(unlink(path)==0);
}
static bool mapping(const SvPreferences *p,char family,unsigned index)
{
    for(size_t i=0;i<sv_preferences_mapping_count(p);++i) {
        const SvPreferenceMapping *m=sv_preferences_mapping(p,i);
        if(m->family==family && m->index==index)return true;
    }
    return false;
}
static void live_wire(const SvVisualDefinitions *definitions)
{
    const int version[6]={4,9,4,0,0,0}, legacy[6]={4,6,1,1,0,0};
    const int play_gate[6]={4,9,1,2,0,3};
    unsigned char encoded[20000],bytes[32768];
    size_t size=sv_visual_encode(definitions,version,false,encoded);
    assert(size==(127+512+1280+1280)*5u);
    assert(encoded[127*5+3*5]==9 && encoded[127*5+3*5+4]==81);
    assert(sv_visual_encode(definitions,legacy,true,bytes)==(127+256+1024+1152)*2u);
    assert(sv_visual_encode(definitions,play_gate,true,bytes)==(127+512+1280+1280)*5u);
    assert(sv_visual_encode(definitions,play_gate,false,bytes)==(127+256+1280+1280)*5u);
    const unsigned char address[6]={0};
    SvLogin *login=sv_login_create(version,address);assert(login);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_OK);
    assert(sv_login_set_visuals(login,definitions,"fixture.pcf")==SV_OK);
    assert(sv_login_update_visuals(login,definitions,"fixture.pcf")==SV_INVALID);
    unsigned char overview[29]={PKT_SERVERDETAILS};overview[17]=PKT_LOGIN;
    assert(sv_login_receive(login,overview,sizeof(overview))==SV_OK);
    assert(sv_login_create_character(login,"Mapfont",false)==SV_OK);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_OK);
    unsigned char accepted=E_NEED_INFO;
    assert(sv_login_receive(login,&accepted,1)==SV_OK);
    int16_t stats[6]={0};
    assert(sv_login_start_play(login,0,0,0,0,stats,"fixture.pcf")==SV_OK);
    SvOutput output=sv_login_take_output(login,bytes,sizeof(bytes));
    assert(output.result==SV_OK && output.size>=size);
    /* The real PLAY serializer ends in the effective visual tables. */
    assert(!memcmp(bytes+output.size-size,encoded,size));
    const unsigned char reply[]={PKT_REPLY,PKT_PLAY,PKT_SUCCESS};
    assert(sv_login_receive(login,reply,sizeof(reply))==SV_OK);
    assert(sv_login_update_visuals(login,definitions,"replacement.pcf")==SV_OK);
    output=sv_login_take_output(login,bytes,sizeof(bytes));
    assert(output.result==SV_OK && output.size>size+1);
    assert(bytes[0]==PKT_CLIENT_SETUP && !memcmp(bytes+1,encoded,size));
    assert(bytes[size+1]==PKT_FONT);
    bool found=false;
    for(size_t i=size+2;i+sizeof("replacement.pcf")<=output.size;++i)
        if(!memcmp(bytes+i,"replacement.pcf",sizeof("replacement.pcf")))found=true;
    assert(found);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_WAITING);
    sv_login_destroy(login);
}
int main(void)
{
    char root[]="/tmp/sv-map-font-XXXXXX";assert(mkdtemp(root));
    char user[1024],library[1024],path[1024];
    assert(snprintf(user,sizeof(user),"%s/U",root)<(int)sizeof(user));
    assert(snprintf(library,sizeof(library),"%s/B",root)<(int)sizeof(library));
    assert(!mkdir(user,0700) && !mkdir(library,0700));
    assert(snprintf(path,sizeof(path),"%s/user",user)<(int)sizeof(path));assert(!mkdir(path,0700));
    assert(snprintf(path,sizeof(path),"%s/user",library)<(int)sizeof(path));assert(!mkdir(path,0700));
    write_file(user,"user/initial.prf","F:1:2:65\nR:2:3:66\nK:3:4:67\nU:4:5:68\nS:1:2:3\nA:xy\nP:q\n#:old effect\n");
    write_file(user,"user/next-macro.prf","P:r\n");
    SvSettingsOptions options;sv_settings_options_defaults(&options);
    assert(sv_settings_option_set(&options,"rogue_like_commands",true));
    SvPreferences *base=sv_preferences_create(user,library,&options);assert(base);
    SvPreferenceContext ctx=sv_preferences_default_context();ctx.subtileset=3;ctx.generation=17;
    assert(sv_preferences_load(base,SV_OPTION_SHARED,"initial.prf",SV_PREF_EXPLICIT,&ctx).status==SV_PREF_CLEAN);
    assert(sv_preferences_glyph_subtileset(base,65)==3);
    assert(sv_preferences_effect_count(base)==1);
    SvPreferences *reset=sv_preferences_clone(base,true);assert(reset);
    sv_preferences_reset_visuals(reset);
    assert(sv_preferences_mapping_count(reset)==1 && mapping(reset,'S',1));
    assert(!mapping(reset,'F',1) && !mapping(reset,'R',2) && !mapping(reset,'K',3) && !mapping(reset,'U',4));
    assert(sv_preferences_glyph_subtileset(reset,65)==-1);
    assert(sv_preferences_macro_count(reset)==1 && sv_preferences_effect_count(reset)==1);
    assert(sv_preferences_options(reset)->values[sv_settings_option_slot("rogue_like_commands")]);
    assert(sv_preferences_load(reset,SV_OPTION_SHARED,"next-macro.prf",SV_PREF_EXPLICIT,&ctx).status==SV_PREF_CLEAN);
    const SvPreferenceMacro *macro=sv_preferences_macro(reset,1);
    assert(macro && macro->action_size==2 && !memcmp(macro->action,"xy",2));
    sv_preferences_destroy(reset);
    write_file(library,"user/font-custom-fixture.prf","F:3:8:80\n#:bundled\n");
    write_file(user,"user/font-custom-fixture.prf","F:3:9:81\n#:overlay\n");
    write_file(library,"user/font-sv.prf","F:5:6:82\n");
    SvMapFont map;assert(sv_map_font_init(&map,user,library,"fixture.pcf"));
    ctx.generation=18;
    SvPreferences *candidate=sv_map_font_reload(&map,base,"fixture.pcf",&ctx);assert(candidate);
    assert(map.resolved.owner==SV_FILE_SHARED && strstr(map.resolved.path,"/U/user/"));
    assert(map.definitions.features[3].attribute==9 && map.definitions.features[3].character==81);
    assert(map.definitions.features[1].character==0 && mapping(candidate,'S',1));
    assert(sv_preferences_effect_count(candidate)==1);
    const SvPreferenceEffect *effect=sv_preferences_effect(candidate,0);
    assert(effect && effect->generation==18 && effect->size==7 && !memcmp(effect->bytes,"overlay",7));
    /* Reads/repaint of prepared definitions cannot regenerate load effects. */
    SvVisualDefinitions copied;sv_map_font_definitions(&copied,candidate);
    assert(sv_preferences_effect_count(candidate)==1 && !memcmp(&copied,&map.definitions,sizeof(copied)));
    live_wire(&map.definitions);
    /* Invalid overlay is not absence and must retain the active candidate. */
    remove_file(user,"user/font-custom-fixture.prf");
    assert(snprintf(path,sizeof(path),"%s/user/font-custom-fixture.prf",user)<(int)sizeof(path));
    assert(!mkdir(path,0700));
    SvMapFont rejected=map;
    assert(!sv_map_font_reload(&rejected,candidate,"fixture.pcf",&ctx));
    assert(rejected.source_result==SV_FILE_INVALID && rejected.status==SV_PREF_UNAVAILABLE);
    assert(map.definitions.features[3].character==81 && mapping(candidate,'F',3));
    assert(!rmdir(path));
    sv_preferences_destroy(candidate);
    candidate=sv_map_font_reload(&map,base,"fixture.pcf",&ctx);assert(candidate);
    assert(map.resolved.owner==SV_FILE_BUNDLED && map.definitions.features[3].character==80);
    sv_preferences_destroy(candidate);
    remove_file(library,"user/font-custom-fixture.prf");
    candidate=sv_map_font_reload(&map,base,"fixture.pcf",&ctx);assert(candidate);
    assert(!strcmp(map.source,"font-sv.prf") && map.definitions.features[5].character==82);
    assert(!map.definitions.features[3].character && sv_preferences_effect_count(candidate)==0);
    sv_preferences_destroy(candidate);
    remove_file(library,"user/font-sv.prf");
    candidate=sv_map_font_reload(&map,base,"fixture.pcf",&ctx);assert(candidate);
    assert(map.source_result==SV_FILE_MISSING && !map.definitions.features[5].character);
    sv_preferences_destroy(candidate);
    assert(mapping(base,'F',1) && sv_preferences_effect(base,0)->generation==17);
    sv_preferences_destroy(base);
    remove_file(user,"user/initial.prf");remove_file(user,"user/next-macro.prf");
    assert(snprintf(path,sizeof(path),"%s/user",user)<(int)sizeof(path));assert(!rmdir(path));
    assert(snprintf(path,sizeof(path),"%s/user",library)<(int)sizeof(path));assert(!rmdir(path));
    assert(!rmdir(user) && !rmdir(library) && !rmdir(root));
    puts("SV map font: visual reset, U/B custom/system PRFs, effects generation and PLAY/live wire PASS");
    return 0;
}

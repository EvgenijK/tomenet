#define _POSIX_C_SOURCE 200809L
#include "lua/runtime.h"
#include "lua/reload.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    unsigned diagnostics, warnings, events, warning_event, guide_error_event;
    SvLuaResult last;
} Evidence;
static const char *user_root, *library;
static void diagnostic(void *context, const SvLuaResult *result)
{
    Evidence *evidence = context;
    ++evidence->events;
    assert(result->diagnostic[0]);
    if (strstr(result->diagnostic,"restarting the game is recommended")) {
        ++evidence->warnings; evidence->warning_event=evidence->events;
    }
    if (strstr(result->diagnostic,"Cannot rebuild Lua Guide metadata"))
        evidence->guide_error_event=evidence->events;
    if (result->status != SV_LUA_OK) {
        ++evidence->diagnostics; evidence->last = *result;
    }
}
static void path(char *out, const char *root, const char *name)
{
    int count = snprintf(out,4096,"%s/scpt/%s",root,name);
    assert(count > 0 && count < 4096);
}
static void write_script(const char *name, const char *text)
{
    char destination[4096]; path(destination,user_root,name);
    FILE *stream = fopen(destination,"wb"); assert(stream);
    assert(fputs(text,stream) >= 0 && !fclose(stream));
}
static void remove_script(const char *name)
{
    char destination[4096]; path(destination,user_root,name);
    assert(!unlink(destination));
}
static void append_bundled(const char *name, const char *extra)
{
    char source[4096], destination[4096], bytes[4096];
    path(source,library,name); path(destination,user_root,name);
    FILE *input=fopen(source,"rb"), *output=fopen(destination,"wb");
    assert(input && output);
    size_t size;
    while ((size=fread(bytes,1,sizeof(bytes),input)))
        assert(fwrite(bytes,1,size,output)==size);
    assert(!ferror(input) && !fclose(input));
    assert(fputs("\n",output)>=0 && fputs(extra,output)>=0 && !fclose(output));
}
static long number(SvLuaRuntime *runtime, const char *name)
{
    long value=-999; assert(sv_lua_number(runtime,name,&value)==SV_LUA_OK);
    return value;
}
static void expect_ready(SvLuaRuntime *runtime, SvLuaResult result)
{
    if (result.status!=SV_LUA_OK)
        fprintf(stderr,"Lua startup failure: %s: %s\n",result.path,result.diagnostic);
    assert(result.status==SV_LUA_OK && sv_lua_ready(runtime));
    assert(sv_lua_school_count(runtime)>0 && sv_lua_spell_count(runtime)>0);
    assert(sv_lua_school(runtime,0)->name[0] && sv_lua_spell(runtime,0)->name[0]);
}
static void runtime_cases(SvLuaRuntime *runtime, Evidence *evidence)
{
    SvLuaFeatures features={.rpg=true,.temporary=4};
    assert(sv_lua_init(runtime).status==SV_LUA_OK);
    SvLuaResult result=sv_lua_open(runtime,features);
    expect_ready(runtime,result);
    assert(result.scripts_loaded>20);
    assert(number(runtime,"RPG_SERVER")==1 && number(runtime,"TEMP2")==4);
    size_t count;
    assert(sv_lua_guide_count(runtime,SV_LUA_GUIDE_SKILLS,&count)==SV_LUA_OK && count==40);
    size_t schools=sv_lua_school_count(runtime), spells=sv_lua_spell_count(runtime);
    printf("bundled closure: scripts=%u schools=%zu spells=%zu guide_skills=%zu\n",
        result.scripts_loaded,schools,spells,count);
    write_script("native-return.lua","return 6*8, 'native-file-result'\n");
    char fixture[4096], extra[8192]; path(fixture,user_root,"native-return.lua");
    int length=snprintf(extra,sizeof(extra),
        "guide_races=1+1\nguide_race={'Overlay '..(6*7),'Second'}\n"
        "native_dostring,native_string=dostring(\"return 6*7, 'native-string-result'\")\n"
        "native_dofile,native_file_string=dofile('%s')\n",fixture);
    assert(length>0 && (size_t)length<sizeof(extra));
    append_bundled("guide.lua",extra);
    expect_ready(runtime,sv_lua_reload(runtime,features));
    char text[128];
    assert(sv_lua_guide_count(runtime,SV_LUA_GUIDE_RACES,&count)==SV_LUA_OK && count==2);
    assert(sv_lua_guide_entry(runtime,SV_LUA_GUIDE_RACES,0,text,sizeof(text))==SV_LUA_OK);
    assert(!strcmp(text,"Overlay 42"));
    assert(number(runtime,"native_dostring")==42 && number(runtime,"native_dofile")==48);
    assert(sv_lua_string(runtime,"native_string",0,text,sizeof(text))==SV_LUA_OK);
    assert(!strcmp(text,"native-string-result"));
    assert(sv_lua_string(runtime,"native_file_string",0,text,sizeof(text))==SV_LUA_OK);
    assert(!strcmp(text,"native-file-result"));
    assert(sv_lua_school_count(runtime)==schools && sv_lua_spell_count(runtime)==spells);
    unsigned before=evidence->diagnostics;
    append_bundled("c-init.lua","pern_dofile(Ind, 'nested-fixture.lua')\n");
    result=sv_lua_reload(runtime,features);
    assert(result.status!=SV_LUA_OK && !sv_lua_ready(runtime));
    assert(evidence->diagnostics>before && strstr(evidence->last.path,"nested-fixture.lua"));
    before=evidence->diagnostics;
    write_script("nested-fixture.lua","not valid Lua !!!\n");
    result=sv_lua_reload(runtime,features);
    assert(result.status!=SV_LUA_OK && !sv_lua_ready(runtime));
    assert(evidence->diagnostics>before && strstr(evidence->last.path,"nested-fixture.lua"));
    remove_script("nested-fixture.lua"); remove_script("c-init.lua");
    expect_ready(runtime,sv_lua_reload(runtime,(SvLuaFeatures){0}));
    assert(number(runtime,"RPG_SERVER")==0 && number(runtime,"TEMP2")==0);
    sv_lua_clear(runtime);
    assert(!sv_lua_ready(runtime) && !sv_lua_school_count(runtime) && !sv_lua_spell_count(runtime));
    remove_script("guide.lua"); remove_script("native-return.lua");
    puts("runtime: real overlay expressions/native chunks, nested failures, reload and cleanup PASS");
}


static void completion_cases(Evidence *evidence)
{
    SvLuaReload *owner=sv_lua_reload_create(user_root,library,diagnostic,evidence);
    assert(owner && sv_lua_reload_init(owner).status==SV_LUA_OK);
    assert(sv_lua_reload_begin(owner,10,(SvLuaFeatures){.fun=true}).status==SV_LUA_OK);
    /* Rejected INIT has no effect on the process-lifetime sticky flags. */
    assert(sv_lua_reload_file_init(owner,10,"scpt/audio.lua",false));
    assert(sv_lua_reload_file_init(owner,10,"scpt/guide.lua",false));
    assert(sv_lua_reload_end(owner,10,0,true));
    SvLuaCompletion result;
    assert(sv_lua_reload_take(owner,&result));
    assert(result.reload_attempted && result.runtime.status==SV_LUA_OK);
    assert(!result.audio_warning && !result.guide_rebuilt);
    assert(!sv_lua_reload_take(owner,&result));
    /* Successful INIT followed by disconnect BEFORE END must retain flags. */
    assert(sv_lua_reload_file_init(owner,10,"scpt/audio.lua",true));
    assert(sv_lua_reload_file_init(owner,10,"scpt/guide.lua",true));
    sv_lua_reload_close(owner,10);
    assert(!sv_lua_reload_completion(owner));
    assert(!sv_lua_ready(sv_lua_reload_runtime(owner)));
    unsigned before=evidence->events;
    assert(!sv_lua_reload_file_init(owner,10,"guide.lua",true));
    assert(!sv_lua_reload_end(owner,10,0,true) && evidence->events==before);
    assert(sv_lua_reload_begin(owner,11,(SvLuaFeatures){0}).status==SV_LUA_OK);
    append_bundled("guide.lua","guide_races=1+1\nguide_race={'Transferred '..(3*7),'Second'}\n");
    before=evidence->events;
    assert(sv_lua_reload_end(owner,11,1,true));
    assert(sv_lua_reload_take(owner,&result) && !result.reload_attempted);
    assert(evidence->events==before && !sv_lua_reload_take(owner,&result));
    /* The failed close and successful reload are independent production results. */
    assert(sv_lua_reload_end(owner,11,0,false));
    assert(sv_lua_reload_take(owner,&result));
    assert(!result.transfer_success && result.reload_attempted && result.runtime.status==SV_LUA_OK);
    assert(result.audio_warning && result.guide_rebuilt && result.guide_status==SV_LUA_OK);
    assert(sv_lua_reload_guide_valid(owner));
    assert(sv_lua_reload_guide_count(owner,SV_LUA_GUIDE_RACES)==2);
    assert(!strcmp(sv_lua_reload_guide_entry(owner,SV_LUA_GUIDE_RACES,0),"Transferred 21"));
    unsigned warnings=evidence->warnings;
    before=evidence->events;
    assert(sv_lua_reload_completion(owner)->audio_warning);
    assert(sv_lua_reload_completion(owner)->guide_rebuilt);
    assert(!sv_lua_reload_take(owner,&result));
    assert(!sv_lua_reload_end(owner,10,0,true));
    assert(evidence->events==before && evidence->warnings==warnings);
    /* A genuine second END occurrence retains baseline behavior, unlike reads. */
    assert(sv_lua_reload_end(owner,11,0,true));
    assert(sv_lua_reload_take(owner,&result) && result.reload_attempted);
    assert(evidence->warnings==warnings+1);
    /* Malformed metadata is evaluated after the warning, with no false rebuild. */
    append_bundled("guide.lua","guide_races='invalid count'\n");
    evidence->warning_event=evidence->guide_error_event=0;
    assert(sv_lua_reload_end(owner,11,0,true));
    assert(sv_lua_reload_take(owner,&result));
    assert(result.runtime.status==SV_LUA_OK && result.audio_warning && !result.guide_rebuilt);
    assert(result.guide_status!=SV_LUA_OK && !sv_lua_reload_guide_valid(owner));
    assert(evidence->warning_event && evidence->guide_error_event>evidence->warning_event);
    assert(!sv_lua_reload_guide_entry(owner,SV_LUA_GUIDE_RACES,0));
    remove_script("guide.lua");
    sv_lua_reload_close(owner,11);
    assert(!sv_lua_ready(sv_lua_reload_runtime(owner)));
    assert(!sv_lua_reload_take(owner,&result));
    sv_lua_reload_destroy(owner);
    puts("completion: last END/replay/stale, INIT-disconnect sticky flags, failed close and warning order PASS");
}

int main(int argc,char **argv)
{
    assert(argc==3); user_root=argv[1]; library=argv[2];
    Evidence evidence={0};
    SvLuaRuntime *runtime=sv_lua_create(user_root,library,diagnostic,&evidence);
    assert(runtime);
    runtime_cases(runtime,&evidence);
    sv_lua_destroy(runtime);
    completion_cases(&evidence);
    puts("PASS: production Lua startup/reload core");
    return 0;
}

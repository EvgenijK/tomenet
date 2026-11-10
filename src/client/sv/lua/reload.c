#include "lua/reload.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Legacy guide_* rows use MAX_CHARS (common/defines.h), currently 80. */
enum { GUIDE_KINDS = 5, GUIDE_TEXT = 80 };
typedef struct { size_t count; char (*entries)[GUIDE_TEXT]; } GuideRows;
struct SvLuaReload {
    SvLuaRuntime *runtime;
    SvLuaDiagnostic diagnostic;
    void *context;
    SvLuaFeatures features;
    uint64_t generation;
    bool active, updated_audio, updated_guide, guide_valid, pending;
    GuideRows guide[GUIDE_KINDS];
    SvLuaCompletion completion;
};

static void clear_guide(SvLuaReload *owner)
{
    for (unsigned i=0;i<GUIDE_KINDS;++i) {
        free(owner->guide[i].entries);
        owner->guide[i]=(GuideRows){0};
    }
    owner->guide_valid=false;
}

static void report(SvLuaReload *owner, SvLuaStatus status, const char *message)
{
    if (!owner->diagnostic) return;
    SvLuaResult result={.status=status};
    snprintf(result.diagnostic,sizeof(result.diagnostic),"%s",message);
    owner->diagnostic(owner->context,&result);
}

static SvLuaStatus rebuild_guide(SvLuaReload *owner)
{
    /* Baseline destination capacities, with checked counts and copied strings. */
    static const size_t limits[GUIDE_KINDS]={64,64,128,64,256};
    clear_guide(owner);
    SvLuaStatus status=SV_LUA_OK;
    for (unsigned kind=0;kind<GUIDE_KINDS;++kind) {
        GuideRows *rows=&owner->guide[kind];
        status=sv_lua_guide_count(owner->runtime,(SvLuaGuideKind)kind,&rows->count);
        if (status!=SV_LUA_OK) break;
        if (rows->count>limits[kind]) { status=SV_LUA_INVALID; break; }
        if (rows->count) {
            rows->entries=calloc(rows->count,sizeof(*rows->entries));
            if (!rows->entries) { status=SV_LUA_MEMORY; break; }
        }
        for (size_t i=0;i<rows->count;++i) {
            status=sv_lua_guide_entry(owner->runtime,(SvLuaGuideKind)kind,i,
                                     rows->entries[i],GUIDE_TEXT);
            if (status!=SV_LUA_OK) break;
        }
        if (status!=SV_LUA_OK) break;
    }
    if (status!=SV_LUA_OK) {
        clear_guide(owner);
        report(owner,status,"Cannot rebuild Lua Guide metadata: invalid, missing or unavailable getter data");
    } else owner->guide_valid=true;
    return status;
}

SvLuaReload *sv_lua_reload_create(const char *user, const char *bundled,
                                SvLuaDiagnostic diagnostic, void *context)
{
    SvLuaReload *owner=calloc(1,sizeof(*owner));
    if (!owner) return NULL;
    owner->diagnostic=diagnostic;owner->context=context;
    owner->runtime=sv_lua_create(user,bundled,diagnostic,context);
    if (!owner->runtime) { free(owner); return NULL; }
    return owner;
}

void sv_lua_reload_destroy(SvLuaReload *owner)
{
    if (!owner) return;
    clear_guide(owner);
    sv_lua_destroy(owner->runtime);
    free(owner);
}

SvLuaResult sv_lua_reload_init(SvLuaReload *owner)
{
    if (!owner) return (SvLuaResult){.status=SV_LUA_NOT_READY};
    return sv_lua_init(owner->runtime);
}

SvLuaResult sv_lua_reload_begin(SvLuaReload *owner, uint64_t generation, SvLuaFeatures features)
{
    if (!owner || !generation) return (SvLuaResult){.status=SV_LUA_NOT_READY};
    clear_guide(owner);
    sv_lua_clear(owner->runtime);
    owner->features=features;owner->generation=generation;owner->active=true;
    owner->pending=false;owner->completion=(SvLuaCompletion){0};
    return sv_lua_open(owner->runtime,features);
}

void sv_lua_reload_close(SvLuaReload *owner, uint64_t generation)
{
    if (!owner || !owner->active || owner->generation!=generation) return;
    owner->active=false;owner->generation=0;owner->features=(SvLuaFeatures){0};
    owner->pending=false;owner->completion=(SvLuaCompletion){0};
    clear_guide(owner);
    sv_lua_clear(owner->runtime);
}

bool sv_lua_reload_file_init(SvLuaReload *owner, uint64_t generation,
                           const char *filename, bool accepted)
{
    if (!owner || !owner->active || owner->generation!=generation || !filename) return false;
    if (accepted) {
        owner->updated_audio|=strstr(filename,"audio.lua")!=NULL;
        owner->updated_guide|=strstr(filename,"guide.lua")!=NULL;
    }
    return true;
}

bool sv_lua_reload_end(SvLuaReload *owner, uint64_t generation, size_t remaining,
                       bool transfer_success)
{
    if (!owner || !owner->active || owner->generation!=generation) return false;
    owner->completion=(SvLuaCompletion){.generation=generation,.remaining=remaining,
        .transfer_success=transfer_success,.runtime={.status=SV_LUA_NOT_READY},
        .guide_status=SV_LUA_NOT_READY};
    owner->pending=true;
    if (remaining) return true;
    owner->completion.reload_attempted=true;
    report(owner,SV_LUA_OK,"Reinitializing Lua");
    owner->completion.runtime=sv_lua_reload(owner->runtime,owner->features);
    if (owner->updated_audio) {
        owner->completion.audio_warning=true;
        report(owner,SV_LUA_OK,"* Audio information was updated - restarting the game is recommended! *");
        report(owner,SV_LUA_OK,"  Without a restart, you might be hearing the wrong sound effects or music.");
    }
    /* Reload invalidates all previous getter-derived data, even on failure. */
    clear_guide(owner);
    if (owner->updated_guide && owner->completion.runtime.status==SV_LUA_OK) {
        owner->completion.guide_status=rebuild_guide(owner);
        owner->completion.guide_rebuilt=owner->guide_valid;
    }
    return true;
}

const SvLuaCompletion *sv_lua_reload_completion(const SvLuaReload *owner)
{ return owner && owner->active ? &owner->completion : NULL; }

bool sv_lua_reload_take(SvLuaReload *owner, SvLuaCompletion *result)
{
    if (!owner || !result || !owner->pending) return false;
    *result=owner->completion;owner->pending=false;
    return true;
}

bool sv_lua_reload_guide_valid(const SvLuaReload *owner)
{ return owner && owner->guide_valid; }

size_t sv_lua_reload_guide_count(const SvLuaReload *owner, SvLuaGuideKind kind)
{
    return owner && owner->guide_valid && (unsigned)kind<GUIDE_KINDS ? owner->guide[kind].count : 0;
}

const char *sv_lua_reload_guide_entry(const SvLuaReload *owner, SvLuaGuideKind kind, size_t index)
{
    return owner && owner->guide_valid && (unsigned)kind<GUIDE_KINDS && index<owner->guide[kind].count ?
        owner->guide[kind].entries[index] : NULL;
}

const SvLuaRuntime *sv_lua_reload_runtime(const SvLuaReload *owner)
{ return owner ? owner->runtime : NULL; }

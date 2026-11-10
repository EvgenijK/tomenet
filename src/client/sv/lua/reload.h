#ifndef SV_LUA_RELOAD_H
#define SV_LUA_RELOAD_H
#include "lua/runtime.h"

typedef struct SvLuaReload SvLuaReload;
typedef struct {
    uint64_t generation;
    size_t remaining;
    bool transfer_success, reload_attempted, audio_warning, guide_rebuilt;
    SvLuaResult runtime;
    SvLuaStatus guide_status;
} SvLuaCompletion;

/* Application owner: accepted INIT flags survive session close/retry. All
 * calls run on the main thread; the diagnostic recipient must not reenter. */
SvLuaReload *sv_lua_reload_create(const char *user, const char *bundled,
                                SvLuaDiagnostic diagnostic, void *context);
void sv_lua_reload_destroy(SvLuaReload *);
/* Early local scripts only: no invented server feature context. */
SvLuaResult sv_lua_reload_init(SvLuaReload *);
SvLuaResult sv_lua_reload_begin(SvLuaReload *, uint64_t generation, SvLuaFeatures);
void sv_lua_reload_close(SvLuaReload *, uint64_t generation);
/* accepted means successful FILE_INIT, after -u/destination handling by C010. */
bool sv_lua_reload_file_init(SvLuaReload *, uint64_t generation,
                           const char *filename, bool accepted);
/* Each call is a NEW complete END occurrence, even for a repeated wire fnum.
 * C010 supplies the count AFTER close/publication and retains its ACK/ERR.
 * C010 must synchronously consume each END's completion before dispatching the
 * next END (including chained packets), retaining its transfer/runtime/metadata
 * failures in the startup result. This is one latest-result slot, not a queue;
 * the next END replaces it. Reading/taking never reloads. False means stale or
 * inactive session and leaves the previous result unchanged. */
bool sv_lua_reload_end(SvLuaReload *, uint64_t generation, size_t remaining,
                       bool transfer_success);
const SvLuaCompletion *sv_lua_reload_completion(const SvLuaReload *);
bool sv_lua_reload_take(SvLuaReload *, SvLuaCompletion *);
/* Borrowed until mutation/close. NULL means absent or failed metadata rebuild. */
bool sv_lua_reload_guide_valid(const SvLuaReload *);
size_t sv_lua_reload_guide_count(const SvLuaReload *, SvLuaGuideKind);
const char *sv_lua_reload_guide_entry(const SvLuaReload *, SvLuaGuideKind, size_t);
const SvLuaRuntime *sv_lua_reload_runtime(const SvLuaReload *);
#endif

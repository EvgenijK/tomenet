#ifndef SV_LUA_RUNTIME_H
#define SV_LUA_RUNTIME_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct SvLuaRuntime SvLuaRuntime;
typedef struct {
    bool rpg, arcade, fun, party, test;
    uint32_t temporary;
} SvLuaFeatures;
typedef enum { SV_LUA_OK, SV_LUA_MISSING, SV_LUA_INVALID, SV_LUA_IO,
               SV_LUA_MEMORY, SV_LUA_NOT_READY } SvLuaStatus;
typedef struct {
    SvLuaStatus status;
    char path[4096], diagnostic[1024];
    unsigned scripts_loaded;
} SvLuaResult;
typedef void (*SvLuaDiagnostic)(void *context, const SvLuaResult *result);
/* Main-thread owner. Roots are shared SDL3 user/lib roots (contain scpt/).
 * No session/player pointer is borrowed. Destruction frees every owned value. */
SvLuaRuntime *sv_lua_create(const char *user, const char *bundled,
                          SvLuaDiagnostic diagnostic, void *context);
void sv_lua_destroy(SvLuaRuntime *);
SvLuaResult sv_lua_init(SvLuaRuntime *);
/* A failed open is retried from a fresh state; successful open is idempotent.
 * Features belong to the current session. Call clear/reload for a new owner. */
SvLuaResult sv_lua_open(SvLuaRuntime *, SvLuaFeatures);
SvLuaResult sv_lua_reload(SvLuaRuntime *, SvLuaFeatures);
/* Clear session-derived runtime/data while keeping configured resource roots. */
void sv_lua_clear(SvLuaRuntime *);
const SvLuaResult *sv_lua_result(const SvLuaRuntime *);
bool sv_lua_ready(const SvLuaRuntime *);
/* Getter names are global/table identities, never source expressions. Strings
 * are copied into caller storage; no Lua pointer escapes a reload. */
SvLuaStatus sv_lua_number(SvLuaRuntime *, const char *global, long *out);
SvLuaStatus sv_lua_string(SvLuaRuntime *, const char *global, int index,
                         char *out, size_t capacity);
typedef enum { SV_LUA_GUIDE_RACES, SV_LUA_GUIDE_CLASSES, SV_LUA_GUIDE_SKILLS,
               SV_LUA_GUIDE_SCHOOLS, SV_LUA_GUIDE_SPELLS } SvLuaGuideKind;
SvLuaStatus sv_lua_guide_count(SvLuaRuntime *, SvLuaGuideKind, size_t *out);
SvLuaStatus sv_lua_guide_entry(SvLuaRuntime *, SvLuaGuideKind, size_t index,
                             char *out, size_t capacity);
typedef struct { const char *name; long skill; } SvLuaSchool;
typedef struct {
    const char *name;
    long skill_level, mana, mana_max, fail, level, spell_power, extrareq, extralev;
} SvLuaSpell;
/* Borrowed until next mutating runtime call. */
size_t sv_lua_school_count(const SvLuaRuntime *);
size_t sv_lua_spell_count(const SvLuaRuntime *);
const SvLuaSchool *sv_lua_school(const SvLuaRuntime *, size_t);
const SvLuaSpell *sv_lua_spell(const SvLuaRuntime *, size_t);
#endif

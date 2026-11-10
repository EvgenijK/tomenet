#define _POSIX_C_SOURCE 200809L
#include "lua/runtime.h"
#include "files/files.h"
#include "../../../server/lua/lua.h"
#include "../../../server/lua/lualib.h"
#include "../../../server/lua/lauxlib.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Main-thread owned adaptation of c-script.c and startup lua_bind.c data.
 * C009 exports startup data bindings; deferred gameplay functions are not
 * registered as successful stubs. Script function bodies remain real Lua. */
#define DATA_LIMIT 32767
struct SvLuaRuntime {
    lua_State *state;
    char user[4096], bundled[4096], loading[4096];
    SvLuaDiagnostic diagnostic;
    void *context;
    SvLuaResult result;
    bool initialized, opened;
    SvLuaSchool *schools;
    SvLuaSpell *spells;
    int *spell_refs;
    int spell_tag;
    size_t school_count, spell_count;
};
void sv_lua_constants(lua_State *);
static bool text_copy(char *out, size_t capacity, const char *value)
{
    size_t size = strlen(value);
    if (size >= capacity) return false;
    memcpy(out, value, size + 1);
    return true;
}
static SvLuaRuntime *owner(lua_State *L)
{
    lua_getregistry(L);
    lua_pushstring(L, "sv.owner");
    lua_rawget(L, -2);
    SvLuaRuntime *runtime = lua_touserdata(L, -1);
    lua_pop(L, 2);
    return runtime;
}
static void failure(SvLuaRuntime *r, SvLuaStatus status, const char *path, const char *reason)
{
    /* A nested failure must survive an ignored pern_dofile return and later
     * cascading undefined globals. Preserve the first concrete cause. */
    if (r->result.status != SV_LUA_OK) return;
    r->result.status = status;
    snprintf(r->result.path, sizeof(r->result.path), "%s", path ? path : "");
    snprintf(r->result.diagnostic, sizeof(r->result.diagnostic), "%s", reason);
    r->opened = false;
    if (r->diagnostic) r->diagnostic(r->context, &r->result);
}
static int alert_message(lua_State *L)
{
    SvLuaRuntime *r = owner(L);
    const char *message = lua_tostring(L, 1);
    SvLuaResult notice = {.status = SV_LUA_OK, .scripts_loaded = r->result.scripts_loaded};
    snprintf(notice.path, sizeof(notice.path), "%s", r->loading);
    snprintf(notice.diagnostic, sizeof(notice.diagnostic), "%s", message ? message : "Lua notice");
    if (r->diagnostic) r->diagnostic(r->context, &notice);
    return 0;
}
static int error_message(lua_State *L)
{
    SvLuaRuntime *r = owner(L);
    const char *message = lua_tostring(L, 1);
    failure(r, SV_LUA_INVALID, r->loading, message ? message : "Lua execution failed");
    return 0;
}
static SvLuaStatus script(SvLuaRuntime *r, const char *name)
{
    char relative[4096], previous[4096];
    SvFilePath path = {0};
    int n = snprintf(relative, sizeof(relative), "scpt/%s", name);
    if (n < 0 || (size_t)n >= sizeof(relative)) {
        failure(r, SV_LUA_INVALID, name, "Lua script path is too long");
        return SV_LUA_INVALID;
    }
    SvFileResult found = sv_file_resource(&path, r->user, r->bundled, relative);
    if (found != SV_FILE_OK) {
        SvLuaStatus status = found == SV_FILE_MISSING ? SV_LUA_MISSING :
            found == SV_FILE_IO ? SV_LUA_IO : SV_LUA_INVALID;
        failure(r, status, *path.path ? path.path : relative, "Lua script resource is missing, invalid or unreadable");
        return status;
    }
    text_copy(previous, sizeof(previous), r->loading);
    text_copy(r->loading, sizeof(r->loading), path.path);
    int top = lua_gettop(r->state);
    int error = lua_dofile(r->state, path.path);
    lua_settop(r->state, top);
    if (error) failure(r, error == LUA_ERRMEM ? SV_LUA_MEMORY :
        error == LUA_ERRFILE ? SV_LUA_IO : SV_LUA_INVALID, path.path, "Lua script load failed");
    else ++r->result.scripts_loaded;
    text_copy(r->loading, sizeof(r->loading), previous);
    return error ? r->result.status : SV_LUA_OK;
}
static int pern_dofile(lua_State *L)
{
    SvLuaRuntime *r = owner(L);
    const char *name = luaL_check_string(L, 2);
    SvLuaStatus status = script(r, name);
    lua_pushnumber(L, status != SV_LUA_OK); /* baseline bool: true means error */
    return 1;
}
static int bit_mod(lua_State *L)
{
    long a = luaL_check_number(L, 1), b = luaL_check_number(L, 2);
    if (!b || (a == LONG_MIN && b == -1)) lua_error(L, "invalid integer modulus");
    lua_pushnumber(L, a % b); return 1;
}
/* Legacy util bshl/bshr operate on u32b, unlike variadic bitlib shifts. */
static int shift_left(lua_State *L)
{
    uint32_t value = (uint32_t)luaL_check_number(L, 1);
    long shift = luaL_check_number(L, 2);
    if (shift < 0 || shift >= 32) lua_error(L, "invalid shift count");
    lua_pushnumber(L, (long)(value << shift)); return 1;
}
static int shift_right(lua_State *L)
{
    uint32_t value = (uint32_t)luaL_check_number(L, 1);
    long shift = luaL_check_number(L, 2);
    if (shift < 0 || shift >= 32) lua_error(L, "invalid shift count");
    lua_pushnumber(L, (long)(value >> shift)); return 1;
}
static int bit_shift_left(lua_State *L)
{
    unsigned long value = (unsigned long)luaL_check_number(L, 1);
    long shift = luaL_check_number(L, 2);
    if (shift < 0 || shift >= (long)(sizeof(long) * CHAR_BIT)) lua_error(L, "invalid shift count");
    lua_pushnumber(L, (long)(value << shift)); return 1;
}
static int bit_shift_right(lua_State *L)
{
    unsigned long value = (unsigned long)luaL_check_number(L, 1);
    long shift = luaL_check_number(L, 2);
    if (shift < 0 || shift >= (long)(sizeof(long) * CHAR_BIT)) lua_error(L, "invalid shift count");
    lua_pushnumber(L, (long)(value >> shift)); return 1;
}
static int bit_arshift(lua_State *L)
{
    long value = luaL_check_number(L, 1), shift = luaL_check_number(L, 2);
    if (shift < 0 || shift >= (long)(sizeof(long) * CHAR_BIT)) lua_error(L, "invalid shift count");
    lua_pushnumber(L, value >> shift); return 1;
}
static int new_school(lua_State *L)
{
    SvLuaRuntime *r = owner(L);
    long index = luaL_check_number(L, 1);
    const char *name = luaL_check_string(L, 2);
    long skill = luaL_check_number(L, 3);
    if (index < 0 || (size_t)index >= r->school_count) lua_error(L, "school index out of range");
    char *copy = strdup(name);
    if (!copy) lua_error(L, "school name allocation failed");
    free((void *)r->schools[index].name);
    r->schools[index] = (SvLuaSchool){copy, (int16_t)skill};
    lua_pushnumber(L, index); return 1;
}
static int new_spell(lua_State *L)
{
    SvLuaRuntime *r = owner(L);
    long index = luaL_check_number(L, 1);
    const char *name = luaL_check_string(L, 2);
    if (index < 0 || (size_t)index >= r->spell_count) lua_error(L, "spell index out of range");
    char *copy = strdup(name);
    if (!copy) lua_error(L, "spell name allocation failed");
    free((void *)r->spells[index].name);
    r->spells[index].name = copy;
    if (r->spell_refs[index] != LUA_NOREF) lua_unref(L, r->spell_refs[index]);
    r->spells[index] = (SvLuaSpell){.name = copy};
    /* Tolua stores non-C fields in a per-instance mate table. Keep the same
     * behavior for extrareq/extralev and future script-owned metadata. */
    lua_newtable(L);
    r->spell_refs[index] = lua_ref(L, 1);
    lua_pushnumber(L, index); return 1;
}
static int spell_record(lua_State *L)
{
    SvLuaRuntime *r = owner(L);
    long index = luaL_check_number(L, 1);
    if (index < 0 || (size_t)index >= r->spell_count ||
        r->spell_refs[index] == LUA_NOREF) lua_error(L, "uninitialized spell index");
    lua_pushusertag(L, &r->spells[index], r->spell_tag);
    return 1;
}
/* The actual C fields retain the generated w_spells.c numeric assignment
 * policy (byte/s16 casts). Lua-owned extra fields remain in a mate table. */
static long *spell_field(SvLuaSpell *s, const char *name, bool *byte)
{
    *byte = true;
    if (!strcmp(name, "skill_level")) return &s->skill_level;
    if (!strcmp(name, "mana")) return &s->mana;
    if (!strcmp(name, "mana_max")) return &s->mana_max;
    if (!strcmp(name, "fail")) return &s->fail;
    if (!strcmp(name, "spell_power")) return &s->spell_power;
    *byte = false;
    if (!strcmp(name, "level")) return &s->level;
    return NULL;
}
static SvLuaSpell *checked_spell(lua_State *L)
{
    SvLuaRuntime *r = owner(L);
    if (lua_tag(L, 1) != r->spell_tag) lua_error(L, "invalid spell record");
    return lua_touserdata(L, 1);
}
static void spell_mate(lua_State *L, SvLuaSpell *s)
{
    SvLuaRuntime *r = owner(L);
    size_t index = (size_t)(s - r->spells);
    if (index >= r->spell_count) lua_error(L, "invalid spell record owner");
    lua_getref(L, r->spell_refs[index]);
}
static int spell_get(lua_State *L)
{
    SvLuaSpell *s = checked_spell(L);
    const char *name = luaL_check_string(L, 2);
    if (!strcmp(name, "name")) { lua_pushstring(L, s->name); return 1; }
    bool byte;
    long *field = spell_field(s, name, &byte);
    if (field) { lua_pushnumber(L, *field); return 1; }
    spell_mate(L, s); lua_pushvalue(L, 2); lua_rawget(L, -2);
    return 1;
}
static int spell_set(lua_State *L)
{
    SvLuaSpell *s = checked_spell(L);
    const char *name = luaL_check_string(L, 2);
    if (!strcmp(name, "name")) {
        if (lua_type(L, 3) != LUA_TSTRING) lua_error(L, "invalid spell name type");
        char *copy = strdup(lua_tostring(L, 3));
        if (!copy) lua_error(L, "spell name allocation failed");
        free((void *)s->name); s->name = copy; return 0;
    }
    bool byte;
    long *field = spell_field(s, name, &byte);
    if (field) {
        if (lua_type(L, 3) != LUA_TNUMBER) lua_error(L, "invalid spell numeric field type");
        long value = lua_tonumber(L, 3);
        *field = byte ? (uint8_t)value : (int16_t)value;
        return 0;
    }
    spell_mate(L, s); lua_pushvalue(L, 2); lua_pushvalue(L, 3); lua_rawset(L, -3);
    /* Expose script-owned numeric extras in the C view without altering the
     * mate's original value/type or imposing a new binding restriction. */
    if (!strcmp(name, "extrareq")) s->extrareq = lua_tonumber(L, 3);
    if (!strcmp(name, "extralev")) s->extralev = lua_tonumber(L, 3);
    return 0;
}
static int util_not(lua_State *L)
{ lua_pushnumber(L, (long)~(uint32_t)luaL_check_number(L, 1)); return 1; }
static int util_mod(lua_State *L)
{
    uint32_t a = (uint32_t)luaL_check_number(L, 1), b = (uint32_t)luaL_check_number(L, 2);
    if (!b) lua_error(L, "invalid integer modulus");
    lua_pushnumber(L, (long)(a % b)); return 1;
}
static int util_and(lua_State *L)
{
    uint32_t a = (uint32_t)luaL_check_number(L, 1), b = (uint32_t)luaL_check_number(L, 2);
    lua_pushnumber(L, (long)(a & b)); return 1;
}
static int util_or(lua_State *L)
{
    uint32_t a = (uint32_t)luaL_check_number(L, 1), b = (uint32_t)luaL_check_number(L, 2);
    lua_pushnumber(L, (long)(a | b)); return 1;
}
static int util_xor(lua_State *L)
{
    uint32_t a = (uint32_t)luaL_check_number(L, 1), b = (uint32_t)luaL_check_number(L, 2);
    lua_pushnumber(L, (long)(a ^ b)); return 1;
}
static void bindings(SvLuaRuntime *r)
{
    lua_State *L = r->state;
    lua_baselibopen(L); lua_strlibopen(L); lua_dblibopen(L);
    /* NO_CLIENT_IOLIB: deliberately do not call lua_iolibopen. Base dofile and
     * dostring stay native; only pern_dofile resolves the resource overlay. */
    lua_getregistry(L); lua_pushstring(L, "sv.owner");
    lua_pushuserdata(L, r); lua_rawset(L, -3); lua_pop(L, 1);
    lua_register(L, "_ALERT", alert_message);
    lua_register(L, LUA_ERRORMESSAGE, error_message);
    sv_lua_constants(L);
    r->spell_tag = lua_newtag(L);
    lua_pushcfunction(L, spell_get); lua_settagmethod(L, r->spell_tag, "gettable");
    lua_pushcfunction(L, spell_set); lua_settagmethod(L, r->spell_tag, "settable");
    lua_register(L, "pern_dofile", pern_dofile);
    lua_register(L, "new_school", new_school); lua_register(L, "new_spell", new_spell);
    lua_register(L, "spell", spell_record);
    lua_register(L, "bnot", util_not); lua_register(L, "imod", bit_mod);
    lua_register(L, "band", util_and); lua_register(L, "bor", util_or);
    lua_register(L, "bxor", util_xor); lua_register(L, "lshift", bit_shift_left);
    lua_register(L, "rshift", bit_shift_right); lua_register(L, "arshift", bit_arshift);
    lua_register(L, "mod", util_mod); lua_register(L, "bshl", shift_left);
    lua_register(L, "bshr", shift_right);
    lua_pushnumber(L, 1); lua_setglobal(L, "Ind");
}
SvLuaRuntime *sv_lua_create(const char *user, const char *bundled,
                          SvLuaDiagnostic diagnostic, void *context)
{
    if (!user || !*user || !bundled || !*bundled) return NULL;
    SvLuaRuntime *r = calloc(1, sizeof(*r));
    if (!r) return NULL;
    if (!text_copy(r->user, sizeof(r->user), user) ||
        !text_copy(r->bundled, sizeof(r->bundled), bundled)) { free(r); return NULL; }
    r->diagnostic = diagnostic; r->context = context;
    return r;
}
void sv_lua_clear(SvLuaRuntime *r)
{
    if (!r) return;
    if (r->state) lua_close(r->state);
    for (size_t i = 0; i < r->school_count; ++i) free((void *)r->schools[i].name);
    for (size_t i = 0; i < r->spell_count; ++i) free((void *)r->spells[i].name);
    free(r->schools); free(r->spells); free(r->spell_refs);
    r->state = NULL; r->schools = NULL; r->spells = NULL; r->spell_refs = NULL;
    r->school_count = r->spell_count = 0;
    r->initialized = r->opened = false;
    r->loading[0] = 0;
    r->result = (SvLuaResult){.status = SV_LUA_NOT_READY};
}
void sv_lua_destroy(SvLuaRuntime *r) { if (r) { sv_lua_clear(r); free(r); } }
SvLuaResult sv_lua_init(SvLuaRuntime *r)
{
    if (!r) return (SvLuaResult){.status = SV_LUA_INVALID};
    if (r->initialized) return r->result;
    sv_lua_clear(r);
    r->result = (SvLuaResult){.status = SV_LUA_OK};
    r->state = lua_open(0);
    if (!r->state) { failure(r, SV_LUA_MEMORY, "", "Lua state allocation failed"); return r->result; }
    bindings(r);
    const char *early[] = {"xml.lua", "meta.lua", "audio.lua", "guide.lua", "activations.lua"};
    for (size_t i = 0; i < sizeof(early)/sizeof(*early); ++i) {
        if (script(r, early[i]) != SV_LUA_OK || r->result.status != SV_LUA_OK) return r->result;
    }
    r->initialized = true;
    return r->result;
}
static void global_number(lua_State *L, const char *name, long value)
{ lua_pushnumber(L, value); lua_setglobal(L, name); }
static void raw_global(lua_State *L, const char *name);
static bool call_index(SvLuaRuntime *r, const char *function, size_t index)
{
    int top = lua_gettop(r->state);
    raw_global(r->state, function);
    if (!lua_isfunction(r->state, -1)) {
        lua_settop(r->state, top);
        failure(r, SV_LUA_INVALID, function, "Missing Lua data finalizer");
        return false;
    }
    lua_pushnumber(r->state, (long)index);
    int error = lua_call(r->state, 1, 0);
    lua_settop(r->state, top);
    if (error) failure(r, SV_LUA_INVALID, function, "Lua data finalization failed");
    return !error && r->result.status == SV_LUA_OK;
}
SvLuaResult sv_lua_open(SvLuaRuntime *r, SvLuaFeatures features)
{
    if (!r) return (SvLuaResult){.status = SV_LUA_INVALID};
    if (r->opened) return r->result;
    if (r->result.status != SV_LUA_OK && r->state) sv_lua_clear(r);
    if (!r->initialized && sv_lua_init(r).status != SV_LUA_OK) return r->result;
    lua_State *L = r->state;
    global_number(L, "RPG_SERVER", features.rpg); global_number(L, "ARCADE_SERVER", features.arcade);
    global_number(L, "FUN_SERVER", features.fun); global_number(L, "PARTY_SERVER", features.party);
    global_number(L, "TEST_SERVER", features.test);
    for (unsigned i = 0; i < 8; ++i) {
        char name[16]; snprintf(name, sizeof(name), "TEMP%u", i);
        global_number(L, name, features.temporary & (1u << i));
    }
    if (script(r, "c-init.lua") != SV_LUA_OK || r->result.status != SV_LUA_OK) return r->result;
    long schools, spells;
    if (sv_lua_number(r, "__schools_num", &schools) != SV_LUA_OK ||
        sv_lua_number(r, "__tmp_spells_num", &spells) != SV_LUA_OK ||
        schools < 0 || spells < 0 || schools > DATA_LIMIT || spells > DATA_LIMIT) {
        failure(r, SV_LUA_INVALID, "c-init.lua", "Invalid Lua school/spell counts"); return r->result;
    }
    r->schools = calloc((size_t)schools ? (size_t)schools : 1, sizeof(*r->schools));
    r->spells = calloc((size_t)spells ? (size_t)spells : 1, sizeof(*r->spells));
    r->spell_refs = malloc(((size_t)spells ? (size_t)spells : 1) * sizeof(*r->spell_refs));
    if (!r->schools || !r->spells || !r->spell_refs) {
        failure(r, SV_LUA_MEMORY, "c-init.lua", "Lua school/spell allocation failed"); return r->result;
    }
    r->school_count = (size_t)schools; r->spell_count = (size_t)spells;
    for (size_t i = 0; i < r->spell_count; ++i) r->spell_refs[i] = LUA_NOREF;
    for (size_t i = 0; i < r->school_count; ++i) {
        if (!call_index(r, "finish_school", i)) return r->result;
        if (!r->schools[i].name) {
            failure(r, SV_LUA_INVALID, "finish_school", "School finalizer did not produce school data");
            return r->result;
        }
    }
    for (size_t i = 0; i < r->spell_count; ++i) {
        if (!call_index(r, "finish_spell", i)) return r->result;
        if (!r->spells[i].name || r->spell_refs[i] == LUA_NOREF) {
            failure(r, SV_LUA_INVALID, "finish_spell", "Spell finalizer did not produce spell data");
            return r->result;
        }
    }
    r->opened = true;
    return r->result;
}
SvLuaResult sv_lua_reload(SvLuaRuntime *r, SvLuaFeatures features)
{
    if (!r) return (SvLuaResult){.status = SV_LUA_INVALID};
    sv_lua_clear(r); return sv_lua_open(r, features);
}
const SvLuaResult *sv_lua_result(const SvLuaRuntime *r) { return r ? &r->result : NULL; }
bool sv_lua_ready(const SvLuaRuntime *r) { return r && r->opened && r->result.status == SV_LUA_OK; }
/* Raw global lookup avoids c-init's undefined-global tag method: missing typed
 * metadata is an ordinary semantic error, not an unprotected Lua longjmp. */
static void raw_global(lua_State *L, const char *name)
{ lua_getglobals(L); lua_pushstring(L, name); lua_rawget(L, -2); lua_remove(L, -2); }
SvLuaStatus sv_lua_number(SvLuaRuntime *r, const char *name, long *out)
{
    if (!r || !r->state || !name || !out) return SV_LUA_NOT_READY;
    int top = lua_gettop(r->state);
    raw_global(r->state, name);
    SvLuaStatus status = lua_type(r->state, -1) == LUA_TNUMBER ? SV_LUA_OK : SV_LUA_INVALID;
    if (status == SV_LUA_OK) *out = lua_tonumber(r->state, -1);
    lua_settop(r->state, top); return status;
}
SvLuaStatus sv_lua_string(SvLuaRuntime *r, const char *name, int index, char *out, size_t capacity)
{
    if (!r || !r->state || !name || !out || !capacity) return SV_LUA_NOT_READY;
    int top = lua_gettop(r->state);
    raw_global(r->state, name);
    if (index > 0) {
        if (!lua_istable(r->state, -1)) { lua_settop(r->state, top); return SV_LUA_INVALID; }
        lua_rawgeti(r->state, -1, index);
    }
    SvLuaStatus status = lua_type(r->state, -1) == LUA_TSTRING ? SV_LUA_OK : SV_LUA_INVALID;
    if (status == SV_LUA_OK && (lua_strlen(r->state, -1) >= capacity ||
        !text_copy(out, capacity, lua_tostring(r->state, -1)))) status = SV_LUA_INVALID;
    lua_settop(r->state, top); return status;
}
static const char *guide_counts[] = {"guide_races", "guide_classes", "guide_skills", "guide_schools", "guide_spells"};
static const char *guide_entries[] = {"guide_race", "guide_class", "guide_skill", "guide_school", "guide_spell"};
SvLuaStatus sv_lua_guide_count(SvLuaRuntime *r, SvLuaGuideKind kind, size_t *out)
{
    if ((unsigned)kind >= 5 || !out) return SV_LUA_INVALID;
    long count;
    SvLuaStatus status = sv_lua_number(r, guide_counts[kind], &count);
    if (status != SV_LUA_OK) return status;
    if (count < 0 || count > DATA_LIMIT) return SV_LUA_INVALID;
    *out = (size_t)count; return SV_LUA_OK;
}
SvLuaStatus sv_lua_guide_entry(SvLuaRuntime *r, SvLuaGuideKind kind, size_t index, char *out, size_t capacity)
{
    size_t count;
    SvLuaStatus status = sv_lua_guide_count(r, kind, &count);
    if (status != SV_LUA_OK) return status;
    if (index >= count) return SV_LUA_INVALID;
    return sv_lua_string(r, guide_entries[kind], (int)index + 1, out, capacity);
}
size_t sv_lua_school_count(const SvLuaRuntime *r) { return r ? r->school_count : 0; }
size_t sv_lua_spell_count(const SvLuaRuntime *r) { return r ? r->spell_count : 0; }
const SvLuaSchool *sv_lua_school(const SvLuaRuntime *r, size_t i)
{ return r && i < r->school_count ? &r->schools[i] : NULL; }
const SvLuaSpell *sv_lua_spell(const SvLuaRuntime *r, size_t i)
{ return r && i < r->spell_count ? &r->spells[i] : NULL; }

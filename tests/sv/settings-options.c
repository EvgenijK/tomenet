#define _XOPEN_SOURCE 700
#include "settings/options.h"
#include "settings/preferences.h"
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
    assert(snprintf(path, sizeof(path), "%s/%s", root, name) < (int)sizeof(path));
    FILE *file = fopen(path, "w"); assert(file);
    assert(fputs(text, file) >= 0); assert(!fclose(file));
}
static int value(const SvSettingsOptions *options, const char *name)
{
    int slot = sv_settings_option_slot(name); assert(slot >= 0);
    return options->values[slot];
}
static void unchanged(const char *root, const char *name, const char *expected)
{
    char path[1024], bytes[4096];
    assert(snprintf(path, sizeof(path), "%s/%s", root, name) < (int)sizeof(path));
    FILE *file = fopen(path, "rb"); assert(file);
    size_t n = fread(bytes, 1, sizeof(bytes), file);
    assert(!ferror(file) && !fclose(file));
    assert(n == strlen(expected) && !memcmp(bytes, expected, n));
}
static const SvPreferenceMacro *macro(const SvPreferences *preferences, const char *trigger)
{
    for (size_t i = 0; i < sv_preferences_macro_count(preferences); ++i) {
        const SvPreferenceMacro *entry = sv_preferences_macro(preferences, i);
        if (entry->trigger_size == strlen(trigger) &&
            !memcmp(entry->trigger, trigger, entry->trigger_size)) return entry;
    }
    return NULL;
}
static void preference_core(const char *user, const char *library, const char *profile,
                            const char *shared, const char *bundled)
{
    static const char mixed[] =
        "X:censor_swearing\nY:instant_retaliator\n"
        "R:3:4:65\nK:4:5:66\nF:5:6:67\nU:6:7:68\n"
        "A:\\e\\x41^B\\101\nP:normal\n%:nested.prf\n"
        "Y:censor_swearing\nP:after\nH:hybrid\nC:command\nP:gone\nD:gone\n"
        "W:1:2\n#:hello {rred {{brace\n#:{+chat\n!:first\n?:second\n";
    static const char nested[] = "%:linked.opt\nA:\\w12\\W34tail\nP:nested\n";
    static const char linked[] = "Y:censor_swearing\n";
    static const char successive[] = "R:3:8:75\nP:later\nP:excluded\n";
    static const char body[] = "?:skip\n!:keep\n";
    static const char filtered[] =
        "%:nested.prf\nX:censor_swearing\n#:skip\n!:skip\n?:skip\nP:filtered\n";
    static const char partial[] =
        "%:missing-include.prf\nbogus\n%:partial.prf\nY:censor_swearing\n";
    char boundary[1026];
    memset(boundary, 'z', 1024); boundary[0] = '!'; boundary[1] = ':';
    boundary[1024] = '\n'; boundary[1025] = 0;
    write_file(shared, "mixed.prf", mixed);
    write_file(bundled, "nested.prf", nested); /* Shared role, bundled physical source. */
    write_file(profile, "linked.opt", linked);
    write_file(shared, "graphics-check#1.prf", successive);
    write_file(shared, "body.prf", body);
    write_file(shared, "filtered.prf", filtered);
    write_file(shared, "partial.prf", partial);
    write_file(shared, "boundary.prf", boundary);
    SvPreferences *preferences = sv_preferences_create(user, library, NULL); assert(preferences);
    SvPreferenceContext context = sv_preferences_default_context();
    context.generation = 17;
    SvPreferenceResult result = sv_preferences_load(preferences, SV_OPTION_SHARED,
        "mixed.prf", SV_PREF_EXPLICIT, &context);
    assert(result.status == SV_PREF_CLEAN && result.files == 3);
    assert(value(sv_preferences_options(preferences), "censor_swearing") == 1);
    assert(value(sv_preferences_options(preferences), "new_retaliator") == 0);
    static const unsigned char first_action[] = {27, 'A', 2, 'A'};
    const SvPreferenceMacro *entry = macro(preferences, "normal"); assert(entry);
    assert(entry->kind == SV_PREF_MACRO_NORMAL && entry->action_size == sizeof(first_action));
    assert(!memcmp(entry->action, first_action, sizeof(first_action)));
    const SvPreferenceMacro *included = macro(preferences, "nested"); assert(included);
    assert(included->source.owner == SV_FILE_BUNDLED && strstr(included->source.path, library));
    static const unsigned char included_action[] = "`12\03634tail"; /* WAIT=96, XWAIT=30. */
    assert(included->action_size == sizeof(included_action)-1 &&
           !memcmp(included->action, included_action, included->action_size));
    static const struct { const char *name; SvPreferenceMacroKind kind; } macros[] = {
        {"after", SV_PREF_MACRO_NORMAL}, {"hybrid", SV_PREF_MACRO_HYBRID},
        {"command", SV_PREF_MACRO_COMMAND}
    };
    for (size_t i = 0; i < sizeof(macros)/sizeof(*macros); ++i) {
        entry = macro(preferences, macros[i].name); assert(entry);
        assert(entry->kind == macros[i].kind && entry->action_size == included->action_size);
        assert(!memcmp(entry->action, included->action, entry->action_size));
    }
    assert(!macro(preferences, "gone"));
    assert(sv_preferences_mapping_count(preferences) == 4); /* W creates no Term/mapping. */
    static const char families[] = "RKFU";
    for (size_t i = 0; i < sizeof(families) - 1; ++i) {
        const SvPreferenceMapping *mapping = sv_preferences_mapping(preferences, i); assert(mapping);
        assert(mapping->family == families[i] && mapping->index == i + 3);
        assert(mapping->attribute == i + 4 && mapping->character == i + 65);
    }
    bool profile_origin = false, fallback_origin = false, window_record = false;
    for (size_t i = 0; i < sv_preferences_record_count(preferences); ++i) {
        const SvPreferenceRecord *record = sv_preferences_record(preferences, i);
        if (strstr(record->source.path, "/linked.opt"))
            profile_origin = record->source.owner == SV_FILE_PROFILE && strstr(record->source.path, profile);
        if (strstr(record->source.path, "/nested.prf"))
            fallback_origin = record->role == SV_OPTION_SHARED && record->source.owner == SV_FILE_BUNDLED;
        if (!strcmp(record->text, "W:1:2")) window_record = true;
    }
    assert(profile_origin && fallback_origin && window_record);
    unsigned messages = 0, actions = 0, censor_assignments = 0;
    static const unsigned char loud[] = "hello \377rred {brace";
    for (size_t i = 0; i < sv_preferences_effect_count(preferences); ++i) {
        const SvPreferenceEffect *effect = sv_preferences_effect(preferences, i);
        if (effect->kind == SV_PREF_MESSAGE) {
            if (!messages) assert(!effect->chat && effect->size == sizeof(loud)-1 &&
                                  !memcmp(effect->bytes, loud, effect->size));
            else assert(effect->chat && effect->size == 5 && !memcmp(effect->bytes, "\375chat", 5));
            ++messages;
        } else if (effect->kind == SV_PREF_ACTION) {
            assert(effect->generation == 17 && !effect->in_game);
            ++actions;
        } else if (effect->kind == SV_PREF_OPTION &&
                   effect->option_slot == sv_settings_option_slot("censor_swearing")) {
            assert(effect->option_value == (censor_assignments != 0));
            ++censor_assignments;
        }
    }
    assert(messages == 2 && actions == 2 && censor_assignments == 3);
    size_t size = 0;
    const unsigned char *queue = sv_preferences_queue(preferences, &size);
    static const unsigned char expected_queue[] = "second\035first\035";
    assert(size == sizeof(expected_queue)-1 && !memcmp(queue, expected_queue, size));
    assert(sv_preferences_effect_count(preferences) >= 4); /* Inspection does not consume. */
    sv_preferences_clear_effects(preferences);
    sv_preferences_queue(preferences, &size); assert(!size && !sv_preferences_effect_count(preferences));

    context.exclusive_trigger = (const unsigned char *)"later";
    context.exclusive_trigger_size = 5;
    result = sv_preferences_load(preferences, SV_OPTION_SHARED, "graphics-check#1.prf", SV_PREF_EXPLICIT, &context);
    assert(result.status == SV_PREF_CLEAN);
    entry = macro(preferences, "later"); included = macro(preferences, "nested"); assert(entry && included);
    assert(entry->action_size == included->action_size && !memcmp(entry->action, included->action, entry->action_size));
    assert(!macro(preferences, "excluded"));
    const SvPreferenceMapping *latest = sv_preferences_mapping(preferences, 0);
    assert(sv_preferences_mapping_count(preferences) == 4 && latest->family == 'R');
    assert(latest->character == 75 && latest->attribute == 8 && latest->subtileset == 1);
    context.exclusive_trigger = NULL; context.exclusive_trigger_size = 0;
    context.bodymonster = true;
    assert(sv_preferences_load(preferences, SV_OPTION_SHARED, "body.prf", SV_PREF_EXPLICIT, &context).status == SV_PREF_CLEAN);
    queue = sv_preferences_queue(preferences, &size);
    assert(size == 5 && !memcmp(queue, "keep\035", size));
    sv_preferences_clear_effects(preferences);
    context.bodymonster = false; context.macro_only = true;
    assert(sv_preferences_load(preferences, SV_OPTION_SHARED, "filtered.prf", SV_PREF_EXPLICIT, &context).status == SV_PREF_CLEAN);
    assert(value(sv_preferences_options(preferences), "censor_swearing") == 1 && macro(preferences, "filtered"));
    assert(!sv_preferences_effect_count(preferences));
    sv_preferences_queue(preferences, &size); assert(!size);
    context.macro_only = false;
    assert(sv_preferences_load(preferences, SV_OPTION_SHARED, "missing.prf", SV_PREF_OPTIONAL, &context).status == SV_PREF_CLEAN);
    assert(sv_preferences_load(preferences, SV_OPTION_SHARED, "missing.prf", SV_PREF_EXPLICIT, &context).status == SV_PREF_UNAVAILABLE);
    assert(sv_settings_option_set(sv_preferences_options_mutable(preferences), "censor_swearing", false));
    result = sv_preferences_load(preferences, SV_OPTION_SHARED, "partial.prf", SV_PREF_EXPLICIT, &context);
    assert(result.status == SV_PREF_PARTIAL && result.diagnostics >= 3);
    assert(value(sv_preferences_options(preferences), "censor_swearing") == 1); /* Valid tail survives errors. */
    bool missing = false, invalid = false, cycle = false;
    for (size_t i = 0; i < sv_preferences_diagnostic_count(preferences); ++i) {
        const SvPreferenceDiagnostic *diagnostic = sv_preferences_diagnostic(preferences, i);
        missing |= diagnostic->kind == SV_PREF_DIAG_MISSING;
        invalid |= diagnostic->kind == SV_PREF_DIAG_INVALID;
        cycle |= diagnostic->kind == SV_PREF_DIAG_CYCLE;
    }
    assert(missing && invalid && cycle);
    sv_preferences_clear_effects(preferences);
    result = sv_preferences_load(preferences, SV_OPTION_SHARED, "boundary.prf", SV_PREF_EXPLICIT, &context);
    assert(result.status == SV_PREF_PARTIAL);
    /* Baseline buf[1023]=0 limits the whole encoded record, including '!:',
     * before decoding. Retain source bytes, never execute the discarded tail. */
    assert(sv_preferences_effect_count(preferences) == 1);
    const SvPreferenceEffect *limited = sv_preferences_effect(preferences, 0);
    assert(limited->kind == SV_PREF_ACTION && limited->size == 1021);
    for (size_t i = 0; i < limited->size; ++i) assert(limited->bytes[i] == 'z');
    queue = sv_preferences_queue(preferences, &size);
    assert(size == 1022 && queue[1021] == 29 && !memcmp(queue, limited->bytes, 1021));
    const SvPreferenceRecord *retained = sv_preferences_record(preferences, sv_preferences_record_count(preferences)-1);
    assert(strlen(retained->text) == 1024);
    bool limit = false;
    for (size_t i = 0; i < sv_preferences_diagnostic_count(preferences); ++i)
        limit |= sv_preferences_diagnostic(preferences, i)->kind == SV_PREF_DIAG_LIMIT;
    assert(limit);
    sv_preferences_destroy(preferences);
    const struct { const char *root, *name, *text; } sources[] = {
        {shared,"mixed.prf",mixed}, {bundled,"nested.prf",nested},
        {profile,"linked.opt",linked}, {shared,"graphics-check#1.prf",successive},
        {shared,"body.prf",body}, {shared,"filtered.prf",filtered},
        {shared,"partial.prf",partial}, {shared,"boundary.prf",boundary}
    };
    for (size_t i = 0; i < sizeof(sources)/sizeof(*sources); ++i) {
        char path[1024];
        unchanged(sources[i].root, sources[i].name, sources[i].text);
        assert(snprintf(path, sizeof(path), "%s/%s", sources[i].root, sources[i].name) < (int)sizeof(path));
        assert(!unlink(path));
    }
}
static void packets(const SvSettingsOptions *options)
{
    static const struct { int version[6]; size_t count; bool resources, location; } cases[] = {
        {{4,5,5,0,0,0},96,false,false}, {{4,5,5,0,0,1},128,false,false},
        {{4,5,8,1,0,1},128,false,true}, {{4,5,8,1,0,2},154,false,true},
        {{4,8,1,2,0,0},154,true,true}, {{4,9,1,0,0,0},154,true,true},
        {{4,9,1,0,0,1},199,true,true}
    };
    for (size_t i = 0; i < sizeof(cases)/sizeof(*cases); ++i) {
        unsigned char bytes[32768], overview[64] = {PKT_SERVERDETAILS};
        const unsigned char iaddr[6] = {0};
        SvLogin *login = sv_login_create(cases[i].version, iaddr); assert(login);
        assert(sv_login_take_output(login, bytes, sizeof(bytes)).result == SV_OK);
        size_t n = 17;
        overview[n++] = PKT_LOGIN;
        /* Empty overview row: mode, colour, name, level/race/class, location. */
        memset(overview + n, 0, 10); n += 10;
        if (cases[i].location) overview[n++] = 0;
        assert(sv_login_receive(login, overview, n) == SV_OK);
        assert(sv_login_create_character(login, "Alice", false) == SV_OK);
        assert(sv_login_take_output(login, bytes, sizeof(bytes)).result == SV_OK);
        const unsigned char accepted = E_NEED_INFO;
        assert(sv_login_receive(login, &accepted, 1) == SV_OK);
        assert(sv_login_set_options(login, options) == SV_OK);
        assert(sv_login_send_options(login) == SV_OK);
        SvOutput out = sv_login_take_output(login, bytes, sizeof(bytes));
        assert(out.result == SV_OK && out.size == cases[i].count + 1);
        assert(bytes[0] == PKT_OPTIONS);
        assert(!memcmp(bytes + 1, options->values, cases[i].count));
        const int16_t stats[6] = {0};
        assert(sv_login_start_play(login, 0, 0, 0, 0, stats, "f") == SV_OK);
        out = sv_login_take_output(login, bytes, sizeof(bytes));
        size_t offset = 25 + (cases[i].resources ? 16 : 0);
        assert(out.result == SV_OK && bytes[0] == PKT_PLAY);
        assert(!memcmp(bytes + offset, options->values, cases[i].count));
        assert(sv_login_set_options(login, options) == SV_INVALID);
        sv_login_destroy(login);
    }
}
int main(void)
{
    char user[] = "/tmp/sv-options-XXXXXX", library[] = "/tmp/sv-options-lib-XXXXXX";
    assert(mkdtemp(user) && mkdtemp(library));
    char profile[1024], shared[1024], bundled[1024];
    snprintf(profile, sizeof(profile), "%s/sv", user);
    snprintf(shared, sizeof(shared), "%s/user", user);
    snprintf(bundled, sizeof(bundled), "%s/user", library);
    assert(!mkdir(profile, 0700) && !mkdir(shared, 0700) && !mkdir(bundled, 0700));
    write_file(bundled, "pref.prf", "%:options.prf\n");
    write_file(bundled, "options.prf", "X:censor_swearing\nY:rogue_like_commands\n");
    write_file(shared, "options.prf", "Y:censor_swearing\n");
    write_file(shared, "global.opt", "Y:censor_swearing\n");
    write_file(profile, "options.prf", "X:censor_swearing\r\nX:rogue_like_commands\n");
    write_file(profile, "global.opt", "Y:censor_swearing\nY:rogue_like_commands\n");
    write_file(shared, "pref-sv.prf", "X:censor_swearing\n%:global.opt\n");
    write_file(profile, "global-sv.opt", "X:rogue_like_commands\n");
    SvSettingsOptions base, alice, bob;
    assert(sv_settings_options_load(&base, user, library));
    assert(value(&base, "censor_swearing") == 1); /* include routes S/global.opt */
    assert(value(&base, "rogue_like_commands") == 0); /* last system layer */
    write_file(profile, "Alice.opt", "X:censor_swearing\nY:rogue_like_commands\n");
    assert(sv_settings_options_character(&alice, &base, user, library, "Alice"));
    assert(value(&alice, "censor_swearing") == 0 && value(&alice, "rogue_like_commands") == 1);
    assert(sv_settings_options_character(&bob, &base, user, library, "Bob"));
    assert(!memcmp(bob.values, base.values, sizeof(base.values))); /* retry/reset */
    packets(&alice);
    static const char *old_names[] = {"recall_flicker", "autoloot_depth", "autoloot_off", "kind_diz",
        "auto_inscribe", "hilite_chat", "hibeep_chat", "view_animated_lite", "view_lite_extra",
        "no_lite_fainting", "hilite_player", "colourize_prices", "sp_huge_bar", "auto_insc_off", "stack_allow_wands"};
    static const char *new_names[] = {"subterm_flicker", "autoloot_dunonly", "autoloot_dunonly", "add_kind_diz",
        "auto_inscr_server", "highlight_chat", "highbeep_chat", "view_animated_light", "view_light_extra",
        "no_light_fainting", "highlight_player", "colourize_bignum", "sn_huge_bar", "auto_inscr_off", "stack_allow_devices"};
    for (size_t i = 0; i < sizeof(old_names)/sizeof(*old_names); ++i) {
        for (int enabled = 0; enabled < 2; ++enabled) {
            char record[256]; snprintf(record, sizeof(record), "%c:%s\n", enabled ? 'Y' : 'X', old_names[i]);
            write_file(profile, "alias.opt", record);
            assert(sv_settings_options_file(&base, user, library, SV_OPTION_PROFILE, "alias.opt"));
            assert(value(&base, new_names[i]) == enabled);
        }
    }
    write_file(profile, "alias.opt", "Y:instant_retaliator\nY:basic_players_col\nY:basic_players\nY:view_reduce_lite\n");
    assert(sv_settings_options_file(&base, user, library, SV_OPTION_PROFILE, "alias.opt"));
    assert(value(&base, "new_retaliator") == 0 && value(&base, "basic_players_symb") == 1 && value(&base, "basic_players_col") == 0);
    write_file(profile, "alias.opt", "X:instant_retaliator\nY:basic_players_col\nX:consistent_players\nX:view_reduce_lite\nY:future_option\n");
    assert(!sv_settings_options_file(&base, user, library, SV_OPTION_PROFILE, "alias.opt"));
    assert(value(&base, "new_retaliator") == 1 && value(&base, "basic_players_symb") == 0 && value(&base, "basic_players_col") == 0);
    assert(strstr(base.records, "Y:future_option"));
    write_file(profile, "cycle.opt", "%:cycle.opt\nY:censor_swearing\n");
    assert(!sv_settings_options_file(&base, user, library, SV_OPTION_PROFILE, "cycle.opt"));
    assert(value(&base, "censor_swearing") == 1);
    write_file(profile, "escape.opt", "%:../user/global.opt\n");
    assert(!sv_settings_options_file(&base, user, library, SV_OPTION_PROFILE, "escape.opt"));
    char unreadable[1024];
    assert(snprintf(unreadable, sizeof(unreadable), "%s/sv/unreadable.opt", user) < (int)sizeof(unreadable));
    assert(!mkdir(unreadable, 0700));
    assert(!sv_settings_options_file(&base, user, library, SV_OPTION_PROFILE, "unreadable.opt"));
    preference_core(user, library, profile, shared, bundled);
    /* The production readers leave original bytes and names untouched. */
    char path[1024], contents[128];
    snprintf(path, sizeof(path), "%s/Alice.opt", profile);
    FILE *file = fopen(path, "r"); assert(file);
    size_t n = fread(contents, 1, sizeof(contents) - 1, file); contents[n] = 0;
    assert(!fclose(file));
    assert(!strcmp(contents, "X:censor_swearing\nY:rogue_like_commands\n"));
    const char *profile_files[] = {"options.prf", "global.opt", "global-sv.opt", "Alice.opt", "alias.opt", "cycle.opt", "escape.opt"};
    for (size_t i = 0; i < sizeof(profile_files)/sizeof(*profile_files); ++i) {
        snprintf(path, sizeof(path), "%s/%s", profile, profile_files[i]); assert(!unlink(path));
    }
    assert(!rmdir(unreadable)); assert(!rmdir(profile));
    const char *shared_files[] = {"options.prf", "global.opt", "pref-sv.prf"};
    for (size_t i = 0; i < sizeof(shared_files)/sizeof(*shared_files); ++i) {
        snprintf(path, sizeof(path), "%s/%s", shared, shared_files[i]); assert(!unlink(path));
    }
    assert(!rmdir(shared)); assert(!rmdir(user));
    snprintf(path, sizeof(path), "%s/options.prf", bundled); assert(!unlink(path));
    snprintf(path, sizeof(path), "%s/pref.prf", bundled); assert(!unlink(path));
    assert(!rmdir(bundled)); assert(!rmdir(library));
    puts("settings options: layers, aliases, PRF core/effects/includes and versioned wire PASS");
    return 0;
}

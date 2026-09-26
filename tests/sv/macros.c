#include "app.h"
#include "preferences.h"
#include "preferences-runtime.h"
#include "input/native-macro-loader.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int version[6] = {4, 9, 4, 0, 0, 0};
typedef struct {
    unsigned messages, actions, warnings;
    unsigned char action[64];
    size_t action_size;
    unsigned char message[64];
    size_t message_size;
    SvPrefOwner owner;
    char warning_file[512], warning_text[640];
    size_t warning_line;
} Effects;
static void effect(void *context, SvPrefEffectKind kind, SvPrefOwner owner,
                   const char *file, size_t line, const unsigned char *bytes, size_t size)
{
    Effects *effects = context;
    assert(file && (line || kind == SV_PREF_WARNING));
    effects->owner = owner;
    if (kind == SV_PREF_WARNING) {
        ++effects->warnings;
        snprintf(effects->warning_file, sizeof(effects->warning_file), "%s", file);
        snprintf(effects->warning_text, sizeof(effects->warning_text), "%.*s", (int)size, bytes);
        effects->warning_line = line;
    }
    else if (kind == SV_PREF_MESSAGE) {
        assert(size <= sizeof(effects->message));
        memcpy(effects->message, bytes, size);
        effects->message_size = size;
        ++effects->messages;
    }
    else if (kind == SV_PREF_ACTION) {
        assert(size <= sizeof(effects->action));
        memcpy(effects->action, bytes, size);
        effects->action_size = size;
        ++effects->actions;
    }
}
static void expect_output(SvApp *app, const unsigned char *expected, size_t size)
{
    unsigned char actual[64] = {0};
    SvOutput output = sv_app_take_output(app, sv_app_view(app).generation, actual, sizeof(actual));
    if (output.result != SV_OK || output.size != size) {
        fprintf(stderr, "output mismatch result=%d size=%zu expected=%zu\n", output.result, output.size, size);
        for (size_t i = 0; i < output.size; ++i) fprintf(stderr, "%u,", actual[i]);
        fprintf(stderr, "\n");
    }
    assert(output.result == SV_OK && output.size == size);
    if (memcmp(actual, expected, size)) {
        for (size_t i = 0; i < size; ++i) fprintf(stderr, "%zu expected=%u actual=%u\n", i, expected[i], actual[i]);
    }
    assert(!memcmp(actual, expected, size));
    assert(sv_app_take_output(app, sv_app_view(app).generation, actual, sizeof(actual)).result == SV_WAITING);
}
static void no_output(SvApp *app)
{
    unsigned char actual[64];
    assert(sv_app_take_output(app, sv_app_view(app).generation, actual, sizeof(actual)).result == SV_WAITING);
}
int main(int argc, char **argv)
{
    assert(argc == 3);
    assert(SDL_Init(0));
    SvOptions options;
    SvMacroSet *macros = calloc(1, sizeof(*macros));
    assert(macros);
    Effects effects = {0};
    SvPreferences prefs = {.user_root = argv[1], .library_root = argv[2],
        .options = &options, .macros = macros,
        .sink = {.context = &effects, .effect = effect}};
    SvPrefReport report = sv_preferences_bootstrap(&prefs);
    assert(report.complete && report.files >= 2);
    bool enabled;
    assert(sv_options_get(&options, "censor_swearing", &enabled) && enabled);
    assert(sv_options_get(&options, "ring_bell", &enabled) && enabled);
    assert(sv_options_get(&options, "new_retaliator", &enabled) && !enabled);
    assert(effects.messages == 1 && effects.owner == SV_PREF_USER);
    assert(effects.message_size == 6 && effects.message[0] == 253 &&
           !memcmp(effects.message + 1, "hello", 5));
    assert(prefs.keymap_command['G'] == ';' && prefs.keymap_direction['G'] == 8);
    report = sv_preferences_load_named(&prefs, "pref.prf", true);
    assert(report.complete && report.files == 3);
    assert(sv_options_get(&options, "ring_bell", &enabled) && !enabled);
    report = sv_preferences_character(&prefs, "Hero", "Human", "Maiar", "Warrior", "Wolf");
    assert(report.complete && report.files == 6);
    assert(sv_options_get(&options, "censor_swearing", &enabled) && !enabled);
    assert(effects.actions == 1 && effects.action_size == 2 &&
           effects.action[0] == 'N' && effects.action[1] == 13);
    report = sv_preferences_load_named(&prefs, "cycle.prf", true);
    assert(!report.complete && report.warnings == 1);
    assert(!strcmp(effects.warning_file, "cycle.prf") && effects.warning_line == 1 &&
           strstr(effects.warning_text, "cycle.prf"));
    report = sv_preferences_load_named(&prefs, "missing.prf", true);
    assert(!report.complete && report.warnings == 1);
    report = sv_preferences_load_named(&prefs, "../outside.prf", true);
    assert(!report.complete && report.warnings == 1);
    report = sv_preferences_load_named(&prefs, "invalid.prf", true);
    assert(!report.complete && report.warnings == 2);
    /* The include diagnostic retains its parent path and line. */
    report = sv_preferences_load_named(&prefs, "include-only.prf", true);
    assert(!report.complete && report.warnings == 1 &&
           !strcmp(effects.warning_file, "include-only.prf") &&
           effects.warning_line == 1 && strstr(effects.warning_text, "missing.prf"));
    bool retained = false;
    for (size_t i = 0; i < macros->count; ++i)
        if (macros->definitions[i].trigger_size == 1 &&
            macros->definitions[i].trigger[0] == 'r') {
            retained = macros->definitions[i].action_size == 1 &&
                       macros->definitions[i].action[0] == 'C';
        }
    assert(retained);
    report = sv_preferences_load_class(&prefs, "Warrior");
    assert(report.complete && report.files == 1);
    prefs.body_macros = true;
    unsigned before_actions = effects.actions;
    report = sv_preferences_load_named(&prefs, "body.prf", true);
    assert(report.complete && effects.actions == before_actions + 1 &&
           effects.action_size == 1 && effects.action[0] == 'N');
    prefs.body_macros = false;
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    for (size_t i = 0; i < macros->count; ++i) {
        const SvMacroDefinition *entry = &macros->definitions[i];
        assert(sv_app_define_macro(app, entry->trigger, entry->trigger_size,
                                   entry->action, entry->action_size, entry->kind) == SV_OK);
    }
    uint64_t generation = sv_app_view(app).generation;
    assert(sv_app_queue_macro_action(app, (const unsigned char *)"1", 1) == SV_OK);
    assert(sv_app_queue_macro_action(app, (const unsigned char *)"2", 1) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"c", 1) == SV_OK);
    const unsigned char queued_actions[] = {70, 2, 70, 1, 156, 'c'};
    expect_output(app, queued_actions, sizeof(queued_actions));
    assert(sv_app_keymap_record(app, 'G', prefs.keymap_command['G'],
                                prefs.keymap_direction['G']) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"G", 1) == SV_OK);
    const unsigned char mapped_walk[] = {70, 8};
    expect_output(app, mapped_walk, sizeof(mapped_walk));
    const unsigned char local_message[] = {253, 'h', 'e', 'l', 'l', 'o'};
    assert(sv_app_local_message(app, generation, local_message, sizeof(local_message)) == SV_OK);
    SvMessage delivered;
    assert(sv_app_take_message(app, generation, &delivered) == SV_OK &&
           delivered.length == sizeof(local_message) &&
           !memcmp(delivered.bytes, local_message, sizeof(local_message)));
    assert(sv_app_physical(app, generation, (const unsigned char *)"a", 1) == SV_OK);
    no_output(app); /* a is a prefix of ab. */
    assert(sv_app_physical(app, generation, (const unsigned char *)"b", 1) == SV_OK);
    const unsigned char longest[] = {156, 'B'};
    expect_output(app, longest, sizeof(longest));
    assert(sv_app_physical(app, generation, (const unsigned char *)"a", 1) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"c", 1) == SV_OK);
    const unsigned char pushback[] = {156, 'A', 156, 'c'};
    expect_output(app, pushback, sizeof(pushback));
    assert(sv_app_physical(app, generation, (const unsigned char *)"w", 1) == SV_OK);
    const unsigned char before_wait[] = {156, 'N'};
    expect_output(app, before_wait, sizeof(before_wait));
    uint64_t external_owner = 0;
    assert(sv_app_begin_confirmation(app, generation, &external_owner) == SV_BUSY);
    assert(sv_app_macro_frame(app, generation, 200, 16) == SV_OK);
    no_output(app);
    const unsigned char interleaved[] = {46, 'n', 'e', 't', 0};
    assert(sv_app_receive(app, generation, interleaved, sizeof(interleaved)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_view(app).messages.count == 2);
    const unsigned char confirm[] = {146, 42};
    assert(sv_app_receive(app, generation, confirm, sizeof(confirm)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_macro_frame(app, generation, 201, 16) == SV_OK);
    const unsigned char after_wait[] = {156, 'S'};
    expect_output(app, after_wait, sizeof(after_wait));
    assert(sv_app_begin_confirmation(app, generation, &external_owner) == SV_OK);
    unsigned char unused_confirm;
    assert(sv_app_take_confirmation(app, generation, external_owner, &unused_confirm) == SV_WAITING);
    assert(sv_app_end_confirmation(app, generation, external_owner) == SV_OK);
    assert(sv_app_macro_frame(app, generation, 202, 16) == SV_OK);
    no_output(app); /* Confirmation is not replayed after a redraw. */
    assert(sv_app_physical(app, generation, (const unsigned char *)"x", 1) == SV_OK);
    const unsigned char before_xwait[] = {156, 'Q'};
    expect_output(app, before_xwait, sizeof(before_xwait));
    assert(sv_app_physical(app, generation, (const unsigned char *)" ", 1) == SV_OK);
    const unsigned char after_xwait[] = {156, 'R'};
    expect_output(app, after_xwait, sizeof(after_xwait));
    assert(sv_app_physical(app, generation, (const unsigned char *)"x", 1) == SV_OK);
    expect_output(app, before_xwait, sizeof(before_xwait));
    assert(sv_app_physical(app, generation, (const unsigned char *)"v", 1) == SV_OK);
    no_output(app);
    assert(sv_app_macro_frame(app, generation, 702, 16) == SV_OK);
    const unsigned char preserved[] = {156, 'R', 156, 'v'};
    expect_output(app, preserved, sizeof(preserved));
    assert(sv_app_physical(app, generation, (const unsigned char *)"x", 1) == SV_OK);
    expect_output(app, before_xwait, sizeof(before_xwait));
    assert(sv_app_macro_extended_waiting(app, generation));
    assert(sv_app_physical(app, generation, (const unsigned char *)"\033", 1) == SV_OK);
    assert(!sv_app_macro_extended_waiting(app, generation));
    assert(sv_app_macro_frame(app, generation, 1000, 16) == SV_OK);
    no_output(app);
    const unsigned char controlled_action[] = {
        '8', 28, 'Z', 28, 31, '_', 'F', 'F', '5', '2', 13, '8'};
    assert(sv_app_define_macro(app, (const unsigned char *)"M", 1,
                               controlled_action, sizeof(controlled_action),
                               SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"M", 1) == SV_OK);
    const unsigned char controlled_output[] = {70, 8, 156, 'Z', 70, 8};
    expect_output(app, controlled_output, sizeof(controlled_output));
    const unsigned char unmatched_trigger[] = {31, '_', 'N', 13};
    assert(sv_app_physical(app, generation, unmatched_trigger,
                           sizeof(unmatched_trigger)) == SV_WAITING);
    no_output(app);
    const unsigned char physical_default[] = {31, '_', 'N', 13, 28, '8', 28};
    assert(sv_app_physical(app, generation, physical_default,
                           sizeof(physical_default)) == SV_OK);
    const unsigned char walk[] = {70, 8};
    expect_output(app, walk, sizeof(walk));
    const unsigned char rematch_action[] = {'9'};
    assert(sv_app_define_macro(app, (const unsigned char *)"8", 1,
                               rematch_action, sizeof(rematch_action),
                               SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_physical(app, generation, physical_default,
                           sizeof(physical_default)) == SV_OK);
    expect_output(app, walk, sizeof(walk)); /* Default bypasses macro 8 -> 9. */
    assert(sv_app_physical(app, generation, (const unsigned char *)"a", 1) == SV_OK);
    no_output(app);
    assert(sv_app_physical(app, generation, physical_default,
                           sizeof(physical_default)) == SV_OK);
    const unsigned char prefix_then_default[] = {156, 'A', 70, 8};
    expect_output(app, prefix_then_default, sizeof(prefix_then_default));
    unsigned char long_action[18];
    memset(long_action, 'Z', sizeof(long_action));
    assert(sv_app_define_macro(app, (const unsigned char *)"L", 1,
                               long_action, sizeof(long_action), SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"L", 1) == SV_OK);
    unsigned char first_sixteen[32];
    for (size_t i = 0; i < 16; ++i) {
        first_sixteen[2 * i] = 156;
        first_sixteen[2 * i + 1] = 'Z';
    }
    expect_output(app, first_sixteen, sizeof(first_sixteen));
    assert(sv_app_physical(app, generation, physical_default,
                           sizeof(physical_default)) == SV_OK);
    const unsigned char ordered_default[] = {156, 'Z', 156, 'Z', 70, 8};
    expect_output(app, ordered_default, sizeof(ordered_default));
    /* Native Escape is owned by the wait: ordinary wait keeps its duration,
     * while extended wait cancels the queued action. */
    SvNativeInput wait_input;
    sv_native_input_begin(&wait_input, app);
    SDL_Event escape = {.type = SDL_EVENT_KEY_DOWN};
    escape.key.key = SDLK_ESCAPE;
    escape.key.scancode = SDL_SCANCODE_ESCAPE;
    escape.common.timestamp = wait_input.since_ns;
    assert(sv_app_physical(app, generation, (const unsigned char *)"w", 1) == SV_OK);
    expect_output(app, before_wait, sizeof(before_wait));
    assert(sv_app_macro_waiting(app, generation));
    assert(sv_native_input(&wait_input, app, &escape));
    const unsigned char waiting_default[] = {31, '_', 'N', 13, 28, '8', 28};
    assert(sv_app_physical(app, generation, waiting_default,
                           sizeof(waiting_default)) == SV_OK);
    assert(sv_app_macro_frame(app, generation, 1200, 16) == SV_OK);
    no_output(app);
    assert(sv_app_macro_frame(app, generation, 1500, 16) == SV_OK);
    const unsigned char after_wait_default[] = {156, 'S', 70, 8};
    expect_output(app, after_wait_default, sizeof(after_wait_default));
    assert(sv_app_physical(app, generation, (const unsigned char *)"x", 1) == SV_OK);
    expect_output(app, before_xwait, sizeof(before_xwait));
    assert(sv_native_input(&wait_input, app, &escape));
    assert(!sv_app_macro_waiting(app, generation));
    assert(sv_app_macro_frame(app, generation, 2000, 16) == SV_OK);
    no_output(app);
    assert(sv_app_physical(app, generation, (const unsigned char *)"x", 1) == SV_OK);
    expect_output(app, before_xwait, sizeof(before_xwait));
    escape.key.mod = SDL_KMOD_CTRL; /* encoded trigger plus 28, Escape, 28 */
    assert(sv_native_input(&wait_input, app, &escape));
    assert(!sv_app_macro_waiting(app, generation));
    no_output(app);
    escape.key.mod = 0;
    assert(sv_app_delete_macro(app, (const unsigned char *)"8", 1) == SV_OK);
    const unsigned char movement_action[] = {'8'};
    assert(sv_app_define_macro(app, (const unsigned char *)"d", 1,
                               movement_action, sizeof(movement_action), SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"d", 1) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    const unsigned char up_trigger[] = {31, '_', 'F', 'F', '5', '2', 13};
    assert(sv_app_define_macro(app, up_trigger, sizeof(up_trigger),
                               movement_action, sizeof(movement_action), SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_physical(app, generation, up_trigger, sizeof(up_trigger)) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    const unsigned char up_with_default[] = {31, '_', 'F', 'F', '5', '2', 13, 28, 'Z', 28};
    assert(sv_app_physical(app, generation, up_with_default,
                           sizeof(up_with_default)) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    const unsigned char chat_action[] = {':', 'h', 'i', 13};
    assert(sv_app_define_macro(app, (const unsigned char *)"t", 1,
                               chat_action, sizeof(chat_action), SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"t", 1) == SV_OK);
    const unsigned char chat[] = {46, 'h', 'i', 0};
    expect_output(app, chat, sizeof(chat));
    assert(sv_app_physical(app, generation, (const unsigned char *)"\\", 1) == SV_OK);
    no_output(app);
    assert(sv_app_physical(app, generation, (const unsigned char *)"8", 1) == SV_OK);
    const unsigned char bypass[] = {156, '8'};
    expect_output(app, bypass, sizeof(bypass));
    assert(sv_app_physical(app, generation, (const unsigned char *)"^", 1) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"C", 1) == SV_OK);
    const unsigned char control[] = {156, 3};
    expect_output(app, control, sizeof(control));
    assert(sv_app_command_mode(app, true) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"h", 1) == SV_OK);
    const unsigned char rogue_walk[] = {70, 4};
    expect_output(app, rogue_walk, sizeof(rogue_walk));
    assert(sv_app_command_mode(app, false) == SV_OK);
    const unsigned char command_action[] = {'Y'}, hybrid_action[] = {'J'};
    assert(sv_app_define_macro(app, (const unsigned char *)"q", 1,
                               command_action, 1, SV_MACRO_COMMAND) == SV_OK);
    assert(sv_app_define_macro(app, (const unsigned char *)"p", 1,
                               hybrid_action, 1, SV_MACRO_HYBRID) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)":", 1) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"p", 1) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"\r", 1) == SV_OK);
    const unsigned char chat_hybrid_skipped[] = {46, 'p', 0};
    expect_output(app, chat_hybrid_skipped, sizeof(chat_hybrid_skipped));
    const unsigned char request[] = {184,0,0,0,7,'K','e','y','?',0};
    assert(sv_app_receive(app, generation, request, sizeof(request)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    SvAppView view = sv_app_view(app);
    assert(view.request.pending);
    assert(sv_app_accept_key(app, generation, view.request.sequence, 'q') == SV_OK);
    assert(sv_app_dispatch_input(app, 1).dispatched == 1);
    const unsigned char command_skipped[] = {184,0,0,0,7,'q'};
    expect_output(app, command_skipped, sizeof(command_skipped));
    const unsigned char request2[] = {184,0,0,0,8,'K','e','y','?',0};
    assert(sv_app_receive(app, generation, request2, sizeof(request2)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    view = sv_app_view(app);
    assert(sv_app_accept_key(app, generation, view.request.sequence, 'p') == SV_OK);
    assert(sv_app_dispatch_input(app, 1).dispatched == 1);
    const unsigned char hybrid_reply[] = {184,0,0,0,8,'J'};
    expect_output(app, hybrid_reply, sizeof(hybrid_reply));
    const unsigned char request_wait_action[] = {'N', 96, '9', '9', 'Y'};
    assert(sv_app_define_macro(app, (const unsigned char *)"v", 1,
                               request_wait_action, sizeof(request_wait_action),
                               SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_physical(app, generation, (const unsigned char *)"v", 1) == SV_OK);
    const unsigned char before_request_wait[] = {156, 'N'};
    expect_output(app, before_request_wait, sizeof(before_request_wait));
    const unsigned char request3[] = {184,0,0,0,9,'K','e','y','?',0};
    assert(sv_app_receive(app, generation, request3, sizeof(request3)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_macro_frame(app, generation, 1001, 16) == SV_OK);
    assert(sv_app_dispatch_input(app, 1).dispatched == 1);
    const unsigned char request_wait_reply[] = {184,0,0,0,9,'Y'};
    expect_output(app, request_wait_reply, sizeof(request_wait_reply));
    assert(sv_app_physical(app, generation, (const unsigned char *)"v", 1) == SV_OK);
    expect_output(app, before_request_wait, sizeof(before_request_wait));
    assert(sv_app_close(app) == SV_OK);
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_macro_frame(app, generation, 2000, 16) == SV_STALE);
    assert(sv_app_take_confirmation(app, generation, external_owner, &unused_confirm) == SV_STALE);
    no_output(app);
    assert(sv_app_destroy(app) == SV_OK);
    app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    SvPreferenceRuntime *runtime = sv_preference_runtime_create(app, argv[1], argv[2]);
    assert(runtime);
    assert(sv_preference_runtime_bootstrap(runtime, &report) == SV_OK);
    assert(sv_preference_runtime_include_count(runtime) == 1);
    const SvPrefIncludeOrigin *included = sv_preference_runtime_include(runtime, 0);
    assert(included && included->source_owner == SV_PREF_BUNDLED &&
           included->target_owner == SV_PREF_BUNDLED && included->source_line == 1 &&
           strstr(included->source_path, "/B/user/pref.prf") &&
           strstr(included->target_path, "/B/user/base.prf"));
    bool saw_bundled_option = false;
    size_t global_effects = 0;
    for (size_t i = 0; i < sv_preference_runtime_origin_count(runtime); ++i) {
        const SvPrefOrigin *item = sv_preference_runtime_origin(runtime, i);
        if (strstr(item->path, "/user/global.prf")) ++global_effects;
        if (item->kind == SV_PREF_OPTION && item->owner == SV_PREF_BUNDLED &&
            item->line == 1 && strstr(item->path, "/B/user/base.prf"))
            saw_bundled_option = true;
    }
    assert(saw_bundled_option && global_effects == 0);
    generation = sv_app_view(app).generation;
    assert(sv_preference_runtime_named(runtime, "manual.prf", &report) == SV_OK &&
           report.complete && report.files == 1);
    assert(sv_app_physical(app, generation, (const unsigned char *)"m", 1) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    assert(sv_app_physical(app, generation, (const unsigned char *)"M", 1) == SV_OK);
    no_output(app); /* Explicit S:77:0 overrides the earlier walk mapping. */
    assert(sv_app_physical(app, generation, (const unsigned char *)"X", 1) == SV_OK);
    no_output(app); /* Direction 5 is normalized to no direction. */
    assert(sv_app_physical(app, generation, (const unsigned char *)"8", 1) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    size_t include_count = sv_preference_runtime_include_count(runtime);
    assert(sv_preference_runtime_named(runtime, "parent.prf", &report) == SV_OK &&
           report.complete && report.files == 2);
    assert(sv_preference_runtime_include_count(runtime) == include_count + 1);
    included = sv_preference_runtime_include(runtime, include_count);
    assert(included && included->source_owner == SV_PREF_BUNDLED &&
           included->target_owner == SV_PREF_USER && included->source_line == 1 &&
           strstr(included->source_path, "/B/user/parent.prf") &&
           strstr(included->target_path, "/U/user/child.prf"));
    assert(sv_app_physical(app, generation, (const unsigned char *)"o", 1) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    assert(sv_preference_runtime_class(runtime, "Warrior", &report) == SV_OK &&
           report.complete && report.files == 1);
    assert(sv_app_physical(app, generation, (const unsigned char *)"k", 1) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    assert(sv_preference_runtime_class(runtime, "Absent", &report) == SV_OK &&
           !report.complete && report.files == 0 && report.warnings == 1);
    const SvPrefOrigin *origin = sv_preference_runtime_last_origin(runtime);
    assert(origin && origin->line == 0 && strstr(origin->path, "/user/Absent.prf"));
    assert(sv_preference_runtime_character(runtime, "Hero", "Human", "Maiar",
                                           "Warrior", "Wolf", &report) == SV_OK &&
           report.complete && report.files == 6);
    global_effects = 0;
    for (size_t i = 0; i < sv_preference_runtime_origin_count(runtime); ++i)
        if (strstr(sv_preference_runtime_origin(runtime, i)->path,
                   "/user/global.prf")) ++global_effects;
    assert(global_effects == 3); /* X, A and P, exactly one global load. */
    assert(sv_options_get(sv_preference_runtime_options(runtime),
                          "censor_swearing", &enabled) && !enabled);
    assert(sv_options_get(sv_preference_runtime_options(runtime),
                          "ring_bell", &enabled) && !enabled);
    assert(sv_app_macro_frame(app, generation, 1, 32) == SV_OK);
    const unsigned char queued_hero[] = {156, 'N'};
    expect_output(app, queued_hero, sizeof(queued_hero));
    assert(sv_app_physical(app, generation, (const unsigned char *)"z", 1) == SV_OK);
    const unsigned char form_action[] = {156,'f',156,'o',156,'r',156,'m'};
    expect_output(app, form_action, sizeof(form_action));
    assert(sv_preference_runtime_character(runtime, "Hero", "Human", "Maiar",
                                           "Warrior", "Player", &report) == SV_OK &&
           report.complete && report.files == 4);
    assert(sv_app_macro_frame(app, generation, 2, 32) == SV_OK);
    expect_output(app, queued_hero, sizeof(queued_hero));
    assert(sv_app_physical(app, generation, (const unsigned char *)"z", 1) == SV_OK);
    const unsigned char character_action[] = {156,'c',156,'h',156,'a',156,'r',156,'a',156,'c',156,'t',156,'e',156,'r'};
    expect_output(app, character_action, sizeof(character_action));
    assert(sv_preference_runtime_character(runtime, "Hero", "Human", "Maiar",
                                           "Warrior", "Wolf", &report) == SV_OK &&
           report.complete && report.files == 5);
    assert(sv_app_macro_frame(app, generation, 3, 32) == SV_OK);
    expect_output(app, queued_hero, sizeof(queued_hero));
    assert(sv_app_physical(app, generation, (const unsigned char *)"z", 1) == SV_OK);
    expect_output(app, form_action, sizeof(form_action));
    size_t global_after_reload = 0;
    for (size_t i = 0; i < sv_preference_runtime_origin_count(runtime); ++i)
        if (strstr(sv_preference_runtime_origin(runtime, i)->path,
                   "/user/global.prf")) ++global_after_reload;
    assert(global_after_reload == 3);
    assert(sv_preference_runtime_named(runtime, "include-only.prf", &report) == SV_OK);
    origin = sv_preference_runtime_last_origin(runtime);
    assert(origin && origin->owner == SV_PREF_USER && origin->line == 1 &&
           strstr(origin->path, "/user/include-only.prf"));
    SvNativeMacroLoader loader = {0};
    SvNativeInput native_input;
    sv_native_input_begin(&native_input, app);
    SDL_Event event = {.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F7; event.key.mod = SDL_KMOD_CTRL;
    event.common.timestamp = native_input.since_ns - 1;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) &&
           !loader.active); /* A queued opener from before F5. */
    event.common.timestamp = native_input.since_ns;
    assert(sv_app_queue_macro_action(app, (const unsigned char *)"8", 1) == SV_OK);
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && loader.active);
    event = (SDL_Event){.type = SDL_EVENT_WINDOW_FOCUS_LOST};
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && loader.active);
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN}; event.key.key = SDLK_ESCAPE;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && !loader.active);
    assert(sv_app_macro_frame(app, generation, 2, 16) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    assert(sv_app_macro_frame(app, generation, 3, 16) == SV_OK);
    no_output(app);
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F8; event.key.mod = SDL_KMOD_CTRL;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && loader.class_load);
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT};
    event.text.text = "Warrior";
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN}; event.key.key = SDLK_RETURN;
    event.common.timestamp = native_input.since_ns - 1;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) &&
           !loader.job); /* A queued submit cannot start a load. */
    event.common.timestamp = native_input.since_ns;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && loader.job);
    for (int wait = 0; wait < 1000 && loader.active; ++wait) {
        sv_native_macro_loader_frame(&loader, runtime, app, &native_input);
        SDL_Delay(1);
    }
    assert(!loader.active);
    assert(sv_app_physical(app, generation, (const unsigned char *)"k", 1) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F7; event.key.mod = SDL_KMOD_CTRL;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && loader.active);
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "bad-action.prf";
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN}; event.key.key = SDLK_RETURN;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && loader.job);
    for (int wait = 0; wait < 1000 && loader.job; ++wait) {
        sv_native_macro_loader_frame(&loader, runtime, app, &native_input);
        SDL_Delay(1);
    }
    assert(loader.attempted);
    assert(strstr(loader.status, "Load incomplete"));
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && loader.attempted);
    event.key.key = SDLK_ESCAPE;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) && !loader.active);
    assert(sv_app_macro_frame(app, generation, 4, 16) == SV_OK);
    expect_output(app, walk, sizeof(walk));
    no_output(app);
    size_t before_cancel = sv_preference_runtime_origin_count(runtime);
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F7; event.key.mod = SDL_KMOD_CTRL;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "manual.prf";
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN}; event.key.key = SDLK_RETURN;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) &&
           loader.job);
    SDL_Delay(10); /* Completed worker, result still uncommitted until frame. */
    event.key.key = SDLK_ESCAPE;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) &&
           !loader.active);
    assert(sv_preference_runtime_origin_count(runtime) == before_cancel);
    size_t before_many = sv_preference_runtime_origin_count(runtime);
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F7; event.key.mod = SDL_KMOD_CTRL;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "many.prf";
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN}; event.key.key = SDLK_RETURN;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    for (int wait = 0; wait < 1000 && !sv_native_macro_loader_committing(&loader); ++wait) {
        sv_native_macro_loader_frame(&loader, runtime, app, &native_input);
        SDL_Delay(1);
    }
    assert(loader.active && sv_native_macro_loader_committing(&loader));
    assert(sv_preference_runtime_origin_count(runtime) == before_many + 16);
    assert(!sv_native_macro_loader_dispatch_input(&loader, app));
    const unsigned char during_replay_request[] = {184,0,0,0,9,'K','e','y','?',0};
    assert(sv_app_receive(app, generation, during_replay_request,
                          sizeof(during_replay_request)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1 && sv_app_view(app).request.pending);
    event = (SDL_Event){.type = SDL_EVENT_WINDOW_FOCUS_LOST};
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) &&
           loader.active);
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "Y";
    assert(!sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    assert(sv_native_input(&native_input, app, &event));
    assert(sv_native_macro_loader_dispatch_input(&loader, app));
    assert(sv_app_dispatch_input(app, 1).dispatched == 1);
    const unsigned char replay_request_reply[] = {184,0,0,0,9,'Y'};
    expect_output(app, replay_request_reply, sizeof(replay_request_reply));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_ESCAPE;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) &&
           loader.active); /* Commit finishes in later frames before close. */
    for (int wait = 0; wait < 1000 && loader.active; ++wait)
        sv_native_macro_loader_frame(&loader, runtime, app, &native_input);
    assert(!loader.active && sv_preference_runtime_origin_count(runtime) == before_many + 40);
    size_t before_teardown = sv_preference_runtime_origin_count(runtime);
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F7; event.key.mod = SDL_KMOD_CTRL;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "manual.prf";
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN}; event.key.key = SDLK_RETURN;
    assert(sv_native_macro_loader_event(&loader, runtime, app, &native_input, &event) &&
           loader.job);
    SDL_Delay(10); /* Worker done, but no main-thread commit before teardown. */
    assert(sv_app_close(app) == SV_OK);
    sv_native_macro_loader_frame(&loader, runtime, app, &native_input);
    assert(!loader.active && sv_preference_runtime_origin_count(runtime) == before_teardown);
    sv_preference_runtime_destroy(runtime);
    assert(sv_app_destroy(app) == SV_OK);
    app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    runtime = sv_preference_runtime_create(app, argv[1], argv[2]);
    assert(runtime && sv_preference_runtime_bootstrap(runtime, &report) == SV_OK);
    assert(sv_preference_runtime_global(runtime, &report) == SV_OK && report.files == 1);
    assert(sv_preference_runtime_character(runtime, "Hero", "Human", "Maiar",
                                           "Warrior", "Wolf", &report) == SV_OK &&
           report.complete && report.files == 5);
    assert(sv_app_macro_frame(app, sv_app_view(app).generation, 1, 32) == SV_OK);
    expect_output(app, queued_hero, sizeof(queued_hero));
    assert(sv_options_get(sv_preference_runtime_options(runtime),
                          "censor_swearing", &enabled) && !enabled);
    size_t global_after_character = 0;
    for (size_t i = 0; i < sv_preference_runtime_origin_count(runtime); ++i)
        if (strstr(sv_preference_runtime_origin(runtime, i)->path,
                   "/user/global.prf")) ++global_after_character;
    assert(global_after_character == 3);
    assert(sv_preference_runtime_character(runtime, "HeroOff", "Human", "Maiar",
                                           "Warrior", "Wolf", &report) == SV_OK);
    assert(sv_options_get(sv_preference_runtime_options(runtime),
                          "load_form_macros", &enabled) && !enabled);
    assert(sv_options_get(sv_preference_runtime_options(runtime),
                          "censor_swearing", &enabled) && enabled);
    assert(sv_app_physical(app, sv_app_view(app).generation,
                           (const unsigned char *)"z", 1) == SV_OK);
    const unsigned char class_action[] = {156,'c',156,'l',156,'a',156,'s',156,'s'};
    expect_output(app, class_action, sizeof(class_action));
    assert(sv_preference_runtime_named(runtime, "overflow.prf", &report) == SV_INVALID &&
           !report.complete);
    assert(sv_preference_runtime_named(runtime, "manual.prf", &report) == SV_OK &&
           report.complete); /* Failure is scoped to one ordinary load. */
    SvNativeMacroLoader next_loader = {0};
    SvNativeInput next_input;
    sv_native_input_begin(&next_input, app);
    uint64_t old_timestamp = next_input.since_ns;
    SDL_Delay(1);
    assert(sv_app_close(app) == SV_OK && sv_app_open(app, version) == SV_OK);
    sv_native_input_begin(&next_input, app);
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F7; event.key.mod = SDL_KMOD_CTRL;
    event.common.timestamp = old_timestamp;
    assert(sv_native_macro_loader_event(&next_loader, runtime, app, &next_input, &event) &&
           !next_loader.active);
    event.common.timestamp = next_input.since_ns;
    assert(sv_native_macro_loader_event(&next_loader, runtime, app, &next_input, &event) &&
           next_loader.active);
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "manual.prf";
    event.common.timestamp = next_input.since_ns;
    assert(sv_native_macro_loader_event(&next_loader, runtime, app, &next_input, &event));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN}; event.key.key = SDLK_RETURN;
    event.common.timestamp = old_timestamp;
    assert(sv_native_macro_loader_event(&next_loader, runtime, app, &next_input, &event) &&
           !next_loader.job);
    sv_native_macro_loader_reset(&next_loader);
    sv_preference_runtime_destroy(runtime);
    assert(sv_app_destroy(app) == SV_OK);
    free(macros);
    SDL_Quit();
    puts("PASS: PRF layers and production macro packet route");
}

#include "preferences-runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SvPreferenceRuntime {
    SvApp *app;
    SvPreferences preferences;
    SvOptions options;
    SvMacroSet *macros;
    SvPrefOrigin *origins;
    size_t origin_count, origin_capacity;
    SvPrefIncludeOrigin *includes;
    size_t include_count, include_capacity;
    bool effect_failed;
    bool bootstrapped, global_loaded, character_loaded;
};

static bool resolve_path(const SvPreferenceRuntime *runtime, SvPrefOwner owner,
                         const char *file, char path[4096])
{
    const char *root = owner == SV_PREF_USER ? runtime->preferences.user_root :
        runtime->preferences.library_root;
    return root && file && snprintf(path, 4096, "%s/user/%s", root, file) < 4096;
}

static void include_effect(void *context, SvPrefOwner source_owner, const char *source_file,
                           size_t source_line, SvPrefOwner target_owner,
                           const char *target_file)
{
    SvPreferenceRuntime *runtime = context;
    if (runtime->include_count == runtime->include_capacity) {
        size_t capacity = runtime->include_capacity ? runtime->include_capacity * 2 : 8;
        SvPrefIncludeOrigin *next = realloc(runtime->includes, capacity * sizeof(*next));
        if (!next) { runtime->effect_failed = true; return; }
        runtime->includes = next;
        runtime->include_capacity = capacity;
    }
    SvPrefIncludeOrigin *origin = &runtime->includes[runtime->include_count++];
    origin->source_owner = source_owner;
    origin->target_owner = target_owner;
    origin->source_line = source_line;
    if (!resolve_path(runtime, source_owner, source_file, origin->source_path) ||
        !resolve_path(runtime, target_owner, target_file, origin->target_path))
        runtime->effect_failed = true;
}

static void effect(void *context, SvPrefEffectKind kind, SvPrefOwner owner,
                   const char *file, size_t line, const unsigned char *bytes, size_t size)
{
    SvPreferenceRuntime *runtime = context;
    if (runtime->origin_count == runtime->origin_capacity) {
        size_t capacity = runtime->origin_capacity ? runtime->origin_capacity * 2 : 32;
        SvPrefOrigin *next = realloc(runtime->origins, capacity * sizeof(*next));
        if (!next) { runtime->effect_failed = true; return; }
        runtime->origins = next;
        runtime->origin_capacity = capacity;
    }
    SvPrefOrigin *origin = &runtime->origins[runtime->origin_count++];
    origin->kind = kind;
    origin->owner = owner;
    origin->line = line;
    if (!resolve_path(runtime, owner, file, origin->path)) {
        runtime->effect_failed = true;
        return;
    }
    if (kind == SV_PREF_WARNING)
        fprintf(stderr, "SV PRF %s:%zu: %.*s\n", origin->path, line, (int)size, bytes);
    else if (kind == SV_PREF_ACTION) {
        if (sv_app_queue_macro_action(runtime->app, bytes, size) != SV_OK)
            runtime->effect_failed = true;
    } else if (kind == SV_PREF_MESSAGE && sv_app_view(runtime->app).active &&
               sv_app_local_message(runtime->app, sv_app_view(runtime->app).generation,
                                    bytes, size) != SV_OK)
        runtime->effect_failed = true;
}

SvPreferenceRuntime *sv_preference_runtime_create(SvApp *app, const char *user_root,
                                                  const char *library_root)
{
    if (!app || !user_root || !library_root) return NULL;
    SvPreferenceRuntime *runtime = calloc(1, sizeof(*runtime));
    if (!runtime) return NULL;
    runtime->macros = calloc(1, sizeof(*runtime->macros));
    if (!runtime->macros) { free(runtime); return NULL; }
    runtime->app = app;
    runtime->preferences = (SvPreferences){.user_root = user_root,
        .library_root = library_root, .options = &runtime->options,
        .macros = runtime->macros, .sink = {runtime, effect, include_effect}};
    return runtime;
}

void sv_preference_runtime_destroy(SvPreferenceRuntime *runtime)
{
    if (runtime) {
        free(runtime->includes); free(runtime->origins); free(runtime->macros); free(runtime);
    }
}

static SvResult publish(SvPreferenceRuntime *runtime)
{
    if (runtime->effect_failed) return SV_INVALID;
    bool roguelike = false;
    (void)sv_options_get(&runtime->options, "rogue_like_commands", &roguelike);
    return sv_app_install_input_profile(runtime->app, runtime->macros, roguelike,
        runtime->preferences.keymap_present,
        runtime->preferences.keymap_command, runtime->preferences.keymap_direction);
}

SvResult sv_preference_runtime_bootstrap(SvPreferenceRuntime *runtime,
                                        SvPrefReport *report)
{
    if (!runtime || !report) return SV_INVALID;
    if (runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    *report = sv_preferences_bootstrap(&runtime->preferences);
    SvResult result = publish(runtime);
    if (result == SV_OK) runtime->bootstrapped = true;
    return result;
}

SvResult sv_preference_runtime_global(SvPreferenceRuntime *runtime,
                                     SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    if (runtime->global_loaded || runtime->character_loaded) return SV_INVALID;
    *report = sv_preferences_load_named(&runtime->preferences, "global.prf", false);
    SvResult result = publish(runtime);
    if (result == SV_OK) runtime->global_loaded = true;
    return result;
}

SvResult sv_preference_runtime_named(SvPreferenceRuntime *runtime, const char *name,
                                    SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    *report = sv_preferences_load_named(&runtime->preferences, name, true);
    return publish(runtime);
}

SvResult sv_preference_runtime_class(SvPreferenceRuntime *runtime, const char *class_name,
                                    SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    *report = sv_preferences_load_class(&runtime->preferences, class_name);
    return publish(runtime);
}

SvResult sv_preference_runtime_character(SvPreferenceRuntime *runtime, const char *character,
    const char *race, const char *trait, const char *class_name, const char *form,
    SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    if (runtime->global_loaded || runtime->character_loaded) return SV_INVALID;
    *report = sv_preferences_character(&runtime->preferences, character, race, trait,
                                       class_name, form);
    SvResult result = publish(runtime);
    if (result == SV_OK) runtime->character_loaded = true;
    return result;
}

const SvPrefOrigin *sv_preference_runtime_last_origin(const SvPreferenceRuntime *runtime)
{
    return runtime && runtime->origin_count ?
        &runtime->origins[runtime->origin_count - 1] : NULL;
}
size_t sv_preference_runtime_origin_count(const SvPreferenceRuntime *runtime)
{
    return runtime ? runtime->origin_count : 0;
}
const SvPrefOrigin *sv_preference_runtime_origin(const SvPreferenceRuntime *runtime,
                                                size_t index)
{
    return runtime && index < runtime->origin_count ? &runtime->origins[index] : NULL;
}
size_t sv_preference_runtime_include_count(const SvPreferenceRuntime *runtime)
{
    return runtime ? runtime->include_count : 0;
}
const SvPrefIncludeOrigin *sv_preference_runtime_include(const SvPreferenceRuntime *runtime,
                                                         size_t index)
{
    return runtime && index < runtime->include_count ? &runtime->includes[index] : NULL;
}
const SvOptions *sv_preference_runtime_options(const SvPreferenceRuntime *runtime)
{
    return runtime ? &runtime->options : NULL;
}

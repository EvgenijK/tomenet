#include "preferences-runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#define SV_JOB_INCLUDE ((SvPrefEffectKind)100)

typedef struct SvGlobalOption {
    struct SvGlobalOption *next;
    char record[512];
} SvGlobalOption;

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
    SvMacroSet *global_macros;
    SvOptions global_options;
    bool global_keymap_present[128];
    unsigned char global_keymap_command[128], global_keymap_direction[128];
    unsigned char global_action[SV_MACRO_ACTION];
    size_t global_action_size;
    SvGlobalOption *global_options_first, *global_options_last;
    bool recording_global;
};

typedef struct SvPrefLoadEffect {
    struct SvPrefLoadEffect *next;
    SvPrefEffectKind kind;
    SvPrefOwner owner, target_owner;
    size_t line, size;
    char file[512];
    unsigned char bytes[];
} SvPrefLoadEffect;

struct SvPrefLoadJob {
    SDL_AtomicInt refs, done, cancelled;
    SDL_Thread *thread;
    uint64_t generation;
    SvPreferences preferences;
    SvOptions options;
    SvMacroSet *macros;
    char *user_root, *library_root, *name;
    bool class_load, failed;
    SvPrefReport report;
    SvPrefLoadEffect *first, *last;
    SvPrefLoadEffect *next_effect;
    size_t effect_count;
    bool committed;
};

static char *copy_string(const char *value)
{
    size_t size = strlen(value) + 1;
    char *copy = malloc(size);
    if (copy) memcpy(copy, value, size);
    return copy;
}

static void release_job(SvPrefLoadJob *job)
{
    if (SDL_AddAtomicInt(&job->refs, -1) != 1) return;
    SvPrefLoadEffect *item = job->first;
    while (item) { SvPrefLoadEffect *next = item->next; free(item); item = next; }
    free(job->name); free(job->user_root); free(job->library_root);
    free(job->macros); free(job);
}

static void job_record(SvPrefLoadJob *job, SvPrefEffectKind kind, SvPrefOwner owner,
                       const char *file, size_t line, SvPrefOwner target_owner,
                       const unsigned char *bytes, size_t size)
{
    if (job->failed || SDL_GetAtomicInt(&job->cancelled)) return;
    if (++job->effect_count > 8192 || !file || strlen(file) >= 512 || size > 4096) {
        job->failed = true; return;
    }
    SvPrefLoadEffect *item = calloc(1, sizeof(*item) + size + 1);
    if (!item) { job->failed = true; return; }
    item->kind = kind; item->owner = owner; item->target_owner = target_owner;
    item->line = line; item->size = size;
    strcpy(item->file, file);
    if (size) memcpy(item->bytes, bytes, size);
    if (job->last) job->last->next = item;
    else job->first = item;
    job->last = item;
}

static void job_effect(void *context, SvPrefEffectKind kind, SvPrefOwner owner,
                       const char *file, size_t line, const unsigned char *bytes, size_t size)
{
    job_record(context, kind, owner, file, line, owner, bytes, size);
}

static void job_include(void *context, SvPrefOwner source_owner, const char *source_file,
                        size_t source_line, SvPrefOwner target_owner, const char *target_file)
{
    job_record(context, SV_JOB_INCLUDE, source_owner, source_file, source_line,
               target_owner, (const unsigned char *)target_file, strlen(target_file) + 1);
}

static int load_worker(void *context)
{
    SvPrefLoadJob *job = context;
    job->report = job->class_load ?
        sv_preferences_load_class(&job->preferences, job->name) :
        sv_preferences_load_named(&job->preferences, job->name, true);
    SDL_SetAtomicInt(&job->done, 1);
    release_job(job);
    return 0;
}

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
    if (kind == SV_PREF_OPTION && runtime->recording_global && size < 512) {
        SvGlobalOption *option = calloc(1, sizeof(*option));
        if (!option) { runtime->effect_failed = true; return; }
        memcpy(option->record, bytes, size);
        if (runtime->global_options_last) runtime->global_options_last->next = option;
        else runtime->global_options_first = option;
        runtime->global_options_last = option;
    }
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
        SvGlobalOption *option = runtime->global_options_first;
        while (option) { SvGlobalOption *next = option->next; free(option); option = next; }
        free(runtime->global_macros); free(runtime->includes); free(runtime->origins);
        free(runtime->macros); free(runtime);
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

static SvResult publish_report(SvPreferenceRuntime *runtime, SvPrefReport *report)
{
    SvResult result = publish(runtime);
    if (result != SV_OK) report->complete = false;
    return result;
}

static void begin_effects(SvPreferenceRuntime *runtime)
{
    runtime->effect_failed = false;
}

static SvResult save_global(SvPreferenceRuntime *runtime, const SvOptions *before_character)
{
    if (!runtime->global_macros) runtime->global_macros = malloc(sizeof(*runtime->global_macros));
    if (!runtime->global_macros) return SV_NO_MEMORY;
    *runtime->global_macros = *runtime->macros;
    runtime->global_options = *before_character;
    for (SvGlobalOption *option = runtime->global_options_first; option;
         option = option->next)
        if (!sv_options_apply_pref(&runtime->global_options, option->record[0],
                                   option->record + 2)) return SV_INVALID;
    memcpy(runtime->global_keymap_present, runtime->preferences.keymap_present,
           sizeof(runtime->global_keymap_present));
    memcpy(runtime->global_keymap_command, runtime->preferences.keymap_command,
           sizeof(runtime->global_keymap_command));
    memcpy(runtime->global_keymap_direction, runtime->preferences.keymap_direction,
           sizeof(runtime->global_keymap_direction));
    memcpy(runtime->global_action, runtime->preferences.pending_action,
           runtime->preferences.pending_action_size);
    runtime->global_action_size = runtime->preferences.pending_action_size;
    return SV_OK;
}

static void restore_global(SvPreferenceRuntime *runtime)
{
    *runtime->macros = *runtime->global_macros;
    runtime->options = runtime->global_options;
    memcpy(runtime->preferences.keymap_present, runtime->global_keymap_present,
           sizeof(runtime->global_keymap_present));
    memcpy(runtime->preferences.keymap_command, runtime->global_keymap_command,
           sizeof(runtime->global_keymap_command));
    memcpy(runtime->preferences.keymap_direction, runtime->global_keymap_direction,
           sizeof(runtime->global_keymap_direction));
    memcpy(runtime->preferences.pending_action, runtime->global_action,
           runtime->global_action_size);
    runtime->preferences.pending_action_size = runtime->global_action_size;
}

SvResult sv_preference_runtime_bootstrap(SvPreferenceRuntime *runtime,
                                        SvPrefReport *report)
{
    if (!runtime || !report) return SV_INVALID;
    if (runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    begin_effects(runtime);
    *report = sv_preferences_bootstrap(&runtime->preferences);
    SvResult result = publish_report(runtime, report);
    if (result == SV_OK) runtime->bootstrapped = true;
    return result;
}

SvResult sv_preference_runtime_global(SvPreferenceRuntime *runtime,
                                     SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    if (runtime->global_loaded || runtime->character_loaded) return SV_INVALID;
    begin_effects(runtime);
    SvOptions before_global = runtime->options;
    runtime->recording_global = true;
    *report = sv_preferences_load_named(&runtime->preferences, "global.prf", false);
    runtime->recording_global = false;
    SvResult result = publish_report(runtime, report);
    if (result == SV_OK) {
        result = save_global(runtime, &before_global);
        if (result == SV_OK) runtime->global_loaded = true;
    }
    return result;
}

SvResult sv_preference_runtime_named(SvPreferenceRuntime *runtime, const char *name,
                                    SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    begin_effects(runtime);
    *report = sv_preferences_load_named(&runtime->preferences, name, true);
    return publish_report(runtime, report);
}

SvResult sv_preference_runtime_class(SvPreferenceRuntime *runtime, const char *class_name,
                                    SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    begin_effects(runtime);
    *report = sv_preferences_load_class(&runtime->preferences, class_name);
    return publish_report(runtime, report);
}

SvResult sv_preference_runtime_character(SvPreferenceRuntime *runtime, const char *character,
    const char *race, const char *trait, const char *class_name, const char *form,
    SvPrefReport *report)
{
    if (!runtime || !report || !runtime->bootstrapped ||
        !sv_options_safe_name(character)) return SV_INVALID;
    if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
    begin_effects(runtime);
    SvOptions before_character = runtime->global_loaded ? runtime->global_options :
                                 runtime->options;
    if (runtime->character_loaded) restore_global(runtime);
    if (!sv_options_load_character(&runtime->options, runtime->preferences.user_root,
                                   character)) return SV_INVALID;
    if (!runtime->global_loaded) {
        runtime->recording_global = true;
        SvPrefReport global = sv_preferences_load_named(&runtime->preferences,
                                                        "global.prf", false);
        runtime->recording_global = false;
        SvResult saved = save_global(runtime, &before_character);
        if (saved != SV_OK) return saved;
        runtime->global_loaded = true;
        *report = global;
    } else {
        *report = (SvPrefReport){.complete = true};
        for (SvGlobalOption *option = runtime->global_options_first; option;
             option = option->next)
            if (!sv_options_apply_pref(&runtime->options, option->record[0],
                                       option->record + 2)) return SV_INVALID;
    }
    SvPrefReport character_report = sv_preferences_character_layers(&runtime->preferences,
        character, race, trait, class_name, form, false);
    report->files += character_report.files;
    report->records += character_report.records;
    report->warnings += character_report.warnings;
    report->complete &= character_report.complete;
    SvResult result = publish_report(runtime, report);
    if (result == SV_OK) runtime->character_loaded = true;
    return result;
}

SvPrefLoadJob *sv_preference_runtime_start_load(SvPreferenceRuntime *runtime,
                                                const char *name, bool class_load)
{
    if (!runtime || !runtime->bootstrapped || !name || !sv_app_macro_idle(runtime->app))
        return NULL;
    SvPrefLoadJob *job = calloc(1, sizeof(*job));
    if (!job) return NULL;
    job->macros = malloc(sizeof(*job->macros));
    job->user_root = copy_string(runtime->preferences.user_root);
    job->library_root = copy_string(runtime->preferences.library_root);
    job->name = copy_string(name);
    if (!job->macros || !job->user_root || !job->library_root || !job->name) {
        SDL_SetAtomicInt(&job->refs, 1); release_job(job); return NULL;
    }
    *job->macros = *runtime->macros;
    job->options = runtime->options;
    job->preferences = runtime->preferences;
    job->preferences.user_root = job->user_root;
    job->preferences.library_root = job->library_root;
    job->preferences.macros = job->macros;
    job->preferences.options = &job->options;
    job->preferences.sink = (SvPreferenceSink){job, job_effect, job_include};
    job->generation = sv_app_view(runtime->app).generation;
    job->class_load = class_load;
    SDL_SetAtomicInt(&job->refs, 2);
    job->thread = SDL_CreateThread(load_worker, "sv-prf", job);
    if (!job->thread) { release_job(job); release_job(job); return NULL; }
    return job;
}

SvResult sv_preference_runtime_finish_load(SvPreferenceRuntime *runtime, SvPrefLoadJob *job,
                                            SvPrefReport *report)
{
    if (!runtime || !job || !report) return SV_INVALID;
    if (!SDL_GetAtomicInt(&job->done)) return SV_WAITING;
    if (SDL_GetAtomicInt(&job->cancelled) || !sv_app_view(runtime->app).active ||
        job->generation != sv_app_view(runtime->app).generation) return SV_STALE;
    *report = job->report;
    if (job->failed) return SV_INPUT_OVERFLOW;
    if (!job->committed) {
        if (!sv_app_macro_idle(runtime->app)) return SV_BUSY;
        bool roguelike = false;
        (void)sv_options_get(&job->options, "rogue_like_commands", &roguelike);
        SvResult installed = sv_app_install_input_profile(runtime->app, job->macros,
            roguelike, job->preferences.keymap_present,
            job->preferences.keymap_command, job->preferences.keymap_direction);
        if (installed != SV_OK) return installed;
        *runtime->macros = *job->macros;
        runtime->options = job->options;
        memcpy(runtime->preferences.keymap_present, job->preferences.keymap_present,
               sizeof(runtime->preferences.keymap_present));
        memcpy(runtime->preferences.keymap_command, job->preferences.keymap_command,
               sizeof(runtime->preferences.keymap_command));
        memcpy(runtime->preferences.keymap_direction, job->preferences.keymap_direction,
               sizeof(runtime->preferences.keymap_direction));
        memcpy(runtime->preferences.pending_action, job->preferences.pending_action,
               job->preferences.pending_action_size);
        runtime->preferences.pending_action_size = job->preferences.pending_action_size;
        begin_effects(runtime);
        job->next_effect = job->first;
        job->committed = true;
    }
    /* Replaying origins and permitted effects is main-thread work. Yield after
     * sixteen records, keeping parser order and network/UI service per frame. */
    for (unsigned count = 0; count < 16 && job->next_effect; ++count) {
        SvPrefLoadEffect *item = job->next_effect;
        job->next_effect = item->next;
        if (item->kind == SV_JOB_INCLUDE)
            include_effect(runtime, item->owner, item->file, item->line,
                           item->target_owner, (const char *)item->bytes);
        else effect(runtime, item->kind, item->owner, item->file, item->line,
                    item->bytes, item->size);
    }
    if (job->next_effect) return SV_WAITING;
    if (runtime->effect_failed) {
        report->complete = false;
        runtime->effect_failed = false;
        return SV_INVALID; /* Baseline PRF effects already applied stay visible. */
    }
    return SV_OK;
}

void sv_preference_runtime_cancel_load(SvPrefLoadJob *job)
{
    if (!job) return;
    SDL_SetAtomicInt(&job->cancelled, 1);
    if (SDL_GetAtomicInt(&job->done)) SDL_WaitThread(job->thread, NULL);
    else SDL_DetachThread(job->thread);
    release_job(job);
}

bool sv_preference_runtime_load_committed(const SvPrefLoadJob *job)
{
    return job && job->committed;
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

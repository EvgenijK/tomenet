#ifndef SV_PREFERENCES_RUNTIME_H
#define SV_PREFERENCES_RUNTIME_H
#include "app.h"
#include "preferences.h"

typedef struct SvPreferenceRuntime SvPreferenceRuntime;
typedef struct SvPrefLoadJob SvPrefLoadJob;
typedef struct {
    SvPrefEffectKind kind;
    SvPrefOwner owner;
    char path[4096];
    size_t line;
} SvPrefOrigin;
typedef struct {
    SvPrefOwner source_owner, target_owner;
    char source_path[4096], target_path[4096];
    size_t source_line;
} SvPrefIncludeOrigin;

SvPreferenceRuntime *sv_preference_runtime_create(SvApp *app, const char *user_root,
                                                  const char *library_root);
void sv_preference_runtime_destroy(SvPreferenceRuntime *runtime);
SvResult sv_preference_runtime_bootstrap(SvPreferenceRuntime *runtime,
                                        SvPrefReport *report);
/* The synthetic shell calls this before a character is known; a later character
 * load keeps global effects single-shot and restores character OPT precedence. */
SvResult sv_preference_runtime_global(SvPreferenceRuntime *runtime,
                                     SvPrefReport *report);
SvResult sv_preference_runtime_named(SvPreferenceRuntime *runtime, const char *name,
                                    SvPrefReport *report);
SvResult sv_preference_runtime_class(SvPreferenceRuntime *runtime, const char *class_name,
                                    SvPrefReport *report);
SvResult sv_preference_runtime_character(SvPreferenceRuntime *runtime, const char *character,
    const char *race, const char *trait, const char *class_name, const char *form,
    SvPrefReport *report);
const SvPrefOrigin *sv_preference_runtime_last_origin(const SvPreferenceRuntime *runtime);
size_t sv_preference_runtime_origin_count(const SvPreferenceRuntime *runtime);
const SvPrefOrigin *sv_preference_runtime_origin(const SvPreferenceRuntime *runtime,
                                                size_t index);
size_t sv_preference_runtime_include_count(const SvPreferenceRuntime *runtime);
const SvPrefIncludeOrigin *sv_preference_runtime_include(const SvPreferenceRuntime *runtime,
                                                         size_t index);
const SvOptions *sv_preference_runtime_options(const SvPreferenceRuntime *runtime);
/* Native loads parse against an isolated snapshot. Poll and commit on the owner thread. */
SvPrefLoadJob *sv_preference_runtime_start_load(SvPreferenceRuntime *runtime,
                                                const char *name, bool class_load);
SvResult sv_preference_runtime_finish_load(SvPreferenceRuntime *runtime, SvPrefLoadJob *job,
                                            SvPrefReport *report);
void sv_preference_runtime_cancel_load(SvPrefLoadJob *job);
bool sv_preference_runtime_load_committed(const SvPrefLoadJob *job);
#endif

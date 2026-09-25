#ifndef SV_SETTINGS_H
#define SV_SETTINGS_H
#include "profile.h"
#include "options.h"
#include <stddef.h>

typedef enum {
    SV_SETTINGS_OK, SV_SETTINGS_CONFLICT, SV_SETTINGS_IO_ERROR,
    SV_SETTINGS_INVALID
} SvSettingsResult;
typedef enum {
    SV_OPTIONS_CHARACTER, SV_OPTIONS_GLOBAL, SV_OPTIONS_CLASS, SV_OPTIONS_NAMED
} SvOptionsScope;

typedef struct {
    char path[4096];
    char *opened;
    size_t opened_size;
} SvSettingsFile;

typedef struct {
    SvProfile opening_profile, profile;
    SvOptions opening_options, options;
    SvProfile saved_profile;
    SvOptions saved_options;
    SvSettingsFile cfg, opt;
    bool cfg_conflicts[18], opt_conflicts[188];
    bool options_selected;
    bool options_snapshot;
    char root[4096];
} SvSettings;

/* Snapshot current values and the concrete owned CFG. The form owns the
 * returned state and should be disposed without saving on normal exit. */
bool sv_settings_begin(SvSettings *settings, const char *root,
                       const SvProfile *profile, const SvOptions *options);
void sv_settings_end(SvSettings *settings);
bool sv_settings_edit_cfg(SvSettings *settings, const char *key, const char *value);
bool sv_settings_edit_option(SvSettings *settings, const char *name, bool value);
bool sv_settings_dirty(const SvSettings *settings);
void sv_settings_cancel(SvSettings *settings);

/* Explicit destination selection never causes automatic class loading.
 * Named destinations are relative to S and must be single safe filenames. */
bool sv_settings_options_target(SvSettings *settings, SvOptionsScope scope,
                                const char *name);
SvSettingsResult sv_settings_save_cfg(SvSettings *settings);
SvSettingsResult sv_settings_save_options(SvSettings *settings);
#endif

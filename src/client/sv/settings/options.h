#ifndef SV_SETTINGS_OPTIONS_H
#define SV_SETTINGS_OPTIONS_H
#include <stdbool.h>
#include <stddef.h>
#define SV_SETTINGS_OPTION_COUNT 199
#define SV_SETTINGS_OPTION_RECORD_CAPACITY 32768
/* Values retain baseline option_info slot identities, including inactive holes.
 * Retained records are source data for future explicit Save, never executable. */
typedef struct {
    unsigned char values[SV_SETTINGS_OPTION_COUNT];
    unsigned warnings, aliases, files;
    char records[SV_SETTINGS_OPTION_RECORD_CAPACITY];
    size_t record_bytes;
} SvSettingsOptions;
typedef enum { SV_OPTION_BUNDLED, SV_OPTION_PROFILE, SV_OPTION_SHARED } SvOptionOwner;
void sv_settings_options_defaults(SvSettingsOptions *options);
bool sv_settings_option_set(SvSettingsOptions *options, const char *name, bool value);
int sv_settings_option_slot(const char *name);
/* Exact baseline aliases, including inversion/one-to-many/discard. */
bool sv_settings_option_apply(SvSettingsOptions *, const char *name, bool value);
/* Assignment slots in baseline order, including unchanged values and aliases. */
bool sv_settings_option_apply_record(SvSettingsOptions *, const char *name,
    bool value, int slots[2], size_t *count);
/* B is game library root; U is SDL3 user root. All automatic OPT reads use
 * U/sv. B/user is shipped bootstrap; shared user PRFs use U/user, then B/user.
 * Missing optional files succeed; malformed/unreadable files return false and
 * preserve usable values. No loader writes any file. */
bool sv_settings_options_load(SvSettingsOptions *options, const char *user_root,
                              const char *library_root);
bool sv_settings_options_character(SvSettingsOptions *out,
    const SvSettingsOptions *base, const char *user_root, const char *library_root,
    const char *character);
/* Explicit resource PRF hook for its later owner, at the baseline font/graphics
 * load point before global-sv.opt. Compatibility wrapper; use SvPreferences
 * for retained macro/mapping/effect state. */
bool sv_settings_options_file(SvSettingsOptions *options, const char *user_root,
    const char *library_root, SvOptionOwner owner, const char *name);
#endif

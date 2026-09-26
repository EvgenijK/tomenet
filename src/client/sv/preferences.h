#ifndef SV_PREFERENCES_H
#define SV_PREFERENCES_H
#include "input/macros.h"
#include "options.h"

typedef enum { SV_PREF_USER, SV_PREF_BUNDLED } SvPrefOwner;
typedef enum { SV_PREF_MESSAGE, SV_PREF_ACTION, SV_PREF_WARNING,
               SV_PREF_OPTION, SV_PREF_KEYMAP, SV_PREF_MACRO,
               SV_PREF_MACRO_ACTION } SvPrefEffectKind;
typedef struct {
    void *context;
    void (*effect)(void *context, SvPrefEffectKind kind, SvPrefOwner owner,
                   const char *file, size_t line, const unsigned char *bytes, size_t size);
    void (*include)(void *context, SvPrefOwner source_owner, const char *source_file,
                    size_t source_line, SvPrefOwner target_owner, const char *target_file);
} SvPreferenceSink;
typedef struct {
    const char *user_root, *library_root;
    SvOptions *options;
    SvMacroSet *macros;
    SvPreferenceSink sink;
    bool body_macros;
    unsigned char keymap_command[128], keymap_direction[128];
    bool keymap_present[128];
    unsigned char pending_action[SV_MACRO_ACTION];
    size_t pending_action_size;
} SvPreferences;
typedef struct { size_t files, records, warnings; bool complete; } SvPrefReport;

/* Ordinary load executes permitted effects. Missing automatic layers are quiet;
 * a named/manual load reports them. Both paths are read only. */
SvPrefReport sv_preferences_load_named(SvPreferences *preferences, const char *name,
                                       bool manual);
SvPrefReport sv_preferences_bootstrap(SvPreferences *preferences);
SvPrefReport sv_preferences_character(SvPreferences *preferences, const char *character,
                                      const char *race, const char *trait,
                                      const char *class_name, const char *form);
SvPrefReport sv_preferences_character_layers(SvPreferences *preferences,
    const char *character, const char *race, const char *trait,
    const char *class_name, const char *form, bool load_options);
SvPrefReport sv_preferences_load_class(SvPreferences *preferences, const char *class_name);
size_t sv_preferences_decode(const char *text, unsigned char *out, size_t capacity);
#endif

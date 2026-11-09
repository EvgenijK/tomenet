#ifndef SV_SETTINGS_PREFERENCES_H
#define SV_SETTINGS_PREFERENCES_H
#include "settings/options.h"
#include "files/files.h"
#include <stdint.h>

#define SV_PREFERENCE_MACRO_MAXLEN 1024
#define SV_PREFERENCE_INCLUDE_DEPTH 16
/* Bounded retained production data, including pending effects. */
#define SV_PREFERENCE_BYTE_LIMIT (64u * 1024u * 1024u)
typedef struct SvPreferences SvPreferences;
typedef enum {
    SV_PREF_CLEAN, SV_PREF_PARTIAL, SV_PREF_UNAVAILABLE, SV_PREF_FATAL
} SvPreferenceStatus;
typedef enum { SV_PREF_OPTIONAL, SV_PREF_EXPLICIT, SV_PREF_INCLUDE } SvPreferencePurpose;
typedef enum {
    SV_PREF_DIAG_MISSING, SV_PREF_DIAG_IO, SV_PREF_DIAG_INVALID,
    SV_PREF_DIAG_CYCLE, SV_PREF_DIAG_LIMIT, SV_PREF_DIAG_MAPPING,
    SV_PREF_DIAG_ALIAS
} SvPreferenceDiagnosticKind;
typedef struct {
    SvPreferenceStatus status;
    unsigned files, diagnostics;
} SvPreferenceResult;
typedef struct {
    SvPreferenceDiagnosticKind kind;
    SvFilePath source;
    unsigned line;
    const char *message;
} SvPreferenceDiagnostic;
typedef struct {
    /* This SV-local interpreter supports SDL3 graphics records independently
     * of the later renderer. Callers may declare a build without that support. */
    bool graphics_build, sdl3_build, graphics_enabled, custom_mapping_always;
    bool macro_only, bodymonster, in_game;
    int character_offset, subtileset;
    const unsigned char *exclusive_trigger;
    size_t exclusive_trigger_size;
    uint64_t generation;
    /* Explicit server-specific selection; NULL disables the arcade tail-load.
     * The explicit source itself has no tail; its includes use normal loading.
     * No server mode is guessed. */
    const char *arcade_file;
} SvPreferenceContext;
typedef struct {
    SvOptionOwner role;
    SvFilePath source;
    unsigned line;
    const char *text;
} SvPreferenceRecord;
typedef struct {
    char family;
    unsigned index;
    int subtileset;
    unsigned char attribute;
    unsigned character;
    long fields[4]; /* I geometry or m RGBA; S retains baseline field semantics. */
    SvFilePath source;
    unsigned line;
} SvPreferenceMapping;
/* Ordered semantic keymap inputs, retained independently of pending effects.
 * INITIAL captures the starting layout; OPTION never implies keymap_init.
 * S preserves the baseline first-field compatibility interpretation. */
typedef enum {
    SV_PREF_KEYMAP_INITIAL, SV_PREF_KEYMAP_OPTION, SV_PREF_KEYMAP_S
} SvPreferenceKeymapKind;
typedef struct {
    SvPreferenceKeymapKind kind;
    bool rogue;
    unsigned char key, command, direction;
} SvPreferenceKeymapRecord;
typedef enum { SV_PREF_MACRO_NORMAL, SV_PREF_MACRO_HYBRID, SV_PREF_MACRO_COMMAND } SvPreferenceMacroKind;
typedef struct {
    SvPreferenceMacroKind kind;
    const unsigned char *trigger, *action;
    size_t trigger_size, action_size;
    SvFilePath source;
    unsigned line;
} SvPreferenceMacro;
typedef enum { SV_PREF_MESSAGE, SV_PREF_ACTION, SV_PREF_OPTION } SvPreferenceEffectKind;
typedef struct {
    SvPreferenceEffectKind kind;
    const unsigned char *bytes;
    size_t size;
    bool chat, in_game;
    int option_slot;
    bool option_value;
    /* ACTION requires parse_macro=true, sleep semaphore=false,
     * macro_missing_item=0 and abort_prompt=false before runtime consumption. */
    uint64_t generation;
    SvFilePath source;
    unsigned line;
} SvPreferenceEffect;

SvPreferenceContext sv_preferences_default_context(void);
SvPreferences *sv_preferences_create(const char *user_root, const char *library_root,
                                     const SvSettingsOptions *initial);
/* Copies values/A/macros/mappings/records; pending effects/queue and diagnostics
 * are cleared when copy_effects=false (fresh character attempt). */
SvPreferences *sv_preferences_clone(const SvPreferences *, bool copy_effects);
void sv_preferences_destroy(SvPreferences *);
const SvSettingsOptions *sv_preferences_options(const SvPreferences *);
SvSettingsOptions *sv_preferences_options_mutable(SvPreferences *);
SvPreferenceResult sv_preferences_load(SvPreferences *, SvOptionOwner role,
    const char *name, SvPreferencePurpose, const SvPreferenceContext *);
/* Compiled defaults -> shipped bootstrap -> own global -> system prefs ->
 * resource hook -> own system. Character overlay is deliberately separate. */
typedef SvPreferenceResult (*SvPreferenceResourceHook)(SvPreferences *,
    const SvPreferenceContext *, void *);
SvPreferenceResult sv_preferences_bootstrap(SvPreferences *,
    const SvPreferenceContext *, SvPreferenceResourceHook, void *);
SvPreferences *sv_preferences_character(const SvPreferences *base,
    const char *character, const SvPreferenceContext *, SvPreferenceResult *);
size_t sv_preferences_record_count(const SvPreferences *);
const SvPreferenceRecord *sv_preferences_record(const SvPreferences *, size_t);
size_t sv_preferences_mapping_count(const SvPreferences *);
const SvPreferenceMapping *sv_preferences_mapping(const SvPreferences *, size_t);
const SvPreferenceKeymapRecord *sv_preferences_keymap_record(const SvPreferences *, size_t);
/* Baseline c_subtileset is glyph-indexed and shared by mapping families. */
int sv_preferences_glyph_subtileset(const SvPreferences *, unsigned glyph);
size_t sv_preferences_macro_count(const SvPreferences *);
const SvPreferenceMacro *sv_preferences_macro(const SvPreferences *, size_t);
size_t sv_preferences_effect_count(const SvPreferences *);
const SvPreferenceEffect *sv_preferences_effect(const SvPreferences *, size_t);
const unsigned char *sv_preferences_queue(const SvPreferences *, size_t *size);
size_t sv_preferences_diagnostic_count(const SvPreferences *);
const SvPreferenceDiagnostic *sv_preferences_diagnostic(const SvPreferences *, size_t);
/* Lifecycle owner calls this after delivery or when abandoning a generation.
 * Inspection never consumes or regenerates effects. */
void sv_preferences_clear_effects(SvPreferences *);
#endif

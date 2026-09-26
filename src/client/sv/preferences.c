#include "preferences.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SV_PREF_DEPTH 16
typedef struct {
    SvPreferences *preferences;
    SvPrefReport report;
    char names[SV_PREF_DEPTH][512];
    unsigned depth;
    bool bootstrap;
    unsigned char action[SV_MACRO_ACTION];
    size_t action_size;
} Loader;

static bool safe_name(const char *name)
{
    if (!name || !*name || strlen(name) > 500 || !strcmp(name, ".") ||
        !strcmp(name, "..")) return false;
    for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
        if (*p < 32 || *p == 127 || *p == '/' || *p == '\\' || *p == ':') return false;
    return true;
}

size_t sv_preferences_decode(const char *text, unsigned char *out, size_t capacity)
{
    if (!text || !out) return 0;
    size_t size = 0;
    for (const unsigned char *p = (const unsigned char *)text; *p;) {
        unsigned char byte = *p++;
        if (byte == '\\') {
            byte = *p++;
            if (!byte) return 0;
            switch (byte) {
            case 'w': byte = 96; break;
            case 'W': byte = 30; break;
            case 's': byte = ' '; break;
            case 'e': byte = 27; break;
            case 'b': byte = 8; break;
            case 'n': byte = 10; break;
            case 'r': byte = 13; break;
            case 't': byte = 9; break;
            case 'x': {
                unsigned value = 0;
                for (unsigned i = 0; i < 2; ++i) {
                    unsigned char digit = *p++;
                    if (!digit) return 0;
                    if (digit >= '0' && digit <= '9') value = value * 16 + digit - '0';
                    else if (digit >= 'a' && digit <= 'f') value = value * 16 + digit - 'a' + 10;
                    else if (digit >= 'A' && digit <= 'F') value = value * 16 + digit - 'A' + 10;
                    else return 0;
                }
                byte = (unsigned char)value;
                break;
            }
            default:
                if (byte >= '0' && byte <= '3') {
                    unsigned value = byte - '0';
                    for (unsigned i = 0; i < 2; ++i) {
                        unsigned char digit = *p++;
                        if (digit < '0' || digit > '7') return 0;
                        value = value * 8 + digit - '0';
                    }
                    byte = (unsigned char)value;
                }
                break;
            }
        } else if (byte == '^') {
            if (!*p) return 0;
            byte = *p++ & 31;
        }
        if (!byte || size == capacity) return 0;
        out[size++] = byte;
    }
    return size;
}

static void emit(Loader *loader, SvPrefEffectKind kind, SvPrefOwner owner,
                 const char *file, size_t line, const unsigned char *bytes, size_t size)
{
    if (kind == SV_PREF_WARNING) ++loader->report.warnings;
    if (loader->preferences->sink.effect)
        loader->preferences->sink.effect(loader->preferences->sink.context,
                                         kind, owner, file, line, bytes, size);
}

static void warn(Loader *loader, SvPrefOwner owner, const char *file, size_t line,
                 const char *message)
{
    loader->report.complete = false;
    emit(loader, SV_PREF_WARNING, owner, file, line,
         (const unsigned char *)message, strlen(message));
}

static size_t message_bytes(const char *value, unsigned char *out, size_t capacity,
                            bool chat_prefix)
{
    size_t size = 0;
    if (chat_prefix && value[0] == '{' && value[1] == '+') {
        if (!capacity) return 0;
        out[size++] = 253;
        value += 2;
    }
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p) {
        if (size == capacity) return 0;
        if (*p == '{' && p[1] == '{') { out[size++] = '{'; ++p; }
        else out[size++] = *p == '{' && p[1] ? 255 : *p;
    }
    return size;
}

static bool keymap_record(const char *value, unsigned char *key,
                          unsigned char *command, unsigned char *direction)
{
    long fields[3];
    const char *cursor = value;
    for (size_t i = 0; i < 3; ++i) {
        char *end;
        errno = 0;
        fields[i] = strtol(cursor, &end, 0);
        if (errno == ERANGE || end == cursor ||
            (i < 2 ? *end != ':' : *end != 0)) return false;
        cursor = end + 1;
    }
    *key = (unsigned char)(fields[0] & 127);
    *command = (unsigned char)(fields[1] & 127);
    *direction = (unsigned char)(fields[2] & 127);
    if (*direction > 9 || *direction == 5) *direction = 0;
    return true;
}

static FILE *open_layer(SvPreferences *prefs, const char *name, SvPrefOwner *owner)
{
    char path[4096];
    const char *roots[] = {prefs->user_root, prefs->library_root};
    for (int i = 0; i < 2; ++i) {
        if (!roots[i] || !*roots[i] ||
            snprintf(path, sizeof(path), "%s/user/%s", roots[i], name) >= (int)sizeof(path))
            continue;
        FILE *stream = fopen(path, "rb");
        if (stream) { *owner = (SvPrefOwner)i; return stream; }
    }
    return NULL;
}

static bool load(Loader *loader, const char *name, bool required,
                 SvPrefOwner parent_owner, const char *parent_file, size_t parent_line);

static void record(Loader *loader, SvPrefOwner owner, const char *name,
                   size_t line, char *text)
{
    SvPreferences *prefs = loader->preferences;
    if (!*text || *text == ' ' || *text == '\t' ||
        (*text == '#' && text[1] != ':')) return;
    if (text[1] != ':') { warn(loader, owner, name, line, "invalid PRF record"); return; }
    char *value = text + 2;
    ++loader->report.records;
    if (*text == '%') {
        size_t length = strlen(value);
        if (loader->bootstrap && (!strcmp(value, "options.prf") ||
            !strcmp(value, "window.prf") ||
            (length >= 4 && !strcmp(value + length - 4, ".opt")))) return;
        (void)load(loader, value, true, owner, name, line); return;
    }
    if (*text == 'A') {
        loader->action_size = sv_preferences_decode(value, loader->action, sizeof(loader->action));
        if (!loader->action_size && *value) warn(loader, owner, name, line, "invalid macro action");
        else emit(loader, SV_PREF_MACRO_ACTION, owner, name, line,
                  (const unsigned char *)text, strlen(text));
        return;
    }
    if (*text == 'P' || *text == 'H' || *text == 'C' || *text == 'D') {
        unsigned char trigger[SV_MACRO_TRIGGER];
        size_t size = sv_preferences_decode(value, trigger, sizeof(trigger));
        if (!size) { warn(loader, owner, name, line, "invalid macro trigger"); return; }
        SvResult result = *text == 'D' ? sv_macros_delete(prefs->macros, trigger, size) :
            sv_macros_define(prefs->macros, trigger, size, loader->action,
                loader->action_size, *text == 'P' ? SV_MACRO_NORMAL :
                *text == 'H' ? SV_MACRO_HYBRID : SV_MACRO_COMMAND);
        if (result != SV_OK && result != SV_WAITING)
            warn(loader, owner, name, line, "macro table rejected record");
        else if (result == SV_OK)
            emit(loader, SV_PREF_MACRO, owner, name, line,
                 (const unsigned char *)text, strlen(text));
        return;
    }
    if (*text == 'X' || *text == 'Y') {
        if (!sv_options_apply_pref(prefs->options, *text, value))
            warn(loader, owner, name, line, "unknown option");
        else emit(loader, SV_PREF_OPTION, owner, name, line,
                  (const unsigned char *)text, strlen(text));
        return;
    }
    if (*text == 'S') {
        unsigned char key, command, direction;
        if (!keymap_record(value, &key, &command, &direction)) {
            warn(loader, owner, name, line, "invalid keymap record"); return;
        }
        prefs->keymap_command[key] = command;
        prefs->keymap_direction[key] = direction;
        prefs->keymap_present[key] = true;
        emit(loader, SV_PREF_KEYMAP, owner, name, line,
             (const unsigned char *)text, strlen(text));
        return;
    }
    if (*text == 'W') return; /* W is a legacy Term placement, never a new Term. */
    if (*text == '#') {
        unsigned char message[SV_MESSAGE_BYTES];
        size_t length = message_bytes(value, message, sizeof(message) - 1, true);
        if (!length) { warn(loader, owner, name, line, "invalid message"); return; }
        emit(loader, SV_PREF_MESSAGE, owner, name, line, message, length);
        return;
    }
    if (*text == '!' || *text == '?') {
        if (*text == '?' && prefs->body_macros) return;
        unsigned char translated[SV_MACRO_ACTION];
        size_t translated_size = message_bytes(value, translated, sizeof(translated) - 1, false);
        if (!translated_size) { warn(loader, owner, name, line, "invalid queued action"); return; }
        translated[translated_size] = 0;
        unsigned char action[SV_MACRO_ACTION];
        size_t size = sv_preferences_decode((const char *)translated, action, sizeof(action));
        if (!size) { warn(loader, owner, name, line, "invalid queued action"); return; }
        emit(loader, SV_PREF_ACTION, owner, name, line, action, size);
        return;
    }
    /* The current SV shell has no visual mapping owner. Keep these records
     * pending and visible until a model/renderer can consume them. */
    if (strchr("RKFUrZEIV", *text))
        warn(loader, owner, name, line, "SV visual mapping consumer unavailable");
    else warn(loader, owner, name, line, "unsupported PRF record");
}

static bool load(Loader *loader, const char *name, bool required,
                 SvPrefOwner parent_owner, const char *parent_file, size_t parent_line)
{
    char detail[640];
    const char *source = parent_file ? parent_file : name ? name : "";
    if (!safe_name(name)) {
        snprintf(detail, sizeof(detail), "invalid PRF filename: %s", name ? name : "(null)");
        warn(loader, parent_owner, source, parent_line, detail);
        return false;
    }
    for (unsigned i = 0; i < loader->depth; ++i)
        if (!strcmp(loader->names[i], name)) {
            snprintf(detail, sizeof(detail), "recursive PRF include: %s", name);
            warn(loader, parent_owner, source, parent_line, detail); return false;
        }
    if (loader->depth == SV_PREF_DEPTH) {
        snprintf(detail, sizeof(detail), "PRF include depth exceeded: %s", name);
        warn(loader, parent_owner, source, parent_line, detail); return false;
    }
    SvPrefOwner owner = SV_PREF_USER;
    FILE *stream = open_layer(loader->preferences, name, &owner);
    if (!stream) {
        if (required) {
            snprintf(detail, sizeof(detail), "missing PRF file: %s", name);
            warn(loader, parent_owner, source, parent_line, detail);
        }
        return false;
    }
    if (parent_file && loader->preferences->sink.include)
        loader->preferences->sink.include(loader->preferences->sink.context,
            parent_owner, parent_file, parent_line, owner, name);
    strcpy(loader->names[loader->depth++], name);
    ++loader->report.files;
    char text[1024];
    for (size_t line = 1; fgets(text, sizeof(text), stream); ++line) {
        size_t size = strlen(text);
        if (!size) continue;
        if (text[size - 1] != '\n' && !feof(stream)) {
            int byte;
            while ((byte = fgetc(stream)) != '\n' && byte != EOF) {}
            warn(loader, owner, name, line, "long PRF record");
            continue;
        }
        if (text[size - 1] == '\n') text[--size] = 0;
        if (size && text[size - 1] == '\r') text[--size] = 0;
        record(loader, owner, name, line, text);
    }
    if (ferror(stream)) warn(loader, owner, name, 0, "PRF read failed");
    if (fclose(stream)) warn(loader, owner, name, 0, "PRF close failed");
    --loader->depth;
    return true;
}

SvPrefReport sv_preferences_load_named(SvPreferences *preferences, const char *name,
                                       bool manual)
{
    Loader loader = {.preferences = preferences, .report = {.complete = true}};
    if (!preferences || !preferences->options || !preferences->macros) {
        loader.report.complete = false; return loader.report;
    }
    memcpy(loader.action, preferences->pending_action, preferences->pending_action_size);
    loader.action_size = preferences->pending_action_size;
    (void)load(&loader, name, manual, SV_PREF_USER, NULL, 0);
    memcpy(preferences->pending_action, loader.action, loader.action_size);
    preferences->pending_action_size = loader.action_size;
    return loader.report;
}

static void merge(SvPrefReport *target, SvPrefReport source)
{
    target->files += source.files;
    target->records += source.records;
    target->warnings += source.warnings;
    target->complete &= source.complete;
}

SvPrefReport sv_preferences_bootstrap(SvPreferences *preferences)
{
    SvPrefReport report = {.complete = true};
    if (!preferences || !preferences->options || !preferences->macros) {
        report.complete = false; return report;
    }
    sv_options_defaults(preferences->options);
    Loader loader = {.preferences = preferences, .report = {.complete = true},
                     .bootstrap = true};
    memcpy(loader.action, preferences->pending_action, preferences->pending_action_size);
    loader.action_size = preferences->pending_action_size;
    (void)load(&loader, "pref.prf", false, SV_PREF_BUNDLED, NULL, 0);
    (void)load(&loader, "pref-sdl3.prf", false, SV_PREF_BUNDLED, NULL, 0);
    memcpy(preferences->pending_action, loader.action, loader.action_size);
    preferences->pending_action_size = loader.action_size;
    merge(&report, loader.report);
    if (!sv_options_load_own_layers(preferences->options, preferences->user_root))
        report.complete = false;
    return report;
}

static void layer(SvPreferences *preferences, SvPrefReport *report, const char *name)
{
    if (!name || !*name) return;
    char file[512];
    if (!safe_name(name) || snprintf(file, sizeof(file), "%s.prf", name) >= (int)sizeof(file)) {
        report->complete = false; ++report->warnings; return;
    }
    merge(report, sv_preferences_load_named(preferences, file, false));
}

SvPrefReport sv_preferences_character(SvPreferences *preferences, const char *character,
                                      const char *race, const char *trait,
                                      const char *class_name, const char *form)
{
    SvPrefReport report = {.complete = true};
    if (!preferences || !preferences->options || !preferences->macros ||
        !safe_name(character)) { report.complete = false; return report; }
    if (!sv_options_load_character(preferences->options, preferences->user_root, character))
        report.complete = false;
    merge(&report, sv_preferences_load_named(preferences, "global.prf", false));
    merge(&report, sv_preferences_character_layers(preferences, character, race,
                                                  trait, class_name, form, false));
    return report;
}

SvPrefReport sv_preferences_character_layers(SvPreferences *preferences,
    const char *character, const char *race, const char *trait,
    const char *class_name, const char *form, bool load_options)
{
    SvPrefReport report = {.complete = true};
    if (!preferences || !preferences->options || !preferences->macros ||
        !safe_name(character)) { report.complete = false; return report; }
    if (load_options && !sv_options_load_character(preferences->options,
                                                   preferences->user_root, character))
        report.complete = false;
    layer(preferences, &report, race);
    layer(preferences, &report, trait);
    layer(preferences, &report, class_name);
    layer(preferences, &report, character);
    bool load_form = true;
    (void)sv_options_get(preferences->options, "load_form_macros", &load_form);
    if (load_form && form && *form && strcmp(form, "Player")) {
        char file[512];
        if (!safe_name(form) || snprintf(file, sizeof(file), "%s^%s.prf", character, form) >=
            (int)sizeof(file)) report.complete = false;
        else {
            bool previous = preferences->body_macros;
            preferences->body_macros = true;
            merge(&report, sv_preferences_load_named(preferences, file, false));
            preferences->body_macros = previous;
        }
    }
    return report;
}

SvPrefReport sv_preferences_load_class(SvPreferences *preferences, const char *class_name)
{
    SvPrefReport report = {.complete = true};
    if (!preferences || !safe_name(class_name)) { report.complete = false; return report; }
    char file[512];
    if (snprintf(file, sizeof(file), "%s.prf", class_name) >= (int)sizeof(file)) {
        report.complete = false; ++report.warnings; return report;
    }
    return sv_preferences_load_named(preferences, file, true);
}

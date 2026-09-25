#include "settings.h"
#include <SDL3/SDL.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#define sv_open _open
#define sv_write _write
#define sv_close _close
#define sv_sync _commit
#ifndef O_BINARY
#define O_BINARY _O_BINARY
#endif
#else
#include <unistd.h>
#define sv_open open
#define sv_write write
#define sv_close close
#define sv_sync fsync
#define O_BINARY 0
#endif

#define SV_SETTINGS_LIMIT (1024 * 1024)
static const char *const cfg_keys[] = {
    "svWindowMode", "svLayout", "svUiScalePercent", "graphics", "fps",
    "svTextFont", "svMapFont", "graphic_tiles", "soundpackFolder",
    "musicpackFolder", "svGraphicsFilter", "svPcfFilter", "cacheAudio",
    "audioSampleRate", "audioChannels", "audioBuffer", "soundpackSubset",
    "musicpackSubset"
};
typedef char SvCfgConflictSize[(sizeof(cfg_keys) / sizeof(*cfg_keys) == 18) ? 1 : -1];

static bool read_file(const char *path, char **out, size_t *size)
{
    *out = NULL;
    *size = 0;
    FILE *file = fopen(path, "rb");
    if (!file) {
        if (errno != ENOENT) return false;
        *out = malloc(1);
        if (*out) **out = 0;
        return *out != NULL;
    }
    if (fseek(file, 0, SEEK_END)) { fclose(file); return false; }
    long length = ftell(file);
    if (length < 0 || length > SV_SETTINGS_LIMIT || fseek(file, 0, SEEK_SET)) {
        fclose(file); return false;
    }
    char *bytes = malloc((size_t)length + 1);
    if (!bytes) { fclose(file); return false; }
    bool ok = fread(bytes, 1, (size_t)length, file) == (size_t)length && !ferror(file);
    if (fclose(file)) ok = false;
    if (!ok || memchr(bytes, 0, (size_t)length)) { free(bytes); return false; }
    bytes[length] = 0;
    *out = bytes;
    *size = (size_t)length;
    return true;
}

static bool open_owned(SvSettingsFile *file, const char *path)
{
    int written = snprintf(file->path, sizeof(file->path), "%s", path);
    return written >= 0 && (size_t)written < sizeof(file->path) &&
           read_file(file->path, &file->opened, &file->opened_size);
}

bool sv_settings_begin(SvSettings *s, const char *root,
                       const SvProfile *profile, const SvOptions *options)
{
    if (!s || !root || !*root || !profile || !options) return false;
    memset(s, 0, sizeof(*s));
    int written = snprintf(s->root, sizeof(s->root), "%s", root);
    if (written < 0 || (size_t)written >= sizeof(s->root)) return false;
    s->opening_profile = s->saved_profile = s->profile = *profile;
    s->opening_options = s->saved_options = s->options = *options;
    char path[4096];
    written = snprintf(path, sizeof(path), "%s/sv/tomenet.cfg", root);
    if (written < 0 || (size_t)written >= sizeof(path) ||
        !open_owned(&s->cfg, path)) {
        sv_settings_end(s);
        return false;
    }
    return true;
}

void sv_settings_end(SvSettings *s)
{
    if (!s) return;
    free(s->cfg.opened);
    free(s->opt.opened);
    memset(s, 0, sizeof(*s));
}

bool sv_settings_edit_cfg(SvSettings *s, const char *key, const char *value)
{
    return s && sv_profile_edit(&s->profile, key, value);
}

bool sv_settings_edit_option(SvSettings *s, const char *name, bool value)
{
    return s && sv_options_set(&s->options, name, value);
}

bool sv_settings_dirty(const SvSettings *s)
{
    if (!s) return false;
    for (size_t i = 0; i < sizeof(cfg_keys) / sizeof(*cfg_keys); ++i) {
        char before[4096], after[4096];
        if (sv_profile_value(&s->saved_profile, cfg_keys[i], before, sizeof(before)) &&
            sv_profile_value(&s->profile, cfg_keys[i], after, sizeof(after)) &&
            strcmp(before, after)) return true;
    }
    for (size_t i = 0; i < SV_OPTION_COUNT; ++i)
        if (s->saved_options.value[i] != s->options.value[i]) return true;
    return false;
}

void sv_settings_cancel(SvSettings *s)
{
    if (!s) return;
    s->profile = s->opening_profile;
    s->options = s->opening_options;
    memset(s->cfg_conflicts, 0, sizeof(s->cfg_conflicts));
    memset(s->opt_conflicts, 0, sizeof(s->opt_conflicts));
}

bool sv_settings_options_target(SvSettings *s, SvOptionsScope scope, const char *name)
{
    if (!s || scope < SV_OPTIONS_CHARACTER || scope > SV_OPTIONS_NAMED) return false;
    char filename[512], path[4096];
    if (scope == SV_OPTIONS_GLOBAL) strcpy(filename, "global.opt");
    else if (!sv_options_safe_name(name)) return false;
    else if (scope == SV_OPTIONS_NAMED) {
        if (snprintf(filename, sizeof(filename), "%s", name) >= (int)sizeof(filename))
            return false;
    } else if (snprintf(filename, sizeof(filename), "%s.opt", name) >= (int)sizeof(filename))
        return false;
    if (snprintf(path, sizeof(path), "%s/sv/%s", s->root, filename) >= (int)sizeof(path))
        return false;
    SvSettingsFile next = {0};
    if (!open_owned(&next, path)) return false;
    free(s->opt.opened);
    s->opt = next;
    memset(s->opt_conflicts, 0, sizeof(s->opt_conflicts));
    s->options_selected = true;
    s->options_snapshot = scope != SV_OPTIONS_CHARACTER;
    return true;
}

/* Last complete assignment wins. Missing and empty are distinct. */
static bool record_value(const char *bytes, size_t size, bool option,
                         const char *key, char *out, size_t capacity)
{
    bool found = false;
    size_t key_size = strlen(key);
    for (size_t at = 0; at < size;) {
        size_t end = at;
        while (end < size && bytes[end] != '\n') ++end;
        size_t stop = end;
        if (stop > at && bytes[stop - 1] == '\r') --stop;
        size_t start = at;
        if (!option) while (start < stop && (bytes[start] == ' ' || bytes[start] == '\t')) ++start;
        if (option) {
            if (stop - start == key_size + 2 &&
                (bytes[start] == 'Y' || bytes[start] == 'X') &&
                bytes[start + 1] == ':' &&
                !memcmp(bytes + start + 2, key, key_size) && capacity >= 2) {
                out[0] = bytes[start]; out[1] = 0; found = true;
            }
        } else if (stop - start >= key_size && !memcmp(bytes + start, key, key_size) &&
                   (stop - start == key_size || bytes[start + key_size] == ' ' ||
                    bytes[start + key_size] == '\t')) {
            start += key_size;
            while (start < stop && (bytes[start] == ' ' || bytes[start] == '\t')) ++start;
            size_t length = stop - start;
            if (length < capacity) {
                memcpy(out, bytes + start, length);
                out[length] = 0;
                found = true;
            }
        }
        at = end < size ? end + 1 : end;
    }
    return found;
}

static bool same_record(const SvSettingsFile *file, const char *current,
                        size_t size, bool option, const char *key)
{
    char old[4096], now[4096];
    bool old_found = record_value(file->opened, file->opened_size, option,
                                  key, old, sizeof(old));
    bool now_found = record_value(current, size, option, key, now, sizeof(now));
    return old_found == now_found && (!old_found || !strcmp(old, now));
}

static bool append(char *out, size_t *size, size_t capacity,
                   const char *bytes, size_t length)
{
    if (*size >= capacity || length >= capacity - *size) return false;
    memcpy(out + *size, bytes, length);
    *size += length;
    out[*size] = 0;
    return true;
}

static bool is_record(const char *line, size_t length, bool option, const char *key)
{
    size_t start = 0, key_size = strlen(key);
    if (option)
        return length == key_size + 2 && (line[0] == 'X' || line[0] == 'Y') &&
               line[1] == ':' && !memcmp(line + 2, key, key_size);
    while (start < length && (line[start] == ' ' || line[start] == '\t')) ++start;
    return length - start >= key_size && !memcmp(line + start, key, key_size) &&
           (length - start == key_size || line[start + key_size] == ' ' ||
            line[start + key_size] == '\t');
}

static bool contains_record(const char *bytes, size_t size, bool option,
                            const char *key)
{
    for (size_t at = 0; at < size;) {
        size_t end = at;
        while (end < size && bytes[end] != '\n') ++end;
        size_t stop = end;
        if (stop > at && bytes[stop - 1] == '\r') --stop;
        if (is_record(bytes + at, stop - at, option, key)) return true;
        at = end < size ? end + 1 : end;
    }
    return false;
}

static bool next_temp(const char *path, char out[4096], int *fd)
{
    for (unsigned attempt = 0; attempt < 32; ++attempt) {
        int written = snprintf(out, 4096, "%s.sv-%llx-%u",
                               path, (unsigned long long)SDL_GetTicksNS(), attempt);
        if (written < 0 || written >= 4096) return false;
        *fd = sv_open(out, O_WRONLY | O_CREAT | O_EXCL | O_BINARY, 0600);
        if (*fd >= 0) return true;
        if (errno != EEXIST) return false;
    }
    return false;
}

static bool write_all(int fd, const char *bytes, size_t size)
{
    size_t at = 0;
    while (at < size) {
        int count = sv_write(fd, bytes + at, (unsigned)(size - at));
        if (count <= 0) return false;
        at += (size_t)count;
    }
    return sv_sync(fd) == 0;
}

static bool publish(const SvSettingsFile *file, const char *bytes, size_t size,
                    const char *previous, size_t previous_size, bool backup_safe)
{
    char temporary[4096], backup[4096];
    int fd;
    if (!next_temp(file->path, temporary, &fd)) return false;
    bool ok = write_all(fd, bytes, size);
    if (sv_close(fd)) ok = false;
    if (!ok) { SDL_RemovePath(temporary); return false; }
    bool have_backup = false;
    if (previous_size && backup_safe) {
        if (!next_temp(file->path, backup, &fd)) {
            SDL_RemovePath(temporary); return false;
        }
        have_backup = true;
        ok = write_all(fd, previous, previous_size);
        if (sv_close(fd)) ok = false;
        if (!ok) {
            SDL_RemovePath(backup);
            SDL_RemovePath(temporary);
            return false;
        }
    }
    ok = SDL_RenamePath(temporary, file->path);
    if (!ok) SDL_RemovePath(temporary);
    if (have_backup) SDL_RemovePath(backup);
    return ok;
}

static SvSettingsResult save(SvSettings *s, bool option)
{
    SvSettingsFile *file = option ? &s->opt : &s->cfg;
    if (!file->path[0] || (!option && s->profile.incompatible_schema))
        return SV_SETTINGS_INVALID;
    char *current = NULL;
    size_t current_size = 0;
    if (!read_file(file->path, &current, &current_size)) return SV_SETTINGS_IO_ERROR;
    char schema[32];
    if (!option && record_value(current, current_size, false, "svSchemaVersion",
                                schema, sizeof(schema)) && strcmp(schema, "1")) {
        free(current); return SV_SETTINGS_INVALID;
    }
    size_t count = option ? 188 : sizeof(cfg_keys) / sizeof(*cfg_keys);
    bool changed[188] = {0};
    bool conflicts = false, any = false;
    char values[188][4096];
    for (size_t i = 0; i < count; ++i) {
        const char *key = option ? sv_options_name(i) : cfg_keys[i];
        bool differs;
        if (option) {
            differs = s->options_snapshot ||
                      s->saved_options.value[i] != s->options.value[i];
            values[i][0] = s->options.value[i] ? 'Y' : 'X';
            values[i][1] = 0;
        } else {
            char before[4096];
            if (!sv_profile_value(&s->saved_profile, key, before, sizeof(before)) ||
                !sv_profile_value(&s->profile, key, values[i], sizeof(values[i]))) {
                free(current); return SV_SETTINGS_INVALID;
            }
            differs = strcmp(before, values[i]) != 0;
        }
        if (!differs) continue;
        bool *pinned = option ? &s->opt_conflicts[i] : &s->cfg_conflicts[i];
        if (*pinned || !same_record(file, current, current_size, option, key)) {
            *pinned = true;
            conflicts = true;
        } else { changed[i] = true; any = true; }
    }
    if (!option && contains_record(current, current_size, false, "pass"))
        any = true;
    if (!any) { free(current); return conflicts ? SV_SETTINGS_CONFLICT : SV_SETTINGS_OK; }
    size_t capacity = SV_SETTINGS_LIMIT + count * 4100u + 128u;
    char *out = malloc(capacity);
    if (!out) { free(current); return SV_SETTINGS_IO_ERROR; }
    size_t used = 0;
    bool ok = true;
    if (!option && !current_size)
        ok = append(out, &used, capacity, "svSchemaVersion\t1\n", 18);
    for (size_t at = 0; ok && at < current_size;) {
        size_t end = at;
        while (end < current_size && current[end] != '\n') ++end;
        size_t line_end = end;
        if (line_end > at && current[line_end - 1] == '\r') --line_end;
        bool remove = !option && is_record(current + at, line_end - at, false, "pass");
        for (size_t i = 0; i < count && !remove; ++i)
            if (changed[i] && is_record(current + at, line_end - at, option,
                                        option ? sv_options_name(i) : cfg_keys[i]))
                remove = true;
        size_t after = end < current_size ? end + 1 : end;
        if (!remove) ok = append(out, &used, capacity, current + at, after - at);
        at = after;
    }
    if (ok && used && out[used - 1] != '\n')
        ok = append(out, &used, capacity, "\n", 1);
    for (size_t i = 0; ok && i < count; ++i) {
        if (!changed[i]) continue;
        char row[8192];
        int length = option ?
            snprintf(row, sizeof(row), "%c:%s\n", values[i][0], sv_options_name(i)) :
            snprintf(row, sizeof(row), "%s\t%s\n", cfg_keys[i], values[i]);
        ok = length >= 0 && (size_t)length < sizeof(row) &&
             append(out, &used, capacity, row, (size_t)length);
    }
    char dir[4096];
    int dir_length = snprintf(dir, sizeof(dir), "%s/sv", s->root);
    if (ok && dir_length >= 0 && (size_t)dir_length < sizeof(dir)) {
        if (!SDL_CreateDirectory(dir)) ok = false;
    } else ok = false;
    /* Never create a backup containing a legacy plaintext password. The
     * destination itself remains intact until SDL_RenamePath succeeds. */
    bool safe_backup = option || !contains_record(current, current_size, false, "pass");
    if (ok) ok = publish(file, out, used, current, current_size, safe_backup);
    if (ok) {
        free(file->opened);
        file->opened = out;
        file->opened_size = used;
        for (size_t i = 0; i < count; ++i) if (changed[i]) {
            if (option) s->saved_options.value[i] = s->options.value[i];
            else {
                (void)sv_profile_edit(&s->saved_profile, cfg_keys[i], values[i]);
            }
        }
    } else free(out);
    free(current);
    return !ok ? SV_SETTINGS_IO_ERROR :
           conflicts ? SV_SETTINGS_CONFLICT : SV_SETTINGS_OK;
}

SvSettingsResult sv_settings_save_cfg(SvSettings *s)
{
    return s ? save(s, false) : SV_SETTINGS_INVALID;
}

SvSettingsResult sv_settings_save_options(SvSettings *s)
{
    return s && s->options_selected ? save(s, true) : SV_SETTINGS_INVALID;
}

#include "profile.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sv_profile_defaults(SvProfile *p)
{
    memset(p, 0, sizeof(*p));
    p->wide = p->cache_audio = true;
    p->ui_scale = 100;
    p->graphics = 1;
    p->fps = 100;
    p->audio_rate = 44100;
    p->audio_tracks = 32;
    p->audio_buffer = 1024;
    p->sound_subset = p->music_subset = 1;
    strcpy(p->text_font, "CascadiaMono-Regular.ttf");
    strcpy(p->map_font, "16x24x.pcf");
    strcpy(p->tiles, "16x24sv");
    strcpy(p->graphics_filter, "linear");
    strcpy(p->pcf_filter, "nearest");
    strcpy(p->sound_pack, "sound");
    strcpy(p->music_pack, "music");
}

static bool number(const char *value, int min, int max, int *out)
{
    char *end;
    long parsed;
    if (!*value) return false;
    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno || *end || parsed < min || parsed > max) return false;
    *out = (int)parsed;
    return true;
}

static bool name(char *out, size_t capacity, const char *value)
{
    size_t size = strlen(value);
    if (!size || size >= capacity || !strcmp(value, ".") || !strcmp(value, "..")) return false;
    for (const unsigned char *c = (const unsigned char *)value; *c; ++c)
        if (*c < 32 || *c == 127 || *c == '/' || *c == '\\') return false;
    memcpy(out, value, size + 1);
    return true;
}

static bool choice(char *out, size_t capacity, const char *value,
                   const char *a, const char *b, const char *c)
{
    return (!strcmp(value, a) || !strcmp(value, b) || (c && !strcmp(value, c))) &&
           name(out, capacity, value);
}

static void apply(SvProfile *p, const SvProfile *defaults, const char *key,
                  const char *value, size_t line)
{
    bool valid = true, known = true;
#define SV_NUM(field, lower, upper) \
    do { p->field = defaults->field; valid = number(value, lower, upper, &p->field); } while (0)
#define SV_NAME(field) \
    do { p->field[0] = 0; valid = name(p->field, sizeof(p->field), value); \
         if (!valid) memcpy(p->field, defaults->field, sizeof(p->field)); } while (0)
    if (!strcmp(key, "svSchemaVersion")) return;
    else if (!strcmp(key, "svWindowMode")) {
        p->windowed = defaults->windowed;
        valid = !strcmp(value, "fullscreen") || !strcmp(value, "window");
        if (valid) p->windowed = !strcmp(value, "window");
    } else if (!strcmp(key, "svLayout")) {
        p->wide = defaults->wide;
        valid = !strcmp(value, "wide") || !strcmp(value, "small");
        if (valid) p->wide = !strcmp(value, "wide");
    } else if (!strcmp(key, "svUiScalePercent")) {
        SV_NUM(ui_scale, 50, 200);
        if (p->ui_scale % 5) { p->ui_scale = defaults->ui_scale; valid = false; }
    } else if (!strcmp(key, "graphics")) SV_NUM(graphics, 0, 2);
    else if (!strcmp(key, "fps")) SV_NUM(fps, 1, 1000);
    else if (!strcmp(key, "svTextFont")) SV_NAME(text_font);
    else if (!strcmp(key, "svMapFont")) SV_NAME(map_font);
    else if (!strcmp(key, "graphic_tiles")) SV_NAME(tiles);
    else if (!strcmp(key, "soundpackFolder")) SV_NAME(sound_pack);
    else if (!strcmp(key, "musicpackFolder")) SV_NAME(music_pack);
    else if (!strcmp(key, "svGraphicsFilter")) {
        strcpy(p->graphics_filter, defaults->graphics_filter);
        valid = choice(p->graphics_filter, sizeof(p->graphics_filter), value,
                       "nearest", "linear", "pixelart");
    } else if (!strcmp(key, "svPcfFilter")) {
        strcpy(p->pcf_filter, defaults->pcf_filter);
        valid = choice(p->pcf_filter, sizeof(p->pcf_filter), value,
                       "nearest", "linear", "pixelart");
    } else if (!strcmp(key, "cacheAudio")) {
        int enabled = defaults->cache_audio;
        valid = number(value, 0, 1, &enabled);
        p->cache_audio = enabled != 0;
    } else if (!strcmp(key, "audioSampleRate")) SV_NUM(audio_rate, 4000, 48000);
    else if (!strcmp(key, "audioChannels")) SV_NUM(audio_tracks, 4, 32);
    else if (!strcmp(key, "audioBuffer")) SV_NUM(audio_buffer, 128, 8192);
    else if (!strcmp(key, "soundpackSubset")) SV_NUM(sound_subset, 1, 255);
    else if (!strcmp(key, "musicpackSubset")) SV_NUM(music_subset, 1, 255);
    else known = false;
#undef SV_NUM
#undef SV_NAME
    if (known && !valid)
        fprintf(stderr, "SV profile invalid value for %s at line %zu; using default\n", key, line);
}

bool sv_profile_load(SvProfile *profile, const char *user_root)
{
    char path[4096];
    SvProfile defaults;
    sv_profile_defaults(profile);
    defaults = *profile;
    if (!user_root || !*user_root ||
        snprintf(path, sizeof(path), "%s/sv/tomenet.cfg", user_root) >= (int)sizeof(path))
        return false;
    FILE *stream = fopen(path, "rb");
    if (!stream) {
        if (errno != ENOENT)
            fprintf(stderr, "SV profile CFG unreadable; using defaults\n");
        return true;
    }
    if (fseek(stream, 0, SEEK_END)) { fclose(stream); return false; }
    long length = ftell(stream);
    if (length < 0) { fclose(stream); return false; }
    if (length > 1024 * 1024) {
        fprintf(stderr, "SV profile corrupt CFG: file too large; using defaults\n");
        fclose(stream);
        return true;
    }
    rewind(stream);
    char *bytes = malloc((size_t)length + 1);
    if (!bytes) { fclose(stream); return false; }
    bool ok = fread(bytes, 1, (size_t)length, stream) == (size_t)length && !ferror(stream);
    if (fclose(stream)) ok = false;
    if (!ok) { free(bytes); return false; }
    bytes[length] = 0;
    if (memchr(bytes, 0, (size_t)length)) {
        fprintf(stderr, "SV profile corrupt CFG: embedded NUL; using defaults\n");
        free(bytes);
        return true;
    }
    /* Check every schema declaration before applying any field. */
    for (char *cursor = bytes; *cursor;) {
        char *end = strchr(cursor, '\n');
        size_t size = end ? (size_t)(end - cursor) : strlen(cursor);
        if (size && cursor[size - 1] == '\r') --size;
        if (size >= 15 && !memcmp(cursor, "svSchemaVersion", 15) &&
            (size == 15 || cursor[15] == ' ' || cursor[15] == '\t')) {
            char saved = cursor[size]; cursor[size] = 0;
            char *value = cursor + 15;
            while (*value == ' ' || *value == '\t') ++value;
            if (strcmp(value, "1")) {
                profile->incompatible_schema = true;
                fprintf(stderr, "SV profile incompatible schema; using defaults\n");
                cursor[size] = saved;
                free(bytes);
                return true;
            }
            cursor[size] = saved;
        }
        cursor = end ? end + 1 : cursor + strlen(cursor);
    }
    size_t line = 0;
    for (char *cursor = bytes; *cursor;) {
        char *end = strchr(cursor, '\n');
        if (end) *end = 0;
        ++line;
        size_t size = strlen(cursor);
        if (size && cursor[size - 1] == '\r') cursor[--size] = 0;
        char *key = cursor;
        while (*key == ' ' || *key == '\t') ++key;
        if (*key && *key != '#') {
            char *value = key;
            while (*value && *value != ' ' && *value != '\t') ++value;
            if (*value) *value++ = 0;
            while (*value == ' ' || *value == '\t') ++value;
            apply(profile, &defaults, key, value, line);
        }
        cursor = end ? end + 1 : cursor + size;
    }
    free(bytes);
    return true;
}

void sv_profile_report(const SvProfile *p)
{
    printf("SV profile window=%s ui_scale=%d layout=%s graphics=%d tiles=%s text=%s map=%s fps=%d\n",
           p->windowed ? "window" : "fullscreen", p->ui_scale, p->wide ? "wide" : "small",
           p->graphics, p->tiles, p->text_font, p->map_font, p->fps);
    printf("SV profile audio requested rate=%d tracks=%d buffer=%d cache=%d sound_pack=%s sound_subset=%d music_pack=%s music_subset=%d\n",
           p->audio_rate, p->audio_tracks, p->audio_buffer, p->cache_audio,
           p->sound_pack, p->sound_subset, p->music_pack, p->music_subset);
}

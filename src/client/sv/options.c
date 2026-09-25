#include "options.h"
#include "../../common/pack.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const char *name; bool default_value; } SvOptionInfo;
static const SvOptionInfo option_info[] = {
#include "options-table.inc"
};
typedef char SvOptionTableMatchesBaseline[(sizeof(option_info) / sizeof(*option_info) ==
                                           188) ? 1 : -1];

static int find_option(const char *name)
{
    for (size_t i = 0; i < sizeof(option_info) / sizeof(*option_info); ++i)
        if (!strcmp(name, option_info[i].name)) return (int)i;
    return -1;
}

void sv_options_defaults(SvOptions *options)
{
    memset(options, 0, sizeof(*options));
    for (size_t i = 0; i < sizeof(option_info) / sizeof(*option_info); ++i)
        options->value[i] = option_info[i].default_value;
}

bool sv_options_get(const SvOptions *options, const char *name, bool *value)
{
    int slot = name ? find_option(name) : -1;
    if (!options || !value || slot < 0) return false;
    *value = options->value[slot];
    return true;
}

static void set_named(SvOptions *options, const char *name, bool value)
{
    int slot = find_option(name);
    if (slot >= 0) options->value[slot] = value;
}

static void apply_line(SvOptions *options, char *line, bool *converted)
{
    if ((line[0] != 'X' && line[0] != 'Y') || line[1] != ':') return;
    bool value = line[0] == 'Y';
    char *name = line + 2;
    size_t length = strlen(name);
    if (length && name[length - 1] == '\r') name[--length] = 0;
    if (!length) return;
    static const struct { const char *old_name, *new_name; } aliases[] = {
        {"recall_flicker", "subterm_flicker"},
        {"autoloot_depth", "autoloot_dunonly"},
        {"autoloot_off", "autoloot_dunonly"},
        {"kind_diz", "add_kind_diz"},
        {"auto_inscribe", "auto_inscr_server"},
        {"hilite_chat", "highlight_chat"},
        {"hibeep_chat", "highbeep_chat"},
        {"view_animated_lite", "view_animated_light"},
        {"view_lite_extra", "view_light_extra"},
        {"no_lite_fainting", "no_light_fainting"},
        {"hilite_player", "highlight_player"},
        {"colourize_prices", "colourize_bignum"},
        {"sp_huge_bar", "sn_huge_bar"},
        {"auto_insc_off", "auto_inscr_off"},
        {"stack_allow_wands", "stack_allow_devices"},
    };
    if (!strcmp(name, "view_reduce_lite")) { *converted = true; return; }
    if (!strcmp(name, "instant_retaliator")) {
        name = "new_retaliator"; value = !value; *converted = true;
    } else if (!strcmp(name, "basic_players") ||
               !strcmp(name, "consistent_players")) {
        name = "basic_players_symb";
        set_named(options, "basic_players_col", false);
        *converted = true;
    } else {
        for (size_t i = 0; i < sizeof(aliases) / sizeof(*aliases); ++i)
            if (!strcmp(name, aliases[i].old_name)) {
                name = (char *)aliases[i].new_name;
                *converted = true;
                break;
            }
    }
    set_named(options, name, value);
}

static bool load_file(SvOptions *options, const char *path)
{
    FILE *stream = fopen(path, "rb");
    if (!stream) return errno == ENOENT; /* Missing own OPT keeps defaults. */
    if (fseek(stream, 0, SEEK_END)) { fclose(stream); return false; }
    long length = ftell(stream);
    if (length < 0) { fclose(stream); return false; }
    if (length > 1024 * 1024) {
        fclose(stream);
        fprintf(stderr, "SV options: oversized file skipped\n");
        return true;
    }
    if (fseek(stream, 0, SEEK_SET)) { fclose(stream); return false; }
    char *bytes = malloc((size_t)length + 1);
    if (!bytes) { fclose(stream); return false; }
    bool ok = fread(bytes, 1, (size_t)length, stream) == (size_t)length && !ferror(stream);
    if (fclose(stream)) ok = false;
    if (!ok) { free(bytes); return false; }
    bytes[length] = 0;
    if (memchr(bytes, 0, (size_t)length)) {
        fprintf(stderr, "SV options: NUL-bearing file skipped\n");
        free(bytes);
        return true;
    }
    bool converted = false;
    for (char *cursor = bytes; *cursor;) {
        char *end = strchr(cursor, '\n');
        size_t size = end ? (size_t)(end - cursor) : strlen(cursor);
        if (size < 4096) {
            if (end) *end = 0;
            apply_line(options, cursor, &converted);
        } else fprintf(stderr, "SV options: oversized record skipped\n");
        cursor = end ? end + 1 : cursor + size;
    }
    free(bytes);
    if (converted) fprintf(stderr, "SV options: obsolete names converted in memory\n");
    return true;
}

static bool own_path(char out[4096], const char *root, const char *filename)
{
    return root && *root && filename &&
           snprintf(out, 4096, "%s/sv/%s", root, filename) < 4096;
}

bool sv_options_load_base(SvOptions *options, const char *user_root)
{
    char path[4096];
    if (!options || !user_root || !*user_root) return false;
    sv_options_defaults(options);
    /* Dedicated option entrypoint precedes the global and system snapshots. */
    const char *files[] = {"options.prf", "global.opt", "global-sv.opt"};
    for (size_t i = 0; i < sizeof(files) / sizeof(*files); ++i)
        if (!own_path(path, user_root, files[i]) || !load_file(options, path)) return false;
    return true;
}

bool sv_options_load_character(SvOptions *options, const char *user_root,
                               const char *character)
{
    char name[512], path[4096];
    if (!options || !character || !*character || strlen(character) > 500) return false;
    for (const unsigned char *p = (const unsigned char *)character; *p; ++p)
        if (*p < 32 || *p == 127 || *p == '/' || *p == '\\' || *p == ':') return false;
    if (!strcmp(character, ".") || !strcmp(character, "..") ||
        snprintf(name, sizeof(name), "%s.opt", character) >= (int)sizeof(name) ||
        !own_path(path, user_root, name)) return false;
    return load_file(options, path);
}

static bool newer(const int version[6], const int gate[6])
{
    for (int i = 0; i < 6; ++i)
        if (version[i] != gate[i]) return version[i] > gate[i];
    return false;
}

size_t sv_options_packet(const SvOptions *options, const int version[6],
                         uint8_t *out, size_t capacity)
{
    static const int newest[6] = {4, 9, 1, 0, 0, 0};
    static const int middle[6] = {4, 5, 8, 1, 0, 1};
    static const int oldest[6] = {4, 5, 5, 0, 0, 0};
    if (!options || !version || !out) return 0;
    size_t count = newer(version, newest) ? 199 : newer(version, middle) ? 154 :
                   newer(version, oldest) ? 128 : 96;
    if (capacity < count + 1) return 0;
    out[0] = PKT_OPTIONS;
    for (size_t i = 0; i < count; ++i) out[i + 1] = options->value[i] ? 1 : 0;
    return count + 1;
}

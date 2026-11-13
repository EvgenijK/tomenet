#include "input/keymap.h"
#include <string.h>

enum { PREFIX_NONE, PREFIX_BYPASS, PREFIX_CONTROL, PREFIX_BYPASS_CONTROL };
#define CTRL(c) ((c) & 0x1f)
static unsigned char translate(unsigned char key, bool rogue, unsigned char *direction)
{
    *direction = 0;
    if (key >= '1' && key <= '9') {
        *direction = key - '0';
        return key == '5' ? ',' : ';';
    }
    if (!rogue) {
        switch (key) {
        case CTRL('J'): case CTRL('M'): return 13;
        case 'T': return '+';
        case ',': *direction = 5; return ',';
        default: return key;
        }
    }
    /* c-util.c roguelike_commands: these three groups encode directions. */
    const char walk[] = "bjnh.lyku";
    const char run[] = "BJNH.LYKU";
    for (unsigned i = 0; i < 9; ++i) {
        if (i == 4) continue;
        if (key == (unsigned char)walk[i]) { *direction = i + 1; return ';'; }
        if (key == (unsigned char)run[i]) { *direction = i + 1; return '.'; }
        if (key == CTRL(walk[i])) { *direction = i + 1; return '+'; }
    }
    switch (key) {
    case CTRL('F'): return CTRL('U');
    case CTRL('V'): return CTRL('N');
    case CTRL('C'): return 'K';
    case CTRL('M'): return 13;
    case CTRL('D'): return 'k';
    case CTRL('X'): return 'U';
    case CTRL('W'): return 'L';
    case 'P': return 'b';
    case CTRL('A'): return 'j';
    case '#': return 'S';
    case 'Z': return 'u';
    case 'T': return 't';
    case 't': return 'f';
    case 'f': return 'B';
    case 'x': return 'l';
    case 'z': return 'a';
    case 'a': return 'z';
    case 'O': return 'P';
    case 'S': return 'x';
    case CTRL('E'): return 'h';
    case CTRL('G'): return 'H';
    case CTRL('Z'): return CTRL('G');
    case ',': return '.';
    case '.': *direction = 5; return ',';
    default: return key;
    }
}
void sv_keymap_rebuild(SvKeymap *map)
{
    for (unsigned i = 0; i < 128; ++i)
        map->commands[i] = translate(i, map->rogue, &map->directions[i]);
}
void sv_keymap_init(SvKeymap *map, bool rogue)
{
    memset(map, 0, sizeof(*map));
    map->rogue = rogue;
    sv_keymap_rebuild(map);
}
SvResult sv_keymap_key(SvKeymap *map, unsigned char key, SvCommand *out)
{
    if (!map || !out) return SV_INVALID;
    *out = (SvCommand){0};
    if (!key) return SV_WAITING;
    if (!map->prefix) {
        if (key == '\\') { map->prefix = PREFIX_BYPASS; return SV_WAITING; }
        if (key == '^') { map->prefix = PREFIX_CONTROL; return SV_WAITING; }
    } else if (map->prefix == PREFIX_BYPASS && key == '^') {
        map->prefix = PREFIX_BYPASS_CONTROL;
        return SV_WAITING;
    }
    bool bypass = map->prefix == PREFIX_BYPASS || map->prefix == PREFIX_BYPASS_CONTROL;
    if (map->prefix == PREFIX_CONTROL || map->prefix == PREFIX_BYPASS_CONTROL)
        key = CTRL(key);
    map->prefix = PREFIX_NONE;
    if (!bypass) {
        map->last_direction = map->directions[key & 0x7f];
        key = map->commands[key & 0x7f];
    }
    /* Baseline raw bypass leaves command_dir from the last mapped command. */
    *out = (SvCommand){key ? key : 27, map->last_direction};
    return SV_OK;
}
void sv_keymap_preferences(SvKeymap *map, const SvPreferences *preferences, size_t *cursor)
{
    if (!map || !preferences || !cursor) return;
    const SvPreferenceKeymapRecord *record;
    while ((record = sv_preferences_keymap_record(preferences, *cursor))) {
        if (record->kind == SV_PREF_KEYMAP_INITIAL) {
            map->rogue = record->rogue;
            sv_keymap_rebuild(map);
        } else if (record->kind == SV_PREF_KEYMAP_OPTION) {
            map->rogue = record->rogue;
        } else {
            map->commands[record->key] = record->command;
            map->directions[record->key] = record->direction;
        }
        ++*cursor;
    }
}

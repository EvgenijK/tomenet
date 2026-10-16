#!/usr/bin/env python3
"""Exercise production profile parsing and the deferred companion pass with UBSan."""
from pathlib import Path
import re
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[3]
init = (ROOT / 'src/server/init2.c').read_text()
gen = (ROOT / 'src/server/generate.c').read_text()
def function(name):
    start = re.search(r'^static [^\n]+\b' + name + r'\(', init, re.M).start()
    opening = init.index('{', start)
    level, end = 1, opening + 1
    while level:
        level += (init[end] == '{') - (init[end] == '}')
        end += 1
    return init[start:end]
start = gen.rindex('\t\tfor (sy = 0; sy < room->hgt; sy++) {', 0, gen.index('int r_idx = primary_monsters[sy * room->wid + sx];'))
end = gen.index('\n\t\tfor (sy = 0;', start + 1)
loop = gen[start:end]
prefix = r'''
#define SERVER
#include "angband.h"
#include <assert.h>
#include <errno.h>
monster_race *r_info;
u16b max_r_idx;
monster_ego *re_info;
u16b max_re_idx;
int s_printf(const char *format, ...) { (void)format; return 0; }
int lookup_room_template_monster_flag(cptr name, int *set, u32b *mask) {
    (void)name; *set = 0; *mask = RF1_FRIENDS; return 1;
}
static int calls;
void place_monster_companions(worldpos *wpos, int y, int x, int r_idx,
        bool slp, int clo, int clone) {
    (void)wpos; (void)y; (void)x; (void)r_idx;
    (void)slp; (void)clo; (void)clone; calls++;
}
static void room_template_transform(room_template_type *room, int orientation,
        int x, int y, int *dx, int *dy) {
    (void)room; (void)orientation; *dx = x; *dy = y;
}
'''
tests = r'''
static errr parse(room_template_type *room, const char *text) {
    char buf[100]; strcpy(buf, text);
    return room_template_parse_monster_profile("test", 1, buf, room);
}
int main(void) {
    room_template_type storage = {0}, *room = &storage;
    room_template_init_monster_profiles(room);
    assert(!parse(room, "M:a:companions:none"));
    assert(room->monster_profiles['a'].no_companions);
    assert(parse(room, "M:a:companions:none"));
    const char *bad[] = {"", "yes", "default", "NONE", "none:extra"};
    for (unsigned i = 0; i < sizeof(bad)/sizeof(*bad); i++) {
        char line[100]; snprintf(line, sizeof(line), "M:b:companions:%s", bad[i]);
        assert(parse(room, line));
        assert(!room->monster_profiles['b'].no_companions);
    }
    assert(!parse(room, "M:b:"));
    assert(!parse(room, "M:a:flag:FRIENDS"));
    assert(room->monster_profiles['a'].flags[0] & RF1_FRIENDS);
    room->wid = 3; room->hgt = 1;
    room->monster_map = "aba";
    s16b primary_monsters[] = {42, 42, 42};
    worldpos *wpos = NULL;
    int sy, sx, dx, dy, top = 0, left = 0, orientation = 0;
    PASS_LOOP
    assert(calls == 1); /* Same race, only the default profile gets companions. */
    calls = 0; primary_monsters[1] = 0;
    PASS_LOOP
    assert(calls == 0); /* Failed primary gets no companions either. */
    puts("PASS: parser defaults, duplicates, invalid values, mixed profiles and failed primaries");
}
'''.replace('PASS_LOOP', loop)
helpers = '\n'.join(function(n) for n in ['room_template_error', 'room_template_number', 'room_template_symbol', 'room_template_init_monster_profiles', 'room_template_parse_monster_profile'])
with tempfile.TemporaryDirectory(prefix='companions-') as directory:
    source = Path(directory)/'check.c'
    source.write_text(prefix + helpers + tests)
    exe = Path(directory)/'check'
    subprocess.run(['cc', '-std=gnu99', '-D_DEFAULT_SOURCE', '-DMEXP=19937', '-DENABLE_ROOM_TEMPLATES', '-I'+str(ROOT/'src/server'), '-I'+str(ROOT/'src/server/lua'), '-Wall', '-Wextra', '-Werror', '-fsanitize=undefined', '-fno-sanitize-recover=all', str(source), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)

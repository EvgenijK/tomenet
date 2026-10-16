#!/usr/bin/env python3
"""Compile production eligibility/selection helpers and check explicit dungeon opt-in."""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
generator = (ROOT / 'src/server/generate.c').read_text()


def function(name):
    start = re.search(r'^static [^\n]+\b' + name + r'\(', generator, re.M).start()
    opening = generator.index('{', start)
    level, end = 1, opening + 1
    while level:
        level += (generator[end] == '{') - (generator[end] == '}')
        end += 1
    return generator[start:end]


prefix = r'''
#define SERVER
#include "angband.h"
#include <assert.h>
#define BLOCK_HGT 11
#define BLOCK_WID 11
static struct { int wid, hgt; } floor_size = {80, 60};
static struct {
    typeof(floor_size) *l_ptr;
    int row_rooms, col_rooms;
} harness_dungeon = {&floor_size, 5, 7}, *dun = &harness_dungeon;
static dungeon_info_type metadata[3];
dungeon_info_type *d_info = metadata;
u16b max_d_idx = 3;
static room_template_type templates[2];
room_template_type *room_templates = templates;
size_t room_template_count = 2;
room_template_dungeon_rule room_template_dungeon_rules[MAX_D_IDX];
static bool members[2] = {TRUE, FALSE};
static u32b multipliers[2] = {1000, 1000};
static int rng_calls;
s32b Rand_div(s32b bound) { assert(bound > 0); rng_calls++; return bound - 1; }
int s_printf(const char *format, ...) { (void)format; return 0; }
'''
tests = r'''
static void check_empty(int real, int effective) {
    u64b baseline = 99;
    int before = rng_calls;
    u64b weight = room_template_total_weight(real, effective, 20, &baseline);
    assert(weight == 0 && baseline == 0);
    assert(room_template_pick(real, effective, 20, weight) == NULL);
    assert(rng_calls == before);
}
int main(void) {
    for (int i = 0; i < 2; i++) {
        templates[i].wid = templates[i].hgt = 5;
        templates[i].min_depth = 10;
        templates[i].max_depth = 30;
        templates[i].weight = i + 1;
    }
    /* Neither the ordinary path nor ALWAYS gets candidates without I:+. */
    check_empty(0, 0);
    metadata[0].room_templates_always = TRUE;
    check_empty(0, 0);
    puts("PASS: no list disables templates, including ALWAYS, without RNG draws");

    room_template_dungeon_rule *rule = &room_template_dungeon_rules[1];
    rule->mode = ROOM_TEMPLATE_LIST_DENY;
    rule->members = members;
    rule->multipliers = multipliers;
    check_empty(0, 1);
    puts("PASS: legacy deny list does not enable unlisted templates");

    rule->mode = ROOM_TEMPLATE_LIST_ALLOW;
    u64b baseline, weight = room_template_total_weight(0, 1, 20, &baseline);
    assert(baseline == 1 && weight == 1000);
    assert(room_template_pick(0, 1, 20, weight) == &templates[0]);
    assert(!room_template_is_eligible(1, 0, 1, 20));
    check_empty(1, 0);
    puts("PASS: only explicit I:+ members of the effective theme can be selected");

    metadata[0].no_room_templates = TRUE;
    check_empty(0, 1);
    metadata[0].no_room_templates = FALSE;
    multipliers[0] = 0;
    weight = room_template_total_weight(0, 1, 20, &baseline);
    assert(weight == 0 && baseline == 1);
    assert(!room_template_pick(0, 1, 20, weight));
    multipliers[0] = 2000;
    assert(room_template_total_weight(0, 1, 20, &baseline) == 2000);
    assert(room_template_is_eligible(0, 0, 1, 10));
    assert(room_template_is_eligible(0, 0, 1, 30));
    assert(!room_template_is_eligible(0, 0, 1, 9));
    assert(!room_template_is_eligible(0, 0, 1, 31));
    templates[0].wid = 100;
    check_empty(0, 1);
    assert(!room_template_is_eligible(2, 0, 1, 20));
    assert(!room_template_is_eligible(0, -1, 1, 20));
    assert(!room_template_is_eligible(0, 0, 3, 20));
    puts("PASS: global prohibition, multipliers, depth, size and index guards preserved");
    return 0;
}
'''
helpers = '\n'.join(function(name) for name in [
    'room_template_size_fits', 'room_template_is_eligible',
    'room_template_multiplier', 'room_template_total_weight',
    'room_template_random', 'room_template_pick'])
with tempfile.TemporaryDirectory(prefix='dungeon-allowlist-') as directory:
    source = Path(directory) / 'check.c'
    executable = Path(directory) / 'check'
    source.write_text(prefix + helpers + tests)
    subprocess.run(['cc', '-std=gnu99', '-D_DEFAULT_SOURCE', '-DMEXP=19937',
                    '-DENABLE_ROOM_TEMPLATES', '-I' + str(ROOT / 'src/server'),
                    '-I' + str(ROOT / 'src/server/lua'), '-Wall', '-Wextra',
                    '-Werror', '-fsanitize=undefined', '-fno-sanitize-recover=all',
                    str(source), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)

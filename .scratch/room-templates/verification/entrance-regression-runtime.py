"""GDB probe of ordinary Orc Cave floors in an isolated TOMENET_PATH.

Invoked after stopping before setup_contact_socket. No live server state.
"""
import gdb
from collections import deque


def value(expression):
    return gdb.parse_and_eval(expression)


def number(expression):
    return int(value(expression))


def call(expression):
    gdb.execute('call ' + expression, to_string=True)


def set_value(expression, datum):
    gdb.execute('set ' + expression + ' = ' + str(datum), to_string=True)


placements = []
level_kinds = set()
centers_seen = set()


class Placement(gdb.Breakpoint):
    def stop(self):
        room = value('room').dereference()
        if room['code'].string() != 'orc_camp':
            return False
        frame = gdb.newest_frame()
        while frame and frame.name() != 'cave_gen':
            frame = frame.older()
        if frame:
            level_kinds.add((int(frame.read_var('empty_level')), int(frame.read_var('cavern'))))
        orientation, top, left = (number(name) for name in ('orientation', 'top', 'left'))
        for index in range(number('dun->cent_n')):
            centers_seen.add((number('dun->cent[%d].y' % index), number('dun->cent[%d].x' % index)))
        width, height = int(room['wid']), int(room['hgt'])
        entrances = []
        for index in range(int(room['entrance_count'])):
            entry = room['entrances'][index]
            x, y = int(entry['x']), int(entry['y'])
            symbol = int(room['terrain_map'][y * width + x])
            base = int(room['terrain_feature'][symbol])
            if orientation & 4:
                x = width - 1 - x
            rotation = orientation & 3
            if rotation == 1:
                x, y = height - 1 - y, x
            elif rotation == 2:
                x, y = width - 1 - x, height - 1 - y
            elif rotation == 3:
                x, y = y, width - 1 - x
            actual = number('zcave[%d][%d].feat' % (top + y, left + x))
            assert actual == base, ('entrance changed during placement', index, actual, base)
            entrances.append((top + y, left + x, base, int(entry['feat'])))
        placements.append(dict(top=top, left=left, orientation=orientation, width=height if orientation & 1 else width, height=width if orientation & 1 else height, entrances=entrances))
        return False


def verify_completed_floor(cave, seed):
    used = unused = 0
    for placement in placements:
        for y, x, base, replacement in placement['entrances']:
            cell = cave[y][x]
            actual = int(cell['feat'])
            assert actual in (base, replacement), ('incorrect entrance terrain', seed, y, x, actual, base, replacement)
            assert not int(cell['info2']) & 0x100, 'temporary template mask leaked'
            used += actual == replacement
            unused += actual == base
            if actual == replacement:
                seen, pending = {(y, x)}, deque([(y, x)])
                outside = False
                while pending:
                    cy, cx = pending.popleft()
                    outside |= not (placement['top'] <= cy < placement['top'] + placement['height'] and
                                    placement['left'] <= cx < placement['left'] + placement['width'])
                    for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                        ny, nx = cy + dy, cx + dx
                        if not (0 < ny < 65 and 0 < nx < 197) or (ny, nx) in seen:
                            continue
                        if number('f_info[%d].flags1' % int(cave[ny][nx]['feat'])) & 0x10:
                            seen.add((ny, nx)); pending.append((ny, nx))
                assert outside, ('used entrance disconnected', seed, y, x)
                assert len(seen) > placement['width'] * placement['height'], ('entrance not connected to floor network', seed, y, x, len(seen))
                assert any(point in seen and not (placement['top'] <= point[0] < placement['top'] + placement['height'] and placement['left'] <= point[1] < placement['left'] + placement['width']) for point in centers_seen), ('used entrance does not reach another room center', seed, y, x)
    assert used > 0, ('no tunnel used any entrance', seed)
    return used, unused


try:
    assert gdb.selected_frame().name() == 'setup_contact_socket'
    assert number('d_info[19].fill_type[0]') == 97
    assert number('f_info[97].flags1') & 0x40  # FF1_PERMANENT
    path = gdb.lookup_symbol('build_room_template')[0].symtab.fullname()
    lines = open(path, encoding='utf8').readlines()
    start = next(i for i, line in enumerate(lines) if line.startswith('bool build_room_template('))
    success = next(i for i in range(start, len(lines)) if 'return(TRUE);' in lines[i]) + 1
    Placement('%s:%d' % (path, success), internal=True)
    target = next(((x, y) for y in range(2, 63) for x in range(2, 63)
                   if not number('wild_info[%d][%d].dungeon' % (y, x))
                   and not number('wild_info[%d][%d].tower' % (y, x))), None)
    assert target
    wx, wy = target
    call('(void*)malloc(sizeof(worldpos))')
    gdb.execute('set $entrance_probe = (worldpos*)$', to_string=True)
    for field, datum in [('wx', wx), ('wy', wy), ('wz', -10)]:
        set_value('$entrance_probe->' + field, datum)
    call('(void)add_dungeon($entrance_probe, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0)')
    call('(void)alloc_dungeon_level($entrance_probe)')
    # Avoid unrelated player-only town paths in the isolated process only.
    field = 'wild_info[%d][%d].dungeon->flags2' % (wy, wx)
    set_value(field, number(field) & ~0x0c000000)
    ordinary, total, total_used, total_unused = 0, 0, 0, 0
    for seed in range(1, 9):
        placements.clear()
        level_kinds.clear()
        centers_seen.clear()
        call('(void)init_gen_rand(%d)' % seed)
        set_value('Rand_quick', 0)
        call('(void)generate_cave($entrance_probe, (player_type*)0)')
        assert placements, ('no Orc camp placements', seed)
        cave = value('getcave($entrance_probe)')
        used, unused = verify_completed_floor(cave, seed)
        if (0, 0) in level_kinds:
            ordinary += 1
        total += len(placements)
        total_used += used
        total_unused += unused
        print('ENTRANCE_RUNTIME_FLOOR_PASS', 'seed', seed, 'empty/cavern', sorted(level_kinds),
              'placements', len(placements), 'used', used, 'unused', unused, flush=True)
        call('(void)wipe_m_list_special($entrance_probe)')
        call('(void)wipe_o_list_special($entrance_probe)')
    assert ordinary >= 7, ('expected at least seven ordinary floors', ordinary)
    assert total_unused > 0, 'expected unused potential entrances to remain intact'
    assert number('d_info[19].fill_type[0]') == 97
    print('ENTRANCE_RUNTIME_PASS', 'ordinary_floors', ordinary, 'placements', total,
          'used', total_used, 'unused', total_unused, flush=True)
    # A separate scenario exercises a floor composed only of templates, including
    # its first room when no ordinary center exists. Fill97 is unchanged.
    set_value('d_info[19].room_templates_always', 1)
    placements.clear(); level_kinds.clear(); centers_seen.clear()
    call('(void)init_gen_rand(9)'); set_value('Rand_quick', 0)
    call('(void)generate_cave($entrance_probe, (player_type*)0)')
    assert len(placements) > 1, ('expected several template-only rooms', len(placements))
    used, unused = verify_completed_floor(value('getcave($entrance_probe)'), 9)
    assert unused > 0
    print('ENTRANCE_RUNTIME_TEMPLATE_ONLY_PASS', 'placements', len(placements), 'used', used, 'unused', unused, flush=True)
    gdb.execute('quit 0')
except Exception as error:
    print('ENTRANCE_RUNTIME_FAIL', repr(error), flush=True)
    gdb.execute('quit 1')

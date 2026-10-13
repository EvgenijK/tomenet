"""GDB-only diagnostic: calls normal generation and inspects completed floors.

Run with isolated TOMENET_PATH and generate.c built with -g -O0.
This script neither opens server sockets nor calls build_room_template directly.
"""
import gdb
from collections import deque


def value(expression):
    return gdb.parse_and_eval(expression)


def number(expression):
    return int(value(expression))


def call(expression):
    gdb.execute("call " + expression, to_string=True)


placements = []
fixture = None


class Placement(gdb.Breakpoint):
    def stop(self):
        if value("room->code").string() == fixture["code"]:
            placements.append({key: number(key) for key in ("top", "left", "orientation")})
        return False


def transformed(x, y, orientation):
    # Independent implementation of the eight documented orientation operations.
    width, height = fixture["width"], fixture["height"]
    if orientation >= 4:
        x = width - 1 - x
    for unused in range(orientation % 4):
        x, y = height - 1 - y, x
        width, height = height, width
    return x, y


def verify(cave, placement, height, width):
    top, left = placement["top"], placement["left"]
    orientation = placement["orientation"]
    template = value("room_templates[0]")
    for sy in range(fixture["height"]):
        for sx in range(fixture["width"]):
            offset = sy * fixture["width"] + sx
            terrain = int(template["terrain_map"][offset])
            if terrain == ord("_"):
                continue
            dx, dy = transformed(sx, sy, orientation)
            cell = cave[top + dy][left + dx]
            expected = int(template["terrain_feature"][terrain])
            assert int(cell["feat"]) == expected, ("terrain", sx, sy, int(cell["feat"]), expected)
            assert not int(cell["info2"]) & 0x100, "temporary template mask leaked"
            assert not int(cell["info"]) & 0x4000, "temporary monster block leaked"
            monster = int(template["monster_map"][offset])
            if monster != ord("_"):
                monster_index = int(cell["m_idx"])
                expected_race = int(template["monster_profiles"][monster]["r_idx"])
                actual_race = number("m_list[%d].r_idx" % monster_index) if monster_index else 0
                assert actual_race == expected_race, ("primary M cell/race", sx, sy, orientation, monster_index, actual_race, expected_race)
            item = int(template["item_map"][offset])
            if item != ord("_"):
                profile = template["item_profiles"][item]
                expected_kind = number("lookup_kind(%d, %d)" % (int(profile["tval"]), int(profile["sval"])))
                object_index = int(cell["o_idx"])
                item_kinds = []
                while object_index:
                    item_kinds.append(number("o_list[%d].k_idx" % object_index))
                    object_index = number("o_list[%d].next_o_idx" % object_index)
                assert expected_kind in item_kinds, ("primary I cell/kind", item_kinds, expected_kind)
    entrance_coord = template["entrances"][0]
    ex, ey = transformed(int(entrance_coord["x"]), int(entrance_coord["y"]), orientation)
    entrance = top + ey, left + ex
    seen, todo = {entrance}, deque([entrance])
    outside = False
    while todo:
        y, x = todo.popleft()
        footprint_width = fixture["height"] if orientation & 1 else fixture["width"]
        footprint_height = fixture["width"] if orientation & 1 else fixture["height"]
        if not (top <= y < top + footprint_height and left <= x < left + footprint_width):
            outside = True
        for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            yy, xx = y + dy, x + dx
            if not (0 <= yy < height and 0 <= xx < width) or (yy, xx) in seen:
                continue
            feat = int(cave[yy][xx]["feat"])
            if number("f_info[%d].flags1" % feat) & 0x10:  # FF1_FLOOR
                seen.add((yy, xx))
                todo.append((yy, xx))
    assert outside, ("entrance does not connect to external level", entrance, len(seen))
    return len(seen)


try:
    assert gdb.selected_frame().name() == "setup_contact_socket", "server not initialized before network"
    assert number("room_template_count") == 1, "isolated fixture must be the only template"
    fixture = {"code": value("room_templates[0].code").string(), "width": number("room_templates[0].wid"), "height": number("room_templates[0].hgt")}
    assert number("room_templates[0].entrance_count"), "fixture must exercise entrance"
    assert any(int(value("room_templates[0].monster_map[%d]" % i)) != ord("_") for i in range(fixture["width"] * fixture["height"])), "fixture must exercise M"
    assert any(int(value("room_templates[0].item_map[%d]" % i)) != ord("_") for i in range(fixture["width"] * fixture["height"])), "fixture must exercise I"
    # Observe the successful return line without changing selection frequency/RNG.
    path = gdb.lookup_symbol("build_room_template")[0].symtab.fullname()
    lines = open(path, encoding="utf8").readlines()
    start = next(i for i, line in enumerate(lines) if line.startswith("bool build_room_template("))
    success = next(i for i in range(start, len(lines)) if "return(TRUE);" in lines[i]) + 1
    observer = Placement("%s:%d" % (path, success), internal=True)
    # Add a standard Angband through the normal dungeon API at a free sector.
    # Wilderness placement sometimes omits Angband; no generation flags change.
    target = None
    for wy in range(64):
        for wx in range(64):
            dungeon = value("wild_info[%d][%d].dungeon" % (wy, wx))
            tower = value("wild_info[%d][%d].tower" % (wy, wx))
            if not int(dungeon) and not int(tower):
                target = wx, wy
                break
        if target:
            break
    assert target, "no free sector in isolated wilderness"
    call("(void*)malloc(sizeof(worldpos))")
    gdb.execute("set $smoke_pos = (worldpos*)$", to_string=True)
    wx, wy = target
    for field, datum in (("wx", wx), ("wy", wy), ("wz", -9)):
        gdb.execute("set $smoke_pos->%s = %d" % (field, datum), to_string=True)
    call("(void)add_dungeon($smoke_pos, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0)")
    call("(void)alloc_dungeon_level($smoke_pos)")
    print("SMOKE_POSITION", wx, wy, -9, "depth", number("getlevel($smoke_pos)"))
    passed = 0
    for seed in range(1, 21):
        placements.clear()
        # Rand_state_init reads urandom on the SFMT build. Seed SFMT directly
        # for a reproducible ordinary RNG stream without changing its algorithm.
        call("(void)init_gen_rand(%d)" % seed)
        gdb.execute("set Rand_quick = 0", to_string=True)
        call("(void)generate_cave($smoke_pos, (player_type*)0)")
        cave = value("getcave($smoke_pos)")
        level = value("wild_info[%d][%d].dungeon->level[8]" % (wy, wx))
        height, width = int(level["hgt"]), int(level["wid"])
        # Check cleanup across the completed floor, including external cells.
        for y in range(height):
            for x in range(width):
                assert not int(cave[y][x]["info2"]) & 0x100, "floor template mask leaked"
        for placement in placements:
            connected = verify(cave, placement, height, width)
            print("SMOKE_PLACEMENT_OK", "seed", seed, placement, "connected_cells", connected)
            passed += 1
        print("SMOKE_FLOOR", seed, height, width, "placements", len(placements))
        if passed:
            break
        call("(void)wipe_m_list_special($smoke_pos)")
        call("(void)wipe_o_list_special($smoke_pos)")
    assert passed, "no successful template in 20 bounded normal floors"
    print("GENERATION_SMOKE_PASS", "complete_floors", seed, "placements", passed)
    gdb.execute("source .scratch/room-templates/verification/required-ego-smoke.py")
    gdb.execute("quit 0")
except Exception as error:
    print("GENERATION_SMOKE_FAIL", repr(error))
    gdb.execute("quit 1")

"""Observe ordinary Training Tower floors with the real three sample files.

Isolated TOMENET_PATH; diagnostic -O0 generate/main/go objects. No sockets,
forced category, direct room-builder calls, production test API, or live state.
"""
import gdb
from collections import deque


def value(expression):
    return gdb.parse_and_eval(expression)


def number(expression):
    return int(value(expression))


def call(expression):
    gdb.execute("call " + expression, to_string=True)


fixtures = {}
placements = []


class Placement(gdb.Breakpoint):
    def stop(self):
        code = value("room->code").string()
        if code in fixtures:
            placements.append(dict(code=code, **{
                key: number(key) for key in ("top", "left", "orientation")
            }))
        return False


def transformed(fixture, x, y, orientation):
    width, height = fixture["width"], fixture["height"]
    if orientation >= 4:
        x = width - 1 - x
    for unused in range(orientation % 4):
        x, y = height - 1 - y, x
        width, height = height, width
    return x, y


def verify(cave, placement, height, width):
    fixture = fixtures[placement["code"]]
    template = value("room_templates[%d]" % fixture["index"])
    top, left, orientation = (placement[k] for k in ("top", "left", "orientation"))
    monsters = items = webs = stairs = 0
    for sy in range(fixture["height"]):
        for sx in range(fixture["width"]):
            offset = sy * fixture["width"] + sx
            terrain = int(template["terrain_map"][offset])
            if terrain == ord("_"):
                continue
            dx, dy = transformed(fixture, sx, sy, orientation)
            cell = cave[top + dy][left + dx]
            expected = int(template["terrain_feature"][terrain])
            monster = int(template["monster_map"][offset])
            item = int(template["item_map"][offset])
            actual_feat = int(cell["feat"])
            # Canonical Q14 permits ordinary stairs on marker-empty floor.
            ordinary_stair = (expected == 1 and actual_feat in (6, 7)
                              and monster == ord("_") and item == ord("_"))
            assert actual_feat == expected or ordinary_stair, ("terrain", placement, sx, sy, actual_feat, expected)
            stairs += ordinary_stair
            webs += expected == 16
            assert not int(cell["info2"]) & 0x100, "temporary template mask leaked"
            assert not int(cell["info"]) & 0x4000, "temporary monster block leaked"
            if monster != ord("_"):
                monster_index = int(cell["m_idx"])
                expected_race = int(template["monster_profiles"][monster]["r_idx"])
                actual = number("m_list[%d].r_idx" % monster_index) if monster_index else 0
                assert actual == expected_race, ("monster", placement, sx, sy, actual, expected_race)
                monsters += 1
            if item != ord("_"):
                profile = template["item_profiles"][item]
                expected_kind = number("lookup_kind(%d, %d)" % (int(profile["tval"]), int(profile["sval"])))
                index = int(cell["o_idx"])
                kinds = []
                while index:
                    kinds.append(number("o_list[%d].k_idx" % index))
                    index = number("o_list[%d].next_o_idx" % index)
                assert expected_kind in kinds, ("item", placement, sx, sy, kinds, expected_kind)
                items += 1
    assert int(template["entrance_count"]) == 1
    entrance_coord = template["entrances"][0]
    ex, ey = transformed(fixture, int(entrance_coord["x"]), int(entrance_coord["y"]), orientation)
    entrance = top + ey, left + ex
    seen, todo = {entrance}, deque([entrance])
    outside = False
    footprint_width = fixture["height"] if orientation & 1 else fixture["width"]
    footprint_height = fixture["width"] if orientation & 1 else fixture["height"]
    while todo:
        y, x = todo.popleft()
        outside |= not (top <= y < top + footprint_height and left <= x < left + footprint_width)
        for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            yy, xx = y + dy, x + dx
            if not (0 <= yy < height and 0 <= xx < width) or (yy, xx) in seen:
                continue
            if number("f_info[%d].flags1" % int(cave[yy][xx]["feat"])) & 0x10:
                seen.add((yy, xx))
                todo.append((yy, xx))
    assert outside, ("entrance disconnected", placement, entrance)
    expected_counts = {"spiders_lair": (3, 0, 4), "bats_nest": (5, 0, 0), "treasure_room": (0, 5, 0)}
    assert (monsters, items, webs) == expected_counts[fixture["code"]]
    return monsters, items, webs, stairs, len(seen)


try:
    assert gdb.selected_frame().name() == "setup_contact_socket"
    assert number("room_template_count") == 3
    for index in range(3):
        code = value("room_templates[%d].code" % index).string()
        fixtures[code] = dict(index=index, code=code,
                             width=number("room_templates[%d].wid" % index),
                             height=number("room_templates[%d].hgt" % index))
    assert set(fixtures) == {"spiders_lair", "bats_nest", "treasure_room"}
    assert number("d_info[30].room_templates_always") == 1
    path = gdb.lookup_symbol("build_room_template")[0].symtab.fullname()
    lines = open(path, encoding="utf8").readlines()
    start = next(i for i, line in enumerate(lines) if line.startswith("bool build_room_template("))
    success = next(i for i in range(start, len(lines)) if "return(TRUE);" in lines[i]) + 1
    observer = Placement("%s:%d" % (path, success), internal=True)
    target = None
    # Tower at (0,0) is the PvP arena and rejects monsters. Use an ordinary
    # sector, avoiding the origin and the special IDDC sector (63,63).
    for wy in range(2, 63):
        for wx in range(2, 63):
            if not number("wild_info[%d][%d].dungeon" % (wy, wx)) and not number("wild_info[%d][%d].tower" % (wy, wx)):
                target = wx, wy
                break
        if target:
            break
    assert target
    call("(void*)malloc(sizeof(worldpos))")
    gdb.execute("set $smoke_pos = (worldpos*)$", to_string=True)
    wx, wy = target
    for field, datum in (("wx", wx), ("wy", wy), ("wz", 1)):
        gdb.execute("set $smoke_pos->%s = %d" % (field, datum), to_string=True)
    call("(void)add_dungeon($smoke_pos, 0, 0, 0, 0, 0, 0, 30, 0, 0, 0)")
    print("TRAINING_POSITION", wx, wy)
    for floor, eligible in ((1, {"bats_nest", "treasure_room"}), (2, set(fixtures))):
        gdb.execute("set $smoke_pos->wz = %d" % floor, to_string=True)
        call("(void)alloc_dungeon_level($smoke_pos)")
        assert number("getlevel($smoke_pos)") == floor
        seen_codes = set()
        total = 0
        for seed in range(1, 11):
            placements.clear()
            call("(void)init_gen_rand(%d)" % seed)
            gdb.execute("set Rand_quick = 0", to_string=True)
            call("(void)generate_cave($smoke_pos, (player_type*)0)")
            cave = value("getcave($smoke_pos)")
            level = value("wild_info[%d][%d].tower->level[%d]" % (wy, wx, floor - 1))
            height, width = int(level["hgt"]), int(level["wid"])
            for y in range(height):
                for x in range(width):
                    assert not int(cave[y][x]["info2"]) & 0x100, "floor template mask leaked"
            for placement in placements:
                assert placement["code"] in eligible, ("ineligible template", floor, placement)
                counts = verify(cave, placement, height, width)
                print("TRAINING_PLACEMENT_OK", "floor", floor, "seed", seed, placement, "monsters/items/webs/stairs/connected", counts)
                seen_codes.add(placement["code"])
                total += 1
            print("TRAINING_FLOOR_OK", floor, "seed", seed, height, width, "placements", len(placements))
            if seen_codes == eligible:
                break
            call("(void)wipe_m_list_special($smoke_pos)")
            call("(void)wipe_o_list_special($smoke_pos)")
        assert seen_codes == eligible, ("missing sample after 10 seeds", floor, seen_codes, eligible)
        print("TRAINING_DEPTH_PASS", floor, "seeds", seed, "placements", total, "codes", sorted(seen_codes))
    print("TRAINING_TOWER_SMOKE_PASS")
    gdb.execute("quit 0")
except Exception as error:
    print("TRAINING_TOWER_SMOKE_FAIL", repr(error))
    gdb.execute("quit 1")

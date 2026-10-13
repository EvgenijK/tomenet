"""Run after generation-smoke.py while the isolated inferior remains paused."""
import gdb


def ego_value(expression):
    return gdb.parse_and_eval(expression)


def ego_number(expression):
    return int(ego_value(expression))


def ego_call(expression):
    gdb.execute("call " + expression, to_string=True)


def prepare_dagger(seed):
    ego_call("(void)init_gen_rand(%d)" % seed)
    ego_call("(void)invwipe($ego_forge)")
    ego_call("(void)invcopy($ego_forge, lookup_kind(23,4))")


try:
    ego_call("(void*)malloc(sizeof(object_type))")
    gdb.execute("set $ego_forge = (object_type*)$", to_string=True)
    # A compatible normal dagger, not a broken dagger. Disable artifacts only
    # for this comparison so the observed distinction is definitely ego power.
    for good in (0, 1):
        evidence = None
        for seed in range(1, 41):
            prepare_dagger(seed)
            ego_call("(void)apply_magic($smoke_pos,$ego_forge,75,1,%d,0,0,6)" % good)
            baseline_plain = ego_number("$ego_forge->k_idx") and not any(ego_number("$ego_forge->%s" % field) for field in ("name1", "name2", "name2b"))
            if not baseline_plain:
                continue
            prepare_dagger(seed)
            produced = ego_number("apply_magic_required_ego($smoke_pos,$ego_forge,75,%d,0,6)" % good)
            if not produced:
                # Ordinary bounded ego selection can genuinely fail. The object
                # must then be wiped; search the bounded seeds for a successful
                # otherwise-plain comparison rather than require every seed.
                assert not ego_number("$ego_forge->k_idx"), "failed required selection retained plain item"
                continue
            assert not ego_number("$ego_forge->name1"), "artifact-only control failed"
            assert ego_number("$ego_forge->name2") or ego_number("$ego_forge->name2b"), "required result lacks ego"
            evidence = (seed, ego_number("$ego_forge->name2"), ego_number("$ego_forge->name2b"))
            break
        assert evidence, ("no plain baseline found in bounded comparison", good)
        print("REQUIRED_EGO_QUALITY_PASS", "good" if good else "normal", "seed_ego_pair", evidence)
    # Exact Morgul's dynamic restriction is shared with the ordinary selector.
    prepare_dagger(1)
    ordinary = ego_number("room_template_item_ego_available($smoke_pos,$ego_forge,103,6)")
    assert ordinary, "Morgul unexpectedly prohibited on ordinary floor"
    ego_call("(void*)malloc(sizeof(worldpos))")
    gdb.execute("set $ego_iddc = (worldpos*)$", to_string=True)
    for field, datum in (("wx", 63), ("wy", 63), ("wz", -50)):
        gdb.execute("set $ego_iddc->%s = %d" % (field, datum), to_string=True)
    assert not ego_number("room_template_item_ego_available($ego_iddc,$ego_forge,103,6)"), "exact Morgul escaped IDDC restriction"
    print("EXACT_MORGUL_DYNAMIC_PASS", "ordinary_allowed", ordinary, "iddc_denied", 1)
    print("REQUIRED_EGO_SMOKE_PASS")
except Exception as error:
    print("REQUIRED_EGO_SMOKE_FAIL", repr(error))
    gdb.execute("quit 1")

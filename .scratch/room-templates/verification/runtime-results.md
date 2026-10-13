# Runtime acceptance result, 2026-10-13

Integration source: `ed0e5de90ce572a626087810198f30195b938c64`, containing the
eight room-template tickets and the two final item-ego fixes.

Normal full server build passed before debugger preparation. The existing
linker warning about `tmpnam` in `server/util.c` is unrelated to these changes.
After debugger verification, a forced full optimized rebuild also passed:
`make -C src -B -j4 tomenet.server`. The `generate`, `main`, and `go` diagnostic
objects are therefore restored to standard `-g -O2` compilation. The additional
`tmpnam` warning from the legacy Lua library is likewise pre-existing.

The final debugger verification passed with the real Chromium-exported
`browser_resume.txt`: M `r_idx:29`, I `tval:80/sval:1`, entrance `(0,1)`, a 4×4
footprint including transparent cells. It called ordinary `generate_cave()` for
standard Angband at depth 75, with no direct call to `build_room_template()` and
no forced template selection. SFMT seed 1 produced a completed 66×198 floor:

```text
SMOKE_PLACEMENT_OK seed 1 {'top': 58, 'left': 124, 'orientation': 5} connected_cells 558
SMOKE_PLACEMENT_OK seed 1 {'top': 3, 'left': 14, 'orientation': 4} connected_cells 196
SMOKE_FLOOR 1 66 198 placements 2
GENERATION_SMOKE_PASS complete_floors 1 placements 2
REQUIRED_EGO_QUALITY_PASS normal seed_ego_pair (1, 66, 0)
REQUIRED_EGO_QUALITY_PASS good seed_ego_pair (1, 70, 67)
EXACT_MORGUL_DYNAMIC_PASS ordinary_allowed 1 iddc_denied 1
REQUIRED_EGO_SMOKE_PASS
```

Each observed placement preserved every prescribed terrain feature and put the
expected monster race/item kind at the transformed primary marker cells. Each
transformed entrance reached external floor cells. The completed floor contained
no temporary room-template mask; active fixture cells retained no temporary
monster-block mask.

The required-ego controls produced real egos from the same seeded outcomes where
ordinary `normal`/`good` item generation produced a plain dagger. The Morgul
control checked the shared dynamic availability helper at ordinary and IDDC
coordinates. It did not generate a separate IDDC gameplay floor.

Scope: these are bounded acceptance/regression checks, not exhaustive probability
or all-orientation tests. The sibling README contains the reproducible commands
and the durable GDB/Python/fixture files; no live server or original game state
was used.

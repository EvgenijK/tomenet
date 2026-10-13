# Training Tower sample acceptance, 2026-10-13

Implementation source: `1c03f8035b0cec9174abc3eb3387f84688190a7e`.
Normal server build passed before diagnostic preparation. GDB tested the real
three sample TXT files and real `d_info.txt` in a private copied `lib`, stopping
at `setup_contact_socket()` before the first listening socket. It added dungeon
type 30 normally at `(2,2)`, allocated positive tower floors 1 and 2, and called
ordinary `generate_cave()` with SFMT seed 1 at each depth. No room-builder calls,
forced category, changed runtime frequency, or live server were involved.

Both completed floors were 66×198 and contained 28 successful sample placements:

| Depth | Bats Nest | Spiders Lair | Treasure Room |
| --- | ---: | ---: | ---: |
| 1 | 18 | 0 | 10 |
| 2 | 14 | 9 | 5 |

Every placement passed active-terrain checks, expected primary monsters/items,
entrance connectivity to external walkable floor, and temporary-mask cleanup.
Bats had five Fruit bats (`r_idx=37`); spiders had three Cave spiders
(`r_idx=60`) and four webs (`feat=16`); treasure had all five prescribed exact
item kinds. Spider templates were excluded on depth 1. Per canonical Q14,
ordinary stairs are permitted on floor cells with no primary M/I marker; the
test accepts only expected floor 1 becoming staircase 6/7 on those empty cells.
Walls, webs, and primary marker terrain are checked strictly.

```text
TRAINING_POSITION 2 2
TRAINING_FLOOR_OK 1 seed 1 66 198 placements 28
TRAINING_DEPTH_PASS 1 seeds 1 placements 28 codes ['bats_nest', 'treasure_room']
TRAINING_FLOOR_OK 2 seed 1 66 198 placements 28
TRAINING_DEPTH_PASS 2 seeds 1 placements 28 codes ['bats_nest', 'spiders_lair', 'treasure_room']
TRAINING_TOWER_SMOKE_PASS
```

Harness corrections during preparation: the initial strict terrain assertion
rejected the above permitted staircase substitution; the original free-sector
search then selected `(0,0)`, where any positive tower is the special PvP arena
and monster creation is forbidden. The final harness uses ordinary `(2,2)` and
enforces the documented empty-floor-only staircase exception. Neither required
a production change. Full temporary log: `/tmp/tomenet-training-tower-smoke.log`.

The sibling `training-tower-smoke-README.md` contains reproduction steps.
These are concrete bounded-generation checks, not an exhaustive guarantee of
sample counts or every seed. Final optimized build is performed separately in
the user's managed worktree, replacing diagnostic objects there.

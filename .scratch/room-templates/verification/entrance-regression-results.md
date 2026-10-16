# Deferred entrance regression

Date: 2026-10-16. Covers entrance activation during the common tunnel phase;
placement itself does not require an existing center or a connection.

## Isolated production-function regression

From the repository root:

```sh
python3 .scratch/room-templates/verification/entrance-regression.py
```

Final result:

```text
PARSER_PASS 22 cases (explicit wall, legacy, duplicates, conflicts, malformed)
PLACEMENT_PASS no existing centers, impassable surrounds, no entrance replacement, all 8 transforms, zero optional entrances, unresolvable replacement does not block placement
TUNNEL_PASS actual build_tunnel over permanent fill97, all 8 transforms, exact wall/lava/door/floor, unused unchanged, resolution fallback, failed tunnel keeps entrances, arbitrary room wall/nonfill blocked, template-to-template, wide sidewall protection
ENTRANCE_REGRESSION_PASS
```

The Python driver extracts the current production parser, dungeon/placement
structures, transforms, block reservation, complete placement function, actual
`build_tunnel()`, shared tunnel eligibility predicates and template connection
helpers. It compiles these with real server headers, warnings as errors and the
undefined-behavior sanitizer. `WIDE_CORRIDORS` is enabled by the real configuration.
It stubs file wrappers, feature resolution/setters, deterministic RNG, random
bending and unrelated monster/item/door placement. Feature metadata are controlled
fixtures. The temporary executable and parser fixtures are removed after the run.

Coverage includes explicit entrances on wall/lava, arbitrary allowed replacement
including wall, legacy floor compatibility, duplicate/conflicting declarations
in both orders, malformed coordinates/terrain, and permanent/context-dependent
terrain rejection. Placement leaves all entrances at their base terrain, even
with no centers or impassable surroundings. Zero-entrance templates place and do
not register a connection center.

Connection tests use the actual tunnel builder across dungeon fill 97 flagged
`FF1_PERMANENT`, in both source and destination roles across eight transforms.
They check exact replacement terrain, untouched unused candidates, resolution
fallback, failed tunnels retaining all candidates and template-to-template
connections. Negative cases reject permanent non-fill endpoints and arbitrary
walls marked `CAVE_ROOM`; positive room-marked dungeon fill is actually carved.
Wide corridor tests protect both fill-side cells and outer-wall-side cells.
Existing vault floor (`CAVE_ICKY`) remains a valid traversable target, while
vault-marked non-floor dungeon fill is rejected for carving.

This isolated test does not execute all later floor generation passes; the real
server check below covers completed floors and actual feature metadata.

## Real server generation

Build the normal server, then create a private diagnostic executable and library:

```sh
make -C src tomenet.server
python3 .scratch/room-templates/verification/entrance-regression-runtime-build.py
```

The build script prints a fresh `/tmp/tomenet-entrance-tunnel-runtime-*` path. It
copies `lib`, compiles `generate`, `main` and `go` with `-O0` into that directory,
and relinks with the other normal objects without replacing repository objects.
Use the printed path below:

```sh
TOMENET_PATH=/tmp/PRINTED_DIRECTORY/lib timeout 55s gdb -q -batch \
  -x .scratch/room-templates/verification/entrance-regression-runtime.gdb \
  /tmp/PRINTED_DIRECTORY/tomenet.server
```

The probe stops before `setup_contact_socket`, adds Orc Cave (type 19), depth 10,
in a regular empty wilderness sector and generates seed 1–8. The dungeon's fill
remains terrain 97, including its real PERMANENT flag. Only the unrelated
player-dependent town flags `DF2_TOWNS_RND|DF2_TOWNS_FIX` are cleared in the isolated
process. Each observed successful placement must still contain all original
entrance tiles. After full generation every entrance must equal its original tile
or its declared replacement; used entrances must flood-fill through walkable
terrain outside their footprint and reach another room center. Temporary template
masks must be cleared. At least one unused entrance must remain intact.

Final run after tunnel eligibility, wide-sidewall and vault-floor predicate fixes:

| Seed | Floor | Templates | Used entrances | Unused unchanged |
| --- | --- | ---: | ---: | ---: |
| 1 | Ordinary | 2 | 2 | 6 |
| 2 | Ordinary | 3 | 3 | 9 |
| 3 | Ordinary | 4 | 4 | 12 |
| 4 | Ordinary | 2 | 2 | 6 |
| 5 | Ordinary | 2 | 2 | 6 |
| 6 | Open area | 3 | 3 | 9 |
| 7 | Ordinary | 4 | 4 | 12 |
| 8 | Ordinary | 2 | 2 | 6 |

```text
ENTRANCE_RUNTIME_PASS ordinary_floors 7 placements 22 used 22 unused 66
ENTRANCE_RUNTIME_TEMPLATE_ONLY_PASS placements 45 used 45 unused 135
```

The separate final scenario enables `ROOM_TEMPLATES_ALWAYS` for Orc Cave only in
the private process and generates seed 9. It verifies placement before any normal
room exists and completed connections between templates: 45 rooms, 45 used
entrances, 135 untouched candidates, with the same walkable connectivity checks.

Final diagnostic artifacts and complete log are retained under
`/tmp/tomenet-entrance-tunnel-runtime/`. No running server, live saves or repository
configuration were modified by these tests. This is bounded coverage of concrete
floors rather than an exhaustive assertion about every RNG seed or dungeon.

# Training Tower sample runtime check

The sibling GDB/Python harness loads the actual `lib/game/d_info.txt` and all
three actual sample TXT files from a private copy of `lib`. It stops before the
first server socket, adds ordinary dungeon type 30 through `add_dungeon()`, and
calls `generate_cave()` at tower coordinates `wz=1` and `wz=2`. It does not
modify flags, force a category, or call `build_room_template()` directly.

Build normally first. Diagnostic compilation and a fresh isolated run:

```bash
for task_source in generate main go; do
  clang -pipe -Wall -DUSE_GCU -D_XOPEN_SOURCE -D_BSD_SOURCE -DMEXP=19937 \
    -std=c99 -D_DEFAULT_SOURCE -DACC32 -fPIE -fsigned-char -DLINUX \
    -Isrc/server -Isrc/server/lua -g -O0 \
    -c "src/server/$task_source.c" -o "src/server/$task_source.o"
done
make -C src tomenet.server
task_lib=$(mktemp -d /tmp/tomenet-training-runtime.XXXXXX)
cp -a lib "$task_lib/lib"
TOMENET_PATH="$task_lib/lib" timeout 180s gdb -q -batch \
  -x .scratch/room-templates/verification/training-tower-smoke.gdb \
  ./src/tomenet.server
```

If ptrace is sandboxed, request scoped approval for the final GDB command.
The harness verifies the real `ROOM_TEMPLATES_ALWAYS` setting was parsed and
observes successful template placements through a breakpoint. Up to ten SFMT
seeds per depth are allowed; the first seeds covering all eligible samples end
the depth check. Every observed placement must preserve its active terrain
(canonical Q14 permits ordinary up/down stairs on marker-empty floor cells),
match its exact primary monster races and item kinds, connect its entrance to
external walkable floor, and leave no temporary masks. The spider room must
retain four web tiles and three cave spiders. Bats require five fruit bats;
treasure requires five objects. Spider placement is forbidden on depth 1.

Success ends with `TRAINING_TOWER_SMOKE_PASS`. Restore optimized compilation
after diagnosis by touching the three C sources and rebuilding normally, or by
doing a forced full optimized rebuild. These bounded checks prove concrete
placements on normal completed floors, not exhaustive RNG distributions.

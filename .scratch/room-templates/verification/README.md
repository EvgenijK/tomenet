# Room-template acceptance checks

The generation smoke observes **normal completed floor generation**, rather than
calling the room builder directly or merely checking startup parsing. It uses a
fresh private copy of `lib`, stops the server before it opens its first socket,
and calls the existing `add_dungeon()` and `generate_cave()` APIs through GDB.
The original checkout's game state and any live server are untouched.

## Runtime generation

Prerequisites: server build dependencies and GDB with Python support. Run from
the checkout root. First build the normal server with `make -C src -j4
tomenet.server`. For debugger locals and safe skipping of external IP lookup and
the optional Go engine, rebuild three diagnostic objects without optimization:

```bash
for task_source in generate main go; do
  clang -pipe -Wall -DUSE_GCU -D_XOPEN_SOURCE -D_BSD_SOURCE -DMEXP=19937 \
    -std=c99 -D_DEFAULT_SOURCE -DACC32 -fPIE -fsigned-char -DLINUX \
    -Isrc/server -Isrc/server/lua -g -O0 \
    -c "src/server/$task_source.c" -o "src/server/$task_source.o"
done
make -C src tomenet.server
task_lib=$(mktemp -d /tmp/tomenet-template-generation.XXXXXX)
cp -a lib "$task_lib/lib"
if test -d "$task_lib/lib/game/room_templates"; then
  mv "$task_lib/lib/game/room_templates" "$task_lib/original_room_templates"
fi
mkdir "$task_lib/lib/game/room_templates"
cp .scratch/room-templates/verification/runtime_smoke.txt \
  "$task_lib/lib/game/room_templates/runtime_smoke.txt"
TOMENET_PATH="$task_lib/lib" gdb -q -batch \
  -x .scratch/room-templates/verification/generation-smoke.gdb \
  ./src/tomenet.server
```

Where ptrace is sandboxed, run the final GDB command with the approved scoped
escalation. The smoke quits the isolated inferior and exits nonzero on failures.
To use another fixture, place only that one TXT in the isolated
`room_templates` directory. Its name must match its `N` code. The fixture must
contain an entrance, exact monster race, and exact item `tval`/`sval` compatible
with an ordinary depth-75 Angband floor. `browser_resume.txt` is the real editor
TXT export used by the browser acceptance check.

The script seeds the generator independently for at most 20 floors, retaining
the ordinary template selection frequency and dungeon restrictions. It observes
successful builder returns, then checks after `generate_cave()` returns:

- All active terrain cells after the observed rotation/reflection.
- Exact primary monster race and item kind at their transformed marker cells.
- A floor-only flood fill from the transformed entrance into the external floor.
- No `CAVE2_ROOM_TEMPLATE` bit anywhere in the completed floor, and no temporary
  `CAVE_NO_MONSTER` bit on active fixture cells.

Global startup wilderness state can vary, so dimensions/coordinates/orientation
need not match the example result. A valid run ends with `GENERATION_SMOKE_PASS`.
The SFMT build is seeded through `init_gen_rand()` because `Rand_state_init()`
otherwise reads `/dev/urandom` and ignores its numeric argument.
The smoke does not prove exhaustive random selection distribution, all entity
variants, or all eight orientations in one run.

The same paused inferior then runs `required-ego-smoke.py`: for `normal` and
`good` qualities it compares ordinary plain-item outcomes with required-ego
outcomes on the same seeded dagger. Artifacts are disabled only in this focused
control. Bounded ordinary ego-selection failure is accepted only if the object
is wiped. Successful required outcomes must actually have an ego. The exact-ego
restriction control confirms that the common Morgul availability helper permits
an ordinary Angband coordinate and denies IDDC `(63,63,-50)`; this is a direct
helper control, not a claim that an IDDC gameplay floor was generated.

After diagnosis, restore standard object compilation without deleting sources:
`touch src/server/generate.c src/server/main.c src/server/go.c` and run
`make -C src -j4 tomenet.server`.

## Browser acceptance

The sibling `browser-smoke.cjs` drives the actual local editor in Chromium;
run `node .scratch/room-templates/verification/browser-smoke.cjs` with `playwright`
available, or set `ROOM_TEMPLATE_PLAYWRIGHT` to its module path. Its setup and
results are recorded by the coordinating agent. The exported
`browser_resume.txt` must remain a real UI export, not a hand-written substitute.

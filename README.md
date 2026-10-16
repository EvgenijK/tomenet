# TomeNET Repository Instructions for AI

This repository contains TomeNET, an online multiplayer roguelike role-playing
game derived from MAngband (Multiplayer Angband). It is a long-lived C codebase
with an authoritative game server, separate clients and support tools,
data-driven game definitions, and embedded Lua scripting.

Use this document as the primary repository-level instruction set when an AI
agent analyzes or changes this project.

## Core Working Rules

1. Inspect the relevant implementation, data format comments, and working
   examples before making a change.
2. Treat the current repository as the source of truth. Do not assume that
   upstream TomeNET documentation or another TomeNET version matches this tree.
3. Preserve unrelated user changes. The worktree may already be dirty.
4. Keep changes narrowly scoped and follow existing code and data-file patterns.
5. Do not replace structured parsing with ad hoc text processing when an
   existing parser or structured representation is available.
6. Verify identifiers, field layouts, platform assumptions, and generated-file
   relationships instead of guessing.
7. Clearly separate checks performed from checks that still require a running
   server or client.
8. Do not claim that a change is complete solely because it compiles or a data
   file parses.

## Operational Safety

Do not build, install, launch, stop, restart, or otherwise control the TomeNET
server or client unless the user explicitly requests that action.

In particular, do not run `make`, `make install`, server launcher scripts, or
the `tomenet.server` executable as an automatic validation step. Some build
targets generate C and package files, and runtime actions can modify persistent
state under `lib/save/` and `lib/data/`.

When the user explicitly requests a build, inspect the selected makefile first
and report any generated or modified tracked files. When the user explicitly
requests runtime validation, preserve existing server state and describe the
exact process that will be started or controlled.

## Architecture

TomeNET follows a client-server architecture. The server owns authoritative
game state and gameplay decisions. Clients handle presentation, input, local
preferences, sound, and communication with the server. Shared C code provides
networking, serialization, platform abstractions, utilities, and common game
definitions.

### Source Tree

- `src/server/` contains the authoritative server: initialization, networking,
  world and dungeon generation, players, monsters, objects, combat, spells,
  skills, stores, parties, quests, persistence, scheduling, and administration.
- `src/client/` contains the interactive client, terminal abstraction, input,
  inventory and store interfaces, client networking, sound, and platform
  frontends such as X11, GCU/ncurses, and Windows.
- `src/common/` contains code shared across executables, including sockets,
  packet buffers, file handling, random number generation, common tables,
  hashing, memory helpers, and core types and constants.
- `src/console/` contains the remote console client.
- `src/world/` contains the separate `worldd` service for coordinating servers,
  authentication, accounts, player information, and locks.
- `src/account/` contains the `accedit` account maintenance utility.
- `src/preproc/` and `src/server/lua/` contain the custom preprocessing and
  bundled Lua/tolua toolchain used to generate C bindings.
- `src/updater/` contains the standalone updater.

### Runtime Data

- `lib/game/` contains parser-sensitive game definitions for monsters, objects,
  artifacts, ego items, terrain, stores, towns, dungeons, quests, vaults, and
  overworld data. These files are executable configuration, not prose.
- `lib/scpt/` contains Lua scripts for spells, skills, classes, races, traits,
  powers, quests, modules, and other gameplay behavior.
- `lib/config/` contains server configuration and policy files, including
  `tomenet.cfg`.
- `lib/save/` contains persistent server, account, player, and quest state.
- `lib/data/` contains generated raw records, logs, and other runtime data.
- `lib/text/` contains help, news, descriptions, and other player-facing text.
- `lib/user/` contains user preference and mapping files.
- `lib/xtra/` contains fonts, graphics, sound, and music assets.
- `lib/modules/` contains module-specific runtime content.

### Important Entry Points

- `src/server/main.c` initializes the server and enters its main loop.
- `src/server/nserver.c` handles server-side network protocol behavior.
- `src/client/client.c` and `src/client/c-init.c` provide common client startup
  and initialization.
- `src/client/nclient.c` handles client-side network protocol behavior.
- `src/server/init1.c` implements parsers for many `lib/game/*.txt` formats.
- `src/server/init2.c` initializes paths, configuration, parsed game records,
  raw data, and Lua.
- `src/server/script.c` initializes and manages server-side Lua scripting.

## Sources of Truth

Use the following precedence when information conflicts:

1. The current C implementation and active parser in this repository.
2. Format comments and working records in the current `lib/game/` files.
3. Current Lua scripts and configuration files used by the relevant subsystem.
4. The bundled `TomeNET-Guide.txt`.
5. External documentation and examples from other TomeNET versions.

Comments and guides can be outdated. A documented feature may be unsupported,
partially implemented, or use a different field layout in the active parser.
Trace behavior from input through parsing, stored structures, and runtime use
when correctness depends on those details.

## Data and Generated Files

Many game records use positional, colon-delimited formats. Preserve exact field
counts, value types, sentinel values, line ordering, and parent-child
relationships. A syntactically accepted value is not necessarily semantically
valid.

Before changing a game record:

1. Read the format comments in the target data file.
2. Find working examples using every line type involved.
3. Inspect the matching parser when a field or behavior is unclear.
4. Resolve referenced IDs against the current repository data.
5. Check the runtime code that consumes the parsed structure.

The build uses a custom preprocessor and tolua pipeline. Files such as
`w_util.c`, `w_play.c`, `w_spells.c`, and some `.pkg` files can be generated
from `.pre` or package definitions. Determine which file is authoritative
before editing generated output.

## Quest Development

Quests are primarily defined in `lib/game/q_info.txt`. Their runtime structures
and behavior are implemented in `src/server/quests.h` and
`src/server/quests.c`, while the active data parser is in
`src/server/init1.c`. Quest-related Lua helpers are in
`lib/scpt/quests.lua`.

Before creating or modifying any quest, read
[tomenet_quest_creation_recommendations.md](tomenet_quest_creation_recommendations.md)
in full and follow it as the task-specific instruction set. It contains the
required investigation process, state-machine design guidance, parser and field
rules, questor index handling, item and reward rules, and runtime validation
checklist.

At minimum, quest work must follow these constraints:

1. Design the complete quest as a stage-based state machine before serializing
   it into `q_info.txt`.
2. Validate every colon-delimited field against this repository's format
   comments, parser, and working examples.
3. Preserve positional ownership and ordering between parent and child lines.
4. Track questor indexes explicitly because they are assigned by `Q` line
   declaration order.
5. Verify monster, item, feature, dungeon, town, and other record IDs against
   current data files.
6. Validate dialogue, goals, item handout and delivery, rewards, repeatability,
   and stage transitions independently.
7. Treat successful parsing as only the first validation step. Location,
   interaction, persistence, and lifecycle behavior may require in-game tests.
8. Keep `q_info.txt` configuration problems separate from engine defects. Do
   not modify server code to work around a quest issue unless the user
   explicitly approves server-code changes.
9. Do not build or restart the server as part of quest work unless explicitly
   requested.

When reporting quest work, include the stage flow, questor index map, record IDs
and their source files, validation performed, assumptions, and checks that still
require a running server.

## Build Reference

The primary Unix makefile is `src/makefile`. Other platform-oriented makefiles
are located in `src/`, including GCU, Windows, MinGW, macOS, and ARM64 variants.
These targets are reference information only and must not be invoked without an
explicit user request:

- `tomenet.server` builds the authoritative game server.
- `tomenet` builds the normal client.
- `tomenet.test` builds a test-tagged client.
- `tomenet.console` builds the remote console.
- `accedit` builds the account editor.
- `all` builds the server, normal client, and account editor.
- `install` builds and copies the main executables to the repository root.
- `src/world/makefile` builds the separate `worldd` service.

The default client target expects platform libraries such as X11, SDL2,
SDL2_mixer, and ncurses according to the selected makefile and flags. Do not
infer that a failed build is caused by source code until the selected platform
configuration and dependencies have been checked.

## Validation and Testing

This repository does not have a general automated test suite. Use focused static
inspection and user-approved validation appropriate to the changed subsystem.

- For C changes, inspect callers, shared declarations, conditional compilation,
  protocol compatibility, persistence implications, and generated bindings.
- For network changes, update and verify both endpoints when the packet contract
  is shared, including version gates and failure handling.
- For data-file changes, validate parser shape, IDs, ordering, references, and
  runtime semantics.
- For Lua changes, inspect exposed C bindings and all call sites of the modified
  functions.
- For persistent data changes, do not alter or migrate live files without
  explicit approval and a rollback plan.

If build or runtime validation was not explicitly requested, state that it was
not performed. Never start a server merely to complete a validation checklist.

## External Documentation

- [TomeNET website](https://www.tomenet.eu/)
- [Online TomeNET Guide](https://tomenet.eu/guide.php)
- [Bundled TomeNET Guide](TomeNET-Guide.txt)

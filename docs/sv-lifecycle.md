# Stage A ticket 05: input and session lifecycle

The input router owns a bounded queue of accepted keys. Native SDL input and
configured macros enter the same production `sv_app_accept_key` path; a separate
budgeted `sv_app_dispatch_input` sends commands through the existing serializer.
Each accepted key keeps the request sequence. The owning router belongs to one
session generation and is cleared on close/failure/replacement. A completed or
server-aborted request cannot lend its queued keys to a later request. Dispatch
reports stale entries explicitly; mandatory queue overflow ends the session with
`SV_KEY_OVERFLOW` instead of evicting accepted input.

Bindings are application preferences and survive session replacement. Matching
happens once, when input is accepted, before any subsequent binding edit.
The Stage A profile supports one ASCII trigger and one action key per macro.
Normal and hybrid macros expand in the selected non-shopping key-request context;
command macros remain inactive and their physical key is used. Expansion does
not recursively match another macro. A physical backquote becomes Escape after
matching; Escape sends the existing zero reply. Backquote inside a macro action
is baseline `MACRO_WAIT`, so this unsupported action is explicitly rejected,
as are delimiter/wait control bytes 28–31 and non-ASCII input. Native key repeats
remain ignored. The synthetic lifecycle scenario installs `m` → `Y` as fixture
configuration; it is not a new default or a personal preference-file loader.

Baseline sources are `src/client/c-util.c`: `macro_maybe`, `macro_ready`,
`inkey_aux` (inactive macro classes, no recursive expansion, wait markers),
`inkey` (backquote conversion), and `get_com`; `Receive_request_key` and
`Send_request_key` in `src/client/nclient.c` own the wire contract. This is a
local SV adaptation under AGENTS.md, not a change to those legacy files.
Multi-key triggers, multi-action expansions, waiting macros, pref-file loading,
shops/message-entry policy and full game-command contexts are not implemented
by this foundation slice. No full macro compatibility claim is made.

`SvNativeInput` records the generation and SDL timestamp epoch immediately after
session creation. Events queued before that epoch, and input delivered without
rebinding the adapter to the replacement generation, are consumed without a
command. Accepted application keys also require generation and request identity.
OS focus gain/loss and minimize events never modify logical context or the queue.
Resize/restore/render-reset events discard only derived UI caches via
`sv_ui_rebuild`; the resources, pending interaction and model have other owners.
UI caches and request storage are fixed-size values, cleared on replacement or
completion, with no retained historical frames, input archive or capture ring.

```sh
make -C src -f makefile.sv tomenet-sv
python3 tests/sv_lifecycle_checks.py
python3 tests/sv_lifecycle_native.py --backend software
python3 tests/sv_lifecycle_native.py --backend opengl
./src/tomenet-sv --synthetic --profile-root /tmp/sv-lifecycle-manual --library "$PWD/lib" --fixture-window 1024x768 --lifecycle-check
```

Choose a fresh or already marked synthetic profile. The runnable native scenario
asserts six lifecycle transitions and ten submitted frames. A normal macro is
accepted through SDL and remains queued across resize, focus loss, minimize,
restore, focus gain and surface reconstruction. During each transition, HP changes
and two identical message occurrences continue through production processing;
pixel assertions observe prompt/status/message surfaces. Dispatch emits one
exact reply after all transitions. Replacement then proves that accepted keys,
old SDL events, old identities and undelivered messages cannot affect the new
session, while a new SDL key emits exactly one new response.

Resize/minimize/restore also call actual SDL window APIs. Deterministic events
exercise the same production handlers; focus changes are injected rather than
claiming control of compositor focus. Readback while minimized verifies renderer
content, not on-screen visibility. The scenario remains runnable for human
window/input review, but automation is not human UX approval.

Headless ASan/UBSan/LeakSanitizer assertions additionally exercise an ordered
normal/hybrid/command/physical-cancel sequence, nonrecursive macro expansion,
unsupported action rejection, stale request input, preferences across sessions,
pending teardown, owner-specific server abort, key/message/output hard overflows,
and storage cleanup. All assertions use production Application/Session/router
and capture actual serializer output at the controlled transport boundary.

These checks establish foundation behavior, not complete macro-context coverage
or real login/session acceptance.

## Verification — 2026-09-22

Linux amd64 SV builds with the target's `-Wall -Wextra -Werror` checks; the legacy
SDL3 target is up to date. All 17
available regression runners passed: architecture/message/request/lifecycle
ASan+UBSan+LeakSanitizer, legacy HP, and HP/message/request/lifecycle/architecture/
shell-resource checks on software and OpenGL. Each backend's lifecycle scenario
checked six transitions, ten frame submissions and two exact replies. Existing
HP/message/request assertions remain enabled (78/156, 18/54 and 40/120 cases/
frames; 372 headless request fragmentation cases). LSan and native windows ran
through normal sandbox escalation, without disabling checks. Local temporary
logs: `/tmp/sv05-check-01.log` through `/tmp/sv05-check-17.log` and
`/tmp/sv05-build.log`.

The MinGW target was attempted and stopped at its missing SDL3/SDL3_ttf/FreeType
development dependency check (`/tmp/sv05-mingw.log`). No Windows/Wine, Fedora41
shipping-baseline or human UX acceptance is claimed.

Standards and Spec reviews ran independently in parallel against starting commit
`53718b3b7` and the working-tree implementation, including new files. Both report
zero findings; Spec review explicitly assessed the representative macro profile
against this ticket's foundation scope. The local tracker setup file is absent;
review used the user-supplied local ticket and parent spec.
The separate legacy queue-growth improvement discovered while checking baseline
input processing is recorded as SV-IMP-004 in [sv-improvements.md](sv-improvements.md).

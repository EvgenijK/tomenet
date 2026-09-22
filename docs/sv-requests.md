# Stage A ticket 04: native key requests

The synthetic shell presents a server key request in a logical child surface
inside its one SDL window. Type a character to answer, or Escape to cancel;
Escape after completion exits the shell. The decoded HP and message surfaces
continue to update while the request is pending.

```sh
make -C src -f makefile.sv tomenet-sv
./src/tomenet-sv --synthetic --profile-root /tmp/sv-request-manual --library "$PWD/lib"
python3 tests/sv_request_checks.py
python3 tests/sv_request_native.py --backend software
python3 tests/sv_request_native.py --backend opengl
```

Choose a new profile directory, or reuse one marked by this synthetic client.
The peer, profile bootstrap and acceptance scenarios are temporary fixture code;
the decoder, Session, input router, serializer, SDL input adapter and renderer
are production modules. There is no terminal link or fallback route.

`sv_decode_key_request` and `sv_send_key_reply` live in
`src/client/sv/key-request.h`; legacy handlers retain their baseline code. The
request retains its signed 32-bit identity and original 80-byte prompt slot.
The wire layouts are `%c%d%s` and `%c%d%c`; neither has a version branch.
Preflight waits for the complete NUL-terminated prompt before publishing anything
and rejects an unterminated full slot instead of adopting the legacy scanner's
truncation. This protection belongs to SV; the legacy scanner/handler issue
remains tracked separately in SV-IMP-002. Valid requests and reply bytes retain
baseline behavior.

Session owns the bounded request payload and its session-local sequence.
The input router owns its pending identity and logical parent context. A reply
requires the current generation and sequence; completion clears request storage
and restores the parent. The only parent implemented in this slice is the game
context. A second request before completion fails explicitly rather than silently
replacing the first. Output overflow likewise ends the session explicitly.
Drawing or recreating a surface never emits commands.

Escape maps to key zero. Server abort follows this request owner's contract:
`Receive_request_abort` sets `request_abort`; `inkey` returns Escape;
`get_com` returns false; `Receive_request_key` sends a zero reply. Thus an active
SV key request emits exactly one zero response on server abort. An abort without
a pending owner does nothing, including a repeated abort after completion.
This contract does not claim the cancellation semantics of other request types.

SDL text input handles one ASCII character with keyboard layout/shift applied;
key events handle Escape, Return, Tab, Backspace and Ctrl+A–Z. Unsupported text
keeps the request pending. Raw prompt bytes remain intact; the declared shell
ASCII display profile supplies visible substitutes for unsupported glyphs.
Ticket 05 adds the bounded accepted-input queue, a representative macro profile
and lifecycle/focus evidence; see [sv-lifecycle.md](sv-lifecycle.md). Complete
macro processing and additional contexts remain later migration work. Neither
slice claims broader keyboard or encoding parity.

## Evidence

The headless sanitizer runner covers 372 fragmentation cases: empty, one-byte,
78-byte and 79-byte prompts, two adjacent HP layouts, unsigned prompt bytes and
signed request identity, no publication before completion, Escape and repeated
server abort, exact serialized bytes, stale identities, parent restoration,
cleared payloads and retained output after too-small transport reads. Separate
checks cover normal input and malformed unterminated prompt rejection.

The native scenario covers all splits of a real prompt on two versions (40
cases), with three submitted frames per case. It reads visible HP/prompt pixels,
updates HP during the pending request, reconstructs the view, sends text/Escape
through the SDL event queue and production adapter, exercises server abort,
and verifies exact replies and no duplicate after redraw. These are visibility
assertions, not pixel-perfect or human UX approval.

## Verification — 2026-09-21

Linux amd64 SV and the legacy SDL3 `tomenet` target built successfully. All 14
available regression runners passed after the review fix: request, message and
architecture ASan/UBSan/LeakSanitizer checks; legacy HP regression; native request,
message, HP, architecture and shell/resource checks on both software and OpenGL.
Native request checks submitted 120 checked frames per backend. LeakSanitizer
requires execution outside this environment's ptrace-based sandbox.

Parallel review used starting commit `d7b2b9c50`. Standards initially found one
architecture violation: Application interpreted server abort. The input router
now owns that rule and returns the prepared reply for Application to deliver.
Follow-up Standards review has zero unresolved findings. Spec review has zero
findings. The local tracker setup document `docs/agents/issue-tracker.md` is
absent; review used the explicitly supplied local ticket and parent spec.

MinGW was attempted and is blocked by absent cross SDL3, SDL3_ttf and FreeType
development packages. Windows/Wine, Fedora41 shipping-baseline verification and
human visual acceptance are not claimed.

## SV-ARCH-002 isolation — 2026-09-22

HP/message/request helpers now belong to `src/client/sv`; version comparison
is local to SV. The targeted legacy/common files match upstream `59473651d`
exactly, preserving its unrelated changes. The legacy HP harness now links
`common.c` rather than the SV version implementation. Strict field-boundary
checks remain in SV only; the legacy issue remains open as SV-IMP-002.

Linux SV and legacy SDL3 builds passed. All 14 available regression runners
passed: three ASan/UBSan/LeakSanitizer runners, legacy HP, and HP/message/request/
architecture/shell checks on software and OpenGL. Native counts per backend
remain HP 78/156 frames, messages 18/54, requests 40/120; request sanitizer
coverage remains 372 fragmentation cases. MinGW was attempted and remains
blocked by missing cross SDL3/SDL3_ttf/FreeType development dependencies.
See [SV-ARCH-002](tasks/SV-ARCH-002-isolate-sv-from-legacy-changes.md) for review
and completion evidence; earlier verification sections above are historical.

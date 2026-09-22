# Stage A ticket 03: ordered message occurrences

The synthetic client now decodes two identical messages alongside HP and displays
both as separate rows. Launch normally for visual inspection, or run:

```sh
make -C src -f makefile.sv tomenet-sv
python3 tests/sv_message_checks.py
python3 tests/sv_message_native.py --backend software
python3 tests/sv_message_native.py --backend opengl
```

The application accepts PKT_MESSAGE through SV-local `sv_decode_message`
in `src/client/sv/protocol/message-update.h`. The field uses the same `%c%S` scanner and has
no version-dependent layout; scenarios select both HP boundary versions
4.7.0.2.0.1 and 4.7.0.2.0.2 to verify adjacent packets. A preflight requires NUL
within the 256-byte field slot. Incomplete fields wait without publication;
256 non-NUL bytes fail without cursor advancement or truncated publication.
This intentionally prevents scanner truncation in the SV message path. Legacy
`Receive_message` uses its original scanner directly; its unterminated-field
problem remains separate work tracked as SV-IMP-002.

`SvSession` dispatches decoded semantic changes behind one apply interface;
the application owns sequencing, lifecycle and delivery to alert executors.
Unchanged HP revisions do not trigger HP alerts when a message arrives.

`SvSession` assigns monotonically increasing, session-local sequence numbers to
all messages, including repeated, empty and clear-sentinel fields. Together with
the application's generation this identifies an occurrence. Original unsigned
bytes, explicit length and NUL remain in each event. A 64-entry pending queue
retains mandatory delivery independently of the six-line current live feed.
The feed can evict old visible lines without removing pending deliveries.
`sv_app_take_message` copies and acknowledges exactly one event in input order;
invalid, stale or closed calls consume nothing. The shell acknowledges after
each budgeted processing pass, separately from drawing. This slice's only message
effect is publication to the live feed; chat audio, notes, clone/export sinks,
full recall and routing are later capabilities, not silently claimed here.

The clear-topline field (a lone 0xff followed by NUL) clears the current feed but
remains an ordered delivery. Surface reconstruction reads a value snapshot;
it cannot acknowledge or redispatch events. Session close/failure releases the
queue and clears current message state; reopening starts a new generation and
sequence. The six-line feed is not the full message-recall store: the approved
four baseline recall rings and relog preservation remain Stage B work.
At queue or sequence exhaustion, the application explicitly closes the session
with `SV_EVENT_OVERFLOW`; the SDL shell remains running and can show the reason.
Transport retains its existing 1024-byte buffers and backpressure policy.
There is no recorder, archive or optional diagnostic consumer.

The display projection uses the shell's declared ASCII profile, ordinary
16-color formatting, reset-first, swap-previous, neutral, doubled-0xff literal
brace and safe trailing-marker handling. Routing markers occupy no cells.
Unknown formatting codes retain their symbol in the current color; unmapped
byte symbols use one visible `?` cell without altering the original field.
Animated palette roles and a complete legacy glyph profile remain later renderer
work; this native slice makes no full encoding or renderer-parity claim.
Message-region revisions and session generations invalidate the derived text
cache. Font/scale/geometry changes invalidate clipping and cell geometry through
the existing presentation key; unchanged frames do not parse message text again.
Clipping changes only the derived line, never event data. HP and messages occupy
separate logical surfaces within the existing single SDL window.

## Evidence

The sanitizer runner exercises all splits for payload lengths 0, 1, 254 and 255,
unsigned byte identity, both adjacent HP layouts, exact ordering, duplicate
occurrences, a malformed oversized field, clear sentinel, explicit overflow,
queue reuse, stale generation, close/reopen release and formatted projection.
ASan, UBSan and LeakSanitizer cover the production model/protocol/application;
LeakSanitizer may require execution outside a ptrace-based sandbox.

The native runner exercises 18 complete/fragmented cases with two identical
formatted messages and HP, checks both visible message rows and HP pixels,
reconstructs the UI before delivery, consumes each event exactly once, then
redraws and verifies no further event is available. It submits 54 checked native
frames. Pixel assertions establish visible ink, not pixel-perfect equivalence
or human UX approval. Prepared fixtures enter the production decoder and UI;
there is no terminal route or test-only decoder/renderer.

## Verification — 2026-09-21

Linux amd64 SV and the full legacy SDL3 target built successfully. All available
Linux runners passed: message and architecture ASan/UBSan/LeakSanitizer checks,
legacy HP regression, native HP (78 cases / 156 submissions), architecture,
message (18 cases / 54 submissions), and shell/resource checks on both software
and OpenGL. The full SV matrix passed again after review fixes to model dispatch,
alert evaluation and presentation caching.

Standards review initially found two architecture issues (uncached message
formatting and packet-kind dispatch in application assembly). Both were fixed;
the follow-up Standards review has zero unresolved findings. Spec review has
zero findings. Reviews used the task diff from starting commit `15c6730`.

MinGW verification was attempted but is blocked by absent cross SDL3,
SDL3_ttf and FreeType development packages. No Windows/Wine or human visual
acceptance is claimed for this ticket.

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

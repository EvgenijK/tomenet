# C011 native PAUSE / thin pacing follow-up

2026-11-11. Scope: the current production preparation/native caller only. No
C014 live loop, C013 map application or C053 macro runtime is implemented here.
Linux; dummy SDL software renderer for the focused controls run. No sanitizer,
accelerated renderer, physical-keyboard or Windows claim.

## Checks and result

Actual command, exit 0 after fixes:

```bash
python3 tests/sv_first_session_checks.py --case c011-controls
```

The existing managed peer sends PLAY reply/startup END coalesced with a pending
key request, PAUSE, an interleaved SFLAGS update, another PAUSE, and twelve FLUSH
packets. Native SDL events travel through `run_contact` and the real
`sv_native_input`; they do not call the application acknowledgement directly.
Only test link wrappers observe the real controls/input/presentation functions
on the owner thread. No production counters, copied decoder or alternate policy
were added.

Observed final control subflow:

```
native controls presents=15 pauses=2 stale=2 ack=2 flush=12 paced=10 unpaced=2
```

- Both PAUSE occurrences were actually submitted by SDL_RenderPresent. First old
  Return was rejected; fresh printable KEY_DOWN + TEXT_INPUT acknowledged once.
  The previous gesture's old TEXT_INPUT did not acknowledge the second PAUSE;
  a fresh Return did. Neither acknowledgement answered the pending request 7.
- Text input was active while paused before accepted identity arrived, and was
  already inactive on the next native owner tick after both acknowledgements,
  before runner destruction/SDL_Quit. The paused surface has an explicit
  `Paused - press a key to continue` prompt.
- All twelve FLUSH submissions were observed. The first ten had an actual
  monotonic one-millisecond deadline; each later submission occurred after the
  prior deadline. Flushes 11 and 12 had no added pacing deadline. Interleaved
  SFLAGS value 3 and the pending request survived; final paused state was false.
- Character thin_down_flush=true / disable_flush=false were checked in exact
  embedded PLAY options. The peer rejects a standalone pre-PLAY OPTIONS packet.

Final targeted binary SHA-256:
`70f93d6c5aacc02c01124e551b95989d81167030e95b74fc3a4f7cdf341b0d32`.
Raw artifacts: `.scratch/sv-c011-followup/c011-controls/command.json`,
`build.log`, `stdout.log`, `stderr.log`, `file-transcript.log`. command.json
contains exact revision/dirty state, environment, commands, binary/source hashes,
exitCode=0 and matching before/after original-lib hashes. Root's later complete
11-case run is authoritative for the final consolidated source snapshot.

## Defects reproduced and fixed

1. **Thin count was reset for every packet.** The initial native case observed
   twelve paced and zero unpaced flushes. `preparation.c` had fed bytes through
   `sv_native_controls_step(...,1)`, so artificial decoder WAITING and one-packet
   budget exhaustion repeatedly started a new flush frame. The extracted initial
   observation is saved in `controls-before-thin-fix/observation.txt`; its raw
   runner files had already been overwritten, and no pre-fix hash is claimed.

   Fix: an explicit opt-in `retain_batch` in the existing native controls owner
   keeps the shared bounded 4096-input budget while queued preparation bytes are
   being assembled. Real socket WAITING, budget exhaustion or the terminal
   preparation boundary ends the batch. UI ticks/pacing waits while ingress is
   still queued do not reset it. The due_ns deadline survives boundary reset.
   The default C008 native-controls behavior is unchanged when retain_batch=false.

2. **Printable PAUSE acknowledgement lacked text readiness.** The focused case
   failed `SDL_TextInputActive(window)` during a real presented PAUSE. Raw failed
   stderr/command metadata are preserved in `controls-before-text-input-fix/`.
   Native assembly now temporarily enables text input while paused, including
   before identity; it disables it after acknowledgement unless the ordinary
   name editor owns text input. No C008 input engine semantics were changed.

3. **Standalone pre-PLAY OPTIONS was not a supported server transition.** Root's
   isolated real-server probe exposed the interoperability failure. This agent
   removed only the `sv_login_send_options` call from the current preparation
   caller: effective character options are still loaded/set before PLAY and are
   serialized inside the common PLAY packet. The controlled peer now checks
   those exact embedded slots. This does not assert that every historical legacy
   interactive path never flushes early options; the relevant baseline Net_start
   path clears its output queue and PLAY embeds them. Historical C002 evidence
   is not rewritten.

Changed production responsibilities remain SV-local: `login/preparation.c`,
`login/native-run.c`, `login/flow.c`, `ui/native-controls.[ch]`. Tests extend the
existing `tests/sv/first-session.c` / `tests/sv_first_session_checks.py` binary and
runner. `git diff --check` for those paths passed.

## Other integrated required checks

The same existing harness now includes native committed birth and three actual
initial audio variants, with helper code supplied/reviewed by the corresponding
agents. Individual debug invocations passed before the final wire expectation
change; root reruns the consolidated preset after source freeze:

```bash
python3 tests/sv_first_session_checks.py --case c011-preparation
```

That command now selects all eleven scoped cases. Repeated identity publication,
coherent existing-state redraw under backpressure, and two previously untested
RLE framing branches are folded into the same existing core helpers. The final
checks distinguish their own C011 results from later-stage consumers.

## Consolidated result after freeze

Root subsequently ran the eleven-case preset successfully, exit 0, and
`make -C src -f makefile.sv -j4 shell` successfully. See
`.scratch/sv-c011-followup/final-checks.log` and `build.log`. The controls case in
that consolidated run again passed with the same 10 paced / 2 unpaced policy,
two stale rejections and two real native acknowledgements. Production/test files
remained frozen during this report update.

## Required prefix extension after the initial freeze

A later isolated real-server check exposed equipment frames before identity.
The owning agent added frame-only EQUIP/INVEN support; no C021 model/application
was moved into C011. This agent added one 115-byte EQUIP to the existing managed
prefix, with a 100-byte item name to verify the ONAME_LEN `%I` limit. The same
existing boundary helper checks partial item-name WAITING and exact frame
consumption with a following identity byte. The managed prefix is now 13 frames /
414 bytes. The original four-byte raw INVEN tail after identity remains unchanged.
Strict C99 syntax and diff checks passed. Root owns the subsequent eleven-case
runtime rerun for this updated source snapshot.

The earlier targeted controls receipt corresponding to binary `70f93d...b0d32`
is preserved under `controls-before-equip-framing/command.json` and
`file-transcript.log`. Later consolidated receipts use their own binary/source
hashes; the earlier hash above is not a claim about the new framer build.

The final real-server audit required eight more compact modern prefix fixtures
(audio/unique/wide-item framing only). They are folded into the same managed
prefix and FIFO assertions: now 21 frames / 557 bytes. One wide-item suffix
partial-frame check is added to the existing boundary helper. No new case or
late-stage model/application is introduced.

The same audit exposed the accepted-identity surface being hidden by completed
birth/MOTD presentation. Native assembly now projects a distinct
`preparation_identity` flag only when accepted identity exists and MOTD is
acknowledged; it suppresses the completed birth view and displays actual identity
in the existing card titled `Session preparation`. It does not claim complete
resources or a C014 live session. Ctrl-R requires this post-MOTD context plus
readiness; PAUSE acknowledgement remains available earlier. The existing native
birth driver now asserts the flag is false during MOTD, acknowledges through SDL,
then verifies actual Accepted identity and absence of a winning birth view.
Root owns the final runtime rerun/capture after this last source freeze.

## Terminal receipt

Root's final frozen-source reconciliation supersedes the intermediate receipts:
all eleven managed cases PASS, all 152 production/check source hashes match, and
the unified check binary SHA-256 is
`efe3166a29eb34f4b6dac8945fbe3715110301b4e8c21a8b5800ecbac715a75b`.
The final managed prefix is 21 frames / 557 bytes. Final production build and
Graphify update also passed. `final-reconciliation.json` is authoritative for
this terminal snapshot; earlier hashes above remain historical observations.

The isolated actual-server run 12 also exited 0: accepted identity `Born`,
complete preparation, MOTD acknowledgement observed, phase 8, and 666 retained
deferred frames. Root inspected the captured native preparation card and found
its accepted identity readable. This validates the post-MOTD preparation surface,
not the subsequent C014 live loop or application of deferred gameplay frames.

The final UI/context gate distinguishes accepted preparation identity from full
resource readiness, suppresses completed birth presentation after MOTD, and
allows Ctrl-R only in that preparation context. A final two-line root correction
also gates identity status replacement by `scene.preparation_identity` and the
PAUSE status replacement by `!flow_view.failed`. Consequently, an actual
post-identity failure reason is no longer hidden by stale identity/pause text.
The final eleven-case rerun above includes this failure-status correction.
No production or test files were changed while writing this terminal receipt.

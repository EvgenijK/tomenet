# Stage A human native review — ticket 19

Stage A remains **pending human review** until a person explicitly approves the
final build and the complete automated runner passes. Screenshots, injected
focus events, elapsed time and successful tests are not human approval.

## Launch and restart

Build the synthetic client, then launch on the actual desktop:

```sh
make -C src -f makefile.sv tomenet-sv
/tmp/sv15-venv/bin/python -B tools/stage_a_review.py launch --output /tmp/sv19-human-new --backend software
```

Use a new output directory for each process. This creates a marked isolated
profile and loads the bundled fonts; it needs no account or server. The launcher
prints the executable SHA-256 and retains the OS identity, actual backend,
build ID, output pixels, logical dimensions and display scale in `native.log` /
`review.json`. Build ID identifies compiler configuration; SHA-256 identifies the
actual executable. No personal CFG/OPT files are loaded or saved.

The `--review` fixture adds **F5** (replace synthetic session), **F6** (discard
derived surfaces) and a normal **m → Y** macro. These controls call the same
production session/UI/router paths used by the existing lifecycle tests. They
are not live gameplay commands. Restart the entire process using the command
above with another output path, or press F5 to repeat a flow in this process.

## Short human scenario

1. Check that there is one window and that HP, two separate `Echo` occurrences
   and `Choose a key:` are readable. Report actual monitor/desktop scale; virtual
   geometry tests do not establish physical display coverage.
2. With the prompt pending, switch to another desktop application and back,
   resize the window, minimize and restore it. The prompt must stay pending;
   HP and both messages must remain readable. This must be real desktop input.
3. Press F6 while pending. The surface should return with the same prompt and
   messages. Press an ordinary ASCII key, such as `a`: the prompt closes once.
4. Press F5, then Escape: only the request is cancelled. Press F6 afterwards:
   the old prompt must not return. A second Escape exits the shell.
5. Press F5 again; repeat a focus/resize transition and press `m`. The prompt
   closes through the representative normal macro. The diagnostic serialized
   reply ends in `59` (`Y`); exact bytes are an automated assertion, not something
   the reviewer has to decode. Ordinary `a` ends in `61`, cancellation in `00`.
6. Press F5 while a prompt is pending, then answer the fresh request. Confirm
   a clean new session, no doubled messages or unexpected completion. Close
   the window after checking perceived response and any visual problems.

Visible-response targets are urgent 50 ms, interactive 100 ms, background 250 ms
for implemented flows. The reviewer describes perceived response; this is not a
measurement of a hard timing budget. No background flow is implemented. Actual
submission timing remains the independent ≤20/≤50 ms automated gate.

## Record and validate

After the process closes, `review.json` remains `decision: pending`. Fill in
`reviewer`, timezone-qualified `reviewedAt`, `physicalConditions` and all nine
`actions` with the actions actually performed, their `result` and concrete
`notes`. Set `decision: approved` only on explicit positive human feedback;
otherwise retain pending or record rejected. A failed action cannot accompany
approval. An agent may transcribe the user's explicit response, retaining its
meaning and limits; it must not invent approval or unperformed actions.

The checker validates schema, date, complete positive observations, successful
native exit, log integrity/build/backend identity, executable/configuration,
OS, both bundled font fingerprints and the complete reviewed host dependency
closure against the fresh candidate evidence. Use the same Python environment
as the full runner: its dependency inventory is intentionally included. The
launcher captures host provenance before review and revalidates it afterwards;
changed SDL/TTF/FreeType/graphics libraries invalidate an old review even when
the executable itself is unchanged.
It binds the resulting review status to the exact candidate-evidence digest.
Review files are trusted human attestations, not cryptographic proof of authorship.
The launcher currently supports Linux software/OpenGL human observations; Wine
and other physical conditions are not silently inferred from that review.

```sh
/tmp/sv15-venv/bin/python -B tools/stage_a_review.py check \
  --review /tmp/sv19-human-new/review.json \
  --evidence /tmp/sv19-final-evidence/candidate-evidence.json

/tmp/sv15-venv/bin/python -B tools/run_stage_a.py \
  --output /tmp/sv19-final-evidence --mingw-sdk /tmp/sv16-sdk-final \
  --sdk-downloads /tmp/sv16-downloads --human-review /tmp/sv19-human-new/review.json
```

The check command alone never grants Stage A. The runner checks fresh builds,
full regression/geometry/timing matrices and the production evidence gate before
combining them with the review. Exit 0 means this scoped Stage A checkpoint is
accepted; exit 1 means blocked; exit 2 means automation passed and human review
is still pending. Failed Direct3D timing is retained and blocks closure even if
the human software review passes. Canonical full capability claims and B–F
remain pending. Native/resource changes require matching new human observations.

Required virtual geometry is 1024×768/100%, 1920×1080/100%, 3840×2160/200%, and
1024×768 logical at 125% and 150%, with TTF and PCF. A second physical 4K monitor
is not required. Record only physical conditions actually observed by the person.

## Current result

The full gate in `/tmp/sv19-final-evidence` finished **61/62 commands passing**,
exit **1 / blocked** on 2026-09-23. The fresh production checkpoint passed:
21 records cover all eight scoped A outcomes with no dependency/runtime errors.
The canonical registry remains 925 pending / 0 full accepted capabilities;
all 24 inventories and 2,077 rows reconcile.

Both SV builds, legacy SDL3/X11 builds, all data/checker tests, five sanitizer
suites, required Linux software/OpenGL and Wine software scenarios passed.
The new human-review contract has seven passing tests. Required virtual geometry
passed with both TTF and PCF. Existing timing negative controls still reject
intentional delays on all backends whose positive timing completed.

| Actual backend | Maximum urgent / 20 ms | Maximum interactive / 50 ms |
|---|---:|---:|
| Linux software | 3.644 ms | 3.486 ms |
| Linux OpenGL | 0.855 ms | 1.269 ms |
| Wine software | 0.823 ms | 0.682 ms |
| Wine Direct3D | **37.719 ms — failed** | 0.960 ms |

The additional Wine Direct3D positive timing scenario failed its first prompt
submission; its wrapper stopped before the delayed control. This is the sole
failed command and remains blocking. It is the previously recorded
[Direct3D profiling issue](sv-improvements.md#profile-wine-direct3d-first-prompt-submission),
not missing software coverage or actual Windows acceptance.

Final executable SHA-256 values:

- Linux: `714523fd78c49f6a9fc6b2bd38d93202c478434db2a6bb8ea27184093e7417c7`.
- MinGW/Wine: `10a395f835f9640053ec7227c25fa2682af8823ec9246e88cd037e34a6879db0`.

The [command report](acceptance/stage-a-ticket-19-2026-09-23.json) and
[evidence archive](acceptance/stage-a-ticket-19-2026-09-23.tar.gz) retain logs,
scoped evidence, fingerprints, host provenance and selected build/dependency
metadata. Archive SHA-256:
`3c3c726689916eeb38219802e19ec8c06f0afd469853e7ff73f1ab761d2436bd`.
All report source/log fingerprints were checked before archival. The external
SDK, host files, executable and staged Wine root must remain available for
freshness verification; the archive is not a shipping package.

The manual software launch uses the same Linux executable and isolated profile
`/tmp/sv19-human-software/profile`, build `linux-9dd5a1e98a672a5b25cd`,
Linux 6.18.49-1-MANJARO / Wayland. Initial native output is 1357×1018,
logical 1024.151×768.302, display scale 1.325. These are launch diagnostics,
not a claim of human readability/focus approval or physical 4K testing.
No positive human review has yet been received; ticket 19, ticket 15 and Stage A
remain open for **human review and the Direct3D timing failure**.

## Standards

Independent Standards review against starting commit
`e1238a85ba31c6350486934b9750ffbbba064618`: **0 findings**. Manual controls stay
in SV, use production interfaces and do not modify legacy/common behavior.

## Spec

Independent Spec review initially found stale approval possible after a shared
runtime-library update. The launcher now captures/revalidates host provenance;
the checker matches its digest to fresh candidate evidence. The negative test
covers changed dependencies. Follow-up review: **0 remaining implementation
findings**. Explicit human feedback and fresh complete gate results remain
closure requirements; this review does not grant them.

Review totals: Standards **0**, Spec **0** outstanding implementation findings.

## Human feedback received — 2026-09-23

The user wrote: **«выглядит нормально»** (“looks normal”). This is recorded as
positive general visual feedback, not an assertion that all behavioral steps
were performed. A follow-up asks whether ordinary answer, Escape, real desktop
focus, resize/minimize/restore, F6, m and F5 all worked. No reply to that follow-up
has yet been received, so individual action results and overall decision remain
pending. Reviewer identity is the user in this Codex conversation; no name or
additional physical-monitor description was supplied.

The manual process exited successfully. Its executable and host dependency
identity match the fresh gate. The [manual review archive](acceptance/stage-a-human-review-2026-09-23.tar.gz)
contains the exact feedback record, native diagnostics and host provenance.
Archive SHA-256: `268cc6ed4c036277789c20fd0104e0621b8a2fc4b2d022627bdc0e5102822982`.
Extract into an empty directory and pass its `review.json` to the review checker;
expected status is **pending**, exit **2**. The native log is supporting diagnostic
evidence, not a replacement for the human's confirmation of actions or perceived
response. The separate Wine Direct3D timing blocker still prevents Stage A closure.

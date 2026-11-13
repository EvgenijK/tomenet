# SV-B2-002 — server/account presentation evidence

2026-10-28. Linux implementation and review evidence for B2-002; acceptance of
other B2 tickets is not implied. Existing dirty-tree work was preserved; no commit.

## Production implementation

The existing endpoint and credentials interactions drive the B2 server cards,
direct-address overlay, simultaneous account/password fields, masked/private reveal,
selection/caret/focus, submit/back, and factual contact/auth status. Draw and mouse
input share logical rectangles. The existing protocol, account-creation confirmation,
vault store/restore, and session-only policy remain authoritative.

Review corrections include shared scaled hit testing, stale credential-event filtering,
reveal reset on focus loss, manual selection/focus, reference-sized modal and account
spacing, serif headings, server badges/connect controls, panel-only modal dimming,
and primary Connect styling. Only necessary assertions were added to the existing
endpoint/credentials core checks. No new test suite or provider acceptance matrix.

## Frozen reference and native frames

Reference source is the external UX prototype `/home/svechnik/Projects/github_site/tomenet_interface`,
commit `1bb2d5d45b9001bf2a6f2084f78184c29a73fca7` (HTML/CSS/data/assets),
variant D, Liberation Mono, background 0 (`cirith_ungol_ascii_seed06.png`), reduced
motion, DPR 1. The browser contains prototype data; native frames use the explicit
servers.txt input or the controlled local address 127.0.0.1:1. Password captures
are masked; the production capture hook skips private reveal frames.

| Surface / logical size | Frozen browser | Linux software | Linux OpenGL |
| --- | --- | --- | --- |
| Server 1440×900 | [reference](evidence/sv-b2-002/reference-server-1440x900.png) | [native](evidence/sv-b2-002/software-server-1440x900.png) | [native](evidence/sv-b2-002/opengl-server-1440x900.png) |
| Server 1024×768 | [reference](evidence/sv-b2-002/reference-server-1024x768.png) | [native](evidence/sv-b2-002/software-server-1024x768.png) | [native](evidence/sv-b2-002/opengl-server-1024x768.png) |
| Direct 1440×900 | [reference](evidence/sv-b2-002/reference-manual-1440x900.png) | [native](evidence/sv-b2-002/software-manual-1440x900.png) | [native](evidence/sv-b2-002/opengl-manual-1440x900.png) |
| Direct 1024×768 | [reference](evidence/sv-b2-002/reference-manual-1024x768.png) | [native](evidence/sv-b2-002/software-manual-1024x768.png) | [native](evidence/sv-b2-002/opengl-manual-1024x768.png) |
| Account 1440×900 | [reference](evidence/sv-b2-002/reference-account-1440x900.png) | [native](evidence/sv-b2-002/software-account-1440x900.png) | [native](evidence/sv-b2-002/opengl-account-1440x900.png) |
| Account 1024×768 | [reference](evidence/sv-b2-002/reference-account-1024x768.png) | [native](evidence/sv-b2-002/software-account-1024x768.png) | [native](evidence/sv-b2-002/opengl-account-1024x768.png) |

Composition and typography roles were compared at both supported sizes. The native
modal now measures 390×270 at reference positions; minimum account fields and submit
match reference spacing. Known differences: B2-001 background shading/resampling
without CSS blur/backdrop; square borders and arrow enter symbol; Liberation Serif
substitutes browser Georgia; real native server labels/host/port and unavailable
metadata replace fixture game/version/population; Guide/realm links are explicitly
unavailable, and factual vault status occupies the native status region. Browser
masked bullets and native asterisks differ. No pixel-perfect claim.

## Linux checks and reproduction

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS, warnings-as-errors.
- `python3 tests/sv_endpoint_checks.py` — PASS: production endpoint/editor,
  credentials, normal/sticky input, metaserver and native scene; UBSan core paths.
- `python3 tests/sv_account_create_checks.py` — PASS: existing account success and
  server-confirmed storage scenarios; ASan/UBSan.
- `python3 tests/sv_account_failure_checks.py` — PASS: existing failure/retry/cancel
  scenarios; ASan/UBSan.
- Native software/OpenGL frames — complete production renderer captures at both
  supported logical sizes; actual XTest keyboard input enters direct overlay and
  account/password text. At 1.25 display scale, actual mouse Retry
  returned from confirmed refused contact to the retained account with cleared
  password. Mouse Cancel is verified by a minimal assertion calling the production endpoint
  adapter with the shared Cancel rectangle, in the same existing core suite.

Controlled-peer suites need local socket bind; initial sandbox listener rejection
was resolved by auto-approved escalation. Browser/X11 also needed auto-approved
escalation. Initial X11 `import` attempts were unstable; retained PNGs come from
`SDL_RenderReadPixels` after the complete production frame, using the existing
capture hook with `SV_B2_CAPTURE_FRAME=400`.

Reproduce from repository root:

```sh
node docs/evidence/sv-b2-002/capture-reference.cjs docs/evidence/sv-b2-002
cp src/tomenet-sv /tmp/sv-b2-002-capture
python3 docs/evidence/sv-b2-002/capture-native.py
python3 docs/evidence/sv-b2-002/capture-scaled.py
```

These are evidence reproduction scripts, using the real executable and XTest,
without a second behavioral model. X11 scripts temporarily select XKB group 0
and restore the prior group. The final binary SHA-256 is
`f09409b26bc82aadca0ae7758541ac67b4762ea0241f0a0760d319b5ccd88e3b`.
Account captures precede the final manual-only color/dim adjustment; their account
rendering is unchanged. Final manual/server captures use this binary.
Build flags/dependency versions are in
[evidence build.txt](evidence/sv-b2-002/build.txt); logs, computed browser styles,
input descriptions and file hashes are alongside PNGs. Captures do not confirm
physical-display timing, a system IME, live-server gameplay or a real credential
provider acceptance matrix. Windows/MinGW/Wine remain deferred by the platform rule.

## Standards

Independent review covered the task diff against the pre-task snapshot, preserving
pre-existing dirty-tree changes. The credentials agent reviewed the lead-owned
integration, and the orchestrator independently reviewed credentials. Findings
about stale input, private reveal lifecycle, manual selection/focus and scaled
hit testing were corrected. Final existing endpoint suite, including the shared-rectangle mouse Cancel
assertion, passed. No actionable standards findings remain.

## Spec

Independent Spec review compared the B2-002 ticket, B2 spec, frozen reference,
production paths and native captures. Initial findings about manual width/position,
card structure, account spacing/typography and modal dimming/primary styling were
corrected. The reviewed server/account/manual compositions now follow the frozen
reference with the explicit native adaptations above. The initial bounded error capture used `--frames`, whose existing fixture contract
exits immediately after network failure. Removing that flag produced the actual
refused-contact surface; no renderer lifetime fix was needed. Independent scaled
Retry replay shows the retained account and cleared password. Representative scaled hit testing is confirmed by Retry;
mouse Cancel additionally passes the production core adapter check. No actionable
Spec findings remain within this ticket. An optional actual XTest Cancel replay
at 1.25/OpenGL did not confirm exit; a bounded software 1× retry stalled before
reaching the refused-contact surface. These attempts are retained as limitations,
not PASS, and do not claim a client-renderer defect. No successful physical Cancel replay is claimed.

Representative scaled evidence: [refused contact](evidence/sv-b2-002/opengl-scaled-error.png),
[Retry returning to account](evidence/sv-b2-002/opengl-scaled-retry.png).
Each uses 1024×768 logical units and 1280×960 output at 1.25 display scale.
Retry preserves the account and clears the password. The corresponding actual
failure log was checked before dispatching the retry click. The default reproduction script attempts Direct/Connect, error and Retry.
The retained `opengl-scaled-manual-connect.png` was overwritten by a failed
replay and does not confirm Direct/Connect; its result is UNCONFIRMED in the
input summary. Manual Connect/Cancel behavior is covered by the production core
adapter assertions. Optional Cancel failures and fixture-flag diagnostics remain
in separate logs.

## Graph and final consistency

`.graphify-venv/bin/graphify update .` completed after the final core change:
17814 nodes, 46437 edges. Code extraction still reports partial syntax extraction
in 77 files and no symbols in 154 files; semantic documentation/image extraction
is not claimed. The graph is supplementary to source review and production checks.
All 40 bundled resource hashes matched. Scoped `git diff --check` passed and no
temporary debug instrumentation remains. Evidence file hashes are recorded in
[SHA256.json](evidence/sv-b2-002/SHA256.json).

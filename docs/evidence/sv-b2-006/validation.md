# SV-B2-006 Linux validation — 2026-10-29

Owner: validation agent. Existing dirty working tree retained; no source edits, no new tests or suites. Linux is the required platform; Windows/MinGW/Wine deferred by docs/agents/platform-validation.md.

Production build: `make -C src -f makefile.sv tomenet-sv -j4` — PASS (warnings-as-errors, incremental build). Executable: `src/tomenet-sv`; build ID `linux-bd7f9b5e26c5789bb7e9`; SHA256 `5955383b5c06d50a4eb9fc63fa3ff5fd06851f687251a4be72346f1dc75405d9`. Full compiler/dependency/environment fingerprint in build-fingerprint.txt; build stdout in build.log. Capture agent notified to use a separate executable copy.

Existing checks selected to cover B2 integration ownership, production state/input/transport/MOTD boundaries and shell resources. No expansion of core assertions was needed.

| Command | Result | Evidence |
| --- | --- | --- |
| `python3 tests/sv_endpoint_checks.py` | PASS | endpoint.log |
| `python3 tests/sv_login_checks.py` | PASS | login.log |
| `python3 tests/sv_first_session_checks.py` | PASS | first-session.log |
| `SV_GUIDE_EVIDENCE=$PWD/.scratch/sv-b2-006/guide python3 tests/sv_guide_motd_checks.py` | PASS | guide-motd.log; guide/ |
| `SDL_VIDEODRIVER=dummy python3 tests/sv_shell_smoke.py --backend software` | PASS | shell-software.log |

Initial first-session and Guide/MOTD launches inside sandbox failed to open their local loopback listener (`peer.listener >= 0`). The same commands were rerun with approved local-socket execution; this was an environment restriction, not a code regression. No behavior/test expectations were changed.

First-session success/disconnect/quit/retry and once-only live handoff use production SV code with a managed peer. Endpoint checks cover production endpoint/editor, credentials, native input/hit rectangles, metaserver and scene. Login checks cover production parser/contact handoff. Shell smoke uses actual production executable and verifies isolation/resource fallbacks plus software backend. Expected missing-resource diagnostics in the negative checks are not failures.

This report does not substitute for B2-006 actual executable reference/native captures, both controlled-peer startup routes, visual review or scaled OpenGL input checks. Those are owned by the implementation/orchestrator agents. Dummy renderer checks cannot establish physical display behavior or real OS IME.

Guide/MOTD PASS: existing default cases `waiting,partial,ack,skip,point` in both `SV_CLASS_BEFORE_RACE=1,0` orders under ASan/UBSan. Preserved exact Guide placeholder/caller return, partial/final ACK gate and once-only live handoff; complete ACK is `02 03 7a`, final handshake capture is 16266 bytes. Existing evidence encoder retained per-case frames/transcripts in guide/.

## Hover review correction — final rebuild

The reviewer added an SV-local current-pointer presentation helper in `src/client/sv/ui/endpoint-scene.c`; exact delta against `/tmp/sv-b2-006-review-endpoint-scene-before.c` retained in hover-fix.diff. Independent source review: PASS, no actionable findings. Pointer coordinates use the same window-pixel-density / render-scale conversion and the same production rectangles as existing native input. Current mouse-focus guards prevent hover in another window; no session state, keyboard focus, interaction availability or protocol logic changes. Enabled/disabled and active/selected colors retain priority. The helper also supplies the existing shared birth buttons without duplicating geometry.

Final `make -C src -f makefile.sv tomenet-sv -j4` — PASS (build-hover.log). Final executable SHA256 `56ddef2d477ba7a806638aaf227b312e9bedfc5384dbc0789d4200dfb1d60a19`; build ID remains `linux-bd7f9b5e26c5789bb7e9` because the build cache key describes toolchain/flags, not source hash. Final fingerprint in build-fingerprint-hover.txt. Capture owner notified to copy/replay this executable.

Affected existing checks after hover correction:

- `python3 tests/sv_endpoint_checks.py` — PASS (endpoint-hover.log).
- `SDL_VIDEODRIVER=dummy python3 tests/sv_shell_smoke.py --backend software` — PASS (shell-software-hover.log).

No new test assertions, suites or hover harness were introduced. Actual hover appearance remains part of native visual capture/review.

## Software fractional startup correction — final source freeze

A deterministic existing input replay exposed a production defect: at X11 virtual display scale 1.25, the actual 1280×960 window retained a 1024×768 software render surface. Draw geometry used renderer output while event geometry used current window pixels. Actual Retry at the visible nominal position did not dismiss the failure scene, and hover feedback was missing.

Feedback command: `python3 /tmp/sv-b2-006-retry-red.py` (a narrowed temporary copy of the existing error replay; no permanent test/suite). Before fix it failed `AssertionError: Retry did not change failure scene`. The same replay was run twice with the original executable, including a temporary LD_PRELOAD geometry probe. At display=1.25, density=1, raw Retry=640,618, window size/pixels=1280×960 but renderer output/current=1024×768 throughout the route.

Ranked hypotheses were: stale renderer surface caused by asynchronous startup resize; stale X11 window identity/origin; divergent event/paint coordinate mapping. A single-variable probe using `SDL_VIDEO_SYNC_WINDOW_OPERATIONS=1` turned the replay green and aligned renderer/window dimensions, confirming the first cause. The environment hint is not required by the final implementation.

Minimal production fix: call `SDL_SyncWindow` before `SDL_CreateRenderer` after SV startup window sizing, in endpoint-run.c and the matching synthetic main.c startup. This prevents the software surface from binding to the pre-resize dimensions. It uses the supported SDL 3.2 API; failure follows existing cleanup/error handling. No legacy/shared protocol changes, no new tests/suites. Exact corrective delta: sync-fix.diff.

Rebuilt executable SHA256 `65b6798b679a48c92f9da05195a7d8d3e34485d4145aa58b12b7ea8bad58450a`; full metadata build-fingerprint-sync.txt; build-sync.log PASS. Same original replay with the fixed executable and **without the sync hint** PASS. Geometry probe confirms window/output/current=1280×960. Actual Retry returns to account; failure/account image difference bbox=(258,190,1020,889). Unfocused Back hover is now distinct: image difference bbox=(283,837,408,875). Retained frames/probe logs in scale-diagnosis/; independent capture owner notified to rerun final routes without any instrumentation.

Affected existing endpoint check PASS (endpoint-sync.log); production software shell check PASS (shell-software-sync.log). Temporary probe exists only as an external LD_PRELOAD debug artifact, never compiled into production source/binary. It is removed from /tmp after evidence retention. No debug logs were added to the repository sources. Real physical display timing and OS IME remain outside the virtual display claims.

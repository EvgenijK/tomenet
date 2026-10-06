# Stage A acceptance — ticket 15

The scoped Stage A gate is **accepted** as of ticket 20 (2026-09-23):
**62/62 commands passed**, fresh production checkpoint passed, and current human
review approved. See [final results, hashes and retained evidence](sv-direct3d-startup.md).
Earlier failed runs below remain historical observations.

This checkpoint runs the production
synthetic foundation and registry tooling, and records missing evidence explicitly.
It does not establish live login, gameplay completeness, shipping archives or
actual Windows acceptance. The canonical native claims remain pending.

## Reproduce

From the repository root, with the compiler/SDL dependencies described in
[the shell build instructions](sv-shell.md), Python 3.12+ and a desktop session:

```sh
python3 -m venv /tmp/sv15-venv
/tmp/sv15-venv/bin/pip install -r tools/requirements-capabilities.txt
/tmp/sv15-venv/bin/python -B tools/run_stage_a.py --output /tmp/sv-stage-a-new-run \
  --mingw-sdk /tmp/sv-sdk-current
```

Use a **new** output directory each time. The runner builds Linux amd64 and
MinGW i686 independently, forces separate legacy SDL3 and X11 links despite their shared executable name, runs all four
data suites, four sanitizer suites, legacy HP, and nine native invocations per
Linux renderer (including both TTF and PCF geometry). GUI and LeakSanitizer need
normal desktop/process access; do not disable checks to turn a failure green.
The registry suite can take several minutes; each command has a 900-second limit.
Tests are sequential to avoid competing timing windows.

Timeouts kill the whole check process group before the next check starts.
Each command has a separate full log and a report entry with argv, exit code,
duration and SHA-256. `report.json` is refreshed after each check so interrupted
runs retain their completed observations. A command failure yields exit 1;
automation passing with outstanding human/platform/evidence gates yields exit 2.
Ticket 19 adds exit 0 only when fresh automation and an explicit current human review both pass; see [human review](sv-human-review.md). The output directory
contains the temporary HTML consumer; the sibling project is not synchronized.

The pinned SDK and automatic Wine staging are documented in
[sv-mingw.md](sv-mingw.md). Use a fresh output directory:

```sh
python3 -B tools/run_stage_a.py --output /tmp/sv-stage-a-current \
  --mingw-sdk /tmp/sv-sdk-current
```

The runner builds and stages the current PE with its DLL closure, checks matching
hashes, creates a fresh Wine prefix and runs software/direct3d scenarios. Wine
is intermediate evidence; actual Windows 10/11 checkpoints remain B/E/F.

## What each observation establishes

The tests call production SV decode, session, input, native rendering and response
serialization. No new game implementation or test-only behavior is introduced.
Existing approved scenario/data-tool seams are reused; this reporting task adds
no alternative decoder or model.

| Exact capability ID | Production observations | Limits |
|---|---|---|
| `capability.status.read-hp` | 78 HP native cases / 156 submissions; both wire layouts, fragmented/adjacent inputs, coherent status, geometry and urgent timing | Synthetic peer; no live player session |
| `capability.messages.read-occurrences` | 18 message cases / 54 submissions; repeated bytes remain ordered occurrences; clear, redraw, teardown and mandatory overflow | Short live feed, not full persistent history |
| `capability.request.answer-key` | 40 request cases / 120 submissions; exact signed ID/key reply; lifecycle keeps prompt during HP/messages and macro queue processing | Representative single-key macro profile, not all command contexts |
| `capability.request.cancel-key` | Escape produces baseline zero reply once; server abort and stale generation/input cannot reply; urgent cancellation timing | Owner-specific key-request contract, not universal cancellation |

The lifecycle scenario integrates HP, pairs of identical messages, pending key
request, accepted macro input, six window/session transitions and two exact replies
in one native application. Headless sanitizer checks add ordered macro cases,
fragmentation, abort, session replacement and hard overflow. Native focus events
are deterministically injected into the production handler; resize/minimize/
restore also call SDL window APIs. Automation does not prove human focus behavior.

Geometry uses actual SDL rendering/readback at 1024×768/100%, 1920×1080/100%,
3840×2160/200%, and 1024×768 logical at 125% and 150%, for TTF and PCF. These
are **virtual output-target** checks, not physical monitor/scale acceptance.
Input is keyboard-only; pointer hit bounds are not applicable to this slice.
Timing uses actual monotonic decode/input→successful `SDL_RenderPresent`, excluding
readback, with ≤20 ms urgent and ≤50 ms interactive budgets. The delayed negative
control must fail internally; the wrapper passes only if that rejection occurs.
It is not a successful latency sample. No background flow is implemented.

## Registry and evidence boundary

The gate invokes the production checker with the manifest, native ledger,
`--evidence`, inventory index, reconciliation and current `--source-root` together.
Its full JSON result is `registry.log`; exact manifest/allocation digests, obligation
IDs and checker claims are joined to the scenario log IDs in `report.json`.
The runner also emits eight local candidate records (four outcomes × two renderers),
a separate version 2 pending ledger, and per-outcome reports containing the actual
scenario logs. The production evidence checker reads those exact executable,
configuration, report and dependency fingerprints in `candidate-evidence-check.log`.
These candidates are **rejected**: `runtimeCheck.completed=false` records the
unmeasured runtime condition; `impact=unknown` on the scoped source/fixture/resource
inventories exposes the absent whole-repository dependency closure. The rejection
is a blocking result, not a successful negative test. No acceptance status is
promoted, and no canonical evidence or ledger file is changed. Build/configuration, source/scenario/tool and font
hashes record observation identity, not a complete transitive SDK closure.

The canonical native evidence set and fallback route inventory remain empty. SV currently
links no terminal adapter; no pending B–F flow is secretly allocated to fallback.
The existing `fallback_routes=0` diagnostics are constants, not instrumented
runtime counters. Thus these logs are **not imported as accepted evidence**; the local candidates
keep the missing runtime check explicit rather than substituting the literal.
The missing runtime check and reviewed dependency closure are recorded in
[the improvement list](sv-improvements.md). Checker-linked candidates remain invalid and evidence promotion remains pending;
a successful registry check establishes valid complete allocation, not acceptance.

The local HTML `sync`/`check` runs against the complete canonical manifest and an
honest all-missing consumer. Missing HTML implementation is not a native gate.
The actual sibling `/home/svechnik/Projects/github_site/tomenet_interface` is
**not synchronized** by this task.

## Storage, startup and exclusions

Smoke scenarios verify synthetic opt-in, one SDL system window, fullscreen defaults,
isolated marked profiles, rejection of unmarked directories, resource selection
and TTF→PCF recovery. CFG/OPT and personal legacy files are not loaded or modified
by the synthetic shell. Object trees remain under separate
`.sv-build/linux/<configuration>` and `.sv-build/mingw/<configuration>` roots.

Required protocol/message/input/output overflow is explicit and terminates the
session when correctness cannot be retained; it does not silently discard required
occurrences or accepted input. Session-owned state is cleared at replacement;
preferences/resources have application lifetime. See [lifecycle](sv-lifecycle.md),
[messages](sv-messages.md), and [timing retention](sv-geometry-timing.md).
No stress/soak campaign, session recorder, global memory ceiling or pixel-perfect
raster comparison is introduced. B–F include real connection/account/character,
gameplay/navigation/items, information/social flows, settings/resources/files,
complete platform and package acceptance; their exact allocations remain in the
canonical ledger. Allocation counts are A=8, B=583, C=165, D=156, E=11, F=2.
The other four A outcomes (`capability.platform.linux-build`,
`capability.platform.windows-build`, `capability.platform.software-renderer`,
`capability.platform.one-window`) remain pending too; build/smoke observations
are not full platform evidence.

## Human native review

**Pending: no human approval has been received for this build.** At each actual
OS display scale, launch with a fresh profile (omit `--frames`):

```sh
./src/tomenet-sv --synthetic --profile-root /tmp/sv15-human-new \
  --library "$PWD/lib" --fixture-window 1024x768
```

Confirm startup build/backend/output identity and record the executable hash.
Review readability of status, repeated Echo messages and the prompt; press an
ordinary key, restart and cancel with Escape; move focus away/back and resize
while the prompt is open. Record reviewer, build hash, renderer, OS scale,
observed behavior and perceived response. A screenshot cannot approve these
flows. Visible-response targets are urgent 50 ms, interactive 100 ms and background
250 ms when implemented; submission measurements do not measure human perception.

## Original ticket 15 verification — 2026-09-23

The final run returned **1 / blocked**: **33 of 35 commands passed**. The two
nonpassing commands were MinGW build (missing SDL3/SDL3_ttf/FreeType cross SDK)
and candidate evidence validation (8 incomplete runtime checks and 24 unknown
scoped dependency inventories). These were acceptance blockers at that run; the platform update below supersedes the missing SDK result.
Wine was not run. Human review, physical DPI/monitor movement, physical 4K timing,
Fedora41-class shipping and actual Windows checks remain unverified.

All **27 regression invocations** passed: 57 data tests (12 registry, 13
reconciliation, 17 evidence, 15 HTML), four sanitizer suites, legacy HP and
18 native runs across software/OpenGL. Linux SV compiled with its existing
`-Wall -Wextra -Werror`; both legacy backends were explicitly relinked and passed.
Legacy emitted its existing `tmpnam` linker warning; no legacy source was changed.
Python compilation and diff whitespace checks passed. No sanitizer was disabled.

Canonical validation: **925 active outcomes, 925 pending, 0 accepted**; all
24 inventories / 2077 rows reconciled, no unresolved rows. HTML sync and freshness
check passed with **925 missing**, zero prototype-complete, zero UX-approved,
zero native-accepted claims. Only the temporary local consumer was synchronized.

| Actual renderer | Normal timing samples | Maximum urgent (≤20 ms) | Maximum interactive (≤50 ms) | Normal violations |
|---|---:|---:|---:|---:|
| software | 8 | 6.793 ms | 6.781 ms | 0 |
| opengl | 8 | 3.295 ms | 2.446 ms | 0 |

Each delayed control reported two violations and failed as required. Both renderer
names were confirmed by SDL; native runs used Wayland at actual 1357×1018 output,
OS scale 1.325, from a requested 1024×768 window. OpenGL's name alone does not
identify the GPU/driver or establish physical presentation latency. All five
virtual geometry cases passed for each renderer with both TTF and PCF.

Host: Manjaro Linux, kernel 6.18.49-1-MANJARO x86_64; Clang 22.1.8; SDL 3.4.16,
SDL_ttf 3.2.2, FreeType pkg-config 26.6.20. Linux configuration:
`linux-9dd5a1e98a672a5b25cd` (full metadata retained as `linux-build.txt`).
Executable SHA-256:
`dda366d39f22a64f3d78552b29c76e99d43f462715497b3319983f5527a5b1f1`.

The [machine-readable report](acceptance/stage-a-2026-09-23.json) records all
commands/results, exact allocation/obligation links, runner identity and artifact
fingerprints. The [evidence archive](acceptance/stage-a-2026-09-23.tar.gz) retains
full logs, per-outcome reports, the pending candidate ledger/evidence, build
metadata and the stale-artifact negative check; it contains no client binary or
shipping package. Archive SHA-256:
`3c6bdd7bb14e97d7e04a90084a410f83fff897b98ec5b59a4a6f6a640d0cf1ad`.
All stored log/source fingerprints were verified before publication.

To recheck the archived candidates against current source/build artifacts:

```sh
mkdir -p /tmp/sv15-review
tar -xzf docs/acceptance/stage-a-2026-09-23.tar.gz -C /tmp/sv15-review
/tmp/sv15-venv/bin/python -B tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger /tmp/sv15-review/candidate-ledger.json \
  --evidence /tmp/sv15-review/candidate-evidence.json \
  --source-root tomenet=. --source-root stage-a=/tmp/sv15-review
```

Expected exit **1**, `evidence-runtime` and `evidence-dependencies`; changed or
missing artifacts additionally produce stale/unavailable diagnostics. Appending a
newline to a copied observation report was verified to add `evidence-stale`;
the original report was restored. `observation-mutation.json` retains that result.
These candidate errors are not waived by the passing automated scenarios.

## Standards

Independent review against starting commit
`2f48dadef6a7113170a05957ad07a0b3328a469a` found two runner correctness issues:
shared legacy binary freshness could skip a backend link, and timeouts could
leave descendants running. Both were fixed; final review has **0 residual
findings**. The fresh consumer directory was also corrected after the initial run.

## Spec

Independent review found that report hashes initially bypassed the evidence
checker. Local pending candidates now run through it, preserving rejection of
missing runtime/dependency checks. Final review has **0 residual implementation
findings**; the acceptance blockers above remain open. Review used the explicit
local ticket and parent spec; no remote tracker setup was required.


## Ticket 16 platform update — 2026-09-23

The missing MinGW SDK blocker is resolved by the
[pinned SDK and automatic fresh-PE staging](sv-mingw.md). Final run:
**56/58 commands passed**, exit **1 / blocked**. Linux amd64 and MinGW i686 builds
and configuration isolation passed; the staged PE matches the current build hash.
All **9 Wine software** invocations passed, including exact request replies,
lifecycle, TTF/PCF geometry and submission timing. Wine 11.17 used `winex11.drv`,
SDL `windows`, actual `software` and `direct3d` renderers.

Direct3D passed **8/9** invocations but failed the first prompt submission budget:
**41.398 ms versus 20 ms**. This is an available renderer with a failed timing
gate. Candidate evidence also remains rejected for the same 8 incomplete runtime
checks and 24 dependency inventories assigned to tickets 17–18. Canonical registry
validation passes after re-auditing the diagnostic-only `main.c` fingerprint.
Neither failure is waived; Stage A and actual Windows acceptance remain open.

The [current report](acceptance/stage-a-ticket-16-2026-09-23.json) and
[full logs/metadata archive](acceptance/stage-a-ticket-16-2026-09-23.tar.gz)
supersede the earlier missing-SDK/no-Wine platform observations. Exact binary,
SDK and archive hashes, runtime versions, limitations and review results are in
[the ticket 16 verification record](sv-mingw.md#verification--2026-09-23).
Repeat on the final build after native changes in ticket 17.

## Ticket 18 scoped evidence gate — 2026-09-23

The [ticket 18 evidence report](sv-stage-a-evidence.md#verification--2026-09-23)
replaces the incomplete dependency candidates with 21 fresh production records.
The production checker passes all eight scoped Stage A outcomes, with no
runtime/dependency errors. Full capability acceptance remains 0/925; Wine retains
its intermediate identity, and actual Windows obligations remain B/E/F work.

The [final command report](acceptance/stage-a-ticket-18-2026-09-23.json) records
60/61 passing commands. Linux and required Wine software matrices pass. The
additional Direct3D timing check still fails (27.085 ms / 20 ms), so the full
runner remains blocked; this failure is not hidden by the passed scoped gate.
Human review under ticket 19 is still pending. No automatic approval was generated.

## Ticket 19 manual review preparation

[Human review instructions](sv-human-review.md) describe the isolated launcher,
manual fixture controls, actual display identity, explicit review record and
full gate integration. The user has explicitly approved the listed manual
actions on the final Linux software build. Stage A and ticket 15 remain open
for the retained Wine Direct3D timing failure.

The [ticket 19 final gate](sv-human-review.md#current-result) supersedes prior
build observations: **61/62 commands passed**, all eight scoped A outcomes
covered with fresh evidence. The remaining automatic failure is Wine Direct3D
first urgent submission **37.719 ms / 20 ms**. The manual review is now approved;
the [supplemental assessment](acceptance/stage-a-human-assessment-2026-09-23.json)
records that confirmation without changing the original command results.
See the [current report](acceptance/stage-a-ticket-19-2026-09-23.json) and
[archived evidence](acceptance/stage-a-ticket-19-2026-09-23.tar.gz).

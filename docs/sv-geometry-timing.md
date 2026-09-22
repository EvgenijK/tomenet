# Stage A ticket 06: native geometry and submission timing

The geometry scenario runs the production SV layout, font preparation, message
projection and request presentation at five virtual output geometries. It checks
native pixel regions for status, each displayed message and the pending prompt;
it does not compare against a legacy golden image. The same pending request and
input context survive all transitions while HP and repeated message occurrences
continue. A native key then emits exactly one baseline reply through production
input and serialization.

| Logical size | Output pixels | Scale |
|---|---|---|
| 1024×768 | 1024×768 | 100% |
| 1920×1080 | 1920×1080 | 100% |
| 1920×1080 | 3840×2160 | 200% |
| 1024×768 | 1280×960 | 125% |
| 1024×768 | 1536×1152 | 150% |

These are **virtual DPI render-target checks**, using actual SDL rendering and
readback. They do not certify monitor transitions or physical presentation at
those resolutions. Attempts to request every size as a normal desktop window
were constrained by the compositor, so they were not counted as passing matrix
evidence. The default native shell still uses `SDL_GetWindowDisplayScale` and
the real renderer output size. No automatic layout selection was added. The
implemented request accepts keyboard input only: coordinate-based controls and
hit bounds are not present, so mouse hit testing is explicitly not applicable.
Ticket 05's real window resize/focus/minimize/restore checks remain in the suite.

Both the bundled Cascadia Mono TTF and the declared direct-FreeType PCF fallback
run through this matrix. Text is prepared at final output size and composed 1:1;
PCF uses its existing nearest sampling. There is no scaled whole-window bitmap,
new asset profile, new player preference or legacy modification.

## Timing contract

An optional observer on the production application stamps complete decode
before model application and effects. Local input starts at acceptance, before
macro matching/queue dispatch; rejected or stale input creates no successful
outcome. The input router supplies cancellation meaning, and the session supplies
message classification. Observers cannot re-enter application mutations.

Each coherent application presentation gets a revision. The native timing path
draws that captured view through the normal shell submission function, calls
`SDL_RenderPresent`, then reads `SDL_GetTicksNS`. It acknowledges only events
covered by that snapshot's generation/revision. An older submitted view cannot
acknowledge newer work. Offscreen targets are rejected as timing evidence.
Readback is excluded from the timed path. This measures successful submission,
not scanout, compositor display latency or a human's perceived response.

HP/request/lifecycle projections retain their first outstanding timestamp when
coalesced; promotion never lengthens the deadline. Message occurrences retain
individual timestamps and IDs. A message absent from the submitted feed is
reported as invalid evidence, not falsely credited as visible. Replaced unsubmitted
generations and diagnostic capacity exhaustion are also explicit failures.
Diagnostics retain at most 128 metadata records, no text payloads, pixels or
session archive. Every measured violation is printed individually; the scenario
returns failure if any measured deadline or evidence check fails.

| Outcome exercised | Submission budget |
|---|---|
| HP, request appearance, Escape cancellation, ordinary server messages | ≤20 ms |
| Explicit chat (baseline leading routing marker 253), ordinary local answer | ≤50 ms |
| Background outcomes | ≤200 ms when implemented; none exercised in Stage A |

The scenario checks fragmented input (no premature timer), two coalesced HP
updates, two identical message occurrences, explicit chat, ordinary native input
and Escape cancellation, including exact reply bytes. A separate negative control
submits an old snapshot, waits 65 ms before the second HP update, and delays
cancel dispatch by 25 ms. It must fail the real submission gate and preserve the
first HP deadline. The runner expects this failure; it never treats the delayed
run as performance acceptance. Controlled fixture clocks are not timing evidence.

## Repeatable commands

Run from the repository root, with a working graphical session:

```sh
make -C src -f makefile.sv tomenet-sv
python3 tests/sv_geometry_native.py --backend software
python3 tests/sv_geometry_native.py --backend software --pcf
python3 tests/sv_geometry_native.py --backend opengl
python3 tests/sv_geometry_native.py --backend opengl --pcf
python3 tests/sv_timing_native.py --backend software
python3 tests/sv_timing_native.py --backend opengl
```

The runners verify the **actual** `SDL_GetRendererName` against the requested
backend. Startup also prints video driver, real output size/display scale and
build configuration ID. Temporary marked profiles isolate all fixture writes.
The timing runner's second invocation intentionally exits nonzero internally.

Full existing regression commands (plus the six commands above):

```sh
python3 tests/sv_arch_checks.py
python3 tests/sv_message_checks.py
python3 tests/sv_request_checks.py
python3 tests/sv_lifecycle_checks.py
python3 tests/sv_hp_checks.py --legacy-only
for backend in software opengl; do
    python3 tests/sv_hp_checks.py --backend "$backend"
    python3 tests/sv_arch_native.py --backend "$backend"
    python3 tests/sv_message_native.py --backend "$backend"
    python3 tests/sv_request_native.py --backend "$backend"
    python3 tests/sv_lifecycle_native.py --backend "$backend"
    python3 tests/sv_shell_smoke.py --backend "$backend"
done
```

When the i686 SDL3, SDL3_ttf and FreeType development/runtime dependencies are
available, build and repeat under Wine (intermediate evidence, not Windows):

```sh
make -C src -f makefile.sv tomenet-sv.exe
python3 tests/sv_shell_smoke.py --binary src/tomenet-sv.exe --wine --backend software
python3 tests/sv_geometry_native.py --binary src/tomenet-sv.exe --wine --backend software
python3 tests/sv_geometry_native.py --binary src/tomenet-sv.exe --wine --backend software --pcf
python3 tests/sv_timing_native.py --binary src/tomenet-sv.exe --wine --backend software
```

Repeat with an accelerated backend available in that Wine installation; the runner
must confirm its actual name. Missing dependencies remain unverified, never a
source-audit substitute for execution.

For manual native review at each **actual OS display scale**, use a fresh marked
profile and omit `--frames` to keep the shell open:

```sh
./src/tomenet-sv --synthetic --profile-root /tmp/sv06-manual --library "$PWD/lib" --fixture-window 1024x768 --timing-check
```

At 100% use `--fixture-window 1920x1080`; at 200% that logical size should yield
3840×2160 output, which must be confirmed in the startup line. Fractional 125%
and 150% require corresponding OS settings. The synthetic shell remains visibly
labelled; there is no real session. Existing request/lifecycle scenarios can be
selected with `--request-check --lifecycle-check`. Human readability, focus and
response review remains separate. Visible-response targets are urgent 50 ms,
interactive 100 ms and background 250 ms; no measured visible hard gate is claimed.
No stress/soak, global memory ceiling or pixel-perfect comparison is introduced.

## Verification — 2026-09-22

Linux amd64 build passed `-Wall -Wextra -Werror`. All **23** regression runners
passed after review fixes: four headless ASan/UBSan/LeakSanitizer suites, legacy
HP, twelve existing native runs, four geometry/resource runs and two timing
runners. No sanitizer checks were disabled. Native SDL software and OpenGL names
were confirmed; both used Wayland at actual output 1357×1018 and display scale
1.325 for a requested 1024×768 logical window. Integer output rounding accounts
for the fractional edge. Each virtual matrix passed all five cases with both
TTF and PCF; these are not physical 4K/DPI acceptance.

| Actual SDL backend | Normal samples | Largest urgent interval | Largest interactive interval | Violations |
|---|---:|---:|---:|---:|
| software | 8 | 3.771 ms | 3.652 ms | 0 |
| opengl | 8 | 8.865 ms | 1.273 ms | 0 |

Each backend's deliberate-delay run reported two individual violations and
returned failure as expected. The normal sample counts and intervals are bounded
observations from this run, not a hardware-independent performance guarantee.
Environment: Manjaro Linux, kernel 6.18.49-1-MANJARO x86_64; SDL 3.4.16,
SDL_ttf 3.2.2, FreeType pkg-config version 26.6.20; build configuration
`linux-9dd5a1e98a672a5b25cd`. Build flags/toolchain are captured in the target's
`.sv-build/linux/9dd5a1e98a672a5b25cd/build.txt`. The OpenGL backend name does not
by itself certify a particular GPU or driver acceleration path.

Executable SHA-256:
`072a596dbfaacb1cc2f49b74a41edffc1a948a792550988f4daa447e9725f05c`.
Cascadia Mono SHA-256:
`06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481`.
PCF SHA-256:
`fb72acdee8d41ed41d2dfc3fc769e4a629e02431376f09485d73be4c5e7758b1`.
Changing the implementation/resources invalidates these run-specific observations.
Local temporary logs: `/tmp/sv06-check-01.log` through
`/tmp/sv06-check-23.log`, `/tmp/sv06-build.log`, and `/tmp/sv06-mingw.log`.

MinGW build was attempted and stopped at the missing SDL3/SDL3_ttf/FreeType
development dependency check. Wine smoke (including software), actual Windows,
Fedora41 shipping baseline, physical DPI matrix/monitor movement, timing at a
physical 3840×2160 output, and human UX/visible-response acceptance remain
**unverified**. Ticket implementation and available Linux checks are complete;
the missing environment gates are not marked accepted.

## Standards

Independent review found one ownership issue: message routing classification
was initially in application timing instrumentation. It was moved into the
session change result. Re-review reports **0 residual findings**. The separate
text-cache opportunity is recorded as SV-IMP-005, outside this implementation.

## Spec

Independent review found that cancellation initially inherited the interactive
budget. The input router now exposes cancellation meaning, and both direct and
queued input use the urgent budget. A native regression failed before the fix
and passed afterward, including exact zero reply bytes. Re-review reports
**0 residual implementation findings**; the MinGW/Wine environment limitation
remains explicitly unverified.

Both reviews compared this work to starting commit `3f01c3eff6cd0837e631abecdf4b2ea4cc551d98`
and used the local ticket/spec. `docs/agents/issue-tracker.md` is absent; no remote
tracker or tracker setup was needed for this local implementation.

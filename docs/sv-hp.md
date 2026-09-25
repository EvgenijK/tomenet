# Stage A ticket 02: native HP/status

The synthetic shell displays HP through an SV-local decoder preserving the
legacy `Receive_hp` baseline. Default startup selects server version **4.7.0.2.0.2** and
queues prepared bytes for HP 50/100, boosted, bar requested and drain present.
It displays the normalized values, marker labels and a bounded visual HP bar.
This HP slice adds no live connection; messages and key requests are documented
in their subsequent slice guides.

## Source layout

Production client modules, including local HP decode/apply in `protocol/hp-update.h`,
live in `src/client/sv`. Legacy retains its baseline handlers.
Temporary transport stubs and the synthetic bootstrap live in
`src/temporary/sv`. The bootstrap owns the isolated-profile setup and prepared
startup packet; these are not live connection or persistence implementations.

Native HP fixtures, assertions and pixel-readback checks live in
`tests/sv/scenarios`. The Python runners and legacy regression harness remain
under `tests`. The client entry point only wires the synthetic bootstrap and
optional `--hp-check` / `--arch-check` into the real rendering path.

The makefile lists client, shared core, temporary and scenario objects
separately. This is a source separation, not a new live-client build: the current
synthetic executable still links the temporary adapters and optional scenarios.
Live transport must replace the adapters in a future target; regression
scenarios remain test sources.

## Run and verify

Build/SDK prerequisites and isolated profile rules are described in
[the shell guide](sv-shell.md). From the repository root:

```sh
make -C src -f makefile.sv tomenet-sv
./src/tomenet-sv --synthetic --profile-root /tmp/sv-hp-demo --library lib
python3 tests/sv_hp_checks.py --backend software
python3 tests/sv_hp_checks.py --backend opengl
python3 tests/sv_hp_checks.py --legacy-only
make -C src -f makefile.sdl3 tomenet
```

`--hp-check` runs the prepared peer scenarios in the real SDL window before
returning to the default demonstration. The native check script adds that flag,
an isolated temporary profile, a 1024×768 logical window and a two-frame exit.
It requires desktop access. Neither an offscreen renderer nor a successful
link substitutes for the native check.

For the temporary MinGW SDK documented in the shell guide:

```sh
make -C src -f makefile.sv tomenet-sv.exe PKG_CONFIG_MINGW=/tmp/sv-pkg-config-i686
cp src/tomenet-sv.exe /tmp/tomenet-sv-wine-bin/tomenet-sv.exe
env WINEPREFIX=/tmp/tomenet-sv-wine-prefix WINEDEBUG=-all \
  python3 tests/sv_hp_checks.py --wine \
  --binary /tmp/tomenet-sv-wine-bin/tomenet-sv.exe --backend software
```

## Ownership and production boundary

- The SV-local HP helper uses the unchanged production `Packet_scanf` and version
  predicate. It decodes into temporary values and only publishes a complete
  update. Legacy `Receive_hp` retains its option handling, off-panel damage and
  low-HP alerts, huge-bar rendering, terminal switching and player invalidation.
- The version predicate is copied unchanged into SV `protocol/version.c`, avoiding
  unrelated common game helpers. Legacy builds use the restored definition in
  `common.c`. Compiler-generated dependencies track the local helper headers.
- `SvProtocol` owns negotiated version, two bounded 1024-byte buffers and wire
  recovery. `SvSession` owns the sole player storage, markers and HP revision;
  it accepts decoded semantic changes without SDL or transport access.
- `SvApp` owns both lifetimes, session generation and the owner-thread processing
  loop. `receive` only appends; `step(budget)` applies at most that many complete
  inputs. The shell uses 16 inputs per pass; this initial count budget is not a
  measured frame-time guarantee. WAITING consumes no partial update. Unknown
  recovery ends the pass and drops the remaining batch as in the baseline.
- Each applied HP update advances revision, even if values repeat. Alerts evaluate
  every change before the next input; rendering reads the latest snapshot.
  The alerts module receives explicit options/attention and emits damage sound,
  low-HP sound and low-HP notice in baseline order. The synthetic executor logs
  these results, without a real audio device or personal preferences.
- `SvStatusCache` owns its text and status copy. Generation/revision changes
  reformat text; geometry/scale/font revision changes invalidate layout only.
  UI owns the cache for one application lifetime, and no model pointers survive
  a frame. Rasterization still uses the existing font adapter each draw; this
  slice caches semantic preparation, not a general glyph/texture atlas.
- Oversized input or mandatory output exhaustion closes the session, releases
  its model/protocol storage and leaves a final value snapshot and reason for UI.
  A temporarily full receive buffer returns BACKPRESSURE without consuming input;
  the producer retains it and retries after processing buffered updates.
- Output reports WAITING separately from OUTPUT_TOO_SMALL (including required
  capacity), without consuming data on the latter. `SvOwnedBytes` retains ownership
  on BACKPRESSURE/BUSY; other outcomes release and clear it, including stale results.
  Completions are delivered on the owner thread; no worker pool is implemented.
- Executor failure is visible through `executor_failed` and does not close the
  session. Mutating application calls, including close/destroy, return BUSY during
  effect delivery; callbacks may inspect the whole committed value snapshot.
- Native HP has no terminal route or linkage. Synthetic transport hooks reject
  accidental socket access; byte parsing and serialization remain production code.
  Pixel-readback storage is check-only and released after each frame.

The legacy drain byte is preserved unsigned in the model; a nonzero value
suppresses the legacy off-panel damage alert. It does not suppress the separate
low-HP warning. Synthetic defaults explicitly enable both warning options with off-panel context
false. Scenarios set that context explicitly; its mapping to future UX surfaces
is not inferred from SDL focus/visibility. The legacy alert path is also verified
separately.
The bar flag reflects the packet marker in this slice; legacy additionally
honors its `hp_bar` option as before.

## Checked outcomes

The native scenario covers 78 cases: six independently specified value/marker
fixtures, every boundary from zero bytes through a complete packet, and layouts
selected by **4.7.0.2.0.1** (five bytes) and **4.7.0.2.0.2** (six bytes).
Fixtures include ordinary values, combined markers, boosted negative HP after
death, exact `10000`/`5000` thresholds, the first values beyond those thresholds,
negative maximum/current values and a drain byte of 255.

Incomplete input and empty retries leave the initial view and outputs unchanged.
Completion increments the revision once. A following HP sentinel begins in the
same receive buffer and completes in order, clearing prior markers. Each case
checks independently written model values and expected visible HP text, real
text pixels in the HP region, and successful `SDL_RenderPresent` for both HP
updates: **156 native frame submissions**. The pixel check establishes visible
ink, not pixel-perfect glyph equivalence or human readability approval.

A NULL/unknown packet queues exact baseline `PKT_UNKNOWNPACKET` and redraw bytes,
drops the rest of its input batch, and permits the next batch. Oversized input
fails explicitly without applying it. Fixed-width HP fields have no invalid
numeric range in the baseline, so negative/extreme values are not artificially
rejected. The legacy harness additionally exercises the scanner's defined
invalid-format `-1` return. It compiles the actual `Receive_hp` implementation,
replacing only external terminal/audio sinks, and checks no premature effects,
bar options, boosted-death values, drain versus damage warning behavior and
`PW_PLAYER` invalidation. It deliberately prints one invalid-format diagnostic.

## Verification — 2026-09-20

| Check | Observed result |
|---|---|
| Linux amd64 SV | Build passed, Clang 22.1.8; isolated core objects |
| MinGW i686 SV | Build passed, GCC 16.2.0 with the ticket 01 temporary SDK |
| Linux software | 78 cases / 156 checked native frames, Wayland, actual software renderer, display scale 1.325 |
| Linux OpenGL | 78 cases / 156 checked native frames, Wayland, actual opengl renderer, display scale 1.325 |
| Wine software | 78 cases / 156 checked native frames, SDL windows driver, scale 1.0; intermediate Wine evidence |
| Legacy behavior | Actual Receive_hp regression harness passed, including sound-enabled alert branches |
| Legacy SDL3 build | Full Linux build and link passed with existing sound/archive/curl feature selection |
| Shell regression | All eight existing Linux software smoke cases passed, including PCF fallback and profile isolation |
| Native fallback linkage | No Term/prt_hp/Receive_hp symbols in the Linux SV executable |

Fedora41 runtime/dependency compatibility, actual Windows 10/11, human UX
approval, later lifecycle/geometry matrices and submission timing remain
unverified or assigned to later tickets. No whole-Stage-A acceptance is claimed.

## SV-ARCH-001 checks — 2026-09-21

```sh
python3 tests/sv_arch_checks.py
python3 tests/sv_arch_native.py --backend software
python3 tests/sv_arch_native.py --backend opengl
```

The headless runner compiles real application/protocol/model/alerts/presentation
modules with strict warnings, AddressSanitizer and UndefinedBehaviorSanitizer.
LeakSanitizer needs a non-ptrace environment (the restricted sandbox may require
an approved outside-sandbox run). It tests budgeted updates, intermediate and
repeated warnings, optional sink failure, stale owned completions, retained
backpressure/retry, partial input in both layouts, exact recovery bytes, bounded
output failure, callback reentry rejection and independent presentation invalidation.

The native runner uses `--arch-check`: three one-input passes each submit a real
frame, followed by a session failure frame with visible reason. It leaves that
failed session in place and requires the ordinary shell to submit three further
frames. The old 78 cases / 156 submissions remain a separate unchanged acceptance
count. See [SV-ARCH-001](tasks/SV-ARCH-001-align-current-implementation.md) for scope.

### Verification of the architecture correction — 2026-09-21

Linux SV and full legacy SDL3 builds passed. The complete available matrix passed:
headless ASan/UBSan/LeakSanitizer, legacy HP regression, 78 native HP cases / 156
submissions on each of software and OpenGL, architecture native scenarios on both
backends, and shell/resource smoke checks on both backends. Headless checks and
the affected software native scenarios were repeated after review fixes.
Standards and Spec review findings were fixed and rechecked.

The MinGW compiler exists, but cross SDL3/SDL_ttf/FreeType packages and the old
`/tmp` SDK/Wine setup are absent. This correction has no new Windows/Wine evidence;
the 2026-09-20 verification table above describes the earlier slice only.

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

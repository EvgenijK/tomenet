# Stage A ticket 16: reproducible i686 SDK and Wine observations

`tools/prepare_sv_mingw.py` prepares an isolated SDK without changing system
packages. Python 3.12+, curl, tar support in Python, CMake, pkg-config and an
installed `i686-w64-mingw32-gcc` toolchain are required. Wine and a working desktop
are required for runtime checks. The compiler itself is supplied by the host;
its exact version is recorded, not installed by this tool.

The recipe pins official upstream SDL3 **3.4.0**, SDL3_ttf **3.2.2** MinGW SDKs,
and FreeType **2.13.3** source. Download URLs and SHA-256 digests are in
`PACKAGES` in the script. Cached archives are checked on every use. A mismatch
fails the command; it is never silently replaced. Archive extraction uses Python's
`data` filter. On first preparation the SDK directory must not exist. The recipe builds static FreeType with zlib, bzip2, PNG, HarfBuzz
and Brotli disabled, as in the ticket 01 recipe. The pkg-config wrapper clears
host search paths and selects only the three isolated i686 installations.
`sdk.json` records archive provenance, compiler identity, pkg-config versions and
installed SDK file hashes and the recipe hash. Reuse checks the complete installed
file inventory, wrapper, compiler version and recipe identity; any mismatch fails
and requires a fresh SDK directory. It never reuses a CMake build cache to rebuild
a modified SDK. The SDK directory must stay at its configured path.

```sh
python3 -B tools/prepare_sv_mingw.py --sdk /tmp/sv-sdk-current
make -C src -f makefile.sv tomenet-sv.exe \
  PKG_CONFIG_MINGW=/tmp/sv-sdk-current/pkg-config-i686
```

An optional `--downloads PATH` chooses a persistent verified archive cache;
neither command requires any file from a previous `/tmp` run. For a complete
new run, choose a fresh output directory and invoke the Stage A runner:

```sh
python3 -B tools/run_stage_a.py --output /tmp/sv-stage-a-current \
  --mingw-sdk /tmp/sv-sdk-current
```

The runner prepares/revalidates the SDK, passes its wrapper directly to make,
stages the newly built executable and its recursive DLL imports, checks PE32/i386
for every staged module and verifies the copied executable hash. No user-supplied
staged executable is accepted. `wine-bin/stage.json` records sources, hashes and
imports. Known OS imports stay with Wine/Windows; other DLLs must be found in the
SDK or through the selected i686 compiler, otherwise staging fails. This compiler
requires `SDL3.dll` and `SDL3_ttf.dll`; direct FreeType is static. Different GCC
runtime DLL requirements are resolved and recorded instead of assumed absent.

Every run uses its own Wine prefix and every scenario uses temporary SV profiles.
Wine version, actual SDL video/renderer names, build ID, pointer width and runtime
SDL/SDL_ttf/direct FreeType versions appear in retained logs. The runner explicitly
selects `software`, then `direct3d`, and runs the existing production scenarios
on both, including PCF geometry and the deliberate delayed-frame timing failure.
A DLL/startup/scenario failure fails the gate. Wine observations never count as
actual Windows 10/11 acceptance; those checkpoints remain B/E/F.

Build objects and `build.txt` stay in separate `.sv-build/linux/<key>` and
`.sv-build/mingw/<key>` trees. SV targets do not write `src/tomenet`; the runner
checks its digest across both SV builds before its explicit legacy regression
links. Compiler/options/dependency versions participate in the configuration key.

Runtime counters/dependency evidence completion belong to tickets 17–18. After
native changes in ticket 17, rerun this same command against the final executable;
the ticket 16 observations do not certify a future build.

## Verification — 2026-09-23

The final runner completed **56/58 commands successfully**, exit **1 / blocked**.
All 57 data tests, four sanitizer suites, legacy HP, both legacy build checks,
18 Linux native invocations and all **9 Wine software invocations** passed.
Wine Direct3D passed **8/9** invocations. The failures remain visible:

- Wine Direct3D first prompt submission: **41.398 ms**, urgent budget **20 ms**.
  Earlier isolated runs measured 30–36 ms. This renderer is available; its timing
  gate failed. No failed check was disabled or budget raised.
- Candidate evidence checker: **8 `evidence-runtime`** and **24
  `evidence-dependencies`** errors, the existing tickets 17–18 obligations.
  Canonical registry validation passed; native claims remain pending.

Mandatory software scenarios exercised production HP, ordered messages, exact
key reply/cancel bytes, lifecycle state preservation, shell/resource failures,
TTF and PCF virtual geometry, and actual submission timing. Software's normal
maximum urgent latency was **2.176 ms**, interactive **1.814 ms**; its delayed
negative control still failed with two violations as required. Physical DPI and
4K timing are not established by the virtual geometry fixture.

The request fixture initially failed on Wine: a zero-timestamp pushed key received
the preceding pump cycle's timestamp, older than the new session's input epoch.
Explicitly timestamping the newly generated event fixed all 40 request cases and
120 frame submissions. The production epoch guard and exact reply checks were
preserved. Earlier failing/diagnostic logs are labelled separately in the archive.

| Identity | Observed value |
|---|---|
| Host | Manjaro Linux amd64; kernel 6.18.49-1-MANJARO |
| Linux | Clang 22.1.8; ELF amd64; `linux-9dd5a1e98a672a5b25cd` |
| Linux runtime | SDL 3.4.16, SDL_ttf 3.2.2, direct FreeType 2.14.3; SDL Wayland; software/OpenGL |
| MinGW | GCC 16.2.0; PE32/i386; `mingw-ec38fb15792483700062` |
| MinGW runtime | 32-bit pointers; SDL 3.4.0, SDL_ttf 3.2.2, direct FreeType 2.13.3 |
| Wine | 11.17; DLL trace confirms `winex11.drv`; SDL video `windows`; actual `software` / `direct3d`, scale 1.0 |

SDL's renderer name identifies the backend API; it does not certify a physical
GPU or hardware acceleration. The supplemental Wine driver trace used the same
executable hash as the final runner, in the preceding isolated prefix. Reproduce
that trace with `WINEDEBUG=+loaddll` and a three-frame synthetic startup; the
complete command appears below.

```sh
env WINEPREFIX=/tmp/sv16-final/wine-prefix WINEDEBUG=+loaddll \
  SDL_RENDER_DRIVER=software wine /tmp/sv16-final/wine-bin/tomenet-sv.exe \
  --synthetic --profile-root 'Z:\tmp\sv16-runtime-profile' \
  --library 'Z:\home\svechnik\Projects\tomenet_modern_client\lib' \
  --fixture-window 1024x768 --frames 3
```

Executable SHA-256 values:

- Linux: `9c1406d4b05ba0bf7c8bbb5bbfaf91ad70c8c4f7e5017b41f870c0cedaab576e`.
- MinGW and staged PE: `0f0d2e72c52f577aaead7f9b1b78248697593917f94c65cfb78f6e92d7eadf7d`.

Changing each platform to `-O0 -g3` used separate configuration outputs and left
the other platform and legacy binary unchanged. Restoring `-O2 -g` restored exact
prior binary hashes. Negative CLI checks rejected a corrupt archive, an extra
installed SDK file, and an unresolved DLL despite a same-name DLL in the working
directory. The only inventory update is the reviewed `main.c` diagnostic-change
fingerprint; capability outcomes and allocations are unchanged.

The [report](acceptance/stage-a-ticket-16-2026-09-23.json) records all commands,
return codes, SDK provenance and hashes. The
[evidence archive](acceptance/stage-a-ticket-16-2026-09-23.tar.gz) contains full
logs, configuration metadata, stage/import inventory, source/tool fingerprints,
pending candidate evidence and labelled earlier diagnosis. Its SHA-256 is
`03b6110a88ea0173f95a2b040716a8cf37c218600920fe0a1ade66f085dd9779`.
Every final source/log/staged executable fingerprint was checked before archiving;
`archive-files.json` inventories retained artifacts. No binaries, SDK packages or
Wine prefix are shipped in this observation archive.

## Standards

Independent review against starting commit
`8ce2d70ec199c897301af2a9fe2896d11d3c1f9c` found SDK cache reuse and unresolved GCC
DLL lookup issues. Both were fixed and rechecked; **0 residual findings**.
The timestamp fix preserves the production input boundary and SV isolation.

## Spec

Independent review against local ticket 16 found **0 implementation findings**.
The failed Direct3D timing gate is retained and recorded separately in
[the improvements list](sv-improvements.md#profile-wine-direct3d-first-prompt-submission).
Platform/Stage A acceptance remains blocked; rerun on the final 17–18 build.

Review totals: Standards **0**, Spec **0** residual findings; runtime gates above
remain failures independently of the source review.

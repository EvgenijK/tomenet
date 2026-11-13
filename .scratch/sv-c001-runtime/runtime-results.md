# C001 isolated Fedora41 runtime observations — 2026-11-04

Execution completed; stage acceptance/status не менялись. Archive SHA-256:
`5c03d91101978ae0b1e3cbe3ea4bfb614c9d8a1a90971e98b9aa76ba1e730d5c`.

Exact command from repository root:

```sh
docker run --rm --network none --user 1000:1000 \
  -v /tmp/sv-c001-packages:/packages:ro \
  -v /tmp/sv-c001-results:/results \
  -v /home/svechnik/Projects/tomenet_modern_client/.scratch/sv-c001-runtime/run-extracted.py:/run-extracted.py:ro \
  tomenet-sv-c001-runtime41:local python3 /run-extracted.py \
  /packages/tomenet-sv-4.9.4-linux-amd64.tar.bz2 /results
```

Orchestration exit0. Only package, results, and one-off script mounted; no source
checkout. Network disabled; default metaserver and Chronicle unavailable are
expected, no game/account contact occurred. Artifact extracted into container
`/tmp/sv-c001-observation/unpacked`, then moved to `relocated/installed`; all CLI
calls cwd `/tmp/sv-c001-observation/unrelated`, no `--endpoint`, `--library`,
`TOMENET_PATH` or LD search overrides. HOME/XDG/runtime independent per case.

## Results

| Observation | Actual result | Evidence |
| --- | --- | --- |
| SHA256SUMS of extracted stock files | Every item OK; verifier exit0 | `/tmp/sv-c001-results/inventory-check.log` |
| Default profile, software SDL renderer | exit0, real startup captured; 2 bounded frames | `default-software.log`, `.bmp`, `.png`, `.strace` |
| Default profile, OpenGL renderer | exit0, real startup captured; 2 bounded frames | `default-opengl.log`, `.bmp`, `.png`, `.strace` |
| Bare invocation, no CLI arguments | own PID163/X11 window0x400014 received WM_DELETE_WINDOW; exit0 | `bare-close.log`, `.bmp`, `.png`, `observations.json` |
| Shared user overlay font before bundle | exit0; trace opens overlay Cascadia TTF, bundled login assets; unrelated tomenet.cfg sentinel unchanged | `user-overlay.log`, `.bmp`, `.strace` |
| Explicit root is a file | exit0 via declared bundled fallback; diagnostic says Not a directory; sentinel unchanged | `unavailable-root.log`, `.bmp` |
| Required fonts removed in throwaway package copy | exit1; missing TTF/PCF, explicit fatal resource/startup error; no capture | `missing-required-fonts.log` |
| Forced nonexistent renderer | exit1, explicit renderer unavailable startup failure; no capture | `invalid-renderer.log` |
| Bundled libSDL3.so.0 removed in throwaway copy | exit127; loader says shared library unavailable; runtime image does not supply hidden SDL fallback | `missing-loader-dependency.log` |

All filenames in table are under `/tmp/sv-c001-results/`. `observations.json`
records exact per-case argv/environment/outputs/exit codes. Original archive was
read-only; faults changed only container `/tmp` copies. No login/private profile
writer exercised; root-as-file success is correct overlay fallback, not proof of
C002/003 write/bootstrap. `--frames` cases use existing production render path;
bare-close additionally confirms ordinary close without that argument.

`elf-header.log`, `elf-dynamic.log`, `elf-versions.log`, `ldd.log` record actual
ELF architecture/link/ABI. ldd resolves all non-glibc direct/transitive client
libraries from relocated package, system libc/libm/libresolv/loader from Fedora41.
Runtime strace confirms bundled `libSDL3.so.0`, bundled Cascadia TTF and 17 login
asset opens, user-overlay-first on the overlay case. Zero file opens in
`/home/svechnik` or `/usr/include`; checkout was not mounted.

OpenGL here is **Mesa software GL on Xvfb**, with `LIBGL_ALWAYS_SOFTWARE=1`.
Trace loads system `libGLX_mesa.so.0`, `libgallium-25.0.7.so`, `libLLVM.so.19.1`;
this confirms selected environment runtime driver dependencies beyond ldd.
Physical GPU behavior is not claimed. X11/display/graphics prerequisites belong
to reference runtime rather than bundled SDL closure. No broad B2 visual matrix.

Visual inspection: `bare-close.png` displays the TomeNET production server
selection shell with bundled background/icons, readable font, and expected
«No server list available» because network is disabled. This is startup
confirmation, not a fresh pixel-parity/design acceptance.

## Initial orchestration failure retained

First attempt completed software/opengl but selected a transient hidden SDL
window using PID alone; XSendEvent caused X11 BadWindow. That attempt is retained
at `/tmp/sv-c001-results-first-attempt/`. The one-off script was corrected to
select the matching own PID **and** `TomeNET SV - Server` title, then re-query
before close. Second complete run exited0. This was orchestration behavior,
not a client defect; no production change or extra suite resulted.

BMP captures were losslessly converted to PNG with installed Pillow for viewing;
source BMPs and logs retained. No canonical promotion or automatic accepted mark.

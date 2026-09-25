# Stage A ticket 01: single-window shell

This is a runnable **synthetic development shell**, not a login screen or a
gameplay client. It opens one SDL3 system window and renders logical native
surfaces. [Ticket 02 adds production HP decoding and native status](sv-hp.md).
[SV-ARCH-001](tasks/SV-ARCH-001-align-current-implementation.md) adds separated
protocol/model/application modules, budgeted input processing, semantic alert
results, revision-based status preparation and session-failure recovery in the shell.
[Ticket 03 adds ordered message delivery and a native live feed](sv-messages.md).
Requests and the input router belong to subsequent tickets.
It compiles selected shared core sources into isolated objects and does not link
a terminal, legacy object outputs, Lua generators, historical modern artifacts
or a live network transport.

Temporary synthetic startup and transport adapters are kept in
`src/temporary/sv`, separately from production modules in `src/client/sv`.
Native check scenarios live in `tests/sv/scenarios`; build rules identify each
source group explicitly. The current synthetic executable links these groups
together, while their source ownership and future replacement remain separate.

## Build and launch

From the repository root, with an amd64 compiler and development packages for
SDL3 >= 3.2.0, SDL3_ttf >= 3.2.0 and FreeType:

```sh
make -C src -f makefile.sv tomenet-sv
./src/tomenet-sv --synthetic --profile-root /tmp/tomenet-sv-demo --library lib
```

The profile directory must be new or previously marked by this shell. Existing
unmarked directories are rejected, even if they are empty. Closing the window
or pressing Escape exits. Without `--synthetic`, startup is rejected. No account
or server is needed. `--frames 3` exits after three successful frame submissions;
early close and initialization/rendering failure return nonzero.

Desktop fullscreen and UI scale 100% are product defaults. The explicit
`--fixture-window 1024x768` test override creates a windowed scenario; it is not
a saved preference or an automatic layout switch. Layout uses logical units;
text is rasterized at the display scale and composed at its final pixel size.
The shell's illustrative surface layout is not a claim of approved gameplay UX.

The installation library defaults to `lib/` beside the executable, with
`TOMENET_PATH` or `--library` overriding it. Since development binaries live in
`src/`, use `--library lib` from the repository root. A colocated installation can
place `tomenet-sv` next to the legacy executable and their shared `lib/`.

The production SDL3 identity remains `TomenetGame/tomenet`, with the existing
`TOMENET_SDL3_USER_PATH` override. This synthetic-only executable requires an
explicit isolated root instead of opening the personal default pref path.
`--profile-root` selects that same override U; an existing marked root can also
be selected through `TOMENET_SDL3_USER_PATH`. SV settings ownership is U/sv.
Only the isolation marker and directories are created: CFG/OPT files are neither
loaded nor saved by this shell. Resource overlays are read from U/xtra/font.

The default font is the unmodified Cascadia Mono Regular **v2407.24**, shipped
with its upstream copyright/OFL in `lib/xtra/font`. Its pinned SHA-256 is checked
by the smoke harness. The registered `sv-shell-ascii-v1` profile is implemented
in the font adapter and described in `sv-shell-profiles.json`; it supports the
shell's printable ASCII UI strings only. It does not infer a game-byte charset
or claim a complete game/map glyph profile. Glyph presence and fixed width are
validated on opening TTFs. Unusable overlay → bundled Cascadia → bundled
16x24x PCF; each failure is reported and the requested font remains unchanged.
PCF uses direct FreeType numeric encoding lookup, native strike/bearings and
nearest preparation at final size. If all candidates fail, startup stops.

## Build isolation

`makefile.sv` is independent of the existing makefiles. Each configuration writes
objects, dependency files, executable and generated build metadata exclusively
under `src/.sv-build/<platform>/<configuration-hash>/`. The hash includes compiler
identity, dependency versions, compiler/linker flags and library selection.
The selected executable is then copied to `src/tomenet-sv[.exe]`; no legacy
binary is overwritten. Switching `CFLAGS` builds a separate configuration and
switching back reuses the matching configuration. Build metadata is `build.txt`
alongside the configuration executable.

Linux requires an x86_64 Linux compiler; Windows requires an i686 MinGW compiler.
Override `CC_LINUX`, `CC_MINGW`, `PKG_CONFIG_LINUX`, `PKG_CONFIG_MINGW`, `CFLAGS`,
`CPPFLAGS` and `LDFLAGS` as needed. Reduced shell features are explicit: native
text only; network, audio, image export, archive extraction and guide update are
not implemented here. This is not a final player package. Build on the approved
Fedora41-class builder for shipping; a newer host build is development evidence.

```sh
make -C src -f makefile.sv tomenet-sv CFLAGS='-O0 -g3'
make -C src -f makefile.sv tomenet-sv CFLAGS='-O2 -g'
make -C src -f makefile.sv tomenet-sv.exe
```

## Reproduce the temporary MinGW SDK used for verification

The system MinGW compiler was available but SDL3 development libraries were not.
The checks used official SDL3 3.4.0 and SDL3_ttf 3.2.2 MinGW SDKs plus a static
FreeType 2.13.3 build, entirely below `/tmp/sv-sdk`. No system SDK was installed.
The following commands assume curl, tar, CMake, pkg-config and i686 MinGW GCC.

```sh
mkdir -p /tmp/sv-sdk
curl -fL https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-devel-3.4.0-mingw.tar.gz -o /tmp/sv-sdl3-mingw.tar.gz
curl -fL https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-devel-3.2.2-mingw.tar.gz -o /tmp/sv-ttf-mingw.tar.gz
curl -fL https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-3.tar.gz -o /tmp/sv-freetype.tar.gz
tar -xf /tmp/sv-sdl3-mingw.tar.gz -C /tmp/sv-sdk
tar -xf /tmp/sv-ttf-mingw.tar.gz -C /tmp/sv-sdk
tar -xf /tmp/sv-freetype.tar.gz -C /tmp/sv-sdk
cmake -S /tmp/sv-sdk/freetype-VER-2-13-3 -B /tmp/sv-sdk/freetype-build \
  -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
  -DCMAKE_INSTALL_PREFIX=/tmp/sv-sdk/freetype-i686 -DCMAKE_BUILD_TYPE=Release \
  -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_PNG=ON \
  -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON -DBUILD_SHARED_LIBS=OFF
cmake --build /tmp/sv-sdk/freetype-build -j4
cmake --install /tmp/sv-sdk/freetype-build
```

The upstream SDL_ttf SDK's `.pc` prefix points at its original build directory.
Make that temporary SDK relocatable and select only cross libraries:

```sh
python3 - <<'PY'
from pathlib import Path
p = Path('/tmp/sv-sdk/SDL3_ttf-3.2.2/i686-w64-mingw32/lib/pkgconfig/sdl3-ttf.pc')
s = p.read_text()
p.write_text(s.replace(s.splitlines()[0], 'prefix=${pcfiledir}/../..'))
p = Path('/tmp/sv-pkg-config-i686')
p.write_text('#!/bin/sh\nPKG_CONFIG_LIBDIR=/tmp/sv-sdk/SDL3-3.4.0/i686-w64-mingw32/lib/pkgconfig:/tmp/sv-sdk/SDL3_ttf-3.2.2/i686-w64-mingw32/lib/pkgconfig:/tmp/sv-sdk/freetype-i686/lib/pkgconfig exec pkg-config "$@"\n')
p.chmod(0o755)
PY
make -C src -f makefile.sv tomenet-sv.exe PKG_CONFIG_MINGW=/tmp/sv-pkg-config-i686
mkdir -p /tmp/tomenet-sv-wine-bin /tmp/tomenet-sv-wine-prefix
cp src/tomenet-sv.exe /tmp/tomenet-sv-wine-bin/
cp /tmp/sv-sdk/SDL3-3.4.0/i686-w64-mingw32/bin/SDL3.dll /tmp/tomenet-sv-wine-bin/
cp /tmp/sv-sdk/SDL3_ttf-3.2.2/i686-w64-mingw32/bin/SDL3_ttf.dll /tmp/tomenet-sv-wine-bin/
env WINEPREFIX=/tmp/tomenet-sv-wine-prefix WINEDEBUG=-all wineboot -u
```

Other compiler builds may need their MinGW runtime DLLs too; inspect the PE
imports instead of assuming this host's UCRT compiler closure. This temporary
smoke directory is not a release package. Shipped packages must also include
dependency licenses and pass the later package closure checks.

## Native smoke checks

Run with access to the desktop graphical session. A dummy/offscreen backend does
not supply the required desktop evidence. The harness creates its own temporary
profiles and tests real executables, without touching a personal profile.

```sh
python3 tests/sv_shell_smoke.py --backend software
python3 tests/sv_shell_smoke.py --backend opengl
env WINEPREFIX=/tmp/tomenet-sv-wine-prefix WINEDEBUG=-all \
  python3 tests/sv_shell_smoke.py --wine \
  --binary /tmp/tomenet-sv-wine-bin/tomenet-sv.exe --backend software
env WINEPREFIX=/tmp/tomenet-sv-wine-prefix WINEDEBUG=-all \
  wine /tmp/tomenet-sv-wine-bin/tomenet-sv.exe --synthetic \
  --profile-root 'Z:\tmp\tomenet-sv-wine-default' \
  --library 'Z:\absolute\checkout\lib' --fixture-window 1024x768 --frames 3
```

The final command needs the actual checkout path and no forced
`SDL_RENDER_DRIVER` environment variable. Expected successes report
`windows=1`, the **actual** video/renderer name, scale, requested/effective font,
`submitted_frames=3` and zero terminal fallback routes. The harness expects
nonzero results for missing synthetic opt-in, unmarked profile, nonexistent
renderer and failure of every font candidate. It checks unchanged legacy/SV
settings and TTF→PCF recovery. A successful link alone does not pass a smoke.

## Verification record — 2026-09-20

| Check | Observed result |
|---|---|
| Native build | Clang 22.1.8; ELF amd64 PIE; SDL3 3.4.16, SDL_ttf 3.2.2, FreeType 2.14.3 |
| Cross build | GCC 16.2.0, i686-w64-mingw32; PE32 i386; SDL3 3.4.0, SDL_ttf 3.2.2, FreeType 2.13.3 |
| Linux software | All eight smoke cases passed; Wayland, actual `software`, fullscreen and windowed; OS scale 1.325 |
| Linux accelerated | All eight smoke cases passed; Wayland, actual `opengl`; OS scale 1.325 |
| Wine software | All eight smoke cases passed; Wine 11.17, SDL `windows` video, actual `software`; scale 1.0 |
| Wine automatic backend | Three frames submitted, actual `direct3d`, one window, scale 1.0 |
| Configuration isolation | `-O0 -g3` used a different directory; restoring `-O2 -g` restored the exact prior binary SHA-256; existing `src/tomenet` hash unchanged |
| Legacy shared code | No shared client source or legacy build rule changed, so no legacy rebuild was needed for this ticket |
| Linux baseline | Host is Manjaro with glibc 2.44. Direct executable symbol requirements top out at GLIBC_2.34; this does **not** prove transitive dependency compatibility with Fedora41 |

Fedora41 runtime/dependency closure is **unverified**: no Fedora41 container/VM
or container runtime was available. Actual Windows 10/11 runtime and human UX
approval are also unverified; Wine is intermediate evidence only. Wine emitted
Mesa/EGL driver warnings but the named renderers successfully submitted frames.
No submission-latency, gameplay, full glyph-corpus or full Stage A acceptance is
claimed by these shell checks. There are no terminal fallback routes or linkage.

## Architecture checks

The synthetic startup now queues its HP bytes; the ordinary application pass
processes them with a 16-input budget before rendering. `ui.c` owns drawing and
prepared status, `app.c` owns session lifecycle and module wiring, `protocol.c`
owns buffers/decoding, `session.c` owns semantic state and `alerts.c` owns warning
rules. Production modules contain no pixel assertions or fixture dispatch.

`--arch-check` is a test-only scenario, like `--hp-check`. It leaves a failed
session in place to verify that the normal shell remains alive, displays the
reason and submits the requested frames. Failure of a check itself still returns
nonzero; optional alert executor failure is reported without disconnecting.

```sh
python3 tests/sv_arch_checks.py
python3 tests/sv_arch_native.py --backend software
python3 tests/sv_arch_native.py --backend opengl
```

See [the HP guide](sv-hp.md) for ownership contracts
and checks. Temporary transport and logging alert adapters remain in
`src/temporary/sv`; native fixtures and readbacks remain in `tests/sv/scenarios`.

The 2026-09-21 correction passed Linux SV/legacy builds, headless sanitizer checks,
legacy HP regression and software/OpenGL HP, architecture and shell smoke checks.
Windows/Wine was not rerun: cross dependencies and the previous temporary SDK are
not present. Earlier platform evidence above is historical, not a claim for this revision.

# SV C001 Linux package contract

This is a **pregame-only** package of the current SV production startup. It does
not claim final gameplay, audio, screenshot, archive-extraction or Guide parity.
Creating an archive does not accept C001. Windows remains deferred.

## Build and archive

The shipping baseline is Fedora 41 x86_64 with glibc 2.40. A minimal Linux-only
builder is sufficient: install the production modules named by `makefile.sv`
(SDL3 >= 3.2.0, SDL3_ttf >= 3.2.0, FreeType, libpng, libcurl >= 7.85.0 and
libsecret development packages) and the native build/package tools. Fedora41
SDL3 3.2.16 and SDL3_ttf 3.2.2 satisfy those production minimums. The builder must
provide `git`, `make`, `clang`, `pkg-config`, `python3`, GNU `tar`, `bzip2`,
`binutils` (`readelf`), `ldd` and `patchelf`. The repository's larger
`.github/docker/fedora41/Containerfile` also contains legacy, audio and Windows
dependencies and newer source SDL pins; running that multi-platform builder is
not required for C001. Do not describe the minimal builder as reproducing those
exact pins.
Use the same immutable builder image/digest for reproduction; record its digest
and installed library versions in C001 evidence. This contract names Fedora41,
not an unverified promise for every Linux distribution.

Inside that builder, from the checkout:

```bash
bash tools/package_sv_linux.sh /tmp/sv-c001-packages
```

The script builds the current production `src/tomenet-sv` using `makefile.sv`;
output archives must be outside the checkout. Do not run it concurrently with
another SV build. For a container mount the checkout at `/workspace` and mount a
separate output directory, then run the command inside the container. No legacy
`release-all.sh`, Windows build, release upload or CI change is involved.

For source-installed libraries without RPM ownership (for example SDL3 and SDL3_ttf
in the larger repository builder), supply the corresponding upstream license files in a
separate directory with one child directory per resolved SONAME, for example
`/tmp/sv-licenses/libSDL3.so.0/LICENSE.txt` and
`/tmp/sv-licenses/libSDL3_ttf.so.0/LICENSE.txt`, and set
`SV_UNOWNED_LICENSES=/tmp/sv-licenses`. These are licensing inputs, not resource
lookup directories. Use the license from the actual dependency source version.
RPM dependencies retain package version/license metadata and installed license
files. The script refuses unresolved ELF dependencies or unowned libraries
without explicit license inputs.

`--development-host` explicitly allows a diagnostic build on another amd64
Linux host. It writes `*-linux-amd64-devhost.tar.bz2` and marks `development_host=yes`
in `PACKAGE.txt`. Such an archive does not satisfy Fedora41 shipping evidence.
On Arch/Manjaro it records pacman package/license metadata. This option does not
install tools or dependencies and does not bypass missing-library errors.

## Contents and dependency boundary

The shipping filename is `tomenet-sv-<version>-linux-amd64.tar.bz2`, with a matching
root directory. Version comes from `src/common/defines.h`. Contents:

- Production `tomenet-sv`, with `$ORIGIN` runtime search path.
- Git-tracked stock `lib/` files, including Cascadia and PCF fonts, B2 login fonts,
  backgrounds, icons and doors, and their existing font licenses. Untracked saves,
  local profiles, generated caches, other checkout files and build objects are
  excluded. Tracked resources retain current working-tree contents.
- All resolved transitive ELF libraries except the declared system glibc/loader
  family. Bundled ELF libraries receive `$ORIGIN` search paths. Any unresolved
  `ldd` dependency or conflicting library basename aborts packaging.
- `COPYING`, this `README-package.md`, `licenses/runtime/`,
  `runtime-dependencies.tsv`, `system-dependencies.tsv`, `PACKAGE.txt` and
  `SHA256SUMS`. The latter inventories every other packaged file after RUNPATH
  patching. Archive checksum and build log are siblings of the archive.

The runtime must supply x86_64 Linux, compatible glibc and ELF interpreter
(`/lib64/ld-linux-x86-64.so.2` on Fedora41), and an actual desktop display session.
`PACKAGE.txt` records builder libc/interpreter; `system-dependencies.tsv` names
resolved system libraries. The target runtime must meet symbol-version demands
of the bundled binaries. GPU/vendor drivers and display providers are host
services; `ldd` closure alone does not prove those dynamically loaded providers.
Validate software and available accelerated startup on the declared independent
runtime environment. Secret Service is an optional external provider; copying
libsecret/GLib does not create a persistent vault. Unavailable vault behavior
remains the production session-only contract.

## Launch and profiles

Extract the archive, then run the binary, including from a different current
working directory:

```bash
/path/to/tomenet-sv-<version>-linux-amd64/tomenet-sv
```

The normal launch opens the production startup UI. `--endpoint` remains accepted;
`--synthetic` is a separate diagnostic mode. C001 acceptance uses normal startup,
not synthetic startup. Close the window normally to exit.

The default user root U is `SDL_GetPrefPath("TomenetGame", "tomenet")`, overridden
by `TOMENET_SDL3_USER_PATH` or `--profile-root`. SV settings own U/sv; shared
resources remain in U. Explicit profile directories must already exist. For an
isolated run create a temporary U first and set `TOMENET_SDL3_USER_PATH` to it;
use temporary HOME/XDG paths during acceptance. Never use a real account or profile
for failure scenarios. Do not use the synthetic marker/bootstrap for this path.

Bundled resources resolve beside the executable under `lib/`, independently of
CWD. `TOMENET_PATH` and `--library` explicitly override that directory. User font
resources take precedence according to the existing production font consumer.
No `LD_LIBRARY_PATH`, checkout resource fallback, installation step or root access
is required to run the extracted package. Full settings/persistence and future
resource consumers remain with their allocated stages.

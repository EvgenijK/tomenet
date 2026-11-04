#!/usr/bin/env bash
# SV-only relocatable package. Run in the Fedora41 builder for shipping evidence.
set -euo pipefail
export LC_ALL=C

usage() {
    echo 'Usage: tools/package_sv_linux.sh [--development-host] ABSOLUTE-OUTPUT-DIRECTORY'
    echo 'Default: Fedora 41 amd64 builder. --development-host marks an unaccepted diagnostic package.'
}
fail() { echo "SV package: $*" >&2; exit 1; }
if [[ ${1:-} == --help ]]; then usage; exit 0; fi
development=no
if [[ ${1:-} == --development-host ]]; then development=yes; shift; fi
[[ $# == 1 && $1 == /* ]] || { usage >&2; exit 2; }
repo=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
mkdir -p -- "$1"
output=$(cd -- "$1" && pwd -P)
case "$output/" in "$repo/"*) fail 'output must be outside the checkout';; esac
[[ $(uname -sm) == 'Linux x86_64' ]] || fail 'Linux amd64 is required'
# os-release is trusted local builder metadata, never package input.
source /etc/os-release
[[ $development == yes || ($ID == fedora && $VERSION_ID == 41) ]] ||
    fail 'shipping build requires Fedora 41; use --development-host only for a diagnostic package'
for tool in make git tar bzip2 sha256sum ldd readelf patchelf python3; do
    command -v "$tool" >/dev/null || fail "missing tool: $tool"
done
version=$(python3 - "$repo/src/common/defines.h" <<'PY'
import re, sys
text = open(sys.argv[1]).read()
names = ('MAJOR', 'MINOR', 'PATCH', 'EXTRA', 'BRANCH', 'BUILD')
parts = [int(re.search(r'^#define\s+VERSION_' + name + r'\s+(\d+)', text, re.M)[1]) for name in names]
print('.'.join(map(str, parts if any(parts[3:]) else parts[:3])))
PY
)
name="tomenet-sv-${version}-linux-amd64"
[[ $development != yes ]] || name+="-devhost"
work=$(mktemp -d "$output/.sv-package.XXXXXXXX")
trap 'rm -rf -- "$work"' EXIT
stage="$work/$name"
mkdir -p "$stage/licenses/runtime"
make -C "$repo/src" -f makefile.sv tomenet-sv > "$work/build.log" 2>&1 || {
    cat "$work/build.log" >&2; fail 'production build failed';
}
cp -- "$repo/src/tomenet-sv" "$stage/tomenet-sv"
readelf -h "$stage/tomenet-sv" | grep -q 'Machine:.*X86-64' || fail 'binary must be amd64 ELF'
# Export tracked stock resources only: no local profiles, saves, or generated caches.
git -C "$repo" ls-files -z -- lib | tar -C "$repo" --null -T - -cf - | tar -C "$stage" -xf -
cp -- "$repo/COPYING" "$stage/COPYING"
cp -- "$repo/docs/tasks/c001/package-contract.md" "$stage/README-package.md"
: > "$stage/runtime-dependencies.tsv"
: > "$stage/system-dependencies.tsv"
declare -A copied=() licensed=()
queue=("$stage/tomenet-sv")
for ((index=0; index<${#queue[@]}; index++)); do
    binary=${queue[index]}
    listing=$(ldd "$binary" 2>&1) || fail "ldd failed for $binary: $listing"
    [[ $listing != *'not found'* ]] || fail "unresolved runtime dependency for $binary: $listing"
    while IFS= read -r dependency; do
        [[ -f $dependency ]] || fail "dependency disappeared: $dependency"
        base=$(basename -- "$dependency")
        case $base in
            ld-linux*.so*|libc.so.*|libm.so.*|libdl.so.*|libpthread.so.*|librt.so.*|libresolv.so.*|libnss_*.so.*)
                printf '%s\t%s\n' "$base" "$dependency" >> "$stage/system-dependencies.tsv"
                continue;;
        esac
        if [[ ${copied[$base]+present} ]]; then
            cmp -s -- "${copied[$base]}" "$dependency" || fail "conflicting dependency basename: $base"
            continue
        fi
        copied[$base]=$dependency
        cp -L -- "$dependency" "$stage/$base"
        patchelf --set-rpath '$ORIGIN' "$stage/$base"
        printf '%s\t%s\t%s\n' "$base" "$dependency" "$(sha256sum "$dependency" | cut -d' ' -f1)" >> "$stage/runtime-dependencies.tsv"
        queue+=("$dependency")
        if [[ -n ${SV_UNOWNED_LICENSES:-} && -d $SV_UNOWNED_LICENSES/$base ]]; then
            find "$SV_UNOWNED_LICENSES/$base" -type f -print -quit | grep -q . ||
                fail "empty license input: $SV_UNOWNED_LICENSES/$base"
            cp -aL -- "$SV_UNOWNED_LICENSES/$base" "$stage/licenses/runtime/$base"
        elif command -v rpm >/dev/null; then
            owner=$(rpm -qf --qf '%{NAME}' "$dependency") ||
                fail "unowned dependency $dependency: supply SV_UNOWNED_LICENSES/$base"
            if [[ ! ${licensed[$owner]+present} ]]; then
                licensed[$owner]=yes
                mkdir -p "$stage/licenses/runtime/$owner"
                rpm -q --qf '%{NAME} %{VERSION}-%{RELEASE}\nLicense: %{LICENSE}\n' "$owner" > "$stage/licenses/runtime/$owner/package.txt"
                while IFS= read -r license; do
                    [[ -f $license ]] || continue
                    cp -L --parents -- "$license" "$stage/licenses/runtime/$owner/"
                done < <(rpm -ql "$owner" | sed -n '\|^/usr/share/licenses/|p')
            fi
        elif [[ $development == yes ]] && command -v pacman >/dev/null; then
            owner=$(pacman -Qoq "$dependency") || fail "unowned dependency: $dependency"
            if [[ ! ${licensed[$owner]+present} ]]; then
                licensed[$owner]=yes
                mkdir -p "$stage/licenses/runtime/$owner"
                pacman -Qi "$owner" > "$stage/licenses/runtime/$owner/package.txt"
                [[ ! -d /usr/share/licenses/$owner ]] || cp -aL -- "/usr/share/licenses/$owner" "$stage/licenses/runtime/$owner/"
            fi
        else
            fail 'runtime license inventory requires rpm (or pacman for development host)'
        fi
    done < <(printf '%s\n' "$listing" | awk '/=> \// {print $3} /^[[:space:]]*\/.*ld-linux/ {print $1}')
done
patchelf --set-rpath '$ORIGIN' "$stage/tomenet-sv"
sort -u -o "$stage/system-dependencies.tsv" "$stage/system-dependencies.tsv"
{
    echo 'TomeNET SV C001 — pregame-only; gameplay/final release acceptance pending.'
    echo "package=$name"
    echo "development_host=$development"
    echo "builder=$PRETTY_NAME"
    echo "revision=$(git -C "$repo" rev-parse HEAD)"
    echo "source_changes_sha256=$(git -C "$repo" diff HEAD -- src tests/sv lib | sha256sum | cut -d' ' -f1)"
    echo "packager_sha256=$(sha256sum "$repo/tools/package_sv_linux.sh" | cut -d' ' -f1)"
    echo "libc=$(getconf GNU_LIBC_VERSION)"
    echo "interpreter=$(patchelf --print-interpreter "$stage/tomenet-sv")"
    echo 'System requirements: x86_64 Linux kernel and compatible glibc/loader; display session and GPU drivers for accelerated rendering.'
    echo 'Secret Service is an optional external provider; absence uses the existing session-only policy.'
    echo 'No general all-Linux compatibility or C001 acceptance is asserted by packaging.'
} > "$stage/PACKAGE.txt"
cp -- "$work/build.log" "$output/$name.build.log"
# Inventory is relative to package root and covers every shipped file except itself.
(cd "$stage" && find . -type f ! -name SHA256SUMS -print0 | sort -z | xargs -0 sha256sum > SHA256SUMS)
tar -C "$work" -cjf "$work/$name.tar.bz2" "$name"
mv -- "$work/$name.tar.bz2" "$output/$name.tar.bz2"
(cd "$output" && sha256sum "$name.tar.bz2" > "$name.tar.bz2.sha256")
printf 'SV package: %s\n' "$output/$name.tar.bz2"

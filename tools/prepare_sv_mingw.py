#!/usr/bin/env python3
"""Prepare the pinned, isolated SV i686 SDK. Never installs system packages."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import tarfile

PACKAGES = {
    'sdl': ('https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-devel-3.4.0-mingw.tar.gz',
            '991cb59e28328a3e4598515e181011e5041f5111bae0ec83cca7eedfd25a1f86'),
    'ttf': ('https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-devel-3.2.2-mingw.tar.gz',
            'bb57f26787d6a2e108158562feb061fcdf6f68a110f9c8cf9af42ff343d4e41c'),
    'freetype': ('https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-3.tar.gz',
                 'bc5c898e4756d373e0d991bab053036c5eb2aa7c0d5c67e8662ddc6da40c4103'),
}
SDK_DIRS = ('SDL3-3.4.0/i686-w64-mingw32', 'SDL3_ttf-3.2.2/i686-w64-mingw32', 'freetype-i686')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command):
    print(shlex.join(map(str, command)), flush=True)
    subprocess.run(list(map(str, command)), check=True)


def sdk_files(root):
    files = {str(p.relative_to(root)): digest(p) for d in SDK_DIRS
             for p in sorted((root / d).rglob('*')) if p.is_file()}
    files['pkg-config-i686'] = digest(root / 'pkg-config-i686')
    return files


def prepare(root, downloads):
    downloads.mkdir(parents=True, exist_ok=True)
    # Validate the cache even when reusing an already prepared SDK.
    for name, (url, sha) in PACKAGES.items():
        archive = downloads / (name + '.tar.gz')
        if not archive.exists():
            partial = archive.with_suffix('.part')
            run(['curl', '-fL', '--retry', '2', url, '-o', partial])
            if digest(partial) != sha:
                raise ValueError('Archive checksum mismatch: ' + name)
            partial.replace(archive)
        if digest(archive) != sha:
            raise ValueError('Archive checksum mismatch: ' + name)
    compiler = subprocess.check_output(['i686-w64-mingw32-gcc', '--version'], text=True)
    recipe = digest(Path(__file__))
    if root.exists():
        metadata = json.loads((root / 'sdk.json').read_text())
        current = sdk_files(root)
        if (metadata['files'] != current or metadata['compiler'] != compiler or
                metadata['recipeSha256'] != recipe):
            raise ValueError('SDK changed: choose a fresh --sdk directory')
        print('SV SDK verified:', root / 'pkg-config-i686')
        return
    root.mkdir(parents=True)
    for name in PACKAGES:
        with tarfile.open(downloads / (name + '.tar.gz')) as source:
            source.extractall(root, filter='data')
    build = root / 'freetype-build'
    run(['cmake', '-S', root / 'freetype-VER-2-13-3', '-B', build,
         '-DCMAKE_SYSTEM_NAME=Windows', '-DCMAKE_C_COMPILER=i686-w64-mingw32-gcc',
         '-DCMAKE_INSTALL_PREFIX=' + str(root / 'freetype-i686'), '-DCMAKE_BUILD_TYPE=Release',
         '-DFT_DISABLE_ZLIB=ON', '-DFT_DISABLE_BZIP2=ON', '-DFT_DISABLE_PNG=ON',
         '-DFT_DISABLE_HARFBUZZ=ON', '-DFT_DISABLE_BROTLI=ON', '-DBUILD_SHARED_LIBS=OFF'])
    run(['cmake', '--build', build, '-j4'])
    run(['cmake', '--install', build])
    for directory in SDK_DIRS:
        for pc in (root / directory / 'lib/pkgconfig').glob('*.pc'):
            lines = pc.read_text().splitlines()
            pc.write_text('\n'.join('prefix=${pcfiledir}/../..' if line.startswith('prefix=') else line
                                    for line in lines) + '\n')
    wrapper = root / 'pkg-config-i686'
    directories = ':'.join(str(root / d / 'lib/pkgconfig') for d in SDK_DIRS)
    wrapper.write_text('#!/bin/sh\nunset PKG_CONFIG_PATH PKG_CONFIG_SYSROOT_DIR\nexport PKG_CONFIG_LIBDIR=' +
                       shlex.quote(directories) + '\nexec pkg-config "$@"\n')
    wrapper.chmod(0o755)
    versions = subprocess.check_output([str(wrapper), '--modversion', 'sdl3', 'sdl3-ttf', 'freetype2'], text=True)
    files = sdk_files(root)
    (root / 'sdk.json').write_text(json.dumps({'packages': PACKAGES, 'compiler': compiler,
        'recipeSha256': recipe, 'pkgConfigVersions': versions.splitlines(), 'files': files}, indent=2) + '\n')
    print('SV SDK ready:', wrapper)


def stage(root, binary, destination):
    """Copy the fresh PE and complete non-system DLL closure into a new directory."""
    destination.mkdir(parents=True, exist_ok=False)
    available = {p.name.lower(): p for d in SDK_DIRS for p in (root / d / 'bin').glob('*.dll')}
    # These are Wine/Windows system imports, never copied from a Linux host.
    system = {'kernel32.dll', 'user32.dll', 'gdi32.dll', 'advapi32.dll', 'shell32.dll',
              'ole32.dll', 'oleaut32.dll', 'imm32.dll', 'setupapi.dll', 'version.dll',
              'rpcrt4.dll', 'usp10.dll', 'winmm.dll', 'ws2_32.dll', 'msvcrt.dll', 'ucrtbase.dll', 'dinput8.dll'}
    pending = [binary]
    files = {}
    while pending:
        source = pending.pop()
        name = source.name.lower()
        if name in files:
            continue
        headers = subprocess.check_output(['i686-w64-mingw32-objdump', '-p', str(source)], text=True)
        if 'file format pei-i386' not in headers:
            raise ValueError('Not PE32/i386: ' + str(source))
        imports = [line.split('DLL Name:', 1)[1].strip() for line in headers.splitlines() if 'DLL Name:' in line]
        shutil.copy2(source, destination / source.name)
        files[name] = {'source': str(source), 'sha256': digest(source), 'imports': imports}
        for dependency in imports:
            lower = dependency.lower()
            if lower in system or lower.startswith(('api-ms-win-', 'ext-ms-win-')):
                continue
            found = available.get(lower)
            if found is None:
                compiler_path = subprocess.check_output(
                    ['i686-w64-mingw32-gcc', '-print-file-name=' + dependency], text=True).strip()
                if compiler_path == dependency:
                    raise ValueError('Unresolved runtime DLL: ' + dependency)
                found = Path(compiler_path)
            if not found.is_file():
                raise ValueError('Unresolved runtime DLL: ' + dependency)
            pending.append(found)
    (destination / 'stage.json').write_text(json.dumps(files, indent=2) + '\n')
    if digest(destination / binary.name) != digest(binary):
        raise ValueError('Staged executable differs from build')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--downloads', type=Path)
    parser.add_argument('--stage', type=Path)
    parser.add_argument('--binary', type=Path)
    args = parser.parse_args()
    root = args.sdk.resolve()
    if args.stage:
        if not args.binary:
            parser.error('--stage requires --binary')
        stage(root, args.binary.resolve(), args.stage.resolve())
    else:
        prepare(root, (args.downloads or root.with_name(root.name + '-downloads')).resolve())


if __name__ == '__main__':
    main()

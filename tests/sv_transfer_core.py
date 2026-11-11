#!/usr/bin/env python3
"""One focused check of production SV transfer/startup paths (Linux)."""
from pathlib import Path
import json
import hashlib
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
ENGINE = (
    'lapi lcode ldebug ldo lfunc lgc llex lmem lobject lparser lstate lstring '
    'ltable ltm lundump lvm lzio lauxlib lbaselib ldblib lstrlib'
).split()
with tempfile.TemporaryDirectory(prefix='sv-transfer-core-') as temporary:
    work = Path(temporary)
    compiler = os.environ.get('CC', 'clang')
    common = [compiler, '-std=c99', '-D_DEFAULT_SOURCE', '-O1', '-g',
              '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
              '-Isrc/client/sv', '-Isrc/server/lua']
    objects = []
    commands = []
    sources = ['tests/sv/transfer.c', 'src/client/sv/lua/runtime.c',
               'src/client/sv/lua/reload.c', 'src/client/sv/files/files.c',
               'src/client/sv/lua/constants.c',
               'src/client/sv/transfer/transfer.c',
               'src/client/sv/transfer/startup.c',
               'src/client/sv/protocol/login.c',
               'src/client/sv/settings/options.c',
               'src/client/sv/settings/preferences.c',
               'src/common/md5.c']
    engine_start = len(sources)
    sources += [f'src/server/lua/{name}.c' for name in ENGINE]
    for index, source in enumerate(sources):
        output = work / f'{index}.o'
        flags = ['-Wall', '-Wextra', '-Werror', '-Wno-deprecated-non-prototype'] if index < engine_start else ['-w']
        command = [*common, *flags, '-c', source, '-o', str(output)]
        commands.append(command)
        subprocess.run(command, cwd=ROOT, check=True)
        objects.append(str(output))
    binary = work / 'transfer-core'
    command = [*common, *objects, '-lm', '-o', str(binary)]
    commands.append(command)
    subprocess.run(command, cwd=ROOT, check=True)
    environment = os.environ.copy()
    # Match the existing core-check sanitizer policy; callers can enable LSan.
    # ASan and UBSan remain enabled.
    environment.setdefault('ASAN_OPTIONS', 'detect_leaks=0')
    environment.setdefault('UBSAN_OPTIONS', 'halt_on_error=1')
    command = [str(binary), str(ROOT / 'lib')]
    commands.append(command)
    subprocess.run(command, cwd=ROOT, env=environment, check=True)
    if destination := os.environ.get('SV_TRANSFER_COMMAND_LOG'):
        Path(destination).write_text(json.dumps({
            'commands': commands, 'sourcesSha256': {
                source: hashlib.sha256((ROOT / source).read_bytes()).hexdigest()
                for source in sources
            }, 'ASAN_OPTIONS': environment.get('ASAN_OPTIONS'),
            'UBSAN_OPTIONS': environment.get('UBSAN_OPTIONS'),
            'compiler': subprocess.check_output([compiler, '--version'], text=True).splitlines()[0],
        }, indent=2) + '\n')

#!/usr/bin/env python3
"""Headless checks through the real SV application and presentation interfaces."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='sv-messages-') as temp:
    binary = Path(temp) / 'checks'
    sources = ['tests/sv/messages.c']
    sources += ['src/client/sv/' + name + '.c' for name in
                ('message-text', 'alerts', 'app', 'protocol', 'result', 'session', 'status', 'version')]
    sources += ['src/temporary/sv/peer.c']
    sources += ['src/common/' + name + '.c' for name in ('sockbuf', 'z-util', 'z-form', 'z-virt')]
    subprocess.run([os.environ.get('CC', 'clang'), '-std=c99', '-D_DEFAULT_SOURCE', '-DCLIENT=',
                    '-Wall', '-Wextra', '-Werror', '-Wno-deprecated-non-prototype',
                    '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                    '-Isrc/client/sv', '-O1', '-g', '-fsanitize=address,undefined',
                    *sources, '-o', str(binary)], cwd=ROOT, check=True)
    subprocess.run([str(binary)], cwd=ROOT, check=True)

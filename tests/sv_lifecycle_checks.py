#!/usr/bin/env python3
"""Sanitized lifecycle checks through production Application/input interfaces."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='sv-lifecycle-') as temp:
    binary = Path(temp) / 'checks'
    sources = ['tests/sv/lifecycle.c']
    sources += ['src/client/sv/' + name + '.c' for name in
                ('message-text', 'input', 'alerts', 'app', 'protocol', 'result', 'session', 'status', 'version')]
    sources += ['src/common/' + name + '.c' for name in ('sockbuf', 'z-util', 'z-form', 'z-virt')]
    sources += ['src/temporary/sv/peer.c']
    subprocess.run([os.environ.get('CC', 'clang'), '-std=c99', '-D_DEFAULT_SOURCE', '-DCLIENT=',
                    '-Wall', '-Wextra', '-Werror', '-Wno-deprecated-non-prototype',
                    '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                    '-Isrc/client/sv', '-O1', '-g', '-fsanitize=address,undefined',
                    *sources, '-o', str(binary)], cwd=ROOT, check=True)
    subprocess.run([str(binary)], cwd=ROOT, check=True)

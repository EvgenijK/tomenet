#!/usr/bin/env python3
"""SV-B-002 control packets through the production application."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='sv-contact-') as temp:
    binary = Path(temp) / 'checks'
    sources = ['tests/sv/contact-control.c']
    sources += ['src/client/sv/' + name + '.c' for name in
                ('input/input', 'session/alerts', 'app', 'protocol/protocol', 'result',
                 'session/session', 'protocol/version')]
    sources += ['src/temporary/sv/peer.c']
    sources += ['src/common/' + name + '.c' for name in ('sockbuf', 'z-util', 'z-form', 'z-virt')]
    subprocess.run([os.environ.get('CC', 'clang'), '-std=c99', '-D_DEFAULT_SOURCE', '-DCLIENT=',
                    '-Wall', '-Wextra', '-Werror', '-Wno-deprecated-non-prototype',
                    '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                    '-Isrc/client/sv', '-O1', '-g', '-fsanitize=address,undefined',
                    *sources, '-o', str(binary)], cwd=ROOT, check=True)
    subprocess.run([str(binary)], cwd=ROOT, check=True,
                   env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
    negotiation = Path(temp) / 'negotiation'
    subprocess.run([os.environ.get('CC', 'clang'), '-std=c99', '-Wall', '-Wextra', '-Werror',
                    '-fsanitize=address,undefined', '-Isrc/client/sv',
                    'tests/sv/contact-negotiation.c', 'src/client/sv/protocol/contact.c',
                    '-o', str(negotiation)], cwd=ROOT, check=True)
    subprocess.run([str(negotiation)], cwd=ROOT, check=True,
                   env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))

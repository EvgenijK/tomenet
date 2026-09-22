#!/usr/bin/env python3
"""Actual monotonic decode/input-to-present timings, with a deliberate negative control."""
import argparse
import os
import re
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--binary', type=Path, default=REPO / 'src/tomenet-sv')
p.add_argument('--backend', default='software')
p.add_argument('--wine', action='store_true')
a = p.parse_args()
with tempfile.TemporaryDirectory(prefix='sv-timing-') as directory:
    def native(path):
        path = str(path.resolve())
        return 'Z:' + path.replace('/', '\\') if a.wine else path
    base = (['wine'] if a.wine else []) + [str(a.binary.resolve()), '--synthetic',
        '--profile-root', native(Path(directory) / 'profile'), '--library', native(REPO / 'lib'),
        '--fixture-window', '1024x768', '--timing-check', '--frames', '1']
    for delay in (False, True):
        run = subprocess.run(base + (['--timing-delay'] if delay else []),
            env=dict(os.environ, SDL_RENDER_DRIVER=a.backend), capture_output=True, text=True, timeout=60)
        output = run.stdout + run.stderr
        print(output, end='')
        assert (run.returncode != 0) == delay, run.returncode
        for expected in (f'renderer={a.backend}', 'clock=SDL_GetTicksNS', 'class=urgent',
                         'class=interactive', 'background=unexercised', 'coalesced=2'):
            assert expected in output, expected
        assert ('violation=true' in output) == delay
        assert re.search(r'origin=3 occurrence=2 class=urgent .*budget_ms=20', output), 'cancellation must be urgent'
        assert 'SV timing result' in output
    print('PASS: submission timing and delayed-frame rejection; ' + ('Wine (not Windows)' if a.wine else 'Linux'))

#!/usr/bin/env python3
"""Production request decode, ordered events and native frame checks."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--binary', type=Path, default=REPO / 'src/tomenet-sv')
p.add_argument('--backend', default='software')
p.add_argument('--wine', action='store_true')
a = p.parse_args()

with tempfile.TemporaryDirectory(prefix='sv-request-check-') as directory:
    work = Path(directory)
    def native(path):
        path = str(path.resolve())
        return 'Z:' + path.replace('/', '\\') if a.wine else path
    command = (['wine'] if a.wine else []) + [str(a.binary.resolve()), '--synthetic',
               '--profile-root', native(work / 'profile'), '--library', native(REPO / 'lib'),
               '--fixture-window', '1024x768', '--request-check', '--frames', '2']
    result = subprocess.run(command, env=dict(os.environ, SDL_RENDER_DRIVER=a.backend),
                            capture_output=True, text=True, timeout=60)
    output = result.stdout + result.stderr
    print(output, end='')
    assert result.returncode == 0, result.returncode
    for expected in ('cases=40 submitted_frames=120', 'windows=1',
                     'fallback_routes=0', f'renderer={a.backend}', 'SV exit submitted_frames=2'):
        assert expected in output, expected
    print('PASS: actual native request submissions; ' + ('Wine (not Windows)' if a.wine else 'Linux'))

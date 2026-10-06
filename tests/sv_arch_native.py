#!/usr/bin/env python3
"""Native evidence for budgeted frames and a live shell after session failure."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--backend', default='software')
p.add_argument('--binary', type=Path, default=ROOT / 'src/tomenet-sv')
p.add_argument('--wine', action='store_true')
a = p.parse_args()
with tempfile.TemporaryDirectory(prefix='sv-arch-native-') as directory:
    def native(path):
        value = str(Path(path).resolve())
        return 'Z:' + value.replace('/', '\\') if a.wine else value
    command = (['wine'] if a.wine else []) + [str(a.binary.resolve()), '--synthetic',
        '--profile-root', native(Path(directory) / 'profile'), '--library', native(ROOT / 'lib'),
        '--fixture-window', '1024x768', '--arch-check', '--frames', '3']
    completed = subprocess.run(command, cwd=ROOT, env=dict(os.environ, SDL_RENDER_DRIVER=a.backend),
                               capture_output=True, text=True, timeout=60)
    output = completed.stdout + completed.stderr
    print(output, end='')
    assert completed.returncode == 0, completed.returncode
    for expected in ('SV architecture native passed budget_frames=3 failure_frames=1',
                     'SV exit submitted_frames=3 session_active=0',
                     'Session closed: input exceeds buffer limit', f'renderer={a.backend}'):
        assert expected in output, expected
    print('PASS: budgeted native frames, session failure, responsive shell; ' +
          ('Wine (not Windows)' if a.wine else 'Linux'))

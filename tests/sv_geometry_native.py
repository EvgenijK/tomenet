#!/usr/bin/env python3
"""Bounded native geometry/submission checks; virtual DPI is explicitly labelled."""
import argparse
import os
from pathlib import Path
import subprocess
import shutil
import tempfile

REPO = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--binary', type=Path, default=REPO / 'src/tomenet-sv')
p.add_argument('--backend', default='software')
p.add_argument('--wine', action='store_true')
p.add_argument('--pcf', action='store_true', help='exercise the declared bitmap fallback at every scale')
a = p.parse_args()
with tempfile.TemporaryDirectory(prefix='sv-geometry-') as directory:
    def native(path):
        path = str(path.resolve())
        return 'Z:' + path.replace('/', '\\') if a.wine else path
    library = REPO / 'lib'
    if a.pcf:
        library = Path(directory) / 'fallback-library'
        (library / 'xtra/font').mkdir(parents=True)
        shutil.copyfile(REPO / 'lib/xtra/font/16x24x.pcf', library / 'xtra/font/16x24x.pcf')
    command = (['wine'] if a.wine else []) + [str(a.binary.resolve()), '--synthetic',
        '--profile-root', native(Path(directory) / 'profile'), '--library', native(library),
        '--fixture-window', '1024x768', '--geometry-check', '--frames', '1']
    run = subprocess.run(command, env=dict(os.environ, SDL_RENDER_DRIVER=a.backend),
                         capture_output=True, text=True, timeout=90)
    output = run.stdout + run.stderr
    print(output, end='')
    assert run.returncode == 0, run.returncode
    if a.pcf:
        assert 'effective font fallback:' in output
    for expected in ('windows=1', f'renderer={a.backend}',
                     'logical=1024x768 output=1024x768 scale=1.00',
                     'logical=1920x1080 output=1920x1080 scale=1.00',
                     'logical=1920x1080 output=3840x2160 scale=2.00',
                     'logical=1024x768 output=1280x960 scale=1.25',
                     'logical=1024x768 output=1536x1152 scale=1.50',
                     'SV geometry checks passed cases=5 input=keyboard hit_bounds=not-applicable dpi=virtual'):
        assert expected in output, expected
    print('PASS: native virtual geometry; ' + ('Wine (not Windows)' if a.wine else 'Linux'))

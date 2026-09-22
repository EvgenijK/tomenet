#!/usr/bin/env python3
"""Production HP decoder/model/native frame and legacy presentation regression checks."""
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
p.add_argument('--legacy-only', action='store_true')
a = p.parse_args()

with tempfile.TemporaryDirectory(prefix='sv-hp-check-') as directory:
    work = Path(directory)
    if a.legacy_only:
        sources = ['tests/sv_hp_legacy.c', 'src/client/variable.c', 'src/common/common.c']
        sources += ['src/common/' + name + '.c' for name in ('sockbuf', 'z-util', 'z-form', 'z-virt')]
        command = ['clang', '-std=c99', '-D_DEFAULT_SOURCE', '-DCLIENT', '-O2',
                   '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                   '-Isrc/client', '-Isrc/common', '-Isrc/server', '-Isrc/server/lua']
        subprocess.run(command + sources + ['-o', str(work / 'legacy')], cwd=REPO, check=True)
        subprocess.run([str(work / 'legacy')], check=True)
    else:
        def native(path):
            path = str(path.resolve())
            return 'Z:' + path.replace('/', '\\') if a.wine else path
        command = (['wine'] if a.wine else []) + [str(a.binary.resolve()), '--synthetic',
                   '--profile-root', native(work / 'profile'), '--library', native(REPO / 'lib'),
                   '--fixture-window', '1024x768', '--hp-check', '--frames', '2']
        result = subprocess.run(command, env=dict(os.environ, SDL_RENDER_DRIVER=a.backend),
                                capture_output=True, text=True, timeout=60)
        output = result.stdout + result.stderr
        print(output, end='')
        assert result.returncode == 0, result.returncode
        for expected in ('cases=78 submitted_frames=156', 'windows=1',
                         'fallback_routes=0', f'renderer={a.backend}', 'SV exit submitted_frames=2'):
            assert expected in output, expected
        print('PASS: actual native HP submissions; ' + ('Wine (not Windows)' if a.wine else 'Linux'))

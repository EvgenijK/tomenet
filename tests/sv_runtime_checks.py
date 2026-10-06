#!/usr/bin/env python3
"""Sanitized lifecycle checks through production Application/input interfaces."""
from pathlib import Path
import os
import json
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='sv-runtime-') as temp:
    binary = Path(temp) / 'checks'
    sources = ['tests/sv/runtime.c']
    sources += ['src/client/sv/' + name + '.c' for name in
                ('ui/message-text', 'input/input', 'session/alerts', 'app', 'protocol/protocol', 'result', 'session/session', 'ui/status', 'protocol/version')]
    sources += ['src/common/' + name + '.c' for name in ('sockbuf', 'z-util', 'z-form', 'z-virt')]
    sources += ['src/temporary/sv/peer.c']
    subprocess.run([os.environ.get('CC', 'clang'), '-std=c99', '-D_DEFAULT_SOURCE', '-DCLIENT=',
                    '-Wall', '-Wextra', '-Werror', '-Wno-deprecated-non-prototype',
                    '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections',
                    '-Isrc/client/sv', '-O1', '-g', '-fsanitize=address,undefined',
                    *sources, '-o', str(binary)], cwd=ROOT, check=True)
    result = subprocess.run([str(binary)], cwd=ROOT, check=True, capture_output=True, text=True)
    print(result.stdout, end='')
    # Feed measured production results to the real evidence CLI. Other evidence
    # obligations intentionally remain missing; runtime success must not hide them.
    from sv_evidence_checks import EvidenceChecks
    for measured in (json.loads(line) for line in result.stdout.splitlines() if line.startswith('{')):
        fixture = EvidenceChecks()
        fixture.setUp()
        try:
            record = fixture.candidate()
            record['scenario'] = measured['scenario']
            record['runtimeCheck'] = measured
            _, report = fixture.run_validator('--source-root', f'fixture={fixture.root}',
                                               '--source-root', f'tomenet={ROOT}')
            errors = {e['code'] for e in report['errors']}
            assert ('evidence-runtime' in errors) == (not measured['completed']), report
            assert ('evidence-fallback' in errors) == bool(measured['fallbackEntries']), report
            assert 'evidence-coverage' in errors, report
        finally:
            fixture.doCleanups()

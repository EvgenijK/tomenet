#!/usr/bin/env python3
"""Collector integrity at its public command; uses the real controlled-peer producer."""
import argparse
import contextlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, help='retain focused local evidence in a fresh directory')
args = parser.parse_args()
context = contextlib.nullcontext(str(args.output.resolve())) if args.output else tempfile.TemporaryDirectory(prefix='sv-seal-check-')
with context as temp:
    output = Path(temp) / 'capture'
    command = [sys.executable, '-B', str(ROOT / 'tests/sv_birth_runtime_evidence.py'),
               '--integrity-only', str(output)]
    subprocess.run(command, cwd=ROOT, check=True)
    report = json.loads((output / 'manifest.json').read_text())
    binding = json.loads((ROOT / '.scratch/sandcastle-0241e0da/batch-2.json').read_text())
    check = next(c for c in binding['executionPlan']['artifactChecks'] if c['id'] == 'birth-runtime')
    assert set(report['sources']) == set(check['inputPaths'])
    for name in ('src/common/pack.h', 'src/common/md5.h', 'src/common/defines.h',
                 'src/common/colours.h', 'lib/xtra/font/CascadiaMono-Regular.ttf',
                 'lib/xtra/font/16x24x.pcf'):
        assert name in report['sources'], name
    assert report['inputs_unchanged']
    assert report['suites'][0]['caseCount'] == 3
    verify = [sys.executable, '-B', str(ROOT / 'tests/sv_birth_runtime_evidence.py'),
              '--verify-inputs', str(output)]
    target = ROOT / 'src/common/pack.h'
    original = target.read_bytes()
    try:
        target.write_bytes(original + b'\n/* controlled collector integrity mutation */\n')
        assert subprocess.run(verify, cwd=ROOT).returncode != 0
    finally:
        target.write_bytes(original)
    subprocess.run(verify, cwd=ROOT, check=True)
    manifest = output / 'manifest.json'
    original_manifest = manifest.read_text()
    try:
        report = json.loads(original_manifest)
        report['runtime_before']['tools']['compiler']['version'] += '\ncontrolled identity mutation'
        manifest.write_text(json.dumps(report))
        assert subprocess.run(verify, cwd=ROOT).returncode != 0
    finally:
        manifest.write_text(original_manifest)
    subprocess.run(verify, cwd=ROOT, check=True)
    # A consumed common header omitted from the declaration must stop before capture.
    binding_path = ROOT / '.scratch/sandcastle-0241e0da/batch-2.json'
    original_binding = binding_path.read_text()
    try:
        changed = json.loads(original_binding)
        check = next(c for c in changed['executionPlan']['artifactChecks'] if c['id'] == 'birth-runtime')
        check['inputPaths'].remove('src/common/pack.h')
        binding_path.write_text(json.dumps(changed))
        failure = subprocess.run(command[:-1] + [str(Path(temp) / 'omitted-header')],
                                 cwd=ROOT, capture_output=True, text=True)
        (Path(temp) / 'omitted-header.log').write_text(failure.stdout + failure.stderr)
        assert failure.returncode != 0
        assert 'Consumed repository inputs absent from inventory' in failure.stderr
        assert 'src/common/pack.h' in failure.stderr
    finally:
        binding_path.write_text(original_binding)
    subprocess.run(verify, cwd=ROOT, check=True)
print('Collector closure, production ordinary capture, file and runtime mutation checks passed')

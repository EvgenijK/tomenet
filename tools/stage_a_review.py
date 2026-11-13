#!/usr/bin/env python3
"""Launch a human Stage A review or check an explicit review against candidate evidence.

A passing review check is not Stage A acceptance: the full runner owns that gate.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
import platform
import subprocess
from pathlib import Path
import re
import sys

from stage_a_provenance import capture_host, verify_host

FONTS = ('xtra/font/CascadiaMono-Regular.ttf', 'xtra/font/16x24x.pcf')

ACTIONS = ('readability', 'answer', 'cancel', 'focus', 'resizePending',
           'surfaceRestore', 'macro', 'sessionRestart', 'perceivedResponse')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_review(path, evidence_path):
    result = {'status': 'pending', 'reason': 'No explicit human review supplied.'}
    if path is None:
        return result
    try:
        review = json.loads(path.read_text())
        result.update(reviewSha256=digest(path), evidenceSha256=digest(evidence_path))
        if review['decision'] != 'approved':
            result.update(status='rejected' if review['decision'] == 'rejected' else 'pending',
                          reason='Human review is not approved.')
            return result
        if review['nativeExit'] != 0:
            raise ValueError('Native review did not exit successfully.')
        if review['schemaVersion'] != 1:
            raise ValueError('Unsupported review schema.')
        for key in ('reviewer', 'physicalConditions'):
            if not isinstance(review[key], str) or not review[key].strip():
                raise ValueError('Missing ' + key)
        reviewed = datetime.fromisoformat(review['reviewedAt'])
        started = datetime.fromisoformat(review['startedAt'])
        if (reviewed.tzinfo is None or started.tzinfo is None or reviewed < started or
                reviewed > datetime.now(timezone.utc)):
            raise ValueError('Review requires a real date with timezone, not a future date.')
        for name in ACTIONS:
            action = review['actions'][name]
            if (action['result'] != 'pass' or not isinstance(action['notes'], str) or
                    not action['notes'].strip()):
                raise ValueError('Missing positive observation: ' + name)
        if digest(path.parent / review['hostProvenance']['path']) != review['hostProvenance']['sha256']:
            raise ValueError('Reviewed host dependency artifact changed.')
        log = path.parent / review['log']['path']
        if digest(log) != review['log']['sha256']:
            raise ValueError('Native review log changed.')
        identities = re.findall(r'SV review build=(\S+) video=(\S+) renderer=(\S+) '
                                r'output=(\d+x\d+) logical=([\d.]+x[\d.]+) display_scale=([\d.]+)',
                                log.read_text())
        if not identities or any(row[0] != review['identity']['build'] or
                                 row[2] != review['identity']['renderer'] for row in identities):
            raise ValueError('Missing or mismatched native review geometry/identity.')
        evidence = json.loads(evidence_path.read_text())
        identity = review['identity']
        if set(identity['resources']) != set(FONTS):
            raise ValueError('Incomplete reviewed font resources.')
        if not any(r.get('checkpoint') == 'stage-a' and
                   r['executable']['sha256'] == identity['executableSha256'] and
                   r['hostProvenance']['sha256'] == review['hostProvenance']['sha256'] and
                   identity['build'] == '-'.join(Path(r['configuration']['path']).parts[-3:-1]) and
                   identity['resources'] and any(
                       d['repository'] == 'tomenet' and d['path'] == 'lib' and
                       all(d['files'].get(p) == sha for p, sha in identity['resources'].items())
                       for d in r['dependencies']) and
                   r['environment']['renderer'] == (
                       'accelerated' if identity['renderer'] == 'opengl' else identity['renderer']) and
                   all(r['environment'][key] == identity[key] for key in ('platform', 'osVersion'))
                   for r in evidence['records']):
            raise ValueError('Reviewed executable/environment does not match current evidence.')
        result.update(status='approved', reason='Explicit human review matches candidate evidence.',
                      reviewer=review['reviewer'], reviewedAt=review['reviewedAt'], identity=identity)
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        result.update(status='invalid', reason=str(error))
    return result


def launch(args):
    root = Path(__file__).resolve().parents[1]
    if any(os.environ.get(key) for key in ('LD_PRELOAD', 'LD_LIBRARY_PATH', 'PYTHONPATH')):
        raise ValueError('Review does not permit injected runtime libraries or Python paths.')
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    print('Capturing host dependency identity before manual review...', flush=True)
    host = output / 'host-provenance.json'
    capture_host(host)
    binary = root / 'src/tomenet-sv'
    resources = {name: digest(root / 'lib' / name) for name in FONTS}
    identity = {'platform': 'linux', 'osVersion': platform.platform(),
                'renderer': args.backend, 'executableSha256': digest(binary), 'resources': resources}
    command = [str(binary), '--synthetic', '--review', '--profile-root', str(output / 'profile'),
               '--library', str(root / 'lib'), '--fixture-window', '1024x768']
    record = {'schemaVersion': 1, 'startedAt': datetime.now(timezone.utc).isoformat(),
              'identity': identity, 'command': command, 'reviewer': '', 'reviewedAt': '',
              'hostProvenance': {'path': host.name, 'sha256': digest(host)},
              'decision': 'pending', 'physicalConditions': '',
              'actions': {name: {'result': 'pending', 'notes': ''} for name in ACTIONS}}
    path = output / 'review.json'
    path.write_text(json.dumps(record, indent=2) + '\n')
    print('SHA-256:', identity['executableSha256'], flush=True)
    print('Manual review: F5 restarts session; F6 rebuilds surfaces; m expands to Y.\n'
          'Check ordinary key, Escape cancellation, real desktop focus away/back, resize, minimize/restore.\n'
          'Close the window when finished. Record your actual observations in ' + str(path), flush=True)
    log = output / 'native.log'
    with log.open('w') as stream:
        run = subprocess.run(command, cwd=root, env=dict(os.environ, SDL_RENDER_DRIVER=args.backend),
                             stdout=stream, stderr=subprocess.STDOUT)
    record['nativeExit'] = run.returncode
    try:
        verify_host(host)
    except (OSError, ValueError, KeyError) as error:
        record['nativeExit'] = -1
        record['dependencyError'] = str(error)
    record['log'] = {'path': log.name, 'sha256': digest(log)}
    text = log.read_text()
    builds = set(re.findall(r'SV review build=(\S+)', text))
    record['identity']['build'] = next(iter(builds)) if len(builds) == 1 else ''
    if digest(binary) != identity['executableSha256'] or any(
            digest(root / 'lib' / p) != sha for p, sha in resources.items()):
        record['nativeExit'] = -1
    path.write_text(json.dumps(record, indent=2) + '\n')
    print(text, end='')
    print('Review remains pending. No automatic approval was generated.')
    return 0 if record['nativeExit'] == 0 else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    check = commands.add_parser('check')
    check.add_argument('--review', type=Path)
    check.add_argument('--evidence', type=Path, required=True)
    manual = commands.add_parser('launch')
    manual.add_argument('--output', type=Path, required=True)
    manual.add_argument('--backend', choices=('software', 'opengl'), default='software')
    args = parser.parse_args()
    if args.command == 'launch':
        return launch(args)
    result = check_review(args.review, args.evidence)
    print(json.dumps(result, indent=2))
    return 0 if result['status'] == 'approved' else 2


if __name__ == '__main__':
    sys.exit(main())

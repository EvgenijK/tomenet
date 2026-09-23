#!/usr/bin/env python3
"""Run the existing Stage A checks and retain observations, never certify a platform.

Exit 1: a command failed; exit 2: automation passed but acceptance is pending.
Every invocation needs a new output directory so old successes cannot leak in.
"""
import argparse
import copy
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import signal
import subprocess
import sys
import time

from native_evidence import allocation_digest

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ('hp_checks', 'arch_native', 'message_native', 'request_native',
          'lifecycle_native', 'shell_smoke', 'geometry_native', 'timing_native')
DATA = ('capabilities', 'reconciliation', 'evidence', 'html')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


SCENARIOS = {'hp_checks': 'hp', 'message_native': 'message', 'request_native': 'request',
             'lifecycle_native': 'lifecycle', 'geometry_native': 'geometry', 'timing_native': 'timing'}


def measured_runtime(output, checks, scenario):
    """Compose required production scopes, never infer completion from a log/exit."""
    result = {'scenario': scenario, 'completed': bool(checks), 'fallbackEntries': 0, 'routes': []}
    for check in checks:
        suite = next((s for s in SCENARIOS if check['name'].endswith('-' + s)), None)
        try:
            rows = [json.loads(line) for line in (output / check['runtimeReport']).read_text().splitlines()]
            expected = ['scenario.stage-a.' + SCENARIOS[suite]]
            if suite == 'timing_native':
                expected.append('scenario.stage-a.timing-delayed')
            if [r['scenario'] for r in rows] != expected:
                raise ValueError('Missing, duplicate or unexpected scope')
            for row in rows:
                if (set(row) != {'scenario', 'completed', 'fallbackEntries', 'routes'} or
                    type(row['completed']) is not bool or type(row['fallbackEntries']) is not int or
                    not 0 <= row['fallbackEntries'] <= 0xffffffff or not isinstance(row['routes'], list)):
                    raise ValueError('Invalid runtime fields')
                total = 0
                for route in row['routes']:
                    if (set(route) != {'routeId', 'reason', 'count'} or route['routeId'] != 'route.terminal-handoff' or
                        route['reason'] != 'future-flow' or type(route['count']) is not int or
                        not 1 <= route['count'] <= 0xffffffff):
                        raise ValueError('Invalid route')
                    total += route['count']
                if total != row['fallbackEntries'] or len(row['routes']) > 1:
                    raise ValueError('Inconsistent total')
                # Include negative-control entries too; its intentional incomplete
                # result cannot complete (or substitute for) the positive scope.
                result['fallbackEntries'] += total
                result['routes'].extend(row['routes'])
            result['completed'] &= rows[0]['completed'] and check['result'] == 'pass'
            if suite == 'timing_native' and rows[1]['completed']:
                result['completed'] = False
        except (OSError, ValueError, KeyError, TypeError):
            result['completed'] = False
    if result['routes']:
        result['routes'] = [{'routeId': 'route.terminal-handoff', 'reason': 'future-flow',
                             'count': sum(r['count'] for r in result['routes'])}]
    return result


def write_candidates(output, ledger, report):
    """Bind real observations to checker inputs, explicitly lacking acceptance proof."""
    candidate = copy.deepcopy(ledger)
    candidate['schemaVersion'] = 2
    evidence = {'schemaVersion': 1, 'consumer': 'native-sv', 'records': [], 'fallbackRoutes': []}
    dependencies = []
    for role, directory in (('source', 'src/client/sv'), ('fixture', 'tests/sv'),
                            ('resource', 'lib/xtra/font')):
        path = ROOT / directory
        dependencies.append({'repository': 'tomenet', 'path': directory, 'role': role,
            'impact': 'unknown', 'files': {str(file.relative_to(path)): digest(file)
                for file in sorted(path.rglob('*')) if file.is_file()}})
    (output / 'observations').mkdir()

    def artifact(repository, path, root):
        return {'repository': repository, 'path': str(path.relative_to(root)), 'sha256': digest(path)}

    for observation in report['observations']:
        row = next(r for r in candidate['coverage'] if r['capabilityId'] == observation['capabilityId'])
        row['evidenceIds'] = []
        row['evidenceStatus'] = 'pending'
        for backend in ('software', 'opengl'):
            checks = [check for check in report['checks'] if check['name'] in observation['checks']
                      and check['name'].startswith(backend + '-')]
            if not checks:
                continue
            logs = {check['log']: (output / check['log']).read_text() for check in checks}
            builds = set(re.findall(r'SV startup build=(\S+)', '\n'.join(logs.values())))
            if len(builds) != 1:
                report['limitations'].append('Candidate unavailable: ambiguous/missing build in ' + backend)
                continue
            build = builds.pop()
            config = ROOT / 'src/.sv-build/linux' / build.removeprefix('linux-') / 'build.txt'
            identifier = row['capabilityId'].removeprefix('capability.') + '-' + backend
            result_path = output / 'observations' / (identifier + '.json')
            result_path.write_text(json.dumps({'capabilityId': row['capabilityId'],
                'build': build, 'checks': checks, 'logs': logs,
                'runtimeCheck': measured_runtime(output, checks, 'scenario.stage-a.' + identifier),
                'scope': 'Partial synthetic observations; delayed timing run is a negative control.'}, indent=2) + '\n')
            record = {'id': 'evidence.stage-a.' + identifier, 'kind': 'native-runtime',
                'scenario': 'scenario.stage-a.' + identifier, 'capabilityId': row['capabilityId'],
                'manifestSha256': ledger['manifestSha256'], 'allocationSha256': allocation_digest(row),
                'obligationIds': [o['id'] for o in row['evidenceObligations']],
                'expectedOutcomes': [o['description'] for o in row['evidenceObligations']],
                'actualResult': 'pass' if all(c['result'] == 'pass' for c in checks) else 'fail',
                'environment': {'platform': 'linux', 'osVersion': platform.platform(),
                    'renderer': 'software' if backend == 'software' else 'accelerated',
                    'architecture': platform.machine(),
                    'serverVersion': row['conditions']['serverVersions'][0],
                    'build': row['conditions']['builds'][0]},
                'executable': artifact('tomenet', ROOT / 'src/tomenet-sv', ROOT),
                'configuration': artifact('tomenet', config, ROOT),
                'report': artifact('stage-a', result_path, output),
                'dependencies': dependencies,
                'runtimeCheck': measured_runtime(output, checks, 'scenario.stage-a.' + identifier)}
            row['evidenceIds'].append(record['id'])
            evidence['records'].append(record)
    for name, value in (('candidate-ledger.json', candidate), ('candidate-evidence.json', evidence)):
        (output / name).write_text(json.dumps(value, indent=2) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--mingw-sdk', type=Path,
                        help='prepare pinned SDK here and stage the fresh build for Wine')
    parser.add_argument('--sdk-downloads', type=Path, help='verified archive cache for the pinned SDK')
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    report = {'runnerSha256': digest(Path(__file__)), 'started': datetime.now(timezone.utc).isoformat(),
              'host': platform.platform(), 'python': sys.version,
              'checks': [], 'acceptance': 'pending', 'siblingSynchronized': False,
              'limitations': [
                  'Human native review pending; no automatic approval.',
                  'Physical DPI/monitor transitions and physical 4K timing unverified.',
                  'Actual Windows 10/11 and Fedora41 shipping baseline unverified.',
                  'Synthetic peer only: no live login, complete gameplay or shipping archives.',
                  'Complete evidence dependency closure absent; observations are not accepted evidence.',
                  'B–F capabilities remain pending; no stress/soak, recording or global memory ceiling gate.']}

    def save():
        (output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')

    def run(name, command, timeout=900, env=None):
        start = time.monotonic()
        log = output / (name + '.log')
        runtime = output / (name + '.runtime.jsonl')
        env = dict(os.environ if env is None else env)
        runtime_path = str(runtime)
        if name.startswith('wine-'):
            runtime_path = 'Z:' + runtime_path.replace('/', '\\')
        env['SV_RUNTIME_REPORT'] = runtime_path
        print('RUN', name, flush=True)
        with log.open('w') as stream:
            try:
                with subprocess.Popen(command, cwd=ROOT, env=env, stdout=stream,
                                      stderr=subprocess.STDOUT, start_new_session=True) as process:
                    try:
                        code = process.wait(timeout=timeout)
                    except subprocess.TimeoutExpired:
                        os.killpg(process.pid, signal.SIGKILL)
                        process.wait()
                        raise
            except (OSError, subprocess.TimeoutExpired) as error:
                stream.write(str(error) + '\n')
                code = -1
        report['checks'].append({'name': name, 'command': list(map(str, command)),
            'returncode': code, 'result': 'pass' if code == 0 else 'fail',
            'seconds': round(time.monotonic() - start, 3),
            'log': log.name, 'sha256': digest(log), 'runtimeReport': runtime.name})
        save()
        print('PASS' if code == 0 else 'FAIL', name, flush=True)
        return code == 0

    python = [sys.executable, '-B']
    legacy_path = ROOT / 'src/tomenet'
    legacy_before = digest(legacy_path) if legacy_path.exists() else None
    linux = run('build-linux', ['make', '-C', 'src', '-f', 'makefile.sv', 'tomenet-sv'])
    sdk_ready = False
    cross = ['make', '-C', 'src', '-f', 'makefile.sv', 'tomenet-sv.exe']
    if args.mingw_sdk:
        sdk = args.mingw_sdk.resolve()
        prepare = python + ['tools/prepare_sv_mingw.py', '--sdk', str(sdk)]
        if args.sdk_downloads:
            prepare += ['--downloads', str(args.sdk_downloads.resolve())]
        sdk_ready = run('prepare-mingw-sdk', prepare)
        cross += ['PKG_CONFIG_MINGW=' + str(sdk / 'pkg-config-i686')]
    mingw = run('build-mingw', cross) if not args.mingw_sdk or sdk_ready else False
    if linux and mingw:
        run('build-formats', ['file', 'src/tomenet-sv', 'src/tomenet-sv.exe'])
    legacy_after = digest(legacy_path) if legacy_path.exists() else None
    report['legacyUnchangedBySvBuild'] = legacy_before == legacy_after
    if legacy_before != legacy_after:
        run('sv-build-isolation', python + ['-c', 'raise SystemExit("SV overwrote legacy binary")'])
    if sdk_ready:
        report['mingwSdk'] = json.loads((sdk / 'sdk.json').read_text())
    run('legacy-sdl3', ['make', '-C', 'src', '-f', 'makefile.sdl3', '-W', 'client/main-sdl3.linux.o', 'tomenet'])
    run('legacy-x11', ['make', '-C', 'src', '-f', 'makefile', '-W', 'client/main-x11.o', 'tomenet'])
    for suite in DATA:
        run(suite, python + ['tests/sv_' + suite + '_checks.py'])
    for suite in ('arch', 'message', 'request', 'lifecycle', 'runtime'):
        run(suite + '-headless', python + ['tests/sv_' + suite + '_checks.py'])
    run('runtime-producer', python + ['tests/sv_runtime_producer_checks.py'])
    run('legacy-hp', python + ['tests/sv_hp_checks.py', '--legacy-only'])
    if linux:
        for backend in ('software', 'opengl'):
            for suite in NATIVE:
                run(backend + '-' + suite, python + ['tests/sv_' + suite + '.py', '--backend', backend])
            run(backend + '-geometry-pcf', python + ['tests/sv_geometry_native.py', '--backend', backend, '--pcf'])
    if mingw and sdk_ready:
        staged = output / 'wine-bin'
        staged_ok = run('stage-wine', python + ['tools/prepare_sv_mingw.py', '--sdk', str(sdk),
            '--stage', str(staged), '--binary', str(ROOT / 'src/tomenet-sv.exe')])
        env = dict(os.environ, WINEPREFIX=str(output / 'wine-prefix'), WINEDEBUG='-all')
        run('wine-version', ['wine', '--version'], env=env)
        booted = run('wineboot', ['wineboot', '-u'], env=env)
        if staged_ok and booted:
            report['wineStage'] = json.loads((staged / 'stage.json').read_text())
            for backend in ('software', 'direct3d'):
                for suite in NATIVE:
                    run('wine-' + backend + '-' + suite,
                        python + ['tests/sv_' + suite + '.py', '--backend', backend,
                        '--wine', '--binary', str(staged / 'tomenet-sv.exe')], env=env)
                run('wine-' + backend + '-geometry-pcf', python + ['tests/sv_geometry_native.py',
                    '--backend', backend, '--pcf', '--wine',
                    '--binary', str(staged / 'tomenet-sv.exe')], env=env)
    else:
        report['limitations'].append('Wine unverified: requires successful --mingw-sdk preparation/build.')

    canonical = ROOT / 'docs/capabilities'
    command = python + ['tools/validate_capabilities.py', '--manifest', str(canonical / 'manifest.json'),
        '--ledger', str(canonical / 'native-coverage.json'), '--evidence', str(canonical / 'native-evidence.json'),
        '--inventory-index', str(canonical / 'inventories/index.json'),
        '--reconciliation', str(canonical / 'reconciliation.json'), '--source-root', 'tomenet=' + str(ROOT)]
    run('registry', command)
    consumer = output / 'html-consumer'
    consumer.mkdir()
    for action in ('sync', 'check'):
        run('html-' + action, python + ['tools/html_capabilities.py', action,
            '--canonical', str(canonical / 'manifest.json'), '--consumer', str(consumer),
            '--source-root', 'tomenet=' + str(ROOT)])

    # Map observations to the same exact allocation contract used by the evidence
    # checker, without forging runtime counters or platform/obligation coverage.
    ledger = json.loads((canonical / 'native-coverage.json').read_text())
    try:
        registry = json.loads((output / 'registry.log').read_text())
        report['registry'] = {key: registry.get(key) for key in ('status', 'summary', 'completeness', 'manifestSha256')}
    except (ValueError, OSError):
        registry = {}
        report['registry'] = {'status': 'unavailable'}
    report['stageAllocation'] = dict(Counter(row['acceptanceStage'] for row in ledger['coverage']))
    report['observations'] = []
    suites = {'capability.status.read-hp': ('hp_checks',),
              'capability.messages.read-occurrences': ('message_native',),
              'capability.request.answer-key': ('request_native',),
              'capability.request.cancel-key': ('request_native',)}
    claims = {row['capabilityId']: row for row in registry.get('nativeClaims', [])}
    for row in ledger['coverage']:
        if row['capabilityId'] not in suites:
            continue
        names = suites[row['capabilityId']] + ('lifecycle_native', 'geometry_native', 'timing_native')
        report['observations'].append({'capabilityId': row['capabilityId'],
            'allocationSha256': allocation_digest(row), 'obligations': row['evidenceObligations'],
            'checkerClaim': claims.get(row['capabilityId']),
            'checks': [c['name'] for c in report['checks'] if any(c['name'].endswith(n) for n in names)],
            'scope': 'Partial synthetic observations; no accepted evidence record.'})
    paths = [canonical / name for name in ('manifest.json', 'native-coverage.json', 'native-evidence.json',
             'reconciliation.json', 'inventories/index.json')]
    paths += [ROOT / 'src/tomenet-sv'] if linux else []
    paths += [ROOT / 'src/tomenet-sv.exe'] if mingw else []
    paths += list((ROOT / 'src/.sv-build').glob('*/*/build.txt'))
    paths += [ROOT / 'lib/xtra/font' / name for name in ('CascadiaMono-Regular.ttf', '16x24x.pcf')]
    paths += list((ROOT / 'src/client/sv').rglob('*.c')) + list((ROOT / 'src/client/sv').rglob('*.h'))
    paths += list((ROOT / 'src/temporary/sv').glob('*.[ch]')) + list((ROOT / 'tests/sv').rglob('*.[ch]'))
    paths += list((ROOT / 'tests').glob('sv_*.py')) + list((ROOT / 'tools').glob('*.py'))
    paths += [ROOT / 'src/makefile.sv']
    report['fingerprints'] = {str(path.relative_to(ROOT)): digest(path) for path in sorted(set(paths))}
    report['fingerprintScope'] = 'Observation identity only; not complete transitive SDK/source dependency closure.'
    write_candidates(output, ledger, report)
    candidate_command = python + ['tools/validate_capabilities.py',
        '--manifest', str(canonical / 'manifest.json'),
        '--ledger', str(output / 'candidate-ledger.json'),
        '--evidence', str(output / 'candidate-evidence.json'),
        '--source-root', 'tomenet=' + str(ROOT), '--source-root', 'stage-a=' + str(output)]
    run('candidate-evidence-check', candidate_command)
    report['limitations'].append('Candidate evidence checker rejection is a blocking result, not a passed negative test; canonical claims unchanged.')
    report['finished'] = datetime.now(timezone.utc).isoformat()
    report['failedCommands'] = sum(c['result'] != 'pass' for c in report['checks'])
    report['acceptance'] = 'blocked' if report['failedCommands'] else 'pending'
    save()
    print('Stage A:', report['acceptance'], '; report:', output / 'report.json', flush=True)
    return 1 if report['failedCommands'] else 2


if __name__ == '__main__':
    sys.exit(main())

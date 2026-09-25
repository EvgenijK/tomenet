"""Limited synthetic checkpoint; never grants full capability or human acceptance."""
import itertools
import json

# Explicitly bounded by Stage A spec, not the later shipping/platform allocation.
OUTCOMES = {
    'capability.status.read-hp': ('hp_checks',),
    'capability.messages.read-occurrences': ('message_native',),
    'capability.request.answer-key': ('request_native',),
    'capability.request.cancel-key': ('request_native',),
    'capability.platform.linux-build': ('arch_native', 'shell_smoke'),
    'capability.platform.windows-build': ('arch_native', 'shell_smoke'),
    'capability.platform.software-renderer': ('arch_native', 'shell_smoke'),
    'capability.platform.one-window': ('arch_native', 'shell_smoke'),
}
LIFECYCLE = ('lifecycle_native', 'geometry_native', 'timing_native')


def environments(capability):
    if capability != 'capability.platform.windows-build':
        yield 'linux', 'software'
        yield 'linux', 'accelerated'
    if capability != 'capability.platform.linux-build':
        yield 'wine', 'software'


def suites(capability):
    return OUTCOMES[capability] + LIFECYCLE


def validate_checkpoint(ledger, evidence, roots, report):
    from native_evidence import local_path
    rows = {r['capabilityId']: r for r in ledger['coverage']}
    records = (evidence or {}).get('records', [])
    results = []
    for capability in OUTCOMES:
        row = rows.get(capability)
        covered = set()
        complete = False
        start = len(report.errors)
        if row is None or row['acceptanceStage'] != 'A':
            report.error('checkpoint-scope', capability, 'Missing Stage A allocation')
        else:
            for record in records:
                if record['capabilityId'] != capability or record.get('checkpoint') != 'stage-a':
                    continue
                env = record['environment']
                identity = env['platform'], env['renderer']
                if (identity not in set(environments(capability)) or
                    env['architecture'] != ('i686' if env['platform'] == 'wine' else 'x86_64') or
                    env['serverVersion'] not in row['conditions']['serverVersions'] or
                    env['build'] not in row['conditions']['builds']):
                    report.error('checkpoint-platform', record['id'], 'Not a required Stage A environment')
                    continue
                try:
                    observation = json.loads(local_path(record['report'], roots).read_text())
                    prefix = 'wine-software' if env['platform'] == 'wine' else (
                        'software' if env['renderer'] == 'software' else 'opengl')
                    required = {prefix + '-' + suite for suite in suites(capability)}
                    checks = observation['checks']
                    if ({c['name'] for c in checks} != required or len(checks) != len(required) or
                        any(c['result'] != 'pass' or c['returncode'] != 0 for c in checks) or
                        observation['runtimeCheck'] != record['runtimeCheck'] or
                        observation['capabilityId'] != capability or
                        observation['environment'] != env or
                        observation['executable'] != record['executable'] or
                        observation['configuration'] != record['configuration']):
                        raise ValueError('Incomplete or mismatched scenario report')
                    builds = observation['buildChecks']
                    required_builds = {'build-linux', 'build-mingw', 'build-formats', 'stage-wine'}
                    if ({c['name'] for c in builds} != required_builds or
                        len(builds) != len(required_builds) or
                        any(c['result'] != 'pass' or c['returncode'] != 0 for c in builds) or
                        observation['legacyUnchangedBySvBuild'] is not True):
                        raise ValueError('Incomplete or failed isolated build checks')
                    covered.update((o, identity, env['serverVersion'], env['build'])
                                   for o in record['obligationIds'])
                except (OSError, ValueError, KeyError, TypeError) as error:
                    report.error('checkpoint-report', record['id'], str(error))
            required = set(itertools.product((o['id'] for o in row['evidenceObligations']),
                environments(capability), row['conditions']['serverVersions'], row['conditions']['builds']))
            complete = required <= covered and len(report.errors) == start
            if not required <= covered:
                report.error('checkpoint-coverage', capability, 'Missing Stage A observations')
        results.append({'capabilityId': capability, 'status': 'covered' if complete else 'pending'})
    if report.errors:
        for outcome in results:
            outcome['status'] = 'pending'
    return {'name': 'stage-a', 'status': 'blocked' if report.errors else 'passed',
            'humanReview': 'pending', 'capabilityAcceptance': 'pending', 'outcomes': results}

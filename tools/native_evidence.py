"""Native claim validation. Reads bytes, never executes evidence or game binaries."""
import hashlib
import itertools
import json
from pathlib import Path


def allocation_digest(row):
    """Bind evidence to obligations/conditions without circular claim references."""
    contract = {k: v for k, v in row.items()
                if k not in ('implementation', 'reason', 'evidenceStatus', 'evidenceIds')}
    return hashlib.sha256(json.dumps(contract, sort_keys=True,
                                    separators=(',', ':'), ensure_ascii=True).encode()).hexdigest()


def local_path(reference, roots):
    root = roots.get(reference['repository'])
    if root is None:
        raise OSError('Dependency repository unavailable')
    relative = Path(reference['path'])
    path = root / relative
    if relative.is_absolute() or '..' in relative.parts or not path.resolve().is_relative_to(root):
        raise ValueError('Dependency escapes repository')
    # Reject links rather than silently losing their target from the dependency closure.
    if any(p.is_symlink() for p in (path, *path.parents) if p.is_relative_to(root)):
        raise ValueError('Dependency symlinks are unsupported')
    return path


def check_record(record, row, digest, roots, report, dependency_cache=None):
    owner = record['id']
    start = len(report.errors)
    if record['manifestSha256'] != digest or record['allocationSha256'] != allocation_digest(row):
        report.error('evidence-stale', owner, 'Manifest or allocation changed')
    if record['capabilityId'] != row['capabilityId']:
        report.error('evidence-scope', owner, 'Evidence belongs to another outcome')
    if record.get('checkpoint'):
        from stage_a_checkpoint import OUTCOMES
        if row['acceptanceStage'] != 'A' or row['capabilityId'] not in OUTCOMES:
            report.error('checkpoint-scope', owner, 'Checkpoint is limited to the eight Stage A outcomes')
    obligations = {o['id'] for o in row['evidenceObligations']}
    if not set(record['obligationIds']) <= obligations:
        report.error('evidence-scope', owner, 'Unknown obligation')
    runtime = record['runtimeCheck']
    if runtime['scenario'] != record['scenario'] or not runtime['completed']:
        report.error('evidence-runtime', owner, 'Scenario runtime check incomplete or mismatched')
    if runtime['fallbackEntries'] or runtime['routes']:
        report.error('evidence-fallback', owner, 'Accepted scenario entered fallback')
    if record['actualResult'] != 'pass':
        report.error('evidence-unavailable' if record['actualResult'] == 'unavailable' else 'evidence-failed',
                     owner, 'Native scenario did not pass')
    dependencies = record['dependencies']
    if {d['role'] for d in dependencies} != {'source', 'fixture', 'resource'}:
        report.error('evidence-dependencies', owner, 'Source, fixture and resource inventories required')
    artifacts = [record['executable'], record['configuration'], record['report']]
    if 'hostProvenance' in record:
        artifacts.append(record['hostProvenance'])
    elif record.get('checkpoint'):
        report.error('evidence-dependencies', owner, 'Checkpoint requires host provenance')
    for reference in artifacts:
        try:
            raw = local_path(reference, roots).read_bytes()
            if hashlib.sha256(raw).hexdigest() != reference['sha256']:
                report.error('evidence-stale', owner, 'Artifact bytes changed')
        except OSError:
            report.error('evidence-unavailable', owner, 'Artifact unavailable')
        except ValueError as error:
            report.error('evidence-path', owner, str(error))
    if 'hostProvenance' in record:
        try:
            from stage_a_provenance import verify_host
            path = local_path(record['hostProvenance'], roots)
            cache = dependency_cache if dependency_cache is not None else {}
            key = ('host', str(path))
            if key not in cache:
                verify_host(path)
                cache[key] = True
        except OSError:
            report.error('evidence-unavailable', owner, 'Host provenance unavailable')
        except (ValueError, KeyError, TypeError) as error:
            report.error('evidence-dependencies', owner, str(error))
    for dependency in dependencies:
        try:
            if dependency['impact'] == 'unknown' and dependency['path'] != '.':
                report.error('evidence-dependencies', owner, 'Unknown impact requires repository-wide snapshot')
            path = local_path(dependency, roots)
            from evidence_dependencies import inventory
            cache = dependency_cache if dependency_cache is not None else {}
            key = ('directory', str(path))
            if key not in cache:
                cache[key] = inventory(path)
            if cache[key] != dependency['files']:
                report.error('evidence-stale', owner, 'Dependency added, removed or changed')
        except OSError:
            report.error('evidence-unavailable', owner, 'Dependency tree unavailable')
        except ValueError as error:
            report.error('evidence-path', owner, str(error))
    return len(report.errors) == start


def required_environments(row):
    """Expand baseline platform obligations; no Linux-only claim accepts Windows."""
    for platform, version, build in itertools.product(row['conditions']['platforms'],
                                                      row['conditions']['serverVersions'],
                                                      row['conditions']['builds']):
        for os_version in (('10', '11') if platform == 'windows' else ('linux',)):
            for renderer in ('software', 'accelerated'):
                yield platform, os_version, renderer, version, build


def environment_key(record):
    env = record['environment']
    return (env['platform'], env['osVersion'] if env['platform'] != 'linux' else 'linux',
            env['renderer'], env['serverVersion'], env['build'])


def validate_evidence(registry, ledger, evidence, digest, roots):
    report = registry.report
    evidence = evidence or {'records': [], 'fallbackRoutes': []}
    rows = {r['capabilityId']: r for r in ledger['coverage']}
    records = {}
    for record in evidence['records']:
        if record['id'] in records:
            report.error('evidence-duplicate', record['id'], 'Duplicate evidence ID')
        records[record['id']] = record
    routes = set()
    for route in evidence['fallbackRoutes']:
        owner = route['id']
        if owner in routes:
            report.error('fallback-route', owner, 'Duplicate route ID')
        routes.add(owner)
        row = rows.get(route['capabilityId'])
        registry.reference(route['contextId'], 'context', owner)
        if not row or row['implementation'] != 'fallback' or row['evidenceStatus'] != 'pending':
            report.error('fallback-route', owner, 'Only explicitly allocated development fallback flows may have routes')
        elif route['replacementStage'] < row['acceptanceStage']:
            report.error('fallback-route', owner, 'Replacement precedes flow allocation')
    claims = []
    dependency_cache = {}
    used = set()
    for row in ledger['coverage']:
        owner = row['capabilityId']
        claim = {'capabilityId': owner, 'implementation': row['implementation'], 'status': 'pending'}
        claims.append(claim)
        start = len(report.errors)
        if row['implementation'] == 'fallback' and not any(r['capabilityId'] == owner for r in evidence['fallbackRoutes']):
            report.error('fallback-route', owner, 'Fallback implementation needs a named route')
        accepted = row['evidenceStatus'] == 'accepted'
        if accepted and row['implementation'] != 'native':
            report.error('evidence-unsupported', owner, 'Acceptance requires native implementation')
        if accepted and not row['evidenceIds']:
            report.error('evidence-missing', owner, 'Acceptance requires runtime evidence')
        covered = set()
        for identifier in row['evidenceIds']:
            used.add(identifier)
            record = records.get(identifier)
            if record is None:
                report.error('evidence-missing', owner, 'Referenced evidence is missing')
                continue
            valid = check_record(record, row, digest, roots, report, dependency_cache)
            if accepted and record.get('checkpoint'):
                report.error('evidence-unsupported', owner, 'Checkpoint evidence cannot grant full acceptance')
            if valid and not record.get('checkpoint'):
                covered.update((obligation, environment_key(record)) for obligation in record['obligationIds'])
        if accepted:
            required = set(itertools.product((o['id'] for o in row['evidenceObligations']), required_environments(row)))
            if not required <= covered:
                report.error('evidence-coverage', owner, 'Missing passing obligations/platform/build/version coverage')
        errors = {e['code'] for e in report.errors[start:]}
        if errors:
            claim['status'] = ('unavailable' if 'evidence-unavailable' in errors else
                               'stale' if 'evidence-stale' in errors else 'invalid')
        elif accepted:
            claim['status'] = 'accepted'
    for identifier in records.keys() - used:
        report.error('evidence-unreferenced', identifier, 'Evidence must belong to a current claim')
    by_id = {c['capabilityId']: c for c in claims}
    # Propagate failed/pending children to parents, including long chains.
    changed = True
    while changed:
        changed = False
        for claim in claims:
            if claim['status'] == 'accepted' and any(by_id.get(p, {}).get('status') != 'accepted'
                                                    for p in rows[claim['capabilityId']]['prerequisites']):
                claim['status'] = 'invalid'
                report.error('evidence-prerequisite', claim['capabilityId'], 'Prerequisite outcome is not accepted')
                changed = True
    return claims

#!/usr/bin/env python3
"""Validate and atomically publish a local HTML capability snapshot (no Git/network)."""
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import uuid

from validate_capabilities import Report, Registry, load, source_root, unique_object, validate_sources
from html_claims import evaluate, validate_scope

STORE = '.sv-html'
ARTIFACTS = '.sv-html-artifacts'


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def encoded(value):
    return (json.dumps(value, sort_keys=True, indent=2) + '\n').encode()


def inventory(root):
    """Conservative whole consumer closure; only VCS, store and evidence are excluded."""
    files = {}
    def visit(directory):
        for path in sorted(directory.iterdir()):
            relative = path.relative_to(root).as_posix()
            if relative in ('.git', STORE, ARTIFACTS):
                continue
            if path.is_symlink():
                raise ValueError(f'Consumer symlink is outside the supported inventory contract: {relative}')
            if path.is_dir():
                visit(path)
            elif path.is_file():
                files[relative] = sha(path.read_bytes())
            else:
                raise ValueError(f'Unsupported consumer file: {relative}')
    visit(root)
    return files


def registry(raw, report):
    manifest = json.loads(raw, object_pairs_hook=unique_object)
    report.structure(manifest, 'manifest.schema.json', 'manifest')
    if report.errors:
        return None
    result = Registry(manifest, report)
    result.validate()
    return result


def read_pair(store, report):
    pointer, _ = load(store / 'CURRENT')
    if (not isinstance(pointer, dict) or set(pointer) != {'generation', 'manifestSha256', 'ledgerSha256'}
            or not isinstance(pointer['generation'], str)
            or len(pointer['generation']) != 32
            or any(c not in '0123456789abcdef' for c in pointer['generation'])):
        raise ValueError('Invalid generation pointer')
    directory = store / pointer['generation']
    if directory.is_symlink():
        raise ValueError('Generation must not be a symlink')
    raw = (directory / 'manifest.json').read_bytes()
    ledger_raw = (directory / 'coverage.json').read_bytes()
    if sha(raw) != pointer['manifestSha256'] or sha(ledger_raw) != pointer['ledgerSha256']:
        report.error('pair-digest', 'consumer', 'Published generation bytes do not match CURRENT')
    return raw, json.loads(ledger_raw, object_pairs_hook=unique_object)


def missing(identifier):
    return {'capabilityId': identifier, 'implementation': 'missing',
            'reason': 'New active outcome; no HTML implementation or approval asserted.',
            'mapping': None, 'scenarios': [], 'evidence': [], 'approvals': []}


def validate_ledger(raw, ledger, report):
    reg = registry(raw, report)
    report.structure(ledger, 'html-coverage.schema.json', 'html-ledger')
    if report.errors:
        return None
    if ledger['manifestSha256'] != sha(raw):
        report.error('manifest-digest', 'html', 'Ledger does not identify exact snapshot bytes')
    rows = set()
    active = {c['id'] for c in reg.manifest['capabilities'] if c['lifecycle'] == 'active'}
    for row in ledger['coverage']:
        identifier = row['capabilityId']
        reg.reference(identifier, 'capability', identifier)
        if identifier in rows:
            report.error('duplicate-coverage', identifier, 'Duplicate current outcome')
        rows.add(identifier)
        if identifier not in active:
            report.error('inactive-coverage', identifier, 'Current rows require active outcomes')
        validate_scope(reg, row, report, current=False)
    if active - rows:
        report.error('missing-coverage', 'html', f'Missing active rows: {sorted(active - rows)}')
    return reg


def updated_ledger(raw, reg, files, previous):
    old_rows = {r['capabilityId']: r for r in previous['coverage']} if previous else {}
    history = copy.deepcopy(previous['history']) if previous else []
    if previous:
        retained = {k: v for k, v in previous.items() if k != 'history'}
        if not history or history[-1] != retained:
            history.append(retained)
    return {'schemaVersion': 1, 'consumer': 'html-prototype', 'manifestSha256': sha(raw),
            'sourceInventory': files, 'coverage': [copy.deepcopy(old_rows.get(c['id'], missing(c['id'])))
            for c in reg.manifest['capabilities'] if c['lifecycle'] == 'active'], 'history': history}


def durable_write(path, raw):
    with path.open('xb') as stream:
        stream.write(raw)
        stream.flush()
        os.fsync(stream.fileno())


def fsync_directory(path):
    descriptor = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(descriptor)
    finally:
        os.close(descriptor)


def publish(store, raw, ledger, report, fail_at):
    generation = uuid.uuid4().hex
    directory = store / generation
    directory.mkdir()
    durable_write(directory / 'manifest.json', raw)
    if fail_at == 'after-snapshot':
        raise OSError('Injected failure after snapshot staging')
    ledger_raw = encoded(ledger)
    durable_write(directory / 'coverage.json', ledger_raw)
    validate_ledger((directory / 'manifest.json').read_bytes(), load(directory / 'coverage.json')[0], report)
    if report.errors:
        return
    fsync_directory(directory)
    fsync_directory(store)
    pointer = store / (generation + '.pointer')
    durable_write(pointer, encoded({'generation': generation, 'manifestSha256': sha(raw),
                                    'ledgerSha256': sha(ledger_raw)}))
    if fail_at == 'before-publish':
        raise OSError('Injected failure before pointer replacement')
    if fail_at == 'interrupt-before-publish':
        os._exit(99)  # Fault injection: abrupt process loss, including lock release.
    os.replace(pointer, store / 'CURRENT')
    fsync_directory(store)


def run(args, report):
    store = args.consumer / STORE
    if store.is_symlink():
        raise ValueError('Consumer store must not be a symlink')
    previous_raw = previous = None
    if (store / 'CURRENT').exists():
        previous_raw, previous = read_pair(store, report)
        validate_ledger(previous_raw, previous, report)
    elif args.command == 'check':
        raise FileNotFoundError('No published consumer generation')
    if report.errors:
        return {}
    raw = args.canonical.read_bytes()
    reg = registry(raw, report)
    if reg is None:
        return {}
    validate_sources(reg.manifest, dict(args.source_root), report)
    if previous_raw:
        old = registry(previous_raw, report)
        if old:
            reg.history(old)
    files = inventory(args.consumer)
    if report.errors:
        return {}
    if args.command == 'sync':
        ledger = updated_ledger(raw, reg, files, previous)
        if args.ledger:
            ledger, _ = load(args.ledger)
            validate_ledger(raw, ledger, report)
            if report.errors:
                return {}
            if previous and ledger.get('history') != updated_ledger(raw, reg, files, previous)['history']:
                report.error('claim-history', 'html', 'Candidate must retain the complete prior history')
        if ledger.get('sourceInventory') != files:
            report.error('source-stale', 'html', 'Candidate inventory differs from current consumer files')
        claims = evaluate(reg, sha(raw), ledger, files, args.consumer, report) if not report.errors else []
        if not report.errors:
            publish(store, raw, ledger, report, args.fail_at)
    else:
        ledger = previous
        if previous_raw != raw:
            report.error('snapshot-stale', 'html', 'Canonical bytes changed; run sync')
        if ledger['sourceInventory'] != files:
            report.error('source-stale', 'html', 'Consumer files were added, removed or changed')
        # Evaluate against the published registry when canonical IDs have changed.
        claims = evaluate(registry(previous_raw, Report()), sha(raw), ledger, files, args.consumer, report)
    rows = ledger['coverage']
    complete = sum(c['prototypeComplete'] for c in claims)
    return {'summary': {'active': len(rows), 'missing': sum(r['implementation'] == 'missing' for r in rows),
                        'planned': sum(r['implementation'] == 'planned' for r in rows),
                        'prototypeComplete': complete, 'uxApproved': sum(c['uxApproved'] for c in claims),
                        'staleClaims': sum(c['stale'] for c in claims), 'nativeAccepted': 0},
            'claims': claims, 'prototypeReady': bool(rows) and complete == len(rows) and not report.errors,
            'manifestSha256': sha(raw)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('sync', 'check'))
    parser.add_argument('--canonical', type=Path, required=True)
    parser.add_argument('--consumer', type=Path, required=True)
    parser.add_argument('--source-root', type=source_root, action='append', default=[])
    parser.add_argument('--ledger', type=Path, help='Explicit reviewed candidate ledger for sync')
    parser.add_argument('--fail-at', choices=('after-snapshot', 'before-publish', 'interrupt-before-publish'),
                        help='Sync fault injection; interruption exits 99 without a report')
    args = parser.parse_args()
    report = Report()
    result = {}
    try:
        args.consumer = args.consumer.resolve(strict=True)
        if args.command == 'sync':
            import fcntl
            store = args.consumer / STORE
            if store.is_symlink():
                raise ValueError('Consumer store must not be a symlink')
            store.mkdir(exist_ok=True)
            with (store / 'LOCK').open('a') as lock:
                fcntl.flock(lock, fcntl.LOCK_EX)
                result = run(args, report)
        else:
            if args.ledger or args.fail_at:
                raise ValueError('--ledger and --fail-at require sync')
            result = run(args, report)
    except (OSError, ImportError) as error:
        report.error('unavailable', 'input/environment', str(error))
    except (ValueError, UnicodeError) as error:
        report.error('invalid-input', 'html', str(error))
    unavailable = any(e['code'] in ('unavailable', 'source-unavailable') for e in report.errors)
    status = 'unavailable' if unavailable else 'invalid' if report.errors else 'valid'
    result.update(status=status, freshness='unavailable' if unavailable else 'stale' if report.errors else 'current',
                  errors=report.errors)
    print(json.dumps(result, indent=2))
    return 2 if unavailable else 1 if report.errors else 0


if __name__ == '__main__':
    raise SystemExit(main())

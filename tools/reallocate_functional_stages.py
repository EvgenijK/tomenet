#!/usr/bin/env python3
"""Deterministically migrate the native ledger to functional stages C001-C059."""

import argparse
import copy
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_LEDGER = ROOT / 'docs/capabilities/native-coverage.json'
DEFAULT_STAGES = ROOT / 'docs/capabilities/stages.json'

DEFAULT_STAGE_BY_BLOCK = {
    'account': 'C037', 'admin': 'C040', 'alerts': 'C045', 'audio': 'C050',
    'birth': 'C039', 'character': 'C038', 'chat': 'C027',
    'clipboard': 'C041', 'combat': 'C025', 'configuration': 'C051',
    'credentials': 'C036', 'direction': 'C019', 'documents': 'C030',
    'exports': 'C028', 'files': 'C003', 'fonts': 'C006', 'guide': 'C057',
    'housing': 'C032', 'imports': 'C046', 'information': 'C029',
    'input': 'C005', 'items': 'C021', 'lua': 'C053', 'macros': 'C049',
    'map': 'C015', 'messages': 'C016', 'network': 'C008', 'options': 'C044',
    'os': 'C055', 'platform': 'C052', 'preferences': 'C004',
    'rendering': 'C007', 'request': 'C017', 'screenshots': 'C056',
    'server-flow': 'C034', 'session': 'C031', 'settings': 'C043',
    'skills': 'C022', 'social': 'C042', 'special-store': 'C033',
    'spells': 'C023', 'status': 'C012', 'store': 'C026', 'target': 'C018',
    'transfer': 'C010', 'world': 'C020',
}


def capabilities(block, *names):
    return {f'capability.{block}.{name}' for name in names}


# Every non-default slice is an explicit set. Catalog count validation makes
# newly added outcomes fail closed instead of entering a slice accidentally.
STAGE_OVERRIDES = {
    'C001': capabilities('platform', 'package-launch'),
    'C002': capabilities('settings', 'load', 'defaults', 'parse',
                         'cli-overrides', 'aliases', 'exit-unsaved'),
    'C009': capabilities('lua', 'reload'),
    'C011': capabilities('session', 'load-profile-input',
                         'transfer-startup-files', 'startup-file-failure',
                         'read-identity', 'redraw'),
    'C013': capabilities('world', 'read-map'),
    'C014': capabilities('session', 'enter-game', 'reconnect',
                         'portal-relogin', 'quit'),
    'C024': capabilities('items', 'activate', 'server-spell-answer',
                         'server-spell-cancel', 'use-selected',
                         'cancel-use-selected'),
    'C035': capabilities('store', 'service'),
    'C047': capabilities('preferences', 'macro-precedence'),
    'C048': capabilities('input', 'macro-match', 'macro-wait', 'macro-xwait'),
    'C054': capabilities('files', 'ins-shared'),
    'C058': capabilities('files', 'bookmarks-load', 'bookmarks-save'),
    'C059': capabilities('guide', 'bookmark-set', 'bookmark-open',
                         'bookmark-delete'),
}
EXPLICIT_STAGE_BY_CAPABILITY = {
    capability_id: stage
    for stage, capability_ids in STAGE_OVERRIDES.items()
    for capability_id in capability_ids
}

SESSION_OPTIONAL_PREREQUISITES = {
    'capability.input.macro-match', 'capability.input.macro-wait',
    'capability.input.macro-xwait', 'capability.items.autoinscribe-on-update',
    'capability.macros.load', 'capability.preferences.macro-precedence',
}
SETTINGS_TRANSACTION_PREREQUISITES = capabilities(
    'settings', 'preview', 'save', 'cancel')
TARGETED_PREREQUISITE_REMOVALS = {
    'capability.session.load-profile-input': SESSION_OPTIONAL_PREREQUISITES,
    'capability.rendering.weather': {'capability.audio.weather'},
    'capability.world.walk': capabilities('store', 'enter', 'kicked', 'leave'),
    'capability.world.run': capabilities('store', 'enter', 'kicked', 'leave'),
    'capability.world.open': capabilities('store', 'enter', 'kicked', 'leave'),
    'capability.world.stay': capabilities('items', 'pickup-accept',
                                          'pickup-decline'),
    'capability.world.stay-one': capabilities('items', 'pickup-accept',
                                              'pickup-decline'),
    'capability.store.service': {'capability.screenshots.capture'},
    'capability.chat.send': {'capability.clipboard.paste'},
    'capability.chat.history': {'capability.clipboard.paste'},
    'capability.fonts.text-select': SETTINGS_TRANSACTION_PREREQUISITES,
    'capability.fonts.map-select': SETTINGS_TRANSACTION_PREREQUISITES,
    'capability.fonts.graphics-filter': SETTINGS_TRANSACTION_PREREQUISITES,
    'capability.fonts.pcf-filter': SETTINGS_TRANSACTION_PREREQUISITES,
}
REQUIRED_PREREQUISITES = {
    'capability.input.macro-wait': {'capability.input.macro-match'},
    'capability.input.macro-xwait': {'capability.input.macro-match'},
}


def load_json(path):
    return json.loads(path.read_bytes())


def stage_digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def capability_block(capability_id):
    parts = capability_id.split('.')
    if len(parts) < 3 or parts[0] != 'capability':
        raise ValueError(f'invalid capability ID: {capability_id}')
    return parts[1]


def target_stage(capability_id):
    if capability_id in EXPLICIT_STAGE_BY_CAPABILITY:
        return EXPLICIT_STAGE_BY_CAPABILITY[capability_id]
    try:
        return DEFAULT_STAGE_BY_BLOCK[capability_block(capability_id)]
    except KeyError as error:
        raise ValueError(f'no functional-stage rule for {capability_id}') from error


def corrected_prerequisites(row):
    removals = set(TARGETED_PREREQUISITE_REMOVALS.get(row['capabilityId'], ()))
    if capability_block(row['capabilityId']) not in ('input', 'macros'):
        # Macro matching is an alternate invocation path, not a prerequisite of
        # the ordinary semantic action that the macro invokes.
        removals.add('capability.input.macro-match')
    before = row['prerequisites']
    after = [item for item in before if item not in removals]
    added = []
    for prerequisite in sorted(REQUIRED_PREREQUISITES.get(row['capabilityId'], ())):
        if prerequisite not in after:
            after.append(prerequisite)
            added.append((row['capabilityId'], prerequisite))
    removed = [(row['capabilityId'], item) for item in before if item in removals]
    return after, removed, added


def update_scope(scope, old_stage, new_stage):
    old = f'Full outcome allocation: stage {old_stage}.'
    return scope.replace(old, f'Full outcome allocation: stage {new_stage}.')


def catalog_by_id(catalog):
    entries = catalog.get('stages')
    if catalog.get('schemaVersion') != 1 or not isinstance(entries, list):
        raise ValueError('unsupported stage catalog')
    result = {}
    orders = set()
    for stage in entries:
        if stage['id'] in result:
            raise ValueError(f'duplicate stage ID: {stage["id"]}')
        if stage['order'] in orders:
            raise ValueError(f'duplicate stage order: {stage["order"]}')
        result[stage['id']] = stage
        orders.add(stage['order'])
    expected = ['A', 'B'] + [f'C{number:03d}' for number in range(1, 60)]
    ordered = [stage['id'] for stage in sorted(entries, key=lambda item: item['order'])]
    if ordered != expected:
        raise ValueError('stage catalog must order A, B and C001-C059 exactly')
    return result


def reallocate(ledger, catalog, catalog_sha256):
    if ledger.get('schemaVersion') not in (1, 2, 3):
        raise ValueError(f'unsupported ledger schema: {ledger.get("schemaVersion")}')
    stages = catalog_by_id(catalog)
    original_baseline = [copy.deepcopy(row) for row in ledger['coverage']
                         if row['acceptanceStage'] in ('A', 'B')]
    output = copy.deepcopy(ledger)
    output['schemaVersion'] = 3
    output['stageCatalogSha256'] = catalog_sha256
    removed_edges = []
    added_edges = []
    seen = set()
    for row in output['coverage']:
        identifier = row['capabilityId']
        if identifier in seen:
            raise ValueError(f'duplicate coverage row: {identifier}')
        seen.add(identifier)
        if row['acceptanceStage'] in ('A', 'B'):
            continue
        old_stage = row['acceptanceStage']
        new_stage = target_stage(identifier)
        row['acceptanceStage'] = new_stage
        row['prerequisites'], removed, added = corrected_prerequisites(row)
        removed_edges.extend(removed)
        added_edges.extend(added)
        row['conditions']['scope'] = update_scope(
            row['conditions']['scope'], old_stage, new_stage)

    new_baseline = [row for row in output['coverage']
                    if row['acceptanceStage'] in ('A', 'B')]
    if new_baseline != original_baseline:
        raise AssertionError('A/B coverage rows changed during reallocation')

    counts = {identifier: 0 for identifier in stages}
    blocks = {identifier: set() for identifier in stages}
    for row in output['coverage']:
        stage = row['acceptanceStage']
        if stage not in stages:
            raise ValueError(f'{row["capabilityId"]} uses unknown stage {stage}')
        counts[stage] += 1
        if stage.startswith('C'):
            blocks[stage].add(capability_block(row['capabilityId']))
    for identifier, stage in stages.items():
        if counts[identifier] != stage['expectedOutcomeCount']:
            raise ValueError(
                f'{identifier} has {counts[identifier]} outcomes, '
                f'expected {stage["expectedOutcomeCount"]}')
        if identifier.startswith('C') and blocks[identifier] != {stage['block']}:
            raise ValueError(
                f'{identifier} has blocks {sorted(blocks[identifier])}, '
                f'expected only {stage["block"]}')
    if sum(counts[s] for s in counts if s.startswith('C')) != 863:
        raise ValueError('post-B allocation must contain exactly 863 outcomes')
    if len(output['coverage']) != 925:
        raise ValueError('canonical ledger must contain exactly 925 outcomes')
    return output, sorted(removed_edges), sorted(added_edges), counts


def canonical_bytes(data):
    return (json.dumps(data, ensure_ascii=False, indent=2) + '\n').encode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ledger', type=Path, default=DEFAULT_LEDGER)
    parser.add_argument('--stages', type=Path, default=DEFAULT_STAGES)
    parser.add_argument('--output', type=Path,
                        help='output ledger; defaults to replacing --ledger')
    parser.add_argument('--check', action='store_true',
                        help='fail unless the ledger already matches generated bytes')
    args = parser.parse_args()
    output_path = args.output or args.ledger
    ledger = load_json(args.ledger)
    result, removed_edges, added_edges, counts = reallocate(
        ledger, load_json(args.stages), stage_digest(args.stages))
    raw = canonical_bytes(result)
    if args.check:
        if output_path.read_bytes() != raw:
            print(f'{output_path}: reallocation is not up to date', file=sys.stderr)
            return 1
    else:
        output_path.write_bytes(raw)
    print(f'A/B rows preserved: {counts["A"] + counts["B"]} (8 + 54)')
    print('Post-B rows allocated exactly once: '
          f'{sum(counts[s] for s in counts if s.startswith("C"))}')
    print(f'Prerequisite edges removed: {len(removed_edges)}')
    for owner, prerequisite in removed_edges:
        print(f'  {owner} <- {prerequisite}')
    print(f'Prerequisite edges added: {len(added_edges)}')
    for owner, prerequisite in added_edges:
        print(f'  {owner} <- {prerequisite}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

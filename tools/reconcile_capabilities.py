"""Inventory denominator checks for the capability validator (no acceptance claims)."""
import hashlib
import re
import subprocess


def inventory_rows(text):
    """Enumerate Markdown table data, preserving physical lines and exact row bytes.

    A header is followed by a separator. Escaped pipes do not affect enumeration.
    Narrative contracts remain in the content-addressed inventory, not a second
    interpretation of its table columns.
    """
    lines = text.splitlines()
    separator = re.compile(r'^\s*\|[\s:|\-]+\|\s*$')
    for i, line in enumerate(lines):
        if not line.startswith('|') or separator.fullmatch(line):
            continue
        if i + 1 < len(lines) and separator.fullmatch(lines[i + 1]):
            continue
        yield i + 1, hashlib.sha256(line.encode()).hexdigest()


def reconcile(index, mapping, directory, registry, ledger, roots):
    report = registry.report
    before = len(report.errors)
    expected = {}
    inventories = set()
    if not index['inventories']:
        report.error('inventory-empty', 'completeness', 'No inventory denominator supplied')
    indexed_paths = {(directory / i['path']).resolve() for i in index['inventories']}
    for path in directory.rglob('*.md'):
        if path.resolve() not in indexed_paths:
            report.error('inventory-unindexed-file', str(path), 'Inventory corpus file is missing from index')
    for inventory in index['inventories']:
        name = inventory['id']
        if name in inventories:
            report.error('inventory-duplicate', name, 'Repeated inventory ID')
        inventories.add(name)
        path = (directory / inventory['path']).resolve()
        if not path.is_relative_to(directory.resolve()):
            report.error('inventory-path', name, 'Inventory leaves index directory')
            continue
        raw = path.read_bytes()
        if hashlib.sha256(raw).hexdigest() != inventory['sha256']:
            report.error('inventory-digest', name, 'Inventory changed; reconcile new content')
        for line, digest in inventory_rows(raw.decode()):
            expected[name, line] = digest
    allocations = {r['capabilityId']: r for r in ledger['coverage']}
    obligations = {o['id']: r['capabilityId'] for r in ledger['coverage']
                   for o in r['evidenceObligations']}

    def references(row, owner):
        for identifier in row['capabilities']:
            registry.reference(identifier, 'capability', owner)
            if identifier not in allocations:
                report.error('inventory-allocation', owner, f'{identifier} needs active allocation')
            if not any(obligations.get(o) == identifier for o in row['obligations']):
                report.error('inventory-obligation', owner, f'{identifier} needs a caller obligation')
        for obligation in row['obligations']:
            if obligations.get(obligation) not in row['capabilities']:
                report.error('inventory-obligation', owner, f'Unknown or unrelated obligation {obligation}')

    seen = set()
    unresolved = []
    counts = {}
    for row in mapping['rows']:
        key = row['inventory'], row['line']
        owner = f'{key[0]}:{key[1]}'
        if key in seen:
            report.error('inventory-duplicate-row', owner, 'Only one disposition per inventory row')
        seen.add(key)
        if key not in expected:
            report.error('inventory-extra-row', owner, 'No such inventory data row')
        elif row['sha256'] != expected[key]:
            report.error('inventory-row-digest', owner, 'Disposition describes different row content')
        disposition = row['disposition']
        counts[disposition] = counts.get(disposition, 0) + 1
        if disposition == 'unresolved' or (disposition == 'required' and not row['capabilities']):
            unresolved.append(owner)
            report.error('inventory-unresolved', owner, 'Required outcome has no complete disposition')
        references(row, owner)
    for name, line in sorted(expected.keys() - seen):
        owner = f'{name}:{line}'
        unresolved.append(owner)
        report.error('inventory-missing-row', owner, 'Inventory row has no disposition')
    scenario_ids = set()
    for scenario in mapping['scenarios']:
        owner = scenario['id']
        if owner in scenario_ids:
            report.error('inventory-duplicate-scenario', owner, 'Scenario ID must be unique')
        scenario_ids.add(owner)
        if not scenario['capabilities']:
            report.error('inventory-scenario', owner, 'Scenario must identify concrete callers')
        references(scenario, owner)
    referenced = {c for r in mapping['rows'] + mapping['scenarios'] for c in r['capabilities']}
    for identifier in allocations.keys() - referenced:
        report.error('inventory-unmapped-outcome', identifier,
                     'Active outcome has no inventory or cross-cutting scenario obligation')

    root = roots.get('tomenet')
    if index['sourceFiles'] or index['sourceTrees']:
        if root is None:
            report.error('source-unavailable', 'inventory', 'Completeness requires --source-root tomenet=PATH')
        else:
            paths = set()
            for source in index['sourceFiles']:
                name = source['path']
                if name in paths:
                    report.error('inventory-source-duplicate', name, 'Repeated source file')
                paths.add(name)
                path = (root / name).resolve()
                if not path.is_relative_to(root):
                    report.error('source-path', name, 'Source leaves repository')
                    continue
                if hashlib.sha256(path.read_bytes()).hexdigest() != source['sha256']:
                    report.error('inventory-source-digest', name, 'Baseline changed; re-audit outcomes and allocation')
            for tree in index['sourceTrees']:
                path = (root / tree).resolve()
                if not path.is_relative_to(root):
                    report.error('source-path', tree, 'Tree leaves repository')
                    continue
                tracked = subprocess.run(['git', '-C', str(root), 'ls-files', '-z', '--', tree],
                                         capture_output=True, text=True)
                if tracked.returncode:
                    report.error('source-unavailable', tree, 'Cannot enumerate tracked baseline sources')
                    continue
                for name in tracked.stdout.split('\0'):
                    if name and name not in paths:
                        report.error('inventory-new-source', name, 'New baseline source needs inventory audit')
    return {'status': 'incomplete' if len(report.errors) > before else 'complete',
            'inventories': len(inventories), 'inventoryRows': len(expected),
            'dispositions': counts, 'unresolvedRows': unresolved,
            'scenarios': len(scenario_ids), 'acceptedCapabilities': 0}

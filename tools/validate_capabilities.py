#!/usr/bin/env python3
"""Read-only validation of canonical capability data and native allocation."""
import argparse
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SCHEMAS = ROOT / 'docs/capabilities'
KINDS = {'capabilities': 'capability', 'surfaces': 'surface', 'actions': 'action',
         'states': 'state', 'bindings': 'binding', 'inputContexts': 'context',
         'sources': 'source'}
RELATIONS = {
    'capability-exposed-on-surface': ('capability', 'surface'),
    'capability-uses-action': ('capability', 'action'),
    'capability-observes-state': ('capability', 'state'),
    'surface-presents-state': ('surface', 'state'),
    'binding-invokes-action': ('binding', 'action'),
}
TRANSITIONS = {
    'proposed': {'proposed', 'active', 'retired'},
    'active': {'active', 'deprecated', 'retired'},
    'deprecated': {'deprecated', 'retired'},
    'retired': {'retired'},
}


def check_cycles(graph, report, code):
    """Iterative DFS keeps long replacement histories independent of recursion limits."""
    done = set()
    for start in graph:
        if start in done:
            continue
        active = set()
        stack = [(start, False)]
        while stack:
            node, leaving = stack.pop()
            if leaving:
                active.remove(node)
                done.add(node)
            elif node in active:
                report.error(code, node, 'Cycle in directed references')
            elif node not in done:
                active.add(node)
                stack.append((node, True))
                stack.extend((target, False) for target in graph.get(node, []))


class DataError(ValueError):
    pass


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise DataError(f'duplicate JSON member {key!r}')
        result[key] = value
    return result


def load(path):
    raw = path.read_bytes()
    return json.loads(raw, object_pairs_hook=unique_object), hashlib.sha256(raw).hexdigest()


class Report:
    def __init__(self):
        self.errors = []

    def error(self, code, entity, message):
        self.errors.append({'code': code, 'entity': entity, 'message': message})

    def structure(self, data, schema_name, label):
        from jsonschema import Draft202012Validator
        schema, _ = load(SCHEMAS / schema_name)
        Draft202012Validator.check_schema(schema)
        for error in sorted(Draft202012Validator(schema).iter_errors(data),
                            key=lambda e: str(list(e.absolute_path))):
            path = '/'.join(map(str, error.absolute_path))
            instance = data
            owner = label
            for component in error.absolute_path:
                instance = instance[component]
                if isinstance(instance, dict):
                    owner = instance.get('id', instance.get('capabilityId', owner))
            self.error('schema', owner, f'{label}/{path}: {error.message}')


class Registry:
    def __init__(self, manifest, report):
        self.manifest = manifest
        self.report = report
        self.entities = {}
        self.kinds = {}
        for collection, kind in KINDS.items():
            for entity in manifest[collection]:
                identifier = entity['id']
                if identifier in self.entities:
                    report.error('duplicate-id', identifier, 'ID must be globally unique')
                if not identifier.startswith(kind + '.'):
                    report.error('id-type', identifier, f'{collection} requires {kind} IDs')
                self.entities[identifier] = entity
                self.kinds[identifier] = kind

    def reference(self, identifier, kind, owner):
        if identifier not in self.entities:
            self.report.error('unknown-id', owner, f'Unknown {kind} reference: {identifier}')
            return None
        if self.kinds[identifier] != kind:
            self.report.error('reference-type', owner, f'{identifier} must refer to {kind}')
            return None
        return self.entities[identifier]

    def validate(self):
        replacements = {}
        for identifier, entity in self.entities.items():
            if self.kinds[identifier] == 'source':
                continue
            for source in entity['sources']:
                self.reference(source, 'source', identifier)
            if self.kinds[identifier] == 'capability' and entity['lifecycle'] == 'active':
                if not any(self.entities.get(source, {}).get('role') == 'behavior'
                           for source in entity['sources']):
                    self.report.error('missing-behavior', identifier,
                                      'Active outcome requires behavior provenance')
            replacements[identifier] = entity.get('replacedBy', [])
            if replacements[identifier] and entity['lifecycle'] not in ('deprecated', 'retired'):
                self.report.error('replacement-lifecycle', identifier,
                                  'Only deprecated/retired entities can declare replacements')
            for replacement in replacements[identifier]:
                self.reference(replacement, self.kinds[identifier], identifier)
        check_cycles(replacements, self.report, 'replacement-cycle')
        for binding in self.manifest['bindings']:
            self.reference(binding['actionId'], 'action', binding['id'])
            self.reference(binding['contextId'], 'context', binding['id'])
        for relation in self.manifest['relations']:
            owner = f"{relation['from']} -> {relation['to']}"
            for endpoint, kind in zip(('from', 'to'), RELATIONS[relation['kind']]):
                self.reference(relation[endpoint], kind, owner)
            if relation['kind'] == 'binding-invokes-action':
                binding = self.entities.get(relation['from'], {})
                if 'actionId' in binding and binding['actionId'] != relation['to']:
                    self.report.error('binding-action', owner, 'Relation contradicts binding actionId')

    def history(self, previous):
        for identifier, old in previous.entities.items():
            # Source records describe observations and may be refreshed; entity IDs persist.
            if previous.kinds[identifier] == 'source':
                continue
            current = self.entities.get(identifier)
            if current is None:
                self.report.error('removed-id', identifier, 'Historical entity must remain, even after retirement')
                continue
            if self.kinds[identifier] != previous.kinds[identifier]:
                self.report.error('reused-id', identifier, 'Entity kind cannot change')
                continue
            if current['lifecycle'] not in TRANSITIONS[old['lifecycle']]:
                self.report.error('lifecycle-transition', identifier,
                                  f"Illegal transition {old['lifecycle']} -> {current['lifecycle']}")
            if not set(old.get('replacedBy', [])) <= set(current.get('replacedBy', [])):
                self.report.error('replacement-history', identifier, 'Historical replacement links cannot be removed')


def validate_coverage(registry, ledger, digest):
    report = registry.report
    if ledger['manifestSha256'] != digest:
        report.error('manifest-digest', 'native-sv', 'Ledger does not identify actual manifest bytes')
    rows = {}
    obligations = set()
    for row in ledger['coverage']:
        identifier = row['capabilityId']
        capability = registry.reference(identifier, 'capability', identifier)
        if identifier in rows:
            report.error('duplicate-coverage', identifier, 'Only one allocation row per outcome')
        rows[identifier] = row
        if capability and capability['lifecycle'] != 'active':
            report.error('inactive-coverage', identifier, 'Current allocation requires an active outcome')
        for source in row['conditions']['sources']:
            registry.reference(source, 'source', identifier)
        for obligation in row['evidenceObligations']:
            if not obligation['id'].startswith('obligation.'):
                report.error('id-type', obligation['id'], 'Expected obligation ID')
            if obligation['id'] in obligations:
                report.error('duplicate-id', obligation['id'], 'Evidence obligation IDs must be unique')
            obligations.add(obligation['id'])
            for source in obligation['sources']:
                registry.reference(source, 'source', obligation['id'])
        for prerequisite in row['prerequisites']:
            registry.reference(prerequisite, 'capability', identifier)
    for capability in registry.manifest['capabilities']:
        if capability['lifecycle'] == 'active' and capability['id'] not in rows:
            report.error('missing-coverage', capability['id'], 'Active outcome needs an explicit stage/allocation row')
    for identifier, row in rows.items():
        for prerequisite in row['prerequisites']:
            dependency = rows.get(prerequisite)
            if dependency is None:
                report.error('missing-prerequisite', identifier, f'{prerequisite} has no current allocation')
            elif dependency['acceptanceStage'] > row['acceptanceStage']:
                report.error('prerequisite-stage', identifier, f'{prerequisite} is allocated to a later stage')
    check_cycles({identifier: row['prerequisites'] for identifier, row in rows.items()},
                 report, 'prerequisite-cycle')


def validate_sources(manifest, roots, report):
    for source in manifest['sources']:
        identifier = source['id']
        root = roots.get(source['repository'])
        if root is None:
            report.error('source-unavailable', identifier,
                         f"No local root supplied for repository {source['repository']}")
            continue
        path = (root / source['path']).resolve()
        if not path.is_relative_to(root):
            report.error('source-path', identifier, 'Source resolves outside its repository root')
            continue
        try:
            raw = path.read_bytes()
        except OSError as error:
            report.error('source-unavailable', identifier, str(error))
            continue
        if hashlib.sha256(raw).hexdigest() != source['sha256']:
            report.error('source-digest', identifier, f"Content changed: {source['path']}")
        if 'anchor' in source and source['anchor'].encode() not in raw:
            report.error('source-anchor', identifier, f"Locator not found: {source['anchor']}")


def source_root(value):
    repository, separator, path = value.partition('=')
    if not repository or not separator or not path:
        raise argparse.ArgumentTypeError('expected REPOSITORY=PATH')
    return repository, Path(path).resolve()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--ledger', type=Path, required=True)
    parser.add_argument('--previous-manifest', type=Path,
                        help='validate ID preservation and lifecycle transitions against retained history')
    parser.add_argument('--source-root', type=source_root, action='append', default=[],
                        help='verify source bytes and literal anchors locally; repeat REPOSITORY=PATH')
    parser.add_argument('--inventory-index', type=Path,
                        help='content-addressed inventory denominator for completeness')
    parser.add_argument('--reconciliation', type=Path,
                        help='inventory dispositions and cross-cutting outcome obligations')
    args = parser.parse_args()
    report = Report()
    digest = None
    summary = {}
    unavailable = False
    completeness = {'status': 'not-requested'}
    try:
        manifest, digest = load(args.manifest)
        ledger, _ = load(args.ledger)
        report.structure(manifest, 'manifest.schema.json', 'manifest')
        report.structure(ledger, 'native-coverage.schema.json', 'ledger')
        inventory = mapping = None
        if bool(args.inventory_index) != bool(args.reconciliation):
            report.error('inventory-input', 'completeness', 'Supply both --inventory-index and --reconciliation')
        elif args.inventory_index:
            completeness = {'status': 'incomplete'}
            inventory, inventory_digest = load(args.inventory_index)
            mapping, reconciliation_digest = load(args.reconciliation)
            completeness.update(inventoryIndexSha256=inventory_digest,
                                reconciliationSha256=reconciliation_digest)
            report.structure(inventory, 'inventory-index.schema.json', 'inventory-index')
            report.structure(mapping, 'reconciliation.schema.json', 'reconciliation')
        previous = None
        if args.previous_manifest:
            previous, _ = load(args.previous_manifest)
            report.structure(previous, 'manifest.schema.json', 'previous-manifest')
        if not report.errors:
            registry = Registry(manifest, report)
            registry.validate()
            if previous is not None:
                history = Registry(previous, report)
                history.validate()
                registry.history(history)
            validate_coverage(registry, ledger, digest)
            if args.source_root:
                validate_sources(manifest, dict(args.source_root), report)
            if inventory is not None:
                from reconcile_capabilities import reconcile
                completeness.update(reconcile(inventory, mapping, args.inventory_index.parent,
                                              registry, ledger, dict(args.source_root)))
                if report.errors:
                    completeness['status'] = 'incomplete'
            summary = {
                'activeCapabilities': sum(c['lifecycle'] == 'active' for c in manifest['capabilities']),
                'pendingEvidence': len(ledger['coverage']),
                'acceptedCapabilities': 0,
            }
    except (OSError, ImportError) as error:
        unavailable = True
        report.error('unavailable', 'input/environment', str(error))
    except (ValueError, UnicodeError) as error:
        report.error('malformed-json', 'input', str(error))
    unavailable = unavailable or any(e['code'] == 'source-unavailable' for e in report.errors)
    if (args.inventory_index or args.reconciliation) and report.errors:
        completeness['status'] = 'incomplete'
    status = 'unavailable' if unavailable else 'invalid' if report.errors else 'valid'
    print(json.dumps({'status': status, 'manifestSha256': digest, 'summary': summary,
                      'historyChecked': args.previous_manifest is not None,
                      'sourceVerification': ('checked' if args.source_root else 'not-requested'),
                      'completeness': completeness,
                      'errors': report.errors}, indent=2))
    return 2 if unavailable else 1 if report.errors else 0


if __name__ == '__main__':
    sys.exit(main())

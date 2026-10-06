"""Conservative HTML claim gates. Artifact attestations never provide native parity."""
import hashlib
import json
from pathlib import Path

from validate_capabilities import load

# Every complete scenario explains the full visual/input flow. Non-applicability
# remains explicit and source-backed; entry/result/cancel/error/focus are mandatory.
OBLIGATIONS = ('entry', 'mouse', 'keyboard', 'selection', 'confirm', 'cancel', 'result',
               'error', 'focus', 'normal', 'empty', 'boundary', 'loading', 'unavailable',
               'resize', 'minimum-scale', 'fallback', 'persistence', 'reset', 'chat', 'blocking')
MANDATORY = {'entry', 'result', 'cancel', 'error', 'focus'}


def fingerprint(manifest_digest, files, row):
    scope = {key: row[key] for key in ('capabilityId', 'mapping', 'scenarios')}
    raw = json.dumps({'manifestSha256': manifest_digest, 'sourceInventory': files, 'scope': scope},
                     sort_keys=True, separators=(',', ':'), ensure_ascii=True).encode()
    return hashlib.sha256(raw).hexdigest()


def local_path(root, relative, artifact=False):
    path = root / relative
    if Path(relative).is_absolute() or '..' in Path(relative).parts or not path.resolve().is_relative_to(root):
        raise ValueError(f'Escaping consumer path: {relative}')
    if artifact and (not Path(relative).parts or Path(relative).parts[0] != '.sv-html-artifacts'):
        raise ValueError('Evidence and decisions must be in .sv-html-artifacts/')
    return path


def check_source(root, source, files, report, owner):
    path = local_path(root, source['path'])
    if files.get(source['path']) != source['sha256']:
        report.error('claim-source', owner, f"UX source not in current inventory: {source['path']}")
    elif source['anchor'].encode() not in path.read_bytes():
        report.error('claim-anchor', owner, f"Missing literal anchor in {source['path']}")


def artifact(root, record, report, owner):
    path = local_path(root, record['path'], artifact=True)
    data, digest = load(path)
    if digest != record['sha256']:
        report.error('artifact-digest', owner, f'Changed artifact: {record["path"]}')
        return None
    return data


def validate_scope(registry, row, report, current=True):
    owner = row['capabilityId']
    mapping = row['mapping']
    if mapping:
        for key, kind in (('contextIds', 'context'), ('bindingIds', 'binding')):
            for identifier in mapping[key]:
                entity = registry.reference(identifier, kind, owner)
                if current and entity and entity['lifecycle'] != 'active':
                    report.error('claim-scope', owner, 'Mapping references inactive entity')
        for identifier in mapping['entityIds']:
            kind = registry.kinds.get(identifier)
            if kind not in ('surface', 'action', 'state', 'binding', 'context'):
                report.error('claim-scope', owner, f'Unknown/non-UX entity: {identifier}')
            elif current and registry.entities[identifier]['lifecycle'] != 'active':
                report.error('claim-scope', owner, f'Inactive entity: {identifier}')
        for identifier in mapping['bindingIds']:
            binding = registry.entities.get(identifier, {})
            if current and binding.get('contextId') not in mapping['contextIds']:
                report.error('claim-scope', owner, 'Binding context is outside mapping scope')
    scenario_ids = [s['id'] for s in row['scenarios']]
    for name, records in (('scenario', row['scenarios']), ('evidence', row['evidence']), ('approval', row['approvals'])):
        ids = [r['id'] for r in records]
        if len(set(ids)) != len(ids):
            report.error('duplicate-claim', owner, f'Duplicate {name} ID')
    for evidence in row['evidence']:
        if evidence['scenarioId'] not in scenario_ids:
            report.error('claim-reference', owner, 'Evidence references unknown scenario')
    if row['implementation'] == 'prototype-complete':
        if not mapping or not row['scenarios'] or not row['evidence']:
            report.error('incomplete-flow', owner, 'Complete flow requires mapping, scenarios and evidence')
        for scenario in row['scenarios']:
            obligations = {e['id']: e for e in scenario['expectations']}
            if len(obligations) != len(scenario['expectations']) or set(obligations) != set(OBLIGATIONS):
                report.error('incomplete-flow', owner, 'Scenario must account for every visual/input obligation')
            if any(obligations.get(key, {}).get('applicability') != 'required' for key in MANDATORY):
                report.error('incomplete-flow', owner, 'Entry/result/cancel/error/focus cannot be excluded')
    if row['approvals'] and not mapping:
        report.error('approval-scope', owner, 'Human approval requires an explicit mapping scope')
    if mapping and current:
        validate_variants(registry, row, report)


def validate_variants(registry, row, report):
    """Bind full-outcome coverage to canonical routes and explicit gate selections."""
    owner, mapping = row['capabilityId'], row['mapping']
    actions = {r['to'] for r in registry.manifest['relations']
               if r['from'] == owner and r['kind'] == 'capability-uses-action'}
    related = {r['to'] for r in registry.manifest['relations'] if r['from'] == owner}
    bindings = [b for b in registry.manifest['bindings']
                if b['lifecycle'] == 'active' and b['actionId'] in actions]
    required = set()
    for binding in bindings:
        related.update((binding['id'], binding['contextId']))
        for keyset in binding.get('keyset', ('normal', 'roguelike')):
            for platform in binding.get('platform', ('linux', 'windows')):
                for gate in mapping['gates']:
                    required.add((binding['id'], binding['contextId'], keyset, platform, gate))
    selected = set()
    for scenario in row['scenarios']:
        binding_id, context_id = scenario['bindingId'], scenario['contextId']
        selection = (binding_id, context_id, scenario['keyset'], scenario['platform'], scenario['gate'])
        selected.add(selection)
        if scenario['gate'] not in mapping['gates']:
            report.error('scenario-scope', owner, 'Scenario gate is outside mapping scope')
        if context_id is not None and context_id not in mapping['contextIds']:
            report.error('scenario-scope', owner, 'Scenario context is outside mapping scope')
        if binding_id is not None:
            binding = registry.entities.get(binding_id, {})
            if (binding_id not in mapping['bindingIds'] or binding.get('contextId') != context_id
                    or scenario['keyset'] not in binding.get('keyset', ('normal', 'roguelike'))
                    or scenario['platform'] not in binding.get('platform', ('linux', 'windows'))):
                report.error('scenario-scope', owner, 'Scenario selection contradicts its canonical binding')
    if row['implementation'] == 'prototype-complete':
        mapped = set(mapping['entityIds'] + mapping['contextIds'] + mapping['bindingIds'])
        if not related <= mapped or not {b['id'] for b in bindings} <= set(mapping['bindingIds']):
            report.error('incomplete-scope', owner, 'Complete mapping must include all canonical outcome routes')
        if required - selected:
            report.error('incomplete-scope', owner, 'Missing canonical binding/keyset/platform/gate scenarios')
        if set(mapping['gates']) - {s['gate'] for s in row['scenarios']}:
            report.error('incomplete-scope', owner, 'Missing scenarios for declared gates')


def evaluate(registry, raw_digest, ledger, files, root, report):
    claims = []
    for row in ledger['coverage']:
        owner = row['capabilityId']
        start = len(report.errors)
        current = fingerprint(raw_digest, files, row)
        claim = {'capabilityId': owner, 'implementation': row['implementation'],
                 'contractSha256': current, 'prototypeComplete': False, 'uxApproved': False, 'stale': False}
        records = row['evidence'] + row['approvals']
        fresh = [r for r in records if r['contractSha256'] == current]
        claim['stale'] = len(fresh) != len(records)
        validate_scope(registry, row, report, current=bool(fresh) or not records)
        # Changed contracts retain historical declarations without recertifying them.
        if fresh:
            mapping = row['mapping']
            if mapping:
                for source in mapping['sources']:
                    check_source(root, source, files, report, owner)
                for location in mapping['entrypoint'].values():
                    if location['path'] not in files:
                        report.error('entrypoint', owner, 'Entrypoint is outside inventoried source files')
                    elif location['anchor'].encode() not in local_path(root, location['path']).read_bytes():
                        report.error('entrypoint', owner, 'Entrypoint/registration anchor is missing')
            for scenario in row['scenarios']:
                check_source(root, scenario['fixture'], files, report, owner)
                for expectation in scenario['expectations']:
                    check_source(root, expectation['source'], files, report, owner)
        passed = set()
        scenarios = {s['id']: s for s in row['scenarios']}
        for evidence in row['evidence']:
            if evidence not in fresh:
                continue
            data = artifact(root, evidence['artifact'], report, owner)
            scenario = scenarios.get(evidence['scenarioId'])
            if data is None or scenario is None:
                continue
            environment = evidence['environment']
            if (not row['mapping'] or environment['page'] != row['mapping']['entrypoint']['page']['path']
                    or environment['keyset'] != scenario['keyset']
                    or environment['platform'] != scenario['platform']
                    or environment['gates'] != scenario['gate']):
                report.error('evidence-scope', owner, 'Evidence environment differs from the scenario selection')
            expected = {e['id']: 'pass' if e['applicability'] == 'required' else 'not-applicable'
                        for e in scenario['expectations']}
            if (not isinstance(data, dict) or data.get('kind') != 'html-scenario'
                    or data.get('capabilityId') != owner or data.get('scenarioId') != evidence['scenarioId']
                    or data.get('contractSha256') != current or data.get('environment') != evidence['environment']
                    or data.get('observedAt') != evidence['observedAt'] or data.get('observer') != evidence['observer']
                    or data.get('results') != expected):
                report.error('flow-evidence', owner, 'Artifact must attest all scoped expectation outcomes')
            else:
                passed.add(evidence['scenarioId'])
        for approval in row['approvals']:
            if approval not in fresh:
                continue
            data = artifact(root, approval['decision'], report, owner)
            if (not isinstance(data, dict) or data.get('actorKind') != 'human'
                    or data.get('decision') != 'ux-approved' or data.get('capabilityId') != owner
                    or data.get('contractSha256') != current or data.get('approver') != approval['approver']
                    or data.get('approvedAt') != approval['approvedAt']
                    or not isinstance(data.get('decisionText'), str) or not data['decisionText'].strip()):
                report.error('human-provenance', owner, 'Approval requires an explicit scoped human decision')
            else:
                claim['uxApproved'] = True
        if row['implementation'] == 'prototype-complete' and not claim['stale'] and set(scenarios) - passed:
            report.error('incomplete-flow', owner, 'Every declared scenario needs current passing evidence')
        claim['prototypeComplete'] = (row['implementation'] == 'prototype-complete' and bool(scenarios)
                                      and set(scenarios) <= passed)
        if len(report.errors) != start:
            claim.update(prototypeComplete=False, uxApproved=False)
        claims.append(claim)
    return claims

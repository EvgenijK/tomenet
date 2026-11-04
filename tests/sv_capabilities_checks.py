#!/usr/bin/env python3
"""Contract checks through the standalone registry validator process."""
import json
import hashlib
import copy
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / 'tests/capabilities/fixtures'
TOOL = ROOT / 'tools/validate_capabilities.py'
STAGES = ROOT / 'docs/capabilities/stages.json'


class RegistryChecks(unittest.TestCase):
    def run_validator(self, manifest, ledger, *extra):
        arguments = [sys.executable, str(TOOL), '--manifest', str(manifest),
                     '--ledger', str(ledger)]
        if json.loads(Path(ledger).read_text()).get('schemaVersion') == 3 and \
                '--stages' not in extra:
            arguments.extend(('--stages', str(STAGES)))
        arguments.extend(map(str, extra))
        run = subprocess.run(arguments,
                             capture_output=True, text=True, cwd=ROOT)
        self.assertIn(run.returncode, (0, 1, 2), run.stderr)
        return run.returncode, json.loads(run.stdout)

    def validate_data(self, manifest, ledger, *extra):
        with tempfile.TemporaryDirectory(prefix='sv-registry-') as directory:
            path = Path(directory)
            raw = (json.dumps(manifest, indent=2) + '\n').encode()
            (path / 'manifest.json').write_bytes(raw)
            ledger = copy.deepcopy(ledger)
            ledger['manifestSha256'] = hashlib.sha256(raw).hexdigest()
            if ledger.get('schemaVersion') == 3:
                ledger['stageCatalogSha256'] = hashlib.sha256(STAGES.read_bytes()).hexdigest()
            (path / 'ledger.json').write_text(json.dumps(ledger))
            return self.run_validator(path / 'manifest.json', path / 'ledger.json', *extra)

    def canonical_data(self):
        return (json.loads((ROOT / 'docs/capabilities/manifest.json').read_text()),
                json.loads((ROOT / 'docs/capabilities/native-coverage.json').read_text()))

    def fixture_data(self):
        return (json.loads((FIXTURES / 'valid/manifest.json').read_text()),
                json.loads((FIXTURES / 'valid/native-coverage.json').read_text()))

    def append_capability(self, manifest, ledger, identifier, stage, prerequisites=()):
        capability = copy.deepcopy(manifest['capabilities'][0])
        capability['id'] = identifier
        manifest['capabilities'].append(capability)
        row = copy.deepcopy(ledger['coverage'][0])
        row.update(capabilityId=identifier, acceptanceStage=stage,
                   prerequisites=list(prerequisites))
        suffix = identifier.removeprefix('capability.')
        for index, obligation in enumerate(row['evidenceObligations']):
            obligation['id'] = f'obligation.{suffix}.case-{index}'
        ledger['coverage'].append(row)
        return row

    def test_published_negative_fixtures(self):
        cases = json.loads((FIXTURES / 'invalid-cases.json').read_text())
        for case in cases:
            with self.subTest(case=case['name']):
                manifest, ledger = self.canonical_data()
                for operation in case['operations']:
                    parent = {'manifest': manifest, 'ledger': ledger}[operation['target']]
                    for component in operation['path'][:-1]:
                        parent = parent[component]
                    key = operation['path'][-1]
                    if operation['op'] == 'delete':
                        del parent[key]
                    else:
                        parent[key] = operation['value']
                code, report = self.validate_data(manifest, ledger)
                self.assertEqual(code, 1, report)
                self.assertIn(case['expectedCode'], {e['code'] for e in report['errors']}, report)

    def test_canonical_slice_and_source_provenance(self):
        code, report = self.run_validator(ROOT / 'docs/capabilities/manifest.json',
                                          ROOT / 'docs/capabilities/native-coverage.json',
                                          '--source-root', f'tomenet={ROOT}')
        self.assertEqual(code, 0, report)
        manifest, ledger = self.canonical_data()
        active = [row for row in manifest['capabilities'] if row['lifecycle'] == 'active']
        pending = [row for row in ledger['coverage'] if row['evidenceStatus'] == 'pending']
        self.assertEqual(report['summary']['activeCapabilities'], len(active))
        self.assertEqual(report['summary']['pendingEvidence'], len(pending))
        self.assertEqual(report['summary']['acceptedCapabilities'], 0)

    def test_byte_identity_includes_whitespace(self):
        with tempfile.TemporaryDirectory(prefix='sv-digest-') as directory:
            manifest = Path(directory) / 'manifest.json'
            manifest.write_bytes((FIXTURES / 'valid/manifest.json').read_bytes() + b'\n')
            code, report = self.run_validator(manifest, FIXTURES / 'valid/native-coverage.json')
        self.assertEqual(code, 1, report)
        self.assertIn('manifest-digest', {e['code'] for e in report['errors']}, report)

    def test_replacement_needs_its_own_coverage(self):
        manifest, ledger = self.canonical_data()
        replacement = copy.deepcopy(manifest['capabilities'][0])
        replacement['id'] = 'capability.status.read-hitpoints'
        manifest['capabilities'][0].update(lifecycle='retired', replacedBy=[replacement['id']])
        manifest['capabilities'].append(replacement)
        ledger['coverage'].pop(0)
        code, report = self.validate_data(manifest, ledger)
        self.assertEqual(code, 1, report)
        self.assertTrue(any(e['code'] == 'missing-coverage' and e['entity'] == replacement['id']
                            for e in report['errors']), report)

    def test_rename_preserves_id_but_removal_fails_history(self):
        manifest, ledger = self.canonical_data()
        with tempfile.TemporaryDirectory(prefix='sv-history-') as directory:
            previous = Path(directory) / 'previous.json'
            previous.write_text(json.dumps(manifest))
            manifest['capabilities'][0]['title'] = 'Renamed display title'
            code, report = self.validate_data(manifest, ledger, '--previous-manifest', previous)
            self.assertEqual(code, 0, report)
            manifest['capabilities'].pop(0)
            code, report = self.validate_data(manifest, ledger, '--previous-manifest', previous)
            self.assertEqual(code, 1, report)
            self.assertIn('removed-id', {e['code'] for e in report['errors']}, report)

    def test_unavailable_and_malformed_data_are_distinct(self):
        with tempfile.TemporaryDirectory(prefix='sv-invalid-') as directory:
            path = Path(directory) / 'manifest.json'
            code, report = self.run_validator(path, FIXTURES / 'valid/native-coverage.json')
            self.assertEqual((code, report['status']), (2, 'unavailable'))
            path.write_text('{"schemaVersion": 1, "schemaVersion": 2}')
            code, report = self.run_validator(path, FIXTURES / 'valid/native-coverage.json')
            self.assertEqual((code, report['status']), (1, 'invalid'))
            self.assertIn('malformed-json', {e['code'] for e in report['errors']}, report)

    def test_missing_source_is_unavailable(self):
        with tempfile.TemporaryDirectory(prefix='sv-unavailable-') as directory:
            code, report = self.run_validator(FIXTURES / 'valid/manifest.json',
                                              FIXTURES / 'valid/native-coverage.json',
                                              '--source-root', f'tomenet={directory}')
        self.assertEqual((code, report['status']), (2, 'unavailable'))
        self.assertEqual(report['errors'][0]['entity'], 'source.baseline.hp')

    def test_unknown_reference_identifies_owner(self):
        manifest, ledger = self.canonical_data()
        manifest['bindings'][0]['contextId'] = 'context.request.unknown'
        code, report = self.validate_data(manifest, ledger)
        self.assertEqual(code, 1, report)
        self.assertTrue(any(e['code'] == 'unknown-id' and
                            e['entity'] == 'binding.request.answer-key'
                            for e in report['errors']), report)

    def test_active_outcome_requires_behavior_and_coverage(self):
        manifest, ledger = self.canonical_data()
        manifest['capabilities'][0]['sources'] = ['source.acceptance.hp']
        ledger['coverage'].pop(0)
        code, report = self.validate_data(manifest, ledger)
        self.assertEqual(code, 1, report)
        self.assertTrue({'missing-behavior', 'missing-coverage'} <=
                        {e['code'] for e in report['errors']}, report)

    def test_replacement_cycle_and_reactivation_are_rejected(self):
        manifest, ledger = self.canonical_data()
        old = copy.deepcopy(manifest)
        first, second = manifest['capabilities'][:2]
        for entity, replacement in ((first, second), (second, first)):
            entity['lifecycle'] = 'retired'
            entity['replacedBy'] = [replacement['id']]
        code, report = self.validate_data(manifest, ledger)
        self.assertEqual(code, 1, report)
        self.assertIn('replacement-cycle', {e['code'] for e in report['errors']}, report)
        old['capabilities'][0]['lifecycle'] = 'retired'
        manifest, ledger = self.canonical_data()
        with tempfile.TemporaryDirectory(prefix='sv-history-') as directory:
            previous = Path(directory) / 'previous.json'
            previous.write_text(json.dumps(old))
            code, report = self.validate_data(manifest, ledger, '--previous-manifest', previous)
        self.assertEqual(code, 1, report)
        self.assertIn('lifecycle-transition', {e['code'] for e in report['errors']}, report)

    def test_source_fingerprint_detects_dirty_bytes_at_same_revision(self):
        manifest, ledger = self.canonical_data()
        with tempfile.TemporaryDirectory(prefix='sv-sources-') as directory:
            root = Path(directory)
            for source in manifest['sources']:
                path = root / source['path']
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes((ROOT / source['path']).read_bytes())
            code, report = self.validate_data(manifest, ledger, '--source-root', f'tomenet={root}')
            self.assertEqual(code, 0, report)
            path = root / 'src/client/nclient.c'
            path.write_bytes(path.read_bytes() + b'\n/* changed with HEAD unchanged */\n')
            code, report = self.validate_data(manifest, ledger, '--source-root', f'tomenet={root}')
            self.assertEqual(code, 1, report)
            self.assertTrue(any(e['code'] == 'source-digest' and
                                e['entity'] == 'source.baseline.hp'
                                for e in report['errors']), report)

    def test_pending_coverage_is_valid_without_acceptance(self):
        code, report = self.run_validator(FIXTURES / 'valid/manifest.json',
                                          FIXTURES / 'valid/native-coverage.json')
        self.assertEqual(code, 0, report)
        self.assertEqual(report['status'], 'valid')
        self.assertEqual(report['summary']['activeCapabilities'], 1)
        self.assertEqual(report['summary']['pendingEvidence'], 1)
        self.assertEqual(report['summary']['acceptedCapabilities'], 0)

    def test_canonical_legacy_a_b_projection_is_frozen(self):
        _, ledger = self.canonical_data()
        fixture = json.loads((FIXTURES / 'legacy-ab-projection.json').read_text())
        fields = fixture['projectionFields']
        expected = {row['id']: row for row in fixture['stages']}
        for stage in ('A', 'B'):
            rows = [row for row in ledger['coverage'] if row['acceptanceStage'] == stage]
            ids = [row['capabilityId'] for row in rows]
            projection = [{field: row[field] for field in fields} for row in rows]
            encode = lambda value: (json.dumps(
                value, ensure_ascii=False, sort_keys=True, separators=(',', ':')) + '\n').encode()
            self.assertEqual(len(rows), expected[stage]['expectedOutcomeCount'])
            self.assertEqual(hashlib.sha256(encode(ids)).hexdigest(),
                             expected[stage]['capabilityIdsSha256'])
            self.assertEqual(hashlib.sha256(encode(projection)).hexdigest(),
                             expected[stage]['semanticProjectionSha256'])

    def test_v3_orders_numbered_post_b_stages(self):
        sys.path.insert(0, str(ROOT / 'tools'))
        from validate_capabilities import stage_rank
        orders = {'A': 1, 'B': 2, 'C002': 4, 'C010': 12}
        self.assertLess(stage_rank('C002', 3, orders), stage_rank('C010', 3, orders))

    def test_v3_post_b_stage_contains_one_block(self):
        manifest, ledger = self.canonical_data()
        mixed = next(row for row in ledger['coverage']
                     if row['acceptanceStage'] == 'C002')
        mixed['acceptanceStage'] = 'C001'
        code, report = self.validate_data(manifest, ledger)
        self.assertEqual(code, 1, report)
        self.assertTrue(any(error['code'] == 'stage-block' and error['entity'] == 'C001'
                            for error in report['errors']), report)

    def test_v3_post_b_stage_is_capped_at_50_outcomes(self):
        catalog = json.loads(STAGES.read_text())
        post_b = [stage for stage in catalog['stages'] if stage['id'].startswith('C')]
        self.assertTrue(post_b)
        self.assertLessEqual(max(stage['expectedOutcomeCount'] for stage in post_b), 50)
        self.assertEqual(next(stage for stage in catalog['stages']
                              if stage['id'] == 'B')['expectedOutcomeCount'], 54)

        invalid = copy.deepcopy(catalog)
        invalid['stages'][2]['expectedOutcomeCount'] = 51
        manifest, ledger = self.canonical_data()
        with tempfile.TemporaryDirectory(prefix='sv-stage-limit-') as directory:
            path = Path(directory)
            manifest_raw = (json.dumps(manifest, indent=2) + '\n').encode()
            stages_raw = (json.dumps(invalid, indent=2) + '\n').encode()
            (path / 'manifest.json').write_bytes(manifest_raw)
            (path / 'stages.json').write_bytes(stages_raw)
            ledger['manifestSha256'] = hashlib.sha256(manifest_raw).hexdigest()
            ledger['stageCatalogSha256'] = hashlib.sha256(stages_raw).hexdigest()
            (path / 'ledger.json').write_text(json.dumps(ledger))
            code, report = self.run_validator(
                path / 'manifest.json', path / 'ledger.json',
                '--stages', path / 'stages.json')
        self.assertEqual(code, 1, report)
        self.assertIn('schema', {error['code'] for error in report['errors']}, report)

    def test_v3_rejects_legacy_and_zero_post_b_stage_ids(self):
        for stage in ('C', 'D', 'C000'):
            with self.subTest(stage=stage):
                manifest, ledger = self.fixture_data()
                ledger['schemaVersion'] = 3
                ledger['stageCatalogSha256'] = hashlib.sha256(STAGES.read_bytes()).hexdigest()
                ledger['coverage'][0]['acceptanceStage'] = stage
                code, report = self.validate_data(manifest, ledger)
                self.assertEqual(code, 1, report)
                self.assertIn('schema', {error['code'] for error in report['errors']}, report)


if __name__ == '__main__':
    unittest.main()

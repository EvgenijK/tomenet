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


class RegistryChecks(unittest.TestCase):
    def run_validator(self, manifest, ledger, *extra):
        run = subprocess.run([sys.executable, str(TOOL), '--manifest', str(manifest),
                              '--ledger', str(ledger), *map(str, extra)],
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
            (path / 'ledger.json').write_text(json.dumps(ledger))
            return self.run_validator(path / 'manifest.json', path / 'ledger.json', *extra)

    def canonical_data(self):
        return (json.loads((ROOT / 'docs/capabilities/manifest.json').read_text()),
                json.loads((ROOT / 'docs/capabilities/native-coverage.json').read_text()))

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


if __name__ == '__main__':
    unittest.main()

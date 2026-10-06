#!/usr/bin/env python3
"""Completeness checks through the production registry validator process."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / 'tests/capabilities/fixtures/valid'


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


class ReconciliationChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='sv-reconcile-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'src/client').mkdir(parents=True)
        (self.root / 'src/client/nclient.c').write_bytes((ROOT / 'src/client/nclient.c').read_bytes())
        self.inventory = '| Outcome | Contract |\n|---|---|\n| HP | Read current HP |\n'
        (self.root / 'baseline.md').write_text(self.inventory)
        self.index = {'schemaVersion': 1, 'inventories': [{
            'id': 'baseline', 'path': 'baseline.md',
            'sha256': digest(self.inventory.encode()), 'origin': 'Test baseline',
        }], 'sourceFiles': [], 'sourceTrees': []}
        self.row = {
            'inventory': 'baseline', 'line': 3,
            'sha256': digest(b'| HP | Read current HP |'), 'disposition': 'required',
            'capabilities': ['capability.status.read-hp'],
            'obligations': ['obligation.status.read-hp.primary'],
            'rationale': 'The HP surface must display the decoded current and maximum HP.',
        }
        self.mapping = {'schemaVersion': 1, 'rows': [self.row], 'scenarios': []}

    def run_validator(self, *extra):
        (self.root / 'index.json').write_text(json.dumps(self.index))
        (self.root / 'mapping.json').write_text(json.dumps(self.mapping))
        run = subprocess.run([
            sys.executable, str(ROOT / 'tools/validate_capabilities.py'),
            '--manifest', str(FIXTURE / 'manifest.json'),
            '--ledger', str(FIXTURE / 'native-coverage.json'),
            '--inventory-index', str(self.root / 'index.json'),
            '--reconciliation', str(self.root / 'mapping.json'), *extra,
        ], capture_output=True, text=True)
        self.assertIn(run.returncode, (0, 1, 2), run.stderr)
        return run.returncode, json.loads(run.stdout)

    def invalid(self, error, *extra):
        code, report = self.run_validator(*extra)
        self.assertEqual(code, 1, report)
        self.assertIn(error, {e['code'] for e in report['errors']}, report)
        self.assertEqual(report['completeness']['status'], 'incomplete')

    def test_omitted_inventory_row_fails_even_with_valid_registry(self):
        self.mapping['rows'] = []
        self.invalid('inventory-missing-row')

    def test_complete_allocation_is_still_pending_not_accepted(self):
        code, report = self.run_validator()
        self.assertEqual(code, 0, report)
        self.assertEqual(report['completeness']['status'], 'complete')
        self.assertEqual(report['summary']['pendingEvidence'], 1)
        self.assertEqual(report['summary']['acceptedCapabilities'], 0)

    def test_new_outcome_requires_mapping_even_after_inventory_digest_refresh(self):
        raw = (self.inventory + '| MP | Read current mana |\n').encode()
        (self.root / 'baseline.md').write_bytes(raw)
        self.index['inventories'][0]['sha256'] = digest(raw)
        self.invalid('inventory-missing-row')

    def test_removing_outcome_and_allocation_together_does_not_hide_gap(self):
        manifest = json.loads((FIXTURE / 'manifest.json').read_text())
        ledger = json.loads((FIXTURE / 'native-coverage.json').read_text())
        manifest['capabilities'] = []
        manifest['relations'] = [r for r in manifest['relations']
                                 if r['from'] != 'capability.status.read-hp']
        raw = (json.dumps(manifest, indent=2) + '\n').encode()
        (self.root / 'manifest.json').write_bytes(raw)
        ledger.update(manifestSha256=digest(raw), coverage=[])
        (self.root / 'ledger.json').write_text(json.dumps(ledger))
        self.invalid('inventory-allocation', '--manifest', str(self.root / 'manifest.json'),
                     '--ledger', str(self.root / 'ledger.json'))

    def test_changed_contract_cannot_reuse_old_mapping(self):
        raw = self.inventory.replace('current HP', 'current and maximum HP').encode()
        (self.root / 'baseline.md').write_bytes(raw)
        self.index['inventories'][0]['sha256'] = digest(raw)
        self.invalid('inventory-row-digest')

    def test_unknown_caller_cannot_hide_behind_primitive_evidence(self):
        self.row['capabilities'].append('capability.items.quaff')
        self.invalid('inventory-allocation')

    def test_wrong_obligation_is_rejected(self):
        self.row['obligations'] = ['obligation.items.quaff.result']
        self.invalid('inventory-obligation')

    def test_explicit_unresolved_disposition_fails(self):
        self.row['disposition'] = 'unresolved'
        self.invalid('inventory-unresolved')

    def test_changed_source_at_unchanged_revision_invalidates_completeness(self):
        (self.root / 'baseline.c').write_text('new behavior')
        self.index['sourceFiles'] = [{'path': 'baseline.c', 'sha256': digest(b'old behavior')}]
        self.invalid('inventory-source-digest', '--source-root', f'tomenet={self.root}')

    def test_duplicate_disposition_fails(self):
        duplicate = dict(self.row, rationale='A second contradictory disposition')
        self.mapping['rows'].append(duplicate)
        self.invalid('inventory-duplicate-row')

    def test_missing_source_cannot_report_complete(self):
        self.index['sourceFiles'] = [{'path': 'missing.c', 'sha256': digest(b'absent')}]
        code, report = self.run_validator('--source-root', f'tomenet={self.root}')
        self.assertEqual(code, 2, report)
        self.assertEqual(report['completeness']['status'], 'incomplete')

    def test_removed_inventory_cannot_hide_retained_corpus(self):
        self.index['inventories'] = []
        self.mapping['rows'] = []
        self.invalid('inventory-unindexed-file')

    def test_source_discovery_uses_tracked_baseline_not_personal_files(self):
        subprocess.run(['git', 'init', '-q', str(self.root)], check=True)
        subprocess.run(['git', '-C', str(self.root), 'add', 'src/client/nclient.c'], check=True)
        self.index['sourceTrees'] = ['src/client']
        self.index['sourceFiles'] = [{
            'path': 'src/client/nclient.c',
            'sha256': digest((self.root / 'src/client/nclient.c').read_bytes()),
        }]
        personal = self.root / 'src/client/personal.c'
        personal.write_text('personal experiment')
        code, report = self.run_validator('--source-root', f'tomenet={self.root}')
        self.assertEqual(code, 0, report)
        subprocess.run(['git', '-C', str(self.root), 'add', 'src/client/personal.c'], check=True)
        self.invalid('inventory-new-source', '--source-root', f'tomenet={self.root}')


if __name__ == '__main__':
    unittest.main()

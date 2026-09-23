#!/usr/bin/env python3
"""Checkpoint contracts through the production CLI; fixtures are not runtime evidence."""
import copy
import hashlib
import json
from pathlib import Path
import sys
import unittest

from sv_evidence_checks import EvidenceChecks, ROOT
sys.path.insert(0, str(ROOT / 'tools'))
from native_evidence import allocation_digest
from stage_a_checkpoint import OUTCOMES, environments, suites
from stage_a_provenance import PACKAGES


class CheckpointChecks(EvidenceChecks):
    # Reuse CLI setup, not inherited tests (the canonical matrix differs).
    def setUp(self):
        super().setUp()
        self.manifest = json.loads((ROOT / 'docs/capabilities/manifest.json').read_text())
        self.ledger = json.loads((ROOT / 'docs/capabilities/native-coverage.json').read_text())
        self.ledger['schemaVersion'] = 2
        self.ledger['manifestSha256'] = hashlib.sha256(json.dumps(self.manifest).encode()).hexdigest()
        self.row = self.ledger['coverage'][0]
        base = self.candidate()
        host = self.root / 'host.json'
        host.write_text(json.dumps({'schemaVersion': 1, 'packages': list(PACKAGES),
                                   'files': {str(self.root / 'artifact'): {'sha256': base['executable']['sha256']}}}))
        host_ref = {'repository': 'fixture', 'path': 'host.json', 'sha256': hashlib.sha256(host.read_bytes()).hexdigest()}
        self.evidence['records'] = []
        for row in self.ledger['coverage']:
            if row['capabilityId'] not in OUTCOMES:
                continue
            row.update(evidenceStatus='pending', evidenceIds=[])
            for platform, renderer in environments(row['capabilityId']):
                record = copy.deepcopy(base)
                identifier = str(len(self.evidence['records']))
                record.update(id='evidence.fixture-' + identifier, capabilityId=row['capabilityId'],
                              checkpoint='stage-a', allocationSha256=allocation_digest(row),
                              obligationIds=[o['id'] for o in row['evidenceObligations']], hostProvenance=host_ref)
                record['environment'].update(platform=platform, renderer=renderer,
                    architecture='i686' if platform == 'wine' else 'x86_64',
                    serverVersion=row['conditions']['serverVersions'][0], build=row['conditions']['builds'][0])
                prefix = 'wine-software' if platform == 'wine' else ('software' if renderer == 'software' else 'opengl')
                observation = {'capabilityId': row['capabilityId'], 'runtimeCheck': record['runtimeCheck'],
                    'environment': record['environment'], 'executable': record['executable'],
                    'configuration': record['configuration'], 'legacyUnchangedBySvBuild': True,
                    'checks': [{'name': prefix + '-' + s, 'result': 'pass', 'returncode': 0} for s in suites(row['capabilityId'])],
                    'buildChecks': [{'name': n, 'result': 'pass', 'returncode': 0} for n in
                                    ('build-linux', 'build-mingw', 'build-formats', 'stage-wine')]}
                path = self.root / (identifier + '.json')
                path.write_text(json.dumps(observation))
                record['report'] = {'repository': 'fixture', 'path': path.name,
                                    'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
                row['evidenceIds'].append(record['id'])
                self.evidence['records'].append(record)

    def check(self, passing=False):
        code, report = self.run_validator('--checkpoint', 'stage-a', '--source-root', f'fixture={self.root}',
                                          '--source-root', f'tomenet={ROOT}')
        self.assertEqual(code == 0, passing, report)
        self.assertEqual(report['checkpoint']['status'], 'passed' if passing else 'blocked')
        self.assertEqual(report['summary']['acceptedCapabilities'], 0)
        return report

    def test_checkpoint_covers_eight_outcomes_without_acceptance(self):
        report = self.check(True)
        self.assertEqual(len(report['checkpoint']['outcomes']), 8)
        self.assertEqual(report['checkpoint']['humanReview'], 'pending')

    def test_artifact_mutations(self):
        for name in ('executable', 'configuration', 'report'):
            with self.subTest(name=name):
                ref = self.evidence['records'][0][name]
                path = self.root / ref['path']
                original = path.read_bytes()
                path.write_bytes(original + b' ')
                report = self.check()
                self.assertTrue(all(r['status'] == 'pending' for r in report['checkpoint']['outcomes']))
                path.write_bytes(original)

    def test_dependency_mutations(self):
        path = self.root / 'source/input'
        path.write_text('changed')
        self.check()
        path.unlink()
        self.check()
        path.write_bytes(b'original')
        extra = self.root / 'source/new-include.h'
        extra.write_text('new header')
        self.check()
        extra.unlink()
        (self.root / 'resource/new-font').write_text('new resource')
        self.check()

    def test_unavailable_sdk(self):
        for record in self.evidence['records']:
            record['dependencies'][0]['repository'] = 'missing-sdk'
        self.check()

    def test_incomplete_runtime(self):
        self.evidence['records'][0]['runtimeCheck']['completed'] = False
        self.check()

    def test_false_windows_identity(self):
        record = next(r for r in self.evidence['records'] if r['environment']['platform'] == 'wine')
        record['environment'].update(platform='windows', osVersion='11')
        self.check()

    def test_checkpoint_cannot_accept_capability(self):
        next(r for r in self.ledger['coverage'] if r['capabilityId'] in OUTCOMES).update(
            implementation='native', evidenceStatus='accepted')
        self.check()

    def test_missing_outcome_or_obligation(self):
        record = self.evidence['records'][0]
        record['obligationIds'].pop()
        report = self.check()
        outcome = next(r for r in report['checkpoint']['outcomes'] if r['capabilityId'] == record['capabilityId'])
        self.assertEqual(outcome['status'], 'pending')

    def test_no_checkpoint_claim_from_full_evidence(self):
        for record in self.evidence['records']:
            record.pop('checkpoint')
        self.check()

    def test_unknown_checkpoint_is_rejected(self):
        self.evidence['records'][0]['checkpoint'] = 'stage-b'
        self.check()

    def test_failed_build_is_not_hidden_by_successful_runtime(self):
        record = self.evidence['records'][0]
        path = self.root / record['report']['path']
        data = json.loads(path.read_text())
        data['buildChecks'][0]['result'] = 'fail'
        path.write_text(json.dumps(data))
        record['report']['sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
        self.check()


if __name__ == '__main__':
    suite = unittest.TestSuite(CheckpointChecks(name) for name in CheckpointChecks.__dict__ if name.startswith('test_'))
    result = unittest.TextTestRunner(verbosity=1).run(suite)
    sys.exit(not result.wasSuccessful())

#!/usr/bin/env python3
"""Negative evidence contracts via the production CLI; no native runtime certification."""
import hashlib
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / 'tests/capabilities/fixtures/valid'


class EvidenceChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='sv-evidence-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.manifest = json.loads((FIXTURE / 'manifest.json').read_text())
        self.ledger = json.loads((FIXTURE / 'native-coverage.json').read_text())
        self.ledger['schemaVersion'] = 2
        self.ledger['manifestSha256'] = hashlib.sha256(json.dumps(self.manifest).encode()).hexdigest()
        self.row = self.ledger['coverage'][0]
        self.row.update(implementation='native', evidenceStatus='accepted', evidenceIds=['evidence.hp'])
        self.evidence = {'schemaVersion': 1, 'consumer': 'native-sv', 'records': [], 'fallbackRoutes': []}

    def run_validator(self, *extra):
        raw = json.dumps(self.manifest).encode()
        (self.root / 'manifest.json').write_bytes(raw)
        self.ledger['manifestSha256'] = hashlib.sha256(raw).hexdigest()
        (self.root / 'ledger.json').write_text(json.dumps(self.ledger))
        (self.root / 'evidence.json').write_text(json.dumps(self.evidence))
        run = subprocess.run([sys.executable, str(ROOT / 'tools/validate_capabilities.py'),
                              '--manifest', str(self.root / 'manifest.json'),
                              '--ledger', str(self.root / 'ledger.json'),
                              '--evidence', str(self.root / 'evidence.json'), *extra],
                             capture_output=True, text=True)
        self.assertIn(run.returncode, (0, 1, 2), run.stderr)
        return run.returncode, json.loads(run.stdout)

    def test_acceptance_requires_evidence(self):
        code, report = self.run_validator()
        self.assertEqual(code, 1, report)
        self.assertIn('evidence-missing', {e['code'] for e in report['errors']})
        self.assertEqual(report['summary']['acceptedCapabilities'], 0)


    def candidate(self):
        """Deliberately incomplete claim: never represents an actual native run."""
        artifact = self.root / 'artifact'
        artifact.write_bytes(b'validator negative fixture, not runtime evidence')
        fingerprint = {'repository': 'fixture', 'path': 'artifact',
                       'sha256': hashlib.sha256(artifact.read_bytes()).hexdigest()}
        contract = {k: v for k, v in self.row.items()
                    if k not in ('implementation', 'reason', 'evidenceStatus', 'evidenceIds')}
        dependencies = []
        for role in ('source', 'fixture', 'resource'):
            (self.root / role).mkdir()
            (self.root / role / 'input').write_bytes(b'original')
            dependencies.append({'repository': 'fixture', 'path': role, 'role': role,
                                 'impact': 'complete', 'files': {
                                     'input': hashlib.sha256(b'original').hexdigest()}})
        record = {'id': 'evidence.hp', 'kind': 'native-runtime', 'scenario': 'scenario.negative-fixture',
                  'capabilityId': self.row['capabilityId'],
                  'manifestSha256': self.ledger['manifestSha256'],
                  'allocationSha256': hashlib.sha256(json.dumps(contract, sort_keys=True,
                      separators=(',', ':'), ensure_ascii=True).encode()).hexdigest(),
                  'obligationIds': [self.row['evidenceObligations'][0]['id']],
                  'expectedOutcomes': ['Negative validator fixture; no runtime claim'],
                  'actualResult': 'pass',
                  'environment': {'platform': 'linux', 'osVersion': 'fixture', 'renderer': 'software',
                                  'serverVersion': self.row['conditions']['serverVersions'][0],
                                  'build': self.row['conditions']['builds'][0], 'architecture': 'fixture'},
                  'executable': fingerprint.copy(), 'configuration': fingerprint.copy(),
                  'report': fingerprint.copy(), 'dependencies': dependencies,
                  'runtimeCheck': {'scenario': 'scenario.negative-fixture', 'completed': True,
                                   'fallbackEntries': 0, 'routes': []}}
        self.evidence['records'] = [record]
        return record

    def reject(self, expected, exit_code=1):
        code, report = self.run_validator('--source-root', f'fixture={self.root}', '--source-root', f'tomenet={ROOT}')
        self.assertEqual(code, exit_code, report)
        self.assertIn(expected, {e['code'] for e in report['errors']}, report)
        self.assertEqual(report['summary']['acceptedCapabilities'], 0)
        return report

    def test_partial_platform_and_obligation_coverage_cannot_accept(self):
        self.candidate()
        self.reject('evidence-coverage')

    def test_dependency_add_remove_change_without_git(self):
        self.candidate()
        path = self.root / 'source/input'
        for mutation in ('add', 'remove', 'change'):
            with self.subTest(mutation=mutation):
                path.write_bytes(b'original')
                extra = self.root / 'source/new'
                if mutation == 'add':
                    extra.write_bytes(b'added')
                elif mutation == 'remove':
                    path.unlink()
                else:
                    path.write_bytes(b'changed')
                report = self.reject('evidence-stale')
                self.assertEqual(report['nativeClaims'][0]['status'], 'stale')
                if extra.exists():
                    extra.unlink()

    def test_missing_artifact_is_unavailable(self):
        self.candidate()
        (self.root / 'artifact').unlink()
        report = self.reject('evidence-unavailable', 2)
        self.assertEqual(report['nativeClaims'][0]['status'], 'unavailable')

    def test_changed_configuration_is_stale(self):
        self.candidate()
        (self.root / 'artifact').write_bytes(b'changed')
        self.reject('evidence-stale')

    def test_allocation_change_is_stale(self):
        self.candidate()
        self.row['conditions']['scope'] = 'Changed scope'
        self.reject('evidence-stale')

    def test_fallback_entry_fails_accepted_flow(self):
        record = self.candidate()
        record['runtimeCheck']['fallbackEntries'] = 1
        self.reject('evidence-fallback')

    def test_named_fallback_route_still_rejects(self):
        record = self.candidate()
        record['runtimeCheck'].update(fallbackEntries=1, routes=[{
            'routeId': 'route.terminal-handoff', 'reason': 'future-flow', 'count': 1}])
        self.reject('evidence-fallback')

    def test_completed_runtime_only_removes_runtime_blocker(self):
        record = self.candidate()
        record['runtimeCheck']['completed'] = False
        before = self.reject('evidence-runtime')
        record['runtimeCheck']['completed'] = True
        after = self.reject('evidence-coverage')
        self.assertNotIn('evidence-runtime', {e['code'] for e in after['errors']})
        self.assertEqual({e['code'] for e in before['errors']} - {'evidence-runtime'},
                         {e['code'] for e in after['errors']})

    def test_runtime_trace_rejects_payloads(self):
        record = self.candidate()
        record['runtimeCheck']['routes'] = [{'routeId': 'route.future', 'reason': 'future-flow',
                                              'count': 1, 'prompt': 'not allowed'}]
        self.reject('schema')

    def test_incomplete_runtime_check(self):
        self.candidate()['runtimeCheck']['completed'] = False
        self.reject('evidence-runtime')

    def test_schema_html_and_ux_are_not_native_evidence(self):
        record = self.candidate()
        for kind in ('schema-valid', 'html', 'ux-approved'):
            with self.subTest(kind=kind):
                record['kind'] = kind
                self.reject('schema')

    def test_unknown_impact_requires_broader_snapshot(self):
        self.candidate()['dependencies'][0]['impact'] = 'unknown'
        self.reject('evidence-dependencies')

    def test_symlink_cannot_hide_dependencies(self):
        self.candidate()
        (self.root / 'source/link').symlink_to(self.root / 'artifact')
        self.reject('evidence-path')

    def test_pending_empty_routes_are_valid(self):
        self.row.update(implementation='pending', evidenceStatus='pending', evidenceIds=[])
        code, report = self.run_validator()
        self.assertEqual(code, 0, report)
        self.assertEqual(report['nativeClaims'][0]['status'], 'pending')

    def test_pending_implementation_cannot_be_accepted(self):
        self.row['implementation'] = 'pending'
        self.reject('evidence-unsupported')

    def test_fallback_needs_named_route(self):
        self.row.update(implementation='fallback', evidenceStatus='pending', evidenceIds=[])
        self.reject('fallback-route')


    def test_pending_child_rejects_otherwise_complete_parent_claim(self):
        child = copy.deepcopy(self.manifest['capabilities'][0])
        child['id'] = 'capability.status.child'
        self.manifest['capabilities'].append(child)
        child_row = copy.deepcopy(self.row)
        child_row.update(capabilityId=child['id'], implementation='pending',
                         evidenceStatus='pending', evidenceIds=[])
        for index, obligation in enumerate(child_row['evidenceObligations']):
            obligation['id'] = f'obligation.child.case-{index}'
        self.ledger['coverage'].append(child_row)
        self.row['prerequisites'] = [child['id']]
        self.ledger['manifestSha256'] = hashlib.sha256(json.dumps(self.manifest).encode()).hexdigest()
        record = self.candidate()
        # Complete only the validator's input matrix; this is still a negative
        # claim fixture, never a real scenario execution or accepted evidence.
        records = []
        for platform, os_version in [('linux', 'fixture'), ('windows', '10'), ('windows', '11')]:
            for renderer in ('software', 'accelerated'):
                sample = copy.deepcopy(record)
                sample['id'] = f'evidence.fixture-{len(records)}'
                sample['obligationIds'] = [o['id'] for o in self.row['evidenceObligations']]
                sample['environment'].update(platform=platform, osVersion=os_version, renderer=renderer)
                records.append(sample)
        self.row['evidenceIds'] = [r['id'] for r in records]
        self.evidence['records'] = records
        self.reject('evidence-prerequisite')

    def test_native_implementation_alone_is_pending(self):
        self.row.update(evidenceStatus='pending', evidenceIds=[])
        code, report = self.run_validator()
        self.assertEqual(code, 0, report)
        self.assertEqual(report['nativeClaims'][0], {
            'capabilityId': self.row['capabilityId'], 'implementation': 'native', 'status': 'pending'})


if __name__ == '__main__':
    unittest.main()

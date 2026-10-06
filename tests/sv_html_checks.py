#!/usr/bin/env python3
"""Local consumer contracts through the production sync/check CLI."""
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
TOOL = ROOT / 'tools/html_capabilities.py'


class HtmlChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='sv-html-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.consumer = self.root / 'consumer'
        self.consumer.mkdir()
        (self.consumer / 'index.html').write_text('<main>HP</main>')
        self.source = self.root / 'baseline.c'
        self.source.write_text('Receive_hp')
        self.manifest = json.loads((ROOT / 'tests/capabilities/fixtures/valid/manifest.json').read_text())
        self.manifest['sources'][0].update(path='baseline.c', sha256=hashlib.sha256(self.source.read_bytes()).hexdigest())
        self.canonical = self.root / 'manifest.json'
        self.save_manifest()

    def save_manifest(self):
        self.canonical.write_text(json.dumps(self.manifest, indent=3) + '\n')

    def run_tool(self, command, *extra):
        run = subprocess.run([sys.executable, str(TOOL), command,
                              '--canonical', str(self.canonical), '--consumer', str(self.consumer),
                              '--source-root', f'tomenet={self.root}', *map(str, extra)],
                             capture_output=True, text=True)
        self.assertIn(run.returncode, (0, 1, 2), run.stderr)
        return run.returncode, json.loads(run.stdout)

    def pair(self):
        store = self.consumer / '.sv-html'
        pointer = json.loads((store / 'CURRENT').read_text())
        generation = store / pointer['generation']
        return generation, json.loads((generation / 'coverage.json').read_text())

    def candidate(self, change):
        _, ledger = self.pair()
        ledger['history'].append({k: copy.deepcopy(v) for k, v in ledger.items() if k != 'history'})
        change(ledger)
        path = self.root / 'candidate.json'
        path.write_text(json.dumps(ledger))
        return path

    def test_complete_without_flow_evidence_is_rejected(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        candidate = self.candidate(lambda ledger: ledger['coverage'][0].update(implementation='prototype-complete'))
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 1, report)
        self.assertIn('incomplete-flow', {e['code'] for e in report['errors']})
        self.assertEqual(self.pair()[1]['coverage'][0]['implementation'], 'missing')

    def test_exact_snapshot_all_missing_and_read_only_check(self):
        code, report = self.run_tool('sync')
        self.assertEqual(code, 0, report)
        generation, ledger = self.pair()
        self.assertEqual((generation / 'manifest.json').read_bytes(), self.canonical.read_bytes())
        self.assertEqual(ledger['coverage'][0]['implementation'], 'missing')
        before = {str(p): p.read_bytes() for p in self.consumer.rglob('*') if p.is_file()}
        code, report = self.run_tool('check')
        self.assertEqual(code, 0, report)
        self.assertEqual(report['freshness'], 'current')
        self.assertEqual(report['summary']['missing'], 1)
        self.assertEqual(report['summary']['nativeAccepted'], 0)
        self.assertFalse(report['prototypeReady'])
        self.assertEqual(before, {str(p): p.read_bytes() for p in self.consumer.rglob('*') if p.is_file()})

    def test_add_retire_replace_and_preserve_planned_history(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        planned = self.candidate(lambda ledger: ledger['coverage'][0].update(
            implementation='planned', reason='Explicit future HTML work'))
        self.assertEqual(self.run_tool('sync', '--ledger', planned)[0], 0)
        replacement = copy.deepcopy(self.manifest['capabilities'][0])
        replacement['id'] = 'capability.status.new-hp'
        self.manifest['capabilities'][0].update(lifecycle='retired', replacedBy=[replacement['id']])
        self.manifest['capabilities'].append(replacement)
        self.save_manifest()
        code, report = self.run_tool('sync')
        self.assertEqual(code, 0, report)
        _, ledger = self.pair()
        self.assertEqual(len(ledger['coverage']), 1)
        self.assertEqual(ledger['coverage'][0]['implementation'], 'missing')
        self.assertEqual(ledger['coverage'][0]['approvals'], [])
        self.assertEqual(ledger['history'][-1]['coverage'][0]['implementation'], 'planned')
        self.assertEqual(report['summary']['active'], 1)

    def test_failed_publication_keeps_previous_pair_and_recovers(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        original = (self.consumer / '.sv-html/CURRENT').read_bytes()
        self.manifest['capabilities'][0]['title'] = 'Changed title'
        self.save_manifest()
        for boundary in ('after-snapshot', 'before-publish'):
            with self.subTest(boundary=boundary):
                self.assertEqual(self.run_tool('sync', '--fail-at', boundary)[0], 2)
                self.assertEqual((self.consumer / '.sv-html/CURRENT').read_bytes(), original)
                generation, ledger = self.pair()
                self.assertEqual(hashlib.sha256((generation / 'manifest.json').read_bytes()).hexdigest(),
                                 ledger['manifestSha256'])
        interrupted = subprocess.run([sys.executable, str(TOOL), 'sync',
            '--canonical', str(self.canonical), '--consumer', str(self.consumer),
            '--source-root', f'tomenet={self.root}', '--fail-at', 'interrupt-before-publish'],
            capture_output=True, text=True)
        self.assertEqual(interrupted.returncode, 99, interrupted.stderr)
        self.assertEqual((self.consumer / '.sv-html/CURRENT').read_bytes(), original)
        self.assertEqual(self.run_tool('sync')[0], 0)
        self.assertEqual(self.run_tool('check')[0], 0)

    def test_freshness_catches_changed_added_removed_sources_without_git(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        for change in ('edit', 'add', 'remove'):
            with self.subTest(change=change):
                path = self.consumer / 'index.html'
                path.write_text('<main>HP</main>')
                if change == 'edit':
                    path.write_text('<main>Changed</main>')
                elif change == 'add':
                    (self.consumer / 'new.js').write_text('new feature')
                else:
                    path.unlink()
                code, report = self.run_tool('check')
                self.assertEqual(code, 1, report)
                self.assertIn('source-stale', {e['code'] for e in report['errors']})
                (self.consumer / 'new.js').unlink(missing_ok=True)
        (self.consumer / 'index.html').write_text('<main>HP</main>')
        self.source.write_text('Receive_hp changed without HEAD')
        code, report = self.run_tool('check')
        self.assertEqual(code, 1, report)
        self.assertIn('source-digest', {e['code'] for e in report['errors']})

    def test_unavailable_canonical_is_never_fresh(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        self.canonical.unlink()
        code, report = self.run_tool('check')
        self.assertEqual(code, 2, report)
        self.assertEqual(report['freshness'], 'unavailable')

    def test_source_revert_preserves_chronological_history(self):
        for text in ('A', 'B', 'A', 'C'):
            (self.consumer / 'index.html').write_text(text)
            code, report = self.run_tool('sync')
            self.assertEqual(code, 0, report)
        self.assertEqual(len(self.pair()[1]['history']), 3)

    def test_invalid_canonical_cannot_replace_published_pair(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        before = (self.consumer / '.sv-html/CURRENT').read_bytes()
        self.manifest['relations'].append({'kind': 'capability-exposed-on-surface',
            'from': self.manifest['capabilities'][0]['id'], 'to': 'surface.unknown'})
        self.save_manifest()
        code, report = self.run_tool('sync')
        self.assertEqual(code, 1, report)
        self.assertIn('unknown-id', {e['code'] for e in report['errors']})
        self.assertEqual((self.consumer / '.sv-html/CURRENT').read_bytes(), before)

    def test_ledger_negative_integrity_cases(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        cases = [
            ('digest', lambda x: x.update(manifestSha256='0'*64), 'manifest-digest'),
            ('schema', lambda x: x.update(nativeAccepted=1), 'schema'),
            ('missing', lambda x: x.update(coverage=[]), 'missing-coverage'),
            ('unknown', lambda x: x['coverage'][0].update(capabilityId='capability.unknown'), 'unknown-id'),
            ('history', lambda x: x.update(history=[]), 'claim-history'),
            ('approval-is-not-implementation', lambda x: x['coverage'][0].update(implementation='ux-approved'), 'schema'),
        ]
        for name, mutate, diagnostic in cases:
            with self.subTest(name=name):
                path = self.candidate(mutate)
                code, report = self.run_tool('sync', '--ledger', path)
                self.assertEqual(code, 1, report)
                self.assertIn(diagnostic, {e['code'] for e in report['errors']})
        generation, _ = self.pair()
        (generation / 'manifest.json').write_bytes(b'{}')
        code, report = self.run_tool('check')
        self.assertEqual(code, 1, report)
        self.assertIn('pair-digest', {e['code'] for e in report['errors']})

    def test_malformed_candidate_reports_invalid_json_without_traceback(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        for candidate in ([], {}, {'coverage': 'wrong type'}, None):
            with self.subTest(candidate=candidate):
                path = self.root / 'malformed-candidate.json'
                path.write_text(json.dumps(candidate))
                code, report = self.run_tool('sync', '--ledger', path)
                self.assertEqual(code, 1, report)
                self.assertIn('schema', {e['code'] for e in report['errors']})

    def scoped_candidate(self):
        """Synthetic checker fixture, explicitly not real HTML acceptance/approval."""
        path = self.consumer / 'index.html'
        source = {'path': 'index.html', 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                  'anchor': 'HP', 'role': 'ux'}
        location = {'path': 'index.html', 'anchor': 'HP'}
        obligations = ('entry', 'mouse', 'keyboard', 'selection', 'confirm', 'cancel', 'result',
            'error', 'focus', 'normal', 'empty', 'boundary', 'loading', 'unavailable', 'resize',
            'minimum-scale', 'fallback', 'persistence', 'reset', 'chat', 'blocking')
        def add_scope(ledger):
            row = ledger['coverage'][0]
            row['mapping'] = {'scope': 'Synthetic fixture for checker only', 'entityIds': [],
                'contextIds': [], 'bindingIds': [], 'gates': ['synthetic'], 'sources': [source],
                'entrypoint': {key: location for key in ('page', 'module', 'registration')}}
            row['scenarios'] = [{'id': 'scenario.fixture', 'steps': ['Exercise synthetic checker fixture'],
                'contextId': None, 'bindingId': None, 'keyset': 'normal', 'platform': 'linux',
                'gate': 'synthetic', 'fixture': source, 'expectations': [{'id': key, 'expected': 'Synthetic outcome',
                    'applicability': 'required', 'reason': 'Checker fixture', 'source': source}
                    for key in obligations]}]
        candidate = self.candidate(add_scope)
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 0, report)
        fingerprint = report['claims'][0]['contractSha256']
        artifacts = self.consumer / '.sv-html-artifacts'
        artifacts.mkdir(exist_ok=True)
        return fingerprint, obligations

    def add_attestations(self, fingerprint, obligations, approval=False, implementation='prototype-complete'):
        environment = {key: 'synthetic checker fixture' for key in
            ('browser', 'page', 'transport', 'viewport', 'keyset', 'platform', 'gates')}
        environment.update(page='index.html', keyset='normal', platform='linux', gates='synthetic')
        identifier = self.manifest['capabilities'][0]['id']
        artifact = self.consumer / '.sv-html-artifacts/scenario.json'
        artifact.write_text(json.dumps({'kind': 'html-scenario', 'capabilityId': identifier,
            'scenarioId': 'scenario.fixture', 'contractSha256': fingerprint, 'environment': environment,
            'observedAt': 'fixture-time', 'observer': 'test fixture', 'results': {key: 'pass' for key in obligations}}))
        decision = self.consumer / '.sv-html-artifacts/decision.json'
        decision.write_text(json.dumps({'actorKind': 'human', 'decision': 'ux-approved',
            'capabilityId': identifier, 'contractSha256': fingerprint, 'approver': 'synthetic human fixture',
            'approvedAt': 'fixture-time', 'decisionText': 'Fixture only; not an actual UX approval'}))
        def attest(ledger):
            row = ledger['coverage'][0]
            row['implementation'] = implementation
            row['evidence'] = [{'id': 'evidence.fixture', 'scenarioId': 'scenario.fixture',
                'contractSha256': fingerprint, 'observedAt': 'fixture-time', 'observer': 'test fixture',
                'environment': environment, 'artifact': {'path': '.sv-html-artifacts/scenario.json',
                'sha256': hashlib.sha256(artifact.read_bytes()).hexdigest()}}]
            if approval:
                row['approvals'] = [{'id': 'approval.fixture', 'claim': 'ux-approved',
                    'contractSha256': fingerprint, 'approver': 'synthetic human fixture',
                    'approvedAt': 'fixture-time', 'decision': {'path': '.sv-html-artifacts/decision.json',
                    'sha256': hashlib.sha256(decision.read_bytes()).hexdigest()}}]
        return self.candidate(attest)

    def test_scoped_complete_is_independent_of_approval_and_native_parity(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        fingerprint, obligations = self.scoped_candidate()
        candidate = self.add_attestations(fingerprint, obligations)
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 0, report)
        self.assertTrue(report['prototypeReady'])
        self.assertEqual(report['summary']['prototypeComplete'], 1)
        self.assertEqual(report['summary']['uxApproved'], 0)
        self.assertEqual(report['summary']['nativeAccepted'], 0)
        (self.consumer / 'index.html').write_text('<main>HP changed</main>')
        code, report = self.run_tool('sync')
        self.assertEqual(code, 0, report)
        self.assertFalse(report['prototypeReady'])
        self.assertEqual(report['summary']['staleClaims'], 1)
        self.assertEqual(self.pair()[1]['coverage'][0]['implementation'], 'prototype-complete')
        self.assertEqual(report['summary']['prototypeComplete'], 0)

    def test_evidence_environment_must_match_scenario_selection(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        fingerprint, obligations = self.scoped_candidate()
        candidate = self.add_attestations(fingerprint, obligations)
        ledger = json.loads(candidate.read_text())
        row = ledger['coverage'][0]
        artifact = self.consumer / '.sv-html-artifacts/scenario.json'
        data = json.loads(artifact.read_text())
        data['environment']['page'] = 'unrelated-login-page.html'
        artifact.write_text(json.dumps(data))
        row['evidence'][0]['environment'] = data['environment']
        row['evidence'][0]['artifact']['sha256'] = hashlib.sha256(artifact.read_bytes()).hexdigest()
        candidate.write_text(json.dumps(ledger))
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 1, report)
        self.assertIn('evidence-scope', {e['code'] for e in report['errors']})

    def test_complete_requires_canonical_variants_and_drift_preserves_claim(self):
        base = copy.deepcopy(self.manifest['capabilities'][0])
        self.manifest['actions'] = [{**base, 'id': 'action.fixture'}]
        self.manifest['inputContexts'] = [{**base, 'id': 'context.fixture'}]
        binding = {**base, 'id': 'binding.fixture', 'actionId': 'action.fixture',
                   'contextId': 'context.fixture', 'gesture': 'h',
                   'keyset': ['normal'], 'platform': ['linux']}
        self.manifest['bindings'] = [binding]
        self.manifest['relations'] = [{'kind': 'capability-uses-action',
            'from': base['id'], 'to': 'action.fixture'}]
        self.save_manifest()
        self.assertEqual(self.run_tool('sync')[0], 0)
        _, obligations = self.scoped_candidate()
        def scope(ledger):
            row = ledger['coverage'][0]
            row['mapping'].update(entityIds=['action.fixture'], contextIds=['context.fixture'],
                                  bindingIds=['binding.fixture'])
            row['scenarios'][0].update(contextId='context.fixture', bindingId='binding.fixture')
        candidate = self.candidate(scope)
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 0, report)
        candidate = self.add_attestations(report['claims'][0]['contractSha256'], obligations)
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 0, report)
        self.assertTrue(report['prototypeReady'])
        # Adding a canonical variant preserves but invalidates the old declaration.
        binding['keyset'].append('roguelike')
        self.save_manifest()
        code, report = self.run_tool('sync')
        self.assertEqual(code, 0, report)
        self.assertEqual(report['summary']['prototypeComplete'], 0)
        self.assertEqual(report['summary']['staleClaims'], 1)
        # Merely updating the artifact fingerprint cannot cover the missing variant.
        candidate = self.add_attestations(report['claims'][0]['contractSha256'], obligations)
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 1, report)
        self.assertIn('incomplete-scope', {e['code'] for e in report['errors']})

    def test_approval_can_exist_without_complete_and_becomes_stale(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        fingerprint, obligations = self.scoped_candidate()
        candidate = self.add_attestations(fingerprint, obligations, approval=True, implementation='missing')
        code, report = self.run_tool('sync', '--ledger', candidate)
        self.assertEqual(code, 0, report)
        self.assertEqual(report['summary']['uxApproved'], 1)
        self.assertEqual(report['summary']['prototypeComplete'], 0)
        self.manifest['capabilities'][0]['title'] = 'New title invalidates conservative claims'
        self.save_manifest()
        code, report = self.run_tool('sync')
        self.assertEqual(code, 0, report)
        self.assertEqual(report['summary']['uxApproved'], 0)
        self.assertEqual(len(self.pair()[1]['coverage'][0]['approvals']), 1)

    def test_failed_or_partial_flow_and_generated_approval_cannot_count(self):
        self.assertEqual(self.run_tool('sync')[0], 0)
        fingerprint, obligations = self.scoped_candidate()
        for mode in ('cancel', 'fail', 'generated-approval', 'fixture-behavior', 'unknown-entity'):
            with self.subTest(mode=mode):
                candidate = self.add_attestations(fingerprint, obligations, approval=True)
                ledger = json.loads(candidate.read_text())
                row = ledger['coverage'][0]
                if mode in ('cancel', 'fail'):
                    path = self.consumer / '.sv-html-artifacts/scenario.json'
                    data = json.loads(path.read_text())
                    if mode == 'cancel':
                        del data['results']['cancel']
                    else:
                        data['results']['result'] = 'fail'
                    path.write_text(json.dumps(data))
                    row['evidence'][0]['artifact']['sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
                elif mode == 'generated-approval':
                    path = self.consumer / '.sv-html-artifacts/decision.json'
                    data = json.loads(path.read_text())
                    data['actorKind'] = 'agent'
                    path.write_text(json.dumps(data))
                    row['approvals'][0]['decision']['sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
                elif mode == 'fixture-behavior':
                    row['mapping']['sources'][0]['role'] = 'behavior'
                else:
                    row['mapping']['entityIds'] = ['surface.unknown']
                candidate.write_text(json.dumps(ledger))
                code, report = self.run_tool('sync', '--ledger', candidate)
                self.assertEqual(code, 1, report)
                self.assertFalse(report.get('prototypeReady', False))



if __name__ == '__main__':
    unittest.main()

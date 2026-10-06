#!/usr/bin/env python3
"""Review policy through the production checker; synthetic records are not approvals."""
import copy
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ReviewChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.log = self.root / 'native.log'
        self.log.write_text('SV review build=linux-fixture video=wayland renderer=software '
                            'output=1280x960 logical=1024.000x768.000 display_scale=1.250\n')
        self.host = self.root / 'host-provenance.json'
        self.host.write_text('fixture dependency bytes')
        self.identity = {'build': 'linux-fixture', 'platform': 'linux', 'osVersion': 'fixture OS',
                         'renderer': 'software', 'executableSha256': 'a' * 64,
                         'resources': {'xtra/font/CascadiaMono-Regular.ttf': 'b' * 64, 'xtra/font/16x24x.pcf': 'b' * 64}}
        self.review = {'schemaVersion': 1, 'identity': self.identity, 'nativeExit': 0,
                      'startedAt': '2026-01-01T00:00:00+00:00',
                      'reviewer': 'Test fixture, not a real reviewer',
                      'reviewedAt': datetime.now(timezone.utc).isoformat(), 'decision': 'approved',
                      'physicalConditions': 'Fixture display, not an actual observation',
                      'hostProvenance': {'path': self.host.name, 'sha256': hashlib.sha256(self.host.read_bytes()).hexdigest()},
                      'actions': {name: {'result': 'pass', 'notes': 'Fixture observation'} for name in
                                  ('readability', 'answer', 'cancel', 'focus', 'resizePending',
                                   'surfaceRestore', 'macro', 'sessionRestart', 'perceivedResponse')},
                      'log': {'path': str(self.log), 'sha256': hashlib.sha256(self.log.read_bytes()).hexdigest()}}
        self.evidence = {'records': [{'checkpoint': 'stage-a', 'build': 'linux-fixture',
                                    'environment': {'platform': 'linux', 'osVersion': 'fixture OS',
                                                    'renderer': 'software'},
                                    'executable': {'sha256': 'a' * 64},
                                    'configuration': {'path': 'src/.sv-build/linux/fixture/build.txt'},
                                    'hostProvenance': {'sha256': hashlib.sha256(self.host.read_bytes()).hexdigest()},
                                    'dependencies': [{'repository': 'tomenet', 'path': 'lib',
                                                      'files': {'xtra/font/CascadiaMono-Regular.ttf': 'b' * 64, 'xtra/font/16x24x.pcf': 'b' * 64}}]}]}

    def check(self, expected, review=True):
        evidence = self.root / 'evidence.json'
        evidence.write_text(json.dumps(self.evidence))
        command = [sys.executable, '-B', str(ROOT / 'tools/stage_a_review.py'), 'check',
                   '--evidence', str(evidence)]
        if review:
            path = self.root / 'review.json'
            path.write_text(json.dumps(self.review))
            command += ['--review', str(path)]
        run = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(run.returncode, 0 if expected == 'approved' else 2, run.stderr + run.stdout)
        result = json.loads(run.stdout)
        self.assertEqual(result['status'], expected, result)
        return result

    def test_only_explicit_current_positive_review_is_approved(self):
        self.check('pending', review=False)
        self.review['decision'] = 'pending'
        self.check('pending')
        self.review['decision'] = 'rejected'
        self.check('rejected')
        self.review['decision'] = 'approved'
        self.check('approved')

    def test_stale_binary_environment_or_log_cannot_approve(self):
        for key in ('executableSha256', 'osVersion', 'renderer', 'build'):
            with self.subTest(key=key):
                saved = self.identity[key]
                self.identity[key] = 'changed'
                self.check('invalid')
                self.identity[key] = saved
        self.log.write_text('changed')
        self.check('invalid')

    def test_incomplete_negative_or_undated_observations_cannot_approve(self):
        original = copy.deepcopy(self.review)
        for field, value in (('reviewer', ''), ('reviewedAt', 'tomorrow'),
                             ('reviewedAt', '2999-01-01T00:00:00+00:00'),
                             ('physicalConditions', ''), ('schemaVersion', 2)):
            with self.subTest(field=field, value=value):
                self.review = copy.deepcopy(original)
                self.review[field] = value
                self.check('invalid')
        for name in original['actions']:
            for value in (None, {'result': 'fail', 'notes': 'bad'}, {'result': 'pass', 'notes': ''}):
                self.review = copy.deepcopy(original)
                if value is None:
                    del self.review['actions'][name]
                else:
                    self.review['actions'][name] = value
                self.check('invalid')

    def test_crash_changed_font_or_review_before_launch_is_invalid(self):
        original = copy.deepcopy(self.review)
        self.review['nativeExit'] = 1
        self.check('invalid')
        self.review = copy.deepcopy(original)
        self.review['identity']['resources']['xtra/font/CascadiaMono-Regular.ttf'] = 'c' * 64
        self.check('invalid')
        self.review = copy.deepcopy(original)
        self.review['reviewedAt'] = '2025-01-01T00:00:00+00:00'
        self.check('invalid')

    def test_human_approval_never_waives_failed_automation(self):
        sys.path.insert(0, str(ROOT / 'tools'))
        from run_stage_a import acceptance_status
        report = {'checks': [{'result': 'pass', 'returncode': 0}],
                  'checkpoint': {'status': 'passed'}, 'humanReview': {'status': 'approved'}}
        self.assertEqual(acceptance_status(report), 'accepted')
        report['humanReview']['status'] = 'pending'
        self.assertEqual(acceptance_status(report), 'pending')
        for status in ('invalid', 'rejected'):
            report['humanReview']['status'] = status
            self.assertEqual(acceptance_status(report), 'blocked')
        report['humanReview']['status'] = 'approved'
        report['checks'][0] = {'result': 'fail', 'returncode': 1}
        self.assertEqual(acceptance_status(report), 'blocked')
        report['checks'][0] = {'result': 'pass', 'returncode': 0}
        report['checkpoint']['status'] = 'blocked'
        self.assertEqual(acceptance_status(report), 'blocked')

    def test_opengl_review_matches_accelerated_evidence_but_retains_actual_backend(self):
        self.identity['renderer'] = 'opengl'
        self.log.write_text(self.log.read_text().replace('renderer=software', 'renderer=opengl'))
        self.review['log']['sha256'] = hashlib.sha256(self.log.read_bytes()).hexdigest()
        self.evidence['records'][0]['environment']['renderer'] = 'accelerated'
        result = self.check('approved')
        self.assertEqual(result['identity']['renderer'], 'opengl')

    def test_changed_host_dependencies_invalidate_review(self):
        self.evidence['records'][0]['hostProvenance']['sha256'] = 'd' * 64
        self.check('invalid')
        self.evidence['records'][0]['hostProvenance']['sha256'] = self.review['hostProvenance']['sha256']
        self.host.write_text('updated SDL library')
        self.check('invalid')


if __name__ == '__main__':
    unittest.main()

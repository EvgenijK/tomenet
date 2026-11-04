#!/usr/bin/env python3
"""Fail-closed production evidence composition; log strings never complete a scope."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from run_stage_a import measured_runtime


class RuntimeProducerChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.path = self.root / 'runtime.jsonl'
        self.checks = [{'name': 'software-request_native', 'runtimeReport': self.path.name, 'result': 'pass'}]
        self.row = {'scenario': 'scenario.stage-a.request', 'completed': True, 'fallbackEntries': 0, 'routes': []}

    def result(self, rows):
        self.path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        return measured_runtime(self.root, self.checks, 'scenario.composite')

    def test_complete_and_incomplete(self):
        self.assertTrue(self.result([self.row])['completed'])
        self.row['completed'] = False
        self.assertFalse(self.result([self.row])['completed'])

    def test_no_log_or_exit_substitute(self):
        self.assertFalse(measured_runtime(self.root, self.checks, 'scenario.composite')['completed'])
        self.path.write_text('fallback_routes=0\n')
        self.assertFalse(measured_runtime(self.root, self.checks, 'scenario.composite')['completed'])
        self.assertFalse(self.result([])['completed'])
        self.assertFalse(self.result([self.row, self.row])['completed'])
        self.row['scenario'] = 'scenario.stage-a.hp'
        self.assertFalse(self.result([self.row])['completed'])

    def test_measured_entries_preserved(self):
        self.row.update(fallbackEntries=2, routes=[{'routeId': 'route.terminal-handoff',
                                                  'reason': 'future-flow', 'count': 2}])
        result = self.result([self.row])
        self.assertEqual(result['fallbackEntries'], 2)
        self.assertEqual(result['routes'], self.row['routes'])
        self.row['fallbackEntries'] = 0
        self.assertFalse(self.result([self.row])['completed'])

    def test_timing_negative_cannot_substitute_for_positive(self):
        self.checks[0]['name'] = 'software-timing_native'
        self.row['scenario'] = 'scenario.stage-a.timing'
        delayed = dict(self.row, scenario='scenario.stage-a.timing-delayed', completed=False)
        self.assertTrue(self.result([self.row, delayed])['completed'])
        self.assertFalse(self.result([delayed])['completed'])
        delayed['completed'] = True
        self.assertFalse(self.result([self.row, delayed])['completed'])


if __name__ == '__main__':
    unittest.main()

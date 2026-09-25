#!/usr/bin/env python3
"""Regression checks at the production planning-checker file-input seam."""
import contextlib
import copy
import importlib.util
import io
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch

PATH = Path(__file__).with_name("check-plan.py")
SPEC = importlib.util.spec_from_file_location("stage_b_checker", PATH)
CHECKER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CHECKER)


class SnapshotChecks(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="sv-b-plan-check-")
        self.addCleanup(self.directory.cleanup)
        self.canonical = Path(self.directory.name)
        for name in CHECKER.REQUIRED_SNAPSHOTS:
            destination = self.canonical / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(CHECKER.CANONICAL / name, destination)
        self.plan = CHECKER.read_json(CHECKER.PLAN / "coverage.json")

    def run_check(self):
        original_read = CHECKER.read_json
        def read_input(path):
            if path == CHECKER.PLAN / "coverage.json":
                return copy.deepcopy(self.plan)
            return original_read(path)
        stream = io.StringIO()
        with patch.object(CHECKER, "CANONICAL", self.canonical), \
             patch.object(CHECKER, "read_json", side_effect=read_input), \
             contextlib.redirect_stdout(stream):
            code = CHECKER.check()
        return code, json.loads(stream.getvalue())

    def assert_rejected(self):
        code, report = self.run_check()
        self.assertEqual(code, 1, report)
        self.assertEqual(report["status"], "invalid")
        self.assertFalse(report["canonicalFilesUnchanged"], report)
        self.assertTrue(report["errors"], report)

    def mutate_manifest(self):
        path = self.canonical / "manifest.json"
        document = json.loads(path.read_text())
        document["capabilities"][0]["description"] += " Changed contract."
        path.write_text(json.dumps(document))

    def mutate_scenario(self):
        path = self.canonical / "reconciliation.json"
        document = json.loads(path.read_text())
        document["scenarios"][0]["description"] += " Changed scenario."
        path.write_text(json.dumps(document))

    def test_complete_snapshot_passes(self):
        code, report = self.run_check()
        self.assertEqual(code, 0, report)
        self.assertTrue(report["canonicalFilesUnchanged"], report)

    def test_empty_snapshot_fails(self):
        self.plan["canonicalSha256"] = {}
        self.assert_rejected()

    def test_each_missing_snapshot_fails(self):
        original = dict(self.plan["canonicalSha256"])
        for name in CHECKER.REQUIRED_SNAPSHOTS:
            with self.subTest(name=name):
                self.plan["canonicalSha256"] = dict(original)
                del self.plan["canonicalSha256"][name]
                self.assert_rejected()

    def test_extra_snapshot_fails(self):
        self.plan["canonicalSha256"]["unexpected.json"] = "0" * 64
        self.assert_rejected()

    def test_malformed_digest_fails(self):
        for value in (None, 123, [], "", "0" * 63, "g" * 64):
            with self.subTest(value=value):
                self.plan["canonicalSha256"]["manifest.json"] = value
                self.assert_rejected()

    def test_changed_manifest_with_missing_hash_fails(self):
        del self.plan["canonicalSha256"]["manifest.json"]
        self.mutate_manifest()
        self.assert_rejected()

    def test_changed_scenario_with_missing_hash_fails(self):
        del self.plan["canonicalSha256"]["reconciliation.json"]
        self.mutate_scenario()
        self.assert_rejected()

    def test_changed_manifest_with_complete_hash_map_fails(self):
        self.mutate_manifest()
        self.assert_rejected()

    def test_changed_scenario_with_complete_hash_map_fails(self):
        self.mutate_scenario()
        self.assert_rejected()


if __name__ == "__main__":
    unittest.main()

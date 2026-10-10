#!/usr/bin/env python3
"""Collect B-024 controlled-peer evidence; never substitute for controller gates."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import sv_birth_input_seal as seal

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = "0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef"
SUITES = (
    ("ordinary", "sv_birth_complete_checks.py", "SV_BIRTH_EVIDENCE"),
    ("versions", "sv_birth_version_checks.py", "SV_BIRTH_EVIDENCE"),
    ("dedicated", "sv_birth_dedicated_checks.py", "SV_DEDICATED_EVIDENCE"),
    ("submission", "sv_birth_single_submission_checks.py", "SV_BIRTH_EVIDENCE"),
    ("replies", "sv_birth_reply_checks.py", "SV_BIRTH_EVIDENCE"),
    ("guide-motd", "sv_guide_motd_checks.py", "SV_GUIDE_EVIDENCE"),
)


def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()


def fingerprint(path):
    data = path.read_bytes()
    return {"size": len(data), "sha256": hashlib.sha256(data).hexdigest()}


def require_inventory(consumed, sources):
    missing = set(consumed) - set(sources)
    if missing:
        raise RuntimeError(f"Consumed repository inputs absent from inventory: {sorted(missing)}")


def verify_inputs(report):
    errors = []
    if git("rev-parse", "HEAD") != report["tested_head"]:
        errors.append("HEAD changed")
    if report["contract_digest"] != CONTRACT:
        errors.append("Contract changed")
    if set(seal.binding()["inputPaths"]) != set(report["sources"]):
        errors.append("Binding inventory changed")
    for name, expected in report["sources"].items():
        try:
            if fingerprint(ROOT / name) != expected:
                errors.append(f"Repository input changed: {name}")
        except OSError:
            errors.append(f"Repository input missing: {name}")
    try:
        after = seal.runtime(report["runtime_before"]["external_files"])
        if after != report["runtime_before"]:
            errors.append("Tool/runtime identity or external dependency changed")
        report["runtime_after"] = after
    except Exception as error:
        errors.append(f"Tool/runtime unavailable: {error}")
    absence = report["oracle_provenance"]["font_mapping_absence"]
    if any((not (ROOT / name).exists()) != expected for name, expected in absence.items()):
        errors.append("Baseline font mapping absence changed")
    return not errors, errors


def finish(report, save):
    report["finished_utc"] = datetime.now(timezone.utc).isoformat()
    report["inputs_unchanged"], report["input_errors"] = verify_inputs(report)
    if any(suite["status"] != "passed" for suite in report["suites"]):
        report["inputs_unchanged"] = False
        report["input_errors"].append("Failed producer cannot establish complete unchanged input coverage")
    save()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="new or empty evidence directory")
    parser.add_argument("--integrity-only", action="store_true", help="discover all inputs, capture ordinary suite only; no final acceptance")
    parser.add_argument("--verify-inputs", action="store_true", help="compare retained before records with current inputs")
    args = parser.parse_args()
    if args.verify_inputs:
        report = json.loads((args.output / "manifest.json").read_text())
        unchanged, errors = verify_inputs(report)
        print(json.dumps({"inputs_unchanged": unchanged, "errors": errors}))
        return 0 if unchanged else 1
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if any(output.iterdir()):
        parser.error("output must be empty: old captures cannot count as fresh evidence")

    # A caller's focused-case filters must not silently narrow this full matrix.
    env = {key: value for key, value in os.environ.items()
           if not key.startswith(("SV_BIRTH_", "SV_DEDICATED_", "SV_SUBMISSION_",
                                  "SV_REPLY_", "SV_GUIDE_", "SV_MOTD_"))}
    env.update(SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
               PYTHONDONTWRITEBYTECODE="1")
    sys.dont_write_bytecode = True
    declared = seal.binding()
    names = declared["inputPaths"]
    if len(names) != len(set(names)) or any(not (ROOT / name).is_file() for name in names):
        raise RuntimeError("Inventory must enumerate unique existing files")
    tested_head = git("rev-parse", "HEAD")
    sources = {name: fingerprint(ROOT / name) for name in names}
    discovery_dir = output / "discovery"
    discovery_dir.mkdir()
    dependencies, consumed, external = seal.discover(SUITES, env, discovery_dir)
    require_inventory(consumed, sources)
    runtime_before = seal.runtime(external)
    if any(value.get("missing") for value in runtime_before["external_files"].values()):
        raise RuntimeError("Missing external dependency identity")
    report = {
        "contract_digest": CONTRACT,
        "tested_head": tested_head,
        "branch": git("branch", "--show-current"),
        "worktree_status": git("status", "--porcelain"),
        "started_utc": datetime.now(timezone.utc).isoformat(),
        "seam": "sv_endpoint_run(SvEndpointOptions)",
        "backend": "native SDL dummy/software; not an OS-window capture",
        "sources": sources,
        "capture_scope": "focused-integrity" if args.integrity_only else "full-108",
        "dependency_discovery": dependencies,
        "runtime_before": runtime_before,
        "oracle_provenance": seal.provenance(sources),
        "inputs_unchanged": False,
        "artifacts": {},
        "suites": [],
        "gates": {
            "sv-build": {"status": "pending", "command": "make -s -C src -f makefile.sv tomenet-sv",
                         "reason": "orchestrator-owned; not run by this focused collector"},
            "sv-core": {"status": "pending", "command": "bash -s -- core < .sandcastle/checks.sh",
                        "reason": "configured runner exists; full gate is orchestrator-owned"},
            "controller-review": {"status": "pending", "reason": "requires independent controller verdict and fresh final HEAD audit"},
            "P1-entry-evidence": {"status": "deferred", "owner": "SV-B-025",
                "reason": "The assigned source places end-to-end verification in the downstream SV-B-025 ticket; this checkpoint covers SV-B-024 birth completion."},
        },
    }
    manifest = output / "manifest.json"

    def save():
        manifest.write_text(json.dumps(report, indent=2) + "\n")

    save()
    for label, runner, variable in (SUITES[:1] if args.integrity_only else SUITES):
        directory = output / label
        directory.mkdir()
        command = [sys.executable, str(ROOT / "tests" / runner)]
        print(f"Collecting {label}: {runner}", flush=True)
        with (output / (label + ".runner.log")).open("w") as log:
            import contextlib
            returncode = 0
            observation = {}
            try:
                with contextlib.redirect_stdout(log), contextlib.redirect_stderr(log):
                    observation = seal.observe(runner, dict(env, **{variable: str(directory)}), output / "observations")
                require_inventory(observation["repository_reads"], sources)
                for dependency in observation.get("actual_dependencies", []):
                    missing_headers = set(dependency["external"]) - set(runtime_before["external_files"])
                    if missing_headers:
                        raise RuntimeError(f"Unsealed external headers: {sorted(missing_headers)}")
                loaded = set(observation.get("linked_libraries", [])) | set(observation["loaded_libraries"])
                sealed_external = {str(Path(path).resolve()) for path in runtime_before["external_files"]}
                missing = {str(Path(path).resolve()) for path in loaded} - sealed_external
                if missing:
                    raise RuntimeError(f"Unsealed loaded libraries: {sorted(missing)}")
            except Exception as error:
                print(f"Producer failed: {error}", file=log)
                returncode = getattr(error, "returncode", 1)
            case_logs = sorted(str(path.relative_to(output)) for path in directory.glob("*.log"))
            if len(case_logs) != declared["expectedSuites"][label]:
                print(f"Wrong case count: {len(case_logs)}", file=log)
                returncode = 1
        report["suites"].append({"name": label, "command": command,
                                 "environment": {variable: str(directory)},
                                 "execution": "runpy.run_path in collector Python; subprocess compiler/native commands retained in observation",
                                 "returncode": returncode, "caseCount": len(case_logs),
                                 "caseLogs": case_logs, "observation": observation,
                                 "status": "passed" if returncode == 0 else "failed"})
        report["artifacts"] = {str(path.relative_to(output)): fingerprint(path)
                               for path in sorted(output.rglob("*"))
                               if path.is_file() and path != manifest}
        save()
        if returncode:
            finish(report, save)
            print(f"Failed {label}; see {output / (label + '.runner.log')}", flush=True)
            return 1
    finish(report, save)
    if not report["inputs_unchanged"]:
        print("Inputs changed during capture; evidence needs a fresh run", flush=True)
        return 1
    print("Focused evidence captured; full gates/controller acceptance remain pending", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())

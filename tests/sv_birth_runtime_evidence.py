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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="new or empty evidence directory")
    args = parser.parse_args()
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
    inputs = [ROOT / name for name in git("ls-files", "src/client/sv", "src/common/md5.c",
              "tests/sv", "tests/sv_birth*", "tests/sv_guide_motd_checks.py",
              ".sandcastle/checks.sh").splitlines()]
    inputs.append(Path(__file__).resolve())
    sources = {str(path.relative_to(ROOT)): fingerprint(path) for path in inputs}
    report = {
        "contract_digest": CONTRACT,
        "tested_head": git("rev-parse", "HEAD"),
        "branch": git("branch", "--show-current"),
        "started_utc": datetime.now(timezone.utc).isoformat(),
        "seam": "sv_endpoint_run(SvEndpointOptions)",
        "backend": "native SDL dummy/software; not an OS-window capture",
        "sources": sources,
        "suites": [],
        "gates": {
            "sv-build": {"status": "pending", "command": "make -s -C src -f makefile.sv tomenet-sv",
                         "reason": "orchestrator-owned; not run by this focused collector"},
            "sv-core": {"status": "pending", "command": "bash -s -- core < .sandcastle/checks.sh",
                        "reason": "configured runner exists; full gate is orchestrator-owned"},
            "controller-review": {"status": "pending", "reason": "requires independent controller verdict and fresh final HEAD audit"},
            "P1-entry-evidence": {"status": "deferred", "owner": "SV-B-025"},
        },
    }
    manifest = output / "manifest.json"

    def save():
        manifest.write_text(json.dumps(report, indent=2) + "\n")

    save()
    for label, runner, variable in SUITES:
        directory = output / label
        directory.mkdir()
        command = [sys.executable, str(ROOT / "tests" / runner)]
        print(f"Collecting {label}: {runner}", flush=True)
        with (output / (label + ".runner.log")).open("w") as log:
            result = subprocess.run(command, cwd=ROOT, env=dict(env, **{variable: str(directory)}),
                                    stdout=log, stderr=subprocess.STDOUT)
        report["suites"].append({"name": label, "command": command,
                                 "environment": {variable: str(directory)},
                                 "returncode": result.returncode,
                                 "status": "passed" if result.returncode == 0 else "failed"})
        report["artifacts"] = {str(path.relative_to(output)): fingerprint(path)
                               for path in sorted(output.rglob("*"))
                               if path.is_file() and path != manifest}
        save()
        if result.returncode:
            print(f"Failed {label}; see {output / (label + '.runner.log')}", flush=True)
            return 1
    report["finished_utc"] = datetime.now(timezone.utc).isoformat()
    report["inputs_unchanged"] = (git("rev-parse", "HEAD") == report["tested_head"] and
                                  all(fingerprint(ROOT / name) == value
                                      for name, value in sources.items()))
    save()
    if not report["inputs_unchanged"]:
        print("Inputs changed during capture; evidence needs a fresh run", flush=True)
        return 1
    print("Focused evidence captured; full gates/controller acceptance remain pending", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())

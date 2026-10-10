# Ticket 09 exact-HEAD evidence recovery

Finding D-9496281ca15e0741 remains **open** pending independent controller
acceptance and final-HEAD build/core receipts. Author capture success and author
integrity readback do not satisfy that gate.

Contract digest: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.
Reviewed HEAD: `9a98594669cba32bef9d98236cf8f0902a27eba0`.
Worker starting HEAD: `72fac69a58b7875f1d812980312acaa63573d4c9`.
The documentation commit changes the worker HEAD; it is explicitly a separate
capture target. No historical receipt is relabelled.

## Persistent discovery

Fresh evidence lives outside the Git tree at:

```text
/home/agent/sv-b024-final-audit/<exact-HEAD>/runtime/manifest.json
/home/agent/sv-b024-final-audit/<exact-HEAD>/author-readback.json
/home/agent/sv-b024-final-audit/<exact-HEAD>/capture-receipt.json
```

Obtain the worker final HEAD with `git rev-parse HEAD` on the assigned branch,
then read that exact directory. The reviewed-HEAD capture has its own directory.
The persistent readback command is:

```sh
python3 /home/agent/sv-b024-final-audit/tools/author-readback.py <checkout> /home/agent/sv-b024-final-audit/<exact-HEAD>/runtime
```

This checks every artifact size/SHA-256, every source fingerprint, unchanged
HEAD/inputs, all six session counts (3/40/20/18/17/10 = 108), suite exit codes,
and ordinary/Guide successful reply bytes `02 03 7a`. Its role is explicitly
author integrity verification, not independent semantic review or acceptance.
The receipt fingerprints the readback script and runtime manifest.

## Reproduction and controller handoff

In a clean checkout of the target exact HEAD, run the existing collector with
an unused output directory:

```sh
python3 tests/sv_birth_runtime_evidence.py /home/agent/sv-b024-final-audit/<exact-HEAD>/runtime
```

The collector rejects nonempty directories, removes inherited case filters,
records commands/backend/source fingerprints/raw requests/replies/transcripts/
Guide and MOTD frames, and verifies HEAD/inputs stayed unchanged. If a directory
already exists, use a new sibling attempt directory and retain both attempts;
do not overwrite receipts. Native SDL dummy/software is not an OS-window or
real-server integration claim. Known Guide transcript prose `02 03 01` remains
historical text; raw reply bytes are the success oracle.

The independent controller must review the entire retained trace for exact
handshake/version gates, dedicated/legal completion, no early/duplicate sends,
reply ownership/generation isolation, actual Guide caller/preserved draft, and
ack-gated peer MOTD/handoff with inactive gameplay. It must write a fresh verdict
and `acceptance.json` beside `runtime/`, binding this contract digest and the
exact final HEAD. No such independent verdict is supplied by this worker.

Per the worker assignment, the orchestrator owns the full gates:
`make -s -C src -f makefile.sv tomenet-sv` and configured
`bash -s -- core < .sandcastle/checks.sh` (declared `/opt/sv-venv/bin` PATH).
It must retain actual commands, environment, exit codes and fingerprinted logs
for that same final HEAD/digest, including failed attempts. Historical passing
receipts and collector pending entries are not new passes. Any later integration
commit requires a new capture and independent final audit at its own exact HEAD.

P1-entry-evidence stays deferred to SV-B-025; SV-B-023 full acceptance stays with
its owner. Canonical SV-B-024, its historical filename and dependency are intact.
DNA, profile/resource workflows, real-server integration, session.enter-game and
first gameplay surface remain stage C; the boundary is startup live handoff.

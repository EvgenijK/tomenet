# Worker verification, acceptance pending

Precommit tested HEAD: `4f2d4c99449bcfedae30cd938e7e1a29eb8e76eb` with the
recorded working collector changes. Contract:
`0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

`python3 -B tests/sv_birth_inventory_checks.py --output
/home/agent/sv-b024-input-seal-13/delivery`: exit 0; full dependency discovery,
ordinary 3 production cases, exact 162-path inventory match, controlled file and
retained tool-identity mutation rejection/rollback and omitted-header rejection.
See `focused-checks.log`. Readback rehashed 68 artifacts. Runtime before/after
matched across 2,734 external fingerprints; 12 compiler dependency resolutions
covered six runners and both choice orders. Discovery retained all 40 generated
version fixtures/matrix and generated profiles.

Additional observation smoke: no-send cancel-sex 1, resource-gate version 1,
iddc-skipped 4 in both choice orders, actual Guide/MOTD caller 1: passed. Retained
commands, generated inputs, effective resource and loaded-library records live
in `/home/agent/sv-b024-input-seal-13/smoke/observations/` and
`/home/agent/sv-b024-input-seal-13/observer-smoke.log`.

The focused capture is historical worker verification, not final acceptance.
No full build/core/review workflow or final 108-case capture was run. Build/core/
controller review remain pending; P1-entry-evidence remains deferred to SV-B-025.
Any final integrated HEAD needs new controller evidence and external receipts.

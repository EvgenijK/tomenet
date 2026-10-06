# 14: Synchronize truthful HTML snapshots and coverage

**What to build:** A local HTML consumer can receive a validated canonical manifest snapshot and maintain an honest coverage ledger, without losing a coherent previous state or confusing prototype completeness with native acceptance.

**Blocked by:** 13: Reject unsupported or stale native evidence.

**Status:** completed (2026-09-22)

- [x] Validate canonical data before publication, preserve exact snapshot bytes and SHA-256 identity, and reject digest/schema/reference mismatches.
- [x] Initialize new active IDs as explicit missing entries; exercise addition, retirement and replacement without transferring evidence or approval automatically.
- [x] Support missing, planned and prototype-complete implementation states; ux-approved remains independent and requires actual human provenance. Preserve historical claims while counting only current, adequately scoped ones.
- [x] Publish snapshot and ledger coherently: injected/interrupted synchronization failures cannot leave a new snapshot paired with an incompatible old ledger or destroy the previous coherent pair.
- [x] Read-only freshness checks detect source changes at unchanged HEAD and report unavailable canonical data as unavailable rather than fresh; synchronization requires no Git/network service, PR or UX-document rewrite.
- [x] Verify a truthful all-missing ledger and negative/integrity cases using a temporary local consumer. Report whether the actual sibling HTML consumer was synchronized; its implementation completeness and new UX approvals are not Stage A gates.
- [x] Publish repeatable sync/freshness checks and confirm that HTML claims never supply native parity.


## Implementation

Contract and repeatable commands: `docs/capabilities/html-consumer.md`.
Local snapshot/ledger publication uses atomic generations and scoped claims.
Temporary canonical consumer: 925 active outcomes, all missing. The actual sibling
HTML consumer was not synchronized; no native parity or actual UX approval claimed.

Validation: 15 HTML CLI tests; full regression 25/25 runner invocations
(57 data tests and 21 runtime runners). Standards 0, Spec 0 residual findings.

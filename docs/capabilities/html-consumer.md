# Local HTML consumer — ticket 14

`tools/html_capabilities.py` publishes a validated canonical manifest and an
independent HTML coverage ledger. It does not alter native allocation/evidence,
legacy or SV runtime, MENUS/HOTKEYS, or other UX documents. A truthful all-missing
ledger is valid. HTML implementation and UX approval never establish native parity;
`nativeAccepted` is always zero.

Python 3.10+, the existing `tools/requirements-capabilities.txt` dependency, and a
local POSIX filesystem with atomic rename, directory fsync and flock are required.
No Git command, network service, PR or web server is used. Source roots are required
for every canonical source repository; the existing registry schema, semantic and
source-byte validator runs before publication. Missing inputs return unavailable,
including when an older consumer snapshot remains locally readable.

```sh
mkdir -p /tmp/sv-html-consumer
/tmp/sv-capabilities-venv/bin/python -B tools/html_capabilities.py sync \
  --canonical docs/capabilities/manifest.json \
  --consumer /tmp/sv-html-consumer --source-root tomenet=.
/tmp/sv-capabilities-venv/bin/python -B tools/html_capabilities.py check \
  --canonical docs/capabilities/manifest.json \
  --consumer /tmp/sv-html-consumer --source-root tomenet=.
/tmp/sv-capabilities-venv/bin/python -B tests/sv_html_checks.py
```

The read-only `check` returns one JSON report and writes nothing. Exit codes are
0 valid, 1 invalid/stale inputs, 2 unavailable. Reports separate freshness,
implementation declarations, effective complete/approved claims, stale claims and
prototype readiness. Validity of the ledger is not readiness. Retained stale claims
are legal historical declarations, but do not count unless the particular claim
matches the current fingerprint and passes its evidence gate. `prototypeReady`
requires every active row to have effective complete evidence and all inputs to
validate; it does not require new human approvals.

## Publication and recovery

The consumer owns `.sv-html/`. `CURRENT` is a JSON pointer to a generation directory
containing `manifest.json` (exact source bytes) and `coverage.json`; it records both
SHA-256 digests. Readers read CURRENT once, then both files from that generation,
and verify their digests. Do not read two independently resolved generations or
edit a published generation in place. Old generations remain available.

Sync holds an exclusive writer lock, stages and fsyncs both files, validates the
staged pair, then atomically replaces CURRENT and fsyncs its directory. A failure
before replacement leaves the previous pair reachable. A crash after replacement
can expose only a complete new pair. Interrupted staging leaves unreferenced
files; the next sync uses a new generation and does not need to destroy them.
`--fail-at after-snapshot` and `--fail-at before-publish` inject write failures for
repeatable checks. `--fail-at interrupt-before-publish` simulates abrupt process
loss (exit 99 without a report or cleanup); the OS releases the writer lock and
the next sync recovers normally. Atomic publication does not claim to make arbitrary concurrent
edits to source files a filesystem snapshot: run against a quiescent local tree;
a subsequent check detects source drift.

## Coverage and historical claims

The closed [schema](html-coverage.schema.json) allows only `missing`, `planned`,
`prototype-complete`. Each active canonical capability requires exactly one row;
new IDs start missing with no mappings, evidence or approvals. Retirement removes a
row from current coverage and retains it in history. Replacement never transfers
claims. Existing canonical IDs and replacement/lifecycle history are validated
against the previous manifest. Native coverage schemas reject this HTML ledger.

Each sync retains the prior ledger body in `history`. To submit reviewed changes,
copy the current ledger outside the consumer, append its previous body (all fields
except `history`) to history unless already its last entry, update the manifest
SHA/inventory/current coverage as needed, and pass `sync --ledger PATH`. The CLI
rejects candidates that discard prior history. Ordinary sync carries declarations
forward; it never manufactures evidence, imports checkboxes, or renews approval.

## Sources, mappings and evidence

`sourceInventory` conservatively includes every regular file under the consumer,
including untracked/ignored files and newly added modules. Only root `.git`,
`.sv-html` and `.sv-html-artifacts` are excluded: version-control metadata,
generations/lock, and separately hashed evidence respectively. Source symlinks and
special files are unsupported. Keep runtime/generated artifacts outside this
source tree. Inventory changes invalidate all previous evidence and approvals;
there is no selective dependency exemption. Source roots referenced by the
canonical manifest are also verified by content and anchor, independent of HEAD.

A row's mapping declares exact canonical UX entity/context/binding IDs, scope,
gates, UX source hashes/anchors, and page/module/registration locations. Scenarios
select explicit `contextId`, `bindingId` (nullable for nonbinding routes), `keyset`,
`platform` and `gate`. Complete mappings include the capability's canonical
relation endpoints and every active binding/context invoking its actions. Every
canonical binding's supported keyset/platform combination needs a scenario for
each declared gate; omission of a binding restriction means both supported
keysets/platforms. Nonbinding outcomes still require scenarios for every declared
gate. Gate labels must enumerate the applicable conditions; reviewers check their
meaning against the baseline, since prose conditions cannot be inferred from a
hash. Evidence environment page/keyset/platform/gates must match the mapping and
scenario selection. Scenarios also record fixture provenance, ordered steps and
explicit expectations for entry,
mouse, keyboard, selection, confirm, cancel, result, error, focus, normal, empty,
boundary, loading, unavailable, resize, minimum-scale, fallback, persistence,
reset, chat and blocking. Each expectation is required or explicitly inapplicable
with reason and UX source. Entry/result/cancel/error/focus cannot be excluded for
a complete flow. A placeholder or screenshot message is not proof of an execution
outcome. Review the expected result against the canonical outcome and the actual
reachable route; a static anchor alone cannot establish reachability or behavior.

The CLI returns `claims[].contractSha256` even for missing/planned scoped rows.
It hashes compact, sorted, ASCII-escaped JSON with keys `manifestSha256`,
`sourceInventory`, `scope`; scope contains exactly `capabilityId`, `mapping`,
`scenarios`. This binds each claim to exact manifest bytes, the complete consumer
inventory, mapping, gates, fixtures, steps and expectations without a circular
artifact reference. Producers must use the same encoding or obtain the fingerprint
from a validated scoped row. Changing any of those inputs requires new evidence.

Evidence references a scenario, fingerprint, observer/time, browser/page/transport/
viewport/keyset/platform/gates and a separately hashed JSON artifact in
`.sv-html-artifacts/`. That artifact must declare `kind: html-scenario`, the same
`capabilityId`, `scenarioId`, `contractSha256`, `environment`, `observedAt`,
`observer`, and an exact `results` object keyed by all expectation IDs. Required
results must be `pass`; justified exclusions must be `not-applicable`. All declared
scenarios need passing evidence. A report or screenshot without these outcomes
cannot certify complete implementation. Artifacts are read as data, never executed.

Approval is independent of implementation: a layout can be approved while missing.
An approval record requires `claim: ux-approved`, its current contract fingerprint,
approver/time and a hashed decision artifact. The decision contains `actorKind:
human`, `decision: ux-approved`, matching capability/fingerprint/approver/approvedAt,
and nonempty `decisionText` copied from an actual human decision. Generated records
or agent decisions cannot supply provenance. As with native evidence, hashes and
schemas cannot authenticate a person's identity or the truth of reported results;
producers/reviewers remain responsible for retaining real source decisions and
adequately scoped observations. The automated tests use explicitly labelled
synthetic attestations to exercise the gates, never actual product approvals.

## Actual consumer status

The sibling `/home/svechnik/Projects/github_site/tomenet_interface` was **not
synchronized** by ticket 14. Verification uses temporary local consumers, including
the full canonical manifest with an honest all-missing ledger. No prototype
implementation completeness or new human UX approval is claimed or required for
native Stage A.

## Verification — 2026-09-22

The full canonical manifest synchronized to a temporary local consumer with 925
active outcomes, 925 explicit missing rows, zero complete/approved/native claims;
read-only freshness returned current. The sibling HTML project remains untouched.

All 15 HTML process-level tests passed, including exact-byte snapshots, lifecycle
and replacement history, source additions/removals/changes, unavailable canonical
data, malformed schemas/references/digests, partial/failed evidence, nonhuman
approval, environment mismatch, canonical input variant drift, chronological
source reverts, injected write failures and abrupt sync process termination.
Positive attestations are synthetic validator fixtures only.

Linux build, Python compilation, schema validation and diff whitespace checks
passed. Full regression: 25/25 runner invocations passed (57 data tests across
registry/reconciliation/native evidence/HTML, four headless sanitizer runners,
legacy HP, and eight native runners on each of software/OpenGL). Sandbox prevented
LeakSanitizer and video-device access; those 20 runners passed when rerun outside
it. Windows/Wine and actual human visual acceptance were not exercised.

### Standards review

Independent review against starting commit `8e34657ec`: no documented standard
violations or material baseline smells. Two correctness findings (malformed
candidate traceback and duplicate chronological history rejection) were fixed
with red/green production CLI regressions. Follow-up review: zero residual findings.

### Spec review

Independent review identified inadequate scenario/environment variant binding.
Explicit context/binding/keyset/platform/gate selections, canonical route coverage
and evidence-environment matching now enforce that scope. Targeted regressions
pass; final independent Spec review reports zero residual findings. The temporary
consumer and absent sibling sync are explicit; no new native/UX acceptance is claimed.

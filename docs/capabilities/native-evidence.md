# Native evidence — ticket 13

The production data interface is `tools/validate_capabilities.py --evidence
 docs/capabilities/native-evidence.json` with the usual `--manifest`, `--ledger`
and repeated `--source-root REPOSITORY=PATH` arguments. It reads artifacts and
never executes binaries or scripts supplied by evidence. The canonical evidence
set and fallback route set are empty: all 925 outcomes remain pending.

Ledger schema version 1 remains compatible with pending-only allocations.
Version 2 additionally permits `evidenceStatus: accepted` and `evidenceIds`.
Implementation remains independent (`pending`, `native`, `fallback`). Each
`nativeClaims` result retains implementation and computes `pending`, `accepted`,
`stale`, `unavailable` or `invalid`. Missing coverage, missing evidence, malformed
schema, unsupported acceptance and failing runtime checks have separate diagnostic
codes. Exit codes remain 0 valid, 1 invalid, 2 unavailable. A data error anywhere
conservatively suppresses all accepted counts; valid allocations alone accept none.

## Evidence contract

`native-evidence.schema.json` is a closed schema. Every record identifies a native
scenario, capability, expected outcomes, actual pass/fail/unavailable result,
obligations, exact manifest bytes and allocation contract; it records environment
(platform, OS version, renderer, architecture, server version, build), executable,
configuration and native report fingerprints, source/fixture/resource inventories,
and the runtime fallback check. HTML, schema-valid and UX-approved record kinds
are rejected. Hashes cannot prove a report is truthful: producers must capture
real production-path observations and reviewers must inspect their provenance.
No current synthetic regression log has been promoted to accepted evidence.

`allocationSha256` is SHA-256 of UTF-8 JSON for the coverage row excluding
`implementation`, `reason`, `evidenceStatus`, `evidenceIds`, using sorted keys,
ASCII escaping and separators `(',', ':')`. This binds prerequisites, conditions,
stage and obligation descriptions without a circular evidence reference.

Every obligation needs passing evidence for every declared server-version/build
label and platform. Linux requires software and accelerated rendering; Windows
requires actual Windows 10 and 11 with both renderer classes. Windows OS versions
are exactly `10` and `11`; Linux retains the producer's distribution/version.
Wine is not Windows evidence. Version/build strings are exact allocation labels,
not executable expressions: producers must demonstrate all cases described by a
label in the referenced scenario report. Architecture is recorded, not inferred.
Prerequisite outcomes must themselves be accepted. Partial runs can be referenced
by pending rows; they do not certify a parent or the entire client.

## Dependency freshness

Each dependency entry names repository, directory, role, impact and an exhaustive
map of relative file names to SHA-256. All regular files, including untracked and
ignored files, are compared recursively. Changes, additions and removals invalidate
evidence independently of Git HEAD. Artifact absence and unreadable directories
are unavailable; differing bytes or directory membership are stale. Escaping paths
and symlinks fail closed. Empty resource/fixture directories can be explicit when
the scenario has no inputs of that role.

`impact: complete` asserts the producer and reviewer have established the full
transitive dependency closure for this scenario (including build scripts, headers,
configuration loaders, fixture generators and external SDKs). Scope directories
must include places where new dependencies can appear, not just today's input
files. `impact: unknown` requires `path: .`, invalidating the whole supplied
repository snapshot. Use immutable source/SDK snapshots and separate artifact
roots: a repository-wide snapshot intentionally includes metadata and generated
files, and cannot contain the evidence document describing its own hash.
The validator cannot infer C include closure or discover undeclared repositories;
reviewing dependency scope is part of native evidence review.

## Development fallback and runtime checks

An empty route set is valid. A route is an explicit development allocation with
stable ID, capability, input context, owner, replacement stage and removal
criterion. It requires `implementation: fallback`, pending evidence and a stage
no earlier than the flow allocation. Pending unimplemented flows have no routes.
Routes do not imply working runtime behavior or acceptance.

A scenario producer starts counters at zero, records all fallback entries across
its parent and child interactions, and completes the check only after the entire
scenario (including cancellation and replies) has finished. `runtimeCheck` carries
only scenario ID, completion, total entry count, and route ID / bounded reason
(`future-flow`) / positive count tuples. It must not carry prompts, account names,
secrets, arbitrary payloads or free-form errors. Any fallback entry fails acceptance,
even when a route is registered. A missing or incomplete check cannot pass.
Later runtime integration must instrument entry before transferring control; a
zero counter without such instrumentation is not valid native evidence.

No adapter or runtime route is introduced by this ticket. Any later temporary
adapter must route explicitly named flow/context pairs through the single input
router, preserve queue order, request identity, parent continuation, reply and
cancellation semantics, and continue network/model/timer/render work. Unknown
gestures and native errors must not trigger catch-all fallback. Such an adapter
requires its own production-path scenarios and removal criterion.

## Verification

`tests/sv_evidence_checks.py` calls the production CLI. Deliberately incomplete
negative claim fixtures never constitute runtime evidence: they exercise missing
coverage, absent evidence, stale allocations/artifacts, dependency additions,
removals and changes, unavailable artifacts, unsafe paths, incomplete checks,
fallback entry and unsupported schema/HTML/UX claims. The only valid fixture in
this suite is pending with an empty route set.

## Verification result — 2026-09-22

17 evidence tests, 12 registry tests (including 32 published negative fixtures),
and 13 reconciliation tests passed. The full Linux regression set passed all
24 runner invocations: those three data suites, four sanitizer/headless suites,
legacy HP, and eight native suites on each of software and OpenGL. SV built
successfully; Python compilation and schema validation passed. Canonical validation
reported complete reconciliation of 24 inventories / 2,077 rows, 925 pending
outcomes and zero accepted outcomes.

The aggregate runner's 240-second registry timeout was insufficient; the separate
registry run passed in 279 seconds. Sandbox runs could not access the SDL display
or run LeakSanitizer; the 21 runtime checks passed outside the sandbox. Windows
was not run and no platform acceptance evidence was imported.

## Standards

Independent review against starting commit `c4ed908e4`: no documented violations
or material heuristic findings. Changes preserve SV isolation and test the
production CLI. Final findings: 0.

## Spec

Independent review found no blocking omissions or scope creep against ticket 13.
Dependency-closure review remains explicit; this tool does not infer undeclared
inputs or prove the truth of a producer's runtime report. Final findings: 0.

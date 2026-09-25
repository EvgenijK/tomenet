# Capability registry and native allocation

Tickets 08–11 extend the initial slice with connection/account/character lifecycle,
world navigation, map exploration, targeting, items, combat, spells and ordinary
stores, information/social flows, documents and special-store canvases.
See the [information reconciliation](information-reconciliation.md) for ticket 10,
including typed request consumers and preserved source bytes. See the [item reconciliation](item-reconciliation.md) for ticket 09's
atomic outcomes, caller-specific cancellation, inventory mappings and earlier-stage
prerequisites, and its [planning authority](item-policy.md) for acceptance/UX sources.
See the
[session reconciliation](session-reconciliation.md) for the outcome index,
inventory mappings, cross-domain obligations and verification, and the
[resolved policy snapshot](session-policy.md) for its planning authority.
Ticket 11 adds settings, resources, files and platform outcomes; see the
[settings reconciliation](settings-reconciliation.md) and
[final policy snapshot](settings-policy.md) for ownership, defaults and exclusions.
Ticket 12 reconciles the full inventory corpus and adds explicit live HUD,
input and network outcomes; see the [complete reconciliation](complete-reconciliation.md).
The current registry contains 925 active outcomes, all pending, with no accepted
native claims, plus one retained deprecated ID. Current allocation after the
[Stage B source review](../research/sv-stage-b-allocation-proposal.md) is
A=8, B=508, C=176, D=163, E=68, F=2. The counts below describe the historical ticket 07 slice.

## Initial slice — ticket 07

This is a source-backed **initial slice**, not the complete behavior baseline.
It registers four independently observable outcomes: read current HP, read
ordered message occurrences, answer a generic key request, and cancel that
request. Related surfaces, state, actions, bindings and input contexts are
normalized references. It does not register packet fields or UI controls as
capabilities. All four native allocations remain **pending**, including evidence.
Existing native tests are useful source material, but have not been imported as
accepted native claims.

Tickets 08–12 expand and reconcile the remaining baseline. Ticket 13 supplies
scoped native evidence, dependency invalidation and fallback-route verification;
ticket 14 supplies HTML snapshot synchronization. This tool does not certify
runtime behavior, UX approval or Stage A acceptance. Inventory completeness
requires the explicit ticket 12 arguments documented below.

## Run

Python 3.10+ and `jsonschema` are required. Basic registry validation requires no
game build, account, Git or network access after installing the Python dependency.
Inventory completeness additionally uses local Git to enumerate tracked source files:

```sh
python3 -m venv /tmp/sv-capabilities-venv
/tmp/sv-capabilities-venv/bin/python -m pip install -r tools/requirements-capabilities.txt
/tmp/sv-capabilities-venv/bin/python tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger docs/capabilities/native-coverage.json \
  --source-root tomenet=.
/tmp/sv-capabilities-venv/bin/python tests/sv_capabilities_checks.py
```

The read-only CLI writes one JSON report to stdout. Exit 0 means data-valid;
exit 1 means invalid data; exit 2 means unavailable input/dependency/source
(or an argparse usage error, printed to stderr). Diagnostics have `code`,
`entity` and `message`; schema errors also include their JSON location.
The report separates the active denominator, pending evidence and accepted
capabilities. Version 1 of this allocation tool always reports zero accepted
capabilities. Valid input is not acceptance.

Use `--source-root REPOSITORY=PATH` for each named repository to verify source
file SHA-256 and literal UTF-8 anchors against local bytes. Missing repositories
or files report unavailable; changed content reports `source-digest`. Paths
cannot escape the repository, including through symlinks. Without these options,
provenance records are validated structurally, and `sourceVerification` explicitly
says `not-requested`. This checks referenced files, not the complete dependency
inventory or native evidence freshness required by ticket 13.

## Data contracts

`manifest.schema.json` and `native-coverage.schema.json` are JSON Schema Draft
2020-12, evaluated by the production validator. Unknown properties are rejected.
`schemaVersion: 1` identifies structure, not content. The manifest's SHA-256 is
computed from its exact bytes, including whitespace; the consumer ledger must
match. Editing the manifest requires explicitly updating the ledger digest:

```sh
sha256sum docs/capabilities/manifest.json
```

The manifest contains `capabilities`, `surfaces`, `actions`, `states`, `bindings`,
`inputContexts`, `relations` and reusable `sources`. Every entity has a stable
typed ID, title, description, lifecycle and source references. IDs use lowercase
dotted names (hyphens allowed), with collection prefixes `capability`, `surface`,
`action`, `state`, `binding`, `context`, and `source`. Two-segment names such as
`surface.status` are permitted by the approved contract. IDs are globally unique
across collections. Display names can change without changing identity.

Sources distinguish `behavior`, `protocol`, `ux` and `acceptance`, and retain
repository identity, observed revision, relative path, optional literal anchor,
and SHA-256 of the full source file. A revision alone cannot identify dirty
working-tree content. Every active capability needs behavior provenance;
acceptance reports and UX sources cannot substitute for it. Source roles describe
authority, not whether a report passed. Authors still need to review that a cited
source actually supports the outcome; a checksum cannot establish semantic truth.

Relations validate these exact endpoint meanings:

| Relation | From → to |
| --- | --- |
| `capability-exposed-on-surface` | capability → surface |
| `capability-uses-action` | capability → action |
| `capability-observes-state` | capability → state |
| `surface-presents-state` | surface → state |
| `binding-invokes-action` | binding → action |

A binding carries its gesture, action and input context; optional nonempty
`keyset` and `platform` lists restrict it to `normal`/`roguelike` and
`linux`/`windows`. Omitting a restriction means all supported values. A binding
relation, when present, must agree with its `actionId`. These restrictions
describe baseline applicability, not current SV implementation coverage.

The separate native ledger requires one row per active capability, exactly one
acceptance stage A–F, explicit prerequisite outcome IDs (possibly empty),
source-backed version/build/platform/scope conditions, and nonempty source-backed
evidence obligations with unique `obligation.*` IDs. Prerequisites must have an
active allocation at the same or an earlier stage, without cycles. Obligations
express expected results; they are not test execution records.

`implementation` is `pending`, `native` or `fallback`; each requires an explicit
reason. It is independent of `evidenceStatus`, currently restricted to `pending`,
and `evidenceIds`, currently empty. Native implementation does not imply native
acceptance. Unsupported accepted, prototype-complete and UX-approved values are
rejected rather than silently counted. The initial slice claims no fallback
routes. Additional evidence/claim fields require a schema change in ticket 13.

## Lifecycle and history

Keep deprecated/retired entities in the manifest permanently. Only those two
lifecycles may carry `replacedBy`, which names existing entities of the same kind.
Replacement cycles and self-replacement fail. Replacements get their own explicit
coverage; no stage or evidence is inherited from the old entity. Current coverage
may only reference active outcomes; archive historical consumer claims separately.

For every update, retain the last validated manifest and check against it:

```sh
/tmp/sv-capabilities-venv/bin/python tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger docs/capabilities/native-coverage.json \
  --previous-manifest /path/to/previous-manifest.json \
  --source-root tomenet=.
```

The report records `historyChecked`. Without a prior manifest, validation checks
only the current snapshot and cannot detect historical deletion or reuse.
History checking rejects removal, kind changes, removed replacement links and
backwards lifecycle changes. Allowed transitions are:

- proposed → proposed, active, retired;
- active → active, deprecated, retired;
- deprecated → deprecated, retired;
- retired → retired.

An unrelated new outcome must receive a new ID; no tool can infer intentional
semantic reuse from an arbitrary rewritten description. Source observations may
be refreshed; the entity-history retention rule is for product entities.

## Fixtures and verification

[`tests/capabilities/fixtures/valid`](../../tests/capabilities/fixtures/valid)
contains a minimal source-backed manifest/ledger pair, usable directly as CLI
arguments. [`invalid-cases.json`](../../tests/capabilities/fixtures/invalid-cases.json)
publishes 32 named negative fixtures as mutations of the canonical slice. Each
lists the target document, JSON path, `set`/`delete` operation, and expected
diagnostic. The test runner materializes each pair in a temporary directory,
recomputes its manifest digest to isolate the intended defect, and invokes the
production CLI. Run only these fixtures with:

```sh
/tmp/sv-capabilities-venv/bin/python tests/sv_capabilities_checks.py \
  RegistryChecks.test_published_negative_fixtures
```

Other process-level tests cover valid pending coverage, canonical source bytes,
exact-byte identity, replacement independence, lifecycle history, display-title
changes, missing files, malformed JSON with duplicate members, and changed source
bytes at unchanged revision. The data-tool interface is separate from the game
core and introduces no decoder, game-state or renderer seam.

The local ticket is
`.scratch/single-window-sdl3-client-stage-a/issues/07-validate-capability-registry-and-native-ledger.md`;
its normalized model follows the resolved manifest contract in the preceding
design phase. The local tracker bootstrap file `docs/agents/issue-tracker.md` is
absent; review uses these explicit local ticket/spec paths.

## Verification — 2026-09-22

The registry suite passed 12 process-level tests, including all 32 published
negative fixtures. Canonical validation with local source verification returned
four active outcomes, four pending evidence rows and zero accepted outcomes.
Python compilation and both JSON Schema metaschema checks passed.

The existing Linux SV target built successfully. The complete Linux regression
run passed 22/22 runner invocations: this registry suite, four headless sanitizer
suites (architecture, messages, requests, lifecycle), legacy HP regression, and
eight native suites on each of software and OpenGL (HP, architecture, messages,
requests, lifecycle, geometry, timing, shell/resources). These regression results
are not imported as native ledger acceptance. Windows/Wine and human visual
acceptance were not exercised for this data-tool change.

### Standards review

Independent review against starting commit `05d040a33` found no documented
standard violations. One optional schema-duplication finding was fixed by shared
entity fields with closed entity/binding schemas. Follow-up review: no unresolved
findings; the registry suite passed again after the schema change.

### Spec review

Independent review found no actionable deviations from ticket 07. The initial
source-backed slice, pending allocation and separation of later evidence and
reconciliation work match the ticket. Final findings: Standards 0, Spec 0.

## Complete baseline reconciliation — ticket 12

[Complete reconciliation](complete-reconciliation.md) connects all retained inventories
to the canonical outcomes and native allocation. Run the validator with both
`--inventory-index docs/capabilities/inventories/index.json` and
`--reconciliation docs/capabilities/reconciliation.json`, plus
`--source-root tomenet=.` to verify completeness and current source bytes.
Without these arguments, the report explicitly says completeness was not requested.
Valid pending allocation never means native acceptance.

## Native evidence validation — ticket 13

The [native evidence contract](native-evidence.md) adds optional `--evidence`
validation, closed runtime/fallback records, dependency freshness, and version 2
ledger claims. The version 1 descriptions above remain the historical pending-only
contract. Canonical [evidence](native-evidence.json) is empty, with no accepted
native claims and no declared fallback routes.

## Local HTML synchronization — ticket 14

[HTML consumer contract and commands](html-consumer.md) describe the independent
local snapshot/coverage CLI, atomic generations, conservative freshness, scoped
prototype evidence and human approval provenance. Honest missing coverage is
valid; no HTML claim supplies native acceptance. The actual sibling consumer was
not synchronized; validation uses temporary local consumers.

## Integrated Stage A gate — ticket 15

[Stage A acceptance](../sv-stage-a-acceptance.md) provides the complete build,
scenario, registry/evidence and local HTML commands, retained observations and
explicit blockers. Passing registry allocation is not native acceptance.

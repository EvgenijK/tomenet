# Ticket 12: complete inventory reconciliation

Spec: `.scratch/single-window-sdl3-client-stage-a/issues/12-reconcile-complete-registry-against-all-inventories.md`.
Starting revision: `a3d99eeb4`. This change is a registry/data-tool allocation, not a gameplay implementation or native acceptance claim.

The canonical registry now contains **925 active outcomes**, all native implementation/evidence **pending**, and one retained deprecated ID. Ticket 12 adds 56 independently observable outcomes and resolves one duplicate. No legacy/common/SV game source is changed.

## Reproducible completeness check

```sh
/tmp/sv-capabilities-venv/bin/python tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger docs/capabilities/native-coverage.json \
  --inventory-index docs/capabilities/inventories/index.json \
  --reconciliation docs/capabilities/reconciliation.json \
  --source-root tomenet=.
```

Install the pinned `tools/requirements-capabilities.txt` into a virtual environment if needed. To check ID/replacement history, also pass `--previous-manifest PATH` containing the manifest from the starting revision. The validator is read-only and emits JSON plus exit 0 (valid), 1 (invalid/incomplete), or 2 (unavailable inputs/environment). No completeness is claimed without both inventory arguments. Supplying only one fails.

The report identifies exact manifest, inventory-index and reconciliation bytes. `complete` means the supplied inventories have valid dispositions and allocations against the checked source content; it does **not** mean the client is implemented, its evidence is accepted, or semantic audit can be replaced by a counter. Ticket 13 owns acceptance/evidence freshness; ticket 14 owns the independent HTML consumer.

## Denominator and current source identity

[inventories/index.json](inventories/index.json) retains exact-byte snapshots of the original baseline and all 23 scoped/remaining input, packet, slash, surface, renderer, resource, persistence, platform, text, font, retention and consumer research reports. `origin` preserves each original location/revision. Historical links/line numbers inside these immutable copies resolve relative to their original location; use the current `sourceFiles` paths for local code. Current final policy takes precedence over proposals preserved inside these snapshots.

The original baseline was recovered from `87ead6ff5:docs/research/single-window-behavior-baseline.md`. Copies of previously ignored research are committed here, so completeness does not depend on a private `.scratch` directory or another worktree. The three reports without tables remain content-addressed narrative contracts. The corpus scan rejects a Markdown inventory omitted from the index.

Every Markdown table data row has a physical-line locator and SHA-256 of its exact text in [reconciliation.json](reconciliation.json). Headers/separators are excluded structurally, not by remembered counts. The validator independently enumerates inventory rows, so deleting a mapping cannot shrink the denominator. It detects added/changed rows even after the inventory file hash is refreshed. Removed/duplicate/unknown references and obligations belonging to another caller fail.

The index also hashes **610 tracked files** in `src`, `lib/scpt`, `lib/user`, `lib/xtra/font`, `lib/xtra/graphics`, and `.github`. It verifies working-file bytes, including wire/server contracts, Lua, default preferences, assets and build/package definitions. Git's tracked file set discovers newly added files in those scopes; personal untracked experiments/profile files are outside the baseline. Stage a new baseline file before running completeness. Missing/changed sources fail closed; a changed source requires semantic re-audit, updated inventory/dispositions and allocation before the affected completeness claim can pass. Merely refreshing a digest is not a semantic review.

Source identity is intentionally conservative: any tracked change in these scopes invalidates the inventory audit until reviewed. The machine does not infer new behavior from C syntax. Reviewer-owned mapping remains necessary for a new conditional branch, function, command or field contract.

## Dispositions and authority

Current rows: **2,077** — 1,918 required, 67 source-accounting, 61 excluded, 25 dead, 6 directional. These counts are a reproducible cross-check, not hard-coded acceptance thresholds.

Required rows name concrete stable capabilities and their existing result/lifecycle/input/wire obligations. Broad original family rows reference all relevant leaves, not a family placeholder. Source-accounting rows cover historical count/hash tables, scan-negative translation units, research alternatives and HTML consumer observations; they do not exclude gameplay. Excluded/dead/directional rows retain the original cited source plus an explicit reason.

Final [session policy](session-policy.md), [item policy](item-policy.md) and [settings policy](settings-policy.md) override historical drafts. In particular:

- FON metadata does not require a FON backend: SV loads PCF/TTF and preserves numeric glyph identity. Unselected default-font candidates do not become mandatory bundled fonts.
- Terminal/GCU topology, fixed-framebuffer alternatives, user map zoom, XHTML routing, unused repaint techniques and an implicit session recorder are not reinstated by old research wording.
- All active display options retain semantic consumers; `big_map` derives from layout. Inactive lexical keys stay inactive. No new file-transfer allow-list is introduced.
- Reserved/unimplemented packet names are classified by source direction and dispatch. A server-only binding does not invent an incoming receiver or a shipped client command.
- Source hashes and HTML checkboxes are observations, not native runtime evidence or UX approval.

## New outcomes, overlaps and corrections

The previous broad profile/input and family rows missed explicit allocation of live HUD fields, input behavior and network reactions. Added atomic IDs cover 27 HUD outcomes, nine input primitives plus raw-key dispatch, eleven network/control outcomes, five warning/page occurrences, live player-list and unique-record collections, and explicit redraw.

Configuration options and delivered warning occurrences remain distinct: `options.alert-*` configures behavior, `alerts.*` must observe each eligible event. `status.read-hp` stays the original Stage A synthetic display outcome; it does not absorb live audio warnings or full HUD acceptance. `status.read-live-hp` owns the complete Stage B live HP display, depends on the A primitive and is explicitly required by live game entry. `information.live-players` owns `PKT_PLAYERLIST`, separately from the `SPECIAL_FILE_PLAYER` document. `information.unique-records` owns keyed kill records, separately from the unique knowledge document.

`credentials.unavailable` duplicated `account.secret-provider-failure`. Its stable ID is retained as **deprecated**, `replacedBy` points to the surviving outcome, and all current native allocations, prerequisites and reconciliation/scenario references use the survivor. Historical relations remain traceable. `credentials.lookup` is the exact provider-key/byte operation; `account.restore-secret` is the login caller's restore/manual-entry flow. `credentials.change-write` is the immediate vault write; `account.change-password` includes private prompts and the send. These are independent observable outcomes, not duplicates.

Current source corrections are explicit: SDL3's physical owner is `react_keypress` (historical audit says `key_press`); `Receive_keypress` consumes only the packet byte and does not inject a key. `Receive_keepalive` is a receive no-op, while `Send_keepalive` is independently scheduled. Ping with `pong=0` echoes once with `pong=1`; an incoming pong updates telemetry without another echo. `Receive_end` has no frame-counter payload despite its old comment. `SPECIAL_FILE_EXTRAINFO` is compiled out; `/ex` remains normal message/slash output. `PKT_AUTOPHASE` has a server binding but no shipped client sender; the disabled `cmd_mind` must not become a new user command.

## Cross-cutting scenarios and stage prerequisites

The ten original scenarios in `reconciliation.json` connect live entry, HP urgency, item/spell/store callers, formatted requests, live players, persistence/resources, byte boundaries, map composition, teardown and source-defined extensions. Every listed capability names its own evidence obligations. The complete registry is checked in reverse too: an active allocated outcome with no inventory/scenario reference fails.

The [2026-09-23 source-backed allocation](../research/sv-stage-b-allocation-proposal.md) changes 75 of the original 583 B outcomes: 508 remain B, 11 move to C, 7 to D and 57 to E. Current totals are **A=8, B=508, C=176, D=163, E=68, F=2**. All 925 active IDs and the deprecated history are retained. The 387 prerequisite changes follow concrete callers; read-only/direction/target B paths no longer depend on the C quantity prompt. Live entry gains settings, FILE/Lua and startup dependencies. Existing B owners explicitly retain shared INS reads/conversion, eight audio CFG startup projections and early Guide/chat cancellation; full late IDs preserve their semantics.

The reconciliation now contains 209 scenarios: the ten original scenarios plus 184 concrete option-consumer fixtures, five early obligation handoffs and ten late option-caller regression groups. Numeric input is removed only from early live-entry coverage; its C/D/E callers remain mapped. The quantity description corrects historical wording: Escape returns zero; accepted empty input uses the default. Immutable inventory quotations are retained and do not override this source-backed correction. For `disturb_other`, storage/default/wire slots remain required; no new effect is invented where no consumer was found.

The existing validator rejects later-stage prerequisites and cycles. Scenarios may span stages: readiness prerequisites, partial startup projection and whole-caller acceptance are distinct. All native implementation/evidence remains pending; the new obligations are requirements, not runtime evidence.

## Validation

The production process seam is the already approved registry interface: canonical data and content-addressed sources enter; reports and exit codes leave. `tests/sv_reconciliation_checks.py` demonstrates omission of a required row, discovery of a new row after a digest refresh, changed contract/source bytes, missing files, tracked source additions, wrong caller obligations, duplicate dispositions, explicit unresolved work and a removed inventory. A valid future pending allocation still succeeds with **zero accepted outcomes**.

Run the complete data-tool suite:

```sh
/tmp/sv-capabilities-venv/bin/python tests/sv_capabilities_checks.py
/tmp/sv-capabilities-venv/bin/python tests/sv_reconciliation_checks.py
python3 -m py_compile tools/validate_capabilities.py tools/reconcile_capabilities.py tests/sv_reconciliation_checks.py
```

Native binaries/rendering suites do not validate this data-only change; no game implementation or native evidence is altered. The immutable text-field inventory retains one original trailing space; whitespace checks exclude the byte-preserved inventory corpus.

## Standards

Independent review found no documented standard violations or actionable
heuristic concerns. Runtime isolation, production-process tests and deferred
improvement records conform to AGENTS.md. Final findings: **0**.

## Spec

Independent review found an unallocated live HP caller: the original Stage A HP
primitive and warning occurrence did not certify live HUD display. Added
`status.read-live-hp` at B with its own evidence, the A prerequisite, live-entry
dependency and all HP inventory/scenario references. Follow-up review verified
the correction. Remaining findings: **0**.

Review totals after correction: Standards 0; Spec 0.

## Verification result — 2026-09-22

Final data-tool suite: **12 registry tests passed** (including every published
negative fixture), **13 reconciliation tests passed**. Python compilation and
JSON Schema validation passed. Complete validation against the starting manifest
reported `valid`, history checked, source bytes checked, 24 inventories and
2,077 disposed rows, zero unresolved rows, 925 pending outcomes and zero accepted.
The omitted-outcome test removes both capability and native allocation together;
the independent inventory still reports `inventory-allocation`.

## Stage B allocation verification — 2026-09-23

Applied the complete [allocation proposal](../research/sv-stage-b-allocation-proposal.md):
75 stage changes, 387 prerequisite-list changes, no new/deleted capability IDs,
no removed obligations and no native acceptance claims. New primary-source
observations identify concrete consumers; the existing inventory source hashes
and immutable audit rows did not need refreshing because code and policy sources
were unchanged. Only the quantity and `disturb_other` descriptions were corrected;
all other original outcome descriptions remain intact.

The production validator passed with history, source, inventory, reconciliation
and evidence verification: 925 active/pending outcomes, zero accepted, 24
inventories, 2,077 disposed rows, no unresolved rows and no errors. Existing data
suites passed: 12 registry tests (including 32 published negative fixtures), 13
reconciliation tests and 21 evidence tests. `git diff --check` passed. This is a
data-only allocation change; game binaries/platform acceptance were not rerun.

The pre-edit manifest was retained at
`/tmp/sv-b-allocation-before/manifest.json`; recover the same bytes from the
recorded starting commit for a later checkout:

```sh
git show 887ac4d215376daaa169afca8216f65c18855107:docs/capabilities/manifest.json > /tmp/sv-b-previous-manifest.json
/tmp/sv15-venv/bin/python tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger docs/capabilities/native-coverage.json \
  --previous-manifest /tmp/sv-b-previous-manifest.json \
  --inventory-index docs/capabilities/inventories/index.json \
  --reconciliation docs/capabilities/reconciliation.json \
  --evidence docs/capabilities/native-evidence.json \
  --source-root tomenet=.
```

### Targeted review corrections — 2026-09-23

Both P2 findings from review of the allocation were corrected. `subterm_flicker`
and `misc_no_flicker` now name the real auxiliary inventory consumer, message/chat
scope and unchanged main-map control. Their checks preserve the global animation
override and the message exception that applies only to misc suppression, using
semantic content roles in SV. Primary references are `fix_inven`/`display_inven`,
`fix_message`, `flick_colour` and the approved display-option policy.

The new ASCII regression mistakenly assigned to `information.unique-records` was
removed, together with its unsupported source/scenario association. This B outcome
still owns kill counts/names; actual artifact/monster lore glyph regressions remain.
No obligation from the original 887 baseline was removed. Stages, prerequisites,
capability IDs, source code and immutable inventory hashes are unchanged by these
corrections. Targeted independent re-review reports Standards **0** / Spec **0**;
this does not constitute native runtime acceptance.

After the fixes, full validation passed against both the pre-fix manifest and the
original 887 manifest (history/source/inventory/reconciliation/evidence). Repeated
data suites passed: registry 12/12 including all 32 negative fixtures,
reconciliation 13/13 and evidence 21/21. `git diff --check` passed.

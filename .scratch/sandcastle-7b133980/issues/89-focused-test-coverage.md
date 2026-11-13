# 89: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-67-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed focused production-path coverage on the isolated branch
  `codex/sandcastle-7b133980-wave-67-1`; full assembly gates and deferred native
  acceptance evidence remain with their originating owners.
- 2026-10-09: Inspected all effort child statuses: 89 is the only unresolved
  child. Ticket 88 is the production/test delta since focused record 87. Read
  root AGENTS.md, CONTEXT.md, domain/tracker rules, SV architecture, ADR-0001
  through ADR-0006, canonical SV-B-022/SV-B-023, and the tdd skill with its
  test/mocking guidance. No separate GLOSSARY.md exists.
- Accepted seam: public `sv_endpoint_run(SvEndpointOptions)` from the immutable
  contract and ticket 88. Authoritative controlled-peer setup and real SDL
  keyboard/mouse input exercise the production native adapter, birth interaction
  and input router; `SvEndpointOutcome` and peer EOF checks observe outcomes and
  prohibited sends. This seam does not replace deferred native runtime evidence.
- Coverage inspection found all 13 legacy mode scenarios already implemented
  at that seam in both compile orders. No missing focused test or production
  defect was identified. This is coverage of existing behavior, with no claimed
  test-first/red-green loop and no test-only behavior implementation.

## Answer

Focused coverage inspected and exercised on assembled pre-resolution HEAD
`d6bcc66d5ba50e5b9be4bdad049ea573d0396fd7`, accepted contract digest
`15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6`.
The final commit changes only this execution record and its map index.

Production seam: `sv_endpoint_run(SvEndpointOptions)`, agreed by the contract
and ticket 88. The controlled peer supplies authoritative server versions,
creation flags, race/class/trait lists, roster and dedicated limits over the real
protocol path. Real SDL gestures exercise native input routing and the real SV
birth interaction. Public `SvEndpointOutcome` assertions observe owner, legal
identities, highlight, draft and completion state. The peer requires EOF after
creation gestures, failing on any unexpected partial or completed-character
bytes. Unsupported-mode scenarios require EOF before name submission, exact
`Unsupported character creation mode`, unchanged selection and untouched draft.
The peer provides protocol fixtures, not an alternate behavior implementation.

Existing tests cover the changed behavior: all 13 legacy mode cases execute in
both class/race orders. They verify Normal/Fruit Bat confirmation, keyboard and
mouse navigation, stable highlighting, unknown/stale input rejection, legal
random, staged/automatic DNA, invalid DNA, first-run/RPG legality, traditional
and point-buy stats, Backspace to stats and the nearest available predecessor,
and re-entry. No unavailable body owner receives ownership. Modern body, Archer
random and DNA controls, dedicated IDDC/PvP entry, post-mode Maia/deep Backspace,
quit/Ctrl-Q, session events and Guide controls exercise adjacent accepted
behavior. No additional test or production change was needed; no red/green
loop is claimed for verification of already implemented behavior.

Commands:

```sh
SV_BIRTH_CASE=legacy-mode-fruit-bat,legacy-mode-back,legacy-mode-normal,legacy-mode-mouse,legacy-mode-reenter,legacy-mode-random,legacy-mode-dna,legacy-mode-invalid-dna,legacy-mode-first-run,legacy-mode-rpg,legacy-mode-point,legacy-mode-auto-dna,legacy-mode-predecessor,body-unavailable-keyboard,body-unavailable-mouse,non-first-run-body-manual,archer-body-random,body-dna,stats-order-navigate,stats-point-navigate,mode-dedicated,mode-pvp-maia,mode-pvp-maia-deep-back,exclusive-iddc-production,exclusive-pvp-maia-production,session-interleave,session-stale-input,session-disconnect,session-retry-generation,auto-dna-stale-event,legacy-hidden-class-disconnect,unsupported-creation-ordinary,unsupported-creation-exclusive,guide-all-owners,stats-point-guide,guide-ordered-stats-no-op,quit-mode,ctrl-quit-mode \
  PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
  python3 -B tests/sv_name_editor_submit_checks.py
PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
  python3 -B tests/sv_ordinary_create_cancel_checks.py
git diff --check
```

Results: **PASS**, both runners exit 0. Birth selection executed 38 default
and 34 alternate-order scenarios (72 executions; retry-generation opens two
connections in its one default scenario). All 13 legacy mode cases ran in both
orders. Session interleave/stale/disconnect/retry scenarios are default-only;
automatic-DNA stale and hidden-class disconnect controls ran in both orders.
Capacity checks executed 18 scenarios covering ordinary available/full limits
for neither, IDDC-only, PvP-only and both feature combinations, plus dedicated
boundaries. Production sources compiled with `-Wall -Wextra -Werror`, ASan and
UBSan; compiler, assertions and sanitizer checks passed. Existing declared-font
fallback and SDL `Leaked thread` diagnostics appeared without failures.
`git diff --check` — **PASS**.

Full `sv-build`, unfiltered `sv-core`, assembly gates and `controller-review`
were not run and remain orchestrator-owned. `P1-birth-runtime-evidence` /
P1-AC30 remains **deferred; not run** at its originating named producer/verifier
for the exact accepted digest and acceptance HEAD. No native full-acceptance
claim is made. Stage C DNA persistence, Stage G Guide ownership and SV-B-024
final completed-character send remain with their existing owners.

Legacy/shared and production behavior files are unchanged. No unrelated
improvement opportunity was identified.

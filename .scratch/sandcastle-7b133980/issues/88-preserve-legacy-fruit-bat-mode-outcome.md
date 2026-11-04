# 88: Сохранить legacy Fruit Bat outcome в mode step

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок на pre-4.4.1a server, где отдельного body step ещё нет, по-прежнему может завершить исторические Normal и Fruit Bat outcomes через baseline mode owner. Недоступный body step не получает ownership, но его отсутствие не удаляет Fruit Bat из legal creation outcomes: keyboard/mouse, random/DNA, legality, stable highlighting и Backspace сохраняют протокольную и игровую семантику без partial или completed-character send.

**Blocked by:** [86: Пропустить authoritative unavailable body step](86-skip-authoritatively-unavailable-body-step.md) (completed; establishes the version-aware absence of the separate body owner that this repair must preserve).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-66-1

Labels: bug, ready-for-agent

**Finding ID:** `D-3dd8c1894797fcf5` (`sv.birth.legacy-body-mode.drops-fruit-bat-outcome`), one coherent repair of the lost pre-4.4.1a Fruit Bat outcome and its production-path regressions.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative pre-4.4.1a contact/setup от controlled peer; реальные SDL keyboard/mouse gestures проходят native adapter и публичную birth-interaction/input-router boundary, а `SvEndpointOutcome` и peer наблюдают owner, legal mode identities, highlight, draft/outcome и отсутствие запрещённых outbound bytes. Этот seam принят для production interaction, mode/body semantics и production-path tests по P1-AC02, P1-AC07, P1-AC09, P1-AC10, P1-AC11, P1-AC28 и P2-AC05; проверки — `sv-core`, `sv-build` и `controller-review`.

- [x] RED production-endpoint scenarios для authoritative pre-4.4.1a setup доходят до mode и воспроизводят потерю Fruit Bat; GREEN предоставляет Normal и Fruit Bat как legal исторические outcomes у baseline mode owner, подтверждает каждый outcome в draft/result и никогда не назначает отсутствующий body owner. Eligibility и compatibility filters соответствуют behavior baseline, а unknown/illegal identity не продвигает interaction. [Contract: P1-AC02, P1-AC07, P1-AC09, P1-AC10, P1-AC19, P2-AC04, P2-AC05; check: `sv-core`]
- [x] Keyboard и mouse navigation сохраняют stable highlighted identity при выборе Normal и Fruit Bat, confirm завершает выбранный legacy outcome, а random и применимый staged DNA проходят тот же production command path и дают только legal outcome; invalid DNA сохраняет принятую invalidation semantics. [Contract: P1-AC07, P1-AC09, P1-AC10, P1-AC11, P1-AC26, P1-AC27, P1-AC28, P2-AC05; check: `sv-core`]
- [x] В обоих class/race compile orders Backspace из legacy mode возвращает к правильному ближайшему доступному owner, повторный вход сохраняет согласованные draft/highlight semantics, отдельный body owner остаётся skipped, а Backspace, quit и подтверждение mode не отправляют partial либо completed-character packet. [Contract: P1-AC12, P1-AC16, P1-AC18, P1-AC19, P1-AC29, P2-AC04; checks: `sv-build`, `sv-core`]
- [x] Production-endpoint controls сохраняют modern separate Normal/Fruit Bat body step, supported traditional/point-buy stats, ordinary и dedicated IDDC/PvP flows, post-mode PvP Maia return, disconnect/stale-generation guards и unsupported-mode fail-fast без расширения ownership SV-B-023. [Contract: P1-AC02, P1-AC07, P1-AC08, P1-AC20, P1-AC22, P1-AC29, P1-AC31; checks: `sv-build`, `sv-core`, `controller-review`]
- [x] Repair остаётся SV-local по возможности, использует production interaction state/input router и не добавляет fixed-response либо test-only behavior; unrelated legacy/shared/SV organization improvements записываются отдельно с требуемыми полями и не включаются в repair. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; check: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим repair; evidence собирается только на exact accepted contract digest `15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6` и implementation HEAD.

## Comments

- 2026-10-09: Claimed on the retained isolated worker branch before implementation. Public production seam is the ticket-approved `sv_endpoint_run(SvEndpointOptions)` with authoritative pre-4.4.1a setup, SDL gestures, `SvEndpointOutcome` and controlled-peer outbound-byte checks. Startup and diagnostic tool execution succeeded; no launcher failure recurred in this worker turn.

## Answer

Implemented on the retained worker branch only. The worker startup and tool
execution succeeded in this turn; no further recovery decision, quota change,
branch merge or launcher modification was needed. Read root AGENTS.md,
CONTEXT.md, the domain/tracker rules, architecture and ADR-0001 through ADR-0006,
and the tdd skill with its test/mocking guidance before implementation.

Production seam: the explicitly accepted public
`sv_endpoint_run(SvEndpointOptions)`. The controlled peer supplies the actual
pre-separate-body contact version `4.4.1.0.0.0` and decoded race/class setup;
real SDL keyboard/mouse events pass through native-birth and the production
interaction/input router. `SvEndpointOutcome` exposes legal identities,
highlight, owner and draft. The peer asserts EOF after creation gestures,
so any partial or completed-character bytes fail the scenario. No behavior is
implemented in the test peer, and no alternate test-only wizard is used.

The absent body owner remains skipped with an empty legal body list and no body
confirmation. Ordinary returning-player, non-RPG legacy mode lists now include
the historical `0x20` Fruit Bat identity alongside Normal and existing modes.
Confirmation records the corresponding Normal/Fruit Bat body outcome through
the mode owner. The same legal identity list serves keyboard `f`, mouse,
random, staged DNA and automatic DNA. First-run, RPG and dedicated filtering
remain in the production mode preparation path. Modern servers keep their
separate body step. The legal-mode capacity, endpoint outcome and view grew
from six to seven entries; the native label and binding are SV-local.

### RED → GREEN

First vertical slice: added `legacy-mode-fruit-bat` at the approved endpoint
seam before changing production sources. The initial test draft hit a compiler
array-bounds diagnostic from referring to the absent seventh slot; that test
reference was removed before obtaining the required behavioral RED.

```sh
SV_BIRTH_CASE=legacy-mode-fruit-bat \
  PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
  python3 -B tests/sv_name_editor_submit_checks.py
```

RED: exit 1, endpoint assertion `outcome.birth_legal_mode_count == 7` failed;
the authoritative old setup reached mode but offered only six identities.
GREEN after the minimal SV implementation: exit 0, the scenario passed in both
compile orders and retained Fruit Bat in highlight, draft mode and body outcome,
without assigning body ownership or sending a result. The Backspace slice was
coverage of already implemented behavior and passed without a production
change; no separate red/green claim is made for that coverage.

### Focused verification

Exact tested implementation HEAD:
`8554c88d88ae7864ef112da986d881beb635f02e`.

```sh
SV_BIRTH_CASE=legacy-mode-fruit-bat,legacy-mode-back,legacy-mode-normal,legacy-mode-mouse,legacy-mode-reenter,legacy-mode-random,legacy-mode-dna,legacy-mode-invalid-dna,legacy-mode-first-run,legacy-mode-rpg,legacy-mode-point,legacy-mode-auto-dna,legacy-mode-predecessor,first-run-body-reject,non-first-run-body-manual,archer-body-random,body-dna,stats-order-navigate,stats-point-navigate,mode-dedicated,mode-pvp-maia,exclusive-iddc-production,exclusive-pvp-maia-production,session-interleave,session-stale-input,session-disconnect,auto-dna-stale-event,legacy-hidden-class-disconnect,unsupported-creation-ordinary,unsupported-creation-exclusive,body-unavailable-keyboard,body-unavailable-mouse \
  PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
  python3 -B tests/sv_name_editor_submit_checks.py
git diff --check
```

Result: **PASS**, exit 0, all 61 endpoint executions. Production sources
compiled with `-Wall -Wextra -Werror`, ASan and UBSan; assertions and
sanitizers passed. Existing declared-font fallback and SDL `Leaked thread`
diagnostics appeared without failures. `git diff --check` — **PASS**.

The runner's focused selector now accepts comma-separated cases, retaining the
single-case and unfiltered behavior. The selection contains 32 default and 29
alternate-order endpoint executions, including all 13 new legacy cases in both
orders. The existing session-interleave/stale/disconnect cases are default-only;
automatic-DNA stale and hidden-class disconnect controls run in both orders.
Coverage includes Normal/Fruit Bat keyboard/mouse confirmation, legal random,
explicit/automatic DNA, invalidation, illegal identity rejection, stable
highlight across navigation and stale input, first-run/RPG eligibility,
traditional/point-buy stats, Backspace to stats and then the nearest available
race/class predecessor, re-entry with preserved highlight, and EOF/no-send.
Modern body/DNA/Archer-random controls, IDDC/PvP and post-mode Maia flows,
unsupported-mode fail-fast and session guards are included.

Legacy/shared files were not changed. No unrelated improvement opportunity
was identified. Final resolution adds only this evidence and the map entry to
the tested implementation. Full `sv-build`, unfiltered `sv-core`, assembly and
`controller-review` remain orchestrator-owned and were not run by this worker.
`P1-birth-runtime-evidence` / P1-AC30 remains deferred, not run, at its existing
named producer/verifier and accepted contract digest
`15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6`;
this implementation repair does not claim full acceptance or SV-B-024 sends.

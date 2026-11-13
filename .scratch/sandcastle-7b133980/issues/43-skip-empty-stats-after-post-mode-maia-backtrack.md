# 43: Пропустить пустые stats после post-mode PvP Maia Backspace

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** При authoritative unknown creation setup (`creation_flags > 1`) Backspace из обязательного post-mode PvP Maia trait возвращает игрока к подготовленному mode owner, минуя недоступную stats surface. Обычный и dedicated PvP flow сохраняют подтверждённый draft и позволяют заново выбрать mode и post-mode trait в обеих compile-time class/race order configurations без partial или completed send.

**Blocked by:** 28: Пропустить unusable stats для unknown creation setup (completed); 32: Восстановить обычный owner graph после post-mode PvP Maia (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-29-1

Labels: bug, ready-for-agent

**Finding ID:** `D-498640166ca1cc34` (`birth.unknown-creation-flags-stats-deadlock`), coherent repair of the post-mode PvP Maia recurrence only; the resolved forward-entry repair in ticket 28 remains preserved.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative `creation_flags > 1`, server-provided Maia choices и ordinary либо dedicated PvP intent; реальные SDL mode, trait и Backspace events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, legal choices, confirmed draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC14, P1-AC20 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path regression для ordinary PvP Maia с `creation_flags > 1` достигает обязательного post-mode trait и нажимает Backspace; green outcome не назначает пустой stats owner, а сразу показывает подготовленный legal mode owner с сохранёнными ранее подтверждёнными choices и неизменным baseline representation недоступных body/stats. [Contract: P1-AC02, P1-AC08, P1-AC14, P1-AC20, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Тот же end-to-end regression проходит для production dedicated PvP handoff; ordinary и dedicated cases выполняются в обеих compile-time class/race order configurations и не нарушают уже проверенные unknown-setup forward skip и deeper post-mode owner graph. [Contract: P1-AC02, P1-AC14, P1-AC19, P1-AC20, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] После возврата к mode игрок может подтвердить legal PvP mode повторно, снова получить обязательный Maia trait owner и завершить его legal confirm либо random choice; illegal input сохраняет owner, highlight и draft, а flow не застревает на zero-choice stats. [Contract: P1-AC02, P1-AC05, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC14, P1-AC20; checks: `sv-core`]
- [x] Backspace, повторный выбор, Q/Ctrl-Q, disconnect и stale-generation input в repaired branch не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC14, P1-AC20, P1-AC22, P1-AC29; checks: `sv-core`]
- [x] Repair остаётся по возможности SV-local, использует только production interaction/input-router path и не добавляет fixed-response или test-only behavior implementation; обнаруженные unrelated improvements только записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

## Answer

Repair выполнен локально в production SV birth interaction. Backspace из
обязательного post-mode PvP Maia trait по-прежнему возвращает к stats для
известных setup `0/1`, но при authoritative `creation_flags > 1` заново
подготавливает server-derived mode choices и сразу передаёт ownership mode.
Подтверждённые sex/class/race/mode остаются в draft, а недоступные body/stats
сохраняют baseline representation `NONE` / `-1` и нулевые legal counts.

Согласованный public seam: `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative unknown setup, Maia choices и ordinary либо
dedicated PvP intent; настоящие SDL mode, trait, Backspace, illegal, stale,
quit и disconnect events проходят native birth adapter и production
interaction/input router. Outcome наблюдает owners, legal lists, highlight и
draft. Peer допускает только name packet и затем требует EOF, поэтому любой
partial/completed birth send провалил бы scenarios.

TDD red → green, 2026-10-08:

- Red: `SV_BIRTH_CASE=unknown-setup-pvp-maia-back-confirm python3 -B
  tests/sv_name_editor_submit_checks.py` — **FAIL**, exit 1: production flow
  после post-mode Backspace назначил zero-choice stats и не смог повторно
  достичь READY.
- Green: тот же command после минимального SV-local перехода — **PASS**, exit
  0 для default и `SV_CLASS_BEFORE_RACE=0` binaries.
- Focused green slices: cases `unknown-setup-pvp-maia-back`,
  `unknown-setup-pvp-maia-back-confirm`,
  `unknown-setup-exclusive-pvp-maia-back-random` и
  `unknown-setup-pvp-maia-back-disconnect` — **PASS**, exit 0 каждый в обеих
  compile configurations с `-Wall -Wextra -Werror`, ASan и UBSan. Они покрывают
  прямой mode outcome, preserved draft/highlight, legal confirm и random trait,
  illegal/stale input, Q/Ctrl-Q и disconnect без запрещённых outbound bytes.
- Focused regressions: cases `unknown-setup-mode`, `unknown-setup-mode-back`,
  `mode-pvp-maia-back`, `mode-pvp-maia-deep-back` и
  `exclusive-pvp-maia-deep-back` — **PASS**, exit 0 каждый в обеих compile
  configurations. `git diff --check` — **PASS**.

Legacy/shared gameplay code и test-only behavior не менялись; новых
out-of-scope improvements не обнаружено. Полные `sv-build`, `sv-core` и
controller review не запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** до full
acceptance на exact accepted contract digest и HEAD.

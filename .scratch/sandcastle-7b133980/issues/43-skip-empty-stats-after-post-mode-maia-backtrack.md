# 43: Пропустить пустые stats после post-mode PvP Maia Backspace

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** При authoritative unknown creation setup (`creation_flags > 1`) Backspace из обязательного post-mode PvP Maia trait возвращает игрока к подготовленному mode owner, минуя недоступную stats surface. Обычный и dedicated PvP flow сохраняют подтверждённый draft и позволяют заново выбрать mode и post-mode trait в обеих compile-time class/race order configurations без partial или completed send.

**Blocked by:** 28: Пропустить unusable stats для unknown creation setup (completed); 32: Восстановить обычный owner graph после post-mode PvP Maia (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-498640166ca1cc34` (`birth.unknown-creation-flags-stats-deadlock`), coherent repair of the post-mode PvP Maia recurrence only; the resolved forward-entry repair in ticket 28 remains preserved.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative `creation_flags > 1`, server-provided Maia choices и ordinary либо dedicated PvP intent; реальные SDL mode, trait и Backspace events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, legal choices, confirmed draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC14, P1-AC20 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production-path regression для ordinary PvP Maia с `creation_flags > 1` достигает обязательного post-mode trait и нажимает Backspace; green outcome не назначает пустой stats owner, а сразу показывает подготовленный legal mode owner с сохранёнными ранее подтверждёнными choices и неизменным baseline representation недоступных body/stats. [Contract: P1-AC02, P1-AC08, P1-AC14, P1-AC20, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Тот же end-to-end regression проходит для production dedicated PvP handoff; ordinary и dedicated cases выполняются в обеих compile-time class/race order configurations и не нарушают уже проверенные unknown-setup forward skip и deeper post-mode owner graph. [Contract: P1-AC02, P1-AC14, P1-AC19, P1-AC20, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] После возврата к mode игрок может подтвердить legal PvP mode повторно, снова получить обязательный Maia trait owner и завершить его legal confirm либо random choice; illegal input сохраняет owner, highlight и draft, а flow не застревает на zero-choice stats. [Contract: P1-AC02, P1-AC05, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC14, P1-AC20; checks: `sv-core`]
- [ ] Backspace, повторный выбор, Q/Ctrl-Q, disconnect и stale-generation input в repaired branch не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC14, P1-AC20, P1-AC22, P1-AC29; checks: `sv-core`]
- [ ] Repair остаётся по возможности SV-local, использует только production interaction/input-router path и не добавляет fixed-response или test-only behavior implementation; обнаруженные unrelated improvements только записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

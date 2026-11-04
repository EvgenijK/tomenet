# 32: Восстановить обычный owner graph после post-mode PvP Maia

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** После Backspace из обязательного post-mode PvP Maia trait игрок может повторить stats, но при следующем Backspace к более раннему выбору production birth flow выходит из post-mode phase, восстанавливает обычные trait availability, stable highlight и draft и продолжает Backspace по нормальному owner graph. Игрок может изменить body, trait или race и снова пройти вперёд в обеих compile-time class/race order configurations, включая недоступные trait/body steps и ordinary либо dedicated PvP, без partial или completed send.

**Blocked by:** 06: Сохранить alternate class/race compile order (completed); 15: Восстановить stats при возврате Backspace (completed); 23: Войти в dedicated birth через production E selection (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-21-1

Labels: bug, ready-for-agent

**Finding ID:** `D-b1166b14335e6828` (`birth.pvp-maia-backtrack-context-leak`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided birth setup и ordinary/dedicated PvP intent; реальные SDL Backspace, navigation и confirm events проходят native birth adapter и production interaction/input router, а `SvEndpointOutcome` и controlled peer наблюдают owner, legal trait identities, stable highlight, committed draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC05, P1-AC10, P1-AC14, P1-AC16, P1-AC17, P1-AC20 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path regression проходит mode → обязательный PvP Maia trait → Backspace к stats → Backspace глубже и воспроизводит stale post-mode trait context; green outcome очищает или безопасно приостанавливает этот context до назначения обычного compile-order predecessor. [Contract: P1-AC02, P1-AC14, P1-AC17, P1-AC20, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] В class-before-race order глубокий Backspace проходит через доступный body, затем обычный trait и race; при недоступных body или ordinary trait соответствующий owner пропускается. В race-before-class order он сначала возвращается к class, затем через фактически доступные body/trait owners к race. Ни один переход не открывает stale Enlightened/Corrupted-only list. [Contract: P1-AC05, P1-AC10, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC19, P1-AC20; checks: `sv-build`, `sv-core`]
- [x] При доступном ordinary trait восстанавливаются его pre-mode legal identities, stable highlighted server identity и committed draft; дальнейшее изменение body, trait или race пересчитывает совместимость, а повторное продвижение к mode заново формирует требуемый post-mode PvP Maia choice из актуального draft. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC07, P1-AC10, P1-AC14, P1-AC16, P1-AC20, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Одинаковый multi-level backtracking сценарий проходит для ordinary PvP и production dedicated PvP handoff в обеих compile configurations; Backspace, reject, reconfirm и quit не отправляют partial/completed character result, а final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC19, P1-AC20, P1-AC29; checks: `sv-build`, `sv-core`]
- [x] Repair и regression tests используют только SV production interaction/input-router path, минимизируют изменения legacy/shared code и не добавляют test-only choice implementation; найденные вне scope улучшения только записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-21-1`. TDD seam is the ticket-approved public `sv_endpoint_run(SvEndpointOptions)` path through authoritative setup, native SDL birth input and the production interaction/input router.

## Answer

Исправление выполнено локально в SV birth interaction. При первом входе в
обязательный post-mode PvP Maia trait сохраняются ordinary trait highlight и
committed draft. Backspace из post-mode trait по-прежнему возвращает к чистому
stats interaction; следующий Backspace атомарно снимает post-mode context,
восстанавливает ordinary legal trait list/highlight/draft, сбрасывает mode и
назначает predecessor по текущему compile order и фактическим trait/body skips.
Повторный выбор не-PvP mode также восстанавливает ordinary draft, а повторный
PvP проход не перезаписывает сохранённый snapshot post-mode identity.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative setup и ordinary/dedicated PvP intent; настоящие SDL
events проходят `sv_native_birth_event()` и `sv_birth_command()`. Outcome
наблюдает owner, legal identities, highlight и draft, а peer допускает только
name request и затем требует EOF, поэтому partial/completed birth send ломает
сценарий. Test-only choice implementation не добавлена; legacy/shared files не
изменялись.

TDD red → green, 2026-10-07:

- `SV_BIRTH_CASE=mode-pvp-maia-deep-back python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: после stats Backspace outcome сохранил post-mode legal traits `14/15` вместо ordinary identities `1/2`. После SV repair — **GREEN**, exit 0 для default и `SV_CLASS_BEFORE_RACE=0` binaries.
- Та же production slice для dedicated handoff: `SV_BIRTH_CASE=exclusive-pvp-maia-deep-back python3 -B tests/sv_name_editor_submit_checks.py` — **GREEN**, exit 0 в обеих compile configurations.
- Skip/re-entry coverage: focused cases `mode-pvp-maia-skip-deep-back`, `exclusive-pvp-maia-skip-deep-back`, `mode-pvp-maia-trait-reenter` и `exclusive-pvp-maia-trait-reenter` — **GREEN**, exit 0 каждый в обеих configurations. Они проверяют ordinary trait skip и изменение restored trait с повторным формированием `14/15` post-mode list.
- Существующие regressions `mode-pvp-maia`, `mode-pvp-maia-back` и `stats-order-reenter-pvp-maia` — **GREEN**, exit 0 каждый в обеих configurations. `git diff --check` — **PASS**.

Полные `sv-build`, `sv-core` и controller review не запускались: их выполняет
orchestrator после assembly. `P1-birth-runtime-evidence` / P1-AC30 остаётся
**deferred; not run** до full acceptance на exact contract digest и HEAD.

# 32: Восстановить обычный owner graph после post-mode PvP Maia

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** После Backspace из обязательного post-mode PvP Maia trait игрок может повторить stats, но при следующем Backspace к более раннему выбору production birth flow выходит из post-mode phase, восстанавливает обычные trait availability, stable highlight и draft и продолжает Backspace по нормальному owner graph. Игрок может изменить body, trait или race и снова пройти вперёд в обеих compile-time class/race order configurations, включая недоступные trait/body steps и ordinary либо dedicated PvP, без partial или completed send.

**Blocked by:** 06: Сохранить alternate class/race compile order (completed); 15: Восстановить stats при возврате Backspace (completed); 23: Войти в dedicated birth через production E selection (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-b1166b14335e6828` (`birth.pvp-maia-backtrack-context-leak`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided birth setup и ordinary/dedicated PvP intent; реальные SDL Backspace, navigation и confirm events проходят native birth adapter и production interaction/input router, а `SvEndpointOutcome` и controlled peer наблюдают owner, legal trait identities, stable highlight, committed draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC05, P1-AC10, P1-AC14, P1-AC16, P1-AC17, P1-AC20 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production-path regression проходит mode → обязательный PvP Maia trait → Backspace к stats → Backspace глубже и воспроизводит stale post-mode trait context; green outcome очищает или безопасно приостанавливает этот context до назначения обычного compile-order predecessor. [Contract: P1-AC02, P1-AC14, P1-AC17, P1-AC20, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] В class-before-race order глубокий Backspace проходит через доступный body, затем обычный trait и race; при недоступных body или ordinary trait соответствующий owner пропускается. В race-before-class order он сначала возвращается к class, затем через фактически доступные body/trait owners к race. Ни один переход не открывает stale Enlightened/Corrupted-only list. [Contract: P1-AC05, P1-AC10, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC19, P1-AC20; checks: `sv-build`, `sv-core`]
- [ ] При доступном ordinary trait восстанавливаются его pre-mode legal identities, stable highlighted server identity и committed draft; дальнейшее изменение body, trait или race пересчитывает совместимость, а повторное продвижение к mode заново формирует требуемый post-mode PvP Maia choice из актуального draft. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC07, P1-AC10, P1-AC14, P1-AC16, P1-AC20, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Одинаковый multi-level backtracking сценарий проходит для ordinary PvP и production dedicated PvP handoff в обеих compile configurations; Backspace, reject, reconfirm и quit не отправляют partial/completed character result, а final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC19, P1-AC20, P1-AC29; checks: `sv-build`, `sv-core`]
- [ ] Repair и regression tests используют только SV production interaction/input-router path, минимизируют изменения legacy/shared code и не добавляют test-only choice implementation; найденные вне scope улучшения только записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

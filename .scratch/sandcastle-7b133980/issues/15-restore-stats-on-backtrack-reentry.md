# 15: Восстановить stats при возврате Backspace

**Type:** implementation

**What to build:** После Backspace из mode или другого допустимого возврата к stats игрок получает снова пригодное к редактированию stats interaction: может заменить частичный или уже завершённый ordered-stat выбор, подтвердить новый legal результат и снова перейти вперёд без stale consumption или сетевой отправки.

**Blocked by:** 09: Завершить или откатить creation без send (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-a034cf7e7cd5e676` (`birth.stats-backtrack-reentry-state`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` ведёт production birth flow до stats/mode и возвращает Backspace через native SDL adapter и `sv_birth_command()`; outcome наблюдает legal stats, order/allocation, highlight, confirmation flags, owner и draft, а socket boundary — отсутствие send. Seam принят P1-AC08, P1-AC17, P1-AC18 и P1-AC28; проверка — `sv-core`.

- [ ] Возврат к ordered stats восстанавливает согласованные legal choices, count, order progress, stable highlight и confirmation state как для частично набранного, так и для полностью подтверждённого порядка; ранее consumed список не блокирует новый confirm. [Contract: P1-AC08, P1-AC10, P1-AC17; checks: `sv-core`]
- [ ] Re-entry определяет и проверяет явную baseline-семантику ранее committed stats draft: новый выбор заменяет его только после legal production confirmation, а illegal/stale input сохраняет согласованный draft и owner. [Contract: P1-AC02, P1-AC08, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Production-path scenarios проходят Backspace после partial и completed ordered stats, делают новый keyboard/mouse выбор и снова достигают mode в обеих compile configurations. [Contract: P1-AC11, P1-AC17, P1-AC18, P1-AC19, P1-AC28; checks: `sv-build`, `sv-core`]
- [ ] Тот же re-entry проверен через dedicated PvP Maia owner path с сохранением его принятого owner graph; Backspace/reconfirm не отправляют partial или completed packet. [Contract: P1-AC02, P1-AC18, P1-AC20, P1-AC29; checks: `sv-core`]
- [ ] Исправление и tests остаются в SV production path и не вводят отдельную test-only stats implementation. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

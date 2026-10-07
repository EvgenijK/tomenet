# 34: Возобновить automatic DNA после ordered stats

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Когда `%` приводит automatic reincarnation к ordered stats, игрок вручную завершает legal six-stat order, не теряя pending automatic state; при переходе к mode production birth interaction автоматически применяет legal mode DNA и продолжает baseline flow. Поведение работает в ordinary и dedicated creation в обеих compile-time class/race order configurations, не меняя уже проверенную point-buy DNA semantics и не добавляя persistence или final send.

**Blocked by:** 19: Продолжить automatic DNA choices через birth flow (completed); 24: Нормализовать point-buy DNA отдельно для каждого stat (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-36ccf3e367566848` (`birth.dna-auto-reincarnation-semantics`), residual repair of the still-open ledger finding.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative creation setup и DNA identities; реальный SDL `%`, ручные ordered-stat navigation/confirm events и последующий mode transition проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают pending automatic state, owner, highlights/draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production-path scenario с authoritative `creation_flags == 0` доводит `%` до ordered stats и фиксирует baseline suspension: stats DNA не применяется и не помечается illegal, `auto_reincarnation` остаётся pending, а owner/highlight и пустой stat order готовы к обычному ручному вводу. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Keyboard и mouse production input позволяют вручную выбрать все шесть legal stat identities; pending automatic state сохраняется после каждого подтверждения, committed order совпадает с вводом игрока, а шестое подтверждение переводит interaction к mode и автоматически применяет legal mode DNA без дополнительного `%`. [Contract: P1-AC02, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC26, P1-AC28; checks: `sv-core`]
- [ ] Missing или illegal mode DNA после ordered stats останавливает automatic continuation на mode по существующим reject rules, не подтверждает illegal value и оставляет согласованные owner, stable highlight и draft для ручного продолжения; legal mode DNA завершает только local draft. [Contract: P1-AC02, P1-AC09, P1-AC10, P1-AC26, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Production-path regressions покрывают ordinary и dedicated creation с ordered stats в default и `SV_CLASS_BEFORE_RACE=0` configurations; существующие point-buy automatic-DNA scenarios остаются green. Ни один scenario не подменяет interaction fixed-response или test-only implementation. [Contract: P1-AC19, P1-AC20, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Repair остаётся по возможности SV-local, не добавляет `birth.restore-dna`/`birth.save-dna`, DNA persistence, partial/completed-character send или ownership SV-B-024; найденные побочные улучшения только записываются отдельно. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29, P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC08, P2-AC09; checks: `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

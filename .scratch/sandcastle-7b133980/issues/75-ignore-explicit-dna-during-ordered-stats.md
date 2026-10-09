# 75: Сохранить DNA при явном вводе в ordered stats

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Когда ordered stats уже владеет birth input, явный staged-DNA ввод `#` ведёт себя как baseline unrecognized no-op: не меняет stat order, owner, highlight, draft, automatic continuation или общую валидность загруженного DNA outcome. После завершения всех шести legal ordered stats игрок по-прежнему может применить тот же валидный DNA outcome на mode. Унаследованное automatic DNA, point-buy DNA и общая необратимая invalidation действительно missing/illegal outcome сохраняют существующую семантику.

**Blocked by:** 73: Восстановить общую валидность DNA success fixtures (completed).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-e6f756264f5a2d20` (`birth.dna-validity-lifecycle`), one coherent recurrence repair for explicit staged DNA input while ordered stats owns the production interaction. This ticket does not reopen or duplicate the broader lifecycle repair resolved by ticket 71.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative ordered-stat setup и один valid loaded-DNA aggregate; настоящий SDL `#`, ordered-stat navigation/confirm events и последующий mode DNA input проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают aggregate availability, active owner, stat/highlight state, committed/local draft, automatic state, successful mode DNA apply и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [ ] Red production-path scenario в default class-before-race configuration вводит реальный SDL `#` после частичного legal ordered-stat выбора и доказывает baseline no-op: общая DNA availability остаётся true, а owner, выбранный stat order, текущий stable highlight, local/committed draft и automatic state не меняются; partial/completed result не отправляется. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC11, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Тот же interaction после no-op завершает все шесть legal ordered stats и применяет всё ещё валидный staged DNA на mode; production regression проходит также в alternate race-before-class configuration и сохраняет server-provided identity, highlight и draft semantics всех ранее пройденных sex/race/trait/class/body owners. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Guard различает новый explicit staged `#` от automatic state, принятого до ordered stats: explicit input остаётся no-op и не начинает, не останавливает и не возобновляет automatic continuation, тогда как inherited automatic DNA по-прежнему переживает ручной ordered-stat ввод и возобновляет legal mode DNA по существующему production flow. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Focused controls подтверждают, что point-buy staged DNA не изменён, а missing или illegal component на владельце, где DNA действительно применяется, по-прежнему необратимо инвалидирует общий outcome согласно ticket 71; поздние `#`/`%` после такой invalidation остаются отклонёнными. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC26, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Repair остаётся по возможности внутри SV birth interaction и production-path tests, не меняет legacy/shared behavior, не добавляет test-only implementation, DNA persistence, `birth.restore-dna`, `birth.save-dna`, fixed-response wizard или final completed-character send; quit, disconnect и stale-generation suppression остаются green. [Contract: P1-AC02, P1-AC12, P1-AC22, P1-AC26, P1-AC27, P1-AC28, P1-AC29, P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `sv-core`, `controller-review`]
- [ ] Relevant focused endpoint scenarios, `sv-build` and `sv-core` pass at the repair HEAD; any newly discovered unrelated legacy/shared/SV-organization opportunity is recorded separately with problem, affected code, proposal, status and required checks rather than included in this repair. [Contract: P1-AC21, P1-AC24, P1-AC25, P1-AC28, P2-AC06, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to the named full-acceptance producer/verifier and is not claimed by this implementation repair. Scenario-indexed native evidence must still be collected and verified at accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` and the exact accepted HEAD.

# 73: Восстановить общую валидность DNA success fixtures

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Production-path matrix снова достоверно подтверждает прямой успешный выбор DNA для class, race, trait, body, point-buy stats и mode после введения общей валидности загруженного DNA outcome. Каждый такой success scenario начинает с валидного aggregate вместе со своим owner-local fragment; missing и illegal controls, а также сценарии необратимой invalidation, остаются невалидными и продолжают проверять отказ.

**Blocked by:** 71: Сохранить общий lifecycle валидности birth DNA (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-95991cac35e0c4c5` (`sv-birth-tests.shared-dna-success-fixture-validity`), one coherent repair of the aggregate-valid setup and exact-HEAD evidence for direct DNA success fixtures.

**Production seam for repair verification:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided birth setup и loaded DNA fixture; реальные SDL `#` и последующие quit events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` наблюдает owner, confirmed value, DNA-used marker, aggregate availability и отсутствие запрещённых outbound bytes. Seam принят P1-AC03–P1-AC10, P1-AC26, P1-AC28 и P2-AC05; проверка — `sv-core`. Ticket меняет только setup production-path tests, а не production behavior или отдельную test-only реализацию.

- [ ] Direct success scenarios `class-dna`, `race-dna`, `trait-dna`, `body-dna`, `stats-point-dna` и `mode-dna` объявляют валидным общий loaded DNA outcome вместе с нужным owner-local fragment; каждый реальный `#` снова подтверждает ожидаемое server identity/value через production seam в обеих compile-order configurations. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Missing/illegal direct DNA controls сохраняют false aggregate либо отклоняются по своему intended owner-local constraint; scenarios из ticket 71 сохраняют необратимую общую invalidation после missing/illegal обязательного component, так что fixture repair не ослабляет production guard и не создаёт ложноположительных результатов. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] До исправления хотя бы один затронутый direct success scenario воспроизводит rejection из-за false aggregate, а после исправления все шесть success scenarios и репрезентативные missing/illegal/shared-invalidation controls проходят через тот же production seam без изменения production-кода, legacy/shared gameplay или протокольной семантики. [Contract: P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC26, P1-AC28, P2-AC01, P2-AC04, P2-AC05, P2-AC09; checks: `sv-core`, `controller-review`]
- [ ] На exact repair HEAD выполняется полный unfiltered `sv-core` с явно снятой переменной `SV_BIRTH_CASE`; сохраняется полный output, доказывающий запуск всего inventoried scenario set, успешное завершение обеих compile-order configurations и отсутствие compiler, sanitizer или assertion failures. Focused-only запуск не выдаётся за эту проверку. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC20, P1-AC21, P1-AC22, P1-AC24, P1-AC25, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Repair не добавляет DNA persistence, `birth.restore-dna`, `birth.save-dna`, final completed-character send, fixed-response wizard или test-only behavior implementation; новые unrelated improvement opportunities только записываются отдельно с обязательными полями и не входят в этот ticket. [Contract: P1-AC26, P1-AC27, P1-AC28, P1-AC29, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим fixture/evidence repair; native evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и exact accepted HEAD.

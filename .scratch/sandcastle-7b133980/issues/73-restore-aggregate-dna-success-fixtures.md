# 73: Восстановить общую валидность DNA success fixtures

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Production-path matrix снова достоверно подтверждает прямой успешный выбор DNA для class, race, trait, body, point-buy stats и mode после введения общей валидности загруженного DNA outcome. Каждый такой success scenario начинает с валидного aggregate вместе со своим owner-local fragment; missing и illegal controls, а также сценарии необратимой invalidation, остаются невалидными и продолжают проверять отказ.

**Blocked by:** 71: Сохранить общий lifecycle валидности birth DNA (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-50-1

Labels: bug, ready-for-agent

**Finding ID:** `D-95991cac35e0c4c5` (`sv-birth-tests.shared-dna-success-fixture-validity`), one coherent repair of the aggregate-valid setup and exact-HEAD evidence for direct DNA success fixtures.

**Production seam for repair verification:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided birth setup и loaded DNA fixture; реальные SDL `#` и последующие quit events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` наблюдает owner, confirmed value, DNA-used marker, aggregate availability и отсутствие запрещённых outbound bytes. Seam принят P1-AC03–P1-AC10, P1-AC26, P1-AC28 и P2-AC05; проверка — `sv-core`. Ticket меняет только setup production-path tests, а не production behavior или отдельную test-only реализацию.

- [x] Direct success scenarios `class-dna`, `race-dna`, `trait-dna`, `body-dna`, `stats-point-dna` и `mode-dna` объявляют валидным общий loaded DNA outcome вместе с нужным owner-local fragment; каждый реальный `#` снова подтверждает ожидаемое server identity/value через production seam в обеих compile-order configurations. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Missing/illegal direct DNA controls сохраняют false aggregate либо отклоняются по своему intended owner-local constraint; scenarios из ticket 71 сохраняют необратимую общую invalidation после missing/illegal обязательного component, так что fixture repair не ослабляет production guard и не создаёт ложноположительных результатов. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] До исправления хотя бы один затронутый direct success scenario воспроизводит rejection из-за false aggregate, а после исправления все шесть success scenarios и репрезентативные missing/illegal/shared-invalidation controls проходят через тот же production seam без изменения production-кода, legacy/shared gameplay или протокольной семантики. [Contract: P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC26, P1-AC28, P2-AC01, P2-AC04, P2-AC05, P2-AC09; checks: `sv-core`, `controller-review`]
- [x] На exact repair HEAD выполняется полный unfiltered `sv-core` с явно снятой переменной `SV_BIRTH_CASE`; сохраняется полный output, доказывающий запуск всего inventoried scenario set, успешное завершение обеих compile-order configurations и отсутствие compiler, sanitizer или assertion failures. Focused-only запуск не выдаётся за эту проверку. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC20, P1-AC21, P1-AC22, P1-AC24, P1-AC25, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Repair не добавляет DNA persistence, `birth.restore-dna`, `birth.save-dna`, final completed-character send, fixed-response wizard или test-only behavior implementation; новые unrelated improvement opportunities только записываются отдельно с обязательными полями и не входят в этот ticket. [Contract: P1-AC26, P1-AC27, P1-AC28, P1-AC29, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим fixture/evidence repair; native evidence собирается только на exact accepted contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и exact accepted HEAD.

## Comments

- 2026-10-09: Claimed by `codex/sandcastle-7b133980-wave-50-1`.
  Confirmed the ticket-approved production seam `sv_endpoint_run(SvEndpointOptions)`
  before changing the fixture.

## Progress

Исправлен только production-path test setup. Шесть прямых success fixtures
(`class-dna`, `race-dna`, `trait-dna`, `body-dna`, `stats-point-dna`,
`mode-dna`) теперь передают общий valid loaded-DNA aggregate вместе со своим
owner-local fragment. Их assertions дополнительно наблюдают сохранённую общую
availability через `SvEndpointOutcome`. Missing/illegal controls явно проверяют
false aggregate после отказа; сценарии общей необратимой invalidation из ticket
71 остались без изменений. Та же fixture correction применена к двум companion
success flows (`mode-dedicated-dna` и post-mode DNA trait), найденным обязательным
unfiltered matrix; reject controls не расширены. Production, legacy/shared
gameplay и protocol code не менялись.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Authoritative
setup и loaded DNA поступают в endpoint, реальный SDL `#` проходит native birth
adapter и production birth-interaction/input-router boundary, а outcome наблюдает
owner, confirmed server identity/value, DNA-used marker и aggregate availability.
Controlled peer по-прежнему требует отсутствие запрещённых outbound bytes.

Red → green:

- `SV_BIRTH_CASE=class-dna python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1 / SIGABRT: `birth_class_confirmed` оставался false из-за
  false aggregate; после fixture repair — **GREEN**, exit 0.

Focused verification:

- Все шесть direct success cases тем же runner — **PASS**, exit 0 в обеих
  compile-order configurations.
- `class-dna-incompatible`, `race-dna-missing`, `trait-dna-incompatible`,
  `body-dna-missing`, `stats-point-dna-missing`, `mode-dna-missing`,
  `auto-dna-shared-manual-advance` и `auto-dna-shared-backtrack-reentry` —
  **PASS**, exit 0; shared-invalidation cases выполнены в обеих configurations.
- `env -u SV_BIRTH_CASE bash .sandcastle/checks.sh core` на exact committed
  repair HEAD — DNA success/invalidation matrix проходит исправленные места,
  но полный gate **FAIL**, exit 1 позже на unrelated existing scenario
  `case-sensitive-mode-ordinary-hellish`: assertion
  `birth_highlighted_mode == SV_BIRTH_MODE_HELLISH`. Та же ошибка воспроизводится
  focused. Полный output сохранён в
  `/tmp/sandcastle-7b133980-ticket-73-sv-core.log`; обе compile-order
  configurations поэтому не завершены, и full check не объявляется пройденным.
- `git diff --check` — **PASS**.

Unrelated blocker записан отдельно в `docs/sv-improvements.md`; его исправление
не включено в DNA fixture repair. Полный build/review workflow не запускался;
его выполняет orchestrator. `P1-birth-runtime-evidence`
/ P1-AC30 остаётся **deferred; not run** у named full-acceptance owner на exact
accepted contract digest и HEAD.

## Answer

Восстановлена достоверность production-path matrix на согласованном публичном
seam `sv_endpoint_run(SvEndpointOptions)`. Commit `166bbe508f` сохранил только
fixture repair: шесть direct DNA success scenarios и два companion success flow
получают valid loaded-DNA aggregate вместе с owner-local fragment; missing,
illegal и shared-invalidation controls сохраняют отказ и false aggregate.
Последующий unfiltered tracer выявил тот же stale aggregate setup в 13
`hidden-derived-dna*` success/mismatch flows. Они теперь также начинают с valid
aggregate; exact `missing`, `incompatible` и `stale` controls остаются false.
Тот же repair применён к трём point-buy DNA normalization variants
(`illegal`, `over-budget`, `multiple-max`), которые подтверждают загруженные
stats после owner-local normalization; `stats-point-dna-missing` остаётся false.

При exact-HEAD проверке обнаружился не связанный с DNA fixture, но обязательный
для полного gate дефект production native adapter. Authoritative profile setup
уже передавал `first_run=false`, а outcome содержал все шесть legal ordinary
modes. SDL layout translation возвращала ASCII uppercase для Shift/Caps+letter,
после чего `letter_key()` не распознавала identity вне lowercase `SDLK_A..Z`.
SV-local `input/native-birth.c` теперь только для mode owner нормализует
переведённые ASCII `A..Z` в lowercase identity; регистр по-прежнему вычисляется
из Shift/Caps. Остальные owners сохраняют прежнее отклонение uppercase bindings.
Mode legality, case-sensitive binding semantics, protocol и legacy/shared
gameplay не изменены.

Red → green через тот же production seam:

- `SV_BIRTH_CASE=class-dna python3 -B tests/sv_name_editor_submit_checks.py`
  — до aggregate fixture repair **RED**, SIGABRT на false
  `birth_class_confirmed`; после repair **GREEN**, exit 0.
- `SV_BIRTH_CASE=case-sensitive-mode-ordinary-hellish python3 -B tests/sv_name_editor_submit_checks.py`
  — на сохранённом repair checkpoint
  **RED**, exit 1 / SIGABRT: legal-mode count был 6, но uppercase layout key
  отклонялся и highlight оставался Normal; после native identity normalization
  **GREEN**, exit 0 в обеих compile-order configurations.
- `SV_BIRTH_CASE=hidden-derived-dna python3 -B tests/sv_name_editor_submit_checks.py`
  — после первого recovery slice **RED**, exit 1 / SIGABRT: false aggregate
  останавливал class DNA success; после aggregate fixture correction **GREEN**,
  exit 0 в обеих compile-order configurations.
- `SV_BIRTH_CASE=stats-point-dna-over-budget python3 -B tests/sv_name_editor_submit_checks.py`
  — unfiltered tracer дал **RED**, exit 1 / SIGABRT: false aggregate не позволял
  перейти к Mode; после fixture correction **GREEN**, exit 0 в обеих
  compile-order configurations.

Focused evidence:

- Все шесть direct DNA success cases и representative
  missing/illegal/shared-invalidation controls — **PASS** в обеих compile-order
  configurations; результаты сохранены в Progress и commit `166bbe508f`.
- `case-sensitive-mode-lower-h`, `case-sensitive-mode-uppercase-reject`,
  `case-sensitive-mode-dedicated-hellish` и
  `case-sensitive-shift-caps-class-binding` — **PASS**, exit 0, в обеих
  compile-order configurations.
- `hidden-derived-dna`, `hidden-derived-dna-race-compatible`,
  `hidden-derived-dna-race-mismatch`, representative race-refresh flow и exact
  `missing/incompatible/stale` controls — **PASS**; inventoried configurations
  выполнены тем же runner. Race mismatch подтверждает успешный class DNA до
  необратимой aggregate invalidation на несовместимом race fragment.
- `stats-point-dna-illegal`, `stats-point-dna-over-budget`,
  `stats-point-dna-multiple-max` и `stats-point-dna-missing` — **PASS**, exit 0,
  в обеих compile-order configurations; success variants сохраняют aggregate,
  missing control явно наблюдает false.
- `env -u SV_BIRTH_CASE bash .sandcastle/checks.sh core` — **PASS**, exit 0,
  на exact resolved-ticket HEAD; полный output сохранён в
  `/tmp/sandcastle-7b133980-ticket-73-sv-core-exact-head.log`. Runner завершил
  весь inventoried scenario set в обеих compile-order configurations без
  compiler, sanitizer или assertion failures.
- `git diff --check` — **PASS**.

DNA persistence, `birth.restore-dna`, `birth.save-dna`, fixed-response wizard и
final completed-character send не добавлены. `P1-birth-runtime-evidence` /
P1-AC30 остаётся **deferred; not run** у named full-acceptance owner на accepted
contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
и exact accepted HEAD.

# 38: Отклонить новый automatic DNA command в ordered stats

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Когда ordered stats уже владеет birth input, введённый игроком `%` не включает automatic reincarnation, не меняет owner, highlight или draft и оставляет последующий mode выбор ручным. При этом pending automatic state, начатый на более раннем owner, по-прежнему переживает ручной ordered-stat ввод и возобновляется на mode согласно уже проверенному baseline flow.

**Blocked by:** 34: Возобновить automatic DNA после ordered stats (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-25-1

Labels: bug, ready-for-agent

**Finding ID:** `D-36ccf3e367566848` (`birth.dna-auto-reincarnation-semantics`), coherent repair of the newly evidenced owner-local input defect.

**Human disposition:** текущий пользователь явно авторизовал исправление командой исправить перечисленные review findings после уведомления о repair ceiling. Разрешено только узкое SV-local correction из этого ticket; дополнительное расширение automatic-DNA semantics не авторизовано.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative ordered-stat creation setup; настоящий SDL `%` и последующие stat/mode gestures проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают automatic state, owner, highlighted identity, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC26 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path regression достигает ordered stats без ранее pending automatic reincarnation, вводит реальный SDL `%` и подтверждает, что команда отклонена без изменения automatic flag, owner, highlighted identity, partial/committed draft или legal server-provided state. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC11, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] После отклонённого `%` все шесть legal ordered-stat choices проходят production input path, а завершение stats оставляет mode у ручного owner: mode DNA не применяется автоматически и local draft не завершается без отдельного legal mode choice. [Contract: P1-AC02, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC26, P1-AC28; checks: `sv-core`]
- [x] Existing control scenario, где `%` был принят до входа в ordered stats, остаётся green: automatic DNA сохраняет уже проверенные sex/race/trait/class/body semantics, inherited pending state не очищается owner-local reject, переживает шесть ручных stat choices и возобновляет legal mode DNA. Проверка проходит в обеих compile-time class/race order configurations. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC26, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Repair не добавляет DNA persistence, partial/completed-character send, fixed-response или test-only behavior path; final send остаётся у SV-B-024, а изменение остаётся по возможности внутри SV birth interaction и его production-path tests. [Contract: P1-AC02, P1-AC27, P1-AC28, P1-AC29, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC09; checks: `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

## Comments

- 2026-10-08: Claimed by `codex/sandcastle-7b133980-wave-25-1`. TDD seam is the ticket-approved public `sv_endpoint_run(SvEndpointOptions)` path through authoritative ordered-stat setup, native SDL birth input and the production birth interaction/input router.

## Answer

Исправление выполнено локально в SV birth interaction. Новая automatic-DNA
команда теперь отклоняется, если active owner — ordered stats, до
изменения `auto_reincarnation`. Point-buy DNA не изменён. Pending automatic
state, принятый на более раннем owner, сохраняется при повторном
owner-local `%`, переживает ручной ordered-stat input и возобновляет
legal mode DNA через существующий flow.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative `creation_flags == 0` setup; реальные SDL `%`, stat
и mode gestures проходят native adapter и production interaction/input router.
`SvEndpointOutcome` наблюдает automatic state, owner, highlight, legal lists и draft.
Controlled peer требует EOF после разрешённого name packet, поэтому
partial/completed-character send провалил бы scenarios. Test-only behavior
и fixed-response path не добавлены; legacy/shared runtime files не изменялись.

TDD red → green, 2026-10-08:

- `SV_BIRTH_CASE=auto-dna-ordered-local-reject python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: assertion обнаружил, что owner-local `%` выставил
  `birth_auto_reincarnation`; после минимального owner guard — **GREEN**,
  exit 0 в default и `SV_CLASS_BEFORE_RACE=0` configurations.

Focused verification:

- `auto-dna-ordered-local-manual` — **PASS**, exit 0 в обеих configurations:
  шесть legal stats подтверждены вручную, mode остался незавершённым
  у ручного owner без DNA apply.
- `auto-dna-ordered-keyboard` — **PASS**, exit 0 в обеих configurations:
  inherited pending state сохранён после повторного `%` на stats, а sex/race/trait/
  class/body и resumed mode DNA semantics подтверждены.
- `auto-dna-ordered-mouse-dedicated` и `auto-dna-full` — **PASS**, exit 0 каждый
  в обеих configurations; dedicated ordered и point-buy controls не регрессировали.
- `git diff --check` — **PASS**.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review workflow
не запускались: их выполняет orchestrator после assembly. Диагностики
declared font fallback и SDL `Leaked thread` совпадают с существующим
focused-runner output; assertion, compiler и sanitizer failures в green runs не было.
Новых побочных improvement opportunities не обнаружено. `P1-birth-runtime-evidence` /
P1-AC30 остаётся **deferred; not run** до full acceptance на exact contract digest
и HEAD.

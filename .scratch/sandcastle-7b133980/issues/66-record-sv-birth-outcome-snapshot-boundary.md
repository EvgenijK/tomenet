# 66: Зафиксировать birth-owned outcome snapshot boundary

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о том, что application-level endpoint outcome разворачивает почти всё внутреннее состояние birth interaction в плоскую диагностическую поверхность. Запись предлагает будущий birth-owned snapshot/evidence aggregate через устойчивую interaction boundary, но не выполняет рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** Codex (`codex/sandcastle-7b133980-wave-44-2`)

Labels: enhancement, ready-for-agent

**Finding ID:** `D-901b12e65f193779` (`birth.endpoint-outcome-state-exposure-unrecorded`), one coherent documentation repair; the proposed outcome-boundary refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change `sv_endpoint_run(SvEndpointOptions)`, production or test behavior, or invent a test-only interface. Preservation of the accepted production interaction/input-router path is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [x] The SV improvement register contains exactly one standalone record describing the flattened publication of detailed birth interaction state through the endpoint outcome as an orchestration/diagnostics coupling problem. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [x] The record states Status, Problem, Affected code, Proposal and Required checks, identifies endpoint outcome publication and its public outcome type, and proposes a birth-owned snapshot/evidence aggregate exposed through the stable interaction boundary. [Contract: P2-AC06, P2-AC07, P2-AC12; check: `controller-review`]
- [x] The proposal keeps gameplay detail behind its responsible interface, preserves the current public production endpoint seam and observable evidence semantics, and marks the refactor as separate future SV-local work rather than changing production behavior or legacy/shared code in this repair. [Contract: P1-AC02, P1-AC28, P2-AC01, P2-AC03, P2-AC04, P2-AC08, P2-AC09; check: `controller-review`]
- [x] Required future checks cover complete field-to-snapshot evidence mapping, generation and lifecycle data, owner/highlight/draft state, Guide placeholder evidence, partial/completed-send suppression, both compile orders and existing production endpoint scenarios; they are recorded as required and explicitly not claimed passed by this documentation repair. [Contract: P1-AC02, P1-AC19, P1-AC21, P1-AC22, P1-AC24, P1-AC25, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

## Comments

- 2026-10-08: Claimed by `codex/sandcastle-7b133980-wave-44-2` before
  inspecting endpoint outcome publication and recording the standalone
  improvement opportunity.

## Answer

В `docs/sv-improvements.md` добавлена ровно одна самостоятельная запись
`Publish birth evidence through a birth-owned snapshot`. Она фиксирует coupling:
application-level `publish_outcome` читает почти всё детальное состояние birth
interaction и разворачивает его в десятки плоских `birth_*` полей публичного
`SvEndpointOutcome`, поэтому orchestration и diagnostics зависят от внутренних
деталей birth owner.

Proposal оставлен отдельной будущей SV-local работой: birth-owned copied
snapshot/evidence aggregate должен формироваться за существующей стабильной
границей `input/birth-interaction.h` и становиться birth diagnostics portion,
потребляемой endpoint outcome publication. Запись сохраняет production entry
seam `sv_endpoint_run(SvEndpointOptions)`, все наблюдаемые evidence values,
defaults, ordering, copy/lifetime и send-suppression semantics, оставляет
gameplay rules за birth interface и не меняет legacy/shared code или scope
SV-B-023.

Production seam для этого documentation-only repair неприменим, как прямо
зафиксировано контрактом ticket: production/test code и behavior не менялись.
Поэтому behavioral TDD, production-path session и новые behavior tests не
запускались; skill `tdd` применён как запрет на создание искусственного
test-only seam. Выполнена сфокусированная структурная red/green проверка:

- RED до записи: shell assertion единственности заголовка
  `## Publish birth evidence through a birth-owned snapshot` — exit 1,
  `heading_count=0`;
- GREEN после записи: тот же count плюс assertions полей Status, Problem,
  Affected code, Proposal и Required checks, production boundaries,
  preservation constraints и будущей check matrix — exit 0,
  `heading_count=1; structure_and_contract=present`;
- `git diff --check` — PASS, exit 0;
- scope inspection — PASS: изменены только improvement register, этот Wayfinder
  history ticket и его map entry; production SV, tests, canonical SV-B-023 и
  legacy/shared code не изменены.

Полный build/test/review workflow не запускался согласно указанию: behavior не
изменялся, а orchestrator выполняет общие gates после tickets. Будущие
`sv-build`, `sv-core`, `controller-review` и полный production endpoint birth
scenario matrix записаны как обязательные для самого refactor и не заявлены
пройденными этой документационной задачей. `P1-birth-runtime-evidence` /
P1-AC30 остаётся **deferred; not run** у named producer/verifier на full
acceptance.

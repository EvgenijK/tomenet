# 67: Зафиксировать единый inventory production endpoint test sources

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о семи Python runners, которые дублируют production endpoint source inventory и могут разойтись при добавлении новой production dependency. Запись предлагает будущий authoritative test build inventory/helper при сохранении production-path execution и изоляции runners, но не выполняет консолидацию и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** Codex (`codex/sandcastle-7b133980-wave-44-3`)

Labels: enhancement, ready-for-agent

**Finding ID:** `D-ba1f78c75abfd157` (`sv-tests.endpoint-source-inventory-duplication-unrecorded`), one coherent documentation repair; the proposed test-build consolidation remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change the runners, production sources or public `sv_endpoint_run(SvEndpointOptions)` path, and must not create a test-only behavior implementation. Preservation of production-path execution is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [x] The SV improvement register contains exactly one standalone record describing the duplicated production endpoint source inventory across all seven current Python runners and the resulting dependency-drift risk. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [x] The record states Status, Problem, Affected code, Proposal and Required checks, identifies the seven affected endpoint runners, and proposes one authoritative test build inventory/helper while retaining runner-specific scenario orchestration and isolation. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [x] The proposal preserves compilation and execution of the real production endpoint, native birth adapter and birth interaction path, forbids moving behavior into a test-only implementation, and marks consolidation as separate future test-infrastructure work rather than changing production or legacy/shared code in this repair. [Contract: P1-AC28, P2-AC01, P2-AC03, P2-AC04, P2-AC05, P2-AC08, P2-AC09; check: `controller-review`]
- [x] Required future checks cover authoritative inventory completeness and ordering, all seven runner consumers, runner-specific sources/flags, dependency additions and removals, both birth compile orders where applicable, and existing production endpoint regression runs; they are recorded as required and explicitly not claimed passed by this documentation repair. [Contract: P1-AC19, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-44-3`.

## Answer

В `docs/sv-improvements.md` добавлена ровно одна самостоятельная запись
`Centralize production endpoint test build inventory`. Она фиксирует риск
dependency drift из-за независимо перечисляемых production sources во всех семи
Python runners: `sv_endpoint_checks.py`, `sv_account_create_checks.py`,
`sv_account_failure_checks.py`, `sv_first_session_checks.py`,
`sv_name_editor_submit_checks.py`, `sv_name_rejection_recovery_checks.py` и
`sv_ordinary_create_cancel_checks.py`.

Proposal оставлен отдельной будущей test-infrastructure работой: один
authoritative ordered inventory/helper только для общих production sources, при
этом каждый runner сохраняет собственные scenario harness и orchestration,
executable, дополнительные sources, flags и environment. Будущий helper обязан
компилировать и исполнять реальный `sv_endpoint_run(SvEndpointOptions)`, native
birth adapter и birth interaction/input-router path; перенос поведения в
test-only implementation запрещён. Production, tests, legacy/shared code и scope
SV-B-023 этой правкой не изменены.

Production seam для изменения поведения неприменим: тикет documentation-only и
согласованная граница `sv_endpoint_run(SvEndpointOptions)` должна остаться
неизменной. Поэтому behavioral TDD и новые behavior tests не требовались и
test-only seam не создавался. Навык TDD использован как ограничение этой границы.
Для самой записи выполнена сфокусированная структурная red/green проверка:

- до добавления записи Python assertion на единственный заголовок
  `## Centralize production endpoint test build inventory` — **RED**, exit 1,
  `heading_count=0`;
- после добавления assertions на единственный заголовок, пять обязательных
  полей, ровно семь runners, production-path constraints и future-check matrix —
  **GREEN**, exit 0,
  `heading_count=1; fields=5; runners=7; scope_and_future_checks=present`;
- `SV_BIRTH_CASE=guide-sex-no-op python3 tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0; существующий production endpoint scenario прошёл в default
  и `SV_CLASS_BEFORE_RACE=0` compile orders;
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: его выполняет orchestrator.
Будущие проверки completeness/ordering inventory, всех семи consumers,
runner-specific sources/flags, dependency additions/removals, `sv-build`,
`sv-core`, `controller-review` и полные production endpoint regressions записаны
как обязательные для будущей консолидации и не заявляются выполненными этой
документационной правкой. `P1-birth-runtime-evidence` / P1-AC30 остаётся
**deferred; not run** у named producer/verifier на full acceptance.

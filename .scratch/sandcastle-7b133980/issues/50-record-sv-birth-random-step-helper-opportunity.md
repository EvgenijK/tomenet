# 50: Записать SV-local random-step helper

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В `docs/sv-improvements.md` появляется одна самостоятельная запись о повторении одинакового xorshift state transition в owner-specific random command handlers. Запись предлагает будущий SV-local random-step helper с сохранением детерминированной production semantics, но не выполняет рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-33-3

Labels: enhancement, ready-for-agent

**Finding ID:** `D-668860756a9d6f66` (`birth.command-rng-duplication-unrecorded`), one coherent documentation repair; the proposed random-dispatch refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, random state or tests and must not invent a test-only interface; P2-AC05 is a preservation constraint checked by `controller-review`.

- [x] `docs/sv-improvements.md` contains exactly one standalone entry identifying the identical xorshift state transition duplicated across the seven owner-specific birth random handlers. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [x] The entry states Problem, Affected code, Proposal, Status and Required checks and proposes a narrowly scoped SV-local random-step helper that owns only the state transition, leaving owner-specific legal-list selection, compatibility, flags, confirmation and empty-list rejection with their current handlers. [Contract: P2-AC02, P2-AC03, P2-AC06, P2-AC07; checks: `controller-review`]
- [x] Status marks the helper as proposed future work only: this ticket records the opportunity without changing production SV, tests, legacy/shared code, protocol/gameplay semantics or the accepted SV-B-023 scope. [Contract: P2-AC01, P2-AC03, P2-AC04, P2-AC05, P2-AC08, P2-AC09; checks: `controller-review`]
- [x] Required future checks cover deterministic next-state vectors from fixed seeds, unchanged owner-specific modulo/list selection and legality, random behavior for all seven server-provided choice owners, empty/unavailable-step handling, both compile-time class/race orders and the existing production interaction/input-router regression matrix; they are explicitly recorded as required and not claimed passed by this documentation repair. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

## Comments

- 2026-10-08: Claimed by `codex/sandcastle-7b133980-wave-33-3` before
  inspecting the production handlers and adding the standalone improvement
  record.

## Answer

В `docs/sv-improvements.md` добавлена ровно одна самостоятельная запись
`Extract an SV-local birth random-state step`. Она фиксирует одинаковый
трёхоперационный xorshift transition в семи `SV_BIRTH_RANDOM` branches:
`sv_birth_command`, `class_command`, `race_command`, `trait_command`,
`body_command`, `stats_command` и `mode_command`.

Proposal оставлен отдельной будущей SV-local работой. Узкий helper должен владеть
только текущим unsigned state transition и сохранять момент каждого шага;
server-provided legal-list selection, modulo/sex-bit mapping, compatibility,
flags, confirmation, ordered-stat iteration и empty/unavailable rejection
остаются в owner-specific handlers. Production SV, tests, legacy/shared code,
protocol/gameplay semantics и accepted scope SV-B-023 не изменены.

Production seam для documentation-only repair неприменим, поэтому behavioral
TDD, production-path session и новые behavior tests не запускались; test-only
seam не создавался. Навык TDD применён как ограничение против искусственного
интерфейса. Выполнена сфокусированная структурная red/green проверка документа:

- до записи команда с `grep -Fxc` для точного заголовка и assertion количества
  `1` — **RED**, exit 1 (`heading_count=0`);
- после записи bash assertions — **GREEN**, exit 0: заголовок встречается ровно
  один раз; Status, Problem, Affected code, Proposal и Required checks встречаются
  по одному разу; перечислены семь production handlers, fixed-seed vectors, все
  семь owners, оба compile orders, production interaction/input-router matrix и
  явный статус future checks как not claimed passed;
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: код и поведение не менялись, а
orchestrator выполняет общие gates после всех tickets. Required checks записи
относятся к будущему helper refactor и этой задачей не заявляются выполненными.
`P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named
producer/verifier на full acceptance.

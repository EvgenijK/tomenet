# 46: Записать централизацию birth scenario descriptors

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В отдельном списке SV improvements появляется самостоятельная запись о разрозненном raw-string dispatch production-path birth scenarios. Запись предлагает typed scenario descriptor/table как единый источник peer setup, native SDL input script, endpoint options и expected outcome, но не выполняет этот рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-31-1

Labels: bug, ready-for-agent

**Finding ID:** `D-dd1626dd11c9dff2` (`sv-birth-tests.scenario-string-dispatch-sprawl-unrecorded`), one coherent documentation repair; the proposed scenario-harness refactor remains separate future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, production/test code or invent a test-only interface; P2-AC05 is a preservation constraint checked by `controller-review`.

- [x] `docs/sv-improvements.md` contains one standalone entry for the raw-string scenario identity repeatedly redispatched across peer setup, native SDL input scripting, endpoint options and assertions in the SV birth production-path scenario harness. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [x] The entry states the Problem, Affected code, Proposal, Status and Required checks, and proposes a typed scenario descriptor/table that centralizes setup, scripted input, options and expected outcome so one scenario cannot be configured under one identity and asserted under another. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [x] The status marks the refactor as separately proposed future work; this repair only records the opportunity and does not modify the harness, production SV, legacy/shared code, accepted behavior or SV-B-023 scope. [Contract: P2-AC03, P2-AC05, P2-AC08; checks: `controller-review`]
- [x] Focused structural review confirms the improvement record is present exactly once with all required fields and records required future checks for descriptor completeness, unique scenario identity, peer/input/options/assertion agreement and the existing production-path birth regression matrix, without claiming those deferred refactor checks passed. [Contract: P2-AC06, P2-AC07, P2-AC08; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

## Comments

- 2026-10-08: Claimed by `codex/sandcastle-7b133980-wave-31-1` before
  inspecting the production-path birth scenario harness and adding the standalone
  improvement record.

## Answer

В `docs/sv-improvements.md` добавлена ровно одна самостоятельная запись
`Centralize production-path birth scenario descriptors`. Она фиксирует, что
Python runner перечисляет raw-string scenario IDs, а C harness повторно
интерпретирует `argv[3]` / `Peer.mode` при подготовке peer setup, native SDL
input script, `SvEndpointOptions` и expected outcome/peer assertions.

Proposal оставлен отдельной будущей работой: typed scenario identity и descriptor
table должны стать единым источником всех четырёх частей сценария, отклонять
unknown/duplicate identities и не позволять настроить сценарий под одной
identity, а проверить под другой. Запись сохраняет production
`sv_endpoint_run` path, accepted protocol/gameplay behavior и текущую область
SV-B-023; harness, production SV, legacy/shared code и tests не изменялись.

Production seam для этого documentation-only repair неприменим, поэтому
behavioral TDD и production-path session не запускались и test-only seam не
создавался. Навык TDD применён как ограничение против искусственного тестового
интерфейса. Выполнена сфокусированная структурная проверка документации:

- до добавления `rg -n '^## Centralize production-path birth scenario descriptors$' docs/sv-improvements.md` — **RED**, exit 1: самостоятельная запись отсутствовала;
- после добавления bash structural assertions — **GREEN**, exit 0: заголовок
  присутствует ровно один раз, поля Status/Problem/Affected code/Proposal/Required
  checks присутствуют ровно по одному, proposal и deferred checks содержат
  descriptor completeness, unique identity, peer/input/options/assertion
  agreement и existing production-path birth regression matrix;
- будущие descriptor/refactor checks явно записаны как **not run / not passed**.

Полный build/test/review workflow не запускался: поведение и код не менялись, а
orchestrator выполняет общие gates после всех tickets. Deferred
`P1-birth-runtime-evidence` / P1-AC30 остаётся у named producer/verifier на full
acceptance и этой задачей не производится и не заявляется пройденным.

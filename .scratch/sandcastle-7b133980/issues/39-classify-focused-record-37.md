# 39: Классифицировать focused record 37 как Wayfinder history

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Resolved focused coverage record 37 однозначно классифицируется как non-owning Wayfinder planning/execution history канонического SV-B-023 и получает отдельную valid triage-state label, сохраняя enhancement category, resolved execution history и все verification claims без изменений.

**Blocked by:** 37: Verify focused production coverage (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-ce5b76002eb6c0a2` (`sv-task-tracker.noncanonical-scratch-implementation-tickets`), coherent repair of the child-37 recurrence.

- [ ] Record 37 содержит явную `Tracker role` классификацию Wayfinder planning/execution history со ссылкой на canonical SV-B-023 и декларацией, что child не владеет Stage B capability IDs; новый canonical implementation ticket не создаётся. [Contract: P2-AC10; checks: `controller-review`]
- [ ] Отдельная `Labels:` строка record 37 сохраняет category `enhancement` и добавляет ровно одну valid state label `ready-for-agent`, не подменяя Wayfinder `Status: resolved`. [Contract: P2-AC10, P2-AC11; checks: `controller-review`]
- [ ] Tracker-only repair сохраняет Type, Status, Assignee, Comments, Answer, production evidence и deferred-check claims record 37; production code, tests, accepted behavior, canonical SV-B-023 и immutable contract не меняются. [Contract: P1-AC30, P2-AC04, P2-AC05, P2-AC10, P2-AC11; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred; metadata repair не производит runtime evidence и не заявляет этот check выполненным.

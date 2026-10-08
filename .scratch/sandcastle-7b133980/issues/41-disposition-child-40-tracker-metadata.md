# 41: Получить human disposition для tracker metadata child 40

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** После явного human disposition resolved focused coverage record 40 либо получает разрешённое узкое metadata-исправление — явную классификацию как non-owning Wayfinder planning/execution history canonical SV-B-023 и одну valid triage-state label при сохранении `enhancement` — либо обязательный tracker defect остаётся открытым без четвёртого автоматического repair. Execution history, verification claims, accepted production behavior и canonical ownership не меняются.

**Blocked by:** 40: Verify focused production coverage (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-human

**Finding ID:** `D-ce5b76002eb6c0a2` (`sv-task-tracker.noncanonical-scratch-implementation-tickets`), human disposition for the child-40 recurrence after the three-repair ceiling.

**Production seam:** Not applicable. This is a tracker-only disposition and, if authorized, metadata repair; it must not change production SV behavior or invent a test-only interface. The existing accepted production boundary remains `input/birth-interaction.h`, and P1-AC28/P2-AC05 are preservation constraints checked by `controller-review`, not a reason to run a behavioral TDD loop.

- [ ] Human disposition is recorded before any edit to record 40; without explicit authorization this ticket remains `ready-for-human`, finding `D-ce5b76002eb6c0a2` remains open, and no fourth automated repair is attempted. [Contract: P2-AC10, P2-AC11; checks: `controller-review`]
- [ ] If correction is authorized, record 40 gains exactly one explicit `Tracker role` classification as non-owning Wayfinder planning/execution history linked to canonical SV-B-023; no duplicate implementation ticket or capability ownership is created. [Contract: P1-AC01, P2-AC10, P2-AC12; checks: `controller-review`]
- [ ] If correction is authorized, record 40 keeps category `enhancement` and gains exactly one valid state label while `Status: resolved` remains the Wayfinder execution status and is not replaced by a triage role. [Contract: P2-AC10, P2-AC11; checks: `controller-review`]
- [ ] Any authorized edit is limited to the missing tracker-role declaration and triage-state label: Type, Status, Assignee, Comments, Answer, production evidence, accepted behavior, canonical SV-B-023, immutable contract and deferred-check claims remain unchanged. [Contract: P1-AC23, P1-AC26, P1-AC29, P1-AC30, P2-AC04, P2-AC05, P2-AC10; checks: `controller-review`]
- [ ] Structural verification checks the two authorized metadata fields and proves that production, tests, legacy/shared code and canonical task allocation were not modified; it records the result without claiming `sv-build`, `sv-core` or native runtime evidence ran. [Contract: P1-AC28, P1-AC30, P2-AC01, P2-AC03, P2-AC05, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This human-disposition ticket neither performs nor claims that evidence.

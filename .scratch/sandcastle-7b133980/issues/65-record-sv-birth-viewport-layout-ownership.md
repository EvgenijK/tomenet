# 65: Зафиксировать UI-владение birth viewport и layout

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о том, что native birth input adapter владеет row geometry и `SvBirthViewport` presentation state, которые напрямую использует UI renderer. Запись предлагает будущий UI-owned viewport/layout и hit-test contract, потребляемый native input, но не выполняет рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

**Assignee:** Unassigned

Labels: enhancement, ready-for-agent

**Finding ID:** `D-d662d94595732b29` (`birth.viewport-layout-ownership-unrecorded`), one coherent documentation repair; the proposed ownership refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, production or test code, or invent a test-only interface. The accepted production interaction/input-router path remains unchanged and is guarded by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [ ] The SV improvement register contains exactly one standalone record describing the misplaced birth row geometry and viewport presentation ownership as a UI/input boundary problem. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [ ] The record states Status, Problem, Affected code, Proposal and Required checks, identifies both the native birth input adapter and endpoint birth renderer, and proposes a UI-owned viewport/layout and hit-test contract consumed by native input. [Contract: P2-AC06, P2-AC07, P2-AC12; check: `controller-review`]
- [ ] The proposal preserves stable server-provided identity, highlighted selection, keyboard/mouse navigation, resize handling and owner transitions, and marks the refactor as separate future SV-local work rather than changing production behavior or legacy/shared code in this repair. [Contract: P1-AC10, P1-AC11, P1-AC21, P1-AC28, P2-AC01, P2-AC03, P2-AC04, P2-AC08, P2-AC09; check: `controller-review`]
- [ ] Required future checks cover renderer/hit-test geometry agreement for every choice owner, clipped and visible-row mouse input, keyboard and wheel/Page navigation, focus/resize interleaving, both class/race compile orders, and the existing production interaction/input-router regression path; they are recorded as required and explicitly not claimed passed by this documentation repair. [Contract: P1-AC10, P1-AC11, P1-AC19, P1-AC21, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

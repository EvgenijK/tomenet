# 66: Зафиксировать birth-owned outcome snapshot boundary

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о том, что application-level endpoint outcome разворачивает почти всё внутреннее состояние birth interaction в плоскую диагностическую поверхность. Запись предлагает будущий birth-owned snapshot/evidence aggregate через устойчивую interaction boundary, но не выполняет рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

**Assignee:** Unassigned

Labels: enhancement, ready-for-agent

**Finding ID:** `D-901b12e65f193779` (`birth.endpoint-outcome-state-exposure-unrecorded`), one coherent documentation repair; the proposed outcome-boundary refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change `sv_endpoint_run(SvEndpointOptions)`, production or test behavior, or invent a test-only interface. Preservation of the accepted production interaction/input-router path is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [ ] The SV improvement register contains exactly one standalone record describing the flattened publication of detailed birth interaction state through the endpoint outcome as an orchestration/diagnostics coupling problem. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [ ] The record states Status, Problem, Affected code, Proposal and Required checks, identifies endpoint outcome publication and its public outcome type, and proposes a birth-owned snapshot/evidence aggregate exposed through the stable interaction boundary. [Contract: P2-AC06, P2-AC07, P2-AC12; check: `controller-review`]
- [ ] The proposal keeps gameplay detail behind its responsible interface, preserves the current public production endpoint seam and observable evidence semantics, and marks the refactor as separate future SV-local work rather than changing production behavior or legacy/shared code in this repair. [Contract: P1-AC02, P1-AC28, P2-AC01, P2-AC03, P2-AC04, P2-AC08, P2-AC09; check: `controller-review`]
- [ ] Required future checks cover complete field-to-snapshot evidence mapping, generation and lifecycle data, owner/highlight/draft state, Guide placeholder evidence, partial/completed-send suppression, both compile orders and existing production endpoint scenarios; they are recorded as required and explicitly not claimed passed by this documentation repair. [Contract: P1-AC02, P1-AC19, P1-AC21, P1-AC22, P1-AC24, P1-AC25, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

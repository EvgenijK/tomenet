# 67: Зафиксировать единый inventory production endpoint test sources

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о семи Python runners, которые дублируют production endpoint source inventory и могут разойтись при добавлении новой production dependency. Запись предлагает будущий authoritative test build inventory/helper при сохранении production-path execution и изоляции runners, но не выполняет консолидацию и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

**Assignee:** Unassigned

Labels: enhancement, ready-for-agent

**Finding ID:** `D-ba1f78c75abfd157` (`sv-tests.endpoint-source-inventory-duplication-unrecorded`), one coherent documentation repair; the proposed test-build consolidation remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change the runners, production sources or public `sv_endpoint_run(SvEndpointOptions)` path, and must not create a test-only behavior implementation. Preservation of production-path execution is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [ ] The SV improvement register contains exactly one standalone record describing the duplicated production endpoint source inventory across all seven current Python runners and the resulting dependency-drift risk. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [ ] The record states Status, Problem, Affected code, Proposal and Required checks, identifies the seven affected endpoint runners, and proposes one authoritative test build inventory/helper while retaining runner-specific scenario orchestration and isolation. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [ ] The proposal preserves compilation and execution of the real production endpoint, native birth adapter and birth interaction path, forbids moving behavior into a test-only implementation, and marks consolidation as separate future test-infrastructure work rather than changing production or legacy/shared code in this repair. [Contract: P1-AC28, P2-AC01, P2-AC03, P2-AC04, P2-AC05, P2-AC08, P2-AC09; check: `controller-review`]
- [ ] Required future checks cover authoritative inventory completeness and ordering, all seven runner consumers, runner-specific sources/flags, dependency additions and removals, both birth compile orders where applicable, and existing production endpoint regression runs; they are recorded as required and explicitly not claimed passed by this documentation repair. [Contract: P1-AC19, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

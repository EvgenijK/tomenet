# 50: Записать SV-local random-step helper

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В `docs/sv-improvements.md` появляется одна самостоятельная запись о повторении одинакового xorshift state transition в owner-specific random command handlers. Запись предлагает будущий SV-local random-step helper с сохранением детерминированной production semantics, но не выполняет рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding ID:** `D-668860756a9d6f66` (`birth.command-rng-duplication-unrecorded`), one coherent documentation repair; the proposed random-dispatch refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, random state or tests and must not invent a test-only interface; P2-AC05 is a preservation constraint checked by `controller-review`.

- [ ] `docs/sv-improvements.md` contains exactly one standalone entry identifying the identical xorshift state transition duplicated across the seven owner-specific birth random handlers. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] The entry states Problem, Affected code, Proposal, Status and Required checks and proposes a narrowly scoped SV-local random-step helper that owns only the state transition, leaving owner-specific legal-list selection, compatibility, flags, confirmation and empty-list rejection with their current handlers. [Contract: P2-AC02, P2-AC03, P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] Status marks the helper as proposed future work only: this ticket records the opportunity without changing production SV, tests, legacy/shared code, protocol/gameplay semantics or the accepted SV-B-023 scope. [Contract: P2-AC01, P2-AC03, P2-AC04, P2-AC05, P2-AC08, P2-AC09; checks: `controller-review`]
- [ ] Required future checks cover deterministic next-state vectors from fixed seeds, unchanged owner-specific modulo/list selection and legality, random behavior for all seven server-provided choice owners, empty/unavailable-step handling, both compile-time class/race orders and the existing production interaction/input-router regression matrix; they are explicitly recorded as required and not claimed passed by this documentation repair. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC19, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

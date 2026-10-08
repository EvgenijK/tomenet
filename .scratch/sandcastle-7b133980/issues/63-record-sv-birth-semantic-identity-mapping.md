# 63: Записать semantic identity mapping для SV birth

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о том, что birth compatibility, trait-derived hidden classes и post-mode PvP Maia rules распознают серверные domain identities через display strings и raw trait/feature literals. Запись предлагает будущую authoritative decoded identity/feature mapping или named semantic constants, сохраняя server-layout behavior, но не выполняет рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

**Assignee:** Unassigned

Labels: enhancement, ready-for-agent

**Finding ID:** `D-746578f2ab62cfa8` (`birth.semantic-identity-primitives-unrecorded`), one coherent documentation repair; the proposed identity-mapping refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, production or test code, or invent a test-only interface; preservation of the accepted production interaction/input-router path is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [ ] The SV improvement register contains exactly one standalone record identifying display-string recognition of Maia and trait-derived hidden classes together with raw post-mode trait and server-feature literals as one semantic-identity mapping problem. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] The record states Problem, Affected code, Proposal, Status and Required checks, and proposes either one authoritative decoded identity/feature mapping or named SV-local semantic constants for the affected race, class, trait and PvP Maia identities. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC07, P2-AC09; checks: `controller-review`]
- [ ] The proposal explicitly preserves current server-provided title, trait-ID and feature-flag layout behavior plus compatibility, hidden-class and post-mode semantics; Status marks it as separately proposed future work and this ticket performs no production refactor or legacy/shared extraction. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC19, P1-AC20, P2-AC01, P2-AC03, P2-AC04, P2-AC08, P2-AC09; checks: `controller-review`]
- [ ] Required future checks cover decoded identity/layout mapping, Maia race matching, both trait-derived hidden classes, post-mode trait identities, the PvP Maia feature flag, both compile-time class/race orders, and the existing production interaction/input-router regression matrix; they are recorded as required and explicitly not claimed passed by this documentation repair. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC19, P1-AC20, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

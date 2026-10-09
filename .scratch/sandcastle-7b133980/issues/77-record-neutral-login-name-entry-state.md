# 77: Зафиксировать нейтральное имя состояния ввода имени

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о том, что production state `ordinary_name_active` используется и для ordinary, и для exclusive/dedicated name entry, поэтому его имя больше не отражает область действия. Запись предлагает будущую нейтральную замену, например `name_entry_active`, но не выполняет переименование, не меняет production behavior и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding ID:** `D-80a802fa22706ca7` (`sv-login.name-entry-active-misnamed-ordinary`), one coherent documentation repair; the proposed production-state rename remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not rename the production field, change the public SV interaction/input-router path or create a test-only interface. Preservation of production behavior is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [ ] The SV improvement register contains exactly one standalone record explaining that the name-entry active flag now covers both ordinary and exclusive/dedicated creation, so its ordinary-only name is misleading. [Contract: P2-AC06, P2-AC07; check: `controller-review`]
- [ ] The record states Problem, Affected code, Proposal, Status and Required checks; it identifies the name-entry state declaration, both activation paths and its generic endpoint consumers, and proposes a neutral production name such as `name_entry_active`. [Contract: P2-AC06, P2-AC07, P2-AC12; check: `controller-review`]
- [ ] The status marks the rename as separately proposed future work: this repair changes only the improvement record and Wayfinder history, without renaming code, altering ordinary or exclusive/dedicated creation, touching legacy/shared gameplay code, or changing any verified SV-B-023 behavior. [Contract: P1-AC02, P1-AC20, P1-AC28, P2-AC01, P2-AC03, P2-AC04, P2-AC08, P2-AC09; check: `controller-review`]
- [ ] Required future checks cover a complete production reference migration, ordinary and exclusive/dedicated name-entry activation and exit, handoff into the existing birth interaction/input router, quit and prohibited-send behavior, `sv-build`, `sv-core` and `controller-review`; those future rename checks are recorded as required and are not claimed passed by this documentation repair. [Contract: P1-AC02, P1-AC12, P1-AC20, P1-AC22, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

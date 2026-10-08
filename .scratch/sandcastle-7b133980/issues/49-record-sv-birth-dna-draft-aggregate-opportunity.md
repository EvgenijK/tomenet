# 49: Записать SV-local DNA draft aggregate

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В `docs/sv-improvements.md` появляется одна самостоятельная запись о семи повторяющихся DNA availability/value pairs на границах endpoint и birth interaction. Запись предлагает будущий SV-local DNA draft aggregate, но не выполняет рефакторинг, не добавляет DNA persistence и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding ID:** `D-12bba6812f5215a0` (`birth.dna-draft-parameter-clump-unrecorded`), one coherent documentation repair; the proposed API refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, API code or tests and must not invent a test-only interface; P2-AC05 is a preservation constraint checked by `controller-review`.

- [ ] `docs/sv-improvements.md` contains exactly one standalone entry identifying the repeated DNA availability/value pairs carried by endpoint options, birth state, birth initialization and their production handoff. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] The entry states Problem, Affected code, Proposal, Status and Required checks and proposes a typed SV-local DNA draft aggregate with explicit availability for sex, class, race, trait, body, stats and mode while preserving the existing production boundary and value semantics. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] Status marks the aggregate as proposed future work only: this ticket does not perform the refactor, extract shared/legacy code, add DNA persistence, accept `birth.restore-dna`/`birth.save-dna`, or change any verified SV-B-023 behavior. [Contract: P1-AC26, P1-AC27, P2-AC01, P2-AC02, P2-AC03, P2-AC08, P2-AC09; checks: `controller-review`]
- [ ] Required future checks cover field-for-field endpoint-to-birth mapping, unavailable/value pairing, all seven choice owners, both compile-time class/race orders, production-path random/staged/automatic DNA regressions and unchanged protocol/send behavior; they are explicitly recorded as required and not claimed passed by this documentation repair. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC19, P1-AC26, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

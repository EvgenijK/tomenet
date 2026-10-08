# 46: Записать централизацию birth scenario descriptors

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В отдельном списке SV improvements появляется самостоятельная запись о разрозненном raw-string dispatch production-path birth scenarios. Запись предлагает typed scenario descriptor/table как единый источник peer setup, native SDL input script, endpoint options и expected outcome, но не выполняет этот рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-dd1626dd11c9dff2` (`sv-birth-tests.scenario-string-dispatch-sprawl-unrecorded`), one coherent documentation repair; the proposed scenario-harness refactor remains separate future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, production/test code or invent a test-only interface; P2-AC05 is a preservation constraint checked by `controller-review`.

- [ ] `docs/sv-improvements.md` contains one standalone entry for the raw-string scenario identity repeatedly redispatched across peer setup, native SDL input scripting, endpoint options and assertions in the SV birth production-path scenario harness. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] The entry states the Problem, Affected code, Proposal, Status and Required checks, and proposes a typed scenario descriptor/table that centralizes setup, scripted input, options and expected outcome so one scenario cannot be configured under one identity and asserted under another. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] The status marks the refactor as separately proposed future work; this repair only records the opportunity and does not modify the harness, production SV, legacy/shared code, accepted behavior or SV-B-023 scope. [Contract: P2-AC03, P2-AC05, P2-AC08; checks: `controller-review`]
- [ ] Focused structural review confirms the improvement record is present exactly once with all required fields and records required future checks for descriptor completeness, unique scenario identity, peer/input/options/assertion agreement and the existing production-path birth regression matrix, without claiming those deferred refactor checks passed. [Contract: P2-AC06, P2-AC07, P2-AC08; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

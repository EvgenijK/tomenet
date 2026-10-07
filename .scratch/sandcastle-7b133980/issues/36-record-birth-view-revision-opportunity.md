# 36: Записать revision gate для birth view

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В отдельном списке SV improvements появляется самостоятельная запись о том, что birth presentation view сейчас пересобирается и копирует большие choice arrays каждый frame без revision gate. Запись фиксирует проблему, затронутый код, предложение, статус и необходимые проверки, не реализуя оптимизацию и не расширяя SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-c87262912f2ceb4f` (`birth.view-revision-improvement-unrecorded`).

- [ ] `docs/sv-improvements.md` содержит отдельную запись о per-frame rebuild/copy birth view без revision-based invalidation; запись явно называет affected SV endpoint/view preparation code и связь с ADR-0004 guidance. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] Запись содержит все обязательные поля: Problem, Affected code, Proposal, Status и Required checks; proposal предлагает измеряемый revision gate/cache invalidation с учётом model revision и presentation inputs, не выдавая оптимизацию за уже принятую реализацию. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [ ] Ticket меняет только improvement record: production behavior, legacy/shared code и SV-B-023 tests не меняются, а optimization остаётся отдельно proposed work без claims о пройденных runtime/build checks. [Contract: P2-AC03, P2-AC08; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим documentation repair; full-acceptance evidence не производится и не заявляется.

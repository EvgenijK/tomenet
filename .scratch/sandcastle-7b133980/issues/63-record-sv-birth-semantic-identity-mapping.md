# 63: Записать semantic identity mapping для SV birth

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В реестре SV improvements появляется одна самостоятельная запись о том, что birth compatibility, trait-derived hidden classes и post-mode PvP Maia rules распознают серверные domain identities через display strings и raw trait/feature literals. Запись предлагает будущую authoritative decoded identity/feature mapping или named semantic constants, сохраняя server-layout behavior, но не выполняет рефакторинг и не расширяет SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** Codex (`codex/sandcastle-7b133980-wave-42-1`)

Labels: enhancement, ready-for-agent

**Finding ID:** `D-746578f2ab62cfa8` (`birth.semantic-identity-primitives-unrecorded`), one coherent documentation repair; the proposed identity-mapping refactor remains separately scoped future work.

**Production seam:** Not applicable. This documentation-only repair must not change public SV behavior, production or test code, or invent a test-only interface; preservation of the accepted production interaction/input-router path is checked by `controller-review`. [Contract: P1-AC28, P2-AC05]

- [x] The SV improvement register contains exactly one standalone record identifying display-string recognition of Maia and trait-derived hidden classes together with raw post-mode trait and server-feature literals as one semantic-identity mapping problem. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [x] The record states Problem, Affected code, Proposal, Status and Required checks, and proposes either one authoritative decoded identity/feature mapping or named SV-local semantic constants for the affected race, class, trait and PvP Maia identities. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC07, P2-AC09; checks: `controller-review`]
- [x] The proposal explicitly preserves current server-provided title, trait-ID and feature-flag layout behavior plus compatibility, hidden-class and post-mode semantics; Status marks it as separately proposed future work and this ticket performs no production refactor or legacy/shared extraction. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC19, P1-AC20, P2-AC01, P2-AC03, P2-AC04, P2-AC08, P2-AC09; checks: `controller-review`]
- [x] Required future checks cover decoded identity/layout mapping, Maia race matching, both trait-derived hidden classes, post-mode trait identities, the PvP Maia feature flag, both compile-time class/race orders, and the existing production interaction/input-router regression matrix; they are recorded as required and explicitly not claimed passed by this documentation repair. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC19, P1-AC20, P1-AC28, P2-AC04, P2-AC05, P2-AC07, P2-AC08; checks: `sv-build`, `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to its named producer and verifier at full acceptance. This documentation repair neither performs nor claims that evidence.

## Comments

- 2026-10-08: Claimed by `codex/sandcastle-7b133980-wave-42-1` before
  inspecting the decoded setup/feature path and adding the standalone improvement
  record.

## Answer

В `docs/sv-improvements.md` добавлена ровно одна самостоятельная запись
`Decode SV birth semantic identities authoritatively`. Она объединяет в одну
semantic-identity mapping проблему display-title распознавание race `Maia` и
trait-derived hidden classes `Hell Knight` / `Corrupted Priest`, raw post-mode
trait IDs `14` (`Enlightened`) и `15` (`Corrupted`) и PvP Maia feature bit
`server_flags[3] & 0x200`.

Proposal оставлен отдельной будущей SV-local работой: одна authoritative decoded
identity/feature mapping на существующей decoded boundary, либо, если wire data
не позволяет получить такую identity без домысла, единый набор named SV-local
semantic constants и mapping helpers. Запись требует сохранить server-provided
titles, race/class order и choice bits, hidden-class prefix/base decode, trait IDs
и feature-flag layout, а также compatibility, обе trait-derived hidden classes и
post-mode PvP Maia semantics. Production/test code, legacy/shared code и область
SV-B-023 не изменены.

Production seam для этого documentation-only repair неприменим, поэтому
behavioral TDD, production-path session и новые behavior tests не запускались;
test-only seam не создавался. Навык TDD применён как ограничение против
искусственного интерфейса. Выполнена сфокусированная структурная red/green
проверка документа:

- до записи точная проверка заголовка
  `## Decode SV birth semantic identities authoritatively` — **RED**, exit 1,
  `heading_count=0`;
- после записи shell assertions для единственного заголовка, полей Status,
  Problem, Affected code, Proposal и Required checks, всех race/class/trait/feature
  identities, preservation constraints и future-check matrix — **GREEN**, exit 0,
  `heading_count=1; structure_and_contract=present`;
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: поведение не менялось, а
orchestrator выполняет общие gates после всех tickets. Будущие `sv-build`,
`sv-core`, `controller-review` и production interaction/input-router regressions
записаны как обязательные для mapping refactor и не заявляются выполненными этой
документационной задачей. `P1-birth-runtime-evidence` / P1-AC30 остаётся
**deferred; not run** у named producer/verifier на full acceptance.

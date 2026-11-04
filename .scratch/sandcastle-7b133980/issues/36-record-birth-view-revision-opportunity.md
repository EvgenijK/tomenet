# 36: Записать revision gate для birth view

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** В отдельном списке SV improvements появляется самостоятельная запись о том, что birth presentation view сейчас пересобирается и копирует большие choice arrays каждый frame без revision gate. Запись фиксирует проблему, затронутый код, предложение, статус и необходимые проверки, не реализуя оптимизацию и не расширяя SV-B-023.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-23-3

Labels: bug, ready-for-agent

**Finding ID:** `D-c87262912f2ceb4f` (`birth.view-revision-improvement-unrecorded`).

- [x] `docs/sv-improvements.md` содержит отдельную запись о per-frame rebuild/copy birth view без revision-based invalidation; запись явно называет affected SV endpoint/view preparation code и связь с ADR-0004 guidance. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [x] Запись содержит все обязательные поля: Problem, Affected code, Proposal, Status и Required checks; proposal предлагает измеряемый revision gate/cache invalidation с учётом model revision и presentation inputs, не выдавая оптимизацию за уже принятую реализацию. [Contract: P2-AC06, P2-AC07; checks: `controller-review`]
- [x] Ticket меняет только improvement record: production behavior, legacy/shared code и SV-B-023 tests не меняются, а optimization остаётся отдельно proposed work без claims о пройденных runtime/build checks. [Contract: P2-AC03, P2-AC08; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим documentation repair; full-acceptance evidence не производится и не заявляется.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-23-3` before
  inspecting the production endpoint/view preparation path and ADR-0004.

## Answer

В `docs/sv-improvements.md` добавлена самостоятельная proposed-запись о
per-frame подготовке birth view. Она фиксирует, что production
`src/client/sv/endpoint-run.c:run_contact` каждый кадр вызывает
`src/client/sv/session/birth.c:sv_birth_view_prepare`, а функция заново
инициализирует `SvBirthView` и копирует большие legal class/race/trait arrays
без revision gate. В affected code также указаны `SvBirth` / `SvBirthView` из
`src/client/sv/session/birth.h`, а проблема явно связана с guidance ADR-0004.

Proposal оставлен отдельной будущей работой: измерить preparations/copied bytes,
добавить монотонную revision birth presentation model и инвалидировать
endpoint-owned derived cache по generation/model revision и всем реально
используемым presentation inputs (geometry, scale, font/resource revisions,
renderer lifetime). Измеримый критерий: после первой подготовки неизменившиеся
кадры не дают новых rebuild, каждое релевантное изменение даёт ровно один.
Оптимизация не реализована и не включена в SV-B-023.

Production seam и behavioral red → green loop для этого documentation-only
repair отсутствуют: public SV behavior, production code и tests не менялись.
Навык TDD не использовался для создания test-only seam. Вместо behavioral TDD
выполнена сфокусированная структурная проверка записи:

- до изменения `rg -q '^## Gate birth presentation view preparation by revisions$' docs/sv-improvements.md` — **RED**, exit 1:
  `standalone birth view revision-gate improvement is absent`;
- после изменения та же команда плюс `bash` structural assertions для полей
  Status/Problem/Affected code/Proposal/Required checks, affected symbols,
  ADR-0004, model/presentation invalidation, измеримого zero-rebuild критерия и
  отсутствия claims о runtime/build completion — **GREEN**, exit 0:
  `improvement record structure and required content are present`;
- `git diff --check` — **PASS**, exit 0;
- scope inspection — **PASS**: изменены только improvement record и Wayfinder
  history/index; `src/`, `tests/`, legacy/shared code и canonical SV-B-023 не
  менялись.

Полный build/test/review workflow, runtime checks и production-path sessions не
запускались, поскольку behavior не менялся, а orchestrator выполняет свои gates
после сборки tickets. `P1-birth-runtime-evidence` / P1-AC30 остаётся
**deferred; not run**, full-acceptance evidence не производится и не заявляется.

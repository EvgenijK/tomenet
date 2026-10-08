# 62: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-41-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-41-1` before
  coverage inspection. The contract-approved public production seam is
  `sv_endpoint_run(SvEndpointOptions)`: authoritative server setup and native
  SDL events pass through the production birth interaction/input router, while
  `SvEndpointOutcome` and the controlled peer expose stable owner/selection,
  committed draft and outbound-send suppression. This focused seam does not
  replace deferred native runtime evidence P1-AC30.
- 2026-10-08: Inspection of active records 60 and 61 found one focused gap in
  record 60's production-path cases. The implementation extended both PageDown
  and PageUp to body, stats and mode viewports, but each new case exercised only
  PageDown. Add separate reverse PageUp endpoint/native-input scenarios while
  retaining the existing late-identity PageDown scenarios. This is coverage of
  implemented behavior, so no red → green behavior-change loop is claimed.

## Answer

Проверены active records 60 и 61. Record 60 локально в SV синхронизировал
bounded renderer/hit-testing viewport для body, stats и mode и расширил
production PageUp/PageDown routing на этих owners. Record 61 изменил только
canonical tracker status и не требовал нового behavior coverage. Legacy/shared
production code не менялся.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative server setup; реальные SDL resize, focus, wheel,
PageDown, PageUp, clipped click, visible-row double-click и quit events проходят
native adapter и public production birth interaction/input router.
`SvEndpointOutcome` наблюдает stable highlighted identity и committed draft.
Peer допускает только name handoff и затем требует EOF, поэтому partial или
completed birth send провалил бы каждый scenario.

Coverage inspection обнаружил отсутствующую прямую проверку PageUp для новых
body/stats/mode viewport ветвей. Добавлены отдельные production endpoint cases
`body-viewport-page-up`, `stats-viewport-page-up` и
`mode-viewport-page-up` в default и `SV_CLASS_BEFORE_RACE=0` configurations.
Каждый case после уменьшения surface выполняет wheel round trip, PageDown и
PageUp, отклоняет clipped double-click, пропускает interleaved ping/focus/resize
и подтверждает первую отображённую server identity: Normal body, stat 0 или
Normal mode. Если PageUp игнорируется, подтверждается поздняя identity, и
literal assertion падает. Исходные `body-viewport`, `stats-viewport` и
`mode-viewport` сохранены и независимо подтверждают достижимость поздних
identities Fruit bat, stat 1 и No Ghost. Отдельной test-only реализации
rendering, hit testing или behavior нет.

Это coverage-only дополнение уже реализованного behavior. Первый
production-seam запуск новых чувствительных tests был green; behavior defect не
обнаружен, production implementation не менялась, поэтому red → green цикл не
заявляется.

Focused verification, 2026-10-08:

`for birth_case in body-viewport body-viewport-page-up stats-viewport
stats-viewport-page-up mode-viewport mode-viewport-page-up; do
SV_BIRTH_CASE="$birth_case" python3 -B
tests/sv_name_editor_submit_checks.py; done`

Результат: **PASS**, exit 0 для всех шести cases в обеих compile
configurations; runner собрал production seam с `-Wall -Wextra -Werror`, ASan и
UBSan. `git diff --check` — **PASS**. Повторившиеся declared-font fallback и SDL
`Leaked thread` diagnostics совпадают с известным focused-runner output;
compiler, sanitizer и assertion failures не было.

Полные `sv-build`, `sv-core`, controller review и общий workflow не запускались:
их выполняет orchestrator после assembly. Новых unrelated improvement
opportunities не обнаружено. `P1-birth-runtime-evidence` / P1-AC30 для accepted
contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.

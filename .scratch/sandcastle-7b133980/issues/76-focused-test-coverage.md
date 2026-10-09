# 76: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-53-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed on `codex/sandcastle-7b133980-wave-53-1` before
  coverage inspection. The accepted public production seam is
  `sv_endpoint_run(SvEndpointOptions)`: authoritative setup and loaded DNA enter
  the endpoint, real SDL events pass through the native birth adapter and the
  production interaction/input router, and `SvEndpointOutcome` plus EOF at the
  controlled peer observe state and prohibited sends. This focused seam does
  not replace deferred native runtime evidence P1-AC30.
- 2026-10-09: Active-record inspection found ticket 75 as the only production
  or test delta after focused record 74. Its endpoint scenarios cover explicit
  DNA at partial and completed ordered stats, inherited automatic DNA, point-buy
  controls, aggregate invalidation, quit, disconnect and stale-generation
  suppression. No missing focused branch or production defect was found, so no
  new test, behavior change or red -> green loop is claimed.

## Answer

Проверен active record 75. Существующие production endpoint scenarios проводят
настоящий SDL `#` через native birth adapter и production interaction/input
router при partial ordered stats и после завершения всех шести choices. Они
наблюдают неизменные owner/highlight/local и committed draft, доступность общего
loaded-DNA aggregate, отсутствие запуска automatic state и последующее legal
применение того же DNA на mode. Отдельный inherited-automatic scenario
подтверждает, что explicit `#` не останавливает уже принятую automatic
continuation. Point-buy, missing mode, поздний Backspace, quit, disconnect и
stale-generation controls сохраняют прежнюю семантику.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative setup и loaded DNA; реальные SDL keyboard/DNA events
проходят native adapter и production birth-interaction/input-router boundary.
`SvEndpointOutcome` наблюдает owner, stable highlight, local/committed stats,
automatic state, aggregate availability и successful mode DNA apply. Peer
допускает только name handoff и требует EOF, поэтому partial или completed birth
send провалил бы scenario. Отдельной test-only реализации поведения нет.

Focused verification, 2026-10-09:

```sh
for birth_case in \
  ordered-stats-explicit-dna-partial-no-op \
  ordered-stats-explicit-dna-no-op \
  auto-dna-ordered-keyboard \
  auto-dna-ordered-local-reject \
  auto-dna-ordered-local-partial-reject \
  auto-dna-ordered-mode-missing \
  auto-dna-ordered-shared-mode-backtrack \
  stats-point-dna stats-point-dna-missing stats-quit \
  session-disconnect auto-dna-stale-event
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
```

Результат: **PASS**, 12/12 selected cases, общий exit 0. Runner при каждом
запуске собрал default class-before-race и alternate race-before-class
production configurations с `-Wall -Wextra -Werror`, ASan и UBSan; каждый case
выполнился во всех применимых inventory configurations. Compiler, sanitizer и
assertion failures не было. Повторяющиеся declared-font fallback и SDL `Leaked
thread` diagnostics совпали с существующим focused-runner output и не
сопровождались failure. Логи сохранены в
`/tmp/sandcastle-7b133980-ticket-76/`. `git diff --check` — **PASS**.

Это coverage-only ticket уже реализованного поведения. Первый focused
production-seam запуск был green; дефект не обнаружен, поэтому red -> green цикл
и новые tests не заявляются. Полные `sv-build`, unfiltered `sv-core`, controller
review и общий build/test/review workflow не запускались: их выполняет
orchestrator после assembly. Legacy/shared и production behavior files не
изменялись; новых improvement opportunities не обнаружено.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.

# 78: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-55-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-09: Claimed before coverage inspection. The accepted public production
  seam is `sv_endpoint_run(SvEndpointOptions)`: authoritative server setup enters
  the endpoint, real SDL events pass through the native adapters and production
  interaction/input router, and `SvEndpointOutcome` plus the controlled peer
  observe handoff, birth state and prohibited sends. This focused seam does not
  replace deferred native runtime evidence P1-AC30.
- 2026-10-09: Active-record inspection found documentation-only ticket 77 as the
  sole delta after focused record 76. It records a future neutral rename for the
  shared ordinary and exclusive/dedicated name-entry state without changing that
  state or production behavior. Existing ordinary-ready and exclusive-IDDC
  endpoint scenarios cover both affected production handoffs. No focused gap or
  production defect was found, so no new test, behavior change or red -> green
  loop is claimed.

## Answer

Проверен active record 77 и его preservation boundary. Изменение добавляет
только отдельную запись в `docs/sv-improvements.md` о будущем переименовании
`ordinary_name_active`; production и tests оно не меняет. Существующие scenarios
`mode-ordinary-ready` и `exclusive-iddc-production` проводят соответственно
ordinary и exclusive/dedicated name entry через настоящий production handoff в
birth interaction, завершают legal mode choice через native SDL events и
наблюдают ready draft и quit без completed-character send.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative contact/login/birth setup; реальные SDL login, name и
birth events проходят native adapters и production interaction/input router.
`SvEndpointOutcome` наблюдает accepted-name handoff, legal modes, owner,
highlight, navigation, confirmed ready draft и quit. Peer допускает только
предусмотренный name handoff и требует EOF, поэтому partial или completed birth
send провалил бы scenario. Отдельной test-only реализации поведения нет.

Focused verification, 2026-10-09:

```sh
for birth_case in mode-ordinary-ready exclusive-iddc-production
do
  SV_BIRTH_CASE="$birth_case" \
    PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin \
    python3 -B tests/sv_name_editor_submit_checks.py
done
```

Результат: **PASS**, 2/2 selected cases, общий exit 0. Runner собрал и выполнил
каждый case в default class-before-race и alternate race-before-class production
configurations с `-Wall -Wextra -Werror`, ASan и UBSan. Compiler, sanitizer и
assertion failures не было. Повторяющиеся declared-font fallback и SDL `Leaked
thread` diagnostics совпали с существующим focused-runner output и не
сопровождались failure. Логи сохранены в
`/tmp/sandcastle-7b133980-ticket-78/`.

Это coverage-only ticket уже реализованного поведения. Первый focused
production-seam запуск был green; дефект и missing coverage не обнаружены,
поэтому red -> green цикл, новые tests и production changes не заявляются.
Полные `sv-build`, unfiltered `sv-core`, controller review и общий
build/test/review workflow не запускались: их выполняет orchestrator после
assembly. Legacy/shared и production behavior files не изменялись; новых
improvement opportunities не обнаружено. `P1-birth-runtime-evidence` / P1-AC30
для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.

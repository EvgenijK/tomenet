# 13: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-11-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Answer

Проверены все tickets 01–12 effort: они имеют `Status: resolved`, а их
production coverage собрано в `tests/sv_name_editor_submit_checks.py`. Новых
изменений production behavior после boundary review ticket 12 нет. Сопоставление
матрицы сценариев с принятым контрактом не выявило отсутствующей focused coverage,
поэтому новые tests и production changes не потребовались.

Согласованный public production seam: `sv_endpoint_run(SvEndpointOptions)`
получает настоящий decoded server setup, создаёт session-owned `SvBirth`, а
keyboard/mouse события проходят через production SDL adapter
`sv_native_birth_event()` и единый `sv_birth_command()`. Controlled peer задаёт
server-provided lists и flags, наблюдает owner/highlight/draft через
`SvEndpointOutcome` и после единственного допустимого name packet требует EOF;
любой partial или completed-character outbound byte приводит к падению проверки.
Отдельного test-only choice engine или fixed-response wizard нет.

Это focused verification уже реализованного поведения. TDD/red → green claim не
делается: проверка прошла с первого запуска и дефекта поведения не обнаружила.
Существующая suite независимо проверяет ожидаемые stable identities и transitions
для sex/class/race/trait/body/stats/mode в обеих compile configurations, включая
legal/illegal/random/DNA input, keyboard/mouse/confirm, Q/Ctrl-Q, весь Backspace
owner graph, unavailable trait/body, dedicated IDDC/PvP и post-mode PvP Maia,
network/focus/resize interleaving, disconnect/stale-generation suppression и все
reachable Guide placeholder entries с неизменным draft.

Focused result, 2026-10-07:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0; default
  и `SV_CLASS_BEFORE_RACE=0` production binaries под ASan/UBSan.
- `git diff --check` — **PASS**, exit 0.

Повторяющийся diagnostic `Leaked thread` уже отдельно записан в
`docs/sv-improvements.md`; ожидаемый test-profile font fallback использовал
declared library font. Они не расширяют этот ticket. Полный `sv-core`, полный
build/test/review workflow и controller review не запускались: эти assembled
gates выполняет orchestrator.

`P1-birth-runtime-evidence` остаётся **deferred; not run** по исходной причине:
completion scope контракта — implementation, а native runtime evidence собирается
на full acceptance. Producer — SV-B-023 implementation assignee; verifier —
Sandcastle controller full-acceptance verifier. Никакой runtime artifact не
объявляется выполненным или принятым; он должен быть произведён и проверен при
точном accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и точном
HEAD full-acceptance проверки.

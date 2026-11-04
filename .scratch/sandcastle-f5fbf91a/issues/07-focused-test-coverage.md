# 7: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Answer

Проверены resolved tickets 01–06 и изменения после предыдущего focused coverage
ticket 05. Согласованный public production seam не менялся: native account flow
проходит через `sv_endpoint_run(SvEndpointOptions)`, versioned protocol — через
production `sv_contact_*` / `sv_login_*`, authoritative state — через
`sv_pregame_sync_login` / `sv_pregame_fail` / `sv_pregame_disconnect`, а
credential lifecycle — через `sv_vault_store` / `sv_vault_poll` /
`sv_vault_cancel`. После ticket 05 production SV sources и перечисленные
feature suites не менялись; ticket 06 исправлял/проверял только core registry и
сохранил эти production-path suites.

Существующее покрытие уже проверяет required focused slices, поэтому новых
тестов и production изменений не потребовалось. Это проверка уже реализованного
поведения, а не behavior change; red → green или test-first claim не делается.
Suites компилируют и вызывают production SV sources. Provider doubles остаются
только на внешней OS-provider boundary и не реализуют account policy, protocol,
private-field semantics или pregame state вместо production кода.

Focused results 2026-10-06, все команды завершились с exit 0:

- `python3 -B tests/sv_account_create_checks.py` — native private account/password
  input, loopback production contact/login, fragmented authoritative overview,
  exact credential-save timing/bytes, save degradation, pending cancellation и
  stale retry completion.
- `python3 -B tests/sv_account_failure_checks.py` — invalid/used name, bad
  password, server/account-flag rejection, malformed/partial delivery,
  disconnect, fresh-generation retry и оба cancel parent без раннего overview
  или credential save.
- `python3 -B tests/sv_contact_checks.py` — exact protocol 1/2 contact/verify
  bytes, atomic output и fragmented/versioned production decoder paths.
- `python3 -B tests/sv_login_checks.py` — exact old/modern login bytes, atomic
  output, authoritative fragmented overview и terminal/stale-generation state.
- `python3 -B tests/sv_endpoint_checks.py` — native endpoint/private-field and
  SDL scene production behavior remains intact.
- `python3 -B tests/sv_first_session_checks.py` — existing production
  success/disconnect/quit/retry session flow remains intact.
- `python3 -B tests/sv_vault_checks.py` — production identity/provider boundary,
  unavailable provider and approved isolated provider path remain intact.

Полный `sv-build` / `sv-core` / review workflow не запускался: итоговый
headless gate после assembly остаётся за orchestrator. External review checks
не обозначены выполненными. Explicit acceptance deferrals сохранены без
изменений и не запускались:

| Check | Status | Причина | Owner |
|---|---|---|---|
| `P1-controlled-peer-runtime` | **deferred; not run** | `Requires a controlled-peer runtime environment; no allowed executable controlled-peer runner was supplied for this checkpoint.` | SV-B-021 controlled-peer verification owner |
| `P1-linux-native-runtime` | **deferred; not run** | `Requires the Linux native input and credential-provider environment.` | SV-B-021 Linux native-path verification owner |
| `P1-windows-native-runtime` | **deferred; not run** | `Requires the Windows native input and credential-provider environment.` | SV-B-021 Windows native-path verification owner |
| `P1-entry-complete-integration` | **deferred; not run** | `Downstream integration evidence belongs to SV-B-025 after SV-B-021 acceptance.` | SV-B-025 owner |

Во время endpoint/account/first-session suites снова наблюдался SDL resolver
`Leaked thread`; он уже записан отдельно в `docs/sv-improvements.md` и здесь не
заявлен исправленным. Legacy/shared code, production code и improvement list не
изменялись.

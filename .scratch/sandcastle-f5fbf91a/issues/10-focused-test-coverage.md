# 10: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Answer

Проверены resolved tickets 01–09 и изменения на исходном HEAD
`0f7edcf795`. Согласованный public production seam не менялся: native account
flow проходит через `sv_endpoint_run(SvEndpointOptions)`, versioned protocol —
через production `sv_contact_*` / `sv_login_*`, authoritative state — через
`sv_pregame_sync_login` / `sv_pregame_fail` / `sv_pregame_disconnect`, а
credential lifecycle — через `sv_vault_store` / `sv_vault_poll` /
`sv_vault_cancel`. После последней assembled verification production SV sources
и feature suites не менялись; последующие изменения только запланировали этот
focused coverage ticket.

Существующее покрытие уже проверяет обязательные focused slices, поэтому новых
тестов и production изменений не потребовалось. Это проверка уже реализованного
behavior, а не behavior change; red → green или test-first claim не делается.
Suites компилируют и вызывают production SV sources. Provider doubles остаются
только на внешней OS-provider boundary и не реализуют account policy, protocol,
private-field semantics или pregame state вместо production кода.

Focused result 2026-10-06:

- `python3 -B tests/sv_account_create_checks.py && python3 -B tests/sv_account_failure_checks.py`
  — **pass**, exit 0. Первая suite покрыла native private account/password
  input, loopback production contact/login, fragmented authoritative overview,
  exact credential-save timing/identity/secret, saved/session-only outcomes,
  pending cancellation и stale retry completion. Вторая покрыла invalid/used
  account name, bad password, server/account-flag rejection, cancellation,
  malformed/error и partial disconnect, fresh-generation retry и отсутствие
  overview/credential save до server confirmation.

Тем самым current controlled-peer production-path obligation проверена
управляемыми TCP peers этих suites для success, rejection, cancellation, error
handling и fragmented protocol input. Полный `sv-build` / `sv-core` / review
workflow не запускался: итоговый headless gate после assembly остаётся за
orchestrator. Новые тесты, test-only behavior, production, legacy/shared и
improvement-list изменения не вносились.

Явные acceptance deferrals сохранены и не запускались:

| Check | Status | Причина | Owner |
|---|---|---|---|
| `P1-linux-native` | **deferred; not run** | Requires Linux native input and credential-provider runtime evidence that is unavailable at the implementation gate. | `SV-B-021` |
| `P1-windows-native` | **deferred; not run** | Requires Windows native input and credential-provider runtime evidence that is unavailable at the implementation gate. | `SV-B-021` |
| `P1-entry-integration` | **deferred; not run** | This is downstream integration evidence owned by the later SV-B-025 entry-completion task. | `SV-B-025` |

Во время focused suites снова наблюдался SDL resolver `Leaked thread`; он уже
записан отдельно в `docs/sv-improvements.md` и здесь не заявлен исправленным.

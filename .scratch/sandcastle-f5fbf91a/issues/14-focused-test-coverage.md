# 14: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-f5fbf91a-wave-11-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Answer

Проверены active/resolved tickets 01–13 и assembled HEAD `6b7555a12c`.
Согласованный public production seam сохранён: полный native account flow входит
через `sv_endpoint_run(SvEndpointOptions)`, version-aware wire path использует
production `sv_contact_*` / `sv_login_*`, authoritative state проходит через
`sv_pregame_sync_login` / `sv_pregame_fail` / `sv_pregame_disconnect`, а
credential lifecycle — через `sv_vault_store` / `sv_vault_poll` /
`sv_vault_cancel`. Managed TCP peers находятся на внешней network boundary;
suites компилируют production SV sources и не реализуют отдельный test-only
account flow.

Focused verification 2026-10-06:

- `python3 -B tests/sv_account_create_checks.py && python3 -B tests/sv_account_failure_checks.py`
  — **pass**, exit 0. Проверены native private account/password input, exact
  contact/verify/login requests, fragmented server-authoritative overview,
  credential save только после confirmation, saved и explicit session-only
  outcomes, stale-generation cancellation, invalid/used account, bad password,
  account/server rejection, cancel parents, malformed input, partial disconnect
  и fresh-generation retry без раннего overview.

Существующее focused покрытие полностью соответствует current `P1-account-create`,
включая controlled-peer success, rejection, cancellation, error handling и
fragmented protocol input. Новых тестов или production изменений не потребовалось.
Это coverage verification уже реализованного behavior, поэтому red → green и
test-first claim не делаются. Полные `sv-build`, `sv-core` и review workflow не
запускались: post-assembly gates остаются за orchestrator.

Явные acceptance deferrals сохранены без claims о выполнении:

| Check | Status | Причина | Owner |
|---|---|---|---|
| `P1-linux-native` | **deferred; not run** | Requires Linux native input and credential-provider runtime evidence that is unavailable at the implementation gate. | `SV-B-021` |
| `P1-windows-native` | **deferred; not run** | Requires Windows native input and credential-provider runtime evidence that is unavailable at the implementation gate. | `SV-B-021` |
| `P1-entry-integration` | **deferred; not run** | This is downstream integration evidence owned by the later SV-B-025 entry-completion task. | `SV-B-025` |

Во время suites повторилось известное диагностическое сообщение SDL resolver
`Leaked thread`; оно уже записано в `docs/sv-improvements.md`, поэтому scope
этого focused ticket не расширялся. Legacy/shared production code не менялся.

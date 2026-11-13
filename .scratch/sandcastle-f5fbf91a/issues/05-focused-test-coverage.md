# 5: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Answer

Проверены active tickets 01–04 и их production evidence. Согласованный public
SV seam не менялся: полный native account flow проходит через
`sv_endpoint_run(SvEndpointOptions)`; exact versioned bytes — через production
`sv_contact_create` / `sv_contact_take_output` и `sv_login_create` /
`sv_login_take_output`; server-authoritative состояние — через
`sv_pregame_sync_login`, `sv_pregame_fail`, `sv_pregame_disconnect`; credential
policy — через production caller `sv_vault_store` / `sv_vault_poll` /
`sv_vault_cancel`. Provider doubles в success suite остаются только на внешней
OS-provider boundary и не реализуют account policy.

Покрытие уже содержало требуемые focused slices, поэтому новые тесты и
production изменения не понадобились. Это проверка уже реализованного
поведения, а не новый red → green цикл; test-first claim не делается. Tests
вызывают production SV sources и не содержат отдельной реализации serializers,
private-field semantics, pregame state или credential policy.

Focused results 2026-10-06:

- `python3 -B tests/sv_account_create_checks.py` — pass, exit 0: native private
  account/password input, loopback production contact/login, fragmented final
  overview byte, authoritative flags, exact save timing/identity/secret,
  saved/session-only outcomes, pending cancellation и stale retry completion.
- `python3 -B tests/sv_account_failure_checks.py` — pass, exit 0: invalid name,
  bad password, server rejection, used-account login rejection, malformed
  packet, partial disconnect, fresh-generation retry и оба native cancel
  parents без overview или credential save.
- `python3 -B tests/sv_contact_checks.py` — pass, exit 0: exact protocol 1/2
  contact and verify bytes, atomic undersized output, fragmented decoder and
  versioned setup paths.
- `python3 -B tests/sv_login_checks.py` — pass, exit 0: exact old/modern initial
  login bytes, atomic undersized output, fragmented authoritative overview and
  terminal/stale-generation pregame behavior.
- `python3 -B tests/sv_endpoint_checks.py` — pass, exit 0: existing native
  endpoint/private-input production behavior remains intact.
- `python3 -B tests/sv_first_session_checks.py` — pass, exit 0: existing
  production success/disconnect/quit/retry session flow remains intact.
- `python3 -B tests/sv_vault_checks.py` — pass, exit 0: production vault
  identity/provider boundary remains intact.

Полный build/core/review workflow не запускался: он остаётся за orchestrator.
External review checks не обозначены как выполненные. `P1-controlled-peer-runtime`,
`P1-linux-native-runtime`, `P1-windows-native-runtime` и
`P1-entry-complete-integration` остаются **deferred; not run** с исходными
причинами и владельцами из acceptance contract. Наблюдавшийся SDL resolver
`Leaked thread` diagnostic уже записан отдельно в `docs/sv-improvements.md`;
legacy/shared код и список улучшений в этом ticket не изменялись.

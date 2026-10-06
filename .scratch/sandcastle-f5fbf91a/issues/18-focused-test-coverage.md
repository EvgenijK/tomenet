# 18: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-f5fbf91a-wave-13-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Answer

Проверены assembled changes resolved tickets 16 и 17. Согласованные public
production seams взяты из immutable contract и самих tickets: полный
`sv_endpoint_run(SvEndpointOptions)` с managed loopback peers для account-create
и contact rejection, а также public `sv_pregame_fail` /
`sv_pregame_disconnect` через production login suite. Suites компилируют и
вызывают production SV sources; отдельной реализации mapping, terminal state,
account flow или password semantics только для тестов нет.

Focused verification 2026-10-06:

- `python3 -B tests/sv_login_checks.py` — **pass**, exit 0. Проверены production
  login serializer/parser и public pregame terminal transitions: invalid/stale
  generation, distinct failed/disconnected destinations, terminal guard,
  reason/revision preservation и отсутствие ранней authentication/overview.
- `python3 -B tests/sv_account_create_checks.py` — **pass**, exit 0. Managed
  controlled peers проверили подтверждённое сервером создание, fragmented
  protocol input, cancellation/error handling, credential save только после
  authentication, session-only degradation и fresh-generation retry.
- `python3 -B tests/sv_account_failure_checks.py` — **pass**, exit 0. Managed
  controlled peers проверили exact presentation status и persisted
  server-shaped reason для invalid-name, bad-password и game-full rejection, а
  также login rejection, malformed input, disconnect, retry и cancellation без
  преждевременного overview.
- `git diff --check` — **pass**, exit 0.

Coverage audit не обнаружил отсутствующего focused case: changes tickets 16 и
17 уже имеют assertions на обоих изменённых production seams и полный
controlled-peer account flow. Новые тесты и behavior changes не добавлялись;
для coverage-only verification не заявляется test-first/red-green loop.
Наблюдаемые успешными suites fallback-font и SDL `Leaked thread` diagnostics
уже учтены в `docs/sv-improvements.md` и не расширяют этот ticket.

По указанию handoff полный `sv-build` / `sv-core` gate не запускался: его после
assembly выполняет orchestrator. Явные acceptance deferrals сохранены без
claims о выполнении:

| Check | Status | Owner |
|---|---|---|
| `P1-linux-native` | **deferred; not run** | `SV-B-021` |
| `P1-windows-native` | **deferred; not run** | `SV-B-021` |
| `P1-entry-integration` | **deferred; not run** | `SV-B-025` |

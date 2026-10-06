# 12: Зафиксировать current controlled-peer acceptance

Type: repair
Status: resolved
Assignee: codex
Labels: bug, ready-for-agent
Finding IDs: D-345b4357ca451a4d

**What to build:** Implementation record SV-B-021 корректно отражает
immutable acceptance: controlled-peer production-path coverage является
current обязательством P1-10 и подтверждено прошедшими loopback suites, а не
отложенным runtime checkpoint. Pending остаются только Linux native, Windows
native и downstream SV-B-025 acceptance.

**Blocked by:** 10: Verify focused production coverage.

**Production seam preserved for verification:** ticket меняет только evidence.
P1-10 подтверждается существующими suites через публичный
`sv_endpoint_run(SvEndpointOptions)`, production `sv_contact_*` / `sv_login_*`,
authoritative pregame seams и production vault lifecycle; этот accepted seam не
заменяется новым test-only интерфейсом.

- [x] Статус и implementation evidence называют P1-10 current и passed на
  основании обеих прошедших controlled-peer production-path suites, включая
  success, rejection, cancellation, error handling и fragmented input.
  [Contract: P1-3, P1-6, P1-7, P1-10, P2-AC6; check: `P1-account-create`]
- [x] Из record удалены invented controlled-peer deferral/checkpoint, причина и
  owner; managed loopback peers больше не описываются как недостаточные для
  обязательства, которое они фактически покрывают. [Contract: P1-10;
  check: `P1-account-create`]
- [x] Deferred section перечисляет только неизменные Linux native, Windows
  native и SV-B-025 integration checks с их исходными причинами и owners и не
  утверждает, что они выполнялись. [Contract: P1-11, P1-12, P1-13; checks:
  `P1-linux-native`, `P1-windows-native`, `P1-entry-integration`]
- [x] Repair не меняет production behavior, immutable acceptance contract или
  scope Stage C; evidence остаётся согласованным с сохранёнными account seams и
  passing focused results ticket 10. [Contract: P1-1, P1-15, P1-16, P2-AC5,
  P2-AC12; checks: `P1-account-create`, `sv-core`]

## Answer

Implementation record `docs/tasks/stage-b/SV-B-021-account-manage.md` исправлен
без изменений production, legacy/shared кода, immutable acceptance contract или
Stage C scope. Статус и evidence теперь называют controlled-peer production-path
coverage current обязательством P1-10 со статусом **passed** на основании обеих
loopback suites. Удалены invented deferral/checkpoint, его причина и owner;
deferred section содержит только `P1-linux-native`, `P1-windows-native` и
`P1-entry-integration` с исходными причинами и owners и явно помечает их
`deferred; not run`.

Согласованный public production seam: `sv_endpoint_run(SvEndpointOptions)` с
production `sv_contact_*` / `sv_login_*`, authoritative
`sv_pregame_sync_login` / `sv_pregame_fail` / `sv_pregame_disconnect` и
production `sv_vault_store` / `sv_vault_poll` / `sv_vault_cancel`. Suites
компилируют и вызывают production SV sources; отдельный test-only account flow
не добавлен.

Это evidence-only repair уже реализованного поведения, поэтому TDD red → green
не применялся: failing test и implementation command отсутствуют, test-first
claim не делается. Focused green verification 2026-10-06:

- `python3 -B tests/sv_account_create_checks.py` — **pass**, exit 0: success,
  cancellation, credential error handling и fragmented authoritative input.
- `python3 -B tests/sv_account_failure_checks.py` — **pass**, exit 0: rejection,
  cancellation, malformed/error, partial disconnect и fresh-generation retry.
- `git diff --check` и focused evidence assertions — **pass**, exit 0; deferred
  rows ровно три, старый controlled-peer checkpoint отсутствует, current P1-10
  и production seam записаны.

Полный build/core/review workflow не запускался: по условию задачи эти gates
после assembly выполняет orchestrator. Наблюдавшийся suites SDL resolver
`Leaked thread` уже записан в `docs/sv-improvements.md`; scope repair не
расширялся.

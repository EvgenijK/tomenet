# 12: Зафиксировать current controlled-peer acceptance

Type: repair
Status: open
Assignee: unassigned
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

- [ ] Статус и implementation evidence называют P1-10 current и passed на
  основании обеих прошедших controlled-peer production-path suites, включая
  success, rejection, cancellation, error handling и fragmented input.
  [Contract: P1-3, P1-6, P1-7, P1-10, P2-AC6; check: `P1-account-create`]
- [ ] Из record удалены invented controlled-peer deferral/checkpoint, причина и
  owner; managed loopback peers больше не описываются как недостаточные для
  обязательства, которое они фактически покрывают. [Contract: P1-10;
  check: `P1-account-create`]
- [ ] Deferred section перечисляет только неизменные Linux native, Windows
  native и SV-B-025 integration checks с их исходными причинами и owners и не
  утверждает, что они выполнялись. [Contract: P1-11, P1-12, P1-13; checks:
  `P1-linux-native`, `P1-windows-native`, `P1-entry-integration`]
- [ ] Repair не меняет production behavior, immutable acceptance contract или
  scope Stage C; evidence остаётся согласованным с сохранёнными account seams и
  passing focused results ticket 10. [Contract: P1-1, P1-15, P1-16, P2-AC5,
  P2-AC12; checks: `P1-account-create`, `sv-core`]

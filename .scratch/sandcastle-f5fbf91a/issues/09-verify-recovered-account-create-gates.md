# 09: Проверить восстановленные account-create gates на assembled HEAD

**What to build:** На HEAD после assembly принятый account-creation check,
полная SV сборка и полный core gate проходят вместе. Evidence связывает точный
assembled HEAD и новый accepted contract/state digest с результатами команд,
не выдавая external native или downstream проверки за выполненные.

**Blocked by:** 08: Согласовать account-create acceptance check с production
suites.

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Production seam preserved for verification:** ticket не меняет behavior;
`P1-account-create` подтверждает production path через публичный
`sv_endpoint_run(SvEndpointOptions)`, production protocol seams
`sv_contact_*` / `sv_login_*`, authoritative pregame seams и production vault
lifecycle. Это accepted seam для P1-1–P1-10; отсутствие любого из этих
production paths является blocker, а не основанием для test-only интерфейса.

- [ ] На assembled HEAD выполняется именно команда `P1-account-create` из
  принятого contract; обе production suites проходят и вместе сохраняют
  success, rejection, cancellation, error, fragmentation, credential-policy и
  correct-parent/no-early-overview obligations. [Contract: P1-1, P1-2, P1-3,
  P1-4, P1-5, P1-6, P1-7, P1-8, P1-9, P1-10, P1-15; check:
  `P1-account-create`]
- [ ] На том же assembled HEAD без изменения или исключения обязательных
  targets проходит полный `sv-build`. [Contract: P1-1, P1-14, P1-15, P2-AC1,
  P2-AC2, P2-AC5, P2-AC11; check: `sv-build`]
- [ ] На том же assembled HEAD проходит полный `sv-core`, включая обе реальные
  account-creation suites; gate не сужен и не заменён focused-only результатом.
  [Contract: P1-1, P1-14, P1-15, P2-AC5, P2-AC6, P2-AC7, P2-AC8, P2-AC9,
  P2-AC10, P2-AC12, P2-AC13, P2-AC14; check: `sv-core`]
- [ ] Recovery evidence фиксирует assembled HEAD, accepted contract/state
  digest, точные команды и exit status всех трёх gates; worker/assembly ancestry
  сохраняет repair ticket 08 и все ранее completed worker commits. [Contract:
  P2-AC6, P2-AC12; checks: `P1-account-create`, `sv-build`, `sv-core`]
- [ ] Linux native input/provider, Windows native input/provider и SV-B-025
  integration остаются **deferred; not run** у существующих owners; controlled
  production-suite evidence не расширяет их acceptance. [Contract: P1-11,
  P1-12, P1-13; checks: `P1-linux-native`, `P1-windows-native`,
  `P1-entry-integration`]
- [ ] Password change и account information остаются в Stage C, а verification
  не вносит production, legacy/shared или unrelated improvement changes.
  [Contract: P1-16, P2-AC1, P2-AC2, P2-AC4, P2-AC7, P2-AC11; checks:
  `sv-build`, `sv-core`]

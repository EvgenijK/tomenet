# 11: Вернуть change set SV-B-021 в account scope

Type: repair
Status: open
Assignee: unassigned
Labels: bug, ready-for-agent
Finding IDs: D-801cbe32143a46db

**What to build:** Итоговый change set SV-B-021 содержит только production
account-creation flow в SV, его focused production-path tests, task evidence и
минимальную регистрацию обязательных checks. Добавленная этим effort общая
Sandcastle-инфраструктура для contract amendment, command repair, runner
validation, schemas, workflow и документации удалена из feature scope; её
возможное самостоятельное развитие не включается в этот repair без отдельной
авторизации.

**Blocked by:** 10: Verify focused production coverage.

**Production seam preserved for verification:** ticket не меняет account
behavior. После scope cleanup `P1-account-create` по-прежнему проверяет
публичный `sv_endpoint_run(SvEndpointOptions)`, production `sv_contact_*` /
`sv_login_*`, authoritative `sv_pregame_sync_login` / `sv_pregame_fail` /
`sv_pregame_disconnect` и credential lifecycle `sv_vault_store` /
`sv_vault_poll` / `sv_vault_cancel`. Это принятые seams для P1-1–P1-10 и
P2-AC6; отдельный test-only seam не допускается.

- [ ] Diff от planning base больше не содержит созданных в рамках SV-B-021
  generic contract-amendment, command-repair, runner-validation, schema,
  workflow, documentation или framework-test additions; сохранены только
  минимальные account implementation, focused tests, evidence и check
  registration, необходимые immutable contract. [Contract: P2-AC1, P2-AC2,
  P2-AC4, P2-AC11; checks: `sv-build`, `sv-core`]
- [ ] Scope cleanup не удаляет и не ослабляет обе production-path account
  suites: accepted `P1-account-create` проходит и сохраняет success,
  rejection, cancellation, error, fragmentation и credential-policy coverage
  через production SV seams. [Contract: P1-1, P1-3, P1-4, P1-6, P1-7, P1-8,
  P1-9, P1-10, P2-AC5, P2-AC6; check: `P1-account-create`]
- [ ] Полные `sv-build` и `sv-core` проходят после удаления unrelated
  framework scope, а production SV, legacy/shared behavior и зависимости
  SV-B-002/SV-B-005/SV-B-006 не расширены сверх уже принятой реализации.
  [Contract: P1-14, P1-15, P2-AC1, P2-AC2, P2-AC5; checks: `sv-build`,
  `sv-core`]
- [ ] Linux native, Windows native и SV-B-025 integration остаются явно
  **deferred; not run**; cleanup не заявляет эти проверки пройденными и не
  включает password change или account information из Stage C. [Contract:
  P1-11, P1-12, P1-13, P1-16; checks: `P1-linux-native`,
  `P1-windows-native`, `P1-entry-integration`]

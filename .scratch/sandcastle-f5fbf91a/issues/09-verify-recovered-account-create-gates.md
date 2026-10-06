# 09: Проверить восстановленные account-create gates на assembled HEAD

**Type:** implementation

**What to build:** На HEAD после assembly принятый account-creation check,
полная SV сборка и полный core gate проходят вместе. Evidence связывает точный
assembled HEAD и новый accepted contract/state digest с результатами команд,
не выдавая external native или downstream проверки за выполненные.

**Blocked by:** 08: Согласовать account-create acceptance check с production
suites.

**Status:** resolved

**Assignee:** codex/sandcastle-f5fbf91a-wave-8-1

Labels: bug, ready-for-agent

**Production seam preserved for verification:** ticket не меняет behavior;
`P1-account-create` подтверждает production path через публичный
`sv_endpoint_run(SvEndpointOptions)`, production protocol seams
`sv_contact_*` / `sv_login_*`, authoritative pregame seams и production vault
lifecycle. Это accepted seam для P1-1–P1-10; отсутствие любого из этих
production paths является blocker, а не основанием для test-only интерфейса.

- [x] На assembled HEAD выполняется именно команда `P1-account-create` из
  принятого contract; обе production suites проходят и вместе сохраняют
  success, rejection, cancellation, error, fragmentation, credential-policy и
  correct-parent/no-early-overview obligations. [Contract: P1-1, P1-2, P1-3,
  P1-4, P1-5, P1-6, P1-7, P1-8, P1-9, P1-10, P1-15; check:
  `P1-account-create`]
- [x] На том же assembled HEAD без изменения или исключения обязательных
  targets проходит полный `sv-build`. [Contract: P1-1, P1-14, P1-15, P2-AC1,
  P2-AC2, P2-AC5, P2-AC11; check: `sv-build`]
- [x] На том же assembled HEAD проходит полный `sv-core`, включая обе реальные
  account-creation suites; gate не сужен и не заменён focused-only результатом.
  [Contract: P1-1, P1-14, P1-15, P2-AC5, P2-AC6, P2-AC7, P2-AC8, P2-AC9,
  P2-AC10, P2-AC12, P2-AC13, P2-AC14; check: `sv-core`]
- [x] Recovery evidence фиксирует assembled HEAD, accepted contract/state
  digest, точные команды и exit status всех трёх gates; worker/assembly ancestry
  сохраняет repair ticket 08 и все ранее completed worker commits. [Contract:
  P2-AC6, P2-AC12; checks: `P1-account-create`, `sv-build`, `sv-core`]
- [x] Linux native input/provider, Windows native input/provider и SV-B-025
  integration остаются **deferred; not run** у существующих owners; controlled
  production-suite evidence не расширяет их acceptance. [Contract: P1-11,
  P1-12, P1-13; checks: `P1-linux-native`, `P1-windows-native`,
  `P1-entry-integration`]
- [x] Password change и account information остаются в Stage C, а verification
  не вносит production, legacy/shared или unrelated improvement changes.
  [Contract: P1-16, P2-AC1, P2-AC2, P2-AC4, P2-AC7, P2-AC11; checks:
  `sv-build`, `sv-core`]

## Answer

Verification выполнена на assembled HEAD
`5b6194fa698364f44dffd6b82b886add1f6c50e7` (`Sandcastle: independently
verify command-only contract repair`). Tracked acceptance artifact содержит
accepted contract/state digest
`8c107ecd47aeac23a221871fcb80ad5c9f87846ba44b10a1d97a00a0d2bd82c0`;
пересчёт production-функцией `digest()` из `.sandcastle/acceptance.mjs` дал тот
же digest. Artifact также связывает repair с прежним digest
`e706e7e1a96fefbddcf8bd3f0d9ac43f5e9e5d5bc2e9516c66f7ac9bc2b32223`.

Dependency/ancestry проверены: commit ticket 08 `5aa1a0c1d4` и worker commits
01–07 (`8d4cf3d6b6`, `c7c9fd7ff6`, `3c0266a67c`, `d824ba90bc`,
`7a52a88cbc`, `2a24be5408`, `59951697ce`) являются ancestors assembled HEAD;
resolved tickets и их `## Answer` сохранены.

Gate evidence на этом HEAD:

- `P1-account-create`, первая accepted command
  `python3 -B tests/sv_account_create_checks.py` — **pass**, exit 0.
- `P1-account-create`, вторая accepted command
  `python3 -B tests/sv_account_failure_checks.py` — **pass**, exit 0. Обе
  обязательные production suites вместе сохранили success, rejection,
  cancellation, error/retry, fragmentation, credential-policy и
  correct-parent/no-early-overview coverage через публичный
  `sv_endpoint_run(SvEndpointOptions)`, production `sv_contact_*` /
  `sv_login_*`, authoritative pregame seams и production vault lifecycle.
- `sv-build`: `make -s -C src -f makefile.sv tomenet-sv` — **pass**, exit 0;
  обязательные targets не менялись и не исключались.
- `sv-core`: accepted command `bash -s -- core`, с controller stdin
  `.sandcastle/checks.sh`, воспроизведён как
  `env PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin bash -s -- core < .sandcastle/checks.sh`
  — **pass**, exit 0. Полный gate не сужался; он выполнил обе реальные
  account-creation suites.

Перед успешным `sv-core` был диагностический запуск с системным
`/usr/bin/python3`: он дошёл до `tests/sv_runtime_checks.py` и завершился exit 1
из-за отсутствующего environment dependency `jsonschema`. Этот результат не
выдаётся за green evidence; штатный Sandcastle runner environment
`/opt/sv-venv`, уже используемый данным gate, содержит `jsonschema 4.26.0`, и
полный неизменённый gate затем прошёл.

TDD: production behavior не менялся, поэтому это focused verification уже
реализованного behavior, без нового test-first цикла и без test-only
интерфейса. Новые тесты и production/legacy/shared изменения не вносились.
Наблюдаемый SDL resolver `Leaked thread` остаётся ранее записанным improvement;
новых improvement opportunities в scope ticket не обнаружено.

Deferred acceptance не расширялась и не запускалась:

| Check | Status | Owner |
|---|---|---|
| `P1-linux-native` | **deferred; not run** | `SV-B-021` |
| `P1-windows-native` | **deferred; not run** | `SV-B-021` |
| `P1-entry-integration` | **deferred; not run** | `SV-B-025` |

Password change и account information остаются в Stage C.

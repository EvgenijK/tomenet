# 16: Канонизировать причины contact rejection

Type: repair
Status: resolved
Assignee: codex/sandcastle-f5fbf91a-wave-12-2
Labels: bug, ready-for-agent
Finding IDs: D-72dcc7c122d0a059

**What to build:** При server contact rejection сохранённая terminal reason и
показанный игроку status получают один и тот же server-shaped базовый текст из
единственного production mapping. Presentation добавляет только прежнюю
retry/exit guidance; rejected account не открывает overview и остаётся у
правильного parent.

**Blocked by:** 14: Verify focused production coverage.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` с
managed controlled peer и production contact/pregame path. Это принятый seam
для rejection и parent-state поведения по P1-6, P1-7 и P2-AC6; отдельный
test-only mapper или endpoint не допускается.

- [x] Один private production mapping определяет базовую reason для каждого
  поддерживаемого contact rejection code и unknown fallback; persisted pregame
  reason использует её без UI guidance, а presentation добавляет только
  прежнюю retry/exit guidance. Existing player-visible и persisted тексты не
  меняются. [Contract: P1-6, P1-7, P2-AC5; check: `P1-account-create`]
- [x] Production-path rejection coverage через `sv_endpoint_run` подтверждает
  server-shaped invalid-name, bad-password и account/server-flag reasons,
  correct parent и отсутствие overview до server confirmation; обе стороны
  mapping не могут разойтись из-за двух switch tables. [Contract: P1-3, P1-6,
  P1-7, P2-AC6; check: `P1-account-create`]
- [x] Repair остаётся внутри SV, не меняет public endpoint, contact или
  pregame contracts и не затрагивает legacy/shared production sources либо
  Stage C password/account-information scope. [Contract: P1-16, P2-AC1,
  P2-AC2, P2-AC4, P2-AC11; checks: `sv-build`, `sv-core`]
- [x] Focused account suites, `sv-build` и `sv-core` проходят; Linux native,
  Windows native и SV-B-025 integration сохраняются как **deferred; not run**
  у прежних owners. [Contract: P1-11, P1-12, P1-13, P1-14, P1-15;
  checks: `P1-account-create`, `sv-build`, `sv-core`, `P1-linux-native`,
  `P1-windows-native`, `P1-entry-integration`]

## Answer

Repair выполнен внутри SV без изменения public contracts. Согласованный public
production seam: `sv_endpoint_run(SvEndpointOptions)` с managed loopback peer;
контакт проходит через production `sv_contact_socket_*`, terminal state — через
`sv_pregame_fail`, а тот же `input->contact_status` поступает в endpoint
presentation. Отдельный mapper или account flow для тестов не добавлялся.

В `src/client/sv/endpoint-run.c` оставлен один private
`contact_rejection_reason()`, содержащий server-shaped базовый текст всех
поддерживаемых кодов `E_VERSION_OLD`, `E_VERSION_UNKNOWN`, `E_GAME_FULL`,
`E_TWO_PLAYERS`, `E_PASSWORD`, `E_IN_USE_DUP`, `E_LETTER`, `E_IN_USE`,
`E_SOCKET`, `E_INVAL`, `E_INVITE`, `E_BANNED`, `E_LENGTH`, `E_IN_USE_PC`,
`E_CLOSED` и unknown fallback. `sv_pregame_fail` сохраняет этот текст без UI
guidance; `contact_status()` получает ту же reason и добавляет только прежнее
`Escape exits.`. Player-visible и persisted literals не изменены, а две
независимые switch-таблицы устранены.

Focused coverage расширено на presentation-side literals для invalid-name,
bad-password и game-full/server rejection. В каждом из этих controlled-peer
запусков C assertions через `SvEndpointOutcome` по-прежнему подтверждают
`SV_PREGAME_FAILED`, правильную базовую reason, отсутствие authentication,
overview/server flags/creation flags и credential save; Python assertions
проверяют точный status с прежним exit guidance из того же production запуска.

TDD evidence 2026-10-06:

- **Seam:** публичный `sv_endpoint_run(SvEndpointOptions)` с managed controlled
  peer и production contact/pregame/presentation path.
- **Red (test sensitivity):** после добавления seam-level status assertion была
  сделана только временная односторонняя mutation invalid-name presentation в
  существующей дублированной таблице. `python3 -B
  tests/sv_account_failure_checks.py` — **fail**, exit 1: ожидался
  `invalid account name`, diagnostic/presentation получил `invalid identity`,
  тогда как persisted assertion оставался server-shaped. Mutation заменена
  production repair и не входит в итоговый diff.
- **Green:** `python3 -B tests/sv_account_failure_checks.py` — **pass**, exit 0
  после перехода обеих сторон на единственный production mapping.

Итоговая focused verification:

- `python3 -B tests/sv_account_create_checks.py && python3 -B
  tests/sv_account_failure_checks.py` — **pass**, exit 0.
- `make -s -C src -f makefile.sv tomenet-sv` — **pass**, exit 0; build key
  `.sv-build/linux/8b3130721e8ad8146e3e/build.txt`. Выведены только известные
  common/legacy warnings о declaration `atol()`; shared sources не менялись.
- `PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin bash
  .sandcastle/checks.sh core` — **pass**, exit 0. Первый запуск с системным
  Python дошёл до runtime checks и обнаружил отсутствующий `jsonschema`; повтор
  выполнен в документированном для SV-B-021 окружении `/opt/sv-venv` с
  `jsonschema` 4.26.0 и прошёл весь core gate.
- `git diff --check` — **pass**, exit 0.

Legacy/shared production sources, public endpoint/contact/pregame contracts,
password-change и account-information Stage C scope не затронуты. Новых
отдельных improvement opportunities не обнаружено; известные suite diagnostics
`Leaked thread` уже записаны в `docs/sv-improvements.md`.

Явные acceptance deferrals сохранены без claims о выполнении:

| Check | Status | Owner |
|---|---|---|
| `P1-linux-native` | **deferred; not run** | `SV-B-021` |
| `P1-windows-native` | **deferred; not run** | `SV-B-021` |
| `P1-entry-integration` | **deferred; not run** | `SV-B-025` |

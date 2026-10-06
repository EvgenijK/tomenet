# 01: Создать неиспользованный аккаунт через production flow

**Type:** implementation

**What to build:** Игрок вводит новое имя аккаунта и пароль в существующих native private fields; тот же contact/login conversation передаёт точные baseline bytes серверу, а overview появляется только после полного подтверждающего ответа сервера. Успешный путь включает запуск сохранения credentials только после подтверждения authentication.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-f5fbf91a-wave-1-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** полный пользовательский путь вызывается через публичный `sv_endpoint_run(SvEndpointOptions)`. Exact-byte проверки вызывают production serializers `sv_contact_create`/`sv_contact_take_output` и `sv_login_create`/`sv_login_take_output`; публикация overview проверяется через production state seam `sv_pregame_sync_login`. Эти seams прямо относятся к contract criteria P1-3, P1-4, P1-6 и production-path checks `P1-production-path-review`, `P1-protocol-review`, `P1-state-review`.

- [x] Перед изменением поведения подтверждено, что используются существующие production outcomes contact, credential vault и login из SV-B-002/SV-B-005/SV-B-006; отсутствие необходимого seam останавливает ticket как blocker, а не расширяет его до реализации зависимости. [Contract: P1-1; checks: `sv-build`, `sv-core`, `P1-implementation-review`]
- [x] Валидные неиспользованные account/password bytes проходят через общие accepted-input/private-field semantics без второго редактора или test-only поведения; интерактивные границы, live trimming, private masking и star rejection остаются baseline-compatible. [Contract: P1-3, P1-7, P2-3, P2-4; checks: `sv-build`, `sv-core`, `P1-production-path-review`, `P2-behavior-review`, `P2-production-seam-review`]
- [x] Production serializers выдают точные contact, verify и первоначальные login bytes для всех поддержанных ветвей server protocol/version, включая отсутствие partial output при ошибке или недостаточной ёмкости. [Contract: P1-5, P1-6, P2-3; checks: `sv-build`, `sv-core`, `P1-protocol-review`, `P2-behavior-review`]
- [x] Отправка запроса не подтверждает создание локально: incomplete/fragmented server flags и character-list rows не публикуют overview, а полный server response атомарно публикует authoritative flags, validation result и overview. [Contract: P1-4, P1-5; checks: `sv-build`, `sv-core`, `P1-state-review`, `P1-protocol-review`]
- [x] Сохранение exact credential bytes начинается только после server-confirmed authentication; успешный provider result отражается без включения secret в presentation state, logs или diagnostics. [Contract: P1-9; checks: `sv-build`, `sv-core`, `P1-credential-policy-review`]
- [x] Реализация остаётся по возможности внутри SV, не переносит password change или account-information management из Stage C и не извлекает shared code лишь ради устранения допустимого изолирующего дублирования. [Contract: P1-2, P2-1, P2-2, P2-7, P2-8; checks: `sv-core`, `P1-implementation-review`, `P2-isolation-review`, `P2-policy-precedence-review`]

## Answer

Подтверждены и использованы согласованные production seams: полный native путь
`sv_endpoint_run(SvEndpointOptions)`, serializers `sv_contact_create` /
`sv_contact_take_output` и `sv_login_create` / `sv_login_take_output`, а также
authoritative state seam `sv_pregame_sync_login`. До изменения прочитаны корневой
`CONTEXT.md`, `docs/sv-architecture.md`, ADR-0001–0006 и правила local Markdown.
Зависимые contact, vault и login outcomes уже присутствовали; новых зависимостей
или Stage C account-management behavior не добавлено.

Реализация сохраняет server-provided creation/server flags и authentication в
SV pregame state только после полного character-list terminator. Credential store
теперь привязан к этому опубликованному server-confirmed состоянию. Публичный
endpoint outcome возвращает authoritative flags/count и не содержащие secret
флаги save-started/save-succeeded. Managed peer вводит account/password через
существующий native private-field interaction, проверяет exact contact/verify/login
bytes, удерживает последний byte overview и подтверждает отсутствие раннего save.
Vault double находится только на границе OS provider и возвращает outcome
production caller; account policy остаётся в `endpoint-run.c`.

TDD seams и red → green:

- `sv_pregame_sync_login`: `python3 -B tests/sv_login_checks.py` — red, compile
  failed из-за отсутствующих `SvPregame.authenticated`/`creation_flags`; после
  минимальной atomic-publication реализации — green.
- `sv_endpoint_run`: `python3 -B tests/sv_account_create_checks.py` — red, compile
  failed из-за отсутствующего authoritative/save outcome; после публикации
  non-secret outcome и привязки save к confirmed pregame state — green.

Итоговые focused checks:

- `python3 -B tests/sv_account_create_checks.py` — passed (ASan/UBSan, native
  input, controlled TCP peer, fragmented response, exact save timing/bytes).
- `python3 -B tests/sv_contact_checks.py` — passed (protocol 1/2 exact contact
  and verify bytes; undersized output remains unconsumed).
- `python3 -B tests/sv_login_checks.py` — passed (old/modern initial login bytes,
  undersized output, atomic authoritative empty overview).

Полный build/test/review workflow не запускался: согласно handoff его выполняет
orchestrator. Возможностей улучшения legacy/shared/SV вне текущего scope не
обнаружено; `docs/sv-improvements.md` не изменялся.

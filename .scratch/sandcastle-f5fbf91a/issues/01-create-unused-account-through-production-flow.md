# 01: Создать неиспользованный аккаунт через production flow

**What to build:** Игрок вводит новое имя аккаунта и пароль в существующих native private fields; тот же contact/login conversation передаёт точные baseline bytes серверу, а overview появляется только после полного подтверждающего ответа сервера. Успешный путь включает запуск сохранения credentials только после подтверждения authentication.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** полный пользовательский путь вызывается через публичный `sv_endpoint_run(SvEndpointOptions)`. Exact-byte проверки вызывают production serializers `sv_contact_create`/`sv_contact_take_output` и `sv_login_create`/`sv_login_take_output`; публикация overview проверяется через production state seam `sv_pregame_sync_login`. Эти seams прямо относятся к contract criteria P1-3, P1-4, P1-6 и production-path checks `P1-production-path-review`, `P1-protocol-review`, `P1-state-review`.

- [ ] Перед изменением поведения подтверждено, что используются существующие production outcomes contact, credential vault и login из SV-B-002/SV-B-005/SV-B-006; отсутствие необходимого seam останавливает ticket как blocker, а не расширяет его до реализации зависимости. [Contract: P1-1; checks: `sv-build`, `sv-core`, `P1-implementation-review`]
- [ ] Валидные неиспользованные account/password bytes проходят через общие accepted-input/private-field semantics без второго редактора или test-only поведения; интерактивные границы, live trimming, private masking и star rejection остаются baseline-compatible. [Contract: P1-3, P1-7, P2-3, P2-4; checks: `sv-build`, `sv-core`, `P1-production-path-review`, `P2-behavior-review`, `P2-production-seam-review`]
- [ ] Production serializers выдают точные contact, verify и первоначальные login bytes для всех поддержанных ветвей server protocol/version, включая отсутствие partial output при ошибке или недостаточной ёмкости. [Contract: P1-5, P1-6, P2-3; checks: `sv-build`, `sv-core`, `P1-protocol-review`, `P2-behavior-review`]
- [ ] Отправка запроса не подтверждает создание локально: incomplete/fragmented server flags и character-list rows не публикуют overview, а полный server response атомарно публикует authoritative flags, validation result и overview. [Contract: P1-4, P1-5; checks: `sv-build`, `sv-core`, `P1-state-review`, `P1-protocol-review`]
- [ ] Сохранение exact credential bytes начинается только после server-confirmed authentication; успешный provider result отражается без включения secret в presentation state, logs или diagnostics. [Contract: P1-9; checks: `sv-build`, `sv-core`, `P1-credential-policy-review`]
- [ ] Реализация остаётся по возможности внутри SV, не переносит password change или account-information management из Stage C и не извлекает shared code лишь ради устранения допустимого изолирующего дублирования. [Contract: P1-2, P2-1, P2-2, P2-7, P2-8; checks: `sv-core`, `P1-implementation-review`, `P2-isolation-review`, `P2-policy-precedence-review`]

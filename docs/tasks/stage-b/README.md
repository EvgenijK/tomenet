# Этап B: последовательные задачи

Статус: specified, 2026-09-23. Это полный план текущих508 B outcomes, не реализация
и не runtime acceptance. [Спецификация](../../sv-stage-b-spec.md) задаёт общие
условия; [coverage.json](coverage.json) связывает каждый ID с ровно одним primary
implementation/acceptance owner, сохраняя все prerequisites/obligations/sources.

Номера — один допустимый topological порядок готовности production implementation. Полная acceptance остаётся pending до всех obligations и явно назначенных поздних integration checks; readiness не означает accepted claim. Задачи с независимыми dependencies
можно выполнять параллельно. Infrastructure создаётся внутри первого concrete
consumer; это не очередь отдельно принимаемых «движков» без пользовательского пути.
Повторное использование caller/prerequisite/scenario не создаёт второго владельца.

## Порядок

| Ticket | Пользовательский результат | Собственных IDs | Зависимости |
|---|---|---:|---|
| [SV-B-001](SV-B-001-endpoint.md) | Выбор сервера и редактирование адреса | 8 |  |
| [SV-B-002](SV-B-002-contact.md) | Контакт, negotiation и обслуживание соединения | 13 | [SV-B-001](SV-B-001-endpoint.md) |
| [SV-B-003](SV-B-003-profile.md) | Первый запуск с собственным CFG и ресурсами | 8 |  |
| [SV-B-004](SV-B-004-save.md) | Preview, Save, Cancel и конфликт настроек | 10 | [SV-B-003](SV-B-003-profile.md) |
| [SV-B-005](SV-B-005-vault.md) | Приватный ввод и системное хранилище для входа | 8 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-003](SV-B-003-profile.md) |
| [SV-B-006](SV-B-006-login.md) | Вход и выбор существующего персонажа | 14 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-002](SV-B-002-contact.md), [SV-B-005](SV-B-005-vault.md) |
| [SV-B-007](SV-B-007-macros.md) | Загрузка профиля клавиш и исполнение макросов | 11 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-003](SV-B-003-profile.md) |
| [SV-B-008](SV-B-008-guide.md) | Guide и ранний контекст при startup | 10 | [SV-B-003](SV-B-003-profile.md) |
| [SV-B-009](SV-B-009-inventory.md) | Входящие предметы, INS и read-only списки | 8 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-003](SV-B-003-profile.md), [SV-B-007](SV-B-007-macros.md) |
| [SV-B-010](SV-B-010-history-profile.md) | Истории и полное применение session profile | 3 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-003](SV-B-003-profile.md), [SV-B-007](SV-B-007-macros.md), [SV-B-008](SV-B-008-guide.md), [SV-B-009](SV-B-009-inventory.md) |
| [SV-B-011](SV-B-011-transfer.md) | Startup FILE и Lua reload | 7 | [SV-B-002](SV-B-002-contact.md), [SV-B-003](SV-B-003-profile.md), [SV-B-008](SV-B-008-guide.md) |
| [SV-B-012](SV-B-012-audio-start.md) | Звук, музыка и предупреждения в живой сессии | 10 | [SV-B-002](SV-B-002-contact.md), [SV-B-003](SV-B-003-profile.md), [SV-B-010](SV-B-010-history-profile.md) |
| [SV-B-013](SV-B-013-fonts-load.md) | Рабочие шрифты и отказ ресурсов | 7 | [SV-B-003](SV-B-003-profile.md) |
| [SV-B-014](SV-B-014-map-core.md) | Карта, палитра и согласованный viewport | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-006](SV-B-006-login.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-013](SV-B-013-fonts-load.md) |
| [SV-B-015](SV-B-015-hud-core.md) | Живые HP, ресурсы и прогресс персонажа | 9 | [SV-B-002](SV-B-002-contact.md) |
| [SV-B-016](SV-B-016-hud-conditions.md) | Состояния, голод и видимые ограничения | 10 | [SV-B-015](SV-B-015-hud-core.md) |
| [SV-B-017](SV-B-017-hud-detail.md) | Личность, цели и дополнительные HUD данные | 10 | [SV-B-002](SV-B-002-contact.md), [SV-B-015](SV-B-015-hud-core.md) |
| [SV-B-018](SV-B-018-messages-live.md) | Живая лента сообщений с точными occurrences | 1 | [SV-B-002](SV-B-002-contact.md) |
| [SV-B-019](SV-B-019-first-import.md) | First-launch data-only импорт | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-005](SV-B-005-vault.md), [SV-B-013](SV-B-013-fonts-load.md) |
| [SV-B-020](SV-B-020-first-session.md) | M1: первый сквозной вход существующим персонажем | 0 | [SV-B-003](SV-B-003-profile.md), [SV-B-006](SV-B-006-login.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-016](SV-B-016-hud-conditions.md), [SV-B-017](SV-B-017-hud-detail.md), [SV-B-018](SV-B-018-messages-live.md), [SV-B-019](SV-B-019-first-import.md) |
| [SV-B-021](SV-B-021-account-manage.md) | Создание аккаунта и смена пароля | 4 | [SV-B-002](SV-B-002-contact.md), [SV-B-005](SV-B-005-vault.md), [SV-B-006](SV-B-006-login.md), [SV-B-020](SV-B-020-first-session.md) |
| [SV-B-022](SV-B-022-character-manage.md) | Создание slot и порядок персонажей | 10 | [SV-B-006](SV-B-006-login.md), [SV-B-020](SV-B-020-first-session.md) |
| [SV-B-023](SV-B-023-birth-choices.md) | Рождение: выборы и backtracking | 14 | [SV-B-008](SV-B-008-guide.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-022](SV-B-022-character-manage.md) |
| [SV-B-024](SV-B-024-birth-dna.md) | Birth DNA и завершение создания | 4 | [SV-B-003](SV-B-003-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-023](SV-B-023-birth-choices.md) |
| [SV-B-025](SV-B-025-entry-complete.md) | Полная приёмка входа в игру | 1 | [SV-B-002](SV-B-002-contact.md), [SV-B-006](SV-B-006-login.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-015](SV-B-015-hud-core.md), [SV-B-020](SV-B-020-first-session.md), [SV-B-024](SV-B-024-birth-dna.md) |
| [SV-B-026](SV-B-026-target.md) | Направление и выбор цели | 8 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-007](SV-B-007-macros.md), [SV-B-025](SV-B-025-entry-complete.md) |
| [SV-B-027](SV-B-027-pickup-store.md) | Pickup и посещение read-only магазина | 7 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-007](SV-B-007-macros.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-025](SV-B-025-entry-complete.md) |
| [SV-B-028](SV-B-028-movement.md) | Движение, повтор и простые действия | 19 | [SV-B-025](SV-B-025-entry-complete.md), [SV-B-026](SV-B-026-target.md), [SV-B-027](SV-B-027-pickup-store.md) |
| [SV-B-029](SV-B-029-map-explore.md) | Обзор карты и locate | 7 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-025](SV-B-025-entry-complete.md), [SV-B-028](SV-B-028-movement.md) |
| [SV-B-030](SV-B-030-clipboard.md) | Clipboard и URL extraction без нарушения приватности | 5 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-005](SV-B-005-vault.md), [SV-B-020](SV-B-020-first-session.md) |
| [SV-B-031](SV-B-031-chat.md) | Чат, история и локальные формы | 5 | [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-030](SV-B-030-clipboard.md) |
| [SV-B-032](SV-B-032-message-recall.md) | Полный recall и important history | 8 | [SV-B-031](SV-B-031-chat.md) |
| [SV-B-033](SV-B-033-guide-tools.md) | Bookmarks, копирование и вставка из Guide | 5 | [SV-B-008](SV-B-008-guide.md), [SV-B-030](SV-B-030-clipboard.md), [SV-B-031](SV-B-031-chat.md) |
| [SV-B-034](SV-B-034-exports.md) | Экспорт персонажа/сообщений и notes | 4 | [SV-B-003](SV-B-003-profile.md), [SV-B-032](SV-B-032-message-recall.md) |
| [SV-B-035](SV-B-035-sheet.md) | Character sheet и ранние knowledge данные | 5 | [SV-B-008](SV-B-008-guide.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-031](SV-B-031-chat.md), [SV-B-033](SV-B-033-guide-tools.md), [SV-B-034](SV-B-034-exports.md) |
| [SV-B-036](SV-B-036-session-end.md) | Смерть, final review и нормальный выход | 9 | [SV-B-004](SV-B-004-save.md), [SV-B-008](SV-B-008-guide.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-025](SV-B-025-entry-complete.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md) |
| [SV-B-037](SV-B-037-reconnect.md) | Reconnect, portal relogin и redraw | 3 | [SV-B-005](SV-B-005-vault.md), [SV-B-006](SV-B-006-login.md), [SV-B-036](SV-B-036-session-end.md) |
| [SV-B-038](SV-B-038-font-choices.md) | Выбор шрифтов и фильтров в живой сессии | 5 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-020](SV-B-020-first-session.md) |
| [SV-B-039](SV-B-039-map-effects.md) | Tiles, анимации, lighting и восстановление | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-038](SV-B-038-font-choices.md) |
| [SV-B-040](SV-B-040-audio-controls.md) | Громкость и mute ранних audio callers | 5 | [SV-B-004](SV-B-004-save.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-020](SV-B-020-first-session.md) |
| [SV-B-041](SV-B-041-layout-settings.md) | Выбор layout, window mode и UI scale | 3 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-038](SV-B-038-font-choices.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-042](SV-B-042-local-settings.md) | Локальные pickup/destroy и slash option changes | 6 | [SV-B-004](SV-B-004-save.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-031](SV-B-031-chat.md) |
| [SV-B-043](SV-B-043-screenshots-platform.md) | Снимок экрана и optional platform ветви | 4 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-044](SV-B-044-message-clones.md) | Независимые message clone outputs | 2 | [SV-B-003](SV-B-003-profile.md), [SV-B-032](SV-B-032-message-recall.md) |
| [SV-B-045](SV-B-045-config-identity.md) | CFG: account, endpoint и startup names | 8 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-006](SV-B-006-login.md) |
| [SV-B-046](SV-B-046-config-render.md) | CFG: frame rate, palette, tiles и decorations | 8 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-039](SV-B-039-map-effects.md), [SV-B-041](SV-B-041-layout-settings.md) |
| [SV-B-047](SV-B-047-config-audio.md) | CFG: audio enabled flags и category volumes | 10 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-040](SV-B-040-audio-controls.md) |
| [SV-B-048](SV-B-048-options-input.md) | Опции клавиш, макросов и store context | 5 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-007](SV-B-007-macros.md), [SV-B-027](SV-B-027-pickup-store.md) |
| [SV-B-049](SV-B-049-options-messages-1.md) | Опции входящих сообщений и их представления — часть 1 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md) |
| [SV-B-050](SV-B-050-options-messages-2.md) | Опции входящих сообщений и их представления — часть 2 | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md) |
| [SV-B-051](SV-B-051-options-alerts-1.md) | Опции paging и предупреждений — часть 1 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-028](SV-B-028-movement.md) |
| [SV-B-052](SV-B-052-options-alerts-2.md) | Опции paging и предупреждений — часть 2 | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-028](SV-B-028-movement.md) |
| [SV-B-053](SV-B-053-options-hud-1.md) | Опции чисел и status bars — часть 1 | 8 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-017](SV-B-017-hud-detail.md), [SV-B-035](SV-B-035-sheet.md) |
| [SV-B-054](SV-B-054-options-hud-2.md) | Опции чисел и status bars — часть 2 | 8 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-017](SV-B-017-hud-detail.md), [SV-B-035](SV-B-035-sheet.md) |
| [SV-B-055](SV-B-055-options-maplight-1.md) | Опции освещения и видимости карты — часть 1 | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-056](SV-B-056-options-maplight-2.md) | Опции освещения и видимости карты — часть 2 | 8 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-057](SV-B-057-options-mapidentity-1.md) | Опции glyph identity и highlighting — часть 1 | 5 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-058](SV-B-058-options-mapidentity-2.md) | Опции glyph identity и highlighting — часть 2 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-059](SV-B-059-options-animation-1.md) | Опции анимации, weather и color scopes — часть 1 | 4 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-018](SV-B-018-messages-live.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-060](SV-B-060-options-animation-2.md) | Опции анимации, weather и color scopes — часть 2 | 5 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-018](SV-B-018-messages-live.md), [SV-B-039](SV-B-039-map-effects.md) |
| [SV-B-061](SV-B-061-options-movement-1.md) | Опции движения, running и disturbance — часть 1 | 5 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md) |
| [SV-B-062](SV-B-062-options-movement-2.md) | Опции движения, running и disturbance — часть 2 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md) |
| [SV-B-063](SV-B-063-options-movement-3.md) | Опции движения, running и disturbance — часть 3 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md) |
| [SV-B-064](SV-B-064-options-target.md) | Опции target и подготовленного retaliator | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-026](SV-B-026-target.md), [SV-B-028](SV-B-028-movement.md) |
| [SV-B-065](SV-B-065-options-pickup-1.md) | Опции incoming items и pickup — часть 1 | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-042](SV-B-042-local-settings.md) |
| [SV-B-066](SV-B-066-options-pickup-2.md) | Опции incoming items и pickup — часть 2 | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-042](SV-B-042-local-settings.md) |
| [SV-B-067](SV-B-067-options-pickup-3.md) | Опции incoming items и pickup — часть 3 | 8 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-042](SV-B-042-local-settings.md) |
| [SV-B-068](SV-B-068-options-session-1.md) | Опции idle, death и session return — часть 1 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-037](SV-B-037-reconnect.md) |
| [SV-B-069](SV-B-069-options-session-2.md) | Опции idle, death и session return — часть 2 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-037](SV-B-037-reconnect.md) |
| [SV-B-070](SV-B-070-options-audio-1.md) | Опции audio incoming effects и suppression — часть 1 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md), [SV-B-040](SV-B-040-audio-controls.md) |
| [SV-B-071](SV-B-071-options-audio-2.md) | Опции audio incoming effects и suppression — часть 2 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md), [SV-B-040](SV-B-040-audio-controls.md) |
| [SV-B-072](SV-B-072-options-audio-3.md) | Опции audio incoming effects и suppression — часть 3 | 6 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md), [SV-B-040](SV-B-040-audio-controls.md) |
| [SV-B-073](SV-B-073-options-audio-4.md) | Опции audio incoming effects и suppression — часть 4 | 7 | [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md), [SV-B-040](SV-B-040-audio-controls.md) |
| [SV-B-074](SV-B-074-options-integration.md) | Опции pacing, capture и clone outputs | 6 | [SV-B-002](SV-B-002-contact.md), [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-043](SV-B-043-screenshots-platform.md), [SV-B-044](SV-B-044-message-clones.md) |
| [SV-B-075](SV-B-075-acceptance.md) | Приёмка B на всех обязательных платформах | 0 | [SV-B-001](SV-B-001-endpoint.md), [SV-B-002](SV-B-002-contact.md), [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-005](SV-B-005-vault.md), [SV-B-006](SV-B-006-login.md), [SV-B-007](SV-B-007-macros.md), [SV-B-008](SV-B-008-guide.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-015](SV-B-015-hud-core.md), [SV-B-016](SV-B-016-hud-conditions.md), [SV-B-017](SV-B-017-hud-detail.md), [SV-B-018](SV-B-018-messages-live.md), [SV-B-019](SV-B-019-first-import.md), [SV-B-020](SV-B-020-first-session.md), [SV-B-021](SV-B-021-account-manage.md), [SV-B-022](SV-B-022-character-manage.md), [SV-B-023](SV-B-023-birth-choices.md), [SV-B-024](SV-B-024-birth-dna.md), [SV-B-025](SV-B-025-entry-complete.md), [SV-B-026](SV-B-026-target.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-028](SV-B-028-movement.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-030](SV-B-030-clipboard.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md), [SV-B-033](SV-B-033-guide-tools.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md), [SV-B-036](SV-B-036-session-end.md), [SV-B-037](SV-B-037-reconnect.md), [SV-B-038](SV-B-038-font-choices.md), [SV-B-039](SV-B-039-map-effects.md), [SV-B-040](SV-B-040-audio-controls.md), [SV-B-041](SV-B-041-layout-settings.md), [SV-B-042](SV-B-042-local-settings.md), [SV-B-043](SV-B-043-screenshots-platform.md), [SV-B-044](SV-B-044-message-clones.md), [SV-B-045](SV-B-045-config-identity.md), [SV-B-046](SV-B-046-config-render.md), [SV-B-047](SV-B-047-config-audio.md), [SV-B-048](SV-B-048-options-input.md), [SV-B-049](SV-B-049-options-messages-1.md), [SV-B-050](SV-B-050-options-messages-2.md), [SV-B-051](SV-B-051-options-alerts-1.md), [SV-B-052](SV-B-052-options-alerts-2.md), [SV-B-053](SV-B-053-options-hud-1.md), [SV-B-054](SV-B-054-options-hud-2.md), [SV-B-055](SV-B-055-options-maplight-1.md), [SV-B-056](SV-B-056-options-maplight-2.md), [SV-B-057](SV-B-057-options-mapidentity-1.md), [SV-B-058](SV-B-058-options-mapidentity-2.md), [SV-B-059](SV-B-059-options-animation-1.md), [SV-B-060](SV-B-060-options-animation-2.md), [SV-B-061](SV-B-061-options-movement-1.md), [SV-B-062](SV-B-062-options-movement-2.md), [SV-B-063](SV-B-063-options-movement-3.md), [SV-B-064](SV-B-064-options-target.md), [SV-B-065](SV-B-065-options-pickup-1.md), [SV-B-066](SV-B-066-options-pickup-2.md), [SV-B-067](SV-B-067-options-pickup-3.md), [SV-B-068](SV-B-068-options-session-1.md), [SV-B-069](SV-B-069-options-session-2.md), [SV-B-070](SV-B-070-options-audio-1.md), [SV-B-071](SV-B-071-options-audio-2.md), [SV-B-072](SV-B-072-options-audio-3.md), [SV-B-073](SV-B-073-options-audio-4.md), [SV-B-074](SV-B-074-options-integration.md) |

## Первый runnable milestone

Начать с SV-B-001 (endpoint/native input) и SV-B-002 (real contact), затем
завершить зависимости SV-B-020: собственный профиль/Save, private vault/login,
macro/INS/history/Guide, FILE/Lua, audio/fonts, карта, live HUD/messages и
first-launch data-only import. SV-B-020 выполняет real-server existing-character
путь до clean disconnect. Полный `session.enter-game` принадлежит SV-B-025 после
birth, поэтому первый успешный вход не объявляется полной приёмкой этого ID.

После milestone: account/character creation и birth/backtracking/Guide → полная
entry readiness → directions/targets/pickup/store-read/movement/map → chat/history →
sheet/final review/death/quit/reconnect. Затем завершаются уже включённые в B
font/resource selectors, settings/config fields и option consumers; ничего из
этого не переносится в E ради удобства декомпозиции. SV-B-075 — cumulative gate.

## Проверка плана

```sh
python3 docs/tasks/stage-b/check-plan.py
```

Проверка читает canonical files, не пишет их: ровно пять обязательных canonical paths с корректными SHA-256 (manifest, native ledger, reconciliation, native evidence, inventory index),508 current B IDs ровно один
раз, unknown/non-B owners, exact prerequisites/obligations/source IDs и allocation
hashes, references, acyclic ticket DAG, owner-before-dependent либо A foundation,
Markdown owner tables, production subset availability, forward integration checks и локальные ссылки. Пустой/неполный/лишний snapshot map и некорректный digest дают ошибку; отсутствие записи не позволяет скрыть изменённый manifest или scenario. Scenario cross-stage references
разрешены как fixtures/regressions; они не становятся readiness edges.

Новая runtime реализация изменит source/evidence identities: при согласованном
изменении registry обновлять этот planning snapshot явно, а не отключать freshness
проверку. Здесь не утверждается пригодность какого-либо будущего evidence producer.

## Первоначальная проверка planning snapshot — 2026-09-23

`check-plan.py`: **valid**,508 current B IDs /508 unique owners,75 tickets
(73owning +2milestone/gate), DAG acyclic,3,271 local links проверены,
canonical files unchanged. Внешние prerequisites принадлежат только A foundation:
`messages.read-occurrences` и `status.read-hp`; это ссылка на scoped A checkpoint,
не утверждение accepted canonical claims. Все исходные source/obligation IDs и
allocation hashes совпали. New-file whitespace и `git diff --check` прошли;
checker syntax проверен без изменения runtime code. Binaries/GUI/data suites
не запускались для planning-only изменения. Runtime evidence остаётся pending.

## Исправления после review

Ранние production subsets и поздние integration checks явно разделяют готовность
к следующей реализации и закрытие полной acceptance. Подробности:
[две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration).
Map029 получает настоящий chat/capture сейчас; sheet035 использует capture029;
clipboard paste появляется с001; default graphics1/16x24sv painter — с014 доM1.
Полные owners и508 IDs не меняются. Аналогичные cross-ticket caller проверки
явно перечислены в coverage и соответствующих тикетах; скрытого «готово» нет.

```sh
python3 -B docs/tasks/stage-b/check-plan.py
python3 -B docs/tasks/stage-b/check_plan_tests.py
```

Негативные regression checks вызывают production checker с копиями canonical
files и изменённым входным planning snapshot. Проверяются отсутствие каждого
обязательного hash, пустой/лишний map, некорректные digest, изменения manifest и
scenario при пропущенном hash, а также обычное обнаружение drift при полном map.
Оригинальные canonical файлы тесты не меняют. Это проверки инструмента планирования,
не runtime game behavior и не native acceptance.

Повторная проверка после исправлений: **valid**,508 уникальных owners,75тикетов,
DAG acyclic,8 production subsets,71 forward integration-check links,
3,661 local links,все5 canonical snapshots неизменны. Checker regression suite:
**9/9 PASS**, включая subcases для каждого отсутствующего hash и malformed digest.
`git diff --check` и whitespace всех80 новых planning files — PASS.

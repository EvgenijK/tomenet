# C027 — Ordinary stores: отложенная приёмка C012

Статус: planned/deferred acceptance, 2026-11-13. Компактная связка приёмки, не полный план этапа и не заявление реализации.

Связка **C012-DA-07** — [центральный реестр C012](../c012/deferred-acceptance.md).

При настоящем store consumer принять отдельный bank balance в baseline Merchant Guild/nonzero context. Carried gold и bank остаются разными данными C012; не подменять одно другим и не добавлять банковскую строку в Character. Перед размещением проверить store UX.

Это совместная приёмка результата C012 при consumer соответствующего этапа, **не расширение canonical scope**, не передача protocol/model ownership и не изменение IDs/allocation/prerequisites/statuses. Сохранность данных, derived semantics и необходимые core проверки остаются C012; его техническая часть не ждёт этой визуальной приёмки. Evidence и необходимые исправления хранить здесь, затем добавить обратную ссылку в реестр. Эта отсрочка не даёт PASS полному outcome.

Применять [правило платформ](../../agents/platform-validation.md): необходимый scoped Linux минимум, Windows/MinGW/Wine deferred. Новая suite/harness или broad matrix этим планом не разрешена.

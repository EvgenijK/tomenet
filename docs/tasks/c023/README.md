# C023 — Skills и abilities: отложенная приёмка C012

Статус: planned/deferred acceptance, 2026-11-13. Компактная связка приёмки, не полный план этапа и не заявление реализации.

Связка **C012-DA-05** — [центральный реестр C012](../c012/deferred-acceptance.md).

Совместно с настоящим skills consumer принять видимый признак доступности Study. [Технический контракт C012 разрешён](../c012/final-disposition.md): полный source byte сохраняется отдельно от availability=nonzero; фактический sender передаёт bool, а не количество skill points. Это не означает принятого пользовательского отображения. Не приравнивать PKT_STUDY к skill points и не менять его ownership. Место в UX пока не установлено; новых блоков Character не добавлять.

Это совместная приёмка результата C012 при consumer соответствующего этапа, **не расширение canonical scope**, не передача protocol/model ownership и не изменение IDs/allocation/prerequisites/statuses. Сохранность данных, derived semantics и необходимые core проверки остаются C012; его техническая часть не ждёт этой визуальной приёмки. Evidence и необходимые исправления хранить здесь, затем добавить обратную ссылку в реестр. Эта отсрочка не даёт PASS полному outcome.

Применять [правило платформ](../../agents/platform-validation.md): необходимый scoped Linux минимум, Windows/MinGW/Wine deferred. Новая suite/harness или broad matrix этим планом не разрешена.

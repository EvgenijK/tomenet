# C030 — Information surfaces: отложенная приёмка C012

Статус: planned/deferred acceptance, 2026-11-13. Компактная связка приёмки, не полный план этапа и не заявление реализации.

Связка **C012-DA-02** — [центральный реестр C012](../c012/deferred-acceptance.md).

При полном Character read-sheet consumer принять PROFILE/SKILLS/RESISTS, переключения/active state, navigation/close, overflow/scroll и читаемость при small/resize. Для misc combat и conditional max/boosted stat-sheet values сначала установить baseline consumer и утверждённое место, не создавать строки только по наличию packet field. Независимая визуальная приёмка включает целый реально отрисованный HTML и SV при одинаковом viewport/scale/сопоставимых состояниях, все вкладки и actual small/resize. Согласованный Cascadia Mono допустим. Новых блоков Character/StatusDetails не добавлять. Полная resistance matrix — собственная возможность C030; разрешённые для C012 unknown values не означают её готовность.

Это совместная приёмка результата C012 при consumer соответствующего этапа, **не расширение canonical scope**, не передача protocol/model ownership и не изменение IDs/allocation/prerequisites/statuses. Сохранность данных, derived semantics и необходимые core проверки остаются C012; его техническая часть не ждёт этой визуальной приёмки. Evidence и необходимые исправления хранить здесь, затем добавить обратную ссылку в реестр. Эта отсрочка не даёт PASS полному outcome.

Применять [правило платформ](../../agents/platform-validation.md): необходимый scoped Linux минимум, Windows/MinGW/Wine deferred. Новая suite/harness или broad matrix этим планом не разрешена.

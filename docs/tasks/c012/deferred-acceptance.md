# C012 — отложенные приёмки интерфейса

Статус: **deferred acceptance; реализация и PASS поздних flows не заявлены**, 2026-11-13.

Решение пользователя: в C012 интерфейс больше не дорабатываем; связанные с ним приёмки откладываем до соответствующих consumers. Этот реестр задаёт проверяемые follow-up и обратные ссылки. Он не переносит canonical capabilities, allocation, prerequisites или runtime statuses и не расширяет canonical scope принимающих этапов. C012 сохраняет 27 outcomes / 81 obligations в [snapshot](coverage.json).

## Реестр

Все строки ниже открыты. Принимающий этап хранит собственное evidence и возвращает ссылку сюда; отсрочка не является PASS. Формулировка «при этапе» означает момент совместной приёмки результата C012 с настоящим consumer, а не новое владение его protocol/model.

| ID | Отложенная приёмка | Принимающий этап и граница |
|---|---|---|
| C012-DA-01 | HUD на реальных серверных данных: цвета характеристик, выбранная иконка нагрузки, Sanity и XP; применимая временная реакция/снятие critical tile | [C014](../c014/README.md#отложенная-приёмка-c012--2026-11-13): после actual gameplay handoff, включая базовую доступность HUD в small. Проверяется результат C012, не переносится весь status scope. |
| C012-DA-02 | Законченный Character: PROFILE/SKILLS/RESISTS, active tabs, navigation/close, читаемость и доступность всего содержимого при small/resize; conditional stat-sheet values и misc combat только после сверки baseline consumer | [C030](../c030/README.md): итоговая приёмка вместе с полным read-sheet consumer. Повторная визуальная сверка целого HTML/SV при одинаковом viewport, documented state/scale и согласованном Cascadia Mono; исходники HTML сами по себе не заменяют снимки. |
| C012-DA-03 | Общая компоновка wide/small, UI scale, ширины панелей и видимость блоков через продуктовые настройки | [C044](../c044/README.md#отложенная-приёмка-c012--2026-11-13). Локальный resize Character относится DA-02; первого live HUD smoke не нужно откладывать до C044. |
| C012-DA-04 | Клиентские варианты XP/stats/resource bars: exp-bar/exp-need, linear-stats, sn-huge-bar и применимые numeric/bar forms | [C046](../c046/README.md#отложенная-приёмка-c012--2026-11-13), через реальный option consumer. Word/star-bar/percent/exact Sanity задаёт сервер с Health-skill gates; это не четыре новых клиентских настройки. |
| C012-DA-05 | Видимый признак доступности обучения Study | [C023](../c023/README.md), совместная приёмка при skills consumer с [разрешённым source-value/availability контрактом](final-disposition.md). Новая явная связка приёмки, не передача PKT_STUDY или приравнивание к skill points. |
| C012-DA-06 | Место отображения self AFK, opaque extra и известного Probability Travel в игровом окне | [C014](../c014/README.md#отложенная-приёмка-c012--2026-11-13): явный deferred C012 UI follow-up. Подтверждённого самостоятельного позднего владельца в плане нет; placement предстоит согласовать при live consumer. Не назначать похожие иконки и не добавлять новые блоки Character. |
| C012-DA-07 | Bank balance отдельно от carried gold | [C027](../c027/README.md), при store consumer: baseline Merchant Guild и nonzero. Совместная приёмка C012 currency с store surface, не расширение canonical allocation C027 и не новый Character slot. |
| C012-DA-08 | Видимые/звуковые предупреждения HP/mana/Sanity, включая sanity damage и применимый offpanel/context | [C050](../c050/README.md#отложенная-приёмка-c012--2026-11-13): реальные triggers, разрешённые outputs и suppression. Хранение occurrence в C012 не подтверждает выполнение alert. |
| C012-DA-09 | Уведомления Martyr/idle через сообщения | [C016](../c016/README.md#отложенная-приёмка-c012--2026-11-13): настоящий message consumer. Не создаёт обязательства новых Character badges. |
| C012-DA-10 | Audio idle: live auto-mute/unmute music/weather с сохранением пользовательского mute state | [C055](../c055/README.md#отложенная-приёмка-c012--2026-11-13): настоящий audio owner. Не приравнивать PKT_IDLE к gameplay Resting. |

## Техническая часть C012 закрыта — 2026-11-13

[Финальный disposition27/81](final-disposition.md): existing Linux build/core check прошли на окончательном freeze3, technical Study/stat-index вопросы разрешены без изменения canonical wording, сохранность данных и точные HP/body/idle event границы подтверждены. Полный runtime PASS и пользовательская приёмка не заявлены; все строки DA-01–10 выше остаются открытыми. Техническая часть не ждёт этих проверок, новых обратных prerequisites не возникает.

Sanity damage и martyr сейчас сохраняются как latest payload, без отдельного FIFO occurrences. C050/C016 при подключении consumer должны обрабатывать каждый применимый complete input до следующего decode либо обеспечить собственную bounded очередь; draw/latest snapshot не восстанавливают промежуточные события. Проверка этого позднего integration flow и необходимые исправления принадлежат DA-08/09; их исполнение C012 не подтверждено.

## Состояние реализации на момент отсрочки

После freeze5 внесён [implementation candidate existing-slots freeze3](../../../.scratch/sv-c012/existing-slots-fix-2026-11-12/implementation.md): stat colours/weapon precedence, Sanity source-label handling, XP/Level derived values, viewport restoration и narrow Resists reflow. [Зафиксированные hashes](../../../.scratch/sv-c012/existing-slots-fix-2026-11-12/implementation-hashes.json): HUD `80ac4b283a7510c2330e3bdf23bbd63c0407ce237ecd155ef2dcde11efb3d423`, binary `58554f922710f3f0629089e3cab8f73817d4ddbbb4aaf524f3c7ae4830a76c6b`.

На момент отсрочки после команды «стоп» финальные focused core check и affected small/resize review не были завершены. Теперь focused core check выполнен на финальном freeze3 (см. итог выше); small/resize остаётся отложенным. Прежние визуальные проверки сохраняют собственные snapshots и не становятся PASS финального freeze3.

Неизвестные RESISTS и честно недоступный DUMP одобрены для C012. Полная resistance matrix относится к C030, настоящий export — к [C029](../c029/README.md#граница-с-c012--2026-11-13); они не становятся новыми обязательствами C012. Приёмка form-change macro execution остаётся у C053/C052, это не новая Character UI приёмка.

## Правила отложенной приёмки

Новых блоков Character/StatusDetails и неподтверждённых placements не добавлять. Старый удалённый PNG не использовать. Сохранять согласованное разделение map/text font и серверную точность Sanity. Достаточны необходимые scoped Linux проверки соответствующего consumer; Windows/MinGW/Wine отложены по [правилу платформ](../../agents/platform-validation.md). Реестр не требует новых suites/provider matrices.

При закрытии строки указать actual consumer, проверенные сценарии, snapshot/evidence и оставшиеся ограничения. Исторические failed/pending observations не переписываются; canonical registration остаётся отдельным workflow.

# C014 — обязательная интеграция файловой основы

Статус: planned, 2026-11-09.

Это компактный план follow-up по решению пользователя 2026-11-09, не полный
исчерпывающий план outcomes C014. Canonical allocation и порядок этапов
сохраняются; реализация и пользовательская приёмка не выполнены.

| Задача | Результат | Связка |
|---|---|---|
| [SV-C014-001](SV-C014-001-exit-history-parent.md) | Exit/relog: history и родитель export interaction | FI-06 |

Начинать с указанной задачи при реализации соответствующего consumer; внутри
записаны зависимости, production путь, критерии готовности, минимальные Linux
проверки и возврат evidence C003. Не возвращаться к общему списку ради уточнения
обязательств и не повторять файловые алгоритмы C003. Проверенный production core
C003 является инфраструктурой; его незавершённые flow rows не получают PASS
автоматически. C003 принят пользователем 2026-11-09; эти связки остаются задачами поздних
этапов и не блокируют/не переоткрывают C003 автоматически.

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.

## Обязательная интеграция C004 — 2026-11-09

Статус связки: planned. Существующие задачи и обязательства C003 выше сохраняются.

| Задача | Результат | Основа |
|---|---|---|
| [SV-C014-002](SV-C014-002-preference-lifecycle.md) | Preferences: handoff, retry и once-only effects | C004 preference core |

C004 принят и завершён решением пользователя 2026-11-09; поздние delivery/flow
checks и необходимые исправления обязательны этому этапу, не переоткрывают C004
автоматически. Подробные source seams, минимальные Linux checks и условия
завершения находятся непосредственно в задаче. Возврат evidence в C004 optional
для трассировки; canonical allocation/conditions/obligations не меняются.

## Lifecycle controls C008

| Задача | Результат |
|---|---|
| [SV-C014-003](SV-C014-003-network-control-lifecycle.md) | Actual handoff/relog/teardown и deferred incoming caller queues |

Actual live owner использует [production controls C008](../c008/README.md).
Close/relog отменяет pause, deadlines и confirmation ownership; старые SDL events,
ack и callbacks не меняют новое generation. Если caller откладывает входящие
packets, pause resume запускает их обработку по baseline `Flush_queue`, а не
очищает outgoing bytes. Этот настоящий caller flow остаётся integration evidence;
core callback сам по себе его не подтверждает. Canonical obligations C008 остаются
у C008. Core C008 принят; самостоятельная обязательная работа закреплена в
[SV-C014-003](SV-C014-003-network-control-lifecycle.md). Она не блокирует и не
переоткрывает C008 автоматически; backlink evidence служит трассировке.


## Lifecycle Lua runtime C009

| Задача | Результат |
|---|---|
| [SV-C014-004](SV-C014-004-lua-runtime-lifecycle.md) | Actual Lua owner adoption/handoff, close/relog generation и cleanup |

[C009 и задачи 001–003 приняты пользователем](../../sv-stage-c009-acceptance.md)
2026-11-10 в границах Linux core. Самостоятельная обязательная интеграция
закреплена в [SV-C014-004](SV-C014-004-lua-runtime-lifecycle.md): actual live
consumer принимает production Lua owner C009 и результаты preparation C011,
отзывает session bindings при close/relog и отклоняет stale callbacks.
Process-sticky audio/Guide flags сохраняются между sessions; shared script files
не удаляются. Владение runtime и metadata не заменяется заимствованием из
уничтоженного pregame caller.

Минимальные actual lifecycle Linux checks и критерии готовности находятся в
задаче. Прямой core test C009 не подтверждает этот consumer flow; непроверенные
flows не получают PASS. Работа и необходимые исправления обязательны C014,
не блокируют и не переоткрывают C009 автоматически. Canonical allocation,
prerequisites/obligations и pending registry statuses сохранены; evidence backlink
служит трассировке, не gate уже состоявшейся приёмки C009. Windows/MinGW/Wine deferred.

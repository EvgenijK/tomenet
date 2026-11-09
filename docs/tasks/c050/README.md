# C050 — Alerts: применение preference values

Статус: planned, 2026-11-09. Компактное обязательное дополнение к C050,
не полный план его canonical alert outcomes.

## Production seam и работа

[C045](../c045/README.md) предоставляет значения alert/input-message options
и применимые `SV_PREF_OPTION` occurrences. Их источник — живой `SvPreferences`:
`sv_preferences_options` хранит exact slots/values, `sv_preferences_effect` —
assignment occurrence с source, `in_game` и generation, включая same-value.
При реализации C050 подключить эти данные через явную application/session
assembly к настоящему alert owner и действующим source-defined triggers.
Сверить baseline gates и потребителей alert options, включая HP/mana, AFK,
offpanel damage и starvation, по исходникам и settings-policy; не вводить
универсальный alert на любой X/Y assignment. Same-value occurrence сохраняется
для применимого immediate behavior, но сама по себе не является игровым событием
и не должна создавать повторный alert, если baseline этого не делает.

Настоящие alert conditions, разрешённые outputs и suppression/context rules
выполняет C050 при появлении своего consumer. Redraw/recall/config preview не
воспроизводят alert; stale generation не действует в новой сессии. Общий
`clear_effects` не удаляет чужие pending messages/actions. Эта integrated проверка
обязательна C050, а не обратный prerequisite C045/C011/C014.

## Минимальные Linux проверки и завершение

В существующем production alert path применить относящуюся preference, затем
подать реальное semantic game update, вызывающее alert по baseline. Проверить
actual alert/output и applicable enabled/disabled/context gates; повтор
same-value assignment без нового trigger не создаёт выдуманный alert. Redraw
и stale generation не повторяют occurrence. Использовать только необходимые
core cases своего C050 consumer, без расширения прежних stage suites.

Связка завершена, когда actual alert consumer использует C045/preference values
и даёт source-defined результат на реальное событие, с корректным lifecycle.
Bool table, parser intent и тестовый alert sink без production caller не
закрывают flow. Исправления найденного соединения выполняются в C050.

C004 принят и завершён решением пользователя 2026-11-09. Эта связка и необходимые
исправления обязательны своему позднему этапу, не переоткрывают C004 автоматически.
Его три IDs, conditions и девять obligations сохраняют canonical allocation
и statuses; [C004 snapshot](../c004/coverage.json) не переписывается. Evidence
хранится здесь; backlink в C004 optional для трассировки, не gate его приёмки.
Только необходимый минимум Linux production core checks по [AGENTS](../../../AGENTS.md),
расширение согласовать; Windows/MinGW/Wine deferred. Новые suites, full plans,
harness и общие матрицы не вводятся. Реализация и PASS этой связки не заявлены.

# C011 — обязательная интеграция preferences

Статус: planned, 2026-11-09. Это компактное дополнение к этапу, не полный план
всех его canonical outcomes и не заявление реализации.

| Задача | Результат | Основа |
|---|---|---|
| [SV-C011-001](SV-C011-001-preference-adoption.md) | Session preparation и adoption preferences | C004 preference core |
| [SV-C011-002](SV-C011-002-network-control-preparation.md) | Live control preparation/options и handoff contract | Принятый core C008 |

Самодостаточная задача содержит production seam, работу, минимальные Linux
проверки и критерии завершения. C004 принят и завершён решением пользователя
2026-11-09; его оставшиеся проверки и исправления выполняются здесь и в связанных
поздних owners, без автоматического переоткрытия C004. Его три IDs, conditions
и девять obligations сохраняются в canonical allocation; evidence своего этапа
можно связать с C004 для трассировки, это не gate уже состоявшейся приёмки.
Windows/MinGW/Wine deferred; новые broad suites и full plans здесь не создаются.

## Подключение controls C008

При создании actual login → live session handoff подключить тот же production
control owner [C008](../c008/README.md): ordered pause/flush submission, свежий
native input epoch и options pacing. Передать generation, unread bytes и pending
sends без повторного исполнения controls. Synthetic/native core evidence C008
не доказывает handoff. Полный набор игровых packet consumers должен быть готов
до подачи live stream: отсутствие позднего decoder не означает unknown packet
по behavior baseline. Canonical allocation C008 и его integration gaps сохраняются;
C008 принят в границах core. Обязательная работа закреплена в
[SV-C011-002](SV-C011-002-network-control-preparation.md); она не блокирует и
не переоткрывает C008 автоматически. Actual handoff проверяет C014, live map —
C013. Это не переоткрывает B.

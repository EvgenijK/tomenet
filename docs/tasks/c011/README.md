# C011 — обязательные интеграции session preparation

Статус: planned, 2026-11-09. Это компактное дополнение к этапу, не полный план
всех его canonical outcomes и не заявление реализации.

| Задача | Результат | Основа |
|---|---|---|
| [SV-C011-001](SV-C011-001-preference-adoption.md) | Session preparation и adoption preferences | C004 preference core |
| [SV-C011-002](SV-C011-002-network-control-preparation.md) | Live control preparation/options и handoff contract | Принятый core C008 |
| [SV-C011-003](SV-C011-003-lua-startup-preparation.md) | Реальные Lua server features, readiness/results и ownership handoff | Принятый core C009 |

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


## Подключение Lua startup C009

Обязательная работа закреплена в
[SV-C011-003](SV-C011-003-lua-startup-preparation.md). При полном планировании C011
подключить [C009 production runtime и reload](../c009/README.md):
передать реальные server feature flags до open/c-init, использовать отдельные
transfer result и Lua load result перед dependent startup/handoff. Done flag
не означает успешную загрузку. Ошибка сохранения не должна превращаться в
успешное обновление от одного лишь успешного reload. По решению пользователя
2026-11-10 сохраняется baseline: последнее END вызывает попытку reload даже при
ошибке сохранения; результаты учитываются отдельно.
[Решение C009](../c009/source-contract.md).
Actual FILE_END/publication/ACK consumer — C010; прямой core API test C009
не доказывает actual preparation C011. Session teardown/generation остаётся C014.
Этот раздел — integration obligation, не полный план C011 и не его приёмка.

C009 и SV-C009-001–003 завершены и приняты пользователем 2026-11-10 в текущих
границах core. Реализация и проверки 003 обязательны C011, не блокируют и не
переоткрывают C009 автоматически. Canonical allocation/conditions/prerequisites
и registry statuses сохраняются; непроверенный preparation flow не получает PASS.
Backlink evidence в C009 служит трассировке, не является gate его завершения.

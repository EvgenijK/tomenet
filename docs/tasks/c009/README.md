# C009 — startup Lua reload

Статус: **accepted** — Linux core, 2026-11-10. C009 и задачи 001–003 завершены.
[Решение о приёмке](../../sv-stage-c009-acceptance.md).
Полный план одного canonical outcome и трёх
obligations. [Спецификация](../../sv-stage-c009-spec.md),
[source contract](source-contract.md), [точный snapshot](coverage.json).

| Задача | Результат | Зависимость |
|---|---|---|
| [SV-C009-001](SV-C009-001-startup-runtime.md) | Настоящий SV Lua runtime/startup/reload | C003 resource overlay |
| [SV-C009-002](SV-C009-002-reload-lifecycle.md) | Completion API, Guide metadata, audio warning, lifecycle | 001; поведение при ошибке согласовано |
| [SV-C009-003](SV-C009-003-evidence.md) | Минимальное Linux evidence всех трёх obligations | 001–002 |

Единственный canonical owner `capability.lua.reload` — 001; 002 реализует его
интеграционную часть, 003 собирает evidence. Новых IDs нет. Порядок реализации
001 → 002 → 003 не меняет canonical prerequisites. Технические seams выбираются
по baseline без общего refactor legacy/common; намеренная SV-local адаптация допустима.

## Вопросы

Продуктовых UX-вопросов не найдено. Технический closure startup bindings нужно
зафиксировать в 001 до реализации. Пользователь 2026-11-10 выбрал сохранение
baseline: после последнего FILE_END попытка reload выполняется даже при ошибке
сохранения, transfer failure и reload result учитываются отдельно.
[Решение и источники](source-contract.md). Открытый вопрос задачи 002 закрыт.
Sticky audio/Guide flags сохраняют фактический baseline lifetime; их исправление
не включается скрыто. Visible Lua failure требуется самим manifest.

## Обязательства следующих consumers

[C010](../c010/README.md) должен вызвать production owner 002 после фактического последнего END,
сохранив публикацию → reload → audio warning → Guide metadata → ACK/ERR и
разделение результатов; accepted INIT сразу сохраняет process-sticky flags;
проверить split/chained input, failure и отсутствие повторов/stale effects через
свой decoder. [C011](../c011/README.md) должен передать server flags и различать успешную загрузку,
ошибку передачи и ошибку runtime перед dependent startup/handoff;
[C014](../c014/README.md) — закрыть
session data и отклонить запоздалый completion. Эти flows остаются непроверенными
до соответствующих этапов; прямой C009 core test их не принимает.

Linux обязателен; только необходимые core checks через production-путь,
расширение — по согласованию. Windows/MinGW/Wine deferred. Snapshot — planning,
а не native runtime registration, реализация или пользовательская приёмка.

## Результат реализации — 2026-11-10

001–003 реализованы в границах production core и проверены на Linux:
[отчёт](../../sv-c009-evidence.md). `lua/runtime` использует существующий engine,
исполнил 41 script load и подготовил 25 школ/310 spells. `lua/reload` принимает
accepted INIT/END, сохраняет sticky flags и generation, отдельно возвращает
transfer/reload results и обновляет Guide metadata после audio warning.
Native startup владеет application owner и исполняет пять ранних scripts.

Linux production build, единая ASan/UBSan core проверка и bounded native startup
прошли. Actual FILE_END decoder и startup/handoff/teardown остаются C010/C011/C014,
как закреплено выше; они не получают PASS от core evidence. C009 и задачи 001–003
приняты пользователем в текущих границах Linux core; canonical registration
остаётся отдельным workflow. Snapshot остаётся исходным planning-only.

## Самостоятельные обязательства следующих этапов

| Задача | Что завершает |
|---|---|
| [SV-C010-001](../c010/SV-C010-001-lua-reload-integration.md) | Actual transfer caller, publication/ACK/ERR и Lua reload ordering |
| [SV-C011-003](../c011/SV-C011-003-lua-startup-preparation.md) | Server features/open, actual preparation/readiness и handoff contract |
| [SV-C014-004](../c014/SV-C014-004-lua-runtime-lifecycle.md) | Actual handoff/adoption/relog/teardown и stale completion protection |

По решению пользователя 2026-11-10 C009 закрыт сейчас. Эти доработки/проверки
обязательны своим этапам, не блокируют и не переоткрывают C009 автоматически.
Canonical ownership остаётся у C009; evidence backlink — трассировка, не gate
его приёмки. Неисследованные flows остаются pending до собственного evidence.

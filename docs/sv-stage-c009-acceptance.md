# C009 — решение о приёмке

Статус: **accepted**, 2026-11-10, по поручению пользователя закрыть этап сейчас.
C009 и задачи SV-C009-001–003 завершены и приняты в текущих границах Linux core.
[Evidence](sv-c009-evidence.md), [спецификация](sv-stage-c009-spec.md),
[задачи](tasks/c009/README.md).

## Основание решения

После отчёта о готовности core и непроверенных integrations пользователь поручил:

«запусти агентов спланировать работы в этапах C010 C011 C014, чтобы этот этап закрыть сейчас».

Позднее пользователь явно подтвердил: «считаем этап закрытым».

Это решение закрывает C009 сейчас с закреплением оставшейся обязательной работы
в следующих этапах. Оно не объявляет выполненными отсутствующие transfer/session
consumers и не повышает canonical runtime statuses.

Приняты настоящий Lua startup/reload runtime над существующим engine, shared
resource overlay для pern_dofile, server-feature interface, C-owned schools/spells,
Guide metadata, accepted INIT/END API, process-sticky flags, generation/cleanup,
audio restart warning и раздельные transfer/reload results. Сохраняется ранее
выбранный пользователем baseline: last END инициирует reload и при ошибке сохранения.

Linux production build, одна focused ASan/UBSan production core проверка и bounded
native early startup прошли. Bundled chain: 41 загрузка, 25 школ, 310 заклинаний;
actual early startup: 5 scripts. Все подробности и непроверенные условия — в evidence.

| Задача | Решение |
|---|---|
| SV-C009-001 | accepted — Lua runtime/startup/reload core |
| SV-C009-002 | accepted — INIT/END lifecycle, Guide metadata и audio warning core |
| SV-C009-003 | accepted — Linux evidence и disposition трёх obligations |

## Самостоятельная работа следующих этапов

| Owner | Обязательная работа |
|---|---|
| [SV-C010-001](tasks/c010/SV-C010-001-lua-reload-integration.md) | Actual transfer decoder → accepted INIT/END → publication/reload/warning/metadata → ACK/ERR, ошибки записи и порядок результатов |
| [SV-C011-003](tasks/c011/SV-C011-003-lua-startup-preparation.md) | Реальные server flags/open, подготовка runtime, readiness и handoff contract с раздельными результатами |
| [SV-C014-004](tasks/c014/SV-C014-004-lua-runtime-lifecycle.md) | Actual adoption/handoff/close/relog, session generation и освобождение данных без потери process-sticky flags |

Эти задачи обязательны своим этапам; реализация и evidence остаются pending.
Их доработки, проверки и найденные при интеграции дефекты выполняются у этих
owners, не блокируют завершение и не переоткрывают C009 автоматически.
Backlink evidence к C009 служит трассировке, не является gate уже состоявшейся
приёмки. Core success не даёт PASS будущим callers. Принятые ранее этапы также
не переоткрываются автоматически.

Известные пределы evidence сохраняются: native smoke использовал SDL dummy/software,
LSan отключён; latency произвольных overlay scripts не ограничена, reload синхронный.
Приёмка не превращает эти условия в проверенные. Shared Lua files не удаляются
при session teardown; полное приложение освобождает runtime при shutdown.

Canonical capability IDs, allocation, conditions, prerequisite и три obligations
сохраняются. Planning snapshot остаётся историческим. Native-runtime registration —
отдельный workflow, canonical implementation/evidence statuses не повышаются этим
решением. Текущая обязательная платформа — Linux; Windows/MinGW/Wine deferred.

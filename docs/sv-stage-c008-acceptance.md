# C008 — решение о приёмке

Статус: **accepted**, 2026-11-09, явно подтверждён пользователем.
C008 и задачи SV-C008-001–004 завершены и приняты в текущих границах Linux core.
[Evidence](sv-c008-evidence.md), [спецификация](sv-stage-c008-spec.md),
[задачи](tasks/c008/README.md).

## Решение пользователя

«C008 считаем принятым в текущих границах core. Оставшиеся интеграции и проверки
закрепи в задачах C011/C013/C014 и C053, чтобы они не блокировали завершение C008».

Приняты atomic recovery с диагностикой, payload-free KEYPRESS/END, ordered
pause/flush submission, fresh native input и bounded pacing, confirmation
delivery/ownership и Linux evidence. Build, focused ASan/UBSan checks и bounded
synthetic native smoke прошли. LSan отключён из-за ограничения среды; full live
runtime и непроверенные flows не получают PASS по этому решению.

| Задача | Решение |
|---|---|
| SV-C008-001 | accepted — packet/recovery core |
| SV-C008-002 | accepted — native pause/flush core |
| SV-C008-003 | accepted — confirmation delivery/owner core |
| SV-C008-004 | accepted — технический отчёт и disposition 18 obligations |

## Самостоятельная работа следующих этапов

| Owner | Обязательная интеграция и проверка |
|---|---|
| [SV-C011-002](tasks/c011/SV-C011-002-network-control-preparation.md) | Подготовка live control owner, options, generation и handoff contract |
| [SV-C013-001](tasks/c013/SV-C013-001-network-control-live-map.md) | Настоящий map ingress → presentation → pause/flush и применимые packet controls |
| [SV-C014-003](tasks/c014/SV-C014-003-network-control-lifecycle.md) | Actual handoff/relog/teardown и deferred incoming caller queue |
| [SV-C053-001](tasks/c053/SV-C053-001-preference-runtime.md) | Реальный macro WAIT/XWAIT consumer confirmation API и interleaved controls |

Эта работа обязательна своим owners и остаётся pending до фактического evidence.
Её отсутствие не блокирует завершение и не переоткрывает C008 автоматически.
Найденные в этих integrations дефекты исправляются в соответствующем этапе.
Backlink evidence в C008 служит трассировке, не является gate уже состоявшейся
приёмки. Ранее принятые этапы также не переоткрываются автоматически.

Canonical IDs, allocation, conditions, prerequisites и 18 obligations сохранены.
Planning snapshot остаётся историческим. Пользовательская приёмка не повышает
canonical implementation/evidence statuses: native-runtime registration остаётся
отдельным workflow. Текущие проверки — Linux; Windows/MinGW/Wine deferred.

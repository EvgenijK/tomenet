# C011 — решение о приёмке

Статус: **accepted**, 2026-11-11.
C011 и задачи SV-C011-001–006 завершены и приняты пользователем в проверенных
границах Linux preparation/core. [План и задачи](tasks/c011/README.md),
[итоговое evidence](sv-c011-evidence.md).

## Основание решения

После реализации, независимого review и закрытия оставшихся собственных проверок
пользователь явно сообщил: **«Принимаю этап C011. Зафиксируй приёмку в документах
и обнови статусы задач.»**

Приняты пять outcomes session preparation: загрузка profile/input, startup file
transfer, startup file failure, accepted identity и redraw. Подготовленный owner
сохраняет transport/unread/output, preferences/history/effects, Lua и native
resources; move-once и occurrence acknowledgement не повторяют уже исполненное.
Character options загружаются до PLAY и передаются внутри PLAY. Необходимый
startup prefix удерживается до identity без применения поздних capabilities.

| Задача | Решение |
|---|---|
| SV-C011-001 | accepted — profile/preferences, startup INS/audio и retained payload |
| SV-C011-002 | accepted — native PAUSE/thin, generation и transport ownership |
| SV-C011-003 | accepted — startup FILE/Lua results и readiness |
| SV-C011-004 | accepted — authoritative identity, prefix и post-MOTD native readout |
| SV-C011-005 | accepted — redraw bytes, backpressure, context и coherent projection |
| SV-C011-006 | accepted — Linux evidence и сверка пяти outcomes / 18 obligations |

Все 11 focused native cases и Linux production build прошли на финальном source
snapshot. Actual isolated server подтвердил native birth → PLAY → MOTD ack →
accepted Born → complete payload; 666 startup frames сохранены, unsupported=0.
Ограниченный X11/software visual review пройден. Команды, hashes, исправленные
findings, промежуточные failures и точные ограничения сохранены в evidence.

## Сохранённые границы

Приёмка относится к выполненной preparation/core части. Полный live adoption,
close/relog C014, HUD/карта C012/C013, macro runtime C052/C053, effect recipients,
INS application/management C021/C059 и live audio C055/C056 остаются задачами своих
этапов. Они не превращаются в выполненные проверки и не являются обратным gate
приёмки C011. Работы следующих этапов этой приёмкой не запускаются.

Physical keyboard/audio, accelerated matrix, sanitizer и прочие непроверенные
условия не получают PASS от этой записи. Windows/MinGW/Wine отложены по
[правилу платформ](agents/platform-validation.md).

Canonical IDs, ownership, prerequisites, conditions и 18 obligations сохраняются.
Planning snapshot и историческое evidence не переписываются. Как для принятых
C009/C010, canonical runtime implementation/evidence statuses остаются pending:
их регистрация является отдельным workflow, а не автоматическим следствием
пользовательской приёмки.

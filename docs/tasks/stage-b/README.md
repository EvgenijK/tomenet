# Этап B: native startup screen flow

Статус: specified, обновлён 2026-10-05. Этап B реализует полностью рабочий
native flow экранов подключения, аутентификации, выбора и создания персонажа
через production UI/model/input/protocol path. Проверки используют управляемый
protocol peer: fixture управляет входными bytes и отказами, но не подменяет
production decoder, serializer, input router, presentation model или UI.

Реальная интеграция с сервером, startup profile/FILE/Lua, переход в игровую
сессию и первый gameplay screen относятся к этапу C. Поэтому B не заявляет
`session.enter-game`, карту, HUD, сообщения, игровой ввод, профиль, ресурсы,
настройки или другие gameplay outcomes.

[Спецификация](../../sv-stage-b-spec.md) задаёт общий контракт.
[coverage.json](coverage.json) связывает каждый из 54 B capability IDs ровно с
одним implementation owner и сохраняет canonical prerequisites, obligations,
sources и allocation hash. Нулевые milestone/gate тикеты проверяют сквозной
результат, но не присваивают себе чужое ownership.

## Активный implementation DAG

| Ticket | Пользовательский результат | Собственных IDs | Зависимости |
|---|---|---:|---|
| [SV-B-001](SV-B-001-endpoint.md) | Endpoint surface и общие native input primitives | 7 | — |
| [SV-B-002](SV-B-002-contact.md) | Contact/control protocol path с управляемым peer | 7 | SV-B-001 |
| [SV-B-005](SV-B-005-vault.md) | Приватный ввод и системное хранилище credentials | 7 | SV-B-001 |
| [SV-B-006](SV-B-006-login.md) | Login и выбор существующего персонажа | 12 | SV-B-001, SV-B-002, SV-B-005 |
| [SV-B-020](SV-B-020-first-session.md) | M1: сквозной screen flow существующего персонажа | 0 | SV-B-006 |
| [SV-B-021](SV-B-021-account-manage.md) | Создание аккаунта | 1 | SV-B-002, SV-B-005, SV-B-006 |
| [SV-B-022](SV-B-022-character-manage.md) | Новый slot и имя персонажа | 5 | SV-B-006 |
| [SV-B-023](SV-B-023-birth-choices.md) | Birth choices, backtracking и cancel | 14 | SV-B-022 |
| [SV-B-024](SV-B-024-birth-dna.md) | Подтверждение создания персонажа | 1 | SV-B-023 |
| [SV-B-025](SV-B-025-entry-complete.md) | M2: сквозной account/character creation screen flow | 0 | SV-B-020, SV-B-021, SV-B-024 |
| [SV-B-075](SV-B-075-acceptance.md) | Cumulative acceptance Stage B | 0 | SV-B-020, SV-B-025 |

Номера сохраняют исторические ссылки; порядок строк — один допустимый
topological порядок. Независимые ветви после SV-B-006 можно выполнять
параллельно. Infrastructure появляется у первого production consumer, а не в
отдельной задаче без пользовательского пути.

## Перенесённые исторические тикеты

Старые файлы оставлены на месте для истории, но не входят в активный B graph,
не владеют B IDs и не являются B prerequisites.

| Назначение | Тикеты | Причина |
|---|---|---|
| Stage C | SV-B-003–004, SV-B-007, SV-B-009–019, SV-B-026–032, SV-B-034–074 | Profile/resources/settings, FILE/Lua, gameplay entry и все игровые surfaces перенесены за минимальную границу startup screens. |
| Stage G | [SV-B-008](SV-B-008-guide.md), [SV-B-033](SV-B-033-guide-tools.md) | Guide capabilities и tools ранее отдельно перенесены в G. Точная временная placeholder в birth caller остаётся вложенной проверкой canonical birth obligation, но не Guide coverage. |

Каждый такой файл явно помечен `moved-to-C` или `superseded-to-G`. Его прежнее
описание сохранено как архивный контекст и не является текущим планом.

## Milestones и граница готовности

**M1 / SV-B-020** проходит путь endpoint → contact → login → overview → выбор
существующего персонажа, включая peer-driven MOTD в предусмотренном protocol
порядке, и останавливается в явном состоянии ожидания live-session handoff. Он
не ждёт startup files, profile, map, HP или gameplay presentation.

**M2 / SV-B-025** добавляет создание аккаунта и персонажа, все birth choices,
backtracking, cancel/rejection и production `birth.complete` exchange. После
peer acknowledgement и MOTD flow клиент остаётся в live-session handoff;
открытие первого игрового экрана остаётся C.

Управляемый peer обязан проверять настоящие wire bytes, fragmentation/chaining,
version/build branches, interleaved events, ошибки, disconnect и stale session
generation. Fixed-response UI mock не является evidence. Реальный TomeNET server
можно использовать как дополнительное наблюдение, но он не требуется для B и не
заменяет детерминированные branch checks.

## Проверка planning snapshot

```sh
python3 docs/tasks/stage-b/check-plan.py
python3 -m unittest docs/tasks/stage-b/check_plan_tests.py
```

Checker не принимает runtime. Он проверяет пять canonical SHA-256 snapshots,
review token точного множества B IDs, exact-once ownership, неизменность
prerequisites/obligations/sources/allocation hashes, Markdown owner tables,
отсутствие moved tickets в активном DAG, forward integration checks, acyclic DAG
и локальные ссылки. Изменение canonical allocation требует явного обновления
snapshot hashes, reviewed count и digest; одного совпавшего количества
недостаточно.

## Общие ограничения реализации

- Production code по возможности остаётся в SV согласно [AGENTS.md](../../../AGENTS.md)
  и [архитектуре](../../sv-architecture.md); тестовая отдельная реализация
  поведения запрещена.
- UI передаёт смысловой ввод; session model и interaction state отделены от UI и
  protocol согласно ADR. Main thread последовательно владеет состоянием, а
  session-scoped work проверяет generation.
- Secret bytes не попадают в diagnostics, history, screenshots или evidence.
- Acceptance требует Linux и Windows paths, physical keyboard/input method,
  mouse where applicable, focus/resize, success/cancel/retry/rejection и clean
  teardown. Missing/failed/stale evidence остаётся pending.
- Никакой B screen не может завершаться terminal fallback или test-only scene.

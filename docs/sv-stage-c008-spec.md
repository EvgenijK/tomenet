# C008 — оставшиеся сетевые управляющие сценарии

Статус: **accepted** в текущих границах Linux core, 2026-11-09. План
подготовлен по запросу пользователя с анализом трёх агентов. Core реализован
агентами по его поручению; [Linux evidence](sv-c008-evidence.md) подготовлено.
[C008 и задачи 001–004 приняты пользователем](sv-stage-c008-acceptance.md).
Поздние integrations обязательны своим этапам и не блокируют завершение C008.
[Задачи и порядок](tasks/c008/README.md),
[source contract](tasks/c008/source-contract.md),
[точный planning snapshot](tasks/c008/coverage.json).

## Результат и состав

C008 завершает network controls сверх transport baseline этапа B. Канонический
объём — **6 outcomes и 18 evidence obligations**: по `result`, `lifecycle` и
`allocation-checks` на каждый outcome. У всех шести canonical prerequisites
пусты. Порядок реализации внутри плана не добавляет prerequisites в registry.

| Outcome `capability.network.*` | Обязательное поведение |
|---|---|
| `unknown-packet` | Для unknown/NULL handler отправить `PKT_UNKNOWNPACKET` с текущим и предыдущим типом, запросить redraw и очистить входной batch по baseline. Длину неизвестного пакета не угадывать. |
| `keypress-stub` | Потребить один байт `PKT_KEYPRESS` как существующую заглушку без payload и без генерации пользовательского ввода. |
| `pause` | Показать актуальное состояние, удалить накопленный ввод, дождаться свежего подтверждения с продолжающимся обслуживанием сети, затем обработать отложенную очередь входящих пакетов caller по baseline `Flush_queue`. |
| `flush` | Передать накопленные визуальные изменения на показ; сохранить `disable_flush` и `thin_down_flush`, не блокируя сеть. Отключение задержки не отменяет показ. |
| `confirm` | Передать байт подтверждённой команды ожидающему macro owner в порядке получения. Перерисовка не повторяет доставку. |
| `end-marker` | Потребить один байт `PKT_END` без придуманного счётчика кадров и без потери соседних пакетов. |

Snapshot сохраняет canonical rows дословно, включая прежние platform conditions;
текущая обязательная платформа проверки — Linux. Canonical IDs, allocation,
implementation/evidence statuses этим планом не меняются.

## Основа и существующая реализация

Нормативны [AGENTS](../AGENTS.md), [CONTEXT](../CONTEXT.md),
[архитектура SV](sv-architecture.md), ADR
[0001](adr/0001-separate-session-interactions-and-ui.md),
[0002](adr/0002-main-thread-state-ownership.md),
[0003](adr/0003-explicit-module-connections.md),
[0004](adr/0004-revision-based-ui-views.md),
[0005](adr/0005-session-scoped-work.md),
[0006](adr/0006-errors-and-overload.md) и
[правило платформ](agents/platform-validation.md).

`src/client/sv/protocol/protocol.c` уже декодирует все шесть сценариев;
unknown recovery атомарно резервирует ответ и redraw. `app.c` доставляет pause,
flush и confirm в существующие `input/input.c` и `ui/flush-policy.h`. В
`tests/sv/contact-control.c` уже есть проверки части этих production-путей.
Этап должен доработать и подключить существующее поведение, не создавать второй
decoder или тестовую модель протокола.

Основной обнаруженный разрыв — между core effects и native consumer: состояние
pause/flush и confirmation-owner API само по себе не доказывает актуальный
показ кадра, отсечение уже накопленных SDL events или продолжение настоящего
macro wait. Точные места и baseline branches записаны в source contract.

## Границы с соседними этапами

- **B:** transport, частичные данные и вложенный UI уже являются baseline;
  C008 дополняет controls и не переоткрывает принятый B.
- **C007:** готовит rendering. C008 отвечает за момент submission по network
  controls и порядок эффектов; новая реализация графики сюда не входит.
- **C011/C013/C014:** actual session preparation/handoff, live map ingress и
  session lifecycle сохраняют своих владельцев. C008 задаёт production contract
  controls, который поздний live loop обязан использовать.
- **C053:** полноценное распознавание и исполнение макросов, `wait/xwait` остаются
  там. C008 владеет получением и доставкой confirm ожидающему владельцу;
  end-to-end продолжение macro wait требует настоящего consumer C053.
- **C010/C017 и дальнейшие gameplay stages:** file transfer, server requests и
  игровые действия не переносятся сюда ради демонстрации controls.

Отсутствующий поздний consumer не считается успешной проверкой. Core readiness
и полный outcome различаются в отчёте: API/очередь/sequence не дают full runtime
PASS. Canonical obligations остаются у C008; late integration evidence должно
ссылаться на них. Решением пользователя от 2026-11-09 принят текущий core scope; late flows
не получают PASS и не являются обратным gate завершённого C008.

## Минимальная реализация и проверка

Implementation tickets фиксируют единственного owner каждого ID и отдельную
задачу Linux evidence. Минимальные изменения размещаются в SV, с сохранением
wire bytes и игрового поведения legacy. Дублирование ради изоляции допустимо;
общий refactor не требуется. Независимые улучшения legacy/common заносятся в
[список улучшений](sv-improvements.md) отдельно от текущей реализации.

Использовать существующие production core cases, добавляя только необходимое:
неполный confirm без частичного эффекта; соседние пакеты после stub/end;
атомарные exact bytes unknown recovery при backpressure; pause со свежим вводом
и продолжающейся сетью; фактический flush submission и ветви delay options;
однократную ordered confirm delivery и reset/stale generation. Для неизвестного
пакета discard остатка batch — предусмотренное восстановление, а не обычная
потеря sentinel. Конкретный минимум и повторное использование existing checks
описаны в tickets.

Реализация тестов вызывает production-путь. Новый harness, широкая visual matrix
и расширение набора проверок без согласования не требуются. Windows/MinGW/Wine
отложены. На этапе планирования runtime проверки не запускались; snapshot
остаётся planning-only. Фактические результаты реализации приведены ниже.

## Вопросы анализа

Блокирующих вопросов к пользователю по составу этапа не обнаружено. До реализации
нужно технически уточнить native boundary свежего ввода для pause, границу
presentation submission и сброса flush budget, а также lifetime подтверждений
ожидающего owner. Это работа implementation tickets по baseline и ADR, без
неявного изменения поведения. Полнота поздних session/macro flows должна быть
указана при отчёте о готовности, а не скрыта успешными core checks.

## Результат реализации — 2026-11-09

001–003 реализованы и проверены в границах production core; 004 подготовила
[технический отчёт](sv-c008-evidence.md) с disposition всех 18 obligations.
Unknown recovery даёт baseline diagnostic. Явный control sink и native adapter
выполняют ordered draw/SDL present, pause freshness и неблокирующий flush pacing;
повторные PAUSE требуют отдельных acknowledgements. Confirmation owner сохранил
существующую production реализацию, дополнены необходимые core checks.

Linux build, focused ASan/UBSan core и bounded native synthetic smoke прошли.
Технические вопросы freshness/submission/budget решены в core. Реальные live
preparation/map/lifecycle и deferred incoming caller queues C011/C013/C014,
а также macro wait consumer C053 остаются неподтверждёнными интеграциями.
Canonical registry не изменён, полный runtime PASS не заявлен. Пользователь
принял C008 и задачи 001–004 в текущих границах core. Самостоятельные обязательства
закреплены в [решении о приёмке](sv-stage-c008-acceptance.md): C011-002, C013-001,
C014-003 и C053-001. Они не блокируют и не переоткрывают C008 автоматически.

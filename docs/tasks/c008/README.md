# C008 — оставшиеся network controls

Статус: accepted пользователем 2026-11-09 в текущих границах Linux core.
C008 и задачи SV-C008-001–004 завершены.
[Решение о приёмке](../../sv-stage-c008-acceptance.md).
Labels: enhancement

План подготовлен по запросу пользователя с анализом трёх агентов. Ровно
**6 network outcomes и 18 obligations**; [спецификация](../../sv-stage-c008-spec.md),
[source contract](source-contract.md), [точный planning snapshot](coverage.json).
Core реализован агентами по поручению пользователя;
[Linux evidence](../../sv-c008-evidence.md) подготовлено. Полный live runtime
не заявлен. Пользовательская приёмка core подтверждена.

| Задача | Результат | Единственные owned `capability.network.*` |
|---|---|---|
| [SV-C008-001](SV-C008-001-packet-dispatch-recovery.md) | Atomic dispatch/recovery, точные bytes и payload-free controls | `unknown-packet`, `keypress-stub`, `end-marker` |
| [SV-C008-002](SV-C008-002-pause-flush-integration.md) | Native presentation/fresh ack/caller queue, flush submission и pacing | `pause`, `flush` |
| [SV-C008-003](SV-C008-003-command-confirmation.md) | Ordered command bytes, confirmation owner и lifecycle | `confirm` |
| [SV-C008-004](SV-C008-004-evidence.md) | Минимальное Linux evidence по 18 obligations | 0 IDs |

Порядок: аудит 001 → доработка 002/003 → сводка 004. Decode всех controls уже
частично реализован в `protocol/protocol.c`; app/input/flush-policy primitives
также существуют. Нужна сверка и завершение production integration, а не второй
network engine. Все canonical prerequisites шести rows пусты; инженерный порядок
не изменяет canonical dependencies, conditions или allocation.

## Что именно завершаем

Unknown/NULL packet: diagnostic, `PKT_UNKNOWNPACKET` + redraw и очистка receive
batch по baseline. KEYPRESS и END: ровно один payload-free byte без новых
действий. PAUSE: показать последние изменения, отбросить pending keyboard input,
дождаться свежего acknowledgement при работающей сети и продолжить deferred
caller queue. FLUSH: всегда представить visual changes, отдельно применить
`disable_flush`/`thin_down_flush` к необязательной задержке. CONFIRM: передать
command byte зарегистрированному waiting owner по порядку, без replay.

Наличие counters/sequences/deadlines не доказывает actual submission или fresh
input. Нынешние native consumers pause/flush не завершены; prompt-only queue flush
не заменяет input batch/quantity/keymap lifecycle. Baseline `Flush_queue()`
обрабатывает deferred server caller packets; очищать вместо этого outgoing wire
queue запрещено. Точные source anchors — в [source contract](source-contract.md).

## Границы с другими этапами

B transport/pregame остаётся принятой основой; C008 сохраняет его production путь.
C007 предоставляет rendering core, а C008 завершает control-driven submission,
не реализуя заново compositor. C011 — session preparation, C013 — полный live map
packet ingress, C014 — live lifecycle. Их полноценные игровые callers здесь не
создаются скрыто, но control-путь обязан быть пригоден для их подключения.

C053 владеет настоящим macro runtime и wait/xwait consumer. C008 владеет canonical
`network.confirm`; API owner check и очередь не доказывают реальный waiting macro
flow. Этот gap и будущая integration obligation C053 остаются явными; allocation
и obligations не переносятся, полный runtime PASS не выдумывается. Пользователь отдельно принял текущий core scope 2026-11-09. Late caller
результат остаётся обязательной работой C053 без обратного gate C008.

Только минимальные SV-local изменения и необходимый минимум production-core
checks на [Linux](../../agents/platform-validation.md). Расширение набора проверок
согласуется заранее; Windows/MinGW/Wine deferred. Canonical JSON не меняется,
coverage snapshot сохраняет исходные rows и pending статусы verbatim. Изоляция,
термины и lifecycle следуют AGENTS/CONTEXT и ADR 0001–0006. Независимые улучшения
фиксируются в [списке улучшений](../../sv-improvements.md).

## Результат реализации

001–003 завершили собственный core, 004 — техническую сверку всех 18 obligations.
Linux build, packet/confirmation/native-control ASan/UBSan checks и bounded
synthetic native smoke прошли. Текущие decode/app/input owners сохранены;
production native adapter подключён к существующему synthetic shell.
Source contract выше сохраняет planning audit; фактические доработки и пределы
проверки описаны в [evidence](../../sv-c008-evidence.md).
Реальные deferred caller queues, live C011/C013/C014 и macro C053 требуют
своего integration evidence. Пользователь принял core явно; эти интеграции не блокируют и не переоткрывают
C008 автоматически. Pending canonical statuses сохраняются.

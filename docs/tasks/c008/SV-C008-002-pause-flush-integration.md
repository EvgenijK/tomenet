# SV-C008-002 — native pause, visual flush и непрерывное обслуживание сети

Статус: accepted пользователем 2026-11-09 в текущих границах Linux core.
Labels: enhancement
Owned IDs: `capability.network.pause`, `capability.network.flush`.
Зависимости реализации: 001; текущие app/input/frame/render owners B/C005/C007. Полный live handoff остаётся C011/C014.

## Основа и границы

Нормативны [спецификация C008](../../sv-stage-c008-spec.md),
[source contract](source-contract.md), [точный planning snapshot](coverage.json),
[CONTEXT](../../../CONTEXT.md), [архитектура](../../sv-architecture.md), ADR
[0001](../../adr/0001-separate-session-interactions-and-ui.md),
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).
Сохранять все canonical conditions/prerequisites/obligations. Все шесть rows
имеют пустой canonical prerequisite list; зависимости реализации ниже его не меняют.
Существующие production protocol/app/input owners дорабатываются в SV; второй
network engine, legacy Terms и общий refactor не нужны. Необходимые отдельные
улучшения legacy/common/SV записывать в [список улучшений](../../sv-improvements.md).

## Работа

1. Соединить decode → app control → настоящий native event/frame owner.
   Существующие `pause_sequence`, `flush_sequence`, `flush_due_ms` и
   `sv_flush_request` — primitives, а не доказательство presentation.
2. PAUSE сначала представляет актуальные visual changes и отбрасывает уже
   ожидающий keyboard input, затем принимает только свежее acknowledgement
   текущего pause/session. Проверить SDL event batch/epoch, keymap prefix,
   quantity/request routes и pending command; не ограничиваться prompt ring.
   Не превращать отброшенный ввод в raw packet, command или следующий answer.
3. Ожидание ack — явное неблокирующее взаимодействие: network, nested requests,
   repaint/focus/resize обслуживаются. Определить control barrier в текущем
   bounded step/frame пути так, чтобы значимые intermediate reactions не
   исчезали от coalescing view flags при нескольких controls в одном budget.
4. После свежего ack выполнить baseline caller queue flush. `Flush_queue()`
   означает обработку отложенных server caller packets, а не очистку outgoing
   socket bytes и не повторный generic keyboard flush. Отсутствующие поздние
   caller owners явно зафиксировать; интерфейс не подменяет их runtime evidence.
5. FLUSH всегда submits pending visual changes. `disable_flush` отключает
   только задержку; `thin_down_flush` оставляет delay для первых десяти flush
   до outer-cycle reset. Сохранить baseline 1 ms pacing неблокирующим deadline,
   не `sleep` внутри packet handler. Нельзя считать сам timestamp submission.
6. Session teardown отменяет pause/deadlines; stale ack/frame/input не меняет
   новую сессию. Сохранить порядок control effects при repaint и nested UI.

## Минимальная проверка и завершение

Минимальный различимый production сценарий: visual update → PAUSE/FLUSH →
соседний network update; до свежего ack старые queued keys/SDL batch не отвечают,
после present свежий key снимает только текущую pause, deferred caller packets
продолжаются по baseline. Проверить существующие prompt и quantity input routes.
Для FLUSH доказать actual submission при disable on/off и pacing boundary 10/11
с thin_down on/off; network не блокируется. Коротко проверить resize/repaint,
повторный step и teardown/stale ack. Повторное создание полного live gameplay
или broad runtime suite не входит; непокрытые late callers указать явно.

Проверки — только необходимый минимум core через production public path на
[Linux](../../agents/platform-validation.md), с переиспользованием существующих
entrypoints. Fixtures подают bytes, время и ввод, но не реализуют поведение
вместо production. Новый общий harness, широкая matrix и stress/soak не входят;
расширение набора согласуется заранее по AGENTS.md. Windows/MinGW/Wine deferred.
Implementation, runtime evidence, пользовательская приёмка и canonical registry —
отдельные состояния. Пользовательская приёмка core зафиксирована ниже;
непроверенные runtime flows сохраняют свой статус.


## Результат реализации — 2026-11-09

Production `SvControlSink` передаёт coherent view executor до следующего decode.
`ui/native-controls.[ch]` выполняет draw + настоящий `SDL_RenderPresent`, задаёт
fresh SDL boundary после pause presentation и обслуживает flush deadline без
блокирующего sleep. Existing synthetic native shell launcher подключает этот
production adapter к своему draw; endpoint login и будущий live handoff не
подменены synthetic режимом. Нужен `ui/native-controls.o` в production build.

Pause имеет приоритет над request/quantity native routing, отсекает старые SDL
события, repeat/modifier и tail acknowledgement gesture, сохраняет активный
request/quantity draft. Pending keymap prefix/command сбрасываются как old input;
готовые outgoing packets не стираются. Повторные PAUSE не coalesce: bounded
32 occurrences требуют отдельных fresh ack; overflow завершает session явно.
Resume sink — уведомление caller для последующей обработки deferred incoming
work, без re-entry app; настоящий поздний caller queue остаётся integration gap.

Flush всегда submits frame. Delay options управляют только 1 ms pacing;
первые десять thin flush считаются в исходном bounded outer budget, сохранённом
через deadline yields. Budget exhaustion, drained input или partial trailing
packet завершают batch; sustained/partial traffic не сохраняет счётчик навсегда.
Generation replacement/rebind переустанавливает native input epoch; teardown
отменяет deadline. Неуспешный control submit завершает session с `SV_INVALID`.

Минимальный production-core check `tests/sv/network-controls.c` прошёл на Linux
с настоящим SDL software renderer и ASan/UBSan: ordered present до соседнего
network update, stale/fresh request/quantity ack, повторные PAUSE, сохранение
queue без ответа серверу, flush 10/11, disable и partial-tail reset. Exact argv:
`.scratch/sv-c008/network-controls-command.json`; вывод:
`.scratch/sv-c008/network-controls.log`. Запуск с `ASAN_OPTIONS=detect_leaks=0`;
LeakSanitizer в sandbox недоступен из-за ptrace restrictions. Это ограничение
LSan, а не успешная проверка утечек. Полная live session/map и deferred caller
queues C011/C013/C014 этим core evidence не доказаны; full canonical runtime
PASS не заявляется. Core принят пользователем последующим решением ниже.

## Решение пользователя — 2026-11-09

[C008 и задачи 001–004 завершены и приняты](../../sv-stage-c008-acceptance.md)
в текущих границах core. Предыдущие ограничения полного runtime evidence
сохраняются как факты, но не являются gate приёмки C008. Оставшаяся работа
обязательна следующим owners:

- [C011-002](../c011/SV-C011-002-network-control-preparation.md): подготовка control owner/options и handoff contract.
- [C013-001](../c013/SV-C013-001-network-control-live-map.md): реальный map ingress и control-driven submission.
- [C014-003](../c014/SV-C014-003-network-control-lifecycle.md): actual handoff/lifecycle и deferred incoming caller queue.
- [C053-001](../c053/SV-C053-001-preference-runtime.md): настоящий macro WAIT/XWAIT confirmation consumer.

Их доработки и проверки не блокируют и не переоткрывают C008 автоматически;
evidence backlink служит трассировке. Полный live-runtime PASS не выдан, canonical
allocation/obligations/statuses не изменены. Плановые критерии выше читаются
с учётом этого более позднего явного решения пользователя.

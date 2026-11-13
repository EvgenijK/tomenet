# C008 — проверенные источники и production seams

Дата сверки: 2026-11-09. Статус: planning, без нового runtime evidence.
Перед поиском исходников выполнен Graphify query `C008 network control PKT_FLUSH
PKT_PAUSE PKT_COMMAND_CONFIRM PKT_END PKT_UNKNOWNPACKET`; результаты использованы
для навигации и проверены по workspace, включая текущие незакоммиченные изменения.
Ни код, ни canonical registry этим анализом не изменены. Строки ниже — навигация;
перед реализацией сверять актуальные тела функций.

Нормативны [CONTEXT](../../../CONTEXT.md), [архитектура](../../sv-architecture.md),
ADR [0001](../../adr/0001-separate-session-interactions-and-ui.md),
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).
Применяются SV isolation, минимальные core checks через production-путь и
[Linux-only acceptance](../../agents/platform-validation.md).

## Behavior baseline

| Outcome | Проверенный источник и контракт |
|---|---|
| `network.unknown-packet` | `src/client/nclient.c:1844` `Net_packet`, `:8240` `Send_unknownpacket`, `:7956` `Send_redraw`: неизвестный/NULL receive handler сообщает тип и последний успешно обработанный тип, создаёт диагностическое сообщение, запрашивает redraw(0), очищает весь текущий receive buffer. Длину неизвестного пакета не угадывает; соседние bytes этого буфера намеренно отброшены в рамках recovery. Wire recovery: `PKT_UNKNOWNPACKET=211` + два big-endian int32 + `PKT_REDRAW=111,0`, всего 11 bytes. `prev_type` не обновляется неизвестным или неполным пакетом |
| `network.keypress-stub` | `nclient.c:7337` `Receive_keypress`: только один byte `PKT_KEYPRESS=147`, без payload и без injected key. Комментарий в `src/common/pack.h:186` о симуляции клавиш не описывает реализованное поведение |
| `network.end-marker` | `nclient.c:1965` `Receive_end`: только один byte `PKT_END=11`. Исторический комментарий о frame loop counter не подкреплён parser; счётчики кадров не добавляются, соседний пакет сохраняется |
| `network.pause` | `nclient.c:6195` `Receive_pause`: один byte `PKT_PAUSE=58`; показать последние изменения, очистить pending keystrokes, дождаться свежего ввода через `inkey()` с обслуживанием сети, затем `Flush_queue()`. `nclient.c:1938` `Flush_queue` переносит deferred incoming caller packets из `qbuf` в receive buffer и обрабатывает их. Это не очистка исходящего сетевого буфера и не требование прекратить применение остальных сетевых обновлений |
| `network.flush` | `nclient.c:4029` `Receive_flush`: один byte `PKT_FLUSH=150`; `Term_fresh()` всегда выполняется. `disable_flush` выключает только delay. При `thin_down_flush` задержку 1 ms получают первые десять flush до сброса счётчика; без thin delay применяется ко всем. `src/client/c-init.c:3222` сбрасывает `flush_count` на проходе внешнего игрового цикла. В SV нужна семантика submission/pacing без `Term` и без блокирования сети |
| `network.confirm` | `nclient.c:7326` `Receive_confirm`: два bytes `PKT_CONFIRM=146,command`; записать confirmation для ожидающего macro owner. `src/client/c-util.c:426,508,575,665`: wait сбрасывает sentinel при входе и завершается при confirmation; до начала wait прошлое событие не переиспользуется. В SV сохраняется unsigned wire byte, occurrence/order и session ownership; redraw не повторяет доставку. Реальный macro WAIT/XWAIT runtime остаётся C053 |

## Что уже есть в SV

- `src/client/sv/protocol/protocol.c:168–184`: END/KEYPRESS — payload-free NOOP;
  PAUSE/FLUSH — decoded changes; CONFIRM ждёт целиком второй byte.
  `:240–251`: unknown recovery уже атомарно резервирует 11 output bytes,
  сохраняет предыдущий тип при backpressure и очищает receive batch после enqueue.
- `src/client/sv/app.c:482–552` `sv_app_step`: последовательный decode/apply,
  confirmation направляется input owner, PAUSE/FLUSH создают view state;
  в ожидании pause network step продолжает работу. `:360` — sequence-checked ack.
- `src/client/sv/input/input.c:3–15`: pause/ack очищают pending prompt-key ring;
  `:17–59`: begin/take/end confirmation с bounded FIFO, отдельным owner sequence,
  stale rejection и overflow. `app.c:515` без владельца confirmation отбрасывается.
- `src/client/sv/ui/flush-policy.h`: per-frame count и due time, `app.c:140–151`
  позволяют задать options/frame. `app.c:65–84` сбрасывает session state при закрытии.
- `src/client/sv/input/native-input.c:9–25`: native input epoch сейчас задаётся
  на начало сессии, а не на каждый PAUSE; физический ввод делегируется app.
- `tests/sv/contact-control.c:7,57,167` уже содержит production checks
  END/KEYPRESS/ordered CONFIRM, pause + interleaved server flags и flush counters.
  Наличие этих исходников не означает нового PASS: в данном planning запусков нет.

## Существенные пробелы для реализации

1. `flush_due_ms`, `flush_sequence`, `sv_app_frame` не имеют native runtime
   consumer в `src/` вне app API. Счётчик/срок сами по себе не доказывают submit
   pending visual changes. Обработка нескольких FLUSH в одном step budget может
   оставить только последний view state; сохранять нужные промежуточные эффекты
   явным seam исполнения, не полагаться на redraw revision.
2. PAUSE не устанавливает SDL event boundary. Событие, накопленное до паузы,
   но после начала сессии, сейчас может подтвердить её через `sv_app_physical`.
   Нужен boundary свежего физического ввода и защита от tail/повтора того же gesture.
3. В `native-input.c:26–40` text input сначала уходит pending request/quantity
   owner; при одновременно активной паузе такой ввод возвращает BUSY вместо ack.
   Ветвь PAUSE должна иметь согласованный приоритет над вложенным interaction,
   сохраняя его состояние и сетевое обслуживание.
4. `sv_input_pause` чистит prompt ring, но `SvApp` также имеет pending command,
   keymap prefix и quantity state. Технически определить ownership: какой уже
   pending ввод очищается как keystrokes, а какое interaction state сохраняется.
   Не сбрасывать весь серверный запрос или очередь исходящих пакетов по аналогии
   с названием legacy `Flush_queue`.
5. Unknown recovery пока создаёт wire reply и `SV_RECOVERED`, без видимой
   диагностики, аналогичной baseline `c_msg_format`. Связать с текущим semantic
   diagnostics/message seam; не тянуть legacy message/Term dependencies.
6. Реальный игровой native runtime и deferred caller queues ещё относятся к
   поздним session/request/macro этапам. C008 должен дать production control seam
   и измеримый собственный результат; evidence позднего caller сохраняет owner.
   Core integration не объявляется доказательством полной live-сессии.
7. `src/client/sv/protocol/login.c:399–403` использует END как признак завершения
   startup play stream. Этот отдельный login lifecycle нельзя механически менять
   на gameplay NOOP, перенося код C008; frozen B не переоткрывается.

## Вопросы и границы решения

Baseline не требует продуктового решения о packet layout или поведении KEYPRESS/
END/unknown: ответы есть в исходниках. Способ bounded delivery, presentation barrier,
SDL freshness boundary, reset cadence и ownership очистки — технические решения
реализации в рамках принятых ADR; их следует проверить минимальными production
core cases, а не перекладывать на пользователя.

Для приёмки важно явно выбрать и описать границу результата: реализация control
core сейчас и реальные late callers в своих этапах, либо также live session
integration сейчас. Второй вариант расширяет этап за текущий session runtime.
Planning должен оставить late callers и canonical obligations видимыми, не менять
allocation и не присваивать им PASS по одному decoder/queue test.

## Минимальные проверки по риску

Предпочесть небольшой дополняющий набор к существующему production
`tests/sv/contact-control.c`: split CONFIRM + sentinel без partial effects;
END/KEYPRESS + соседний пакет без invented output; exact unknown recovery при
backpressure с сохранением input до успешного enqueue; stale generation/owner и
no redraw replay; PAUSE с накопленным SDL input и вложенным request; FLUSH с
проверяемым production submit и обоими options. Linux build и минимальный
production-core набор; широкие matrix/внешние suites без отдельного решения
пользователя не добавлять. Реализация только для тестового harness недопустима.

## Последующее решение — 2026-11-09

Плановый вопрос о границе приёмки разрешён пользователем: [C008 принят в текущих
границах core](../../sv-stage-c008-acceptance.md). Поздние интеграции обязательны
задачам C011-002/C013-001/C014-003/C053-001 и не блокируют завершение C008.
Source audit выше сохраняется как история планирования, а не новый gate приёмки.

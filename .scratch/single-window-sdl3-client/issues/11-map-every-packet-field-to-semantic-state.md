# Map every packet field to semantic state

Type: research
Status: resolved
Assignee: codex
Blocked by: 01

## Question

Как каждый входящий packet field, version gate и server flag отображается в semantic presentation state: каковы update/clear lifetime, error/abort behavior, outgoing response и lossless raw fallback для обработчиков, которые сегодня рисуют напрямую или хранят только форматированный текст?

## Answer

Полный receive path разложен на 117 именованных dispatch registrations: 114 non-NULL bindings и три намеренных NULL entry, поверх отдельной инициализации всех 256 slots в NULL. Два handler aliases и все version/compile variants дают 224 атомарные строки wire schema для 112 уникальных активных handlers. Все 213 non-comment вызовов `Packet_scanf` учтены: 203 active-handler sites, восемь direct pre-play/helper sites и два disabled sites.

Decoder seam обязан выбирать wire schema по negotiated server version и compile/server gates до обновления presentation state; renderer не знает о version predicates. Общий `Packet_scanf` contract сохраняется: incomplete frame возвращает 0 без продвижения, decode failure возвращает -1 и завершает соединение, unknown/NULL type вызывает `PKT_UNKNOWNPACKET`, redraw и очистку receive buffer. Специальные per-handler validation, sentinels и abort paths перечислены отдельно.

Presentation state разделяется на пять форм с разным lifetime:

- semantic snapshot заменяется атомарно и очищается при teardown session;
- keyed collection заменяет или удаляет адресованную запись, а batch/end sentinels фиксируют границы набора;
- modal request существует до response либо abort и хранит исходный prompt/default/range;
- ordered event получает монотонную sequence identity и потребляется без coalescing одинаковых соседних событий;
- opaque/formatted payload хранится losslessly рядом с parsed view в течение lifetime owning snapshot/event.

Direct drawing, decorated strings и bounded `%s/%S/%I` нельзя восстанавливать из terminal cells или уже обрезанного buffer. До замены `Packet_scanf` нужен raw envelope с packet type, выбранным wire variant, arrival sequence и точными frame bytes; semantic decode может дополнять, но не уничтожать его. Карта, store/special lines, player list, messages, prompts, glyph attrs/codepoints и formatted names требуют такого retention там, где сервер не передаёт эквивалентную структуру.

Все 24 прямых outgoing `Send_*`/`Packet_printf` responses перечислены с форматами: acknowledgements и prompt replies являются частью атомарного transition, а не побочным эффектом UI. Privileged controls — portal reconnect credentials, injected keypresses, file writes/reloads — образуют отдельный allow-listed control channel и не должны попадать в обычный presentation event stream.

Аудит остальных 97 packet IDs доказал направление: 83 client→server-only, три direct server→client handshake/login, три shared status/sentinel, семь dead/stale и один unsupported server→client `PKT_LEAVE` без живых callers. Живых server→client packets вне `Receive_init` и pre-play path не найдено.

Полный field-level отчёт, lifecycle/sentinel rules, response ledger и 660 проверенных source citations: [`docs/research/single-window-packet-state.md`](../../../docs/research/single-window-packet-state.md). Он относится к текущему dirty checkout; отдельная research branch не создавалась, чтобы не менять общую ветку.

Выявленные решения распределены по существующим тикетам: [Choose the presentation-state boundary](02-choose-presentation-state-boundary.md) получает raw envelope, collections, ordered events и privileged controls; [Set the compatible protocol-extension policy](06-set-compatible-protocol-extension-policy.md) — dormant/asymmetric endpoints; [Audit platform deltas and packaging](15-audit-platform-deltas-and-packaging.md) — `PKT_SANITY`/`SHOW_SANITY`; [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md) — per-variant replay, batch sentinels, event ordering и allow-list checks.

# Choose the presentation-state boundary

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 01, 11, 12

## Question

Где проходит глубокая граница между повторно используемым ядром TomeNET и новым presentation state: какие semantic snapshots, keyed collections, modal requests, ordered events, privileged controls и lossless raw/formatted envelopes она предоставляет UI; как задаёт batch/delete sentinels и event ordering; как исключает зависимость конечной архитектуры от `Term`; и как позволяет временный terminal fallback с гарантированным удалением?

## Answer

Итог подтверждён пользователем после обсуждения Q1–Q8. Seam нового UI — собственный interface глубокого Session presentation model, а не `Term`, wire packets или прямой доступ к legacy globals.

### Ownership и interface

Переиспользуемое ядро сохраняет transport, negotiated-version decoding, command/macro semantics и side effects. Model принимает decoded changes и владеет session-derived presentation state и его lifecycle; UI читает согласованные read-only snapshots/views и отдельные ordered events. Обратный маршрут UI actions проходит через отдельный command interface. Конкретные C-сигнатуры и техника хранения — детали реализации в рамках этого контракта.

Внутри предпочтительно переиспользовать подходящие существующие структуры и массивы. Собственный interface не требует новой структуры или второй копии HP/inventory. Изменения имеют единый путь и определённого владельца; нельзя создавать независимо изменяемые источники истины. Новое хранение добавляется для данных, которых legacy structures не сохраняют. Read-only snapshot не требует полной копии всех данных каждый кадр, но исключает mutation под читающим UI.

Model предоставляет semantic snapshots (player/status/session), keyed collections (inventory/equipment/store/maps), modal requests (точные ID/type/prompt/default/bounds), ordered events и lossless formatted representations. Field-level правила нормативны в [Map every packet field to semantic state](11-map-every-packet-field-to-semantic-state.md); formatted documents и special-store canvases определены в [Inventory formatted and server-driven surfaces](12-inventory-formatted-and-server-driven-surfaces.md). Parsed projections не заменяют исходное formatted content.

### State lifetime

Session model хранит смысл и состояние взаимодействия: pending requests, current target, collection progress, полученные документы и server-driven position. Пересоздание UI не отменяет их. View state хранит способ отображения: layout, tabs, hover, tooltips и pixel scroll. Logical input context, unfinished input и focus, влияющий на command/macro routing, не считаются сбрасываемой визуальной подсветкой; точное владение уточняет [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md). Persistence view preferences уточняет [Define settings and migration boundary](05-define-settings-and-migration-boundary.md).

Active model удерживает необходимые текущие данные и lossless content на lifetime owning snapshots/events. Полная историческая запись отделена от рабочей модели и включается явно. Необходимость recorder и каждой истории, retention/compaction и защита чувствительных данных определяются в [Assess history retention and recorder necessity](19-assess-history-retention-and-recorder-necessity.md); обязательный recorder и бесконечный canvas operation log здесь не утверждены.

Последующий нормативный выбор retention/capture и consumer lifecycle зафиксирован в [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md#answer); он разрешает оставленные здесь вопросы, а упоминание исторической записи выше не требует её реализации.

### Decode, ordering и publication

Неполный packet остаётся ждать продолжения в receive buffer; до полного decode конкретного packet model не изменяется. Составные decoders сначала собирают временный результат, затем применяют его: rollback read pointer не заменяет rollback side effects. Decode errors/unknown packets сохраняют protocol failure contract из packet inventory.

Каждый полностью decoded update применяется без искусственной задержки, в исходном порядке и без ожидания unrelated packets. UI читает согласованную revision вне промежуточной mutation связанных полей. Collection update/delete/clear/end сохраняют конкретные реальные protocol sentinels; завершённость набора явна, несуществующие server transactions не придумываются. Незавершённый collection batch не блокирует HP/map updates. Events получают отдельную sequence identity и не объединяются из-за одинакового содержимого; teardown/relogin отделяет старые requests/events от новой session identity.

Приоритет repaint отделён от применения данных. Изменения HP и игрового viewport должны отображаться при ближайшей возможности; менее срочные views допускают задержку, но commands используют актуальные данные. Полная urgency matrix, coalescing только state projections, starvation limits и измеримые latency budgets определяются в [Classify display urgency and latency budgets](20-classify-display-urgency-and-latency-budgets.md).

### Privileged controls и migration

Server-directed reconnect, file writes/reloads и injected input выполняются и проверяются ядром через отдельный контролируемый allow-listed interface. Server-driven controls не обязаны проходить через model; она получает только необходимые безопасные UI-статусы/результаты с причинным ordering. Credentials и произвольные file payloads не попадают в presentation event stream. Это заменяет первоначальное предложение выдавать все control requests из model.

Terminal fallback изолирован в development-only adapter внутри одного системного окна. Для каждой временной legacy surface фиксируются replacement и критерий удаления. Завершённые model/UI не читают `Term`, не зависят от его buffers/hooks/screen stack и не воспроизводят virtual Terms; финальная сборка запрещает подключение fallback. Permanent formatted document/canvas rendering не является terminal fallback.

Исследование истории и решение о latency созданы как отдельные child tickets, с зависимостями и блокированием acceptance; они не мешают закрыть согласованный interface. Реализация клиента в этой сессии не выполнялась.

## Comments

### Согласованные положения — 2026-09-13

- Q1: Session presentation model предоставляет собственный interface между данными и UI; UI не читает legacy globals или `Term` напрямую. Transport, version-aware decoding, command/macro semantics и side effects остаются в переиспользуемом ядре.
- Уточнение пользователя: предпочтительно переиспользовать подходящие готовые структуры для хранения данных внутри model. Собственный interface не требует новой структуры или второй копии каждого значения; новое хранение добавляется для недостающих данных. Ownership и update contract должны исключать независимо изменяемые копии одного факта.
- Q2: UI получает текущее согласованное состояние через собственный read-only snapshot/view interface и отдельно ordered events. Готовые структуры хранения переиспользуются внутри model; snapshot contract не требует полной копии данных каждый кадр. Изменения применяются установленным путём, UI actions идут через отдельный command interface. Конкретные C-сигнатуры и механизм snapshot publication пока не фиксируются.
- Q3: Session state хранит смысл и состояние взаимодействия, необходимые для правильного продолжения сессии; view state хранит способ отображения. Пересоздание UI не отменяет pending requests, не теряет current target, batch progress или server-driven document state. Layout, hover, tooltips и pixel scroll принадлежат UI; persistence layout обсуждается отдельно. Logical input context, unfinished input и focus, влияющий на macro/command routing, нельзя сбрасывать как визуальную подсветку; их точное владение согласуется в Preserve input and macro semantics.
- Q4: согласованы два уровня — active model хранит необходимые текущие данные и lossless content owning snapshots/events; полный исторический поток отделён от рабочей модели и записывается только при включённой записи. По просьбе пользователя необходимость каждого вида истории и самого recorder, lifetime, освобождение и compaction исследуются отдельно в Assess history retention and recorder necessity. Согласование двух уровней само по себе не требует реализовать recorder или бесконечный canvas operation log.
- Q5: privileged operations (server-directed reconnect, file writes/reloads, injected input) проверяются и выполняются ядром через отдельный контролируемый allow-listed interface. Server-driven controls не обязаны проходить через presentation model: она получает только необходимые безопасные UI-статусы и результаты, с сохранением причинного ordering. Credentials и произвольные file payloads не помещаются в presentation event stream. Это уточнённый вариант, заменяющий первоначальную рекомендацию выдавать все PrivilegedControlRequest из model.
- Требование пользователя к Q6: игра требует минимальной задержки отображения мгновенных боевых изменений — появления монстров рядом с игроком и уменьшения HP. Повышение уровня и изменения inventory допускают менее срочное отображение. По просьбе пользователя классификация срочности, допустимые задержки, coalescing и acceptance scenarios вынесены в Classify display urgency and latency budgets. Разделение приоритетного repaint с немедленным применением данных ещё согласуется в текущем тикете.
- Q6: подтверждено немедленное применение всех успешно decoded changes без искусственной задержки и в исходном порядке; откладываться может только отображение. Классификация срочности и latency budgets определяются отдельно в Classify display urgency and latency budgets.
- Q7: terminal fallback изолируется в development-only adapter, не формирует interface завершённого model/UI и не переносит модель virtual Terms. Для каждой временной legacy surface фиксируются замена и критерий удаления adapter; финальная сборка запрещает подключение fallback.
- Q8: каждый packet полностью декодируется во временные данные до применения изменений; incomplete packet остаётся в receive buffer без изменения опубликованного model state. После полного decode update применяется без ожидания unrelated packets. UI читает согласованную revision вне промежуточной mutation связанных полей. Collection update/delete/clear/end сохраняют реальные protocol sentinels; незавершённость набора явна и не блокирует HP/map updates. Events имеют отдельную identity и сохраняют порядок; session teardown/relogin исключает попадание старых requests/events в новую session identity.
- Пользователь подтвердил итоговое общее понимание Q1–Q8 и разрешил закрыть тикет; каноническая резолюция записана в Answer выше.

# Classify display urgency and latency budgets

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 02, 03, 11, 13

## Question

Какие атомарные изменения behavior baseline должны отображаться при ближайшей возможности, а какие допускают отложенный repaint, и каковы проверяемые latency budgets каждого класса? Сопоставить packet/state changes с пользовательскими outcomes: HP, игровой viewport и новые монстры, target/status effects, messages/alerts, pending requests/abort, inventory/equipment, level/XP, stores и остальные surfaces. Учесть доступность семантической классификации при текущем протоколе; не предполагать, что glyph update надёжно сообщает тип или опасность монстра. Определить максимальную задержку от успешного decode до видимого результата, допустимый coalescing только для state projections, starvation limits, поведение под overlays, нагрузкой и длительными animations, согласованность одновременно видимых связанных views и feedback при действиях пользователя. Приоритет rendering не должен откладывать применение данных, менять protocol/event ordering или позволять командам использовать устаревшее состояние. «Мгновенно» означает ограниченную минимальную задержку, а не физически нулевое время. Результат — согласованная urgency/latency matrix и acceptance scenarios для Linux и Windows, не реализация scheduler.


## Comments

### Round 1 — confirmed by user

Пользователь ответил «да» на Q1–Q4.

- Q1: срочные — любые изменения игрового viewport, HP/MP, боевые статусы, target, alerts, появление/отмена запроса и завершение сессии; интерактивные — ввод текста, feedback действия, открытие surface и обновление видимого inventory/equipment/store/document; фоновые — level/XP, неактивные списки и дополнительные информационные панели. Фоновые данные, от которых зависит открытое взаимодействие, повышаются до интерактивного класса. Все viewport updates срочные без предположения о семантическом распознавании монстра по glyph.
- Q2: отдельно измеряются успешный decode → передача готового кадра на показ (обязательный бюджет клиента) и decode → наблюдаемый результат на экране (проверка на согласованных Linux/Windows конфигурациях). Численные бюджеты ещё не согласованы.
- Q3: промежуточные state projections допускают coalescing до последнего согласованного состояния (пример HP 100 → 40 → 70 не требует показа 40). Messages, alerts и effects сохраняют occurrences и порядок; одновременно видимые связанные элементы обновляются согласованно.
- Q4: длительная анимация не задерживает HP, обработку отмены и системное завершение. Скрытые surfaces получают актуальное состояние и показывают его при открытии; карта и критический HUD обновляются под обычными overlays.

Тикет остаётся claimed: следующие вопросы — численные budgets, starvation/deadline semantics, нагрузка и условия невидимого/неактивного системного окна. Итоговое общее понимание ещё не подтверждено.


### Round 2 — confirmed by user

Пользователь ответил «принимаю» на Q5–Q8.

- Q5: верхние бюджеты до передачи кадра / видимого результата: срочный 20/50 мс, интерактивный 50/100 мс, фоновый 200/250 мс. Это требования к будущей реализации, не измеренные результаты. Для локального input отсчёт от получения ввода клиентом; ожидание server response отдельно.
- Q6: deadline от первого ещё не показанного изменения; новые coalescible changes обновляют latest state без перезапуска таймера, повышение срочности только приближает deadline. Отдельные events имеют собственные сроки и порядок.
- Q7: budgets обязательны в согласованной acceptance-конфигурации при packet bursts, scroll, resize и длительных store animations. При перегрузке сокращается необязательная visual work без потери обязательных events/behavior. Превышения фиксируются как нарушения, а не скрываются средними. Машины и интенсивность нагрузки назначает acceptance ticket.
- Q8: отсутствие focus не снимает budgets с видимого окна. Для minimized window физический показ приостанавливается, network/state/обязательные effects продолжаются; после restore latest frame передаётся в пределах urgent budget без replay устаревших кадров.

Остаются уточнения классификации ordinary/opaque messages и согласованности связанных views/первого кадра открываемой surface. Тикет не закрыт.


### Round 3 — partial confirmation and clarification

- Q10: пользователь подтвердил согласованную revision для связанных видимых views с повышением до наиболее срочного класса группы без ожидания будущих packets; opening hidden surface использует latest state в interactive budget 50/100 мс, restore minimized system window — urgent 20/50 мс.
- Q9: пользователь спросил, откуда берутся «неизвестные сообщения». Решение о generic message urgency не принято. Термин уточняется: это valid PKT_MESSAGE formatted text без полного typed severity/outcome contract, а не unknown packet, corrupted data или отдельный источник сообщений. Source: Receive_message (src/client/nclient.c:3339), Send_message (src/server/nserver.c:7918), routing markers (src/client/c-util.c:4825).


### Q9 clarification — confirmed by user

Пользователь принял уточнённое правило: явно отмеченный чат — interactive 50/100 мс; остальные server messages — urgent 20/50 мс без анализа текста для понижения приоритета. «Неизвестные сообщения» не являются отдельной категорией: valid PKT_MESSAGE несёт formatted text/routing markers без полного typed severity contract. Unknown packet сохраняет protocol failure semantics.

## Answer

**Acceptance amendment, 2026-09-20:** пользователь изменил measurement gates в [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md#q21--presentation-timing-gate-revised-2026-09-20). Submission budgets 20/50/200 ms остаются обязательными автоматическими gates; visible 50/100/250 ms — targets с ручной проверкой отклика, без обязательного точного измерения. Intensive-load и длительные soak-прогоны исключены; остаются короткие functional concurrent scenarios. Ниже прежние формулировки про обязательное измерение visible limits и stress acceptance superseded этим amendment; urgency/ordering/deadline semantics сохраняются.

Пользователь подтвердил итоговое общее понимание Q1–Q10 и закрытие тикета 2026-09-15. Ниже каноническая согласованная резолюция, включая уточнение Q9. Это planning requirements; implementation и runtime acceptance не выполнялись.

### Urgency и budgets

Приоритет repaint не задерживает применение успешно decoded updates, не меняет protocol/event ordering и не разрешает commands использовать устаревшее model state. Мгновенное отображение означает ограниченную задержку.

| Класс | До передачи готового кадра на показ | До наблюдаемого результата на экране |
|---|---:|---:|
| Urgent | ≤20 мс | ≤50 мс |
| Interactive | ≤50 мс | ≤100 мс |
| Background | ≤200 мс | ≤250 мс |

Первая граница — обязательный client budget; вторая проверяется на согласованных Linux/Windows acceptance configurations. Числа являются требованиями, не заявлением об измеренной достижимости. Начало отсчёта для server data — successful complete decode; для local input — получение ввода клиентом. Ожидание server response отдельно от client repaint latency. Для событий восстановления/открытия начало — соответствующий local transition.

### Outcome matrix

Классификация применяется к атомарному outcome, а не ко всему packet независимо от его полей. Version/build gates, sentinels и lossless requirements наследуются из [Map every packet field to semantic state](11-map-every-packet-field-to-semantic-state.md) и [Inventory formatted and server-driven surfaces](12-inventory-formatted-and-server-driven-surfaces.md).

| Outcome / state projection | Класс и условия |
|---|---|
| Любое изменение игрового viewport: cell/row, visibility, положение карты и связанные overlays | Urgent; glyph не используется для оценки типа/опасности монстра |
| HP/MP и связанные индикаторы, боевые статусы, target state/description/marker | Urgent |
| Alerts, появление/отмена pending request, abort interaction и завершение session | Urgent; актуальный input context меняется по baseline независимо от repaint |
| Явно отмеченный чат | Interactive; отдельный alert outcome по-прежнему Urgent |
| Остальные server messages, включая formatted text без typed severity | Urgent; без эвристического анализа текста для понижения приоритета |
| Local input draft, feedback action, navigation/opening surface | Interactive; urgent outcomes действия сохраняют более высокий приоритет |
| Видимый inventory/equipment/store, server document page/position и local guide/lore/document | Interactive |
| Special-store canvas writes/clears/animation presentation | Interactive для магазинной surface; animation behavior/order сохраняются, остальные updates не блокируются |
| Level/XP, неактивные списки, дополнительные informational panels и остальные информационные projections | Background; данные открытого взаимодействия повышаются до Interactive, urgent outcome всегда Urgent |
| Связанные одновременно видимые views | Общая согласованная model revision и наиболее срочный класс группы; без ожидания будущих packets или выдуманных server transactions |
| Первый кадр вновь открытой скрытой surface | Latest state, Interactive 50/100 мс |
| Первый кадр после восстановления minimized system window | Latest state, Urgent 20/50 мс |

Пример packet mapping: HP/MP → urgent indicators плюс отдельные alerts; CHAR/LINE_INFO → urgent viewport projections; TARGET_INFO → urgent target; REQUEST/ITEM/SPELL/DIRECTION/abort → urgent request/context transition; MESSAGE → class по routing/outcome выше; INVEN/EQUIP/STORE/SPECIAL_LINE → visible interactive content; EXPERIENCE → background level/XP с promotion по зависимости открытого interaction. Control/transport metadata без visual outcome не требует фиктивного repaint; ядро выполняет обязательные side effects без ожидания visual budget.

### Coalescing, ordering и starvation

State projections могут объединяться до последнего согласованного состояния: HP 100 → 40 → 70 до кадра допускает показ только 70. Старейшее ещё не представленное изменение открывает deadline; последующие changes не сдвигают его вперёд. Повышение класса только приближает deadline. Кадр показывает актуальный projection и согласованные связанные views.

Messages, alerts и effects сохраняют отдельные occurrences, порядок и собственные сроки; coalescing state не означает удаление событий. Это не требует отдельного кадра для каждого сообщения или каждого промежуточного значения. Event lifetime, necessary consumers/overflow и compaction определяются в [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md).

Длительная анимация не задерживает обработку отмены, системного завершения, HP или viewport. Восстановление UI не повторяет commands/replies/consumed effects и не проигрывает устаревшие кадры. Под нагрузкой сначала сокращается необязательная visual work с сохранением baseline behavior и необходимых effects; нарушения бюджета фиксируются, а средние значения их не скрывают.

Последующее исключение для восстановления transient special-store animation определено в [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md#answer). Оно уточняет recreation outcome, не отменяя budgets, обязательную delivery и обычный animation behavior.

### Visibility и нагрузка

Карта и критический HUD обновляются под обычными overlays. Скрытые logical surfaces получают актуальное state, но не требуют рисования невидимых пикселей; при открытии первый кадр сразу актуален. Отсутствие focus не снимает budgets с видимого system window.

Для minimized window физический показ приостановлен; network/model/обязательные effects продолжаются. Restore использует latest state с urgent budget без накопленного frame replay. Остальные lifecycle/cancel/relogin правила остаются baseline.

Budgets должны выполняться одновременно с packet bursts, document scroll, resize и длительными store animations в acceptance configuration. Машины, compositor/display conditions, размеры/нагрузки и измерительный harness назначает [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md); Linux amd64 и Windows10/11 i686 остаются обязательными targets. Здесь не утверждаются универсальные hardware гарантии или численные memory/render resource budgets.

### Acceptance outcomes

- HP/MP, появление map cells/монстров, target и status updates укладываются в urgent limits во время packet bursts и магазинной анимации; срочность viewport не зависит от распознавания glyph.
- Непрерывные coalescible updates не перезапускают deadline; background view не голодает, promotion приближает deadline, связанные индикаторы используют согласованную revision.
- HP 100 → 40 → 70 допускает последний state; одинаковые messages/alerts сохраняют occurrences/order. Чат и non-chat messages имеют утверждённые разные budgets без текстовых severity guesses.
- Pending request/abort/system termination актуализируют context без ожидания animation/repaint; input feedback имеет local timestamp, server wait не выдаётся за client latency.
- Partial document/collection batch не задерживает unrelated HP/map updates. Open hidden surface, UI recreation, overlays и resize не показывают stale state и не повторяют effects/actions.
- Visible unfocused window сохраняет budgets; minimized state продолжает network/model/effects, restore даёт актуальный urgent frame без старого frame replay.
- Обе временные границы проверяются раздельно; каждый обнаруженный over-budget interval сохраняется в evidence. Целевые конфигурации/нагрузки и release gates выбирает acceptance ticket; runtime results пока отсутствуют.

### Scope и followups

Scheduler, rendering implementation и release execution не входят в карту. Решение задаёт latency requirements, а не закрывает численный memory/render resource-budget fog. Terminal fallback removal fog без изменений. Новых самостоятельных decision tickets не требуется: measurement/environment/load и evidence gates уже входят в acceptance, а consumers/overflow — в retention/capture ticket.


### Final confirmation — 2026-09-15

Пользователь ответил «подтверждаю» на итоговую резолюцию Q1–Q10 и закрытие тикета. Status изменён на resolved; контекст добавлен в Decisions so far карты, acceptance связан с нормативной matrix.

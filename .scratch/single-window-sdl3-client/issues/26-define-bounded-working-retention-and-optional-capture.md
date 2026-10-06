# Define bounded working retention and optional capture

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 02, 04, 05, 11, 12, 19

## Question

Использовать [History retention and recorder necessity: source-backed assessment](../research/history-retention-and-recorder-necessity.md), отделяя source facts и proposed minimum от утверждённой политики. Какой контракт bounded working retention и optional capture нормативен для завершённого tomenet-sv?

Согласовать владельцев и lifecycle для message recall, input recall, pending requests/results, lossless envelopes, formatted pages и special-store canvas. Учитывать baseline сохранения полученных сообщений через relog отдельно от сброса input recall и invalidation старых session events; повторяющиеся сообщения остаются occurrences, input recall имеет собственную dedup-семантику. Определить освобождение при close/replacement/abort/relogin и стабильность read views при eviction.

Назначить необходимые ordered-event consumers, registration/recreation/acknowledgement и low-watermark правила; выбрать поведение stalled consumer/overflow без молчаливой потери обязательных effects и без бесконечного удержания. Определить, какие views восстанавливаются из snapshots, а какие обязаны завершить delivery; не блокировать немедленное применение боевых updates ради необязательного diagnostic consumer.

Для logical canvas задать доказуемые условия checkpoint/compaction, сохранение raw text/attrs/coordinates/images/clear semantics и active animation progress; определить owner/reset/retention неизвестных операций, для которых эквивалентность checkpoint ещё не доказана. Checkpoint не является screenshot или Term matrix; восстановление не повторяет уже выполненные sounds/replies и завершённые animations. Определить lossless-envelope release без blanket discard после bounded decode.

Выбрать fixtures-only, test-only bounded capture, optional shipped capture либо обоснованную более полную запись. Зафиксировать recorder-off/failure поведение и границу sensitive-data capture; user macro recording, существующие exports/notes/bookmarks остаются независимыми capabilities. Не утверждать сохранение credentials/private prompt/file payloads по одному диагностическому назначению. Credential mechanism остаётся в [Define credential storage policy](24-define-credential-storage-policy.md); точные disk paths/defaults/import/deletion и file ownership — в [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md).

Передать проверяемые outcomes и выбранный capture scope в [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md), где выбираются harnesses, determinism/evidence и release gates. Конкретные измеренные memory/render budgets остаются fog до интерфейсов и harness; этот тикет задаёт boundedness и overflow contract, не выдумывает числа и не реализует recorder.

## Comments

### Round 1 — awaiting user decisions, 2026-09-18

Тикет взят в работу; применяются grilling, domain-modeling и codebase-design. Ниже предложения первого раунда, не согласованная policy:

- Q1 — диагностическая запись: рекомендованы synthetic fixtures и ограниченный capture только в тестовой/диагностической сборке, без shipped recorder. User macro recording и штатные exports остаются независимыми. Sensitive-data scope требует отдельного уточнения после выбора capture.
- Q2 — message/input recall: рекомендовано сохранить baseline limits, occurrence semantics сообщений, dedup input recall и relog lifecycle; старые сообщения доступны после relog без повторного delivery effects. Disk paths/ownership остаются в persistence ticket.
- Q3 — текущее взаимодействие: рекомендовано хранить lossless current documents/canvas, drafts и pending requests через UI recreation, освобождая заменённые данные и завершённые owners после обязательного delivery; архив прошлых страниц, закрытых stores и завершённых запросов не добавлять. Полный raw envelope освобождается только при сохранности необходимых lossless полей/provenance у surviving owner.
- Q4 — hard overflow: рекомендовано сначала освободить доказуемо ненужное и отключить необязательную диагностику; если обязательные данные всё равно нельзя сохранить/обработать, явно завершить затронутое взаимодействие при наличии корректного baseline exit, иначе соединение. Молчаливый discard обязательных данных, бесконечный рост и indefinite network pause не предлагаются. Это аварийный отказ, не штатный режим и не успешное прохождение acceptance.

После ответов уточняются named consumers/ack/recreation, stable read views при eviction, canvas checkpoint/unknown-operation правила и выбранный capture contract. Решение и закрытие тикета пока не утверждены.

### Q1 — confirmed by user

После объяснения вариантов простыми словами пользователь выбрал «вариант 1»: fixtures-only, заранее подготовленные тестовые данные и сценарии без диагностической записи игровых сессий. Не предусматривается session recorder ни в обычной, ни в специальной тестовой сборке; предложение test-only bounded capture из первого раунда отклонено. Рабочее хранение данных, история сообщений/ввода, пользовательская запись макросов и существующие exports/notes/bookmarks остаются самостоятельными capabilities.

Acceptance должен опираться на подготовленные fixtures и проверки outcomes; этот выбор сам по себе не задаёт harness и не утверждает runtime parity. В дальнейших вопросах нет diagnostic consumer, удерживающего события, и нет recorder buffer, который можно отключить для устранения overflow. Q2–Q4 и подробные working-retention правила ещё не согласованы; тикет остаётся claimed.

### Q2 — confirmed by user

Пользователь ответил «2 - да»: сохраняются baseline rules и существующие размеры message/input recall. Повторные полученные сообщения остаются отдельными occurrences; повторный введённый текст удаляется со старого места и становится новейшей записью. При заполнении bounded history вытесняются старые записи по baseline pointer/text-buffer rules. Message recall сохраняется через relog без повторной доставки звуков/уведомлений; input recall сохраняет baseline reset и последующую загрузку chat history, исключение private/short-limit prompts и отсутствие общего журнала всех inputs.

Для full-client baseline это четыре независимых message rings по 8192 pointer slots (до 8191 occurrences) и 262143 text bytes каждый, с возможным более ранним вытеснением из-за byte pressure; две input histories по 2000 записей с 256-byte slots. Точные disk paths/ownership остаются в persistence ticket. Q3–Q4 и подробные working-retention правила ещё не согласованы.

### Q3 — confirmed by user

Пользователь ответил «3 - да»: содержимое текущего магазина, открытая страница и незавершённый ввод сохраняются при resize/UI recreation. При настоящем закрытии, замене или завершении взаимодействия освобождаются данные, которые больше не нужны, после завершения обязательной обработки. Дополнительный архив прошлых страниц, закрытых stores и завершённых requests не создаётся; message/input recall живёт по отдельным правилам Q2.

Это подтверждение общего lifecycle, не разрешение удалять необходимые lossless данные или недоставленные обязательные effects. Детали consumer completion, read-view stability и canvas compaction ещё предстоит согласовать. Q4 остаётся открытым.

### Q4 — confirmed by user

Пользователь уточнил ответ на «4 - ок» и принял hard-overflow policy: сначала освобождается доказуемо ненужное; если обязательные данные всё равно нельзя сохранить/обработать, затронутое взаимодействие явно завершается с объяснением ошибки при наличии корректного baseline exit, иначе завершается соединение. Молчаливая потеря обязательных данных, бесконечный рост и indefinite network pause не допускаются. Recorder отсутствует согласно Q1, поэтому освобождение diagnostic capture buffer не является частью этой политики. Аварийный отказ при обычных acceptance нагрузках не считается успешным прохождением проверок.

### Round 2 — awaiting user decisions

- Q5 — delivery/UI recreation: предлагается, чтобы смена размера, скрытие и пересоздание UI не сбрасывали обработку обязательных событий. Views берут актуальное состояние; session/core owners продолжают обработку звуков, ответов и transitions в установленном порядке без повторного исполнения после UI recreation. Недоставленное удерживается до обработки либо явной отмены по lifecycle, а не до появления виджета.
- Q6 — special-store reconstruction: предлагается хранить точное текущее содержимое и состояние незавершённой анимации, освобождая старые операции только при доказанной ненужности для восстановления и после обработки их обязательных effects. Данные неизвестной операции, для которой нельзя доказать безопасное удаление, остаются до настоящего завершения owning interaction либо доказанного reset; достижение лимита обрабатывается по Q4. Восстановление UI не повторяет sounds/replies и не запускает анимацию заново.
- Q7 — history eviction while reading: предлагается сохранять позицию по identity читаемого сообщения, пока оно остаётся в истории; если оно вытеснено, переходить к самому старому оставшемуся с понятным указанием, что начало истории удалено. Открытая history view не удерживает безгранично старые данные. Read views должны быть согласованными и безопасными на время чтения/отрисовки, без бессрочного удержания прошлых revisions.

Эти предложения пока не подтверждены. После ответов требуется итоговая сверка полного контракта перед resolution.

### Q5 confirmed; Q6 amended by user

Пользователь ответил «5 - да, 6 - да, но без созарения хода анимаций»; оговорка понята как «без сохранения хода анимаций».

Q5 принят: обязательная обработка событий продолжается независимо от видимости panels; UI recreation не вызывает повторной отправки replies или повторного исполнения уже обработанных effects, views получают актуальное state.

Q6 принят с исключением: сохраняется необходимое текущее содержимое магазина, старые операции освобождаются после доказанной ненужности и обязательной обработки, still-needed unknown data удерживается до owner close/доказанного reset с overflow по Q4. Ход анимаций для восстановления не сохраняется; предложение восстанавливать active animation progress отклонено. Это не отменяет сами baseline animations. Поведение при UI recreation посреди анимации ещё нужно выбрать (Q8), после чего уточнить прежние требования к animation reconstruction в связанных решениях/acceptance.

Q7 о вытеснении просматриваемого сообщения пока без ответа. Q8: предложение при потере визуальной анимации переходить к её итоговому состоянию без повторного запуска и повторных sounds/replies, сохраняя обязательные смысловые outcomes; пользователь это ещё не подтвердил.

### Q7 and Q8 — confirmed by user

Пользователь ответил «8 - да, 7 - да». При UI recreation посреди special-store animation показывается её итог без продолжения или повторного запуска визуальной анимации; уже исполненные sounds/replies не повторяются. При eviction читаемого сообщения позиция переходит к самому старому оставшемуся с отметкой «Старые сообщения удалены»; до eviction сохраняется identity anchor. Просмотр истории не увеличивает retention limit.

### Final confirmation

Пользователь ответил «подтверждаю» на итоговый контракт Q1–Q8 и закрытие тикета. Ниже каноническая резолюция; предыдущие предложения в Comments сохраняются как история обсуждения и не заменяют Answer.

## Answer

Решение подтверждено пользователем 2026-09-18. Это planning contract; реализация клиента и runtime acceptance не выполнялись. Согласованы bounded working retention, fixtures-only evidence и исключение для восстановления store animation: её ход не сохраняется, UI recreation показывает итог.

#### Working owners and lifetime

Session presentation model предоставляет согласованные read-only views текущих данных, bounded message recall, lossless documents/canvas и ordered occurrences. Core сохраняет transport/decoded apply и privileged side effects; input router — logical context, draft и command/macro interaction. Эти роли не требуют дублирования подходящих существующих массивов. Process-level message recall отделён от session-scoped delivery identity.

Message recall сохраняет четыре baseline rings по 8192 pointer slots (до 8191 occurrences) и 262143 text bytes каждый, включая более раннее вытеснение по byte pressure и baseline admission/formatting/classification. Повторы могут разделять хранение текста, но остаются отдельными occurrences. Relog сохраняет recall без redispatch effects. Input recall сохраняет две истории по 2000 записей/256-byte slots, move-to-newest dedup, private/short-limit bypass, relog reset и штатную загрузку chat history. Новое безусловное архивирование не добавляется; disk paths и file ownership определяет persistence ticket.

Draft, pending request, causal result/reply живут до соответствующего completion/cancel/abort либо session invalidation, а не до уничтожения виджета. Ответ проходит baseline send/retry/cancel contract; UI recreation не создаёт новый ответ. Private draft не попадает в recall и освобождается/очищается по окончании нужного lifecycle. Уже принятый credential contract сохраняется.

Formatted document хранит точную текущую полученную композицию страницы и navigation/search/position state. Partial replacement следует настоящим markers и arrival order, без выдуманной server transaction. Заменённые строки/страницы освобождаются, когда больше не участвуют в текущем состоянии и обязательной обработке; true close/abort освобождает owner. Скрытие surface и UI recreation не являются close. Local guide/file/lore использует текущий authoritative resource/document и navigation state; старые revisions не архивируются, bookmarks имеют отдельную persistence.

Lossless envelope принадлежит конкретным surviving state/event/request/document/canvas owners. Полные decoded fields могут заменить frame bytes только при сохранении требуемых исходных bytes, attrs, markers, identity и wire/version/sequence provenance. Успешный bounded decode сам по себе не разрешает удалить исходное содержимое. Устаревший payload освобождается после replacement/owner close и completion всех обязательных consumers. Incomplete bytes принадлежат transport; partial model publication запрещена, unknown wire packet сохраняет baseline failure contract.

#### Required delivery and bounded queues

Необходимые роли consumers: (1) model apply/recall/document/canvas owner; (2) input/request/transition owner; (3) core effect owner для sound, alert и других обязательных occurrences. Это роли, не требование создать три потока или повторно маршрутизировать уже выполненный effect. Core privileged controls и существующие file writers сохраняют своих owners; секреты/file payloads не переносятся в обычную presentation event queue. Панели читают snapshots/recall и не становятся необходимыми consumers только потому, что открыты. Diagnostic recorder consumer отсутствует по Q1.

Необходимые consumers регистрируются до приёма предназначенных им occurrences. Каждый occurrence имеет session identity, sequence и определённых адресатов; ack означает завершённую обработку либо передачу bounded ownership получателю, гарантирующему дальнейший outcome. Перекладывание в бесконечную очередь не является освобождением. Для каждого consumer учитывается непрерывный прогресс по предназначенным ему events; release watermark разрешает удалить event только после всех необходимых acknowledgements. Независимое lossless state ownership может удерживать payload дольше queue entry.

Закрытие/пересоздание UI не снимает обязательного consumer и не сбрасывает его прогресс. Восстановимая view читает latest snapshot; effect owner продолжает с сохранённого delivery position без повтора исполненного. Consumer recreation передаёт bounded pending work и cursor действующему owner. Если корректная передача невозможна, действует failure policy, а не пропуск очереди. При true abort/close/relog явно инвалидируется только работа соответствующего lifecycle; retained message recall не становится новыми событиями следующей session. Выключенный пользователем звук обрабатывается по baseline toggle, без накопления для последующего проигрывания.

Очереди и payload ownership имеют конечные лимиты; pending mandatory events нельзя вытеснять как старую историю. Отставание обязательного consumer является ошибкой, обнаруживаемой по требованиям delivery/latency и конечной ёмкости. Сначала выполняется допустимое освобождение и обработка очереди без изменения ordering или artificial batching. Если восстановить корректную работу в пределах контрактов нельзя, применяем Q4: корректный baseline exit из затронутого interaction с объяснением, иначе прекращение connection. Нельзя продолжать заведомо неполное состояние, бесконечно наращивать память либо замораживать network ради необязательного потребителя. HP/map apply и согласованные latency budgets сохраняются; overflow под принятой обычной нагрузкой — acceptance failure.

#### Read views and eviction

Read view остаётся согласованной и безопасной на время ограниченного чтения/отрисовки; mutation/eviction не изменяет данные под читателем. Конкретная техника borrow/copy/generation — реализация, но не допускаются неограниченные leases на прошлые revisions. UI не хранит указатели на вытесненные записи между кадрами: он хранит устойчивую identity/position и заново получает актуальный view. Число/размер удержанных snapshots также bounded и входит в общий memory accounting.

History anchor остаётся на читаемом сообщении, пока оно retained. После его eviction показывается самое старое оставшееся сообщение и явная отметка об удалении старых записей. Это не меняет baseline capacities и не задерживает eviction ради открытого окна истории.

#### Canvas compaction and animation exception

Logical checkpoint сохраняет всё surviving содержимое: исходные text bytes, attrs, coordinates, image primitives/layers и результаты обычного/force clear, плюс ещё необходимые opaque payloads/provenance. Он не является screenshot или Term matrix. Known operation можно удалить только после применения, необходимого delivery и доказательства, что checkpoint восстанавливает тот же текущий смысловой результат, а будущие writes/clears ведут себя эквивалентно. Partial clear не объявляется глобальным reset; image erasure и force-clear semantics проверяются отдельно.

Still-needed unknown operation принадлежит canvas interaction и удерживается до true close/kick/abort либо reset, для которого доказано полное прекращение её влияния. Если такой compaction proof отсутствует, одни лишь repaint/перекрытие строки не разрешают discard. Конечный лимит с Q4 предотвращает бесконечный operation log. Unknown animation opcode не получает выдуманную визуальную семантику: baseline error/no-visual-effect outcome сохраняется, а raw retention следует lossless ownership.

По Q6/Q8 animation progress и промежуточные кадры не сохраняются для UI reconstruction. Обычная непрерванная анимация существует и использует временное рабочее состояние; recreation не требует его восстановления. При потере визуальной анимации view показывает её уже определённый итог и не начинает её заново. Итог не должен затирать более поздние ordered writes/clears: latest logical canvas остаётся источником истины. Не возникает повторных sounds/replies; ещё обязательные core outcomes обслуживаются своими owners независимо от визуального хвоста. Store close/session abort сохраняют baseline cancellation. Это узкое исключение для восстановления transient store animation, а не отмена постоянных weather/palette effects или нормального animation behavior.

#### Fixtures-only and evidence handoff

Session recorder/capture отсутствует и в shipped, и в test build: нет replay archive, hidden diagnostic ring или raw traffic/input journal. Тесты используют заранее подготовленные synthetic fixtures с явными initial state, source/version/features и при необходимости управляемыми time/RNG. Fixture assertions, измерения latency и штатные диагностические ошибки допустимы; они не превращаются в запись реальной игровой сессии. Full replay determinism, harness и release gates выбирает acceptance ticket.

User macro recording, message exports, screenshots, notes и bookmarks остаются отдельными baseline capabilities. Credential/private prompt/privileged file payloads не получают диагностической копии; fixtures используют synthetic данные. Отдельные recorder-off/failure scenarios не нужны для отсутствующего recorder: проверяется отсутствие session archive и ограниченность working memory при продолжительной игре.

Acceptance получает проверяемые outcomes: relog recall без redispatch; duplicate messages/input dedup и capacity/byte-pressure eviction; UI recreation с pending request без duplicate reply; hidden views и consumer recreation без потери/повтора effects; stalls/overflow с явным failure; stable views и reading-anchor eviction; lossless partial pages, envelope replacement, forced graphical clear и доказанная checkpoint equivalence; unknown-op retention/reset; mid-animation recreation сразу в final/latest canvas без повторных sounds; много page/store replacements без роста архива. Обе платформы и latency requirements сохраняются. Численные новые memory/render budgets остаются fog до interfaces/harness, baseline recall capacities уже определены.

Контекст добавлен в карту. [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md) применяет этот контракт при выборе harness/load/release gates; [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md) определяет disk paths/owners существующих personal-data capabilities без session recorder. Formatted/raster/latency решения ссылаются на принятое animation exception. [Source research](../research/history-retention-and-recorder-necessity.md) остаётся историческим evidence, его предложенные варианты не заменяют эту policy. Новых decision tickets не требуется; numeric resource-budget fog остаётся открытым.

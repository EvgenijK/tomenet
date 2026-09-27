# Decide text-field boundaries and legacy defects

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 04, 22, 24, 27

## Question

Какие observable outcomes должен давать single-window client там, где source/field inventory обнаружил небезопасные или неоднозначные byte boundaries? Использовать [Enumerate source text and server-field byte contracts](27-enumerate-source-text-and-server-field-byte-contracts.md) и его source-backed registry. Это дополнение к [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md#answer) и [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md#answer), не пересмотр согласованных chat expansion, draft-preserving encoding errors или запрета guessed network charset.

Разделить editor capacity, capacity после escaping/substitution, wire capacity вместе с NUL и server-side field truncation. Выбрать limit/reject-with-draft/safe-truncation outcomes для request strings (editor 159 против safe `%s` payload 79), remote administration scripts и остальных выявленных mismatches. Уточнить, что происходит с уже загруженным длинным default/imported value, paste и macro-generated text; согласовать retry/cancel/reply и отсутствие частичного packet send. Штатная обрезка внутри серверного обработчика не означает разрешения повреждать wire framing.

Для локальных полей также отделить payload limit от storage: skill search и autoinscription search передают editor limit 80 при destination[80]; macro-set name и stage comment допускают 20 payload bytes, которые затем копируются в поля[20]. Согласовать сохранение допустимых пользовательских значений, file compatibility и boundary outcomes без переноса writes за границу массива; blanket wire limit 79 к чисто локальным полям не применяется.

Определить disposition legacy `Packet_printf` limit без гарантии NUL и `Packet_scanf` limit без потребления остатка oversized field: безопасная ошибка/отказ и lifecycle незавершённого запроса, без изменения сервера или протокола. Отделить malformed incoming packets от допустимых legacy payloads и от incomplete packet buffering; не восстанавливать границы по догадке.

Включить конкретные receive/callback boundaries: `%s` decoder может писать 80 bytes в `Receive_playerlist` destination размером `NAME_LEN=20`; `RID_ITEM_ORDER` ограничивает вход 40 payload bytes, затем формирует строку в `str2[40]`. Для server-side случая определить, нужен ли клиентский field-specific boundary при неизменном текущем сервере, и какие legal baseline outcomes сохраняются. Само обнаружение не разрешает исправлять сервер или объявлять любое усечение совместимым.

Для editor/clipboard paths согласовать безопасные bounds относительно фактического поля и явную семантику locale/signed-char filtering. Source evidence не задаёт новое alphabet restriction, универсальную кодировку или Unicode repertoire extension; server-owned byte transforms сохраняются согласно конкретному field contract. Отдельно рассмотреть credentials-like archive/resource password editors с обычной history policy, не распространяя молча account credential policy на новые виды данных.

Разделить точное хранение raw account credential bytes и их существующий protocol transform: `my_memfrob` при `server_protocol >= 2` преобразует `*` (0x2a) в NUL, после чего C-string операции могут усечь остаток, включая сравнение подтверждения password change. Определить совместимое поведение login/import/change-password и отображаемой ошибки для такого случая без молчаливой потери draft, новых guessed encodings или одностороннего изменения wire format. Согласованное secure storage и моменты записи из [Define credential storage policy](24-define-credential-storage-policy.md#answer) остаются нормативными; успешный byte round-trip хранилища сам по себе не доказывает protocol round-trip.

Результат — конечная таблица dispositions по выявленным случаям и acceptance outcomes. Настройки profiles/mappings/defaults и storage ownership остаются в [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md); fixtures, target-build/platform proof и framing sentinels — в [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md). Исправление кода вне planning-карты.

## Comments

### Final confirmation — 2026-09-19

После объяснения последствий пользователь подтвердил оставшиеся пункты: «1 - ок, 2 - ок, 3 - ок, 4 - ок, 6 - ок, 8 - ок, 9 - ок». Q5/Q7 подтверждены ранее. Нумерация относится к последнему списку из девяти вопросов; ранние пометки о pending decisions ниже являются историей обсуждения и заменены итоговым Answer.

### Receive names and byte filtering — confirmed, 2026-09-19

В нумерации краткого списка оставшихся вопросов пользователь подтвердил Q5 и Q7: корректно переданное длинное имя в player list сохраняется полностью с исправлением размера приёмного хранения; случайное отбрасывание bytes из-за signed char/locale устраняется с сохранением конкретных field rules и server-owned transforms. Это не подтверждение универсальной сетевой кодировки или расширения server repertoire.

По Q1/Q2/Q3/Q4/Q6 пользователь запросил объяснение последствий; по Q8 — подробности password transform; по Q9 — местоположение archive password prompts. Эти решения пока не приняты.

### Input-field limit — partial decision, 2026-09-18

После объяснения Q1 и перечисления затронутых полей пользователь выбрал: «понял, лучше ограничить поле ввода». Для непосредственно server-bound editors принят предел допустимого payload конкретного поля вместо предложения позволять заведомо слишком длинный черновик и блокировать submit. В частности, общий request-string editor и remote-script editor ограничиваются 79 payload bytes, включая применимые преобразования/префиксы в итоговом wire field; более строгие field-specific ограничения рассматриваются отдельно. Это не универсальный лимит для локальных полей и не пересмотр принятого chat expansion behavior.

Ещё не согласованы: macro insertion сверх лимита; обработка уже загруженного длинного default/imported value; разделение допустимого локального resource name и ограниченного server reporting field. Выбор ограничения editor не разрешает молча обрезать автоматически сформированные packets или отправлять их частично. Остальные вопросы тикета остаются открытыми.

### Paste overflow — confirmed, 2026-09-18

Пользователь уточнил: «вставлять только помещающуюся часть, обрезать без показа уведомления». При вставке в ограниченное поле принимается только помещающийся префикс текста; остаток отбрасывается без уведомления. Вместимость учитывает текущий текст/заменяемое выделение и конкретный byte contract; обрезка не должна создавать повреждённую последовательность представления или незавершённое escaping. Это подтверждение касается вставки; предложенное поведение уже загруженных длинных значений пока не подтверждено.

## Answer

Решение согласовано в живом обсуждении 2026-09-18–19. Нормативный source/field inventory — [Source text and server-field byte contracts](../research/source-text-and-server-field-byte-contracts.md); таблица ниже задаёт dispositions, а не утверждает уже выполненные исправления или runtime proof.

| Случай | Принятое поведение | Проверяемый результат |
|---|---|---|
| Request-string editor 159 / safe wire payload 79; remote-script editor 80 / safe wire payload 79 | Ограничить поле 79 payload bytes с учётом фактического field contract, prefixes и escaping. Более строгий конкретный предел имеет приоритет. | Лишний ввод не принимается; обычная отправка содержит только допустимое значение и завершающий NUL. |
| Paste, текст макроса, уже загруженный default/imported value в ограниченном поле | Принять только помещающийся префикс, остаток обрезать без уведомления. Замена выделения учитывает освободившееся место. Принимается, что последующий Enter макроса отправит именно сокращённое значение серверу. | Обрезка относится к данным поля, не только к отображению. Не оставляет повреждённой последовательности представления или частичного escaping. Открытие поля само по себе не переписывает исходный файл. |
| Длинное имя локально выбранного font/tileset и служебные сведения о ресурсах | Полное значение сохраняется для локальной загрузки и настроек; только reporting copy для сервера сокращается до safe payload своего slot. | Ресурс продолжает работать по полному имени; сервер получает сокращённое название. Не переносить эту политику на file-transfer filenames или исполняемый текст. |
| Skill search / autoinscription search E80 при destination[80] | Обеспечить хранение минимум 81 byte, сохранив предел 80 payload bytes. | Допустимый поиск не сокращается до 79 из-за ошибки внутреннего массива. |
| Macro-set name / stage comment E20 при destination[20] | Обеспечить минимум 21 byte во всей цепочке копирования при прежнем пределе 20 payload bytes. | Сохраняются допустимые пользовательские значения и совместимые форматы файлов. |
| Outgoing packet framing и socket-room exhaustion | Проверить fields и сформировать/поставить в очередь целый пакет; нехватка места ждёт возможности постановки, не оставляет частичный пакет. Неучтённое переполнение вне согласованной editor/reporting truncation — локальный отказ отправки, не неявная обрезка в codec. | Все строковые поля имеют NUL в пределах slot, следующие поля не повреждаются. Baseline cancel/retry/reply и success semantics действуют только для действительного результата операции. |
| Incomplete / malformed incoming packet | Неполный пакет ждёт продолжения без частичного apply. Нарушение границ формата, включая отсутствие NUL в допустимом slot, завершает соединение с ошибкой протокола; границы не восстанавливаются догадкой. | Корректная фрагментация не вызывает disconnect. При malformed packet нет частично применённого состояния или выдуманного ответа незавершённому request; действует согласованный disconnect lifecycle. |
| Receive_playerlist, decoder до 80 bytes при NAME_LEN20 storage | Принять корректный wire field в достаточно большое хранение и сохранить полное значение для отображения. | Никакой обрезки имени до 19 байт ради старого массива; допустимость последующих действий с именем проверяется их собственными contracts. |
| RID_ITEM_ORDER на текущем сервере: 40-byte input / str2[40] | Конкретное поле ответа ограничить 39 payload bytes. Это клиентский обход server-side storage defect, не изменение сервера. | Длинный ответ сокращается по принятой editor policy; parse/count/item semantics, cancel и существующие допустимые короткие ответы сохраняются. Доступная длина заказа сознательно уменьшается. |
| Clipboard bounds, signed-char и locale filtering | Проверять фактическую вместимость после преобразований; устранить случайное отбрасывание bytes из-за signed char/locale, сохраняя явные правила конкретных полей и server-owned transforms. | Нет нового общего alphabet restriction, guessed charset или Unicode repertoire extension. Неизвестное Unicode↔byte соответствие по-прежнему даёт ранее согласованную draft-preserving encoding error. |
| Account credentials с `*`, server_protocol >= 2 | До отправки проверить возможность полного protocol round-trip. `*` превращается XOR42 в NUL, поэтому такой login/change-password не отправлять; показать объяснение несовместимости протокола, не посылать усечённый префикс и не терять введённый draft. Подтверждение нового пароля сравнивать по исходным bytes до transform. | Разные suffix после `*` не считаются совпавшим подтверждением. Локальный отказ смены не обновляет runtime/vault password. На protocol < 2 этот конкретный запрет не переносится. |
| Credential import / secure storage | Сохранить ранее согласованное точное хранение исходных bytes. Успешный импорт не объявляет значение пригодным для login: несовместимый protocol transform блокирует именно отправку. | Нет автоматического усечения/замены секрета. Моменты записи, приоритеты, ошибки и отсутствие plaintext fallback следуют credential policy. |
| Пароли архивов sound/music packs в меню установки | Скрытый ввод, исключение из общей input history и diagnostics, временное хранение для установки/повтора. Account-vault policy на эти пароли не распространяется. | Пароль распаковки не появляется в обычном recall; отмена/пустой ввод и retry сохраняют workflow установки. |

Пределы считаются в bytes конкретного поля, отдельно от Unicode UI, C storage и server-side semantic truncation. Для `%s/%S/%I` intended safe payload составляет 79/255/159 bytes плюс NUL в slot 80/256/160. Это не общий editor limit: purely local fields сохраняют собственные пределы; ранее принятый chat expansion с удалением неуместившегося shortcut/ограниченной обрезкой хвоста остаётся по [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md#answer). Серверные преобразования имён, quest clipping, callback-specific cancel/default/retry и семантика ответов остаются по atomic registry, кроме явно согласованного RID_ITEM_ORDER guard. Макрос теряет только избыточный текст ограниченного поля; последующие команды и control boundaries остаются по input-router contract.

### Follow-through

- [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md): полные локальные resource names и отдельная reporting copy, сохранность внешнего import source, credential raw-byte references и исключение archive passwords из обычной history. Конкретные profile/default/serialization решения остаются там.
- [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md): lengths 0/limit−1/limit/limit+1 до и после transforms, paste/selection/default/import/macro→Enter, source-file preservation, локальные 80/20-byte values, длинный playerlist field, RID_ITEM_ORDER39/40, split/oversized/chained packets с sentinel и queue exhaustion, locale/signedness, protocol branches и synthetic credential suffix collisions, archive history/diagnostics. Проверять сокращённое реальное server-bound значение, не только видимую строку.

Новых decision tickets не требуется: schema и acceptance имеют владельцев; resource-budget fog не изменился. Реализация, серверные исправления и runtime доказательства не выполнялись в этой planning-сессии.

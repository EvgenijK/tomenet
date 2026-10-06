# Define settings and migration boundary

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 01, 02, 14

## Question

Какие существующие настройки, макросы и preference-файлы новый клиент читает или импортирует, какие per-Term поля намеренно отвергает, и как отдельные имена бинарника, пользовательского хранилища и UI-конфигурации предотвращают повреждение legacy-настроек?

Использовать [Persisted settings and files: source inventory](../research/persisted-settings-and-files.md): назначить import/write owner и scope каждому config key, option и файловому семейству; выбрать snapshot/live overlay, precedence и reimport/error policy. Установить data-only import contract для includes и executable PRF records, backend trigger conversion, immediate option effects и автоматической перезаписи `.opt`/`.ins`/`.dna`; определить судьбу неизвестных и legacy window records. Согласовать credentials и personal histories import, пути чтения/записи bookmarks/chat при текущей SDL3 асимметрии, shared resource overrides и независимость нового хранилища от server file destinations. Исследовательские рекомендации не являются утверждённой политикой переноса.

## Historical discussion

Изначальное решение о независимой копии общих настроек пересмотрено пользователем при выборе storage root в Name and ship the new client across platforms. Актуальная граница находится в Answer ниже; исторические ответы не являются текущим требованием изоляции общих данных.

### Round 1 — confirmed by user

- Client profile — независимая копия, включая gameplay settings, macros и auto-inscriptions; legacy sources неизменны, автоматической синхронизации нет.
- Первый запуск предлагает explicit import с выбором источника; можно пропустить и импортировать позднее. X11/SDL3 sources автоматически не смешиваются.
- Переносятся все сохранившие смысл настройки: gameplay, macros, auto-inscriptions, birth defaults, audio, palette и совместимые font/tiles mappings. Per-Term layout/content assignments исключены; спорные visual options требуют явного semantic mapping.
- Пароль тоже переносится; способ хранения уточняется отдельно в [Define credential storage policy](24-define-credential-storage-policy.md).
- Personal data group включает chat input history, private notes и guide bookmarks; всё включено по умолчанию.

### Round 2 — confirmed by user

- Повторный legacy import запускается вручную, сохраняет текущие значения по умолчанию, показывает конфликты и позволяет явно выбрать замену. В импортируемом наборе сохраняется baseline global/class/character/form precedence.
- Только разовый legacy import является data-only: macro definitions и распознанные load-time directives сохраняются без исполнения при переносе, includes проверяются на циклы и границы выбранных источников, settings применяются после commit. Штатная preference loading внутри клиента сохраняет исходное execution/apply behavior, включая сохранённые распознанные directives; запрет исполнения при миграции не делает их навсегда неактивными. Неизвестные records остаются inert.
- Legacy import сначала разбирает данные и показывает отчёт; корректные независимые группы можно перенести, повреждённые пропустить. Связанный macro/include набор принимается целиком либо пропускается; неизвестные записи сохраняются неактивными. Source conversion ничего не перезаписывает; ошибка сохранения оставляет прежний профиль.

### Q9 — confirmed by user

- Ресурсные каталоги текущего SDL3 используются совместно для полноценной работы, включая чтение, запись audio/graphics overrides, установку/обновление паков и caches. Read-only restriction для этих ресурсных каталогов пользователь явно отверг; копирование ресурсов в независимый modern resource namespace не требуется.
- Изменения общих ресурсов видны обоим клиентам. Это исключение из независимости client profile относится к ресурсам Q9 (`xtra` sound/music/font/graphics и связанным resource configuration/cache operations), не является разрешением общей записи legacy account/gameplay/macro files или Term/UI configuration.
- Конкретные пути и SDL3 root selection учитывают выбранную установку и её пользовательский resource overlay; platform/build write behavior и совместимость общего кэша требуют evidence.

### Q10 — confirmed by user

- Штатная preference loading сохраняет SDL3 auto-conversion/save behavior: outdated `.opt`/`options.prf`, `.ins` и `.dna` автоматически записываются под тем же логическим именем в пользовательский `user/` активного клиента. Для single-window client active user storage принадлежит независимому client profile из Q1; bundled read fallback и user write override следуют baseline. Обычные macro `.prf` не получают blanket auto-resave.
- Предложение отдельной явной export-only операции для штатного auto-conversion не принято: штатное сохранение преобразования происходит автоматически. Запрет execution/source rewrite остаётся свойством разового legacy import. Shared resource destinations Q9 сохраняют отдельный совместный owner.
- Source-backed direct `.bak` rename/routing mismatch остаётся известным недоказанным backup path; acceptance должен назначить evidence/disposition, не предполагать корректность.

### Q11 — confirmed by user

- Форматы `.opt`, `.prf`, `.ins`, `.dna` и связанных macro metadata сохраняются вместе со штатной SDL3 семантикой загрузки/сохранения; новая UI configuration отдельна и версионирована.

## Answer

### Current resolution — amended by explicit user clarification

Уточнение 2026-09-20: собственные SV CFG/OPT конвертируются в памяти и записываются только по «Сохранить»; для общих INS/DNA остаётся штатная автоматическая конверсия. Meaningful legacy display preferences сохраняются и влияют на новые semantic surfaces; смена renderer сама по себе не делает их no-op. См. [актуальные решения persistence](25-specify-persistence-ownership-and-ui-configuration-schema.md#comments).

Последующее уточнение 2026-09-19: отдельные SV `.opt` также подтверждены; прежний формат и explicit import сохраняются, automatic inheritance/synchronization legacy option values не используется. Каноническая запись — «Independent OPT files» в [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md#comments).

Уточнение 2026-09-19, Q16 подтверждён: основной конфиг SV независим от SDL3 для connection server, volume, selected resources, graphics и UI. Старый конфиг импортируется только явно, затем изменения не синхронизируются; SV не записывает legacy main config и не меняет его layout/big-map preference. Fonts, tilesets, audio packs и macro files остаются общими в прежнем user root. Каноническое уточнение и точные per-file rules — [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md#comments). Нижеследующий blanket shared main-config read/write contract заменён этим решением; прочие не затронутые правила сохраняются.

Уточнение 2026-09-18: пароль исключён из общего read/write ownership. Пользователь выбрал отдельное защищённое хранилище SV и принимает несовместимость с legacy SDL3 и сохранение прежнего открытого пароля в legacy config; его устранение вне SV. Детали и каноническая credential policy — [Define credential storage policy](24-define-credential-storage-policy.md).

Используется storage root текущего SDL3: `SDL_GetPrefPath("TomenetGame", "tomenet")` или `TOMENET_SDL3_USER_PATH`. Одинаковые по смыслу настройки и пользовательские файлы переиспользуются совместно обоими клиентами для чтения и записи; только настройки, специфичные для SV, и новая UI configuration хранятся отдельно внутри общего root. User resource root текущего SDL3 общий и writable, включая packs, overrides, installation/update и caches. Уточнённое installation layout из naming/packaging ticket использует `lib/` рядом с executable: он общий при colocated placement и bundled отдельно при раздельной распаковке, без автоматического поиска других installations. Независимая snapshot-копия совместимых SDL3 настроек больше не требуется. Изменения common settings/resources видны обоим клиентам.

Штатная preference loading, execution/apply gates, formats `.opt`, `.prf`, `.ins`, `.dna` и macro metadata, а также auto-conversion/save следуют SDL3 baseline. При обычной работе общие файлы могут обновляться штатными save/conversion operations. Legacy Term layouts и назначения их contents не используются новым UI; при сохранении mixed files настройки только старого клиента должны оставаться сохранными. SV-only records не должны менять поведение старого SDL3. Точное per-key/file разделение и совместимое round-trip задаёт persistence schema ticket.

Явный разовый перенос из другого legacy-источника (например, X11 или выбранной другой установки) сохраняет data-only import contract: recognized directives не исполняются во время переноса, но сохраняют исходную семантику последующей штатной загрузки. Settings/personal data/password входят в перенос; include/path checks, preview и ручной reimport сохраняют выбранные текущие значения по умолчанию с явным разрешением конфликтов. Корректные независимые группы можно перенести, linked macro/include группа принимается целиком либо пропускается; неизвестные записи inert, save failure оставляет прежние destination data. Внешний источник переноса не перезаписывается. Уже общие SDL3 files не считаются обязательным отдельным migration source и не требуют независимого копирования.

Пароль — [Define credential storage policy](24-define-credential-storage-policy.md); общие paths/names и packaging — [Name and ship the new client across platforms](08-name-and-ship-the-new-client.md); per-key ownership, semantic mapping, отдельная UI schema, histories/bookmarks paths и write/concurrency contract — [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md); runtime/backup/resource evidence — [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md).

Пользователь подтвердил исходную границу и затем явно пересмотрел shared storage ownership. Тикет остаётся resolved с этим актуальным уточнением. Implementation и runtime acceptance не выполнялись; resource-budget и terminal-fallback fog без изменений.

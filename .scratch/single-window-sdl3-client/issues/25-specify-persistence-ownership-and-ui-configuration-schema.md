# Specify persistence ownership and UI configuration schema

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 05, 08, 19, 21, 22, 23, 24, 26, 27, 32, 35

## Question

Утверждённый итоговый документ: [Persistence contract](../persistence-contract.md). Он объединяет ответы Q1–Q40, exact path/schema mechanics и disposition overlay к полным CFG/OPT/file inventories. Финальное подтверждение и статус решения записаны в Answer.

Применить итоговый [Decide text-field boundaries and legacy defects](35-decide-text-field-boundaries-and-legacy-defects.md#answer): его Follow-through определяет последствия для resource names/reporting, import source, credential bytes и archive-password history. Не выбирать эти dispositions заново при задании schema.

Для byte-boundary outcomes, imported/default strings и history policy archive/resource passwords использовать отдельное решение [Decide text-field boundaries and legacy defects](35-decide-text-field-boundaries-and-legacy-defects.md). Source registry сам по себе не выбирает новую policy; storage schema не должна молча закреплять старые unsafe truncation/secret-history paths.

Применить [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md#answer): runtime recall/lifecycle/eviction уже согласованы; здесь остаются точные disk paths, load/save/import/ownership существующих chat history, exports, notes и bookmarks. Session recorder/capture ни в shipped, ни в test build не предусмотрен, поэтому его settings/files/deletion policy не добавляются. Не приравнивать рабочую историю к новому архиву сессий и не менять принятые recall limits или relog behavior через UI schema.

Применить исключение для credentials из [Define credential storage policy](24-define-credential-storage-policy.md#answer): registry должен исключать запись SV password в shared CFG на всех startup/login/save/conversion/shutdown путях. Существующая legacy password record сохраняется согласно решению пользователя; её cleanup вне SV. Задать collision-free identity serialization для server address + effective port + baseline account с учётом case-insensitive Windows target names и несекретных Linux attributes; exact account/host normalization и metadata references не должны объединять различные login identities или менять wire credentials. Identity/reference/import semantics берутся из credential ticket; UI удаления и новый credential exporter не добавляются. Explicit import записывает secret через выбранный OS backend, отдельно сообщает failure этой группы и не включает fallback из legacy config при обычном login. Common nick/server/fullauto metadata не являются разрешением записать SV secret в config.

Применить «PCF loader, UI scaling and resource failures» из [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md): UI scale меняет text/controls/spacing с relayout независимо от map zoom; отдельная системная text-size setting не обязательна, OS DPI обязателен. Registry должен определить range/steps/default UI scale после согласования поведения при нехватке места. При startup resource-load failure показывается ошибка и автоматически применяется fallback без диалога выбора; определить резервные resources/profiles, порядок fallback, поведение при отказе самого fallback и persistence requested/effective selection. Live-change failure сохраняет предыдущий рабочий набор.

Применить «Independent PCF sampling» из [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md): отдельный от tiles PCF filter с Nearest / Linear / PixelArt и уже принятым default Nearest. Определить scope, key, storage/import и requested/effective presentation; PCF PixelArt использует тот же явный Nearest fallback при отсутствии реальной поддержки. Не подменять успешно загруженный выбранный PCF другим font asset ради масштаба/вида и не связывать его filter с graphics filter неявно; исключение при startup load failure определено выше.

Применить принятые fit zoom, «Prepared assets and 1:1 composition» и «SDL filters and mask sampling» из [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md): определить setting/default/range/steps/storage для user zoom до 100% maximum fit и filter settings. Итоговый cell raster size и cache generation — вычисляемые значения, не legacy Term geometry. По принятому filter contract задать отдельные requested/effective значения и отображение backend fallback; определить явное преобразование imported legacy Lanczos без изменения shared legacy settings ради SV-only supported modes. Nearest/Linear импортировать по смыслу, не по предполагаемому совпадению численных enum; окончательные mappings/defaults и provenance фиксируются здесь, визуальная policy остаётся у raster ticket.

Layout registry follow-up from [Specify surface layouts and responsive rules](21-specify-surface-layouts-and-responsive-rules.md): wide/small preset persistence; panel widths and allowed constraints/defaults; player-selected block visibility; five-line live-feed configuration; ephemeral once-per-launch small-layout offer. Explicit small/wide transition switches effective normal/big_map. Уточнить startup/loading precedence и запись shared `CO_BIGMAP` относительно SV-only layout preset, чтобы implicit configuration restore не менял common settings без ясного ownership. Layer scopes/text-map profiles/metrics остаются по своим source owners; renderer legibility bounds относятся к raster/acceptance decisions, не guessed default396.

Visual role/font/map-scale/tileset ownership и explicit correspondence registry задать по [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md), с фактами из [Enumerate source text and server-field byte contracts](27-enumerate-source-text-and-server-field-byte-contracts.md). Определить defaults/precedence/resource references для independent text/map fonts и visual profiles без universal network charset; сохранить совместимые font/graphics PRF formats и common files, SV role settings отдельно. Mapping legacy Term-font preferences не превращает old Term geometry в новый UI и не запускает map definitions reload от text-font change.

Для histories и возможной diagnostics применять [History retention and recorder necessity: source-backed assessment](../research/history-retention-and-recorder-necessity.md) и [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md): runtime occurrence/delivery retention не равна disk personal history. Назначить отдельные owners/defaults/load/save/import/retention/deletion для input recall, message exports, automatically received private notes и bookmarks; учесть SDL3 load/save asymmetry и chat-export selection из important scrollback. Если diagnostic capture выбран, определить его storage/privacy boundary отдельно; recorder-off не отменяет независимо согласованную запись notes/exports/bookmarks.

Для platform roots/helper write paths применять [Platform deltas and packaging: статический аудит](../research/platform-deltas-and-packaging.md): SDL3 initialization/CWD, shared resource overlay, direct guide writes и user-path-based network fingerprint требуют явных owners и согласованной identity policy. Общая writable resource storage не означает общей записи SV-only/UI configuration.

Какой проверяемый per-key/per-file contract реализует границу из [Define settings and migration boundary](05-define-settings-and-migration-boundary.md): что принадлежит общим SDL3 settings/files/resources либо отдельным SV-only/UI settings; как каждый recognized CFG/INI/option/PRF record переносится, сохраняется, явно отвергается либо остаётся inert? Построить полный registry по [Persisted settings and files: source inventory](../research/persisted-settings-and-files.md), учитывая active-build gates и baseline core storage; не заменять атомарное соответствие общими семействами.

Для каждой записи задать scope/default/precedence, source-to-destination mapping, transforms и runtime load/write owner. Per-Term geometry/fonts/window flags отделить от сохраняемых glyph/resource choices и semantic visual preferences; mappings опираются на согласованные layouts/encoding, не придумывают новый UX. Распознанные load-time directives не исполняются при legacy migration, но сохраняют baseline behavior при последующей штатной preference loading.

Определить отдельную версионированную UI schema: формат и fields, supported scopes/defaults/version migration, неизвестные/повреждённые значения и атомарное сохранение. Существующие игровые форматы сохраняются. Указать реальные history/bookmark load/save paths и guide-update anchoring; credentials mechanism делегирован [Define credential storage policy](24-define-credential-storage-policy.md), здесь нужен только owner/reference boundary без дублирования решения.

Согласовать import preview/commit/rollback и linked-group granularity, reimport conflict identity для settings/macros/history/notes/bookmarks, missing includes/resource policy и неизменность внешнего migration source и сохранность legacy-only records при штатной записи общих mixed files. Для явно общих writable SDL3 resource directories выбрать root selection, pack/config/cache compatibility, concurrent writes/update и failure policy; согласовать core server-file destination allow-list и её отделение от credentials и SV-only/UI data. Имена namespaces берутся из [Name and ship the new client across platforms](08-name-and-ship-the-new-client.md). Результат — точная planning specification, не код клиента.

Актуальное уточнение shared storage в settings/migration ticket отменяет независимую копию common settings. Registry обязан задавать общие filenames/read/write owners для одинаковых значений, отдельные SV-only values и сохранность legacy-only keys/window flags при round-trip; общий root не разрешает legacy-config writer сериализовать фиктивные SV Term layouts.

Уточнение naming/packaging: default game assets — adjacent `lib/`, общий только при colocated binaries; shared SDL3 user root используется в обоих layout cases. Automatic installation discovery не планируется. Supported Windows OS — Windows10/11, target всё ещё i686 MinGW32; Linux builder baseline — Fedora41.

## Comments

### Storage rules — live decisions, 2026-09-19

- Q1 принят: SV UI settings (fonts/scales/layout/panels) общие для всех серверов и персонажей внутри выбранного user root. Existing personal macro/game-setting scopes сохраняются.
- Q2 изменён пользователем: «только значения по умолчанию и показать игроку сообщение». При повреждении UI configuration используются defaults с понятным сообщением игроку, без восстановления последней исправной копии. Гранулярность обработки единичного неверного значения и disposition исходного файла уточняются отдельно; резервное восстановление не предлагать заново.
- Q3 принят: неизвестные поля совместимого формата сохраняются при записи. При несовместимой версии формата интерфейс запускается с defaults и предупреждением, исходный файл не перезаписывается.
- Q4 принят: перед записью shared settings новый клиент перечитывает файл и меняет только изменённые им значения. Конфликт одного значения разрешается сохранением внешнего значения с сообщением игроку. Пользователь принимает ограничение: legacy SDL3 позднее может перезаписать файл своей копией; без изменения legacy writer полной гарантии нет.
- Q5 принят: при startup resource failure исходный requested resource сохраняется; временный effective fallback показывается в настройках. Возврат файла позволяет следующему запуску использовать исходный выбор. Ранее согласованный live-change failure сохраняет предыдущий рабочий набор.

Тикет остаётся открытым: следующие раунды задают concrete defaults/schema/owners и оставшиеся failure/import/resource contracts.

### Startup, scaling and graphics — live decisions, 2026-09-19

- Q6 принят: при одном неверном значении в читаемом конфиге defaults применяются только к нему с сообщением. При нечитаемом файле используются defaults всего UI.
- Q7 принят: сохранённый SV layout имеет приоритет и определяет effective map geometry сессии. При отсутствии сохранённого выбора default — wide; ранее принятый small-layout offer сохраняется. Запись shared CO_BIGMAP при явном переключении ещё требует отдельного решения.
- Q8 принят с поправкой: UI scale 50–200%, шаг 5%, default100%, поверх обязательного OS DPI; text/controls/spacing меняются вместе. При нехватке места содержимое панелей прокручивается, масштаб самопроизвольно не уменьшается. Concrete controls/layout остаются HTML UX prototype.
- Q9 пересмотрен: «нет, всегда 100%». Карта всегда использует maximum full-grid fit; отдельной пользовательской настройки map fit zoom нет. Это отменяет прежний выбор регулируемого fit zoom в raster ticket, но сохраняет final-cell integer geometry, aspect/centering и normal/big viewport. Из schema исключить range/step/default/key для map zoom.
- Q10: graphics включены по умолчанию. Конкретный tileset ещё не выбран. Вопрос лицензий TTF задан до окончательного выбора font bundle/defaults; Cascadia Mono/9x15 proposal пока не считать утверждённым комплектом.
- Q11 принят: если не удалось загрузить даже резервный шрифт, показать системное сообщение с причиной и завершить запуск. Если отказал только tileset, продолжить с map font и сообщением, сохранив requested tileset для следующего запуска. Речь о startup; live-change failure сохраняет предыдущий рабочий набор по принятой raster policy.

License facts checked 2026-09-19: [Cascadia release v2407.24 LICENSE](https://raw.githubusercontent.com/microsoft/cascadia-code/v2407.24/LICENSE) и [JetBrains Mono v2.304 OFL](https://raw.githubusercontent.com/JetBrains/JetBrainsMono/v2.304/OFL.txt) — SIL OFL1.1. Разрешены бесплатное использование и bundled redistribution с программой, включая коммерческое. При распространении сохраняются copyright notice и полный текст лицензии, font остаётся под OFL; продажа font сам по себе запрещена, а модификации учитывают Reserved Font Names. Для unmodified bundled files достаточно поставлять соответствующие copyright/license тексты с клиентом; это не выбор конкретного default.

### Bundled UI font — confirmed, 2026-09-19

Q10 принят после проверки лицензий: Cascadia Mono — комплектный TTF и default text/UI font. Точный исследованный asset — CascadiaMono-Regular.ttf release v2407.24; binary identity и coverage зафиксированы в [Bundled TTF candidate inventory](../research/bundled-ttf-candidates.md). Поставка неизменённого файла с copyright и SIL OFL1.1. Конкретные map font/tileset и fallback chain остаются следующими решениями; JetBrains/Noto не объявляются выбранными для поставки.

### Map assets, filter and config separation — live decisions, 2026-09-19

- Q12: default tileset `16x24sv`, default map font `16x24x.pcf`; предложенный `16x24tg.pcf` не выбран.
- Q13: default graphics filter Linear. Отдельный PCF filter остаётся по ранее принятой policy (Nearest default); вопрос был про graphics filter. Conversion legacy Lanczos → Linear предложена в раунде, но отдельно не подтверждена ответом «Linear».
- Q14: запись общей настройки размера карты при явном переключении отвергнута: «комппановка старого клиента не зависит откомпановки нового». SV layout changes и startup restore не меняют legacy layout/big-map preference на диске. Effective geometry текущей SV session по-прежнему определяется SV layout.
- Пользователь предложил пересмотр shared-config ownership: «наверно всё таки ты был прав и надо использовать отдельный файл конфига для нового клиента». Отдельный основной конфиг SV — выбранное направление; точный охват отделяемых settings, initial migration и сохранение общих personal files/resources уточняются следующим вопросом. Старые blanket common-config write decisions не применять вопреки этому уточнению.
- Q15: резервный PCF — `16x24x.pcf`, а не `9x15.pcf`. Предложенная цепочка для UI уточняется как requested → bundled Cascadia Mono → bundled16x24x.pcf; для карты requested → bundled16x24x.pcf, без повторения совпадающих шагов. Ранее согласованные startup errors, requested/effective persistence и завершение при отказе всех резервов сохраняются.

### Independent main configuration — confirmed, 2026-09-19

Q16 принят: собственный основной конфиг SV хранит connection server, volume, selected resources, graphics и UI settings независимо от SDL3. Legacy main config доступен для явного разового импорта; последующие изменения не синхронизируются. SV не записывает legacy main config. Это заменяет прежний shared main-config read/write contract, включая обсуждавшееся слияние изменённых ключей перед записью такого файла; shared personal/resource writers по-прежнему требуют собственных concurrency rules.

Сами fonts, tilesets, audio packs и пользовательские macro files остаются общими. User root остаётся общим; отдельная конфигурация не означает копирования resources или смены согласованного packaging layout. Пароли по-прежнему хранятся отдельно в OS vault согласно credential policy. Компоновки независимы, SV не меняет legacy big-map preference.

Остаётся точно классифицировать option records в общих PRF/OPT файлах: common macro/resource file не должен обходить независимость SV/legacy layout. Физический filename/format SV config, first-run import affordance и дальнейшие per-file rules ещё задаются этим тикетом.

### Import offer, explicit save and format — live decisions, 2026-09-19

- Q17 не решён: пользователь запросил фактическое содержимое `.opt`; отдельные SV option files пока не утверждены.
- Q18: импорт предлагается только при обнаружении существующих legacy settings. Само обнаружение не разрешает автоматический импорт. Точное место предложения и bounded discovery источников уточняется при необходимости; ранее предложенный unconditional import affordance не считать окончательным UX.
- Q19: собственные настройки сохраняются только после явного нажатия «Сохранить»; автоматическая запись после применения отвергнута. Применение в текущей сессии и ошибки сохранения не означают успешной disk write. Штатные macro save commands и отдельно принятый credential write lifecycle этим не пересматриваются.
- Q20: JSON отвергнут, использовать формат настроек текущего SDL3. Собственный main config должен использовать совместимый SDL3 CFG syntax; конкретный filename и SV-only key/schema version layout задаются далее. Это не разрешает использовать legacy filename или общий writer destination.

### Shared OPT feasibility — source audit, decision pending, 2026-09-19

Пользователь запросил сравнение отдельных SV `.opt` с расширением legacy `.opt`, не подтвердив отдельные файлы.

Проверено по текущему source:

- Unknown `X:<key>` / `Y:<key>` и unknown opcode возвращают parse error (`src/client/c-files.c:1555–1580,1689`). File loop сообщает ошибку и продолжает (`1735–1743`); известные строки до и после применяются, rollback нет. Это не тихая forward compatibility.
- `options_dump` (`src/client/c-util.c:15297–15395`) создаёт файл заново из известных `option_info` и flags существующих Terms. Произвольные unknown records, comments, includes и mixed macro records не сохраняются.
- Deprecated-name conversion может вызвать этот writer при обычной загрузке `.opt`/`options.prf` (`src/client/c-files.c:1794–1805`). Поэтому extensions можно потерять даже без явного ручного save. Предыдущее решение о Save-only SV main config не меняет поведение legacy writer.

Варианты для обсуждения:

1. Общий `.opt` с SV extensions: без обновления legacy reader/writer не является безболезненно совместимым; комментарии не защищены от пересоздания файла.
2. Общий `.opt` только для существующих совместимых опций, SV-only и layout — отдельно: возможна совместная настройка gameplay, но SV writer должен сохранять legacy-only records/flags и не писать effective SV big_map в legacy preference; старый writer всё ещё имеет concurrency ограничения.
3. Старый `.opt` как односторонняя база + собственные SV overrides: изменения legacy могут наследоваться, изменения SV не пишутся обратно; нужны явные precedence/reset-to-inherited rules и фильтрация layout до immediate apply. Это отдельное исключение к выбранному отсутствию синхронизации main config.
4. Отдельные SV `.opt` того же формата с explicit import: независимые настройки, отсутствие cross-client overwrite, но изменения не синхронизируются.

Product choice ещё не сделан. Никакие OPT файлы пользователя не читались и не изменялись; source audit не является runtime proof.

Уточнение аудита: current `src/common/defines-features.h:893` включает GLOBAL_BIG_MAP. При этой сборке legacy `big_map` row скрыта, а её непосредственное изменение geometry исключено guard в `src/client/c-util.c:19927`; прежнее объяснение, будто общий OPT обязательно меняет actual map geometry этой сборки, было слишком широким. Опция остаётся в dumps; другие gameplay/presentation options и W flags продолжают связывать клиентов.

Load layering: defaults → pref.prf/includes → global.opt → pref-<sys>.prf → font/graphics prefs → global-<sys>.opt (`c-init.c:212–254`), позже character.opt, затем macro PRFs с возможными option records. Простое добавление global-sv.opt в существующий hook не гарантирует финального приоритета SV overrides. Hybrid model требует origin/ownership-aware применения и записи на всех этих границах, не только ещё одного file load.

### Independent OPT files — confirmed, 2026-09-19

Пользователь выбрал: «выбираю отдельные opt». Принят вариант отдельных SV option files с прежним X:/Y: format и явным импортом legacy values; после импорта игровые и presentation options не синхронизируются между клиентами. Legacy `.opt`/`options.prf` не становятся штатным writable destination нового клиента и не используются как автоматически наследуемая пользовательская база. Global/character scopes сохраняются в собственном namespace; exact paths и load/write registry задаются далее.

Файлы макросов и ресурсы остаются общими по Q16. Option records внутри явно/штатно загружаемых общих macro/resource PRFs сохраняют допустимые baseline effects в текущей сессии, но не разрешают записать SV option snapshot в legacy OPT или применить legacy Term layout к SV. Load-time conversion/resave routing и mixed-file preservation должны учитывать record owner, а не только extension. Отдельные OPT не означают изменения macro grammar или создания новой копии макросов.

Предыдущие варианты common OPT и inheritance рассмотрены и не выбраны. Shared-setting merge policy больше не относится к main CFG и OPT между SV/legacy; для оставшихся общих пользовательских файлов и нескольких экземпляров SV concurrency policy остаётся применимой по своему scope.

### Storage location and settings editing — live decisions, 2026-09-19

- Q21 принят: собственные main `tomenet.cfg` и option files хранятся в `sv/` внутри существующего SDL3 user root. Общие macros/resources сохраняют прежние locations. Exact per-file registry уточняет nested paths/scopes.
- Q22 не решён: пользователь запросил, что происходит со старыми macro files. Предложенное применение option directives из них в текущей сессии пока не считать отдельным подтверждённым решением. Общий доступ к macro files ранее принят, но boundaries чтения/сохранения mixed records должны быть явно объяснены.
- Q23 принят: изменения settings сразу применяются для preview, disk write только по «Сохранить»; отмена возвращает значения до открытия settings.
- Q24 принят с текстовой поправкой: при закрытии изменённой settings form предложить «Сохранить / Отменить изменения / Вернуться». Дополнительное подтверждение при обычном выходе из игры не добавляется. Label «Отбросить» не использовать.

### Shared macro files and embedded options — confirmed, 2026-09-19

Q22 принят после объяснения: старые macro files остаются на прежних местах и используются обоими клиентами. Загрузка читает определения и сохраняет допустимые option directives/effects в текущей сессии; указания legacy Term windows к новому интерфейсу не применяются. «Сохранить» в settings записывает option values только в собственные SV OPT, не переписывая исходный macro file. Явное сохранение изменённых макросов в тот же общий файл обновляет его; legacy увидит изменения при следующей загрузке. Это намеренное общее macro behavior при раздельном сохранении опций, не синхронизация независимых main/OPT files.

### Remaining personal files and exit — confirmed, 2026-09-19

- Q25 принят: autoinscription `.ins` и birth-template `.dna` остаются общими, как macro files; изменения доступны обоим клиентам. Существующие scopes/load/save и automatic conversion behavior сохраняются с общим destination owner.
- Q26 принят: chat input history и guide bookmarks нового клиента хранятся отдельно в `sv/`, со старых файлов переносятся через explicit import. Это не новый session recorder и не изменение ранее принятых recall limits.
- Q27 принят: received private notes и exports (messages, character descriptions, screenshots) остаются в общем user directory. Новые exports не перезаписывают существующие files молча. Existing automatic append private notes сохраняется; shared visibility не требует возможности legacy viewer отображать все новые screenshot formats.
- Q28 принят: не сохранённые кнопкой изменения settings при обычном выходе теряются; exit не выполняет autosave собственных CFG/OPT и не добавляет отдельный вопрос. Подтверждение закрытия изменённой settings form по Q24 остаётся. Отдельные baseline writes history/bookmarks/notes/DNA и принятый account credential write lifecycle не превращаются в настройки, требующие этой кнопки.

### UI geometry, filter import, discovery and audio-pack settings — confirmed, 2026-09-20

- Q29 принят: точные default panel widths, text size при100%, spacing и minimum widget dimensions определяются основным HTML UX prototype. Native сохраняет пользовательские изменения и соблюдает ранее согласованный layout/geometry contract. Эти значения не выбираются здесь по произвольным цифрам throwaway layout sketch.
- Q30 принят: imported legacy Lanczos → Linear с указанием замены в import result; внешний source остаётся неизменным. Existing Nearest/Linear mapping по смыслу, без предположения числового enum identity; PCF policy отдельна.
- Q31 принят: при первом запуске проверяются известный SDL3 user directory и стандартное расположение X11 settings. При наличии найденных настроек предлагается explicit import. Другую установку пользователь может указать вручную, сканирование всего диска не выполняется. Это discovery пользовательских settings, не автоматический поиск installations для загрузки bundled assets.
- Q32 принят: event→sound mappings, event disabled flags и per-event volumes внутри audio packs остаются общими вместе с packs. Независимы main CFG selections/master/music/effects/weather channel controls каждого клиента. Это намеренная граница pack settings и client settings, не blanket независимость всех audio files.

### Window mode, remaining defaults and font import — confirmed, 2026-09-20

- Q33 изменён пользователем: default startup — fullscreen, с настройкой выбора «Полный экран / Окно». Предложенный default1600×900 оконный запуск не принят. Явно сохранённый выбор window mode должен применяться при следующих запусках в рамках общего persistence contract. Дополнительный пользовательский режим maximized не утверждён.
- Q34 принят: прочие существующие defaults сохраняются по штатному SDL3 baseline, кроме уже явно переопределённых font/graphics/filter/layout/scale/window-mode settings. Registry должен фиксировать конкретные source/stock values и build gates, а не подставлять новые догадки.
- Q35 изменён пользователем: при выборе explicit font import font главного legacy window переносится одновременно в map-font и text/UI-font roles. Шрифты остальных Terms автоматически по новым panels не распределяются. Без импорта defaults остаются Cascadia Mono для UI и16x24x для карты. Import не связывает роли навсегда: впоследствии они независимо выбираются/сохраняются; visual profiles и map-definition reload следуют соответствующему role contract.

### Conversion, presentation options and message cloning — confirmed, 2026-09-20

- Q36 принят: собственные CFG/OPT преобразуются при загрузке только в памяти; запись преобразованного формата — по «Сохранить». Для общих INS/DNA сохраняется штатная automatic conversion/write semantics. Это явное уточнение прежнего auto-conversion решения; legacy source import не перезаписывается.
- Q37 предложение no-op отклонено: «настройки сохраняются и влияют на новый интефейс». Существующие meaningful display options, включая numeric/bar/huge-bar status presentations и message top-line behavior, сохраняют пользовательский смысл и применяются к corresponding semantic SV surfaces. Конкретный новый visual design принадлежит HTML UX prototype; перенос не создаёт legacy Term model. Эти опции нельзя помечать неприменимыми только из-за смены renderer/layout. Данные модели сохраняются независимо от их видимости/представления. Term window assignment/geometry остаются исключением по ранее принятому scope; legacy paint workaround flags не следует путать с пользовательскими display preferences.
- Q38 принят: clone_to_stdout и clone_to_file сохраняются как в SDL3, с исходными defaults (выключены) и output semantics; output-file owner — общий user directory согласно заданному вопросу. Это existing message output, не новый session recorder. Credential/secret exclusion policy сохраняется.

Прерванное короткое сообщение «36 - ок, 37 - настройки сохраняются,» заменено последующим полным ответом пользователя; оно не является отдельным ограничением.

### Registry assets and remaining source deltas

Подготовлены source-backed рабочие таблицы: [main CFG keys](../research/sv-config-key-draft.md), [option keys](../research/sv-option-ownership-draft.md), [files and operations](../research/sv-file-ownership-draft.md). Это drafts: их ранние proposals (в частности L/no-op для meaningful display options) не имеют приоритета над Q37, а вопросы Lanczos/defaults/font import уже решены в Comments выше. Итоговый Answer должен привести таблицы в соответствие перед closure.

File audit обнаружил отдельный guide-routing mismatch: viewer/updater используют installation root, checksum может выбрать user override. Требуется единый source для отображения/check/update. Изменение размещения обновлённого guide может быть не видно старому клиенту — это ещё отдельный observable выбор, а не автоматическое следствие private bookmarks.

Server transfer INIT и CHECK требуют одного resource-destination contract; current shipped update list содержит scpt/*.lua, generic protocol допускает более широкий набор. Новые SV config/credential/personal destinations не должны становиться writable server-file roots из-за изменения путей. Конкретный contract ещё нужно зафиксировать.

### Guide and original server-file behavior — confirmed, 2026-09-20

- Q39 принят после уточнения объекта: речь о `TomeNET-Guide.txt`, встроенном руководстве с поиском/закладками/обновлением. Updated guide записывается в user root, имеет приоритет над bundled copy; viewer/checksum/update используют один выбранный файл. Старый клиент может продолжать показывать прежнюю installation copy — это принятое ограничение.
- Q40: «делаем так же как и в оригинальном клиенте». Предложенный новый resource-only allow-list с исключением configs/macros/history/notes отвергнут. Server file transfer сохраняет original filename handling, validation, destination mapping и CHECK/INIT/DATA/END/reload behavior; не сужать его до shipped Lua update list и не добавлять новый blanket deny-list под видом реализации. Выбор нового CFG/OPT namespace не означает переноса всей legacy файловой маршрутизации в `sv/`. Отдельное защищённое хранение account secrets по-прежнему не является обычным file-transfer backend. Ранее сформулированное требование выбрать новый allow-list заменено этим решением.

## Answer

Финальное общее понимание подтверждено пользователем 2026-09-20: «подтверждаю». Тикет разрешён.

Утверждён [Persistence contract](../persistence-contract.md) целиком: exact paths и schema, ownership всех файлов, независимые CFG/OPT с явным сохранением, общие macros/INS/DNA/resources, private history/bookmarks, defaults/font import/fallbacks, source-backed overlay для meaningful display options, migration/conflicts и original server-file behavior. Детальные решения находятся в этом документе и не дублируются здесь. Ранние предложения в Comments, Question и source-audit drafts, противоречащие итоговому контракту, не являются действующими требованиями.

Source assets: [CFG keys](../research/sv-config-key-draft.md), [200 lexical option keys](../research/sv-option-ownership-draft.md), [files and operations](../research/sv-file-ownership-draft.md). Эти файлы сохраняют факты и историю предложений; normative dispositions задаёт утверждённый contract, включая сохранение влияния старых display options на новый UI. Статическая сверка подтвердила 200 уникальных option names и существование локальных ссылок, но не runtime parity, exact target-build coverage или Windows/Linux execution.

[Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md) получает перечисленные в contract требования к проверкам. Numeric resource budgets остаются у acceptance; UI geometry/details делегированы основному HTML UX prototype по явному решению пользователя. Новых decision tickets не выявлено. Реализация клиента и изменения пользовательских CFG/OPT не выполнялись.

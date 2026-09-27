# Single-window SDL3 client: implementation-ready specification

Label: wayfinder:map
Status: resolved

## Destination

Подготовить согласованную implementation-ready спецификацию нового SDL3-клиента TomeNET с одним системным окном, собственной семантической моделью интерфейса и проверяемой полнотой возможностей относительно существующих X11/SDL3-клиентов.

Спецификация должна быть достаточна для отдельного этапа реализации на Linux и Windows без скрытых продуктовых, архитектурных или acceptance-решений.

## Notes

- Planning-карта завершена 2026-09-20: открытых decision tickets и неуточнённых in-scope вопросов не осталось. Переход к реализации — по [Implementation sequence](migration-sequence.md); runtime acceptance ещё не выполнялась.

- Измеряемые latency gates — только submission 20/50/200 мс; visible 50/100/250 мс остаются targets с ручной проверкой отклика. Уточнение — [Design parity evidence and acceptance](issues/09-design-parity-evidence-and-acceptance.md#q21--presentation-timing-gate-revised-2026-09-20).

- Acceptance scope уточнён пользователем: обязательные intensive-load и длительные soak-прогоны исключены; актуальная policy находится в [Design parity evidence and acceptance](issues/09-design-parity-evidence-and-acceptance.md#q18q20--stresssoak-excluded-2026-09-20). Прежние формулировки о нагрузочных acceptance runs не возвращают эти проверки.

- Домен: нативный клиент TomeNET и параллельный HTML UX prototype в `/home/svechnik/Projects/github_site/tomenet_interface`.
- В следующих сессиях применять `wayfinder`; по типу тикета также применять `research`, `prototype`, либо вместе `grilling` и `domain-modeling`. Для архитектурных границ применять `codebase-design`.
- Используется ровно один системный `SDL_Window`; legacy-модель из десяти физических или виртуальных `Term` не переносится.
- Повторно используются игровое ядро, сеть, протокол, команды, макросы, звук и ресурсы; presentation state и UI создаются заново.
- Для хранения данных предпочтительно переиспользовать подходящие существующие структуры; собственный UI interface не требует дублирования каждого значения.
- Актуальные уточнения масштабов/defaults находятся в [Specify persistence ownership and UI configuration schema](issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#comments); они имеют приоритет над прежними упоминаниями регулируемого map fit zoom.
- Основной CFG и игровые OPT нового клиента независимы от SDL3, сохраняют прежние форматы; legacy settings импортируются только явно и затем не синхронизируются. Fonts, tilesets, audio packs и macro files остаются общими; точные per-file rules находятся в [Specify persistence ownership and UI configuration schema](issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#comments). Это заменяет прежний blanket shared-config read/write contract.
- Behavior baseline — полный SDL3-клиент плюс применимые возможности X11, кроме отдельных терминальных окон и связанной с ними конфигурации.
- HTML prototype нормативен для подтверждённых UX/компоновочных решений; native behavior baseline нормативен для возможностей и поведения.
- Конкретный внешний вид курсора и cell indicators оставлен основному HTML UX prototype; его выбор не блокирует завершение этой planning-карты. Native geometry/lifecycle и acceptance invariants сохраняются.
- Временный terminal fallback допустим при разработке, но должен отсутствовать в завершённом клиенте.
- Клавиатура и пользовательские макросы обязательны; mouse-first маршруты допустимы; поддержка контроллеров не планируется.
- Общие настройки, макросы и preference-файлы импортируются безопасно; UI-конфигурация изолируется от legacy-клиентов.
- Пароли — исключение из shared settings согласно [Define credential storage policy](issues/24-define-credential-storage-policy.md#answer).
- Текущий сервер и протокол — обязательная база. Можно рассматривать только необязательные обратно совместимые расширения, не влияющие на старые клиенты.
- Продукт проектируется для Linux amd64 и Windows MinGW32; допустима реализация Linux первой.
- Старые ignored modern object-файлы и утраченный подход не восстанавливаются; их удаление относится к последующему этапу реализации.
- Код, удаление артефактов и выпуск клиента находятся за пределами этой planning-карты.

## Decisions so far

<!-- Закрытые тикеты добавляются сюда одной строкой со ссылкой на подробный ответ. -->

- [Inventory the complete behavior baseline](issues/01-inventory-behavior-baseline.md): baseline — union наблюдаемого X11/SDL3 behavior из 36 семейств; 100% требует атомарных inventories и evidence для каждого полного input→state→output→fallback→persistence flow.
- [Govern the cross-repo capability manifest](issues/07-govern-the-cross-repo-capability-manifest.md): canonical JSON manifest живёт в native-репозитории; типизированные IDs, provenance, consumer-owned coverage ledgers и локальная snapshot-синхронизация не дают HTML mocks стать источником игрового поведения.
- [Enumerate every input loop](issues/10-enumerate-every-input-loop.md): 52 scoped contexts проходят physical gesture → macro queue → keymap → transition/intent; полная binding-семантика включает modal exit/retry, macro edge states, gates и focus restoration.
- [Map every packet field to semantic state](issues/11-map-every-packet-field-to-semantic-state.md): 224 versioned wire variants отображаются в snapshots, keyed collections, modal requests, ordered events или lossless opaque envelopes до любого rendering; все receive paths и responses учтены.
- [Inventory formatted and server-driven surfaces](issues/12-inventory-formatted-and-server-driven-surfaces.md): typed stores/prompts отделены от lossless formatted documents и special-store canvases; все `SPECIAL_FILE_*`, local documents/lore и acceptance consequences инвентаризированы.
- [Choose the presentation-state boundary](issues/02-choose-presentation-state-boundary.md): Session presentation model даёт read-only views и ordered events поверх предпочтительно готового хранения; полный decode предшествует немедленному apply, repaint имеет отдельные приоритеты, controls остаются в ядре, fallback — development-only.

- [Set the compatible protocol-extension policy](issues/06-set-compatible-protocol-extension-policy.md): расширения требуют обоснованного outcome, явного negotiation и evidence совместимости; baseline работает без них, UI различает недоступность и ноль, незавершённые packet-пути не реактивируются односторонне.

- [Define the single-window interaction model](issues/03-define-single-window-interaction-model.md): один primary с дочерними surfaces; targeting временно скрывает раздел, Messages заменяет primary, завершение/чат/отмена/relogin следуют baseline без нового запоминания; конкретные layouts вынесены в отдельный тикет.

- [Specify renderer parity](issues/13-specify-renderer-parity.md): source-backed inventory покрывает glyphs/fonts/DPI, palettes/effects, tiles/masks/pictures/caches, weather, resize и screenshots; encoding и raster compatibility требуют отдельных решений, runtime parity не заявлена.

- [Classify persisted settings and files](issues/14-classify-persisted-settings-and-files.md): key/file inventory отделяет data import, новую UI configuration и legacy artifacts; stock loaders имеют execution/auto-resave effects, а SDL3 file routing требует явного ownership; import policy остаётся отдельным решением.

- [Define settings and migration boundary](issues/05-define-settings-and-migration-boundary.md): общий SDL3 root и совместимые settings/files/resources используются для чтения и записи; отдельно хранятся SV-only/UI settings, штатная SDL3 загрузка/форматы/auto-conversion сохранены, внешний legacy import — data-only.

- [Audit platform deltas and packaging](issues/15-audit-platform-deltas-and-packaging.md): platform/build/helper/resource/network и packaging matrix source-backed; sanity feature guard, manual optional degradation и недоказанная runtime dependency closure требуют явной release/acceptance policy.

- [Name and ship the new client across platforms](issues/08-name-and-ship-the-new-client.md): рабочее имя tomenet-sv и отдельные binaries/архивы; Fedora41-class Linux amd64 и Windows10/11 MinGW32, полные dependencies, adjacent lib и общий SDL3 user root, Linux-first с ранним Windows target.

- [Make the HTML gap ledger machine-checkable](issues/16-make-the-html-gap-ledger-machine-checkable.md): полный явный consumer ledger разделяет prototype coverage и человеческий UX approval; outcome/scenario evidence, source fingerprints и local sync обнаруживают drift без claims native parity; контракт задан, атомарное наполнение ещё впереди.

- [Enumerate the remaining client input loops](issues/17-enumerate-remaining-client-input-loops.md): ещё 14 самостоятельных contexts и delegated packet/Lua/shutdown/platform owners дополняют scoped inventory; cancellation, reply и restoration contracts зафиксированы без дублирования primitives и без runtime claims.

- [Enumerate slash-command grammar and dispatch](issues/18-enumerate-slash-command-grammar-and-dispatch.md): все локальные verbs и точная parser/substitution семантика связаны с effects/gates и opaque server forwarding; action families и defect disposition переданы input-router решению, runtime parity не заявлена.

- [Preserve input and macro semantics](issues/04-preserve-input-and-macro-semantics.md): единый router сохраняет command/macro, context-specific cancel/requests и slot semantics через semantic prompts; mouse actions прямые, focus не прерывает macro chain, bounds/cleanup исправлены с сохранением empty/truncation behavior.

- [Assess history retention and recorder necessity](issues/19-assess-history-retention-and-recorder-necessity.md): bounded recall и lossless working state нужны без обязательного session recorder; relog history отделена от event delivery, runtime retention/capture contract вынесен в отдельное решение.

- [Classify display urgency and latency budgets](issues/20-classify-display-urgency-and-latency-budgets.md): urgent/interactive/background budgets 20/50, 50/100 и 200/250 мс разделяют frame submission и видимый результат; deadline не перезапускается, events не coalesce, overlays/animations не блокируют срочные updates.

- [Define encoding and glyph identity](issues/22-define-encoding-and-glyph-identity.md): independent text/map fonts и tileset сохраняют mixed rendering и map-font visual-pref lifecycle; legacy bytes/IDs отделены от Unicode projection, PCF lookup исправляется, outgoing field byte contracts сохраняются.

- [Specify surface layouts and responsive rules](issues/21-specify-surface-layouts-and-responsive-rules.md): one-window geometry использует manual wide/small layouts, full normal/big map fitting, source-preserving documents/canvases, stable primary/children/focus и whole-window lifecycle scenes при minimum1024×768 logical units.
- [Research UI and asset scaling standards](issues/30-research-ui-and-asset-scaling-standards.md): SDL/OS coordinate spaces, DPI/accessibility inputs и current custom-asset pipeline сведены в source-backed модель; варианты fractional grid, filters, masks, TTF и lifecycle переданы raster-решению.
- [Inventory bundled TTF candidates](issues/32-inventory-bundled-ttf-candidates.md): Cascadia Mono и JetBrains Mono NL проходят fixed-width SDL_ttf shortlist, Noto Sans Mono условен для map metrics; exact binaries, OFL packaging, explicit ID profiles и cross-platform acceptance зафиксированы без выбора product default.
- [Research SDL3 tile composition and scaling APIs](issues/33-research-sdl3-tile-composition-and-scaling-apis.md): legacy mask/attr/subtileset composition отделена от старой raster-техники; direct-to-final SDL Renderer/GPU alternatives и реальные Nearest/Linear/Pixel-art/mipmap/Lanczos границы переданы raster-решению.

- [Research PCF loading and scaling options](issues/34-research-pcf-loading-and-scaling-options.md): SDL_ttf/FreeType PCF loading, fixed-strike sizing and raw-ID lookup подтверждены bounded Linux smoke; bitmap sampling choices и surface PixelArt→Nearest fallback уточнены, выбор PCF filter остаётся raster-решению.

- [Choose raster references and defect compatibility](issues/23-choose-raster-references-and-defect-compatibility.md#answer): semantic compatibility без pixel-perfect; final-size asset caches и композиция 1:1, независимые UI/map scales, SDL filters и PCF/FreeType policy, custom-resource fallback и effects согласованы; cursor/settings/runtime evidence остаются отдельными решениями.

- [Define credential storage policy](issues/24-define-credential-storage-policy.md#answer): отдельные Secret Service/Credential Manager records на server/port/account, session-only fallback, explicit legacy import и немедленная запись при смене пароля; legacy plaintext вне SV, без новой функции экспорта и «Забыть пароль».

- [Define bounded working retention and optional capture](issues/26-define-bounded-working-retention-and-optional-capture.md#answer): baseline recall и bounded lossless working state сохраняют обязательную delivery; fixtures-only без session recorder, явный отказ при hard overflow, recreation store animation показывает итог без сохранения хода.

- [Enumerate source text and server-field byte contracts](issues/27-enumerate-source-text-and-server-field-byte-contracts.md#answer): source/field registry охватывает editor и wire boundaries, request/Lua/file sources и byte transforms; NUL/capacity/credential defects переданы отдельному решению, общая кодировка и runtime parity не предполагаются.

- [Prototype the server-defined MOTD screen](issues/28-prototype-the-server-defined-motd-screen.md#answer): выбран вариант C — полная server-defined23-row композиция центрируется с maximum uniform fit и полями24 logical units; source layout и baseline input сохраняются, native evidence относится к acceptance.

- [Decide text-field boundaries and legacy defects](issues/35-decide-text-field-boundaries-and-legacy-defects.md#answer): field limits и тихая обрезка paste/macro/default, полные локальные resource names с сокращённым reporting, безопасные storage/framing и item-order guard, credential transform validation и private archive-password input согласованы.

- [Specify persistence ownership and UI configuration schema](issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#answer): независимые CFG/OPT и private histories в sv/, shared macros/INS/DNA/resources, Save-only, defaults/import/fallbacks и сохранение meaningful display options закреплены полным persistence contract.

- [Design parity evidence and acceptance](issues/09-design-parity-evidence-and-acceptance.md#answer): поэтапная evidence-based приёмка на одной машине Linux/Wine/Windows VM; software rendering обязателен, submission gates и короткие scenarios сохранены, XHTML/stress/soak и общий memory ceiling исключены.

- [Sequence surface migration and retire terminal fallback](issues/29-sequence-surface-migration-and-retire-terminal-fallback.md#answer): этапы A–F с полным реестром в A, ранними Windows checks, явным handoff и постепенным отключением fallback; E полностью native, F подтверждает coverage, archives и отсутствие fallback linkage.

## Not yet specified

Нет. Все in-scope planning-вопросы разрешены; создание реестров и реализация согласованных контрактов относятся к этапу выполнения.

## Out of scope

- XHTML screenshots нового клиента: пользователь исключил эту возможность из behavior baseline; остаются composed-window PNG/BMP screenshots. Решение и последствия — [Design parity evidence and acceptance](issues/09-design-parity-evidence-and-acceptance.md#xhtml-screenshots-removed).

- [Prototype cursor and cell indicators](issues/31-prototype-cursor-and-cell-indicators.md#answer): предложенные варианты отклонены; по решению пользователя конкретный visual design оставлен основному HTML UX prototype.
- Поддержка игровых контроллеров.
- Обязательные или ломающие совместимость изменения сервера и протокола.
- Реализация клиента, удаление старых modern object-файлов, упаковка и публикация релиза: это следующий этап после достижения Destination.

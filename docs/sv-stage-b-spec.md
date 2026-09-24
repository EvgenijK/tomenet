# Этап B — живая сессия и игровой экран TomeNET SV

Статус: specified, 2026-09-23. Реализация по этому документу не начата.
Scope зафиксирован по прошедшему review распределению: **508 активных B outcomes**.
Спецификация не меняет canonical allocation и не объявляет новые native claims.

## Цель и границы

Игрок использует отдельный `tomenet-sv` для реального подключения, входа,
выбора или создания персонажа, MOTD, карты/HUD, basic movement, targeting,
ранних inventory/store-read children, чата/историй и штатного завершения,
смерти или переподключения. Для этих сценариев работают весь уже включённый
в B объём настроек, first-launch import, ресурсов, шрифтов, файлов и credentials.

Нормативный denominator — [native ledger](capabilities/native-coverage.json),
источники/outcomes — [manifest](capabilities/manifest.json), caller mappings —
[reconciliation](capabilities/reconciliation.json). [Применённая таблица](research/sv-stage-b-allocation-proposal.md)
и [разбор границ](sv-stage-b-boundaries.md) объясняют состав. Из исходных583
в B осталось508;11 перенесены в C,7 в D,57 в E. Итоги всего реестра:
A8/B508/C176/D163/E68/F2,925 active и один retained deprecated ID.

[Задачи](tasks/stage-b/README.md) назначают каждому из508 ID ровно одного
implementation/acceptance owner. [Машинный snapshot](tasks/stage-b/coverage.json)
содержит полный список IDs, источников и обязательств, canonical hashes и DAG.
75 тикетов включают73 owning tasks и два zero-owner checkpoints: первый real
entry и финальную cumulative приёмку. Дополнительные caller references не
дают вторичной primary ownership и не принимают поздние outcomes.

Приоритет источников: [AGENTS.md](../AGENTS.md), [архитектура](sv-architecture.md),
[stage/layout contract](capabilities/item-policy.md),
[session/byte contract](capabilities/session-policy.md),
[persistence/resource contract](capabilities/settings-policy.md), затем
конкретные pinned behavior/protocol sources manifest. Исторические предложения
в snapshot не возвращают общий legacy CFG, map zoom, XHTML или virtual Terms.
Новых решений о продуктовых границах в этой спецификации нет.

<a id="foundation"></a>
## Основа A и честная граница evidence

[Checkpoint A принят](sv-stage-a-acceptance.md):62/62 команды, fresh production
checkpoint и human review. Он доказывает synthetic foundation; real login,
gameplay и actual Windows acceptance этим не доказаны. Canonical native claims
остаются pending. Это разные уровни утверждения: принятие checkpoint не означает
автоматического принятия всех атомарных outcomes по evidence schema.

Сохраняются отдельные Linux amd64/Windows i686 targets и object paths, одно
SDL_Window, production decoder/model/router/serializer/renderer, ordered
occurrences, HP/key-request semantics, nonblocking pending input, geometry,
software rendering, timing instrumentation и generation teardown. Используются
существующие [модули SV](../src/client/sv), [runtime scenarios](../tests/sv/scenarios)
и [A runner](../tools/run_stage_a.py), а не старые ignored modern objects.

Каждый новый срез выполняет относящиеся к его изменениям A regressions;
финальный B gate повторяет cumulative A matrix. Для canonical acceptance B
потребуются актуальные accepted prerequisite evidence, включая A IDs. Нельзя
подменять missing runtime fallback measurement константой `fallback_routes=0`,
а dependency closure — одним Git hash. [Evidence contract](capabilities/native-evidence.md)
задаёт scoped reports, freshness и promotion. Статус pending меняется только
после выполнения его требований; сейчас canonical файлы не редактируются.

## Последовательность и первый результат

1. SV-B-001/002 дают endpoint/native input и real TCP negotiation/control path.
2. SV-B-003…019 добавляют production consumers startup: own profile/Save,
   vault/login/existing character/MOTD, macro/INS/history/Guide, FILE/Lua,
   audio/fonts, map, live HUD/messages и first-launch data-only import.
3. [SV-B-020](tasks/stage-b/SV-B-020-first-session.md) — первый runnable milestone:
   **real server → existing account → existing character → требуемые startup
   settings/resources/FILE/Lua → MOTD → initial map/live HUD → clean disconnect**.
   Точный протокольный порядок берётся из startup sources: список dependencies
   не является новой сериализацией FILE/MOTD/Net_start. Server wait отделён от
   client frame submission. Fresh-profile offer/import и existing-profile paths
   различаются; импорт не выполняется автоматически.
4. Создание/перестановка персонажей, birth choices/backtracking/Guide/DNA и
   [SV-B-025](tasks/stage-b/SV-B-025-entry-complete.md) завершают полный entry ID.
5. Directions/targets → pickup/store-read → movement/map exploration →
   clipboard/chat/recall/Guide tools → exports/sheet → death/final review/quit
   и reconnect. Затем закрывается оставшаяся включённая полнота settings,
   font/resource selectors, capture/platform/config и всех171 текущих B options.
6. [SV-B-075](tasks/stage-b/SV-B-075-acceptance.md) объединяет всех owners,
   platforms/evidence и human review.

`session.enter-game` в ledger требует **и** `character.select-existing`, **и**
`birth.complete` для полной готовности. Это альтернативные runtime branches,
не требование создавать персонажа при каждом входе. В SV-B-020 реализуется и
демонстрируется existing-character branch общего production transition; его
единственный полный owner — SV-B-025. Ранний runnable milestone не принимает
этот весь ID, не отменяет birth и не сокращает B.

Номера — допустимый topological порядок, не оценка трудозатрат. Независимые
ветви разрешено выполнять одновременно. У инфраструктуры всегда есть первый
конкретный consumer: например, temp ownership появляется с profile/FILE,
private provider — с login, generation filtering — с disconnect. Не выделяется
самостоятельный «универсальный UI/сеть/хранилище» без наблюдаемого сценария.

<a id="readiness-and-integration"></a>
## Готовность реализации и полная приёмка

В плане различаются две проверяемые границы. `dependsOn` задаёт ациклический
порядок **production implementation readiness**: следующий consumer может
использовать уже реализованный путь, даже если полный platform/caller evidence
его owner ещё pending. Это не разрешение считать полный outcome выполненным.
Единственный primary owner каждого ID отвечает и за реализацию, и за сбор
**всех** canonical obligations, включая поздние actual-caller integration checks.
Тикет может иметь implementation-ready / acceptance-pending; полное закрытие
DoD запрещено до этих checks. Ни subset, ни порядковый номер не повышает claim.

`productionSubsets` в coverage назначает раннюю production реализацию конкретному
тикету, перечисляет реальных consumers, полный owner и проверки. `integrationChecks`
назначает последующим тикетам недостающие/повторные caller проверки, результаты
которых возвращаются первичному owner. Это forward links на evidence collection,
а не обратные implementation prerequisites. Final gate сначала выполняет все
integration checks, затем оценивает claims по неизменному canonical capability DAG
с accepted prerequisites; поздний checkpoint не отменяет зависимость ledger.

| Ранний production путь | Producer / потребители | Единственная полная ответственность |
|---|---|---|
| Platform clipboard read/error bridge, bounded native field paste |001;001/005/006 и map chat029.005 добавляет private-field exclusions |030 сохраняет полный clipboard matrix;001 не заявляет все его outcomes |
| Sticky next-key modifier enabled/disabled |001, с первым SDL physical input; повтор006/043 |043 сохраняет platform/build owner |
| Startup audio init/requested/effective failure |003;010/012/M1 используют этот настоящий executor path |012 принимает device-failure; восемь полных device/pack CFG outcomes остаются E |
| Prepare/activate/reject живого font resource |013 создаёт working native resource-change child и failure path |038 завершает full text/map choosers;013 failure checks повторяются там |
| Default tile/subset composition |014 реализует `graphics1`/`16x24sv`;020 использует штатный fresh profile |039 завершает все modes/subsets/gates/effects и повторяет M1 |
| Existing-character entry |020 реализует существующий production transition |025 принимает оба entry branches, включая birth |
| Map/locate chat child |029 реализует все доступные из него branches/editor transforms/history/local/forwarded routes |031 остаётся полным owner;033/035/036 проверяют Guide/sheet/final callers |
| Native composed-frame capture, encoder и collision/file path |029;035/036 переиспользуют настоящий child |034 завершает все export collisions;043 — все aliases/server triggers/platform cases |

У map029 уже есть production chat/capture и точный возврат focus/selection/queue;
у sheet035 capture приходит из029, а не из воображаемого будущего043. Поздние031/043
повторяют эти реальные callers. Early paste001 использует настоящий platform bridge,
а не подмену clipboard тестом;030 закрывает весь owner matrix. Аудит аналогичных
связей также назначает Guide→birth/sheet, profile→audio/M1, font failure→chooser,
read-only inventory/history/export→final review и option-aware movement/target/store
integration checks. Все связи явны в coverage и соответствующих тикетах.

Для штатного M1 реализация014 включает actual resource/PRF mapping, выбранный
subset, placement/masks terrain+foreground для режима1, palette и final-size
prepared tile assets с viewport fit. При доступном16x24sv карта действительно
тайловая. Font-only failure допускается только при реальном отказе ресурса по
утверждённой policy; отсутствие реализованного renderer не считается таким отказом.
Полный mode2/прочие subsets и live effect cases039 не приписываются этому subset.
Fixtures могут задавать вход/ошибку peer/provider/resource; они не заменяют
production child, decoder, painter, file writer или input continuation.

## Реализация в согласованных модулях

Следовать ADR о [разделении model/interactions/UI](adr/0001-separate-session-interactions-and-ui.md),
[главном потоке](adr/0002-main-thread-state-ownership.md),
[явных связях и порядке](adr/0003-explicit-module-connections.md),
[read views/caches](adr/0004-revision-based-ui-views.md),
[generation ownership](adr/0005-session-scoped-work.md) и
[ошибках/перегрузке](adr/0006-errors-and-overload.md).

Application собирает владельцев lifecycle; transport/protocol решает wire/version
правила; session model публикует coherent state и ordered occurrences; interaction
ведёт pending choices/macros/parent; UI читает состояние и передаёт смысловой ввод;
alerts вычисляют каждый эффект, audio/provider/file executors исполняют его.
Один вход полностью применяется до следующего; исполнители не вызывают рекурсивный
apply. Network, timers и urgent frame submissions продолжаются под каждым child.

Длительные DNS/provider/file/font/Lua операции нельзя прятать в блокирующий
UI handler. Конкретное разбиение выбирает реализующий тикет: bounded incremental
work либо фон с принадлежащим владельцу payload/result и generation check.
Session teardown прекращает commands, отменяет interactions/queues и освобождает
late results. Font/audio/preferences application lifetime не смешивается с session.
Независимая optional-feature failure не разрывает рабочую сессию автоматически.

Заимствованные крупные read views живут только в frame phase. Межкадровые caches
восстановимы и зависят от state/resource/profile/geometry generation. Изменение
последнего значения может объединяться в кадре; warning/message occurrences
сохраняют порядок и кратность. Queue overload не теряет обязательные updates
молча: backpressure, then explicit session failure если безопасно продолжить нельзя.

Минимизировать изменения legacy/common. SV-local production adaptation и
осмысленное дублирование разрешены. Общие модули выделяются только по текущей
необходимости. Отдельные улучшения фиксируются в [sv-improvements.md](sv-improvements.md),
не включаются автоматически в этот этап. Точные C signatures, buffers и file split
— инженерный выбор реализации в этих пределах, а не незакрытое продуктовое решение.

## Протокол, текст и ввод

Каждый owner сохраняет все source-defined version/build branches, packet dispatch,
field sizes/defaults и cancel/retry behavior. Пакет сначала полностью декодируется;
incomplete не публикует state/event/reply. Malformed framing не «чинится» догадкой.
Outgoing packet валидируется и ставится целиком либо ждёт места; local refusal
не отправляет частичные поля. Реальная server confirmation отлична от факта send.

Source bytes, slot identities и glyph numbers не восстанавливаются из терминального
raster или formatted display. UI Unicode и field bytes — разные представления.
Использовать [принятые dispositions](capabilities/session-policy.md): interactive
account/password15 bytes, precise field payload/NUL allowances, capacity **после**
chat/address/colour/item/paste transforms, limit−1/limit/limit+1, сохранение draft при
encoding error. Prefix truncation допустима только у согласованного editor/reporting
owner; это не разрешение молча обрезать codec или secret. На server_protocol>=2
`*` в credential блокируется до XOR42→NUL, на старый протокол запрет не переносится.

Один input router владеет logical context, keysets, macro queues и pending replies.
Longest-match/waits/raw bypass, interruption, parent restoration и command/store
key collisions сохраняются. Escape имеет конкретный caller result: не вводить
универсальную отмену. Quantity находится в C; ранние read-only/direction/target
B paths не получают искусственный numeric prompt. Его будущая семантика:
Escape→0, accepted empty→default; ноль интерпретируется caller.

Accepted B flow не входит в fallback. Нет catch-all recovery на unknown key/error.
Будущий явно registered C–E child может быть development fallback только по
[transition contract](capabilities/item-policy.md#nativefallback-transition-and-retirement),
без принятия полного child и с сохранением input/network/generation ownership.
План не требует добавлять адаптер; текущий route set пуст.

## Настройки, файлы и ресурсы

Пути следуют [ownership contract](capabilities/settings-policy.md#ownership-and-paths):
U — существующий SDL3 user root, S=U/sv. CFG/OPT/history/bookmarks независимы;
макросы/INS/DNA/resources/notes/exports разделяются только где это согласовано.
Нельзя глобально перебазировать legacy user root или подменить fingerprint U на S.
Every stat/read/temp/backup/rename/remove использует того же resolved owner.

Preview/Save/Cancel/Return, explicit-save-only CFG/OPT, normal exit без auto-save,
source immutability и external same-key conflict mandatory для каждого раннего
поля. Import — data-only preview/stage/explicit group commit, destination wins
по умолчанию; linked PRF/includes никогда не исполняются импортом. Missing/cyclic
linked files отказывают своей группе, не уничтожают остальные. Пропущенный setup
без CFG снова остаётся first-run; source detection не разрешает применение.

Credentials: Linux binary Secret Service и Windows generic Credential Manager,
namespace `tomenet-sv/v1`, collision-free server spelling/effective port/account
byte identity; Windows target hex не сливает account case. No plaintext fallback,
no secret history/diagnostics, session-only при отказе. Linux API/header наличие
не доказывает usable provider; Win runtime проверяется отдельно. Login success
и принятая смена password сохраняют raw secret в моменты из policy.

Ранние handoffs остаются у existing B owners:

| Отложенный полный outcome | Ранний B owner / необходимый результат |
|---|---|
| `files.ins-shared` E | `session.load-profile-input`, `items.autoinscribe-on-update`: most-specific read, conversion-needed-to-read, rule order и once-per-update; full management/Save branches не приняты |
| Восемь audio device/pack CFG outcomes E | `settings.load`, `session.load-profile-input`, `audio.device-failure`: parse/default/validate и actual initial device/pack/subset/effective identity; stereo не равно effect-track count |
| `guide.context-help` D | `guide.read/search/close`, ранние birth/sheet callers: exact topic и возврат pending selection; skills/lore union позже |
| `chat.cancel` D | `chat.send` и lifecycle каждого раннего caller: no Send_msg, exact focus/draft/queue return; lore/document/skill branches не принимаются заочно |

Весь сохранённый B font/resource scope остаётся обязательным: PCF/TTF/profile corpus,
независимые roles, полный chooser/filter/outline/failure, cache generation и
requested/effective fallback. Text change не reloads map mapping; map font следует
font→mapping→graphics overrides. No user map zoom; final-size assets compose1:1,
maximal uniform viewport fit. Resource fallback сохраняет requested identity;
live replacement failure оставляет previous working stack.

Опции принимаются по реальному consumer, не по parser/Save. `disturb_other` сохраняет
default/storage/wire slots без выдуманного effect. `subterm_flicker` регулирует
auxiliary inventory/messages; `misc_no_flicker` подавляет auxiliary non-message
content, сохраняя message exception и приоритет global/subterm suppression.
Main map — неизменный control для этих двух switches. ASCII glyph regressions
относятся к map и поздним artifact/monster lore; B `unique-records` хранит только
kill count/name. [Consumer scenarios](capabilities/reconciliation.json) сохраняют
как ранние, так и поздние проверки, не создавая новых readiness edges.

<a id="verification"></a>
## Проверки и критерии готовности

Каждый owning ticket содержит точные IDs, outcome/source ссылки, version/build
labels и конкретные allocation checks. Все существующие obligations ledger
обязательны, даже если общее правило вынесено сюда. Для каждого применимого пути:

1. Success/negative/cancel/retry проверяются через production SV decoder/model/
   router/UI/serializer/executor. Подменяется внешний peer/provider/filesystem
   fault/clock, а не поведение. Headless rule checks дополняют real-window checks.
2. Packet split на границах полей, chained sentinel, absent-version defaults,
   malformed input и outgoing queue exhaustion не дают premature/partial effects.
3. Physical normal/roguelike input, ordered macro routes, mouse bindings где
   предусмотрены, resize/focus/minimize/restore сохраняют exact pending step,
   byte draft, stable selection и actual parent. Network continues while waiting.
4. Disconnect/relogin/teardown в каждом существенном ожидании; old-generation
   responses/tasks/macros не меняют новую сессию. Redraw/recreation не дублирует
   commands, file writes, notifications или accepted occurrences.
5. Реальный сервер демонстрирует entry/movement/chat/session round trips.
   Deterministic peers покрывают не воспроизводимые на одном сервере protocol
   versions, rejection и partial cases; это supplement, не замена live milestone.
6. Для settings/files: isolated profile, success→Save→restart, Preview→Cancel,
   missing/invalid/provider/disk/replace/conflict failures, unchanged unrelated
   owners/source files; для options обе BOOL ветви на source-defined consumer.
7. Bounded working ownership/retention, baseline recall capacities и explicit
   hard overflow; короткие lifecycle checks. Не добавляются recorder, stress/soak
   или выдуманный global RAM ceiling.

[Latency contract](../.scratch/single-window-sdl3-client/acceptance-contract.md#timing-concurrency-and-memory)
требует decode/input→successful frame submission **≤20/50/200ms** по urgency,
с первой outstanding deadline, без её сброса coalescing. Записывается каждое
нарушение, не только average. Server wait считается отдельно. Visible targets
50/100/250ms оцениваются вручную; они не выданы за измеренную presentation latency.

Geometry matrix:1024×768logical,1920×1080/100%,3840×2160/200%, fractional125/150%,
TTF/PCF roles, normal66×22/big66×44, wide/small, prompt/primary/sheet/Guide/MOTD.
Полный viewport fit; no automatic layout switch. Typed lists/source-preserving
no-wrap documents и pending children следуют approved layout. UI/generic source
content не теряет unknown bytes, и renderer не реконструирует domain state.

## Платформы, evidence и финальный gate

Linux-first implementation не отменяет MinGW i686 с начала и Wine smoke на
checkpoints. B заканчивается только после **реальных Windows10 и11**, обоих
renderer classes. Platform-specific changes проверяются при появлении. Для
каждого evidence record указываются actual OS/version/architecture/renderer,
server/version/build branches, executable/configuration/report fingerprints,
source/fixture/resource/SDK dependency inventories и completed runtime fallback
check. Wine/headers/build success не подменяют actual OS behavior.

Все508 B outcomes и нужные A prerequisites имеют актуальные применимые records;
accepted implementation и accepted evidence раздельны. Missing/stale/failed checks
блокируют соответствующий claim. Реальные Linux software/accelerated, Windows
software/accelerated, human native review, A regressions и B integration scenarios
сводятся в stage report. HTML coverage/UX ledger отдельны; missing HTML не блокирует
native работу и не даёт native acceptance.

До gate инфраструктура реального сервера, тестовых account/character, Secret
Service provider и Windows10/11 VM должна быть доступна проверяющему. Сейчас их
доступность не утверждается. Недоступность будет concrete execution/evidence
blocker, не повод пересогласовать платформы или расширить fallback. Секреты,
account details и private logs не включаются в planning artifacts.

## Граница после B и риски

C сохраняет quantity, mutating item/store/skill/spell/combat/ghost callers.
D сохраняет remaining information/documents/lore/admin/special-store и полные
context-help/chat-cancel caller unions. E сохраняет оставшиеся integrations,
macro editing/recording/wizard, shared INS management, reimport и полные audio
pack/device editors. F — окончательная archive/dependency closure. Ранние
branches этих семейств не доказывают их поздние полные outcomes.

Новых открытых пользовательских решений для этого scope не найдено. При реализации
остаются source-specific engineering details (C interfaces/bounds/async ownership),
непроверенная runtime совместимость и доступность test environments. Если фактический
caller обнаружит пропущенный outcome/dependency или неразрешённый выбор поведения,
зафиксировать конкретный источник/пример, обновить согласованную allocation через
review до acceptance; не менять508 denominator молча ради закрытия тикета.

План проверяется командой `python3 docs/tasks/stage-b/check-plan.py`: exact owners,
обязательные пять canonical SHA-256, source/obligation/allocation hashes, production
subset availability, forward integration checks, dependency order/DAG, local links. Это проверка
плана, не тест реализации и не evidence приёмки B.

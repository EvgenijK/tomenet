# SV-B-007 — Загрузка профиля клавиш и исполнение макросов

Статус: частичная SV production implementation; полная implementation readiness и
acceptance pending.

## Текущий production срез (2026-09-26)

SV получил отдельный read-only PRF parser для shared U/user→B/user overlay:
обычная загрузка `A/P/H/C/D`, `%` с диагностикой missing/invalid/cycle,
`X/Y`, `S`, `#`, `!/?` с body guard. Startup загружает shipped `pref.prf` и
`pref-sdl3.prf`, затем собственные S OPT layers; character path читает
`global.prf` после character OPT, а shell без персонажа загружает его отдельно;
включения
legacy `options.prf` и `window.prf` в shipped bootstrap исключены. Отдельные
production функции принимают character/race/trait/class/form и named/class PRF.

SV app исполняет multi-byte longest trigger и pushback, normal/hybrid/command
gates, двух- и четырёхзначное ожидание через неблокирующий frame, `PKT_CONFIRM`
и request semaphore, xwait Escape/Space/fresh input, generation reset. Командный
путь реализует baseline normal/roguelike direction maps, raw bypass, control
prefix, walk/run/tunnel и chat serialization; unhandled keys используют
отдельный `PKT_RAW_KEY` path. `tests/sv_macro_checks.py` проверяет exact bytes
через production `SvApp`, parser, protocol и session.

[Частичные production observations и fingerprints](../../sv-b007-evidence.md)
сохраняют проверенный срез без заявления полной acceptance.

Пока не перенесены графические PRF mapping consumers, полноценный gameplay
command dispatch и вызов character layers из live login с поздними
form/character reload points. Native loader работает только в synthetic shell
до появления endpoint→gameplay handoff.
Нужны также платформа Windows и matrix из Definition of Done. Поэтому ни один
полный capability ID этого тикета не объявлен принятым.

## Пользовательский результат

Игрок использует свои baseline preference layers и keyboard/macro routes без необходимости полного редактора макросов.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-003](SV-B-003-profile.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `network-confirm`: executor `capability.input.macro-wait` подключает
  generation-bound confirmation waiter из production input path
  [SV-B-002](SV-B-002-contact.md). Проверить queued macro wait с
  interleaved `PKT_CONFIRM`, одно применение byte в порядке сети, отсутствие
  повторного применения после redraw, teardown/новую generation и отсутствие
  блокировки network/timers. Evidence вернуть owner
  `capability.network.confirm` в SV-B-002; этот тикет сохраняет своё
  `capability.input.macro-wait` acceptance.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.preferences.load` | Ordinary explicit PRF loading preserves % includes, option/mapping records, !/? permitted queued actions and # message effects, exact gates and baseline warnings; this is not migration. | [c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-files.c:1068](../../../src/client/c-files.c#L1068) |
| `capability.preferences.bootstrap` | Load defaults, shipped bootstrap, own global/system/character OPT and shared macro/resource layers in approved order; includes retain provenance and owner; W records create no Terms. | [c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-files.c:1068](../../../src/client/c-files.c#L1068) |
| `capability.preferences.include-failure` | Report missing/invalid/recursive includes through baseline load behavior without executing migration input; linked import group is skipped on missing/cyclic/out-of-source includes. | [c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-files.c:1068](../../../src/client/c-files.c#L1068) |
| `capability.preferences.macro-precedence` | Load global/race/trait/class/character/form macros in baseline order including later form and character reload points; preserve trigger/action bytes and body-macro execution guards. | [c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[nclient.c:358](../../../src/client/nclient.c#L358)<br>[c-files.c:1068](../../../src/client/c-files.c#L1068) |
| `capability.macros.load` | Explicitly load selected shared macro file with normal preference execution semantics. | [c-util.c:1197](../../../src/client/c-util.c#L1197)<br>[c-util.c:1](../../../src/client/c-util.c#L1)<br>[c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.macros.close` | Close macro management and restore caller and its input/macro queue semantics. | [c-util.c:1197](../../../src/client/c-util.c#L1197)<br>[c-util.c:8257](../../../src/client/c-util.c#L8257) |
| `capability.macros.load-class` | Load class-specific macros through normal permitted preference effects. | [c-util.c:1197](../../../src/client/c-util.c#L1197)<br>[c-util.c:1](../../../src/client/c-util.c#L1)<br>[c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.input.keymap` | Normal/roguelike command maps preserve raw backslash bypass and control-prefix input. Escape, CR and hyphen are reserved no-ops; other unhandled keys retain the separate raw-key dispatch path. | [c-util.c:3583](../../../src/client/c-util.c#L3583) |
| `capability.input.macro-match` | Resolve longest macro trigger, unmatched byte pushback, command/hybrid/normal policy and completion/control sentinels in the actual caller context. | [c-util.c:916](../../../src/client/c-util.c#L916) |
| `capability.input.macro-wait` | Execute two-digit macro wait while pumping network/timers, ending on duration/semaphore/confirm; do not invent interactive cancellation. | [c-util.c:417](../../../src/client/c-util.c#L417) |
| `capability.input.macro-xwait` | Execute four-digit extended wait; fresh Escape discards temporary queue, Space resumes/restores old queue, other fresh keys are preserved. | [c-util.c:568](../../../src/client/c-util.c#L568) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Production PRF load: system/user/character/race/trait/class/form precedence, includes and failure, bootstrap не подтягивает legacy CFG/OPT; macro directives сохраняют разрешённые эффекты.
2. Longest trigger, unmatched pushback, normal/hybrid/command boundaries, raw bypass и waits/xwait сохраняют порядок; waiting macro не блокирует network/UI.
3. Native load/class-load/close вызывают актуальный caller и queue policy; editor/recording/wizard E не объявляются готовыми.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.preferences.load` | %,X/Y,mapping,!/?,# records: proper warnings, queues and original execution gates; macro bytes and command order. Имеется отдельный caller regression для C/D actions, primitive load не принимает их игровые результаты. |
| `capability.preferences.bootstrap` | Defaults→bootstrap→own global/system/character OPT→shared macros; legacy global.opt/options.prf не протекают. Имеется отдельный caller regression для C/D actions, primitive load не принимает их игровые результаты. |
| `capability.preferences.include-failure` | Missing/invalid/recursive include при normal load; migration skips linked group and executes nothing. Имеется отдельный caller regression для C/D actions, primitive load не принимает их игровые результаты. |
| `capability.preferences.macro-precedence` | Conflicting trigger по всем layers, later form/character reload, body command guards; final exact bytes and order. Имеется отдельный caller regression для C/D actions, primitive load не принимает их игровые результаты. |
| `capability.macros.load` | Загрузить named PRF, выполнить B movement/chat macro; exact trigger/action и permitted load effects. Missing/invalid file, cancel имени и возврат input context; поздние combat callers получают собственные C regression. |
| `capability.macros.close` | Закрыть B loader при queued macro и восстановить parent без повторного действия; после реализации E повторить для полного editor. |
| `capability.macros.load-class` | Загрузить class PRF, выполнить B movement/chat macro; exact trigger/action и permitted load effects. Missing/invalid file, cancel имени и возврат input context; поздние combat callers получают собственные C regression. |
| `capability.input.keymap` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Normal/roguelike command maps preserve raw backslash bypass and control-prefix input. Escape, CR and hyphen are reserved no-ops; other unhandled keys retain the separate raw-key dispatch path. |
| `capability.input.macro-match` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Resolve longest macro trigger, unmatched byte pushback, command/hybrid/normal policy and completion/control sentinels in the actual caller context. |
| `capability.input.macro-wait` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Execute two-digit macro wait while pumping network/timers, ending on duration/semaphore/confirm; do not invent interactive cancellation. |
| `capability.input.macro-xwait` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Execute four-digit extended wait; fresh Escape discards temporary queue, Space resumes/restores old queue, other fresh keys are preserved. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.preferences.load`, `capability.preferences.bootstrap`, `capability.preferences.include-failure`, `capability.preferences.macro-precedence`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..
- `capability.macros.load`, `capability.macros.close`, `capability.macros.load-class`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; ENABLE_MACROSETS for stage/set operations; ordinary macro actions retain their own compile and prompt gates..
- `capability.input.keymap`, `capability.input.macro-match`, `capability.input.macro-wait`, `capability.input.macro-xwait`: versions — All baseline versions supported by the cited owner; retain its version branches; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

## Comments

### Follow-up после review — native load и повторные layers

Native Ctrl+F7/Ctrl+F8 теперь запускает PRF parse на изолированном snapshot в
SDL worker. Worker читает файлы и собирает ограниченный журнал effects; только
главный поток публикует macro/keymap/option profile и применяет origin/queued
effects по 16 записей за кадр. Escape, F5, смена generation и teardown отменяют
результат до начала публикации; после начала commit Escape закрывает loader по
завершении применения, F5 ждёт конца commit, обычный input dispatch
удерживается, а ответы на server request продолжают отправляться;
server request/pause во время commit не обрывает оставшиеся effects;
их ответный ввод передаётся штатному input adapter. Закрытая session отменяет
worker result даже без смены generation;
ошибка worker или превышение лимита effects остаются видимыми в loader status.
SDL event timestamp и generation проверяются перед UI, loader и gameplay input,
включая queued opener и Enter после F5.

Runtime character API повторно применяет character/race/trait/class/form layers
из сохранённой global базы. `global.prf` effects не выполняются второй раз;
записи global options повторно применяются после character OPT, чтобы сохранить
baseline порядок даже после synthetic global→character. Form layer зависит от
`load_form_macros`. Production-path fixture проверяет обе величины опции,
возврат из form в Player, повторную form загрузку, смену character, exact
command bytes, queued effects, отмену native load и stale opener/submit.

Live endpoint всё ещё останавливается на `SV_LOGIN_SELECTED` без gameplay
handoff и без version-aware character/form changes. `ui/ui.c` не имеет visual
mapping consumer для `R/K/F/U/r/Z/E/I/V`. Следовательно live initial/later
layers и graphical PRF acceptance остаются pending; synthetic/runtime fixture
не является доказательством live acceptance.

### Follow-up после review от 2026-09-26

Командная сериализация перенесена из `app.c` в SV input/command module;
macro matching, wait и action dispatch принадлежат `input/macro-executor.c`.
`PKT_CONFIRM` для macro wait теперь проходит через generation-bound waiter
SV-B-002 с явным begin/take/end и одним потреблением. Сохранён runtime PRF
controller: у каждого применённого macro/option/keymap, `#`/`!` эффекта и
warning остаются U/B owner, разрешённый путь и строка; успешные `%` включения
хранят также источник/строку и разрешённого U/B владельца цели. Диагностика failed
`%` указывает включающий файл/строку и имя цели. Native synthetic shell
получил Ctrl+F7 named PRF и Ctrl+F8 class PRF с Escape/parent/queue semantics;
те же production parser, controller и `SvApp` участвуют в проверке. Bootstrap
оставляет `global.prf` до character OPT; synthetic shell без персонажа грузит
его отдельно один раз. `S` записи с нулевой командой и baseline нормализацией
направления применяются в production command router.

Остаются **pending**, без acceptance claim:

- Character OPT и race/trait/class/character/form PRF реально применяются через
  `sv_preference_runtime_character`, но live endpoint в `endpoint-run.c`
  заканчивается на `SV_LOGIN_SELECTED` до создания gameplay `SvApp` и первой
  отправки option packet. Нет production handoff после выбора персонажа и
  изменений form/character; `SvChangeKind` в `session/session.h` не содержит
  form/character update. Требуется связать этот handoff и поздние updates с
  controller, сохраняя порядок перед options packet. Тестовый вызов controller
  не считается live login/reload acceptance.
- `R/K/F/U/r/Z/E/I/V` graphical PRF mappings не имеют SV visual model,
  renderer mapping table или reload consumer. Текущий `ui/ui.c` рисует shell,
  а не map/game visual stack; parser по-прежнему сообщает unsupported record
  с origin. Требуется production visual consumer и version-aware update path,
  затем применить valid mapping records и проверить rendering/packets.
- Native loader сейчас доступен в synthetic gameplay shell. Live endpoint
  после `SV_LOGIN_SELECTED` не передаёт управление этому gameplay shell; его
  native load/class/close acceptance ждёт тот же handoff.
- `input/command.c` теперь владеет сериализацией доступных B-команд
  walk/run/tunnel/stand/chat/raw. Другие gameplay command owners ещё не
  существуют в SV (`SvCommandKind` их не представляет); соответствующие
  combat/inventory/target flows остаются за поздними B-тикетами. Это не
  подтверждает весь baseline command dispatch.

### Review follow-up, 2026-09-26: control markers and native wait

SV macro execution consumes action control markers before command dispatch: action `28 … 28` removes its delimiters but delivers the enclosed command bytes, while action `31 …` strips the trigger sequence through its control terminator. The physical SDL special-key sequence uses its default byte only when no macro matches; matched sequences discard that fallback. A fallback byte joins pending macro work as resolved input, so it neither overtakes buffered action bytes nor gets matched as another macro. Native Escape stays in the gameplay input route during ordinary `\wXX` and extended `\WXXXX` waits, including encoded physical fallback during a wait. Production `SvApp` exact-byte and native-input fixtures cover these paths. The targeted macro test and Linux build pass. The core sweep is 28/31: the two registry checks report the previously recorded `src/makefile.sv` source digest mismatch, and SDL dummy does not support `SDL_MinimizeWindow` in the native lifecycle test. Full acceptance remains pending on the already listed live gameplay handoff, visual PRF consumer, platform matrix, and later integration checks.

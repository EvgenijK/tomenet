# SV-B-007 — Загрузка профиля клавиш и исполнение макросов

Статус: specified; реализация и runtime evidence не выполнены.

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

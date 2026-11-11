# План функциональных этапов SV после B

> Проверки платформ с 2026-10-28: [обязателен Linux](agents/platform-validation.md).
> Windows/MinGW/Wine сборки и проверки временно отложены для всех задач и этапов
> и не блокируют текущую приёмку; прежние matrix и результаты ниже сохраняются.

2026-10-06. Этапы A (8 outcomes) и B (54 outcomes) заморожены. Новое
разбиение C001–C064 применяется к 863 outcomes прежних C–G; полный denominator
остаётся равен 925. Machine-readable authority —
[`capabilities/stages.json`](capabilities/stages.json), schema —
[`capabilities/stages.schema.json`](capabilities/stages.schema.json).

Каждый этап после B содержит только один namespace `capability.<block>`. Один
блок может иметь несколько последовательных slices: ранний slice содержит только
минимум для usable client, поздний принимает optional или интеграционное поведение.
Это acceptance slices, а не требование создавать отдельный архитектурный модуль.

## Дополнительный presentation этап B2

По запросу пользователя от 2026-10-28 между принятым B и C001 запланирован
[B2 — native startup UI по веб-макету](sv-stage-b2-spec.md),
[implementation tickets](tasks/stage-b2/README.md). Это перенос visual/layout
существующих B screens с нулём новых capability IDs; отдельной строкой
canonical stage catalog он не становится. Таблица C001–C064, порядок и counts
ниже сохраняются. B accepted и снятые проверки B не переоткрываются.
B2 принят пользователем 2026-10-30: [решение](sv-stage-b2-acceptance.md).

## Последовательность

| Этап | Блок | Outcomes | Scope |
|---:|---|---:|---|
| A | `baseline` | 8 | Frozen synthetic foundation |
| B | `baseline` | 54 | Frozen pregame baseline |
| C001 | `platform` | 1 | Запуск установленного клиента |
| C002 | `settings` | 6 | Bootstrap settings data без UI |
| C003 | `files` | 10 | Core resources и persistence |
| C004 | `preferences` | 3 | Preference loading без macro precedence |
| C005 | `input` | 3 | Keymap, quantity, raw key |
| C006 | `fonts` | 11 | Text/map fonts и fallback |
| C007 | `rendering` | 15 | Визуальный игровой rendering |
| C008 | `network` | 6 | Оставшиеся network controls |
| C009 | `lua` | 1 | Startup Lua reload |
| C010 | `transfer` | 5 | Server file transfer |
| C011 | `session` | 5 | Session preparation |
| C012 | `status` | 27 | Live HUD/status |
| C013 | `world` | 1 | Основная live-карта (`world.read-map`) |
| C014 | `session` | 4 | Live session lifecycle |
| C015 | `map` | 7 | Overview/locate карта |
| C016 | `messages` | 9 | Live и historical messages |
| C017 | `request` | 9 | Server requests |
| C018 | `target` | 8 | Targeting и inspection |
| C019 | `direction` | 6 | Direction selection |
| C020 | `world` | 19 | World movement и interaction |
| C021 | `items` | 23 | Inventory data и item-selection surfaces |
| C022 | `items` | 48 | Ordinary item actions |
| C023 | `skills` | 6 | Skills и abilities |
| C024 | `spells` | 34 | Spells и special abilities |
| C025 | `items` | 5 | Spell-dependent item actions |
| C026 | `combat` | 11 | Combat actions |
| C027 | `store` | 28 | Ordinary stores без `store.service` |
| C028 | `chat` | 6 | Chat |
| C029 | `exports` | 2 | Data export |
| C030 | `information` | 43 | Information surfaces |
| C031 | `documents` | 21 | Documents |
| C032 | `session` | 8 | End-of-session flows |
| C033 | `housing` | 11 | Housing |
| C034 | `special-store` | 12 | Special-store presentation |
| C035 | `server-flow` | 15 | Server-driven gameplay flows |
| C036 | `store` | 1 | `store.service` |
| C037 | `credentials` | 1 | Credential update |
| C038 | `account` | 4 | Account management |
| C039 | `character` | 5 | Character ordering |
| C040 | `birth` | 2 | Birth DNA persistence |
| C041 | `admin` | 46 | Administration |
| C042 | `clipboard` | 4 | Clipboard |
| C043 | `social` | 23 | Party, guild и hostility |
| C044 | `settings` | 22 | Settings interface |
| C045 | `options` | 32 | Input, messages и alert options |
| C046 | `options` | 35 | HUD, lighting и visibility options |
| C047 | `options` | 44 | Map identity, animation, movement и targeting options |
| C048 | `options` | 41 | Item handling и session options |
| C049 | `options` | 32 | Audio и client-integration options |
| C050 | `alerts` | 5 | Game alerts |
| C051 | `imports` | 8 | Legacy import |
| C052 | `preferences` | 1 | Macro preference precedence |
| C053 | `input` | 3 | Macro input runtime |
| C054 | `macros` | 35 | Macro management |
| C055 | `audio` | 25 | Audio |
| C056 | `configuration` | 34 | Editable client configuration |
| C057 | `platform` | 5 | Optional platform capabilities |
| C058 | `lua` | 1 | Local Lua execution |
| C059 | `files` | 1 | Shared INS files |
| C060 | `os` | 2 | OS integration |
| C061 | `screenshots` | 3 | Screenshots |
| C062 | `guide` | 14 | Guide core без bookmarks |
| C063 | `files` | 2 | Guide bookmark persistence |
| C064 | `guide` | 3 | Guide bookmark actions |
| **C001–C064** | **46 namespaces** | **863** | Все outcomes прежних C–G |

C001: [спецификация запуска установленного клиента](sv-stage-c001-spec.md),
[задачи и ownership](tasks/c001/README.md). План подготовлен 2026-10-31;
реализация и [техническое Linux evidence](sv-c001-evidence.md) выполнены
2026-11-04; [C001 принят пользователем](sv-stage-c001-acceptance.md) в тот же день.
Canonical registry status
сохраняется pending до отдельной регистрации evidence.

C002: [спецификация bootstrap настроек](sv-stage-c002-spec.md),
[задачи и ownership](tasks/c002/README.md). План подготовлен 2026-11-05;
реализация и [Linux evidence](sv-c002-evidence.md) выполнены в тот же день.
Initial audio и CLI boundary разрешены; исправленный birth fixture прошёл
`waiting/partial/ack` 2026-11-07. [C002 принят пользователем](sv-stage-c002-acceptance.md)
2026-11-07, включая задачи 001–004. Canonical registry status сохраняется pending
до отдельной native-runtime registration.

C003: [спецификация файловой основы](sv-stage-c003-spec.md),
[задачи и ownership](tasks/c003/README.md). План 2026-11-07; решением пользователя
2026-11-09 утверждены core и текущие production consumers, без расширения границ
поздних этапов. Обязательства интеграции перенесены непосредственно в документы
владельцев ниже: они самостоятельны и не требуют общего verification списка.
Current core реализован и проверен на Linux: [evidence](sv-c003-evidence.md),
[C003 принят пользователем](sv-stage-c003-acceptance.md) 2026-11-09, включая
задачи 001–006, без ожидания late flows. Поздние integrations/исправления
выполняются в соответствующих этапах и не переоткрывают C003 автоматически; allocation,
obligations и registry статусы сохранены.

| Этап | Документ планирования интеграции с C003 | Обязательство |
|---|---|---|
| C006 | [План C006](tasks/c006/README.md) | FI-01: effective map font и reporting |
| C007 | [План C007](tasks/c007/README.md) | FI-02: effective graphics и reporting |
| C014 | [План C014](tasks/c014/README.md) | FI-06: quit/relog parent и independent input-history save |
| C016 | [План C016](tasks/c016/README.md) | FI-03: full recall и occurrence-safe notes/clone |
| C028 | [План C028](tasks/c028/README.md) | FI-04: accepted chat input и account history |
| C029 | [План C029](tasks/c029/README.md) | FI-05/06: реальные exports и exit-log child |
| C032 | [План C032](tasks/c032/README.md) | FI-07: final-review/tomb callers |
| C061 | [План C061](tasks/c061/README.md) | FI-08: screenshot producer и safe publication |

Таблица перечисляет целевые C003 integration/evidence obligations. C006 теперь
имеет полный план ниже; остальные ссылки остаются целевыми дополнениями, пока
их собственный документ не заявит полный scope. При разработке своего этапа читать его
документ и относящиеся tickets; прежний verification список сохраняет только
историческую навигацию. Canonical порядок и prerequisite PASS не изменены.

C004: [спецификация preference loading](sv-stage-c004-spec.md),
[задачи и порядок реализации](tasks/c004/README.md),
[сверка baseline и текущих SV callers](tasks/c004/source-contract.md).
План подготовлен 2026-11-09 по анализу трёх агентов: ровно три outcomes и девять
obligations сохранены в planning snapshot. Предлагается единый SV-local PRF owner
поверх C002/C003; normal load effects/includes и startup order сохраняются.
Current core реализован и проверен на Linux 2026-11-09: [evidence](sv-c004-evidence.md),
[решение и техническая сверка](sv-stage-c004-acceptance.md). Следующим решением
2026-11-09 пользователь завершил и принял C004/001–004 сейчас, поручив перенести
оставшиеся доработки и проверки в [планы следующих этапов](tasks/c004/README.md#самостоятельные-обязательства-следующих-этапов):
C005/C006/C007/C011/C014/C016/C044–C055. Работа обязательна своим owners,
не блокирует и не переоткрывает C004 автоматически; возврат evidence в C004
служит трассировке и не является gate его завершения. Поздние этапы не приняты,
реальные flows не получают PASS по parser/queue evidence. Canonical allocation,
prerequisites и registry statuses не меняются.
Пользователь затем явно подтвердил приёмку C004: «этот этап считаем принятым».

C005: [спецификация базового игрового ввода](sv-stage-c005-spec.md),
[задачи и порядок реализации](tasks/c005/README.md),
[точный planning snapshot](tasks/c005/coverage.json). План подготовлен анализом
трёх агентов 2026-11-09: keymap, quantity и raw-key, ровно три outcomes и девять
obligations. Задачи 001–003 реализуют production primitives, 004 собирает минимальное
Linux evidence. Прежнее C004 keymap addendum включено в полный план; C004 остаётся
accepted, поздние session/macro/action callers сохраняют своих владельцев.
Core C005 и задачи 001–004 реализованы и проверены на Linux в тот же день:
[evidence](sv-c005-evidence.md). [C005 и задачи 001–004 приняты пользователем](sv-stage-c005-acceptance.md)
2026-11-09. Canonical runtime registration остаётся отдельным workflow;
canonical JSON не изменён.

C006: [спецификация шрифтов и подготовки ресурсов](sv-stage-c006-spec.md),
[задачи и порядок реализации](tasks/c006/README.md),
[source contract](tasks/c006/source-contract.md),
[точный planning snapshot](tasks/c006/coverage.json). Полный план подготовлен
анализом трёх агентов 2026-11-09: 11 outcomes, 34 obligations, задачи 001–007.
Existing map resource/preference obligations 001/002 сохранены; добавлены PCF/TTF
loaders, independent selection/preview/Save, startup/live policy, resource
filters/outline/tile-fallback и asset validation. C006 доказывает собственный
production rendering/resource результат, C007 завершает полную игровую композицию
и coupled graphics lifecycle. Linux core реализован агентами и интегрирован 2026-11-09:
[evidence](sv-c006-evidence.md), production font/resource editor на F6. Build,
пять focused ASan/UBSan core checks, existing login parser и native software
startup прошли; 51 fonts и 6 sheets проверены. [C006 и задачи 001–007 приняты
пользователем](sv-stage-c006-acceptance.md) 2026-11-09 с известными ограничениями
TTF ASCII profile и временного F6 UI. Продуктовая замена — C044. Canonical registry
не изменён; backend/profile limits и поздние flows отмечены в evidence.

C007: [спецификация игрового rendering](sv-stage-c007-spec.md),
[задачи и порядок](tasks/c007/README.md),
[source contract](tasks/c007/source-contract.md),
[точный planning snapshot](tasks/c007/coverage.json). Полный план подготовлен
анализом трёх агентов 2026-11-09: 15 outcomes, 45 obligations, задачи 001–007.
Сохранены C003/FI-02 и C004 graphics preference integrations 001/002; добавлены
semantic composition, palette/lighting/animation, geometry/indicators и
weather/effects/raw-picture owners. Текущий C006 bootstrap переиспользуется.
Полный map ingress и позднее packet-derived evidence остаются C013; weather
production decoder входит в C007. Linux core реализован агентами и подключён к
native resource consumer; build, focused ASan/UBSan, editor/login и bounded native
smoke прошли: [evidence и ограничения](sv-c007-evidence.md). [C007 и задачи
001–007 приняты пользователем](sv-stage-c007-acceptance.md) 2026-11-09 в представленных
границах Linux core; live карта остаётся C013. Непроверенные hardware/submission
условия и поздние flows сохраняются; canonical registry не изменён.

C008: [спецификация network controls](sv-stage-c008-spec.md),
[задачи и порядок](tasks/c008/README.md),
[source contract](tasks/c008/source-contract.md),
[точный planning snapshot](tasks/c008/coverage.json). План подготовлен анализом
трёх агентов 2026-11-09: 6 outcomes и 18 obligations. Core реализован агентами и
проверен на Linux: [evidence](sv-c008-evidence.md). Atomic recovery/diagnostic,
actual draw/SDL present, pause freshness, bounded flush pacing и ordered
confirmation проверены через production core. [C008 и задачи 001–004 приняты
пользователем](sv-stage-c008-acceptance.md) в текущих границах core 2026-11-09.
Оставшаяся обязательная работа закреплена в
[C011-002](tasks/c011/SV-C011-002-network-control-preparation.md),
[C013-001](tasks/c013/SV-C013-001-network-control-live-map.md),
[C014-003](tasks/c014/SV-C014-003-network-control-lifecycle.md) и
[C053-001](tasks/c053/SV-C053-001-preference-runtime.md): actual live preparation,
map ingress/submission, lifecycle/deferred callers и macro waits. Она не блокирует
и не переоткрывает C008 автоматически; непроверенные flows не получают PASS.
Canonical allocation и pending runtime statuses сохранены.

C009: [спецификация startup Lua reload](sv-stage-c009-spec.md),
[задачи и ownership](tasks/c009/README.md),
[source contract](tasks/c009/source-contract.md),
[точный planning snapshot](tasks/c009/coverage.json). План подготовлен анализом
трёх агентов 2026-11-10: один outcome и три obligations, задачи 001–003.
Настоящий Lua runtime, overlay, reload lifecycle, Guide metadata и audio restart
warning; wire/file publication остаётся C010, actual preparation — C011.
Пользователь 2026-11-10 выбрал сохранение baseline: reload после последнего END
выполняется и при ошибке сохранения, с отдельными transfer/reload results.
Решение закреплено в source contract. Production core и задачи 001–003
реализованы агентами и проверены на Linux 2026-11-10:
[evidence](sv-c009-evidence.md). Build, одна focused ASan/UBSan проверка и bounded
native early startup прошли. Actual C010/C011/C014 consumers остаются своим
этапам. [C009 и задачи 001–003 приняты пользователем](sv-stage-c009-acceptance.md)
в текущих границах Linux core 2026-11-10. Оставшиеся обязательные работы закреплены
в [C010-001](tasks/c010/SV-C010-001-lua-reload-integration.md),
[C011-003](tasks/c011/SV-C011-003-lua-startup-preparation.md) и
[C014-004](tasks/c014/SV-C014-004-lua-runtime-lifecycle.md), не блокируют и не
переоткрывают C009 автоматически. Pending flows не получают PASS;
canonical runtime registration остаётся отдельным workflow.

C010: [полный план и задачи](tasks/c010/README.md),
[baseline и границы переноса](tasks/c010/source-contract.md).
Пять transfer outcomes, включая upload core и интеграцию принятого Lua owner C009,
реализованы в текущем Linux startup/core 2026-11-10. Production build и связанная
core-проверка прошли: [evidence и disposition 20 obligations](sv-c010-evidence.md).
Пользователь [принял C010 и задачи 001–006](sv-stage-c010-acceptance.md)
2026-11-11 в проверенных границах Linux startup/core. Pending faults и
C011/C014/C041 consumers не получают PASS; canonical allocation/statuses сохраняются.

C011: [полный план и задачи 001–006](tasks/c011/README.md),
[baseline/source contract](tasks/c011/source-contract.md),
[planning snapshot пяти outcomes / 18 obligations](tasks/c011/coverage.json).
Реализован и проверен Linux preparation/core 2026-11-11: profile/resources,
controls/handoff contract, startup readiness/failure, accepted identity/prefix и
redraw. [Evidence всех 18 obligations](sv-c011-evidence.md): 11 native core
cases, production build и isolated actual-server startup/visual check прошли.
Оставшиеся собственные проверки C011 закрыты, найденные дефекты исправлены.
[C011 принят пользователем](sv-stage-c011-acceptance.md) 2026-11-11, включая задачи
001–006 в проверенных границах preparation/core. Actual live adoption остаётся
C014, canonical allocation и pending runtime statuses сохранены.


## Текущая приёмка платформ

Правило [Linux-only проверок](agents/platform-validation.md) применяется к каждой
строке A/B/C001–C064 и ко всем последующим implementation tickets этих slices.
Windows-specific outcomes сохраняют IDs и allocation, но их Windows-сборка и
проверка отложены; они не являются текущим prerequisite/gate для Linux slices.
Windows-only проверочная работа остаётся отложенной, а не принимается без evidence.

## Правила slices

1. Prerequisite другого блока принимается раньше, а не включается скрыто в scope.
2. Повтор namespace продолжает production-путь предыдущего slice.
3. Перенос меняет порядок acceptance, но не behavior description, implementation,
   evidence status или evidence obligations.
4. Сумма C001–C064 обязана быть 863, каждый прежний C–G outcome назначается один
   раз. Каждый Cnnn владеет не более чем 50 outcomes; замороженный B54 является
   явным исключением. A/B rows сохраняются semantic deep-equal.
5. `stageCatalogSha256` ledger связывает schema v3 с exact bytes каталога.

## Late-block policy

После core gameplay намеренно расположены social/party, UI настроек, game options,
alerts, import, macro runtime/management, audio, editable configuration, optional
platform/Lua/INS, OS integration, screenshots и Guide. Ранние settings, files и
preferences только загружают необходимые данные. Визуальная погода относится к
rendering, звуковая — к позднему audio.

## Prerequisite metadata corrections

При reallocation удаляются только hard-dependency edges, которые ошибочно делают
optional invocation/integration prerequisite самого пользовательского outcome:

- из `session.load-profile-input` удаляются `input.macro-*`, `macros.load`,
  `preferences.macro-precedence` и `items.autoinscribe-on-update`; обязательные
  profile/settings/input data остаются;
- `input.macro-match` удаляется из ordinary action outcomes, но сохраняется у
  `macros.*` и внутри C053; `macro-wait/xwait` явно требуют `macro-match`, тогда
  как macro invocation обычного action является альтернативным binding;
- `rendering.weather` больше не зависит от `audio.weather`: оба являются
  независимыми consumers semantic weather state;
- `world.walk/run/open` не зависят от downstream store surface, а
  `world.stay/stay-one` — от downstream pickup interaction;
- `store.service` не зависит от `screenshots.capture`;
- `chat.send/history` не зависят от optional `clipboard.paste`;
- core font select/filter outcomes не зависят от поздней settings transaction UI;
- credential update C037 предшествует account password management C038.

Полный детерминированный список удалённых edges печатает
[`tools/reallocate_functional_stages.py`](../tools/reallocate_functional_stages.py).
Script также проверяет counts каталога, один block на post-B stage, A/B equality и
общие суммы. Stage-order validator дополнительно проверяет, что каждый оставшийся
prerequisite находится в том же или более раннем этапе.

`capability.map.*` означает overview/locate. Основная игровая карта —
`capability.world.read-map`, поэтому она отдельно принята в C013.

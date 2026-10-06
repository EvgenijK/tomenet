# Объём этапа B: реестр, сценарии и зависимости

Дата: 2026-09-23. Исследованный HEAD: `887ac4d215376daaa169afca8216f65c18855107`.
Статус: исследование для подготовки спецификации B; не изменение распределения этапов, не приёмка реализации.

## Результат и границы вывода

В текущем [native ledger](../capabilities/native-coverage.json) **ровно 583 уникальных активных capability IDs с `acceptanceStage=B`**. Все найдены в [manifest](../capabilities/manifest.json), все имеют привязку к [reconciliation](../capabilities/reconciliation.json). Ни один ID при группировке ниже не потерян и не учтён дважды. Полный перечень, прямые prerequisites и источники каждого ID приведены в приложении A; источник распределения — сам ledger, а не предположение по имени семейства.

Это **583 назначенных результата, а не 583 самостоятельных тикета и не доказанный минимальный объём B**. Основная сессия, создание персонажа, живая карта/HUD, движение, чат и завершение сессии соответствуют [согласованному плану](../../.scratch/single-window-sdl3-client/migration-sequence.md); его сохраняемый в Git текст — [item-policy, Acceptance sequence](../capabilities/item-policy.md#acceptance-sequence). По коду подтверждён ряд нетривиальных ранних зависимостей: Guide при создании, враждебная цель из запроса направления, pickup/store-переходы, read-only вещи/character/messages после смерти, character OPT до отправки опций и FILE→Lua reload. Но **полное редактирование/запись макросов в B остаётся несогласованным с явной формулировкой E в плане**; отсутствие обратных рёбер не объясняет такое продвижение. До окончательной спецификации нужна отдельная сверка этого назначения и широты settings/option claims. Реестр здесь сохранён без изменений.

Исследование выполнено по локальным первичным исходникам, canonical JSON и утверждённым policy snapshots. Внешний web не нужен: вопрос касается конкретного checkout и зафиксированных ревизий. Исходный обзор [baseline](../capabilities/inventories/baseline.md) использован как указатель, а его старые строки и архитектурные предложения не подменяют текущие policy и [AGENTS.md](../../AGENTS.md).

## Что проверено механически, а что вручную

| Проверка | Результат и предел |
|---|---|
| Отбор B, существование, уникальность, lifecycle | 583/583, все active; распределение A=8, B=583, C=165, D=156, E=11, F=2 |
| Прямые зависимости B | 607 рёбер: 591 к B, 16 к A; неизвестных/поздних зависимостей и циклов нет |
| Пустой список prerequisites | 237 B; это отсутствие записанного ребра, а не доказательство независимой реализации |
| Транзитивное замыкание всех B | 585 IDs: B и только два явно достигнутых A; регрессии остальных A всё равно требуются кумулятивным планом |
| Привязка к инвентарям | У всех 583 есть reconciliation mapping; 303 также перечислены в десяти cross-cutting scenarios; отсутствие такого scenario у остальных 280 само по себе не пропуск capability |
| Production validator | `valid`, source verification `checked`, completeness `complete`: 24 inventories, 2077 rows, 0 unresolved; `historyChecked=false` |
| Прямые источники capabilities B | 395 source records, 22 разных путя, 37 пар revision/path; SHA-256 совпал у всех, anchor найден у всех |
| Историческое содержимое | Для 392 source records прочитан `git show revision:path`; 3 policy snapshots прочитаны из рабочего дерева и проверены по recorded SHA-256, поскольку пути отсутствуют в указанной стартовой ревизии |
| Ручная семантическая проверка | Выбранные критические переходы и границы в таблице ниже; полного ручного прочтения всех 583 поведений/всех ветвей не выполнено |
| Runtime | Не запускались real-server login/gameplay, GUI, Windows или новые production-сценарии. Ни один B этим документом не принимается |

Числа графа получены независимым проходом по `coverage[].prerequisites`; source audit восстановил код соответствующих ревизий, а не перенёс на него строки текущего файла. Совпадение anchor лишь устанавливает место поиска: некоторые anchors встречаются сначала в dispatch table, поэтому оно не доказывает наличие тела нужного обработчика или полноту поведения. Для ручных выводов ниже прочитаны именно тела/ветви.

`historyChecked=false` означает отсутствие сравнения с предыдущим manifest, а не провал source validation. Три snapshots — `source.policy.session`, `source.acceptance.items-allocation`, `source.acceptance.settings-policy-f2aa80`. Сочетание текущего SHA и записанной revision здесь намеренно нужно: [README](../capabilities/README.md) допускает source identity рабочего дерева, которую одна git revision не описывает. Это ограничение воспроизведения через один `git show`, не объявление недействительными утверждённых решений.

В canonical ledger все 583 B остаются `implementation=pending`, `evidenceStatus=pending`, `evidenceIds=[]`. Нулевое число accepted canonical claims в validator не отменяет отдельную [приёмку A](../sv-stage-a-acceptance.md): это разные документы/evidence scopes.

## Пользовательские сценарии

Группы — рабочая организация исследования. У каждого ID ровно одна основная группа; реальные сценарии используют IDs из нескольких групп. Например, создание включает G03, G02, G10, G11, G01 и G04, а не только birth.

| Группа | Пользовательский результат | B IDs | Главные связи |
|---|---|---:|---|
| [G01](#g01) | Запустить и настроить собственный профиль SV | 70 | G10/G12/G14; secure credential import → G02; граница полного редактора требует уточнения |
| [G02](#g02) | Выбрать сервер и войти в учётную запись | 25 | G01/G10/G12; login rejection, byte limits, provider failures |
| [G03](#g03) | Выбрать или создать персонажа | 30 | G02/G04/G10/G11/G12; DNA, backtracking и помощь |
| [G04](#g04) | Загрузить данные, войти в мир и поддерживать соединение | 23 | G01/G02/G03/G05/G10/G12; transport, partial packets, FILE/Lua |
| [G05](#g05) | Видеть живую карту, персонажа и предупреждения | 64 | G01/G04/G13/G14; normal/big map, geometry и ordered alerts |
| [G06](#g06) | Передвигаться, исследовать карту и выбрать направление | 32 | G04/G05/G07/G08/G10; get_dir → target, nested chat |
| [G07](#g07) | Подобрать предмет, прочитать вещи и войти/выйти из магазина | 15 | G06/G09/G10/G14; pickup/store children и final review |
| [G08](#g08) | Общаться и читать историю сообщений | 19 | G04/G10/G12; history, byte limits, item substitution |
| [G09](#g09) | Пережить завершение сессии и просмотреть итог | 17 | G02/G04/G07/G08/G12/G13; death/quit/reconnect cleanup |
| [G10](#g10) | Вводить команды, загружать и редактировать макросы | 49 | G01/G03/G06/G08; runtime loading отдельно от спорного полного editing/wizard |
| [G11](#g11) | Получить справку и вернуться к исходному экрану | 16 | G03/G08/G12; bookmark/resource state и возврат в birth |
| [G12](#g12) | Сохранять пользовательские файлы и открывать результаты | 22 | G01/G02/G09/G11; own/shared paths, dump, screenshot, associations |
| [G13](#g13) | Слышать игровые события и менять воспроизведение | 17 | G04/G05/G09/G12/G14; ordered event vs state и устройство |
| [G14](#g14) | Применить игровые и визуальные OPT-параметры | 184 | G01/G04/G05/G06/G07/G08/G13; load/send ≠ проверка каждого позднего consumer |

Группировка наследует **фактический широкий B**. В частности, в G14 — все 184 B-опции, в G10 — 35 macro outcomes вместе с input/preferences. Их наличие в таблице не разрешает противоречие с поздними этапами и не означает, что все они нужны первому playable slice. Точные условия/ветви, scope и obligations остаются в JSON, на который ссылается каждая строка приложения.

## Первичные исходники: вручную проверенные зависимости

Ниже revision — версия прочитанного исходника. Ссылки открывают файл в checkout; **номера строк относятся к названной revision**, воспроизводить через `git show REV:PATH`. Приложение B содержит полный каталог источников, SHA и anchors для всех B.

| Проверенный переход | Первичное доказательство | Вывод для B и предел проверки |
|---|---|---|
| Startup → profile/options → MOTD → play | `0ef6b36`, [c-init.c](../../src/client/c-init.c), строки 4340–4535: `Net_verify`, `Net_setup`, `Net_login`, `initialize_main_pref_files`, character `.opt`, `Send_options`, `get_char_info`, `show_motd`, `Net_start` | OPT нужен до первого `Send_options`; MOTD до `Net_start`; RETRY_LOGIN пропускает повторный MOTD; history/bookmarks грузятся в startup. Прочитан порядок, не все проверки каждого login packet |
| Направление → hostile target | `0ef6b36`, [c-cmd.c](../../src/client/c-cmd.c), 528–554 (`cmd_walk`, `cmd_run`); [c-util.c](../../src/client/c-util.c), 3672–3706 (`get_dir`) | `*` действительно вызывает `cmd_target`; `-`/`+` дают 10/11, обычное направление идёт через keymap. Target в B обоснован; это не принимает C spell/fire callers |
| Создание → Guide | `0ef6b36`, [c-birth.c](../../src/client/c-birth.c), 512,770,996,1425,1605,1800,1952 | Прямые вызовы `cmd_the_guide` из race/trait/class/stats/mode/body. Read/search/navigation/возврат нужны раннему caller; глубина всех внутренних Guide ветвей здесь не проверена |
| FILE → Lua/guide/audio effects | `33d3582`, [nclient.c](../../src/client/nclient.c), 477–648, `Receive_file` | INIT/DATA/END/CHECK/SUM/ACK/ERR; rollback неполного заголовка/data; checksum branch по server version; последний END вызывает `reopen_lua`, обновляет guide metadata, предупреждает об audio update. Прямые transfer/Lua dependencies реальны даже при пустых prerequisites у startup-transfer |
| Pickup → confirm → one send | `33d3582`, [nclient.c](../../src/client/nclient.c), 5956–5969, `Receive_pickup_check` | Подтверждение вызывает `Send_stay`, отказ не посылает его. Нужен отдельный caller test, не только общий check primitive |
| Store arrival → ordinary/special surface | `33d3582`, [nclient.c](../../src/client/nclient.c), 5445–5498, `Receive_store_info`, `Receive_store_kick` | Три layouts; отрицательный `num_items` выбирает special store; kick выставляет leave и может сбросить safe macro. B entry/read/leave не принимает сделки C или special canvas D; fixture должен назвать достигнутую ветвь |
| Death/retirement → tomb → final children | `33d3582`, [nclient.c](../../src/client/nclient.c), 2004–2110, `Receive_quit`, `Receive_relogin`; `0ef6b36`, [c-util.c](../../src/client/c-util.c), 17707–17962, `c_close_game` | Reason определяет tomb; final scene требует `0`; tomb поддерживает dump и screenshot; финальный список открывает character/inventory/equipment/messages/chat history. Native children и точный возврат нужны B; ghost powers отсюда не следуют |
| Resize → negotiated dimensions | `33d3582`, [main-sdl3.c](../../src/client/main-sdl3.c), 4847–4915, `resize_term_with_window` | Изменяет screen dimensions и big-map projection, отправляет `Send_screen_dimensions` в игре. В SV переносится wire/game semantics, не Term topology; geometry задаёт policy |
| Macro editor/wizard → later commands | `33d3582`, [c-util.c](../../src/client/c-util.c), 7296–7341,8226–8264,8574–8768 и просмотр индекса ветвей 8937–9341 | Recording запускается из editor; wizard предлагает fire/throw, school/mimic/runes, items и вызывает Lua spell lookup. Код подтверждает зависимость полного результата от поздних действий, но не необходимость всего editor для B |
| Option consumer → transaction | `33d3582`, [nclient.c](../../src/client/nclient.c), 5500–5513, `Receive_sell`; [c-tables.c](../../src/client/c-tables.c), `no_verify_sell` | Option влияет на продажу/подтверждение; загрузка/передача значения в B не доказывает consumer C. Этот пример требует ограничить ранний claim или сдвинуть полный outcome, а не молча считать parity |
| Network framing/unknown packet | `a3d99ee`, [nclient.c](../../src/client/nclient.c), 1843–1873, `Net_packet` | Неизвестный type вызывает report/redraw и clear input buffer; receiver вызывается под rollback lock. Чтение данного фрагмента не считается аудитом всех handlers, malformed/fragmentation ветвей |

Сопоставление также использовало [session reconciliation](../capabilities/session-reconciliation.md#allocation-and-cross-domain-handoff), [settings reconciliation](../capabilities/settings-reconciliation.md#authority-and-allocation) и [complete reconciliation](../capabilities/complete-reconciliation.md#cross-cutting-scenarios-and-stage-prerequisites). Это полезные явные handoff-решения, однако их собственное назначение `B` не является независимым доказательством необходимости всех расширений B.

## Границы B, которые необходимо уточнить перед фиксацией спецификации

### 1. Полный macro editor/recording/wizard: подтверждённое расхождение документов

[План](../capabilities/item-policy.md#stage-detail) называет B keyboard/macro routes соответствующих flows, C — load/play/waits и multi-step chains соответствующих действий, E — editing/recording. Ledger помещает в B `capability.macros.create`, `capability.macros.record`, `capability.macros.edit-action`, `capability.macros.wizard` и другие editor outcomes. У этих четырёх IDs нет prerequisites и нет входящих explicit prerequisites во всём ledger. Scope wizard буквально требует всех поддержанных ветвей, item/spell selection и исполнения сериализованной последовательности; source показывает поздние combat/spell ветви.

Ни найденный startup caller, ни граф не обосновывают полный editor как prerequisite раннего B. Требуется зафиксировать одно из решений **до приёмки B**: обоснованное более раннее полное выполнение; либо корректировка stage для editor outcomes при сохранении B load/play/encoding; либо выделение действительно самостоятельного раннего outcome и поздних caller outcomes без потери IDs/обязательств. Здесь не назначается новый этап автоматически и не объявляется пользовательское одобрение расширения.

### 2. Все опции и полный settings workflow: широта результата больше startup

Загрузка своих CFG/OPT, stock defaults, выбранных ресурсов, предельных значений, permitted macro effects и отправка options нужны startup. Это подтверждается кодом и [persistence policy](../capabilities/settings-policy.md#preference-load-order-and-boundaries). Но `settings.preview/save/cancel/dirty-close/save-conflict`, imports и 184 option outcomes содержат самостоятельные UI/file/consumer obligations, которые шире минимального load/send.

Показательный случай `options.no-verify-sell`: в B сохранена формулировка семантического эффекта пропуска подтверждения продажи, а transaction — C. Поэтому таблица не должна превращаться в «184 flags load успешно → 184 outcomes accepted». При декомпозиции для каждого подобного outcome нужны либо ранний реальный caller, либо явная граница раннего/полного результата в canonical registry. `settings.preview` также не имеет prerequisites или dependents в ledger. Полный аудит consumers всех 184 опций ещё не выполнен; это конкретная работа сверки, а не доказательство 184 ошибочных назначений.

### 3. Граф валиден, но не является полным порядком разработки

Пример скрытых связей: `session.transfer-startup-files` имеет `[]`, хотя source вызывает filesystem/Lua/guide и выдаёт предупреждение об audio update. `session.enter-game` зависит одновременно от `character.select-existing` и `birth.complete`: это требования готовности альтернативных сценариев, а не обязательное последовательное создание нового персонажа при каждом входе. Аналогично пустые edges у resources/editors не подтверждают готовность самостоятельного тикета. Для рабочего backlog нужны lifecycle/ownership и scenario edges сверх имеющихся capability prerequisites, с разграничением AND/альтернативы/platform condition. Независимый обход даёт 50 строгих транзитивных prerequisites для `session.enter-game` и 68 для `world.walk` (без самого root); ни в одном замыкании нет `settings.*`, `rendering.*` или `transfer.*`, хотя есть `session.transfer-startup-files`. Для `session.review-final-state` строгое замыкание содержит 31 prerequisite. Эти числа описывают записанный DAG, а не достаточный набор production-модулей.

### 4. Child/fallback boundaries требуют именованных сценариев

Согласованный план разрешает development fallback только для зарегистрированных будущих outcomes. B движение может привести к pickup/store; B death открывает read-only children; birth открывает Guide. Их нельзя объявить B-complete через неявный fallback. Но ordinary store entry не принимает buy/sell C, а arrival в special store не принимает D canvas. Точно обозначить child outcome, replacement stage, parent restoration и teardown; accepted B path, вошедший в fallback, проваливает свою проверку. Источники: [transition contract](../capabilities/item-policy.md#nativefallback-transition-and-retirement), `Receive_store_info`, `c_close_game` выше.

### 5. Общие источники не являются готовыми test cases

Много source anchors указывает на весь `interact_macros`, `cmd_message`, `Receive_*` либо строку таблицы options; conditions нередко требуют «all version/build branches» без отдельного перечня сценариев. В приложении сохранены эти прямые sources, однако это traceability, не полное ручное доказательство. Перед реализацией каждого среза следует выписать применимые packet layouts, input/cancel/parent transitions, build/runtime gates и byte limits из его production source; сверять с source-field/inventory mapping. [Acceptance contract](../../.scratch/single-window-sdl3-client/acceptance-contract.md#evidence-architecture) требует эти проверки даже при валидном registry.

## Нормативные зависимости и приёмка, которые нельзя отложить

- **Изоляция SV.** Реализация и adaptation по возможности внутри SV; допускается локальное дублирование. Старые предложения baseline про обязательный shared refactor не применяются поверх [AGENTS.md](../../AGENTS.md) и [архитектуры](../sv-architecture.md). Тесты вызывают production SV path; это исследование не предлагает отдельную test-only реализацию.
- **Persistence/credentials.** Собственные `U/sv` CFG/OPT/history/bookmarks; shared macro/INS/DNA/resources/notes/exports согласно [ownership](../capabilities/settings-policy.md#ownership-and-paths). Секрет не попадает в CFG/history/diagnostics; Linux Secret Service и Windows generic Credential Manager, exact byte identity, provider/session lifecycle из [session policy](../capabilities/session-policy.md#original-scratchsingle-window-sdl3-clientissues24-define-credential-storage-policymd). Save/Cancel/import/disk conflicts принимаются только с реальными выбранными scopes.
- **Text/bytes.** Unicode UI draft, baseline bytes и glyph IDs различаются. Нельзя подменять byte limits UI character count либо нормализовать password. Producers login/CLI/import/clipboard проверяются отдельно по [field-boundary policy](../capabilities/session-policy.md) и [encoding policy](../capabilities/settings-policy.md#encoding-and-glyph-policy-xhtml-subsequently-excluded).
- **Карта.** Полная сетка 66×22/66×44; максимальное вписывание, без пользовательского map zoom/crop/scroll. UI text scale отдельно; saved SV layout определяет normal/big и wire projection. В [settings policy](../capabilities/settings-policy.md) поздняя final persistence формулировка явно заменяет исторические mentions zoom. Погодные слои/палитра/lighting, numeric glyph identity, requested/effective resources и generation/lifecycle нужны для соответствующих B claims.
- **Evidence.** Реальный test server и round trip; split/chained packets без преждевременных эффектов; ordered occurrences/replies; input normal/roguelike/macro, cancel/focus/parent; relog/teardown удаляют stale requests/commands. Применяются scoped Linux software/accelerated, MinGW/Wine и **реальные Windows 10/11 в B**, Windows software и доступное acceleration. Wine не заменяет Windows acceptance. [Acceptance](../../.scratch/single-window-sdl3-client/acceptance-contract.md) также задаёт geometry matrix, submission gates 20/50/200 ms и короткие lifecycle/concurrency cases; stress/soak не добавляются.
- **Регрессии A.** Проверять ранее принятые capabilities кумулятивно; live HP B не равен synthetic HP A. UI/HTML approval не доказывает native behavior; missing HTML не блокирует native implementation. [Complete reconciliation](../capabilities/complete-reconciliation.md#new-outcomes-overlaps-and-corrections), [migration sequence](../capabilities/item-policy.md#approved-framework).

## Рекомендуемый порядок подготовки и реализации

Это рекомендация исследования, не уже согласованный новый план и не автоматическое изменение stage allocation.

1. **Закрыть две границы scope:** macro editor/wizard/recording и option/settings claims с поздними consumers. Сохранить все текущие IDs; явные изменения allocation/obligations оформить отдельно до заявления полноты B. Эти вопросы не мешают начать бесспорные startup primitives.
2. **Описать foundation для реального входа:** isolated profile/path/resource defaults, byte fields, input and secret provider interfaces, transport framing, session teardown. Использовать production SV seams A и регрессии A; не считать standalone mocks готовой сессией.
3. **Первый вертикальный срез:** server → contact → existing account → existing character → profile/options + требуемый FILE/Lua → MOTD → initial map/live HUD → clean disconnect. Включить все реально приходящие обязательные packet handlers; future receivers/children классифицировать явно. Провести real-server round trip и сохранить evidence scoped к этому срезу, не ко всем 583.
4. **Второй сценарий входа:** new account, character slots/reordering, creation/backtracking/DNA/context Guide, rejection/retry и byte/provider failures. Вернуть фокус в точный шаг после Guide/cancel.
5. **Игровой цикл B:** normal/big map и resize negotiation, движение/target prompts/overview/locate, pickup и entry/read/leave магазина, live messages/chat/history/clipboard, warnings/audio; сохранить очередность и работу сети при overlays.
6. **Завершение и восстановление:** quit, disconnect/portal/reconnect, death/final scene/tomb/dump/read-only final review; очистка session identity, pending requests, macro queue и late provider results.
7. **Остальной подтверждённый B scope:** согласованные settings/options/imports/editor/file/export/resource outcomes по таблице, caller-specific tests и полный применимый platform/geometry matrix. Затем checkpoint report B с new claims, A regressions, limitations/fallback inventory и human review.

Номер шага не заменяет dependency graph. Данные для первых тикетов: входные IDs из приложения, ссылки источников, конкретный caller, ветви success/cancel/error, owned state/path, необходимые предыдущие slices и проверяемый результат. Отдельных задач реализации, изменений canonical JSON или legacy/SV кода в этом исследовании нет.

## Воспроизведение машинной проверки

Команда выполнена из корня репозитория с уже существующим окружением `jsonschema`:

```sh
/tmp/sv15-venv/bin/python tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger docs/capabilities/native-coverage.json \
  --inventory-index docs/capabilities/inventories/index.json \
  --reconciliation docs/capabilities/reconciliation.json \
  --source-root tomenet=. \
  --evidence docs/capabilities/native-evidence.json
```

Путь venv — окружение этой проверки, не repository API. Для другого checkout нужны зависимости [requirements-capabilities.txt](../../tools/requirements-capabilities.txt). `--previous-manifest` здесь не передавался. Интерфейс команды описан в [validator](../../tools/validate_capabilities.py); report был сохранён во временный файл и его результаты зафиксированы выше. Временные файлы не являются источником контракта.

Зафиксированные SHA-256 исходных canonical данных:

| Файл | SHA-256 |
|---|---|
| [docs/capabilities/manifest.json](../../docs/capabilities/manifest.json) | `57f4b26e162899c18697d044cdcc3788bd73eb96fe96d28bd9c1049f5c872d5c` |
| [docs/capabilities/native-coverage.json](../../docs/capabilities/native-coverage.json) | `fbc21a95e1da9fcc77e255ebfbd54595b7e648cbf54be7751bb6bea61de97d5a` |
| [docs/capabilities/reconciliation.json](../../docs/capabilities/reconciliation.json) | `95faec7659c989274994fc207117dc73e68f1fb4019d1c732d5b014c0d9f1d1e` |

Короткая независимая проверка denominator и stage edges:

```python
import json
from collections import Counter
m = json.load(open("docs/capabilities/manifest.json"))
n = json.load(open("docs/capabilities/native-coverage.json"))
by_id = {x["capabilityId"]: x for x in n["coverage"]}
b = [x for x in n["coverage"] if x["acceptanceStage"] == "B"]
ids = {x["capabilityId"] for x in b}
assert len(b) == len(ids) == 583
assert ids <= {x["id"] for x in m["capabilities"] if x["lifecycle"] == "active"}
assert all(p in by_id and by_id[p]["acceptanceStage"] <= "B"
           for x in b for p in x["prerequisites"])
print(Counter(by_id[p]["acceptanceStage"] for x in b for p in x["prerequisites"]))
```

Исторический primary source: `git show 33d3582f8b73d5a853106bf7837ca28285bf9052:src/client/nclient.c`.
Проверка текущего source hash и наличие anchor не заменяют перечисленные ручные чтения. В таблице источников `worktree` означает только три policy snapshots, описанные выше.

## Приложение A. Полное распределение 583 IDs

В таблицах `prerequisites` воспроизведены **точно**, без добавления подразумеваемых связей. `—` означает пустой список ledger. Имена prerequisites даны полностью. Ссылки ID ведут к строке native ledger; заголовки результатов берутся из manifest. `Snnn` — полный набор прямых `capability.sources`, каталог в приложении B; дополнительные conditions/evidence sources и obligations доступны в JSON.

**Глубина:** для каждого ID выполнена машинная сверка presence/lifecycle/allocation/prerequisites/source content/anchor и отнесение к основной группе. Ручной статус каждого ID по умолчанию — **не проверены все ветви**; только конкретные переходы из раздела первичных исходников выше просмотрены семантически. По группе или общей функции нельзя распространять это на все её IDs. Приложение не является матрицей успешной приёмки.

<a id="g01"></a>

### G01. Запустить и настроить собственный профиль SV — 70

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.settings.load`](../capabilities/native-coverage.json#L30379) — settings.load | — | S090, S089 |
| [`capability.settings.defaults`](../capabilities/native-coverage.json#L30423) — settings.defaults | — | S090, S089 |
| [`capability.settings.parse`](../capabilities/native-coverage.json#L30467) — settings.parse | — | S090, S089 |
| [`capability.settings.cli-overrides`](../capabilities/native-coverage.json#L30511) — settings.cli-overrides | — | S090, S089 |
| [`capability.settings.preview`](../capabilities/native-coverage.json#L30555) — settings.preview | — | S090, S089 |
| [`capability.settings.save`](../capabilities/native-coverage.json#L30599) — settings.save | — | S090, S089, S310, S311 |
| [`capability.settings.cancel`](../capabilities/native-coverage.json#L30649) — settings.cancel | — | S090, S089 |
| [`capability.settings.dirty-close`](../capabilities/native-coverage.json#L30693) — settings.dirty-close | — | S090, S089 |
| [`capability.settings.exit-unsaved`](../capabilities/native-coverage.json#L30737) — settings.exit-unsaved | — | S090, S089 |
| [`capability.settings.save-failure`](../capabilities/native-coverage.json#L30781) — settings.save-failure | — | S090, S089 |
| [`capability.settings.save-conflict`](../capabilities/native-coverage.json#L30825) — settings.save-conflict | — | S090, S089 |
| [`capability.settings.save-global-options`](../capabilities/native-coverage.json#L30869) — settings.save-global-options | — | S090, S089 |
| [`capability.settings.save-class-options`](../capabilities/native-coverage.json#L30913) — settings.save-class-options | — | S090, S089 |
| [`capability.settings.save-named-options`](../capabilities/native-coverage.json#L30957) — settings.save-named-options | — | S090, S089 |
| [`capability.settings.aliases`](../capabilities/native-coverage.json#L31001) — settings.aliases | — | S090, S089, S276 |
| [`capability.imports.offer`](../capabilities/native-coverage.json#L40845) — imports.offer | — | S278, S089 |
| [`capability.imports.select`](../capabilities/native-coverage.json#L40889) — imports.select | — | S278, S089 |
| [`capability.imports.conflicts`](../capabilities/native-coverage.json#L40933) — imports.conflicts | — | S278, S089 |
| [`capability.imports.commit`](../capabilities/native-coverage.json#L40977) — imports.commit | — | S278, S089 |
| [`capability.imports.reimport`](../capabilities/native-coverage.json#L41021) — imports.reimport | — | S278, S089 |
| [`capability.imports.resources`](../capabilities/native-coverage.json#L41065) — imports.resources | — | S278, S089 |
| [`capability.imports.credentials`](../capabilities/native-coverage.json#L41109) — imports.credentials | — | S278, S089 |
| [`capability.imports.cancel`](../capabilities/native-coverage.json#L41153) — imports.cancel | — | S278, S089 |
| [`capability.settings.slash-enable`](../capabilities/native-coverage.json#L45595) — settings.slash-enable | — | S298, S089, S325 |
| [`capability.settings.slash-disable`](../capabilities/native-coverage.json#L45650) — settings.slash-disable | — | S298, S089, S325 |
| [`capability.settings.slash-toggle`](../capabilities/native-coverage.json#L45705) — settings.slash-toggle | — | S298, S089, S325 |
| [`capability.settings.local-pickup`](../capabilities/native-coverage.json#L45760) — settings.local-pickup | — | S298, S089 |
| [`capability.settings.local-destroy`](../capabilities/native-coverage.json#L45804) — settings.local-destroy | — | S298, S089 |
| [`capability.settings.local-destroy-unmatched`](../capabilities/native-coverage.json#L45848) — settings.local-destroy-unmatched | — | S298, S089 |
| [`capability.configuration.nick`](../capabilities/native-coverage.json#L45892) — configuration.nick | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.name`](../capabilities/native-coverage.json#L45939) — configuration.name | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.meta`](../capabilities/native-coverage.json#L45986) — configuration.meta | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.server`](../capabilities/native-coverage.json#L46033) — configuration.server | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.port`](../capabilities/native-coverage.json#L46080) — configuration.port | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.realname`](../capabilities/native-coverage.json#L46127) — configuration.realname | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.path`](../capabilities/native-coverage.json#L46174) — configuration.path | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.fullauto`](../capabilities/native-coverage.json#L46221) — configuration.fullauto | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.fps`](../capabilities/native-coverage.json#L46268) — configuration.fps | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.lighterdarkblue`](../capabilities/native-coverage.json#L46315) — configuration.lighterdarkblue | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.colormap--i`](../capabilities/native-coverage.json#L46362) — configuration.colormap--i | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.graphicsforceoutline`](../capabilities/native-coverage.json#L46409) — configuration.graphicsforceoutline | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.graphics`](../capabilities/native-coverage.json#L46456) — configuration.graphics | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.graphic-tiles`](../capabilities/native-coverage.json#L46503) — configuration.graphic-tiles | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.graphic-tiles-i`](../capabilities/native-coverage.json#L46550) — configuration.graphic-tiles-i | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.windowdecorations`](../capabilities/native-coverage.json#L46597) — configuration.windowdecorations | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.sound`](../capabilities/native-coverage.json#L46644) — configuration.sound | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.hintsound`](../capabilities/native-coverage.json#L46691) — configuration.hintsound | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.cacheaudio`](../capabilities/native-coverage.json#L46738) — configuration.cacheaudio | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiosamplerate`](../capabilities/native-coverage.json#L46785) — configuration.audiosamplerate | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiochannels`](../capabilities/native-coverage.json#L46832) — configuration.audiochannels | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiobuffer`](../capabilities/native-coverage.json#L46879) — configuration.audiobuffer | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.soundpackfolder`](../capabilities/native-coverage.json#L46926) — configuration.soundpackfolder | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.soundpacksubset`](../capabilities/native-coverage.json#L46973) — configuration.soundpacksubset | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.musicpackfolder`](../capabilities/native-coverage.json#L47020) — configuration.musicpackfolder | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.musicpacksubset`](../capabilities/native-coverage.json#L47067) — configuration.musicpacksubset | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiomaster`](../capabilities/native-coverage.json#L47114) — configuration.audiomaster | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiomusic`](../capabilities/native-coverage.json#L47161) — configuration.audiomusic | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiosound`](../capabilities/native-coverage.json#L47208) — configuration.audiosound | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audioweather`](../capabilities/native-coverage.json#L47255) — configuration.audioweather | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiovolumemaster`](../capabilities/native-coverage.json#L47302) — configuration.audiovolumemaster | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiovolumemusic`](../capabilities/native-coverage.json#L47349) — configuration.audiovolumemusic | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiovolumesound`](../capabilities/native-coverage.json#L47396) — configuration.audiovolumesound | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.configuration.audiovolumeweather`](../capabilities/native-coverage.json#L47443) — configuration.audiovolumeweather | `capability.settings.load`<br>`capability.settings.save` | S090, S089 |
| [`capability.settings.layout`](../capabilities/native-coverage.json#L47490) — settings.layout | `capability.settings.save` | S299, S089 |
| [`capability.settings.window-mode`](../capabilities/native-coverage.json#L47536) — settings.window-mode | `capability.settings.save` | S299, S089 |
| [`capability.settings.ui-scale`](../capabilities/native-coverage.json#L47582) — settings.ui-scale | `capability.settings.save` | S299, S089 |
| [`capability.settings.panel-widths`](../capabilities/native-coverage.json#L47628) — settings.panel-widths | `capability.settings.save` | S299, S089 |
| [`capability.settings.block-visibility`](../capabilities/native-coverage.json#L47674) — settings.block-visibility | `capability.settings.save` | S299, S089 |
| [`capability.settings.widget-order`](../capabilities/native-coverage.json#L47720) — settings.widget-order | `capability.settings.save` | S299, S089 |
| [`capability.settings.palette-edit`](../capabilities/native-coverage.json#L47856) — settings.palette-edit | — | S301, S089 |

<a id="g02"></a>

### G02. Выбрать сервер и войти в учётную запись — 25

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.connection.select-server`](../capabilities/native-coverage.json#L184) — Choose advertised server | — | S002, S001 |
| [`capability.connection.enter-host`](../capabilities/native-coverage.json#L228) — Enter an explicit server | — | S002, S013, S001 |
| [`capability.connection.cancel-host`](../capabilities/native-coverage.json#L275) — Cancel manual server entry | — | S002, S001 |
| [`capability.connection.contact`](../capabilities/native-coverage.json#L319) — Connect to selected endpoint | `capability.connection.enter-host` | S013, S016, S017, S001 |
| [`capability.connection.contact-failure`](../capabilities/native-coverage.json#L371) — Read contact or setup failure | — | S013, S016, S017, S001 |
| [`capability.account.enter-name`](../capabilities/native-coverage.json#L421) — Enter account identity | `capability.input.text-edit` | S003, S039, S001 |
| [`capability.account.cancel-name`](../capabilities/native-coverage.json#L470) — Exit account entry | — | S003, S001 |
| [`capability.account.enter-password`](../capabilities/native-coverage.json#L514) — Enter private login password | `capability.account.enter-name`<br>`capability.account.secret-provider-failure`<br>`capability.credentials.lookup`<br>`capability.credentials.private-input`<br>`capability.input.text-edit` | S004, S001 |
| [`capability.account.cancel-password`](../capabilities/native-coverage.json#L564) — Return from password to account name | — | S004, S001 |
| [`capability.account.reject-unencodable-password`](../capabilities/native-coverage.json#L608) — Reject an unrepresentable credential | — | S016, S024, S001 |
| [`capability.account.authenticate`](../capabilities/native-coverage.json#L655) — Authenticate an existing account | `capability.account.enter-password`<br>`capability.account.restore-secret`<br>`capability.account.save-secret`<br>`capability.connection.contact` | S016, S014, S039, S001 |
| [`capability.account.create`](../capabilities/native-coverage.json#L710) — Create a new account | `capability.account.enter-password`<br>`capability.account.save-secret`<br>`capability.connection.contact` | S003, S039, S014, S001 |
| [`capability.account.login-rejected`](../capabilities/native-coverage.json#L764) — Recover from rejected authentication | — | S014, S039, S001 |
| [`capability.account.restore-secret`](../capabilities/native-coverage.json#L811) — Restore saved login secret | — | S004, S001 |
| [`capability.account.save-secret`](../capabilities/native-coverage.json#L855) — Save an authenticated secret | `capability.account.secret-provider-failure`<br>`capability.credentials.lookup`<br>`capability.credentials.private-input` | S016, S001 |
| [`capability.account.secret-provider-failure`](../capabilities/native-coverage.json#L903) — Continue with a session-only credential | — | S004, S001 |
| [`capability.account.read-information`](../capabilities/native-coverage.json#L947) — Read account information | — | S023, S014, S001 |
| [`capability.account.change-password`](../capabilities/native-coverage.json#L994) — Submit password change | `capability.account.authenticate`<br>`capability.account.save-secret`<br>`capability.credentials.change-write` | S023, S024, S040, S001 |
| [`capability.account.cancel-password-change`](../capabilities/native-coverage.json#L1048) — Cancel password change | — | S023, S001 |
| [`capability.account.password-change-failure`](../capabilities/native-coverage.json#L1092) — Handle password change rejection | — | S023, S024, S040, S001 |
| [`capability.credentials.lookup`](../capabilities/native-coverage.json#L41749) — credentials.lookup | `capability.credentials.provider-linux`<br>`capability.credentials.provider-windows` | S281, S089 |
| [`capability.credentials.provider-linux`](../capabilities/native-coverage.json#L41796) — credentials.provider-linux | — | S281, S089 |
| [`capability.credentials.provider-windows`](../capabilities/native-coverage.json#L41839) — credentials.provider-windows | — | S281, S089 |
| [`capability.credentials.private-input`](../capabilities/native-coverage.json#L41882) — credentials.private-input | — | S281, S089 |
| [`capability.credentials.change-write`](../capabilities/native-coverage.json#L41926) — credentials.change-write | — | S281, S089 |

<a id="g03"></a>

### G03. Выбрать или создать персонажа — 30

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.character.read-overview`](../capabilities/native-coverage.json#L1142) — Read available characters and slots | `capability.account.authenticate` | S014, S001 |
| [`capability.character.select-existing`](../capabilities/native-coverage.json#L1188) — Enter an existing character | `capability.character.read-overview` | S014, S001 |
| [`capability.character.create-ordinary`](../capabilities/native-coverage.json#L1234) — Start ordinary character creation | `capability.character.read-overview` | S014, S001 |
| [`capability.character.create-exclusive`](../capabilities/native-coverage.json#L1280) — Start slot-exclusive character creation | `capability.character.read-overview` | S014, S012, S001 |
| [`capability.character.name`](../capabilities/native-coverage.json#L1329) — Name a new character | — | S014, S039, S001 |
| [`capability.character.cancel-name`](../capabilities/native-coverage.json#L1376) — Cancel a new character name | — | S014, S001 |
| [`capability.character.name-rejected`](../capabilities/native-coverage.json#L1420) — Recover from rejected character choice | — | S014, S039, S001 |
| [`capability.character.swap`](../capabilities/native-coverage.json#L1467) — Swap characters | `capability.character.read-overview` | S015, S014, S001 |
| [`capability.character.insert-before`](../capabilities/native-coverage.json#L1516) — Insert character before another | `capability.character.read-overview` | S015, S014, S001 |
| [`capability.character.append-after`](../capabilities/native-coverage.json#L1565) — Append character after another | `capability.character.read-overview` | S015, S014, S001 |
| [`capability.character.cancel-reorder-first`](../capabilities/native-coverage.json#L1614) — Cancel first reorder selection | — | S015, S001 |
| [`capability.character.cancel-reorder-second`](../capabilities/native-coverage.json#L1658) — Cancel second reorder selection | — | S015, S001 |
| [`capability.character.quit-overview`](../capabilities/native-coverage.json#L1702) — Quit from character overview | — | S014, S001 |
| [`capability.birth.sex`](../capabilities/native-coverage.json#L1746) — Choose sex | `capability.character.create-ordinary` | S005, S001 |
| [`capability.birth.race`](../capabilities/native-coverage.json#L1792) — Choose race | `capability.character.create-ordinary` | S006, S001 |
| [`capability.birth.trait`](../capabilities/native-coverage.json#L1838) — Choose trait | `capability.character.create-ordinary` | S007, S001 |
| [`capability.birth.class`](../capabilities/native-coverage.json#L1884) — Choose class | `capability.character.create-ordinary` | S008, S001 |
| [`capability.birth.body`](../capabilities/native-coverage.json#L1930) — Choose body modification | `capability.character.create-ordinary` | S011, S001 |
| [`capability.birth.stats`](../capabilities/native-coverage.json#L1976) — Distribute starting stats | `capability.character.create-ordinary` | S009, S001 |
| [`capability.birth.mode`](../capabilities/native-coverage.json#L2022) — Choose character mode | `capability.character.create-ordinary` | S010, S001 |
| [`capability.birth.quit`](../capabilities/native-coverage.json#L2068) — Quit character creation | — | S005, S012, S001 |
| [`capability.birth.restore-dna`](../capabilities/native-coverage.json#L2115) — Reuse saved birth choices | — | S012, S001 |
| [`capability.birth.save-dna`](../capabilities/native-coverage.json#L2159) — Persist completed birth choices | `capability.files.dna-shared` | S012, S001 |
| [`capability.birth.complete`](../capabilities/native-coverage.json#L2205) — Enter play with completed creation | `capability.birth.body`<br>`capability.birth.class`<br>`capability.birth.mode`<br>`capability.birth.race`<br>`capability.birth.restore-dna`<br>`capability.birth.save-dna`<br>`capability.birth.sex`<br>`capability.birth.stats`<br>`capability.birth.trait` | S018, S012, S017, S001 |
| [`capability.birth.backtrack-race`](../capabilities/native-coverage.json#L5452) — Return from race selection | — | S006, S012, S001 |
| [`capability.birth.backtrack-trait`](../capabilities/native-coverage.json#L5499) — Return from trait selection | — | S007, S012, S001 |
| [`capability.birth.backtrack-class`](../capabilities/native-coverage.json#L5546) — Return from class selection | — | S008, S012, S001 |
| [`capability.birth.backtrack-body`](../capabilities/native-coverage.json#L5593) — Return from body selection | — | S011, S012, S001 |
| [`capability.birth.backtrack-stats`](../capabilities/native-coverage.json#L5640) — Return from stats selection | — | S009, S012, S001 |
| [`capability.birth.backtrack-mode`](../capabilities/native-coverage.json#L5687) — Return from mode selection | — | S010, S012, S001 |

<a id="g04"></a>

### G04. Загрузить данные, войти в мир и поддерживать соединение — 23

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.session.read-motd`](../capabilities/native-coverage.json#L2265) — Read and acknowledge MOTD | — | S021, S013, S001 |
| [`capability.session.enter-game`](../capabilities/native-coverage.json#L2312) — Enter a live session | `capability.birth.complete`<br>`capability.character.select-existing`<br>`capability.network.confirm`<br>`capability.network.end-marker`<br>`capability.network.flush`<br>`capability.network.keepalive`<br>`capability.network.malformed-packet`<br>`capability.network.partial-packet`<br>`capability.network.pause`<br>`capability.network.ping-echo`<br>`capability.network.server-flags`<br>`capability.network.unknown-packet`<br>`capability.session.load-profile-input`<br>`capability.session.read-motd`<br>`capability.session.transfer-startup-files`<br>`capability.status.read-live-hp`<br>`capability.world.read-map` | S018, S037, S013, S001 |
| [`capability.session.load-profile-input`](../capabilities/native-coverage.json#L2380) — Apply the session profile and input layers | `capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.macro-wait`<br>`capability.input.macro-xwait`<br>`capability.input.physical-keys`<br>`capability.input.prompt-navigation` | S013, S032, S042, S043, S001 |
| [`capability.session.transfer-startup-files`](../capabilities/native-coverage.json#L2440) — Complete startup server file transfer | — | S035, S017, S013, S001 |
| [`capability.session.startup-file-failure`](../capabilities/native-coverage.json#L5402) — Recover from startup transfer failure | — | S035, S017, S013, S001 |
| [`capability.lua.execute-local`](../capabilities/native-coverage.json#L42956) — lua.execute-local | — | S290, S089 |
| [`capability.lua.reload`](../capabilities/native-coverage.json#L43000) — lua.reload | — | S290, S089, S291 |
| [`capability.transfer.check`](../capabilities/native-coverage.json#L43047) — transfer.check | — | S291, S089, S316 |
| [`capability.transfer.init`](../capabilities/native-coverage.json#L43102) — transfer.init | — | S291, S089, S316 |
| [`capability.transfer.data`](../capabilities/native-coverage.json#L43157) — transfer.data | — | S291, S089, S316 |
| [`capability.transfer.end`](../capabilities/native-coverage.json#L43212) — transfer.end | — | S291, S089, S316 |
| [`capability.transfer.upload`](../capabilities/native-coverage.json#L43267) — transfer.upload | — | S291, S089, S317 |
| [`capability.network.keepalive`](../capabilities/native-coverage.json#L50859) — network / keepalive | — | S365, S394 |
| [`capability.network.ping-echo`](../capabilities/native-coverage.json#L50901) — network / ping echo | — | S366 |
| [`capability.network.unknown-packet`](../capabilities/native-coverage.json#L50942) — network / unknown packet | — | S367 |
| [`capability.network.partial-packet`](../capabilities/native-coverage.json#L50983) — network / partial packet | — | S368 |
| [`capability.network.malformed-packet`](../capabilities/native-coverage.json#L51024) — network / malformed packet | — | S369 |
| [`capability.network.keypress-stub`](../capabilities/native-coverage.json#L51065) — network / keypress stub | — | S370 |
| [`capability.network.server-flags`](../capabilities/native-coverage.json#L51106) — network / server flags | — | S371 |
| [`capability.network.pause`](../capabilities/native-coverage.json#L51724) — network / pause | — | S386 |
| [`capability.network.flush`](../capabilities/native-coverage.json#L51765) — network / flush | — | S387 |
| [`capability.network.confirm`](../capabilities/native-coverage.json#L51806) — network / confirm | — | S388 |
| [`capability.network.end-marker`](../capabilities/native-coverage.json#L51847) — network / end marker | — | S389 |

<a id="g05"></a>

### G05. Видеть живую карту, персонажа и предупреждения — 64

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.world.read-location`](../capabilities/native-coverage.json#L3927) — Read current world location | `capability.session.enter-game` | S034, S036, S001 |
| [`capability.world.read-movement-state`](../capabilities/native-coverage.json#L3976) — Read movement and resting state | `capability.session.enter-game` | S038, S001 |
| [`capability.session.read-identity`](../capabilities/native-coverage.json#L5309) — Read live character identity | — | S037, S001 |
| [`capability.world.read-map`](../capabilities/native-coverage.json#L5353) — Read the live game map | `capability.session.read-identity` | S036, S018, S001 |
| [`capability.rendering.glyph-identity`](../capabilities/native-coverage.json#L43322) — rendering.glyph-identity | — | S292, S089 |
| [`capability.rendering.terrain-stack`](../capabilities/native-coverage.json#L43367) — rendering.terrain-stack | — | S292, S089 |
| [`capability.rendering.tiles`](../capabilities/native-coverage.json#L43412) — rendering.tiles | — | S292, S089 |
| [`capability.rendering.subsets`](../capabilities/native-coverage.json#L43457) — rendering.subsets | — | S292, S089 |
| [`capability.rendering.palette`](../capabilities/native-coverage.json#L43502) — rendering.palette | — | S292, S089 |
| [`capability.rendering.animation`](../capabilities/native-coverage.json#L43547) — rendering.animation | — | S292, S089 |
| [`capability.rendering.lighting`](../capabilities/native-coverage.json#L43592) — rendering.lighting | — | S292, S089 |
| [`capability.rendering.cursor`](../capabilities/native-coverage.json#L43637) — rendering.cursor | — | S292, S089 |
| [`capability.rendering.resize`](../capabilities/native-coverage.json#L43682) — rendering.resize | — | S292, S089 |
| [`capability.rendering.cache-generation`](../capabilities/native-coverage.json#L43727) — rendering.cache-generation | — | S292, S089 |
| [`capability.rendering.effects-restore`](../capabilities/native-coverage.json#L43772) — rendering.effects-restore | — | S292, S089 |
| [`capability.rendering.weather`](../capabilities/native-coverage.json#L43862) — rendering.weather | — | S293, S089, S320 |
| [`capability.fonts.text-select`](../capabilities/native-coverage.json#L43918) — fonts.text-select | — | S294, S089 |
| [`capability.fonts.map-select`](../capabilities/native-coverage.json#L43962) — fonts.map-select | `capability.files.resource-overlay` | S294, S089, S324 |
| [`capability.fonts.pcf`](../capabilities/native-coverage.json#L44019) — fonts.pcf | — | S294, S089 |
| [`capability.fonts.ttf`](../capabilities/native-coverage.json#L44063) — fonts.ttf | — | S294, S089 |
| [`capability.fonts.fallback`](../capabilities/native-coverage.json#L44107) — fonts.fallback | — | S294, S089 |
| [`capability.fonts.live-failure`](../capabilities/native-coverage.json#L44151) — fonts.live-failure | — | S294, S089 |
| [`capability.fonts.tile-fallback`](../capabilities/native-coverage.json#L44195) — fonts.tile-fallback | — | S294, S089 |
| [`capability.fonts.graphics-filter`](../capabilities/native-coverage.json#L44239) — fonts.graphics-filter | — | S294, S089 |
| [`capability.fonts.pcf-filter`](../capabilities/native-coverage.json#L44283) — fonts.pcf-filter | — | S294, S089 |
| [`capability.fonts.outline`](../capabilities/native-coverage.json#L44327) — fonts.outline | — | S294, S089 |
| [`capability.fonts.asset-validation`](../capabilities/native-coverage.json#L44371) — fonts.asset-validation | — | S294, S089 |
| [`capability.rendering.wipe`](../capabilities/native-coverage.json#L47766) — rendering.wipe | — | S300, S089 |
| [`capability.rendering.solid-wall`](../capabilities/native-coverage.json#L47811) — rendering.solid-wall | — | S300, S089 |
| [`capability.status.read-stats`](../capabilities/native-coverage.json#L49786) — status / read stats | — | S339 |
| [`capability.status.read-armour`](../capabilities/native-coverage.json#L49828) — status / read armour | — | S340 |
| [`capability.status.read-demographics`](../capabilities/native-coverage.json#L49869) — status / read demographics | — | S341 |
| [`capability.status.read-combat-bonuses`](../capabilities/native-coverage.json#L49910) — status / read combat bonuses | — | S342 |
| [`capability.status.read-experience`](../capabilities/native-coverage.json#L49951) — status / read experience | — | S343 |
| [`capability.status.read-currency`](../capabilities/native-coverage.json#L49993) — status / read currency | — | S344 |
| [`capability.status.read-mana`](../capabilities/native-coverage.json#L50034) — status / read mana | — | S345 |
| [`capability.status.read-confusion`](../capabilities/native-coverage.json#L50075) — status / read confusion | — | S346 |
| [`capability.status.read-poison`](../capabilities/native-coverage.json#L50116) — status / read poison | — | S347 |
| [`capability.status.read-study`](../capabilities/native-coverage.json#L50157) — status / read study | — | S348 |
| [`capability.status.read-blows`](../capabilities/native-coverage.json#L50198) — status / read blows | — | S349 |
| [`capability.status.read-food`](../capabilities/native-coverage.json#L50240) — status / read food | — | S350 |
| [`capability.status.read-fear`](../capabilities/native-coverage.json#L50281) — status / read fear | — | S351 |
| [`capability.status.read-speed`](../capabilities/native-coverage.json#L50322) — status / read speed | — | S352 |
| [`capability.status.read-cut`](../capabilities/native-coverage.json#L50363) — status / read cut | — | S353 |
| [`capability.status.read-blind-hallucination`](../capabilities/native-coverage.json#L50404) — status / read blind hallucination | — | S354 |
| [`capability.status.read-stun`](../capabilities/native-coverage.json#L50445) — status / read stun | — | S355 |
| [`capability.status.read-monster-health`](../capabilities/native-coverage.json#L50486) — status / read monster health | — | S356 |
| [`capability.status.read-sanity`](../capabilities/native-coverage.json#L50527) — status / read sanity | — | S357 |
| [`capability.status.read-afk`](../capabilities/native-coverage.json#L50570) — status / read afk | — | S358 |
| [`capability.status.read-encumbrance`](../capabilities/native-coverage.json#L50611) — status / read encumbrance | — | S359 |
| [`capability.status.read-stamina`](../capabilities/native-coverage.json#L50654) — status / read stamina | — | S360 |
| [`capability.status.read-extra`](../capabilities/native-coverage.json#L50695) — status / read extra | — | S361 |
| [`capability.status.read-martyr`](../capabilities/native-coverage.json#L50736) — status / read martyr | — | S362 |
| [`capability.status.read-idle`](../capabilities/native-coverage.json#L50777) — status / read idle | — | S363 |
| [`capability.status.read-indicators`](../capabilities/native-coverage.json#L50818) — status / read indicators | — | S364 |
| [`capability.alerts.hp-warning`](../capabilities/native-coverage.json#L51516) — alerts / hp warning | — | S381 |
| [`capability.alerts.mana-warning`](../capabilities/native-coverage.json#L51558) — alerts / mana warning | — | S382 |
| [`capability.alerts.sanity-warning`](../capabilities/native-coverage.json#L51599) — alerts / sanity warning | — | S383 |
| [`capability.alerts.page`](../capabilities/native-coverage.json#L51642) — alerts / page | — | S384 |
| [`capability.alerts.warning-beep`](../capabilities/native-coverage.json#L51683) — alerts / warning beep | — | S385 |
| [`capability.information.live-players`](../capabilities/native-coverage.json#L51888) — information / live players | `capability.network.partial-packet` | S390 |
| [`capability.information.unique-records`](../capabilities/native-coverage.json#L51934) — information / unique records | — | S391 |
| [`capability.session.redraw`](../capabilities/native-coverage.json#L51975) — session / redraw | — | S392 |
| [`capability.status.read-live-hp`](../capabilities/native-coverage.json#L52057) — status / read live hp | `capability.status.read-hp` | S395 |

<a id="g06"></a>

### G06. Передвигаться, исследовать карту и выбрать направление — 32

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.world.walk`](../capabilities/native-coverage.json#L2933) — Walk one step | `capability.direction.cancel`<br>`capability.direction.choose-direction`<br>`capability.direction.choose-target`<br>`capability.direction.use-acquired`<br>`capability.session.enter-game`<br>`capability.store.enter`<br>`capability.store.kicked`<br>`capability.store.leave` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.run`](../capabilities/native-coverage.json#L2998) — Run in a direction | `capability.direction.cancel`<br>`capability.direction.choose-direction`<br>`capability.direction.choose-target`<br>`capability.direction.use-acquired`<br>`capability.session.enter-game`<br>`capability.store.enter`<br>`capability.store.kicked`<br>`capability.store.leave` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.stay`](../capabilities/native-coverage.json#L3063) — Stay and pick up | `capability.items.pickup-accept`<br>`capability.items.pickup-decline`<br>`capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.stay-one`](../capabilities/native-coverage.json#L3123) — Pick up one item | `capability.items.pickup-accept`<br>`capability.items.pickup-decline`<br>`capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.rest`](../capabilities/native-coverage.json#L3183) — Rest | `capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.search`](../capabilities/native-coverage.json#L3241) — Search once | `capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.toggle-search`](../capabilities/native-coverage.json#L3299) — Toggle searching | `capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.ascend`](../capabilities/native-coverage.json#L3357) — Use stairs upward | `capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.descend`](../capabilities/native-coverage.json#L3415) — Use stairs downward | `capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.tunnel`](../capabilities/native-coverage.json#L3473) — Tunnel in a direction | `capability.direction.cancel`<br>`capability.direction.choose-direction`<br>`capability.direction.choose-target`<br>`capability.direction.use-acquired`<br>`capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.open`](../capabilities/native-coverage.json#L3535) — Open adjacent feature | `capability.direction.cancel`<br>`capability.direction.choose-direction`<br>`capability.direction.choose-target`<br>`capability.direction.use-acquired`<br>`capability.session.enter-game`<br>`capability.store.enter`<br>`capability.store.kicked`<br>`capability.store.leave` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.close`](../capabilities/native-coverage.json#L3600) — Close adjacent feature | `capability.direction.cancel`<br>`capability.direction.choose-direction`<br>`capability.direction.choose-target`<br>`capability.direction.use-acquired`<br>`capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.bash`](../capabilities/native-coverage.json#L3662) — Bash adjacent feature | `capability.direction.cancel`<br>`capability.direction.choose-direction`<br>`capability.direction.choose-target`<br>`capability.direction.use-acquired`<br>`capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.disarm`](../capabilities/native-coverage.json#L3724) — Disarm adjacent feature | `capability.direction.cancel`<br>`capability.direction.choose-direction`<br>`capability.direction.choose-target`<br>`capability.direction.use-acquired`<br>`capability.session.enter-game` | S029, S030, S032, S041, S042, S001 |
| [`capability.world.cancel-directional-action`](../capabilities/native-coverage.json#L3786) — Cancel a navigation direction | — | S025, S029, S001 |
| [`capability.world.interrupt-repeat`](../capabilities/native-coverage.json#L3833) — Interrupt repeated actions | — | S030, S041, S001 |
| [`capability.world.clear-command-buffer`](../capabilities/native-coverage.json#L3880) — Clear queued server commands | — | S030, S041, S001 |
| [`capability.map.view-overview`](../capabilities/native-coverage.json#L4022) — Open floor or world map | `capability.session.enter-game` | S027, S036, S032, S001 |
| [`capability.map.pan-overview`](../capabilities/native-coverage.json#L4075) — Pan the overview | `capability.session.enter-game` | S027, S036, S032, S001 |
| [`capability.map.select-sector`](../capabilities/native-coverage.json#L4128) — Inspect a world sector | `capability.session.enter-game` | S027, S036, S032, S001 |
| [`capability.map.cancel-sector`](../capabilities/native-coverage.json#L4181) — Leave sector selection | `capability.session.enter-game` | S027, S036, S032, S001 |
| [`capability.map.close-overview`](../capabilities/native-coverage.json#L4234) — Close the overview | `capability.session.enter-game` | S027, S036, S032, S001 |
| [`capability.map.locate`](../capabilities/native-coverage.json#L4287) — Inspect neighbouring map panels | `capability.session.enter-game` | S028, S036, S032, S001 |
| [`capability.map.close-locate`](../capabilities/native-coverage.json#L4340) — Restore player-centred viewport | `capability.session.enter-game` | S028, S036, S032, S001 |
| [`capability.direction.choose-direction`](../capabilities/native-coverage.json#L4393) — Choose a direction for an action | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S025, S026, S032, S001 |
| [`capability.direction.use-acquired`](../capabilities/native-coverage.json#L4449) — Use the acquired target | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S025, S026, S032, S001 |
| [`capability.direction.choose-target`](../capabilities/native-coverage.json#L4505) — Choose a target from a direction prompt | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.target.cancel`<br>`capability.target.read-description`<br>`capability.target.select-hostile`<br>`capability.target.select-position` | S025, S026, S032, S001 |
| [`capability.direction.cancel`](../capabilities/native-coverage.json#L4565) — Cancel a direction choice | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S025, S026, S032, S001 |
| [`capability.target.select-hostile`](../capabilities/native-coverage.json#L4621) — Select a hostile target | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S026, S032, S033, S001 |
| [`capability.target.select-position`](../capabilities/native-coverage.json#L4677) — Select a target position | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S026, S032, S033, S001 |
| [`capability.target.cancel`](../capabilities/native-coverage.json#L4733) — Cancel hostile target selection | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S026, S032, S033, S001 |
| [`capability.target.read-description`](../capabilities/native-coverage.json#L4846) — Read target coordinates and description | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S033, S026, S001 |

<a id="g07"></a>

### G07. Подобрать предмет, прочитать вещи и войти/выйти из магазина — 15

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.items.read-inventory`](../capabilities/native-coverage.json#L5734) — items.read-inventory | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.autoinscribe-on-update` | S045, S046, S044, S070, S071, S073, S078 |
| [`capability.items.read-equipment`](../capabilities/native-coverage.json#L5826) — items.read-equipment | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S047, S048, S044, S072, S078 |
| [`capability.items.read-bag`](../capabilities/native-coverage.json#L5910) — items.read-bag | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.autoinscribe-on-update`<br>`capability.items.read-inventory` | S049, S050, S044, S078 |
| [`capability.items.read-floor`](../capabilities/native-coverage.json#L5991) — items.read-floor | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S051, S052, S044, S075, S078 |
| [`capability.items.pickup-accept`](../capabilities/native-coverage.json#L6073) — items.pickup-accept | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.read-floor`<br>`capability.items.read-inventory` | S053, S054, S044, S077, S078 |
| [`capability.items.pickup-decline`](../capabilities/native-coverage.json#L6155) — items.pickup-decline | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.read-floor`<br>`capability.items.read-inventory` | S053, S054, S044, S077, S078 |
| [`capability.store.enter`](../capabilities/native-coverage.json#L6237) — store.enter | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.read-inventory`<br>`capability.store.read-actions`<br>`capability.store.read-stock` | S055, S056, S044 |
| [`capability.store.read-stock`](../capabilities/native-coverage.json#L6317) — store.read-stock | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S057, S058, S044, S076, S056 |
| [`capability.store.read-actions`](../capabilities/native-coverage.json#L6404) — store.read-actions | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S059, S060, S044 |
| [`capability.store.leave`](../capabilities/native-coverage.json#L6481) — store.leave | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.store.read-actions`<br>`capability.store.read-stock` | S061, S062, S044 |
| [`capability.store.kicked`](../capabilities/native-coverage.json#L6557) — store.kicked | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S063, S064, S044 |
| [`capability.items.autoinscribe-on-update`](../capabilities/native-coverage.json#L16870) — items.autoinscribe-on-update | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit` | S066, S065, S044, S046, S050, S074, S078 |
| [`capability.items.close-inventory`](../capabilities/native-coverage.json#L17442) — items.close-inventory | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.read-inventory` | S067, S044, S078 |
| [`capability.items.close-equipment`](../capabilities/native-coverage.json#L17591) — items.close-equipment | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.read-equipment` | S068, S044, S078 |
| [`capability.items.close-bag`](../capabilities/native-coverage.json#L17740) — items.close-bag | `capability.input.confirm`<br>`capability.input.keymap`<br>`capability.input.macro-match`<br>`capability.input.quantity`<br>`capability.input.text-edit`<br>`capability.items.read-bag` | S069, S044, S078 |

<a id="g08"></a>

### G08. Общаться и читать историю сообщений — 19

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.messages.recall-read`](../capabilities/native-coverage.json#L18167) — messages.recall-read | `capability.messages.read-occurrences` | S082, S079 |
| [`capability.messages.recall-navigate`](../capabilities/native-coverage.json#L18213) — messages.recall-navigate | `capability.messages.read-occurrences` | S082, S079 |
| [`capability.messages.recall-search`](../capabilities/native-coverage.json#L18259) — messages.recall-search | `capability.messages.read-occurrences` | S082, S079 |
| [`capability.messages.recall-close`](../capabilities/native-coverage.json#L18305) — messages.recall-close | `capability.messages.read-occurrences` | S082, S079 |
| [`capability.messages.important-read`](../capabilities/native-coverage.json#L18351) — messages.important-read | `capability.messages.read-occurrences` | S083, S079 |
| [`capability.messages.important-navigate`](../capabilities/native-coverage.json#L18397) — messages.important-navigate | `capability.messages.read-occurrences` | S083, S079 |
| [`capability.messages.important-search`](../capabilities/native-coverage.json#L18443) — messages.important-search | `capability.messages.read-occurrences` | S083, S079 |
| [`capability.messages.important-close`](../capabilities/native-coverage.json#L18489) — messages.important-close | `capability.messages.read-occurrences` | S083, S079 |
| [`capability.chat.send`](../capabilities/native-coverage.json#L18535) — chat.send | `capability.clipboard.paste`<br>`capability.messages.read-occurrences` | S084, S085, S079, S087, S088 |
| [`capability.chat.cancel`](../capabilities/native-coverage.json#L18602) — chat.cancel | `capability.messages.read-occurrences` | S084, S085, S079, S087, S088 |
| [`capability.chat.history`](../capabilities/native-coverage.json#L18668) — chat.history | `capability.clipboard.paste`<br>`capability.files.history-load`<br>`capability.files.history-save`<br>`capability.messages.read-occurrences` | S084, S085, S079, S087, S088 |
| [`capability.chat.substitute-items`](../capabilities/native-coverage.json#L18737) — chat.substitute-items | `capability.messages.read-occurrences` | S084, S085, S079, S087, S088 |
| [`capability.chat.local-self`](../capabilities/native-coverage.json#L18803) — chat.local-self | `capability.messages.read-occurrences` | S084, S085, S079, S087, S088 |
| [`capability.chat.forward-slash`](../capabilities/native-coverage.json#L18869) — chat.forward-slash | `capability.messages.read-occurrences` | S084, S085, S079, S087, S088 |
| [`capability.messages.read-live`](../capabilities/native-coverage.json#L30088) — messages.read-live | `capability.messages.read-occurrences` | S086, S087, S079 |
| [`capability.clipboard.copy`](../capabilities/native-coverage.json#L42485) — clipboard.copy | — | S284, S089 |
| [`capability.clipboard.failure`](../capabilities/native-coverage.json#L42529) — clipboard.failure | — | S284, S089 |
| [`capability.clipboard.paste`](../capabilities/native-coverage.json#L42573) — clipboard.paste | — | S285, S089 |
| [`capability.clipboard.extract-url`](../capabilities/native-coverage.json#L49501) — clipboard.extract-url | `capability.clipboard.copy` | S326, S089 |

<a id="g09"></a>

### G09. Пережить завершение сессии и просмотреть итог — 17

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.session.disconnect`](../capabilities/native-coverage.json#L2490) — Handle server or transport disconnect | — | S019, S013, S001 |
| [`capability.session.reconnect`](../capabilities/native-coverage.json#L2537) — Reconnect through a fresh session | `capability.account.restore-secret`<br>`capability.session.disconnect` | S013, S019, S001 |
| [`capability.session.portal-relogin`](../capabilities/native-coverage.json#L2587) — Follow server portal relogin | `capability.account.restore-secret`<br>`capability.session.disconnect` | S020, S001 |
| [`capability.session.quit`](../capabilities/native-coverage.json#L2634) — Quit and release the live session | — | S013, S030, S022, S043, S001 |
| [`capability.session.death`](../capabilities/native-coverage.json#L2687) — Observe death or retirement transition | — | S019, S022, S001 |
| [`capability.session.close-tomb`](../capabilities/native-coverage.json#L2734) — Leave the tomb screen | — | S022, S001 |
| [`capability.session.review-final-state`](../capabilities/native-coverage.json#L2778) — Review final character state | `capability.information.close-sheet`<br>`capability.information.navigate-sheet`<br>`capability.information.read-equipment-flags`<br>`capability.information.read-history`<br>`capability.information.read-sheet`<br>`capability.items.close-bag`<br>`capability.items.close-equipment`<br>`capability.items.close-inventory`<br>`capability.items.read-bag`<br>`capability.items.read-equipment`<br>`capability.items.read-inventory`<br>`capability.messages.important-close`<br>`capability.messages.important-navigate`<br>`capability.messages.important-read`<br>`capability.messages.important-search`<br>`capability.messages.recall-close`<br>`capability.messages.recall-navigate`<br>`capability.messages.recall-read`<br>`capability.messages.recall-search`<br>`capability.session.death` | S022, S001 |
| [`capability.session.suicide`](../capabilities/native-coverage.json#L2843) — Confirm suicide | `capability.session.enter-game` | S031, S001 |
| [`capability.session.cancel-suicide`](../capabilities/native-coverage.json#L2889) — Decline suicide | — | S031, S001 |
| [`capability.session.acknowledge-final-scene`](../capabilities/native-coverage.json#L5177) — Acknowledge the final scene | — | S022, S001 |
| [`capability.session.open-final-review`](../capabilities/native-coverage.json#L5221) — Continue from tomb to final review | — | S022, S001 |
| [`capability.session.close-final-review`](../capabilities/native-coverage.json#L5265) — Close final review | — | S022, S001 |
| [`capability.information.read-sheet`](../capabilities/native-coverage.json#L17889) — information.read-sheet | `capability.exports.character`<br>`capability.guide.context-help` | S080, S079 |
| [`capability.information.read-history`](../capabilities/native-coverage.json#L17944) — information.read-history | — | S080, S081, S079 |
| [`capability.information.read-equipment-flags`](../capabilities/native-coverage.json#L18009) — information.read-equipment-flags | — | S080, S079 |
| [`capability.information.navigate-sheet`](../capabilities/native-coverage.json#L18061) — information.navigate-sheet | `capability.guide.context-help` | S080, S079 |
| [`capability.information.close-sheet`](../capabilities/native-coverage.json#L18115) — information.close-sheet | — | S080, S079 |

<a id="g10"></a>

### G10. Вводить команды, загружать и редактировать макросы — 49

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.preferences.load`](../capabilities/native-coverage.json#L40064) — preferences.load | `capability.settings.load` | S276, S089 |
| [`capability.preferences.bootstrap`](../capabilities/native-coverage.json#L40110) — preferences.bootstrap | `capability.settings.load` | S276, S089 |
| [`capability.preferences.include-failure`](../capabilities/native-coverage.json#L40156) — preferences.include-failure | `capability.settings.load` | S276, S089 |
| [`capability.preferences.macro-precedence`](../capabilities/native-coverage.json#L40202) — preferences.macro-precedence | `capability.files.resource-overlay`<br>`capability.settings.load` | S276, S089, S323 |
| [`capability.macros.inspect`](../capabilities/native-coverage.json#L40260) — macros.inspect | — | S277, S089 |
| [`capability.macros.create`](../capabilities/native-coverage.json#L40305) — macros.create | — | S277, S089 |
| [`capability.macros.replace`](../capabilities/native-coverage.json#L40350) — macros.replace | — | S277, S089 |
| [`capability.macros.delete`](../capabilities/native-coverage.json#L40395) — macros.delete | — | S277, S089 |
| [`capability.macros.load`](../capabilities/native-coverage.json#L40440) — macros.load | — | S277, S089 |
| [`capability.macros.save`](../capabilities/native-coverage.json#L40485) — macros.save | — | S277, S089 |
| [`capability.macros.wizard`](../capabilities/native-coverage.json#L40530) — macros.wizard | — | S277, S089 |
| [`capability.macros.record`](../capabilities/native-coverage.json#L40575) — macros.record | — | S277, S089 |
| [`capability.macros.stage-create`](../capabilities/native-coverage.json#L40620) — macros.stage-create | — | S277, S089 |
| [`capability.macros.stage-switch`](../capabilities/native-coverage.json#L40665) — macros.stage-switch | — | S277, S089 |
| [`capability.macros.stage-reorder`](../capabilities/native-coverage.json#L40710) — macros.stage-reorder | — | S277, S089 |
| [`capability.macros.stage-delete`](../capabilities/native-coverage.json#L40755) — macros.stage-delete | — | S277, S089 |
| [`capability.macros.close`](../capabilities/native-coverage.json#L40800) — macros.close | — | S277, S089 |
| [`capability.macros.save-global`](../capabilities/native-coverage.json#L47960) — macros.save-global | — | S277, S089 |
| [`capability.macros.save-form`](../capabilities/native-coverage.json#L48005) — macros.save-form | — | S277, S089 |
| [`capability.macros.save-class`](../capabilities/native-coverage.json#L48050) — macros.save-class | — | S277, S089 |
| [`capability.macros.load-class`](../capabilities/native-coverage.json#L48095) — macros.load-class | — | S277, S089 |
| [`capability.macros.reinitialize`](../capabilities/native-coverage.json#L48140) — macros.reinitialize | — | S277, S089 |
| [`capability.macros.forget-global`](../capabilities/native-coverage.json#L48185) — macros.forget-global | — | S277, S089 |
| [`capability.macros.forget-character`](../capabilities/native-coverage.json#L48230) — macros.forget-character | — | S277, S089 |
| [`capability.macros.forget-both`](../capabilities/native-coverage.json#L48275) — macros.forget-both | — | S277, S089 |
| [`capability.macros.forget-user`](../capabilities/native-coverage.json#L48320) — macros.forget-user | — | S277, S089 |
| [`capability.macros.forget-all`](../capabilities/native-coverage.json#L48365) — macros.forget-all | — | S277, S089 |
| [`capability.macros.test-key`](../capabilities/native-coverage.json#L48410) — macros.test-key | — | S277, S089 |
| [`capability.macros.list`](../capabilities/native-coverage.json#L48455) — macros.list | — | S277, S089 |
| [`capability.macros.paste-action`](../capabilities/native-coverage.json#L48500) — macros.paste-action | — | S277, S089 |
| [`capability.macros.swap`](../capabilities/native-coverage.json#L48545) — macros.swap | — | S277, S089 |
| [`capability.macros.add-load-comment`](../capabilities/native-coverage.json#L48590) — macros.add-load-comment | — | S277, S089 |
| [`capability.macros.add-load-action`](../capabilities/native-coverage.json#L48635) — macros.add-load-action | — | S277, S089 |
| [`capability.macros.edit-action`](../capabilities/native-coverage.json#L48680) — macros.edit-action | — | S277, S089 |
| [`capability.macros.create-hybrid`](../capabilities/native-coverage.json#L48725) — macros.create-hybrid | — | S277, S089 |
| [`capability.macros.create-command`](../capabilities/native-coverage.json#L48770) — macros.create-command | — | S277, S089 |
| [`capability.macros.create-normal`](../capabilities/native-coverage.json#L48815) — macros.create-normal | — | S277, S089 |
| [`capability.macros.create-empty`](../capabilities/native-coverage.json#L48860) — macros.create-empty | — | S277, S089 |
| [`capability.macros.quick`](../capabilities/native-coverage.json#L48905) — macros.quick | — | S277, S089 |
| [`capability.input.physical-keys`](../capabilities/native-coverage.json#L51147) — input / physical keys | — | S372 |
| [`capability.input.keymap`](../capabilities/native-coverage.json#L51188) — input / keymap | — | S373 |
| [`capability.input.macro-match`](../capabilities/native-coverage.json#L51229) — input / macro match | — | S374 |
| [`capability.input.macro-wait`](../capabilities/native-coverage.json#L51270) — input / macro wait | — | S375 |
| [`capability.input.macro-xwait`](../capabilities/native-coverage.json#L51311) — input / macro xwait | — | S376 |
| [`capability.input.prompt-navigation`](../capabilities/native-coverage.json#L51352) — input / prompt navigation | — | S377 |
| [`capability.input.text-edit`](../capabilities/native-coverage.json#L51393) — input / text edit | — | S378 |
| [`capability.input.confirm`](../capabilities/native-coverage.json#L51434) — input / confirm | — | S379 |
| [`capability.input.quantity`](../capabilities/native-coverage.json#L51475) — input / quantity | — | S380 |
| [`capability.input.raw-key`](../capabilities/native-coverage.json#L52016) — input / raw key | — | S393 |

<a id="g11"></a>

### G11. Получить справку и вернуться к исходному экрану — 16

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.guide.read`](../capabilities/native-coverage.json#L41970) — guide.read | `capability.files.bookmarks-load`<br>`capability.files.resource-overlay` | S282, S089 |
| [`capability.guide.navigate`](../capabilities/native-coverage.json#L42018) — guide.navigate | `capability.files.bookmarks-load` | S282, S089 |
| [`capability.guide.search`](../capabilities/native-coverage.json#L42065) — guide.search | `capability.files.bookmarks-load` | S282, S089 |
| [`capability.guide.bookmark-set`](../capabilities/native-coverage.json#L42112) — guide.bookmark-set | `capability.files.bookmarks-load` | S282, S089 |
| [`capability.guide.bookmark-open`](../capabilities/native-coverage.json#L42159) — guide.bookmark-open | `capability.files.bookmarks-load` | S282, S089 |
| [`capability.guide.bookmark-delete`](../capabilities/native-coverage.json#L42206) — guide.bookmark-delete | `capability.files.bookmarks-load` | S282, S089 |
| [`capability.guide.context-help`](../capabilities/native-coverage.json#L42253) — guide.context-help | `capability.files.bookmarks-load` | S282, S089 |
| [`capability.guide.close`](../capabilities/native-coverage.json#L42300) — guide.close | `capability.files.bookmarks-load` | S282, S089 |
| [`capability.guide.checksum`](../capabilities/native-coverage.json#L42347) — guide.checksum | — | S283, S089 |
| [`capability.guide.reload`](../capabilities/native-coverage.json#L42392) — guide.reload | — | S283, S089, S312 |
| [`capability.guide.server-open`](../capabilities/native-coverage.json#L47900) — guide.server-open | `capability.guide.close`<br>`capability.guide.read`<br>`capability.guide.search` | S302, S089, S321 |
| [`capability.guide.copy-lines`](../capabilities/native-coverage.json#L48950) — guide.copy-lines | `capability.clipboard.copy`<br>`capability.guide.read` | S282, S089 |
| [`capability.guide.paste-line`](../capabilities/native-coverage.json#L48998) — guide.paste-line | `capability.guide.read` | S282, S089 |
| [`capability.guide.mark-results`](../capabilities/native-coverage.json#L49045) — guide.mark-results | `capability.guide.read` | S282, S089 |
| [`capability.guide.restore-search`](../capabilities/native-coverage.json#L49092) — guide.restore-search | `capability.guide.read` | S282, S089 |
| [`capability.guide.help`](../capabilities/native-coverage.json#L49139) — guide.help | `capability.guide.read` | S282, S089 |

<a id="g12"></a>

### G12. Сохранять пользовательские файлы и открывать результаты — 22

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.files.history-load`](../capabilities/native-coverage.json#L41197) — files.history-load | — | S279, S089 |
| [`capability.files.history-save`](../capabilities/native-coverage.json#L41241) — files.history-save | — | S279, S089 |
| [`capability.files.bookmarks-load`](../capabilities/native-coverage.json#L41285) — files.bookmarks-load | — | S279, S089, S309 |
| [`capability.files.bookmarks-save`](../capabilities/native-coverage.json#L41332) — files.bookmarks-save | — | S279, S089, S309 |
| [`capability.files.resource-overlay`](../capabilities/native-coverage.json#L41379) — files.resource-overlay | — | S280, S089 |
| [`capability.files.ins-shared`](../capabilities/native-coverage.json#L41423) — files.ins-shared | — | S280, S089, S306, S307 |
| [`capability.files.dna-shared`](../capabilities/native-coverage.json#L41473) — files.dna-shared | — | S280, S089, S308 |
| [`capability.files.notes-append`](../capabilities/native-coverage.json#L41520) — files.notes-append | — | S280, S089, S303 |
| [`capability.files.clone-console`](../capabilities/native-coverage.json#L41567) — files.clone-console | — | S280, S089, S304 |
| [`capability.files.clone-file`](../capabilities/native-coverage.json#L41614) — files.clone-file | — | S280, S089, S305 |
| [`capability.files.export-collision`](../capabilities/native-coverage.json#L41661) — files.export-collision | — | S280, S089 |
| [`capability.files.transient-owner`](../capabilities/native-coverage.json#L41705) — files.transient-owner | — | S280, S089 |
| [`capability.exports.character`](../capabilities/native-coverage.json#L42617) — exports.character | `capability.files.export-collision` | S286, S089 |
| [`capability.exports.messages`](../capabilities/native-coverage.json#L42663) — exports.messages | `capability.files.export-collision` | S287, S089 |
| [`capability.screenshots.capture`](../capabilities/native-coverage.json#L42709) — screenshots.capture | `capability.files.export-collision`<br>`capability.platform.optional-image` | S288, S089, S313, S314 |
| [`capability.screenshots.open-last`](../capabilities/native-coverage.json#L42762) — screenshots.open-last | `capability.files.export-collision` | S288, S089 |
| [`capability.screenshots.server-trigger`](../capabilities/native-coverage.json#L42808) — screenshots.server-trigger | `capability.files.export-collision` | S288, S089, S315, S322 |
| [`capability.os.open-url`](../capabilities/native-coverage.json#L42868) — os.open-url | — | S289, S089 |
| [`capability.os.open-file`](../capabilities/native-coverage.json#L42912) — os.open-file | — | S289, S089 |
| [`capability.platform.optional-image`](../capabilities/native-coverage.json#L45243) — platform.optional-image | — | S297, S089 |
| [`capability.platform.optional-sticky`](../capabilities/native-coverage.json#L45375) — platform.optional-sticky | — | S297, S089 |
| [`capability.files.report-resource-identity`](../capabilities/native-coverage.json#L49711) — files.report-resource-identity | — | S333, S089, S334, S335, S324, S336, S337, S338 |

<a id="g13"></a>

### G13. Слышать игровые события и менять воспроизведение — 17

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.audio.play-event`](../capabilities/native-coverage.json#L44415) — audio.play-event | `capability.files.resource-overlay` | S295, S089, S318 |
| [`capability.audio.music`](../capabilities/native-coverage.json#L44473) — audio.music | — | S295, S089, S319 |
| [`capability.audio.weather`](../capabilities/native-coverage.json#L44529) — audio.weather | — | S295, S089, S320 |
| [`capability.audio.master`](../capabilities/native-coverage.json#L44585) — audio.master | — | S295, S089 |
| [`capability.audio.device-failure`](../capabilities/native-coverage.json#L44630) — audio.device-failure | — | S295, S089 |
| [`capability.audio.afk-mute`](../capabilities/native-coverage.json#L44675) — audio.afk-mute | — | S295, S089 |
| [`capability.audio.close`](../capabilities/native-coverage.json#L45104) — audio.close | — | S296, S089 |
| [`capability.audio.reset`](../capabilities/native-coverage.json#L49186) — audio.reset | — | S296, S089 |
| [`capability.audio.preset-effects`](../capabilities/native-coverage.json#L49231) — audio.preset-effects | — | S296, S089 |
| [`capability.audio.preset-music`](../capabilities/native-coverage.json#L49276) — audio.preset-music | — | S296, S089 |
| [`capability.audio.preset-effects-weather`](../capabilities/native-coverage.json#L49321) — audio.preset-effects-weather | — | S296, S089 |
| [`capability.audio.preset-effects-music`](../capabilities/native-coverage.json#L49366) — audio.preset-effects-music | — | S296, S089 |
| [`capability.audio.preset-all`](../capabilities/native-coverage.json#L49411) — audio.preset-all | — | S296, S089 |
| [`capability.audio.volume-preset`](../capabilities/native-coverage.json#L49456) — audio.volume-preset | — | S296, S089 |
| [`capability.audio.music-volume`](../capabilities/native-coverage.json#L49547) — audio.music-volume | — | S327, S089, S328 |
| [`capability.audio.ambient`](../capabilities/native-coverage.json#L49601) — audio.ambient | — | S329, S089, S330 |
| [`capability.audio.ambient-volume`](../capabilities/native-coverage.json#L49656) — audio.ambient-volume | — | S331, S089, S332 |

<a id="g14"></a>

### G14. Применить игровые и визуальные OPT-параметры — 184

| ID / результат manifest | Прямые prerequisites ledger | Источники |
|---|---|---|
| [`capability.options.rogue-like-commands`](../capabilities/native-coverage.json#L31048) — options.rogue-like-commands | `capability.settings.load` | S092, S089, S091 |
| [`capability.options.newbie-hints`](../capabilities/native-coverage.json#L31097) — options.newbie-hints | `capability.settings.load` | S093, S089, S091 |
| [`capability.options.censor-swearing`](../capabilities/native-coverage.json#L31146) — options.censor-swearing | `capability.settings.load` | S094, S089, S091 |
| [`capability.options.highlight-chat`](../capabilities/native-coverage.json#L31195) — options.highlight-chat | `capability.settings.load` | S095, S089, S091 |
| [`capability.options.highbeep-chat`](../capabilities/native-coverage.json#L31244) — options.highbeep-chat | `capability.settings.load` | S096, S089, S091 |
| [`capability.options.page-on-privmsg`](../capabilities/native-coverage.json#L31293) — options.page-on-privmsg | `capability.settings.load` | S097, S089, S091 |
| [`capability.options.page-on-afk-privmsg`](../capabilities/native-coverage.json#L31342) — options.page-on-afk-privmsg | `capability.settings.load` | S098, S089, S091 |
| [`capability.options.font-map-solid-walls`](../capabilities/native-coverage.json#L31391) — options.font-map-solid-walls | `capability.settings.load` | S099, S089, S091 |
| [`capability.options.view-animated-light`](../capabilities/native-coverage.json#L31440) — options.view-animated-light | `capability.settings.load` | S100, S089, S091 |
| [`capability.options.wall-lighting`](../capabilities/native-coverage.json#L31489) — options.wall-lighting | `capability.settings.load` | S101, S089, S091 |
| [`capability.options.view-lamp-walls`](../capabilities/native-coverage.json#L31538) — options.view-lamp-walls | `capability.settings.load` | S102, S089, S091 |
| [`capability.options.view-shade-walls`](../capabilities/native-coverage.json#L31587) — options.view-shade-walls | `capability.settings.load` | S103, S089, S091 |
| [`capability.options.floor-lighting`](../capabilities/native-coverage.json#L31636) — options.floor-lighting | `capability.settings.load` | S104, S089, S091 |
| [`capability.options.view-lamp-floor`](../capabilities/native-coverage.json#L31685) — options.view-lamp-floor | `capability.settings.load` | S105, S089, S091 |
| [`capability.options.view-shade-floor`](../capabilities/native-coverage.json#L31734) — options.view-shade-floor | `capability.settings.load` | S106, S089, S091 |
| [`capability.options.view-light-extra`](../capabilities/native-coverage.json#L31783) — options.view-light-extra | `capability.settings.load` | S107, S089, S091 |
| [`capability.options.alert-hitpoint`](../capabilities/native-coverage.json#L31832) — options.alert-hitpoint | `capability.settings.load` | S108, S089, S091 |
| [`capability.options.alert-mana`](../capabilities/native-coverage.json#L31881) — options.alert-mana | `capability.settings.load` | S109, S089, S091 |
| [`capability.options.alert-afk-dam`](../capabilities/native-coverage.json#L31930) — options.alert-afk-dam | `capability.settings.load` | S110, S089, S091 |
| [`capability.options.alert-offpanel-dam`](../capabilities/native-coverage.json#L31979) — options.alert-offpanel-dam | `capability.settings.load` | S111, S089, S091 |
| [`capability.options.exp-bar`](../capabilities/native-coverage.json#L32028) — options.exp-bar | `capability.settings.load` | S112, S089, S091 |
| [`capability.options.uniques-alive`](../capabilities/native-coverage.json#L32077) — options.uniques-alive | `capability.settings.load` | S113, S089, S091 |
| [`capability.options.warn-unique-credit`](../capabilities/native-coverage.json#L32126) — options.warn-unique-credit | `capability.settings.load` | S114, S089, S091 |
| [`capability.options.limit-chat`](../capabilities/native-coverage.json#L32175) — options.limit-chat | `capability.settings.load` | S115, S089, S091 |
| [`capability.options.no-afk-msg`](../capabilities/native-coverage.json#L32224) — options.no-afk-msg | `capability.settings.load` | S116, S089, S091 |
| [`capability.options.overview-startup`](../capabilities/native-coverage.json#L32273) — options.overview-startup | `capability.settings.load` | S117, S089, S091 |
| [`capability.options.allow-paging`](../capabilities/native-coverage.json#L32322) — options.allow-paging | `capability.settings.load` | S118, S089, S091 |
| [`capability.options.ring-bell`](../capabilities/native-coverage.json#L32371) — options.ring-bell | `capability.settings.load` | S119, S089, S091 |
| [`capability.options.linear-stats`](../capabilities/native-coverage.json#L32420) — options.linear-stats | `capability.settings.load` | S120, S089, S091 |
| [`capability.options.exp-need`](../capabilities/native-coverage.json#L32469) — options.exp-need | `capability.settings.load` | S121, S089, S091 |
| [`capability.options.depth-in-feet`](../capabilities/native-coverage.json#L32518) — options.depth-in-feet | `capability.settings.load` | S122, S089, S091 |
| [`capability.options.newb-suicide`](../capabilities/native-coverage.json#L32567) — options.newb-suicide | `capability.settings.load` | S123, S089, S091 |
| [`capability.options.show-weights`](../capabilities/native-coverage.json#L32616) — options.show-weights | `capability.settings.load` | S124, S089, S091 |
| [`capability.options.time-stamp-chat`](../capabilities/native-coverage.json#L32665) — options.time-stamp-chat | `capability.settings.load` | S125, S089, S091 |
| [`capability.options.hide-unusable-skills`](../capabilities/native-coverage.json#L32714) — options.hide-unusable-skills | `capability.settings.load` | S126, S089, S091 |
| [`capability.options.short-item-names`](../capabilities/native-coverage.json#L32763) — options.short-item-names | `capability.settings.load` | S127, S089, S091 |
| [`capability.options.keep-topline`](../capabilities/native-coverage.json#L32812) — options.keep-topline | `capability.settings.load` | S128, S089, S091 |
| [`capability.options.target-history`](../capabilities/native-coverage.json#L32861) — options.target-history | `capability.settings.load` | S129, S089, S091 |
| [`capability.options.taciturn-messages`](../capabilities/native-coverage.json#L32910) — options.taciturn-messages | `capability.settings.load` | S130, S089, S091 |
| [`capability.options.always-show-lists`](../capabilities/native-coverage.json#L32959) — options.always-show-lists | `capability.settings.load` | S131, S089, S091 |
| [`capability.options.no-weather`](../capabilities/native-coverage.json#L33008) — options.no-weather | `capability.settings.load` | S132, S089, S091 |
| [`capability.options.player-list`](../capabilities/native-coverage.json#L33057) — options.player-list | `capability.settings.load` | S133, S089, S091 |
| [`capability.options.player-list2`](../capabilities/native-coverage.json#L33106) — options.player-list2 | `capability.settings.load` | S134, S089, S091 |
| [`capability.options.flash-player`](../capabilities/native-coverage.json#L33155) — options.flash-player | `capability.settings.load` | S135, S089, S091 |
| [`capability.options.highlight-player`](../capabilities/native-coverage.json#L33204) — options.highlight-player | `capability.settings.load` | S136, S089, S091 |
| [`capability.options.basic-players-symb`](../capabilities/native-coverage.json#L33253) — options.basic-players-symb | `capability.settings.load` | S137, S089, S091 |
| [`capability.options.subterm-flicker`](../capabilities/native-coverage.json#L33302) — options.subterm-flicker | `capability.settings.load` | S138, S089, S091 |
| [`capability.options.no-verify-sell`](../capabilities/native-coverage.json#L33351) — options.no-verify-sell | `capability.settings.load` | S139, S089, S091 |
| [`capability.options.no-verify-destroy`](../capabilities/native-coverage.json#L33400) — options.no-verify-destroy | `capability.settings.load` | S140, S089, S091 |
| [`capability.options.auto-afk`](../capabilities/native-coverage.json#L33449) — options.auto-afk | `capability.settings.load` | S141, S089, S091 |
| [`capability.options.idle-starve-kick`](../capabilities/native-coverage.json#L33498) — options.idle-starve-kick | `capability.settings.load` | S142, S089, S091 |
| [`capability.options.safe-float`](../capabilities/native-coverage.json#L33547) — options.safe-float | `capability.settings.load` | S143, S089, S091 |
| [`capability.options.safe-macros`](../capabilities/native-coverage.json#L33596) — options.safe-macros | `capability.settings.load` | S144, S089, S091 |
| [`capability.options.auto-untag`](../capabilities/native-coverage.json#L33645) — options.auto-untag | `capability.settings.load` | S145, S089, S091 |
| [`capability.options.clear-inscr`](../capabilities/native-coverage.json#L33694) — options.clear-inscr | `capability.settings.load` | S146, S089, S091 |
| [`capability.options.auto-inscr-server`](../capabilities/native-coverage.json#L33743) — options.auto-inscr-server | `capability.settings.load` | S147, S089, S091 |
| [`capability.options.stack-force-notes`](../capabilities/native-coverage.json#L33792) — options.stack-force-notes | `capability.settings.load` | S148, S089, S091 |
| [`capability.options.stack-force-costs`](../capabilities/native-coverage.json#L33841) — options.stack-force-costs | `capability.settings.load` | S149, S089, S091 |
| [`capability.options.stack-allow-items`](../capabilities/native-coverage.json#L33890) — options.stack-allow-items | `capability.settings.load` | S150, S089, S091 |
| [`capability.options.stack-allow-devices`](../capabilities/native-coverage.json#L33939) — options.stack-allow-devices | `capability.settings.load` | S151, S089, S091 |
| [`capability.options.whole-ammo-stack`](../capabilities/native-coverage.json#L33988) — options.whole-ammo-stack | `capability.settings.load` | S152, S089, S091 |
| [`capability.options.always-repeat`](../capabilities/native-coverage.json#L34037) — options.always-repeat | `capability.settings.load` | S153, S089, S091 |
| [`capability.options.always-pickup`](../capabilities/native-coverage.json#L34086) — options.always-pickup | `capability.settings.load` | S154, S089, S091 |
| [`capability.options.use-old-target`](../capabilities/native-coverage.json#L34135) — options.use-old-target | `capability.settings.load` | S155, S089, S091 |
| [`capability.options.autooff-retaliator`](../capabilities/native-coverage.json#L34184) — options.autooff-retaliator | `capability.settings.load` | S156, S089, S091 |
| [`capability.options.fail-no-melee`](../capabilities/native-coverage.json#L34233) — options.fail-no-melee | `capability.settings.load` | S157, S089, S091 |
| [`capability.options.basic-players-col`](../capabilities/native-coverage.json#L34282) — options.basic-players-col | `capability.settings.load` | S158, S089, S091 |
| [`capability.options.auto-target`](../capabilities/native-coverage.json#L34331) — options.auto-target | `capability.settings.load` | S159, S089, S091 |
| [`capability.options.thin-down-flush`](../capabilities/native-coverage.json#L34380) — options.thin-down-flush | `capability.settings.load` | S160, S089, S091 |
| [`capability.options.disable-flush`](../capabilities/native-coverage.json#L34429) — options.disable-flush | `capability.settings.load` | S161, S089, S091 |
| [`capability.options.find-ignore-stairs`](../capabilities/native-coverage.json#L34478) — options.find-ignore-stairs | `capability.settings.load` | S162, S089, S091 |
| [`capability.options.find-ignore-doors`](../capabilities/native-coverage.json#L34527) — options.find-ignore-doors | `capability.settings.load` | S163, S089, S091 |
| [`capability.options.find-cut`](../capabilities/native-coverage.json#L34576) — options.find-cut | `capability.settings.load` | S164, S089, S091 |
| [`capability.options.find-examine`](../capabilities/native-coverage.json#L34625) — options.find-examine | `capability.settings.load` | S165, S089, S091 |
| [`capability.options.disturb-move`](../capabilities/native-coverage.json#L34674) — options.disturb-move | `capability.settings.load` | S166, S089, S091 |
| [`capability.options.disturb-near`](../capabilities/native-coverage.json#L34723) — options.disturb-near | `capability.settings.load` | S167, S089, S091 |
| [`capability.options.disturb-panel`](../capabilities/native-coverage.json#L34772) — options.disturb-panel | `capability.settings.load` | S168, S089, S091 |
| [`capability.options.disturb-state`](../capabilities/native-coverage.json#L34821) — options.disturb-state | `capability.settings.load` | S169, S089, S091 |
| [`capability.options.disturb-minor`](../capabilities/native-coverage.json#L34870) — options.disturb-minor | `capability.settings.load` | S170, S089, S091 |
| [`capability.options.disturb-other`](../capabilities/native-coverage.json#L34919) — options.disturb-other | `capability.settings.load` | S171, S089, S091 |
| [`capability.options.view-perma-grids`](../capabilities/native-coverage.json#L34968) — options.view-perma-grids | `capability.settings.load` | S172, S089, S091 |
| [`capability.options.view-torch-grids`](../capabilities/native-coverage.json#L35017) — options.view-torch-grids | `capability.settings.load` | S173, S089, S091 |
| [`capability.options.view-reduce-light`](../capabilities/native-coverage.json#L35066) — options.view-reduce-light | `capability.settings.load` | S174, S089, S091 |
| [`capability.options.view-reduce-view`](../capabilities/native-coverage.json#L35115) — options.view-reduce-view | `capability.settings.load` | S175, S089, S091 |
| [`capability.options.easy-open`](../capabilities/native-coverage.json#L35164) — options.easy-open | `capability.settings.load` | S176, S089, S091 |
| [`capability.options.easy-disarm`](../capabilities/native-coverage.json#L35213) — options.easy-disarm | `capability.settings.load` | S177, S089, S091 |
| [`capability.options.easy-tunnel`](../capabilities/native-coverage.json#L35262) — options.easy-tunnel | `capability.settings.load` | S178, S089, S091 |
| [`capability.options.audio-paging`](../capabilities/native-coverage.json#L35311) — options.audio-paging | `capability.settings.load` | S179, S089, S091 |
| [`capability.options.paging-master-vol`](../capabilities/native-coverage.json#L35360) — options.paging-master-vol | `capability.settings.load` | S180, S089, S091 |
| [`capability.options.paging-max-vol`](../capabilities/native-coverage.json#L35409) — options.paging-max-vol | `capability.settings.load` | S181, S089, S091 |
| [`capability.options.no-ovl-close-sfx`](../capabilities/native-coverage.json#L35458) — options.no-ovl-close-sfx | `capability.settings.load` | S182, S089, S091 |
| [`capability.options.ovl-sfx-attack`](../capabilities/native-coverage.json#L35507) — options.ovl-sfx-attack | `capability.settings.load` | S183, S089, S091 |
| [`capability.options.no-combat-sfx`](../capabilities/native-coverage.json#L35556) — options.no-combat-sfx | `capability.settings.load` | S184, S089, S091 |
| [`capability.options.no-magicattack-sfx`](../capabilities/native-coverage.json#L35605) — options.no-magicattack-sfx | `capability.settings.load` | S185, S089, S091 |
| [`capability.options.no-defense-sfx`](../capabilities/native-coverage.json#L35654) — options.no-defense-sfx | `capability.settings.load` | S186, S089, S091 |
| [`capability.options.half-sfx-attack`](../capabilities/native-coverage.json#L35703) — options.half-sfx-attack | `capability.settings.load` | S187, S089, S091 |
| [`capability.options.cut-sfx-attack`](../capabilities/native-coverage.json#L35752) — options.cut-sfx-attack | `capability.settings.load` | S188, S089, S091 |
| [`capability.options.ovl-sfx-command`](../capabilities/native-coverage.json#L35801) — options.ovl-sfx-command | `capability.settings.load` | S189, S089, S091 |
| [`capability.options.ovl-sfx-misc`](../capabilities/native-coverage.json#L35850) — options.ovl-sfx-misc | `capability.settings.load` | S190, S089, S091 |
| [`capability.options.ovl-sfx-mon-attack`](../capabilities/native-coverage.json#L35899) — options.ovl-sfx-mon-attack | `capability.settings.load` | S191, S089, S091 |
| [`capability.options.ovl-sfx-mon-spell`](../capabilities/native-coverage.json#L35948) — options.ovl-sfx-mon-spell | `capability.settings.load` | S192, S089, S091 |
| [`capability.options.ovl-sfx-mon-misc`](../capabilities/native-coverage.json#L35997) — options.ovl-sfx-mon-misc | `capability.settings.load` | S193, S089, S091 |
| [`capability.options.no-monsterattack-sfx`](../capabilities/native-coverage.json#L36046) — options.no-monsterattack-sfx | `capability.settings.load` | S194, S089, S091 |
| [`capability.options.positional-audio`](../capabilities/native-coverage.json#L36095) — options.positional-audio | `capability.settings.load` | S195, S089, S091 |
| [`capability.options.no-house-sfx`](../capabilities/native-coverage.json#L36144) — options.no-house-sfx | `capability.settings.load` | S196, S089, S091 |
| [`capability.options.quiet-house-sfx`](../capabilities/native-coverage.json#L36193) — options.quiet-house-sfx | `capability.settings.load` | S197, S089, S091 |
| [`capability.options.mute-when-idle`](../capabilities/native-coverage.json#L36242) — options.mute-when-idle | `capability.settings.load` | S198, S089, S091 |
| [`capability.options.alert-starvation`](../capabilities/native-coverage.json#L36291) — options.alert-starvation | `capability.settings.load` | S199, S089, S091 |
| [`capability.options.use-color`](../capabilities/native-coverage.json#L36340) — options.use-color | `capability.settings.load` | S200, S089, S091 |
| [`capability.options.other-query-flag`](../capabilities/native-coverage.json#L36389) — options.other-query-flag | `capability.settings.load` | S201, S089, S091 |
| [`capability.options.shuffle-music`](../capabilities/native-coverage.json#L36438) — options.shuffle-music | `capability.settings.load` | S202, S089, S091 |
| [`capability.options.permawalls-shade`](../capabilities/native-coverage.json#L36487) — options.permawalls-shade | `capability.settings.load` | S203, S089, S091 |
| [`capability.options.topline-no-msg`](../capabilities/native-coverage.json#L36536) — options.topline-no-msg | `capability.settings.load` | S204, S089, S091 |
| [`capability.options.targetinfo-msg`](../capabilities/native-coverage.json#L36585) — options.targetinfo-msg | `capability.settings.load` | S205, S089, S091 |
| [`capability.options.live-timeouts`](../capabilities/native-coverage.json#L36634) — options.live-timeouts | `capability.settings.load` | S206, S089, S091 |
| [`capability.options.flash-insane`](../capabilities/native-coverage.json#L36683) — options.flash-insane | `capability.settings.load` | S207, S089, S091 |
| [`capability.options.last-words`](../capabilities/native-coverage.json#L36732) — options.last-words | `capability.settings.load` | S208, S089, S091 |
| [`capability.options.disturb-see`](../capabilities/native-coverage.json#L36781) — options.disturb-see | `capability.settings.load` | S209, S089, S091 |
| [`capability.options.diz-unique`](../capabilities/native-coverage.json#L36830) — options.diz-unique | `capability.settings.load` | S210, S089, S091 |
| [`capability.options.diz-death`](../capabilities/native-coverage.json#L36879) — options.diz-death | `capability.settings.load` | S211, S089, S091 |
| [`capability.options.diz-death-any`](../capabilities/native-coverage.json#L36928) — options.diz-death-any | `capability.settings.load` | S212, S089, S091 |
| [`capability.options.diz-first`](../capabilities/native-coverage.json#L36977) — options.diz-first | `capability.settings.load` | S213, S089, S091 |
| [`capability.options.screenshot-format`](../capabilities/native-coverage.json#L37026) — options.screenshot-format | `capability.settings.load` | S214, S089, S091 |
| [`capability.options.palette-animation`](../capabilities/native-coverage.json#L37075) — options.palette-animation | `capability.settings.load` | S215, S089, S091 |
| [`capability.options.play-all`](../capabilities/native-coverage.json#L37124) — options.play-all | `capability.settings.load` | S216, S089, S091 |
| [`capability.options.id-selection`](../capabilities/native-coverage.json#L37173) — options.id-selection | `capability.settings.load` | S217, S089, S091 |
| [`capability.options.hp-bar`](../capabilities/native-coverage.json#L37222) — options.hp-bar | `capability.settings.load` | S218, S089, S091 |
| [`capability.options.mp-bar`](../capabilities/native-coverage.json#L37271) — options.mp-bar | `capability.settings.load` | S219, S089, S091 |
| [`capability.options.st-bar`](../capabilities/native-coverage.json#L37320) — options.st-bar | `capability.settings.load` | S220, S089, S091 |
| [`capability.options.find-ignore-montraps`](../capabilities/native-coverage.json#L37369) — options.find-ignore-montraps | `capability.settings.load` | S221, S089, S091 |
| [`capability.options.quiet-os`](../capabilities/native-coverage.json#L37418) — options.quiet-os | `capability.settings.load` | S222, S089, S091 |
| [`capability.options.disable-lightning`](../capabilities/native-coverage.json#L37467) — options.disable-lightning | `capability.settings.load` | S223, S089, S091 |
| [`capability.options.macros-in-stores`](../capabilities/native-coverage.json#L37516) — options.macros-in-stores | `capability.settings.load` | S224, S089, S091 |
| [`capability.options.item-error-beep`](../capabilities/native-coverage.json#L37565) — options.item-error-beep | `capability.settings.load` | S225, S089, S091 |
| [`capability.options.keep-bottle`](../capabilities/native-coverage.json#L37614) — options.keep-bottle | `capability.settings.load` | S226, S089, S091 |
| [`capability.options.easy-disarm-montraps`](../capabilities/native-coverage.json#L37663) — options.easy-disarm-montraps | `capability.settings.load` | S227, S089, S091 |
| [`capability.options.no-house-magic`](../capabilities/native-coverage.json#L37712) — options.no-house-magic | `capability.settings.load` | S228, S089, S091 |
| [`capability.options.no-light-fainting`](../capabilities/native-coverage.json#L37761) — options.no-light-fainting | `capability.settings.load` | S229, S089, S091 |
| [`capability.options.auto-pickup`](../capabilities/native-coverage.json#L37810) — options.auto-pickup | `capability.settings.load` | S230, S089, S091 |
| [`capability.options.auto-destroy`](../capabilities/native-coverage.json#L37859) — options.auto-destroy | `capability.settings.load` | S231, S089, S091 |
| [`capability.options.destroy-all-unmatched`](../capabilities/native-coverage.json#L37908) — options.destroy-all-unmatched | `capability.settings.load` | S232, S089, S091 |
| [`capability.options.mp-huge-bar`](../capabilities/native-coverage.json#L37957) — options.mp-huge-bar | `capability.settings.load` | S233, S089, S091 |
| [`capability.options.sn-huge-bar`](../capabilities/native-coverage.json#L38006) — options.sn-huge-bar | `capability.settings.load` | S234, S089, S091 |
| [`capability.options.hp-huge-bar`](../capabilities/native-coverage.json#L38055) — options.hp-huge-bar | `capability.settings.load` | S235, S089, S091 |
| [`capability.options.clone-to-stdout`](../capabilities/native-coverage.json#L38104) — options.clone-to-stdout | `capability.settings.load` | S236, S089, S091 |
| [`capability.options.clone-to-file`](../capabilities/native-coverage.json#L38153) — options.clone-to-file | `capability.settings.load` | S237, S089, S091 |
| [`capability.options.first-song`](../capabilities/native-coverage.json#L38202) — options.first-song | `capability.settings.load` | S238, S089, S091 |
| [`capability.options.equip-text-colour`](../capabilities/native-coverage.json#L38251) — options.equip-text-colour | `capability.settings.load` | S239, S089, S091 |
| [`capability.options.equip-set-colour`](../capabilities/native-coverage.json#L38300) — options.equip-set-colour | `capability.settings.load` | S240, S089, S091 |
| [`capability.options.colourize-bignum`](../capabilities/native-coverage.json#L38349) — options.colourize-bignum | `capability.settings.load` | S241, S089, S091 |
| [`capability.options.flash-player2`](../capabilities/native-coverage.json#L38398) — options.flash-player2 | `capability.settings.load` | S242, S089, S091 |
| [`capability.options.load-form-macros`](../capabilities/native-coverage.json#L38447) — options.load-form-macros | `capability.settings.load` | S243, S089, S091 |
| [`capability.options.auto-inscr-off`](../capabilities/native-coverage.json#L38496) — options.auto-inscr-off | `capability.settings.load` | S244, S089, S091 |
| [`capability.options.ascii-feats`](../capabilities/native-coverage.json#L38545) — options.ascii-feats | `capability.settings.load` | S245, S089, S091 |
| [`capability.options.ascii-items`](../capabilities/native-coverage.json#L38594) — options.ascii-items | `capability.settings.load` | S246, S089, S091 |
| [`capability.options.ascii-monsters`](../capabilities/native-coverage.json#L38643) — options.ascii-monsters | `capability.settings.load` | S247, S089, S091 |
| [`capability.options.ascii-uniques`](../capabilities/native-coverage.json#L38692) — options.ascii-uniques | `capability.settings.load` | S248, S089, S091 |
| [`capability.options.no-flicker`](../capabilities/native-coverage.json#L38741) — options.no-flicker | `capability.settings.load` | S249, S089, S091 |
| [`capability.options.stun-huge-bar`](../capabilities/native-coverage.json#L38790) — options.stun-huge-bar | `capability.settings.load` | S250, S089, S091 |
| [`capability.options.gfx-autooff-fmsw`](../capabilities/native-coverage.json#L38839) — options.gfx-autooff-fmsw | `capability.settings.load` | S251, S089, S091 |
| [`capability.options.solid-bars`](../capabilities/native-coverage.json#L38888) — options.solid-bars | `capability.settings.load` | S252, S089, S091 |
| [`capability.options.autopickup-chemicals`](../capabilities/native-coverage.json#L38937) — options.autopickup-chemicals | `capability.settings.load` | S253, S089, S091 |
| [`capability.options.add-kind-diz`](../capabilities/native-coverage.json#L38986) — options.add-kind-diz | `capability.settings.load` | S254, S089, S091 |
| [`capability.options.hide-lore-paste`](../capabilities/native-coverage.json#L39035) — options.hide-lore-paste | `capability.settings.load` | S255, S089, S091 |
| [`capability.options.new-retaliator`](../capabilities/native-coverage.json#L39084) — options.new-retaliator | `capability.settings.load` | S256, S089, S091 |
| [`capability.options.sunburn-msg`](../capabilities/native-coverage.json#L39133) — options.sunburn-msg | `capability.settings.load` | S257, S089, S091 |
| [`capability.options.wide-scroll-margin`](../capabilities/native-coverage.json#L39182) — options.wide-scroll-margin | `capability.settings.load` | S258, S089, S091 |
| [`capability.options.topline-first`](../capabilities/native-coverage.json#L39231) — options.topline-first | `capability.settings.load` | S259, S089, S091 |
| [`capability.options.ascii-weather`](../capabilities/native-coverage.json#L39280) — options.ascii-weather | `capability.settings.load` | S260, S089, S091 |
| [`capability.options.no2mask-weather`](../capabilities/native-coverage.json#L39329) — options.no2mask-weather | `capability.settings.load` | S261, S089, S091 |
| [`capability.options.auto-inscr-server-ch`](../capabilities/native-coverage.json#L39378) — options.auto-inscr-server-ch | `capability.settings.load` | S262, S089, S091 |
| [`capability.options.wild-resume-from-any`](../capabilities/native-coverage.json#L39427) — options.wild-resume-from-any | `capability.settings.load` | S263, S089, S091 |
| [`capability.options.tavern-town-resume`](../capabilities/native-coverage.json#L39476) — options.tavern-town-resume | `capability.settings.load` | S264, S089, S091 |
| [`capability.options.st-huge-bar`](../capabilities/native-coverage.json#L39525) — options.st-huge-bar | `capability.settings.load` | S265, S089, S091 |
| [`capability.options.huge-bars-gfx`](../capabilities/native-coverage.json#L39574) — options.huge-bars-gfx | `capability.settings.load` | S266, S089, S091 |
| [`capability.options.autoinsc-debug`](../capabilities/native-coverage.json#L39623) — options.autoinsc-debug | `capability.settings.load` | S267, S089, S091 |
| [`capability.options.autoloot-dunonly`](../capabilities/native-coverage.json#L39672) — options.autoloot-dunonly | `capability.settings.load` | S268, S089, S091 |
| [`capability.options.autoloot-dununown`](../capabilities/native-coverage.json#L39721) — options.autoloot-dununown | `capability.settings.load` | S269, S089, S091 |
| [`capability.options.flash-starvation`](../capabilities/native-coverage.json#L39770) — options.flash-starvation | `capability.settings.load` | S270, S089, S091 |
| [`capability.options.autoswitch-inven`](../capabilities/native-coverage.json#L39819) — options.autoswitch-inven | `capability.settings.load` | S271, S089, S091 |
| [`capability.options.show-newest`](../capabilities/native-coverage.json#L39868) — options.show-newest | `capability.settings.load` | S272, S089, S091 |
| [`capability.options.log-music`](../capabilities/native-coverage.json#L39917) — options.log-music | `capability.settings.load` | S273, S089, S091 |
| [`capability.options.misc-no-flicker`](../capabilities/native-coverage.json#L39966) — options.misc-no-flicker | `capability.settings.load` | S274, S089, S091 |
| [`capability.options.prefer-subinven`](../capabilities/native-coverage.json#L40015) — options.prefer-subinven | `capability.settings.load` | S275, S089, S091 |

## Приложение B. Источники всех B capabilities

Каталог фиксирует все 395 прямых source records, в исходном порядке manifest. Ссылка Source ID открывает его запись (роль, repository, полный revision, SHA). В locator указан path, короткая revision, anchor и **первое буквальное совпадение anchor**. Это машинный locator; для dispatch/prototype совпадений ручная проверка должна перейти к телу. Для каждого source `hashMatches=true`, anchor найден. Полные revisions встречаются в manifest и перечислены ниже; SHA каждого файла доступен по ссылке Source ID.

| Короткая revision | Полная revision | Source records |
|---|---|---:|
| `0ef6b36` | `0ef6b362cc7676805fdd0becff680dac50fe83c1` | 43 |
| `bbe7417` | `bbe7417e21d20470a7aeb075aa82650608f6e4ae` | 35 |
| `399fdd3` | `399fdd38211035b0810cf721ba8e0f1efecf5629` | 10 |
| `33d3582` | `33d3582f8b73d5a853106bf7837ca28285bf9052` | 250 |
| `a3d99ee` | `a3d99eeb4e2247ba0c23a17e57fa502ee931f6e3` | 57 |

| Ref | Source ID | Первичный locator: revision, path, anchor:first match |
|---|---|---|
| S001 | [`source.policy.session`](../capabilities/manifest.json#L95) | `worktree; declared 0ef6b36` [docs/capabilities/session-policy.md](../../docs/capabilities/session-policy.md), `## Answer` : 10 |
| S002 | [`source.baseline.session-server`](../capabilities/manifest.json#L104) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `get_server_name` : 2482 |
| S003 | [`source.baseline.session-credentials`](../capabilities/manifest.json#L113) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_name` : 66 |
| S004 | [`source.baseline.session-password`](../capabilities/manifest.json#L122) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `enter_password` : 265 |
| S005 | [`source.baseline.session-birth`](../capabilities/manifest.json#L131) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_sex` : 318 |
| S006 | [`source.baseline.session-race`](../capabilities/manifest.json#L140) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_race` : 423 |
| S007 | [`source.baseline.session-trait`](../capabilities/manifest.json#L149) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_trait` : 645 |
| S008 | [`source.baseline.session-class`](../capabilities/manifest.json#L158) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_class` : 887 |
| S009 | [`source.baseline.session-stats`](../capabilities/manifest.json#L167) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_stat_order` : 1129 |
| S010 | [`source.baseline.session-mode`](../capabilities/manifest.json#L176) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_mode` : 671 |
| S011 | [`source.baseline.session-body`](../capabilities/manifest.json#L185) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `choose_body_modification` : 1899 |
| S012 | [`source.baseline.session-dna`](../capabilities/manifest.json#L194) | `0ef6b36` [src/client/c-birth.c](../../src/client/c-birth.c), `Save Birth DNA` : 2344 |
| S013 | [`source.baseline.session-connect`](../capabilities/manifest.json#L203) | `0ef6b36` [src/client/c-init.c](../../src/client/c-init.c), `Net_verify` : 4349 |
| S014 | [`source.baseline.session-login`](../capabilities/manifest.json#L212) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Receive_login` : 762 |
| S015 | [`source.baseline.session-reorder`](../capabilities/manifest.json#L221) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `reorder_characters` : 702 |
| S016 | [`source.baseline.session-verify`](../capabilities/manifest.json#L230) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Net_verify` : 1426 |
| S017 | [`source.baseline.session-setup`](../capabilities/manifest.json#L239) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Net_setup` : 18 |
| S018 | [`source.baseline.session-play`](../capabilities/manifest.json#L248) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Net_start` : 1672 |
| S019 | [`source.baseline.session-quit`](../capabilities/manifest.json#L257) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Receive_quit` : 344 |
| S020 | [`source.baseline.session-portal`](../capabilities/manifest.json#L266) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Receive_relogin` : 346 |
| S021 | [`source.baseline.session-motd`](../capabilities/manifest.json#L275) | `0ef6b36` [src/client/c-files.c](../../src/client/c-files.c), `show_motd` : 1935 |
| S022 | [`source.baseline.session-shutdown`](../capabilities/manifest.json#L284) | `0ef6b36` [src/client/c-util.c](../../src/client/c-util.c), `c_close_game` : 17707 |
| S023 | [`source.baseline.session-account`](../capabilities/manifest.json#L293) | `0ef6b36` [src/client/c-util.c](../../src/client/c-util.c), `Change account password` : 13174 |
| S024 | [`source.baseline.session-change-password`](../capabilities/manifest.json#L302) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Send_change_password` : 8154 |
| S025 | [`source.baseline.session-direction`](../capabilities/manifest.json#L311) | `0ef6b36` [src/client/c-util.c](../../src/client/c-util.c), `bool get_dir` : 3672 |
| S026 | [`source.baseline.session-target`](../capabilities/manifest.json#L320) | `0ef6b36` [src/client/c-cmd.c](../../src/client/c-cmd.c), `int cmd_target(void)` : 2040 |
| S027 | [`source.baseline.session-minimap`](../capabilities/manifest.json#L347) | `0ef6b36` [src/client/c-cmd.c](../../src/client/c-cmd.c), `void cmd_mini_map` : 555 |
| S028 | [`source.baseline.session-locate`](../capabilities/manifest.json#L356) | `0ef6b36` [src/client/c-cmd.c](../../src/client/c-cmd.c), `void cmd_locate` : 788 |
| S029 | [`source.baseline.session-movement`](../capabilities/manifest.json#L365) | `0ef6b36` [src/client/c-cmd.c](../../src/client/c-cmd.c), `void cmd_walk` : 528 |
| S030 | [`source.baseline.session-commands`](../capabilities/manifest.json#L374) | `0ef6b36` [src/client/c-cmd.c](../../src/client/c-cmd.c), `void process_command` : 274 |
| S031 | [`source.baseline.session-suicide`](../capabilities/manifest.json#L383) | `0ef6b36` [src/client/c-cmd.c](../../src/client/c-cmd.c), `void cmd_suicide` : 9297 |
| S032 | [`source.baseline.session-keymap`](../capabilities/manifest.json#L392) | `0ef6b36` [src/client/c-util.c](../../src/client/c-util.c), `keymap_init` : 1907 |
| S033 | [`source.baseline.session-target-info`](../capabilities/manifest.json#L401) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `int Receive_target_info` : 5515 |
| S034 | [`source.baseline.session-depth`](../capabilities/manifest.json#L410) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `int Receive_depth` : 3624 |
| S035 | [`source.baseline.session-file`](../capabilities/manifest.json#L428) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Receive_file` : 351 |
| S036 | [`source.baseline.session-map`](../capabilities/manifest.json#L437) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Receive_line_info` : 385 |
| S037 | [`source.baseline.session-identity`](../capabilities/manifest.json#L446) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Receive_char_info` : 358 |
| S038 | [`source.baseline.session-state`](../capabilities/manifest.json#L455) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Receive_state` : 367 |
| S039 | [`source.protocol.session-login`](../capabilities/manifest.json#L464) | `0ef6b36` [src/server/nserver.c](../../src/server/nserver.c), `Receive_login` : 320 |
| S040 | [`source.protocol.session-password`](../capabilities/manifest.json#L473) | `0ef6b36` [src/server/nserver.c](../../src/server/nserver.c), `Receive_change_password` : 424 |
| S041 | [`source.protocol.session-wire`](../capabilities/manifest.json#L482) | `0ef6b36` [src/client/nclient.c](../../src/client/nclient.c), `Send_walk` : 7536 |
| S042 | [`source.baseline.session-macros`](../capabilities/manifest.json#L491) | `0ef6b36` [lib/user/pref-sdl3.prf](../../lib/user/pref-sdl3.prf), `A:` : 21 |
| S043 | [`source.baseline.session-general-macros`](../capabilities/manifest.json#L500) | `0ef6b36` [lib/user/pref.prf](../../lib/user/pref.prf), `A:` : 51 |
| S044 | [`source.acceptance.items-allocation`](../capabilities/manifest.json#L509) | `worktree; declared bbe7417` [docs/capabilities/item-policy.md](../../docs/capabilities/item-policy.md), `## Acceptance sequence` : 5 |
| S045 | [`source.behavior.items-receive-inven`](../capabilities/manifest.json#L527) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_inven` : 356 |
| S046 | [`source.protocol.items-receive-inven`](../capabilities/manifest.json#L536) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_inven` : 356 |
| S047 | [`source.behavior.items-receive-equip`](../capabilities/manifest.json#L545) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_equip` : 357 |
| S048 | [`source.protocol.items-receive-equip-wide`](../capabilities/manifest.json#L554) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_equip_wide` : 468 |
| S049 | [`source.behavior.items-receive-subinven`](../capabilities/manifest.json#L563) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_subinven` : 464 |
| S050 | [`source.protocol.items-receive-subinven`](../capabilities/manifest.json#L572) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_subinven` : 464 |
| S051 | [`source.behavior.items-receive-floor`](../capabilities/manifest.json#L581) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_floor` : 394 |
| S052 | [`source.protocol.items-receive-whats-under-you-feet`](../capabilities/manifest.json#L590) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_whats_under_you_feet` : 460 |
| S053 | [`source.behavior.items-receive-pickup-check`](../capabilities/manifest.json#L599) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_pickup_check` : 395 |
| S054 | [`source.protocol.items-send-stay`](../capabilities/manifest.json#L608) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Send_stay` : 5965 |
| S055 | [`source.behavior.items-receive-store-info`](../capabilities/manifest.json#L617) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store_info` : 388 |
| S056 | [`source.protocol.items-receive-store-info`](../capabilities/manifest.json#L626) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store_info` : 388 |
| S057 | [`source.behavior.items-receive-store`](../capabilities/manifest.json#L635) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store` : 387 |
| S058 | [`source.protocol.items-receive-store-wide`](../capabilities/manifest.json#L644) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store_wide` : 425 |
| S059 | [`source.behavior.items-receive-store-action`](../capabilities/manifest.json#L653) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store_action` : 409 |
| S060 | [`source.protocol.items-receive-store-action`](../capabilities/manifest.json#L662) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store_action` : 409 |
| S061 | [`source.behavior.items-display-store`](../capabilities/manifest.json#L671) | `bbe7417` [src/client/c-store.c](../../src/client/c-store.c), `display_store` : 16 |
| S062 | [`source.protocol.items-send-store-leave`](../capabilities/manifest.json#L680) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Send_store_leave` : 7942 |
| S063 | [`source.behavior.items-receive-store-kick`](../capabilities/manifest.json#L689) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store_kick` : 407 |
| S064 | [`source.protocol.items-receive-store-kick`](../capabilities/manifest.json#L698) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store_kick` : 407 |
| S065 | [`source.protocol.items-send-inscribe`](../capabilities/manifest.json#L860) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Send_inscribe` : 7012 |
| S066 | [`source.behavior.items-receive-apply-auto-insc`](../capabilities/manifest.json#L1679) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_apply_auto_insc` : 428 |
| S067 | [`source.behavior.items-cmd-inven`](../capabilities/manifest.json#L1724) | `bbe7417` [src/client/c-cmd.c](../../src/client/c-cmd.c), `cmd_inven` : 333 |
| S068 | [`source.behavior.items-cmd-equip`](../capabilities/manifest.json#L1742) | `bbe7417` [src/client/c-cmd.c](../../src/client/c-cmd.c), `cmd_equip` : 334 |
| S069 | [`source.behavior.items-cmd-subinven`](../capabilities/manifest.json#L1751) | `bbe7417` [src/client/c-cmd.c](../../src/client/c-cmd.c), `cmd_subinven` : 897 |
| S070 | [`source.protocol.items-send-inventory-revision`](../capabilities/manifest.json#L1778) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Send_inventory_revision` : 6627 |
| S071 | [`source.protocol.items-receive-inven-wide`](../capabilities/manifest.json#L1787) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_inven_wide` : 420 |
| S072 | [`source.protocol.items-receive-equip`](../capabilities/manifest.json#L1796) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_equip` : 357 |
| S073 | [`source.protocol.items-receive-inventory-revision`](../capabilities/manifest.json#L1805) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_inventory_revision` : 423 |
| S074 | [`source.protocol.items-receive-apply-auto-insc`](../capabilities/manifest.json#L1814) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_apply_auto_insc` : 428 |
| S075 | [`source.protocol.items-receive-floor`](../capabilities/manifest.json#L1823) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_floor` : 394 |
| S076 | [`source.protocol.items-receive-store`](../capabilities/manifest.json#L1904) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_store` : 387 |
| S077 | [`source.protocol.items-receive-pickup-check`](../capabilities/manifest.json#L1913) | `bbe7417` [src/client/nclient.c](../../src/client/nclient.c), `Receive_pickup_check` : 395 |
| S078 | [`source.behavior.items-c-inven-callbacks`](../capabilities/manifest.json#L1985) | `bbe7417` [src/client/c-inven.c](../../src/client/c-inven.c), `c_get_item` : 117 |
| S079 | [`source.acceptance.information-allocation`](../capabilities/manifest.json#L1994) | `399fdd3` [docs/capabilities/item-policy.md](../../docs/capabilities/item-policy.md), `## Acceptance sequence` : 5 |
| S080 | [`source.behavior.information-cmd-character`](../capabilities/manifest.json#L2012) | `399fdd3` [src/client/c-cmd.c](../../src/client/c-cmd.c), `cmd_character` : 382 |
| S081 | [`source.protocol.information-receive-history`](../capabilities/manifest.json#L2021) | `399fdd3` [src/client/nclient.c](../../src/client/nclient.c), `Receive_history` : 364 |
| S082 | [`source.behavior.information-do-cmd-messages`](../capabilities/manifest.json#L2030) | `399fdd3` [src/client/c-xtra2.c](../../src/client/c-xtra2.c), `do_cmd_messages` : 144 |
| S083 | [`source.behavior.information-do-cmd-messages-important`](../capabilities/manifest.json#L2039) | `399fdd3` [src/client/c-xtra2.c](../../src/client/c-xtra2.c), `do_cmd_messages_important` : 594 |
| S084 | [`source.behavior.information-cmd-message`](../capabilities/manifest.json#L2048) | `399fdd3` [src/client/c-cmd.c](../../src/client/c-cmd.c), `cmd_message` : 412 |
| S085 | [`source.protocol.information-send-msg`](../capabilities/manifest.json#L2057) | `399fdd3` [src/client/nclient.c](../../src/client/nclient.c), `Send_msg` : 6791 |
| S086 | [`source.behavior.information-receive-message`](../capabilities/manifest.json#L2867) | `399fdd3` [src/client/nclient.c](../../src/client/nclient.c), `Receive_message` : 366 |
| S087 | [`source.protocol.information-receive-message`](../capabilities/manifest.json#L2876) | `399fdd3` [src/client/nclient.c](../../src/client/nclient.c), `Receive_message` : 366 |
| S088 | [`source.behavior.information-askfor-aux`](../capabilities/manifest.json#L2921) | `399fdd3` [src/client/c-util.c](../../src/client/c-util.c), `askfor_aux` : 1649 |
| S089 | [`source.acceptance.settings-policy-f2aa80`](../capabilities/manifest.json#L3056) | `worktree; declared 33d3582` [docs/capabilities/settings-policy.md](../../docs/capabilities/settings-policy.md), `## Final persistence contract` : 8 |
| S090 | [`source.behavior.settings-read-mangrc-49a7d2`](../capabilities/manifest.json#L3074) | `33d3582` [src/client/client.c](../../src/client/client.c), `read_mangrc` : 69 |
| S091 | [`source.behavior.settings-option-parser-3df064`](../capabilities/manifest.json#L3083) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `process_pref_file_aux_aux` : 1047 |
| S092 | [`source.behavior.settings-rogue-like-commands-3fda3a`](../capabilities/manifest.json#L3092) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"rogue_like_commands"` : 189 |
| S093 | [`source.behavior.settings-newbie-hints-3fda3a`](../capabilities/manifest.json#L3101) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"newbie_hints"` : 191 |
| S094 | [`source.behavior.settings-censor-swearing-3fda3a`](../capabilities/manifest.json#L3110) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"censor_swearing"` : 193 |
| S095 | [`source.behavior.settings-highlight-chat-3fda3a`](../capabilities/manifest.json#L3119) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"highlight_chat"` : 196 |
| S096 | [`source.behavior.settings-highbeep-chat-3fda3a`](../capabilities/manifest.json#L3128) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"highbeep_chat"` : 198 |
| S097 | [`source.behavior.settings-page-on-privmsg-3fda3a`](../capabilities/manifest.json#L3137) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"page_on_privmsg"` : 200 |
| S098 | [`source.behavior.settings-page-on-afk-privmsg-3fda3a`](../capabilities/manifest.json#L3146) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"page_on_afk_privmsg"` : 202 |
| S099 | [`source.behavior.settings-font-map-solid-walls-3fda3a`](../capabilities/manifest.json#L3155) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"font_map_solid_walls"` : 214 |
| S100 | [`source.behavior.settings-view-animated-light-3fda3a`](../capabilities/manifest.json#L3164) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_animated_light"` : 216 |
| S101 | [`source.behavior.settings-wall-lighting-3fda3a`](../capabilities/manifest.json#L3173) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"wall_lighting"` : 218 |
| S102 | [`source.behavior.settings-view-lamp-walls-3fda3a`](../capabilities/manifest.json#L3182) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_lamp_walls"` : 220 |
| S103 | [`source.behavior.settings-view-shade-walls-3fda3a`](../capabilities/manifest.json#L3191) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_shade_walls"` : 222 |
| S104 | [`source.behavior.settings-floor-lighting-3fda3a`](../capabilities/manifest.json#L3200) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"floor_lighting"` : 224 |
| S105 | [`source.behavior.settings-view-lamp-floor-3fda3a`](../capabilities/manifest.json#L3209) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_lamp_floor"` : 226 |
| S106 | [`source.behavior.settings-view-shade-floor-3fda3a`](../capabilities/manifest.json#L3218) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_shade_floor"` : 228 |
| S107 | [`source.behavior.settings-view-light-extra-3fda3a`](../capabilities/manifest.json#L3227) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_light_extra"` : 230 |
| S108 | [`source.behavior.settings-alert-hitpoint-3fda3a`](../capabilities/manifest.json#L3236) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"alert_hitpoint"` : 233 |
| S109 | [`source.behavior.settings-alert-mana-3fda3a`](../capabilities/manifest.json#L3245) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"alert_mana"` : 235 |
| S110 | [`source.behavior.settings-alert-afk-dam-3fda3a`](../capabilities/manifest.json#L3254) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"alert_afk_dam"` : 237 |
| S111 | [`source.behavior.settings-alert-offpanel-dam-3fda3a`](../capabilities/manifest.json#L3263) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"alert_offpanel_dam"` : 239 |
| S112 | [`source.behavior.settings-exp-bar-3fda3a`](../capabilities/manifest.json#L3272) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"exp_bar"` : 242 |
| S113 | [`source.behavior.settings-uniques-alive-3fda3a`](../capabilities/manifest.json#L3281) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"uniques_alive"` : 245 |
| S114 | [`source.behavior.settings-warn-unique-credit-3fda3a`](../capabilities/manifest.json#L3290) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"warn_unique_credit"` : 247 |
| S115 | [`source.behavior.settings-limit-chat-3fda3a`](../capabilities/manifest.json#L3299) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"limit_chat"` : 249 |
| S116 | [`source.behavior.settings-no-afk-msg-3fda3a`](../capabilities/manifest.json#L3308) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_afk_msg"` : 251 |
| S117 | [`source.behavior.settings-overview-startup-3fda3a`](../capabilities/manifest.json#L3317) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"overview_startup"` : 253 |
| S118 | [`source.behavior.settings-allow-paging-3fda3a`](../capabilities/manifest.json#L3326) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"allow_paging"` : 256 |
| S119 | [`source.behavior.settings-ring-bell-3fda3a`](../capabilities/manifest.json#L3335) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ring_bell"` : 258 |
| S120 | [`source.behavior.settings-linear-stats-3fda3a`](../capabilities/manifest.json#L3344) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"linear_stats"` : 262 |
| S121 | [`source.behavior.settings-exp-need-3fda3a`](../capabilities/manifest.json#L3353) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"exp_need"` : 264 |
| S122 | [`source.behavior.settings-depth-in-feet-3fda3a`](../capabilities/manifest.json#L3362) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"depth_in_feet"` : 266 |
| S123 | [`source.behavior.settings-newb-suicide-3fda3a`](../capabilities/manifest.json#L3371) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"newb_suicide"` : 268 |
| S124 | [`source.behavior.settings-show-weights-3fda3a`](../capabilities/manifest.json#L3380) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"show_weights"` : 270 |
| S125 | [`source.behavior.settings-time-stamp-chat-3fda3a`](../capabilities/manifest.json#L3389) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"time_stamp_chat"` : 274 |
| S126 | [`source.behavior.settings-hide-unusable-skills-3fda3a`](../capabilities/manifest.json#L3398) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"hide_unusable_skills"` : 276 |
| S127 | [`source.behavior.settings-short-item-names-3fda3a`](../capabilities/manifest.json#L3407) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"short_item_names"` : 278 |
| S128 | [`source.behavior.settings-keep-topline-3fda3a`](../capabilities/manifest.json#L3416) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"keep_topline"` : 280 |
| S129 | [`source.behavior.settings-target-history-3fda3a`](../capabilities/manifest.json#L3425) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"target_history"` : 282 |
| S130 | [`source.behavior.settings-taciturn-messages-3fda3a`](../capabilities/manifest.json#L3434) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"taciturn_messages"` : 284 |
| S131 | [`source.behavior.settings-always-show-lists-3fda3a`](../capabilities/manifest.json#L3443) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"always_show_lists"` : 286 |
| S132 | [`source.behavior.settings-no-weather-3fda3a`](../capabilities/manifest.json#L3452) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_weather"` : 289 |
| S133 | [`source.behavior.settings-player-list-3fda3a`](../capabilities/manifest.json#L3461) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"player_list"` : 292 |
| S134 | [`source.behavior.settings-player-list2-3fda3a`](../capabilities/manifest.json#L3470) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"player_list2"` : 294 |
| S135 | [`source.behavior.settings-flash-player-3fda3a`](../capabilities/manifest.json#L3479) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"flash_player"` : 297 |
| S136 | [`source.behavior.settings-highlight-player-3fda3a`](../capabilities/manifest.json#L3488) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"highlight_player"` : 300 |
| S137 | [`source.behavior.settings-basic-players-symb-3fda3a`](../capabilities/manifest.json#L3497) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"basic_players_symb"` : 302 |
| S138 | [`source.behavior.settings-subterm-flicker-3fda3a`](../capabilities/manifest.json#L3506) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"subterm_flicker"` : 305 |
| S139 | [`source.behavior.settings-no-verify-sell-3fda3a`](../capabilities/manifest.json#L3515) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_verify_sell"` : 307 |
| S140 | [`source.behavior.settings-no-verify-destroy-3fda3a`](../capabilities/manifest.json#L3524) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_verify_destroy"` : 309 |
| S141 | [`source.behavior.settings-auto-afk-3fda3a`](../capabilities/manifest.json#L3533) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_afk"` : 312 |
| S142 | [`source.behavior.settings-idle-starve-kick-3fda3a`](../capabilities/manifest.json#L3542) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"idle_starve_kick"` : 314 |
| S143 | [`source.behavior.settings-safe-float-3fda3a`](../capabilities/manifest.json#L3551) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"safe_float"` : 316 |
| S144 | [`source.behavior.settings-safe-macros-3fda3a`](../capabilities/manifest.json#L3560) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"safe_macros"` : 318 |
| S145 | [`source.behavior.settings-auto-untag-3fda3a`](../capabilities/manifest.json#L3569) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_untag"` : 321 |
| S146 | [`source.behavior.settings-clear-inscr-3fda3a`](../capabilities/manifest.json#L3578) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"clear_inscr"` : 323 |
| S147 | [`source.behavior.settings-auto-inscr-server-3fda3a`](../capabilities/manifest.json#L3587) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_inscr_server"` : 325 |
| S148 | [`source.behavior.settings-stack-force-notes-3fda3a`](../capabilities/manifest.json#L3596) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"stack_force_notes"` : 327 |
| S149 | [`source.behavior.settings-stack-force-costs-3fda3a`](../capabilities/manifest.json#L3605) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"stack_force_costs"` : 329 |
| S150 | [`source.behavior.settings-stack-allow-items-3fda3a`](../capabilities/manifest.json#L3614) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"stack_allow_items"` : 331 |
| S151 | [`source.behavior.settings-stack-allow-devices-3fda3a`](../capabilities/manifest.json#L3623) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"stack_allow_devices"` : 333 |
| S152 | [`source.behavior.settings-whole-ammo-stack-3fda3a`](../capabilities/manifest.json#L3632) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"whole_ammo_stack"` : 335 |
| S153 | [`source.behavior.settings-always-repeat-3fda3a`](../capabilities/manifest.json#L3641) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"always_repeat"` : 337 |
| S154 | [`source.behavior.settings-always-pickup-3fda3a`](../capabilities/manifest.json#L3650) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"always_pickup"` : 339 |
| S155 | [`source.behavior.settings-use-old-target-3fda3a`](../capabilities/manifest.json#L3659) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"use_old_target"` : 341 |
| S156 | [`source.behavior.settings-autooff-retaliator-3fda3a`](../capabilities/manifest.json#L3668) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"autooff_retaliator"` : 343 |
| S157 | [`source.behavior.settings-fail-no-melee-3fda3a`](../capabilities/manifest.json#L3677) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"fail_no_melee"` : 345 |
| S158 | [`source.behavior.settings-basic-players-col-3fda3a`](../capabilities/manifest.json#L3686) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"basic_players_col"` : 347 |
| S159 | [`source.behavior.settings-auto-target-3fda3a`](../capabilities/manifest.json#L3695) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_target"` : 349 |
| S160 | [`source.behavior.settings-thin-down-flush-3fda3a`](../capabilities/manifest.json#L3704) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"thin_down_flush"` : 351 |
| S161 | [`source.behavior.settings-disable-flush-3fda3a`](../capabilities/manifest.json#L3713) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disable_flush"` : 353 |
| S162 | [`source.behavior.settings-find-ignore-stairs-3fda3a`](../capabilities/manifest.json#L3722) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"find_ignore_stairs"` : 356 |
| S163 | [`source.behavior.settings-find-ignore-doors-3fda3a`](../capabilities/manifest.json#L3731) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"find_ignore_doors"` : 358 |
| S164 | [`source.behavior.settings-find-cut-3fda3a`](../capabilities/manifest.json#L3740) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"find_cut"` : 360 |
| S165 | [`source.behavior.settings-find-examine-3fda3a`](../capabilities/manifest.json#L3749) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"find_examine"` : 362 |
| S166 | [`source.behavior.settings-disturb-move-3fda3a`](../capabilities/manifest.json#L3758) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disturb_move"` : 364 |
| S167 | [`source.behavior.settings-disturb-near-3fda3a`](../capabilities/manifest.json#L3767) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disturb_near"` : 366 |
| S168 | [`source.behavior.settings-disturb-panel-3fda3a`](../capabilities/manifest.json#L3776) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disturb_panel"` : 368 |
| S169 | [`source.behavior.settings-disturb-state-3fda3a`](../capabilities/manifest.json#L3785) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disturb_state"` : 370 |
| S170 | [`source.behavior.settings-disturb-minor-3fda3a`](../capabilities/manifest.json#L3794) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disturb_minor"` : 372 |
| S171 | [`source.behavior.settings-disturb-other-3fda3a`](../capabilities/manifest.json#L3803) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disturb_other"` : 374 |
| S172 | [`source.behavior.settings-view-perma-grids-3fda3a`](../capabilities/manifest.json#L3812) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_perma_grids"` : 376 |
| S173 | [`source.behavior.settings-view-torch-grids-3fda3a`](../capabilities/manifest.json#L3821) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_torch_grids"` : 378 |
| S174 | [`source.behavior.settings-view-reduce-light-3fda3a`](../capabilities/manifest.json#L3830) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_reduce_light"` : 382 |
| S175 | [`source.behavior.settings-view-reduce-view-3fda3a`](../capabilities/manifest.json#L3839) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"view_reduce_view"` : 384 |
| S176 | [`source.behavior.settings-easy-open-3fda3a`](../capabilities/manifest.json#L3848) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"easy_open"` : 387 |
| S177 | [`source.behavior.settings-easy-disarm-3fda3a`](../capabilities/manifest.json#L3857) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"easy_disarm"` : 389 |
| S178 | [`source.behavior.settings-easy-tunnel-3fda3a`](../capabilities/manifest.json#L3866) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"easy_tunnel"` : 391 |
| S179 | [`source.behavior.settings-audio-paging-3fda3a`](../capabilities/manifest.json#L3875) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"audio_paging"` : 394 |
| S180 | [`source.behavior.settings-paging-master-vol-3fda3a`](../capabilities/manifest.json#L3884) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"paging_master_vol"` : 396 |
| S181 | [`source.behavior.settings-paging-max-vol-3fda3a`](../capabilities/manifest.json#L3893) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"paging_max_vol"` : 398 |
| S182 | [`source.behavior.settings-no-ovl-close-sfx-3fda3a`](../capabilities/manifest.json#L3902) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_ovl_close_sfx"` : 400 |
| S183 | [`source.behavior.settings-ovl-sfx-attack-3fda3a`](../capabilities/manifest.json#L3911) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ovl_sfx_attack"` : 402 |
| S184 | [`source.behavior.settings-no-combat-sfx-3fda3a`](../capabilities/manifest.json#L3920) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_combat_sfx"` : 404 |
| S185 | [`source.behavior.settings-no-magicattack-sfx-3fda3a`](../capabilities/manifest.json#L3929) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_magicattack_sfx"` : 406 |
| S186 | [`source.behavior.settings-no-defense-sfx-3fda3a`](../capabilities/manifest.json#L3938) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_defense_sfx"` : 408 |
| S187 | [`source.behavior.settings-half-sfx-attack-3fda3a`](../capabilities/manifest.json#L3947) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"half_sfx_attack"` : 410 |
| S188 | [`source.behavior.settings-cut-sfx-attack-3fda3a`](../capabilities/manifest.json#L3956) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"cut_sfx_attack"` : 412 |
| S189 | [`source.behavior.settings-ovl-sfx-command-3fda3a`](../capabilities/manifest.json#L3965) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ovl_sfx_command"` : 414 |
| S190 | [`source.behavior.settings-ovl-sfx-misc-3fda3a`](../capabilities/manifest.json#L3974) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ovl_sfx_misc"` : 416 |
| S191 | [`source.behavior.settings-ovl-sfx-mon-attack-3fda3a`](../capabilities/manifest.json#L3983) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ovl_sfx_mon_attack"` : 418 |
| S192 | [`source.behavior.settings-ovl-sfx-mon-spell-3fda3a`](../capabilities/manifest.json#L3992) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ovl_sfx_mon_spell"` : 420 |
| S193 | [`source.behavior.settings-ovl-sfx-mon-misc-3fda3a`](../capabilities/manifest.json#L4001) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ovl_sfx_mon_misc"` : 422 |
| S194 | [`source.behavior.settings-no-monsterattack-sfx-3fda3a`](../capabilities/manifest.json#L4010) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_monsterattack_sfx"` : 424 |
| S195 | [`source.behavior.settings-positional-audio-3fda3a`](../capabilities/manifest.json#L4019) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"positional_audio"` : 426 |
| S196 | [`source.behavior.settings-no-house-sfx-3fda3a`](../capabilities/manifest.json#L4028) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_house_sfx"` : 428 |
| S197 | [`source.behavior.settings-quiet-house-sfx-3fda3a`](../capabilities/manifest.json#L4037) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"quiet_house_sfx"` : 430 |
| S198 | [`source.behavior.settings-mute-when-idle-3fda3a`](../capabilities/manifest.json#L4046) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"mute_when_idle"` : 432 |
| S199 | [`source.behavior.settings-alert-starvation-3fda3a`](../capabilities/manifest.json#L4055) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"alert_starvation"` : 434 |
| S200 | [`source.behavior.settings-use-color-3fda3a`](../capabilities/manifest.json#L4064) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"use_color"` : 439 |
| S201 | [`source.behavior.settings-other-query-flag-3fda3a`](../capabilities/manifest.json#L4073) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"other_query_flag"` : 441 |
| S202 | [`source.behavior.settings-shuffle-music-3fda3a`](../capabilities/manifest.json#L4082) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"shuffle_music"` : 474 |
| S203 | [`source.behavior.settings-permawalls-shade-3fda3a`](../capabilities/manifest.json#L4091) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"permawalls_shade"` : 476 |
| S204 | [`source.behavior.settings-topline-no-msg-3fda3a`](../capabilities/manifest.json#L4100) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"topline_no_msg"` : 478 |
| S205 | [`source.behavior.settings-targetinfo-msg-3fda3a`](../capabilities/manifest.json#L4109) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"targetinfo_msg"` : 480 |
| S206 | [`source.behavior.settings-live-timeouts-3fda3a`](../capabilities/manifest.json#L4118) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"live_timeouts"` : 482 |
| S207 | [`source.behavior.settings-flash-insane-3fda3a`](../capabilities/manifest.json#L4127) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"flash_insane"` : 484 |
| S208 | [`source.behavior.settings-last-words-3fda3a`](../capabilities/manifest.json#L4136) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"last_words"` : 487 |
| S209 | [`source.behavior.settings-disturb-see-3fda3a`](../capabilities/manifest.json#L4145) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disturb_see"` : 489 |
| S210 | [`source.behavior.settings-diz-unique-3fda3a`](../capabilities/manifest.json#L4154) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"diz_unique"` : 492 |
| S211 | [`source.behavior.settings-diz-death-3fda3a`](../capabilities/manifest.json#L4163) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"diz_death"` : 494 |
| S212 | [`source.behavior.settings-diz-death-any-3fda3a`](../capabilities/manifest.json#L4172) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"diz_death_any"` : 496 |
| S213 | [`source.behavior.settings-diz-first-3fda3a`](../capabilities/manifest.json#L4181) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"diz_first"` : 498 |
| S214 | [`source.behavior.settings-screenshot-format-3fda3a`](../capabilities/manifest.json#L4190) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"screenshot_format"` : 500 |
| S215 | [`source.behavior.settings-palette-animation-3fda3a`](../capabilities/manifest.json#L4199) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"palette_animation"` : 502 |
| S216 | [`source.behavior.settings-play-all-3fda3a`](../capabilities/manifest.json#L4208) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"play_all"` : 504 |
| S217 | [`source.behavior.settings-id-selection-3fda3a`](../capabilities/manifest.json#L4217) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"id_selection"` : 506 |
| S218 | [`source.behavior.settings-hp-bar-3fda3a`](../capabilities/manifest.json#L4226) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"hp_bar"` : 508 |
| S219 | [`source.behavior.settings-mp-bar-3fda3a`](../capabilities/manifest.json#L4235) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"mp_bar"` : 510 |
| S220 | [`source.behavior.settings-st-bar-3fda3a`](../capabilities/manifest.json#L4244) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"st_bar"` : 512 |
| S221 | [`source.behavior.settings-find-ignore-montraps-3fda3a`](../capabilities/manifest.json#L4253) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"find_ignore_montraps"` : 515 |
| S222 | [`source.behavior.settings-quiet-os-3fda3a`](../capabilities/manifest.json#L4262) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"quiet_os"` : 518 |
| S223 | [`source.behavior.settings-disable-lightning-3fda3a`](../capabilities/manifest.json#L4271) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"disable_lightning"` : 520 |
| S224 | [`source.behavior.settings-macros-in-stores-3fda3a`](../capabilities/manifest.json#L4280) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"macros_in_stores"` : 522 |
| S225 | [`source.behavior.settings-item-error-beep-3fda3a`](../capabilities/manifest.json#L4289) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"item_error_beep"` : 524 |
| S226 | [`source.behavior.settings-keep-bottle-3fda3a`](../capabilities/manifest.json#L4298) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"keep_bottle"` : 526 |
| S227 | [`source.behavior.settings-easy-disarm-montraps-3fda3a`](../capabilities/manifest.json#L4307) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"easy_disarm_montraps"` : 529 |
| S228 | [`source.behavior.settings-no-house-magic-3fda3a`](../capabilities/manifest.json#L4316) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_house_magic"` : 531 |
| S229 | [`source.behavior.settings-no-light-fainting-3fda3a`](../capabilities/manifest.json#L4325) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_light_fainting"` : 533 |
| S230 | [`source.behavior.settings-auto-pickup-3fda3a`](../capabilities/manifest.json#L4334) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_pickup"` : 536 |
| S231 | [`source.behavior.settings-auto-destroy-3fda3a`](../capabilities/manifest.json#L4343) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_destroy"` : 538 |
| S232 | [`source.behavior.settings-destroy-all-unmatched-3fda3a`](../capabilities/manifest.json#L4352) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"destroy_all_unmatched"` : 540 |
| S233 | [`source.behavior.settings-mp-huge-bar-3fda3a`](../capabilities/manifest.json#L4361) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"mp_huge_bar"` : 543 |
| S234 | [`source.behavior.settings-sn-huge-bar-3fda3a`](../capabilities/manifest.json#L4370) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"sn_huge_bar"` : 545 |
| S235 | [`source.behavior.settings-hp-huge-bar-3fda3a`](../capabilities/manifest.json#L4379) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"hp_huge_bar"` : 547 |
| S236 | [`source.behavior.settings-clone-to-stdout-3fda3a`](../capabilities/manifest.json#L4388) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"clone_to_stdout"` : 550 |
| S237 | [`source.behavior.settings-clone-to-file-3fda3a`](../capabilities/manifest.json#L4397) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"clone_to_file"` : 552 |
| S238 | [`source.behavior.settings-first-song-3fda3a`](../capabilities/manifest.json#L4406) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"first_song"` : 554 |
| S239 | [`source.behavior.settings-equip-text-colour-3fda3a`](../capabilities/manifest.json#L4415) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"equip_text_colour"` : 556 |
| S240 | [`source.behavior.settings-equip-set-colour-3fda3a`](../capabilities/manifest.json#L4424) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"equip_set_colour"` : 558 |
| S241 | [`source.behavior.settings-colourize-bignum-3fda3a`](../capabilities/manifest.json#L4433) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"colourize_bignum"` : 560 |
| S242 | [`source.behavior.settings-flash-player2-3fda3a`](../capabilities/manifest.json#L4442) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"flash_player2"` : 563 |
| S243 | [`source.behavior.settings-load-form-macros-3fda3a`](../capabilities/manifest.json#L4451) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"load_form_macros"` : 565 |
| S244 | [`source.behavior.settings-auto-inscr-off-3fda3a`](../capabilities/manifest.json#L4460) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_inscr_off"` : 567 |
| S245 | [`source.behavior.settings-ascii-feats-3fda3a`](../capabilities/manifest.json#L4469) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ascii_feats"` : 570 |
| S246 | [`source.behavior.settings-ascii-items-3fda3a`](../capabilities/manifest.json#L4478) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ascii_items"` : 572 |
| S247 | [`source.behavior.settings-ascii-monsters-3fda3a`](../capabilities/manifest.json#L4487) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ascii_monsters"` : 574 |
| S248 | [`source.behavior.settings-ascii-uniques-3fda3a`](../capabilities/manifest.json#L4496) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ascii_uniques"` : 576 |
| S249 | [`source.behavior.settings-no-flicker-3fda3a`](../capabilities/manifest.json#L4505) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no_flicker"` : 579 |
| S250 | [`source.behavior.settings-stun-huge-bar-3fda3a`](../capabilities/manifest.json#L4514) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"stun_huge_bar"` : 581 |
| S251 | [`source.behavior.settings-gfx-autooff-fmsw-3fda3a`](../capabilities/manifest.json#L4523) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"gfx_autooff_fmsw"` : 585 |
| S252 | [`source.behavior.settings-solid-bars-3fda3a`](../capabilities/manifest.json#L4532) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"solid_bars"` : 588 |
| S253 | [`source.behavior.settings-autopickup-chemicals-3fda3a`](../capabilities/manifest.json#L4541) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"autopickup_chemicals"` : 590 |
| S254 | [`source.behavior.settings-add-kind-diz-3fda3a`](../capabilities/manifest.json#L4550) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"add_kind_diz"` : 592 |
| S255 | [`source.behavior.settings-hide-lore-paste-3fda3a`](../capabilities/manifest.json#L4559) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"hide_lore_paste"` : 594 |
| S256 | [`source.behavior.settings-new-retaliator-3fda3a`](../capabilities/manifest.json#L4568) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"new_retaliator"` : 597 |
| S257 | [`source.behavior.settings-sunburn-msg-3fda3a`](../capabilities/manifest.json#L4577) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"sunburn_msg"` : 599 |
| S258 | [`source.behavior.settings-wide-scroll-margin-3fda3a`](../capabilities/manifest.json#L4586) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"wide_scroll_margin"` : 601 |
| S259 | [`source.behavior.settings-topline-first-3fda3a`](../capabilities/manifest.json#L4595) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"topline_first"` : 607 |
| S260 | [`source.behavior.settings-ascii-weather-3fda3a`](../capabilities/manifest.json#L4604) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"ascii_weather"` : 609 |
| S261 | [`source.behavior.settings-no2mask-weather-3fda3a`](../capabilities/manifest.json#L4613) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"no2mask_weather"` : 611 |
| S262 | [`source.behavior.settings-auto-inscr-server-ch-3fda3a`](../capabilities/manifest.json#L4622) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"auto_inscr_server_ch"` : 613 |
| S263 | [`source.behavior.settings-wild-resume-from-any-3fda3a`](../capabilities/manifest.json#L4631) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"wild_resume_from_any"` : 618 |
| S264 | [`source.behavior.settings-tavern-town-resume-3fda3a`](../capabilities/manifest.json#L4640) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"tavern_town_resume"` : 620 |
| S265 | [`source.behavior.settings-st-huge-bar-3fda3a`](../capabilities/manifest.json#L4649) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"st_huge_bar"` : 622 |
| S266 | [`source.behavior.settings-huge-bars-gfx-3fda3a`](../capabilities/manifest.json#L4658) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"huge_bars_gfx"` : 624 |
| S267 | [`source.behavior.settings-autoinsc-debug-3fda3a`](../capabilities/manifest.json#L4667) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"autoinsc_debug"` : 627 |
| S268 | [`source.behavior.settings-autoloot-dunonly-3fda3a`](../capabilities/manifest.json#L4676) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"autoloot_dunonly"` : 629 |
| S269 | [`source.behavior.settings-autoloot-dununown-3fda3a`](../capabilities/manifest.json#L4685) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"autoloot_dununown"` : 631 |
| S270 | [`source.behavior.settings-flash-starvation-3fda3a`](../capabilities/manifest.json#L4694) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"flash_starvation"` : 633 |
| S271 | [`source.behavior.settings-autoswitch-inven-3fda3a`](../capabilities/manifest.json#L4703) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"autoswitch_inven"` : 635 |
| S272 | [`source.behavior.settings-show-newest-3fda3a`](../capabilities/manifest.json#L4712) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"show_newest"` : 637 |
| S273 | [`source.behavior.settings-log-music-3fda3a`](../capabilities/manifest.json#L4721) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"log_music"` : 639 |
| S274 | [`source.behavior.settings-misc-no-flicker-3fda3a`](../capabilities/manifest.json#L4730) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"misc_no_flicker"` : 641 |
| S275 | [`source.behavior.settings-prefer-subinven-3fda3a`](../capabilities/manifest.json#L4739) | `33d3582` [src/client/c-tables.c](../../src/client/c-tables.c), `"prefer_subinven"` : 643 |
| S276 | [`source.behavior.settings-process-pref-file-aux-aux-3df064`](../capabilities/manifest.json#L4748) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `process_pref_file_aux_aux` : 1047 |
| S277 | [`source.behavior.settings-interact-macros-b0b823`](../capabilities/manifest.json#L4757) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `interact_macros` : 1197 |
| S278 | [`source.behavior.settings-process-pref-file-3df064`](../capabilities/manifest.json#L4766) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `process_pref_file` : 1045 |
| S279 | [`source.behavior.settings-chathist-305041`](../capabilities/manifest.json#L4775) | `33d3582` [src/client/c-init.c](../../src/client/c-init.c), `chathist` : 3393 |
| S280 | [`source.behavior.settings-my-fopen-3df064`](../capabilities/manifest.json#L4784) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `my_fopen` : 424 |
| S281 | [`source.behavior.settings-pass-49a7d2`](../capabilities/manifest.json#L4793) | `33d3582` [src/client/client.c](../../src/client/client.c), `pass` : 236 |
| S282 | [`source.behavior.settings-cmd-the-guide-59480e`](../capabilities/manifest.json#L4802) | `33d3582` [src/client/c-cmd.c](../../src/client/c-cmd.c), `cmd_the_guide` : 2310 |
| S283 | [`source.behavior.settings-check-guide-checksums-3df064`](../capabilities/manifest.json#L4811) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `check_guide_checksums` : 3725 |
| S284 | [`source.behavior.settings-copy-to-clipboard-b0b823`](../capabilities/manifest.json#L4829) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `copy_to_clipboard` : 2074 |
| S285 | [`source.behavior.settings-paste-from-clipboard-b0b823`](../capabilities/manifest.json#L4838) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `paste_from_clipboard` : 2385 |
| S286 | [`source.behavior.settings-file-character-3df064`](../capabilities/manifest.json#L4847) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `file_character` : 2297 |
| S287 | [`source.behavior.settings-save-chat-305041`](../capabilities/manifest.json#L4856) | `33d3582` [src/client/c-init.c](../../src/client/c-init.c), `save_chat` : 3349 |
| S288 | [`source.behavior.settings-xhtml-screenshot-3df064`](../capabilities/manifest.json#L4865) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `xhtml_screenshot` : 2172 |
| S289 | [`source.behavior.settings-sdl-openurl-59480e`](../capabilities/manifest.json#L4874) | `33d3582` [src/client/c-cmd.c](../../src/client/c-cmd.c), `SDL_OpenURL` : 6992 |
| S290 | [`source.behavior.settings-string-exec-lua-1e0a09`](../capabilities/manifest.json#L4883) | `33d3582` [src/client/c-script.c](../../src/client/c-script.c), `string_exec_lua` : 533 |
| S291 | [`source.behavior.settings-receive-file-9b42f3`](../capabilities/manifest.json#L4892) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_file` : 351 |
| S292 | [`source.behavior.settings-term-pict-sdl3-37ef28`](../capabilities/manifest.json#L4901) | `33d3582` [src/client/main-sdl3.c](../../src/client/main-sdl3.c), `Term_pict_sdl3` : 1947 |
| S293 | [`source.behavior.settings-do-weather-9f2563`](../capabilities/manifest.json#L4919) | `33d3582` [src/client/c-xtra1.c](../../src/client/c-xtra1.c), `do_weather` : 5457 |
| S294 | [`source.behavior.settings-term-force-font-37ef28`](../capabilities/manifest.json#L4928) | `33d3582` [src/client/main-sdl3.c](../../src/client/main-sdl3.c), `term_force_font` : 1065 |
| S295 | [`source.behavior.settings-sound-a7cf5d`](../capabilities/manifest.json#L4937) | `33d3582` [src/client/snd-sdl3.c](../../src/client/snd-sdl3.c), `sound` : 3 |
| S296 | [`source.behavior.settings-interact-audio-b0b823`](../capabilities/manifest.json#L4946) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `interact_audio` : 17376 |
| S297 | [`source.behavior.settings-sdl3-ed859d`](../capabilities/manifest.json#L4955) | `33d3582` [src/makefile.sdl3](../../src/makefile.sdl3), `SDL3` : 5 |
| S298 | [`source.behavior.settings-cmd-message-59480e`](../capabilities/manifest.json#L4964) | `33d3582` [src/client/c-cmd.c](../../src/client/c-cmd.c), `cmd_message` : 412 |
| S299 | [`source.behavior.settings-term-data-init-37ef28`](../capabilities/manifest.json#L4973) | `33d3582` [src/client/main-sdl3.c](../../src/client/main-sdl3.c), `term_data_init` : 1946 |
| S300 | [`source.behavior.settings-term-wipe-sdl3-37ef28`](../capabilities/manifest.json#L4982) | `33d3582` [src/client/main-sdl3.c](../../src/client/main-sdl3.c), `Term_wipe_sdl3` : 1730 |
| S301 | [`source.behavior.settings-do-cmd-options-colourblindness-b0b823`](../capabilities/manifest.json#L4991) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `do_cmd_options_colourblindness` : 16556 |
| S302 | [`source.behavior.settings-receive-guide-9b42f3`](../capabilities/manifest.json#L5000) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_Guide` : 455 |
| S303 | [`source.behavior.settings-receive-message-9b42f3`](../capabilities/manifest.json#L5009) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_message` : 366 |
| S304 | [`source.behavior.settings-clone-to-stdout-b0b823`](../capabilities/manifest.json#L5018) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `clone_to_stdout` : 4862 |
| S305 | [`source.behavior.settings-clone-to-file-b0b823`](../capabilities/manifest.json#L5027) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `clone_to_file` : 4892 |
| S306 | [`source.behavior.settings-inscriptions-3df064`](../capabilities/manifest.json#L5036) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `inscriptions` : 3005 |
| S307 | [`source.behavior.settings-load-auto-inscriptions-305041`](../capabilities/manifest.json#L5045) | `33d3582` [src/client/c-init.c](../../src/client/c-init.c), `load_auto_inscriptions` : 383 |
| S308 | [`source.behavior.settings-save-birth-file-3df064`](../capabilities/manifest.json#L5054) | `33d3582` [src/client/c-files.c](../../src/client/c-files.c), `save_birth_file` : 3478 |
| S309 | [`source.behavior.settings-bookmarks-tmp-305041`](../capabilities/manifest.json#L5063) | `33d3582` [src/client/c-init.c](../../src/client/c-init.c), `bookmarks.tmp` : 3419 |
| S310 | [`source.behavior.settings-options-dump-b0b823`](../capabilities/manifest.json#L5072) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `options_dump` : 15299 |
| S311 | [`source.behavior.settings-write-mangrc-49a7d2`](../capabilities/manifest.json#L5081) | `33d3582` [src/client/client.c](../../src/client/client.c), `write_mangrc` : 187 |
| S312 | [`source.behavior.settings-reinit-guide-59480e`](../capabilities/manifest.json#L5090) | `33d3582` [src/client/c-cmd.c](../../src/client/c-cmd.c), `/reinit_guide` : 8547 |
| S313 | [`source.behavior.settings-sdl3-image-37ef28`](../capabilities/manifest.json#L5153) | `33d3582` [src/client/main-sdl3.c](../../src/client/main-sdl3.c), `SDL3_image` : 8 |
| S314 | [`source.behavior.settings-shot-59480e`](../capabilities/manifest.json#L5162) | `33d3582` [src/client/c-cmd.c](../../src/client/c-cmd.c), `/shot` : 8121 |
| S315 | [`source.behavior.settings-receive-chardump-9b42f3`](../capabilities/manifest.json#L5171) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_chardump` : 408 |
| S316 | [`source.protocol.settings-receive-file-9b42f3`](../capabilities/manifest.json#L5198) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_file` : 351 |
| S317 | [`source.protocol.settings-send-file-data-9b42f3`](../capabilities/manifest.json#L5207) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Send_file_data` : 679 |
| S318 | [`source.protocol.settings-receive-sound-9b42f3`](../capabilities/manifest.json#L5216) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sound` : 391 |
| S319 | [`source.protocol.settings-receive-music-9b42f3`](../capabilities/manifest.json#L5225) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_music` : 426 |
| S320 | [`source.protocol.settings-receive-weather-9b42f3`](../capabilities/manifest.json#L5234) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_weather` : 422 |
| S321 | [`source.protocol.settings-receive-guide-9b42f3`](../capabilities/manifest.json#L5243) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_Guide` : 455 |
| S322 | [`source.protocol.settings-receive-chardump-9b42f3`](../capabilities/manifest.json#L5252) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_chardump` : 408 |
| S323 | [`source.protocol.settings-receive-char-info-9b42f3`](../capabilities/manifest.json#L5261) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_char_info` : 358 |
| S324 | [`source.protocol.settings-send-font-9b42f3`](../capabilities/manifest.json#L5270) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Send_font` : 9204 |
| S325 | [`source.protocol.settings-send-options-9b42f3`](../capabilities/manifest.json#L5279) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Send_options` : 8051 |
| S326 | [`source.behavior.settings-extract-url-b0b823`](../capabilities/manifest.json#L5288) | `33d3582` [src/client/c-util.c](../../src/client/c-util.c), `extract_url` : 47 |
| S327 | [`source.behavior.settings-receive-music-vol-9b42f3`](../capabilities/manifest.json#L5297) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_music_vol` : 459 |
| S328 | [`source.protocol.settings-receive-music-vol-9b42f3`](../capabilities/manifest.json#L5306) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_music_vol` : 459 |
| S329 | [`source.behavior.settings-receive-sfx-ambient-9b42f3`](../capabilities/manifest.json#L5315) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sfx_ambient` : 436 |
| S330 | [`source.protocol.settings-receive-sfx-ambient-9b42f3`](../capabilities/manifest.json#L5324) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sfx_ambient` : 436 |
| S331 | [`source.behavior.settings-receive-sfx-volume-9b42f3`](../capabilities/manifest.json#L5333) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sfx_volume` : 435 |
| S332 | [`source.protocol.settings-receive-sfx-volume-9b42f3`](../capabilities/manifest.json#L5342) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sfx_volume` : 435 |
| S333 | [`source.behavior.settings-send-version-9b42f3`](../capabilities/manifest.json#L5351) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Send_version` : 7502 |
| S334 | [`source.protocol.settings-send-version-9b42f3`](../capabilities/manifest.json#L5360) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Send_version` : 7502 |
| S335 | [`source.protocol.settings-send-audio-9b42f3`](../capabilities/manifest.json#L5369) | `33d3582` [src/client/nclient.c](../../src/client/nclient.c), `Send_audio` : 9190 |
| S336 | [`source.protocol.settings-receive-version-1e8778`](../capabilities/manifest.json#L5378) | `33d3582` [src/server/nserver.c](../../src/server/nserver.c), `Receive_version` : 448 |
| S337 | [`source.protocol.settings-receive-audio-1e8778`](../capabilities/manifest.json#L5387) | `33d3582` [src/server/nserver.c](../../src/server/nserver.c), `Receive_audio` : 442 |
| S338 | [`source.protocol.settings-receive-font-1e8778`](../capabilities/manifest.json#L5396) | `33d3582` [src/server/nserver.c](../../src/server/nserver.c), `Receive_font` : 449 |
| S339 | [`source.reconciliation.status.read-stats`](../capabilities/manifest.json#L5405) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_stat` : 353 |
| S340 | [`source.reconciliation.status.read-armour`](../capabilities/manifest.json#L5414) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_ac` : 355 |
| S341 | [`source.reconciliation.status.read-demographics`](../capabilities/manifest.json#L5423) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_various` : 359 |
| S342 | [`source.reconciliation.status.read-combat-bonuses`](../capabilities/manifest.json#L5432) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_plusses` : 360 |
| S343 | [`source.reconciliation.status.read-experience`](../capabilities/manifest.json#L5441) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_experience` : 361 |
| S344 | [`source.reconciliation.status.read-currency`](../capabilities/manifest.json#L5450) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_gold` : 362 |
| S345 | [`source.reconciliation.status.read-mana`](../capabilities/manifest.json#L5459) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_mp` : 363 |
| S346 | [`source.reconciliation.status.read-confusion`](../capabilities/manifest.json#L5468) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_confused` : 370 |
| S347 | [`source.reconciliation.status.read-poison`](../capabilities/manifest.json#L5477) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_poison` : 371 |
| S348 | [`source.reconciliation.status.read-study`](../capabilities/manifest.json#L5486) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_study` : 372 |
| S349 | [`source.reconciliation.status.read-blows`](../capabilities/manifest.json#L5495) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_bpr_wraith_prob` : 373 |
| S350 | [`source.reconciliation.status.read-food`](../capabilities/manifest.json#L5504) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_food` : 374 |
| S351 | [`source.reconciliation.status.read-fear`](../capabilities/manifest.json#L5513) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_fear` : 375 |
| S352 | [`source.reconciliation.status.read-speed`](../capabilities/manifest.json#L5522) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_speed` : 376 |
| S353 | [`source.reconciliation.status.read-cut`](../capabilities/manifest.json#L5531) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_cut` : 377 |
| S354 | [`source.reconciliation.status.read-blind-hallucination`](../capabilities/manifest.json#L5540) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_blind_hallu` : 378 |
| S355 | [`source.reconciliation.status.read-stun`](../capabilities/manifest.json#L5549) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_stun` : 379 |
| S356 | [`source.reconciliation.status.read-monster-health`](../capabilities/manifest.json#L5558) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_monster_health` : 402 |
| S357 | [`source.reconciliation.status.read-sanity`](../capabilities/manifest.json#L5567) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sanity` : 403 |
| S358 | [`source.reconciliation.status.read-afk`](../capabilities/manifest.json#L5576) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_AFK` : 413 |
| S359 | [`source.reconciliation.status.read-encumbrance`](../capabilities/manifest.json#L5585) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_encumberment` : 414 |
| S360 | [`source.reconciliation.status.read-stamina`](../capabilities/manifest.json#L5594) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_stamina` : 417 |
| S361 | [`source.reconciliation.status.read-extra`](../capabilities/manifest.json#L5603) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_extra_status` : 419 |
| S362 | [`source.reconciliation.status.read-martyr`](../capabilities/manifest.json#L5612) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_martyr` : 450 |
| S363 | [`source.reconciliation.status.read-idle`](../capabilities/manifest.json#L5621) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_idle` : 452 |
| S364 | [`source.reconciliation.status.read-indicators`](../capabilities/manifest.json#L5630) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_indicators` : 456 |
| S365 | [`source.reconciliation.network.keepalive`](../capabilities/manifest.json#L5639) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_keepalive` : 415 |
| S366 | [`source.reconciliation.network.ping-echo`](../capabilities/manifest.json#L5648) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_ping` : 416 |
| S367 | [`source.reconciliation.network.unknown-packet`](../capabilities/manifest.json#L5657) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Net_packet` : 1834 |
| S368 | [`source.reconciliation.network.partial-packet`](../capabilities/manifest.json#L5666) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Net_packet` : 1834 |
| S369 | [`source.reconciliation.network.malformed-packet`](../capabilities/manifest.json#L5675) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Net_packet` : 1834 |
| S370 | [`source.reconciliation.network.keypress-stub`](../capabilities/manifest.json#L5684) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_keypress` : 433 |
| S371 | [`source.reconciliation.network.server-flags`](../capabilities/manifest.json#L5693) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sflags` : 469 |
| S372 | [`source.reconciliation.input.physical-keys`](../capabilities/manifest.json#L5702) | `a3d99ee` [src/client/main-sdl3.c](../../src/client/main-sdl3.c), `react_keypress` : 1173 |
| S373 | [`source.reconciliation.input.keymap`](../capabilities/manifest.json#L5711) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `request_command` : 3583 |
| S374 | [`source.reconciliation.input.macro-match`](../capabilities/manifest.json#L5720) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `inkey_aux` : 916 |
| S375 | [`source.reconciliation.input.macro-wait`](../capabilities/manifest.json#L5729) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `sync_sleep` : 417 |
| S376 | [`source.reconciliation.input.macro-xwait`](../capabilities/manifest.json#L5738) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `sync_xsleep` : 568 |
| S377 | [`source.reconciliation.input.prompt-navigation`](../capabilities/manifest.json#L5747) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `inkey_combo` : 855 |
| S378 | [`source.reconciliation.input.text-edit`](../capabilities/manifest.json#L5756) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `askfor_aux` : 1649 |
| S379 | [`source.reconciliation.input.confirm`](../capabilities/manifest.json#L5765) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `get_check3` : 3816 |
| S380 | [`source.reconciliation.input.quantity`](../capabilities/manifest.json#L5774) | `a3d99ee` [src/client/c-util.c](../../src/client/c-util.c), `c_get_quantity` : 3250 |
| S381 | [`source.reconciliation.alerts.hp-warning`](../capabilities/manifest.json#L5783) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_hp` : 354 |
| S382 | [`source.reconciliation.alerts.mana-warning`](../capabilities/manifest.json#L5792) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_mp` : 363 |
| S383 | [`source.reconciliation.alerts.sanity-warning`](../capabilities/manifest.json#L5801) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_sanity` : 403 |
| S384 | [`source.reconciliation.alerts.page`](../capabilities/manifest.json#L5810) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_beep` : 411 |
| S385 | [`source.reconciliation.alerts.warning-beep`](../capabilities/manifest.json#L5819) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_warning_beep` : 412 |
| S386 | [`source.reconciliation.network.pause`](../capabilities/manifest.json#L5828) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_pause` : 401 |
| S387 | [`source.reconciliation.network.flush`](../capabilities/manifest.json#L5837) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_flush` : 384 |
| S388 | [`source.reconciliation.network.confirm`](../capabilities/manifest.json#L5846) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_confirm` : 432 |
| S389 | [`source.reconciliation.network.end-marker`](../capabilities/manifest.json#L5855) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_end` : 349 |
| S390 | [`source.reconciliation.information.live-players`](../capabilities/manifest.json#L5864) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_playerlist` : 457 |
| S391 | [`source.reconciliation.information.unique-records`](../capabilities/manifest.json#L5873) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_unique_monster` : 421 |
| S392 | [`source.reconciliation.session.redraw`](../capabilities/manifest.json#L5882) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Send_redraw` : 1865 |
| S393 | [`source.reconciliation.input.raw-key`](../capabilities/manifest.json#L5891) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Send_raw_key` : 8107 |
| S394 | [`source.reconciliation.network.keepalive-send`](../capabilities/manifest.json#L5900) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Send_keepalive` : 7592 |
| S395 | [`source.reconciliation.status.read-live-hp`](../capabilities/manifest.json#L5909) | `a3d99ee` [src/client/nclient.c](../../src/client/nclient.c), `Receive_hp` : 354 |

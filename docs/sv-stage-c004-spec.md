# C004 — загрузка preferences в SV

Статус: **accepted**, 2026-11-09; C004 и задачи 001–004 завершены по решению пользователя. Подготовлен по запросу пользователя на анализ
агентами и план реализации; core реализован следующим запросом пользователя.
[Linux evidence](sv-c004-evidence.md), [техническая сверка](sv-stage-c004-acceptance.md).
Оставшиеся интеграции и проверки перенесены в самостоятельные планы следующих этапов.
[Задачи](tasks/c004/README.md), [source contract](tasks/c004/source-contract.md),
[exact canonical rows](tasks/c004/coverage.json).

## Результат и нормативная база

Один production-путь SV загружает PRF, применяет данные и разрешённые load-time
эффекты, разрешает includes с provenance и диагностикой, выполняет startup
слои в исходном порядке. Ordinary preference loading не превращается в
data-only legacy import. C004 содержит ровно три ID и девять obligations:

| Outcome | Результат | Ticket owner |
|---|---|---|
| `capability.preferences.load` | Обычная загрузка PRF: includes, options, mappings, macro records, permitted actions/messages | SV-C004-001 |
| `capability.preferences.include-failure` | Missing/invalid/recursive includes, продолжение и понятный результат | SV-C004-002 |
| `capability.preferences.bootstrap` | Defaults и собственные/shared слои до первой отправки options | SV-C004-003 |

Основа: [AGENTS](../AGENTS.md), [CONTEXT](../CONTEXT.md),
[settings policy, load order](capabilities/settings-policy.md#preference-load-order-and-boundaries),
[архитектура](sv-architecture.md), ADR
[0001](adr/0001-separate-session-interactions-and-ui.md),
[0002](adr/0002-main-thread-state-ownership.md),
[0003](adr/0003-explicit-module-connections.md),
[0005](adr/0005-session-scoped-work.md),
[0006](adr/0006-errors-and-overload.md).
Полная policy последовательность уточняет сокращённое описание bootstrap в
ledger; approved ownership/no-autosave правила имеют приоритет над legacy writes.
C002/C003 приняты пользователем; их текущий production core используется как база,
canonical pending статусы не превращаются в prerequisite PASS.

## Минимальная реализация

Развить ограниченный loader C002 в один SV-local preference owner, предлагаемые
файлы `src/client/sv/settings/preferences.[ch]`. Сохранить public wrappers
`settings/options.[ch]`, таблицу слотов, defaults и exact aliases. Не держать два
разных PRF parser для startup и explicit load. Не подключать `c-files.c`,
`c-util.c` или `nclient.c` целиком и не переделывать legacy/common.

Preference owner владеет include stack, текущим decoded A-action, macro/mapping
данными, source records и одноразовыми effects с точными payload bytes. Файловые
операции используют C003 `SvFilePath`/results. Logical source role и фактический
resolved owner хранятся раздельно; `.prf` не означает shared macro owner.
Публичный load различает optional automatic layer, explicit selected source и
nested include. Внешний explicit-load UI не создаётся ради вызова API.

Assembly подключается в `login/native-run.c`; character overlay — в существующей
точке `login/flow.c` до первого options packet. Данные приложения и попытки входа
имеют разных владельцев; при retry используется свежий character overlay.
Renderer не читает PRF и не исполняет effects. Новые интерфейсы и точные названия
уточняются по минимальному diff, без общего framework.

## Обязательная семантика

- `%` выполняется в позиции строки, до фильтра macro-only processing. Action A
  сохраняется через includes и последовательные loads одного preference state.
- `X/Y` используют исходные option slots и exact alias branches; shared macro
  options намеренно меняют текущие значения, но не owner будущего Save.
- Mapping/macro records сохраняют исходные gates и opaque bytes. Комментарии и
  ветки `#if 0` не становятся новыми возможностями. `W` не создаёт Terms.
- `#:` отличается от комментария: colour escaping и chat route сохраняются.
- `!/?` декодируются по baseline, включая marker 29 и prepend queue semantics.
  Для двух строк action1, action2 очередь начинается action2,29,action1,29.
  `?` пропускается в bodymonster context; `!` — нет. Macro-only processing
  отсекает обе и `#`, но не обход includes. Общий новый запрет startup actions
  или gate `in_game` не добавляется.
- Ошибка строки/include диагностируется, дальнейшие строки обрабатываются;
  уже применённые effects не откатываются. Structured result различает clean,
  partial с диагностикой, unavailable и fatal. Cycle guard обязателен:
  baseline сам его не имеет, нынешний C002 уже имеет ограничение глубины/stack.
- Own CFG/OPT не переписываются при загрузке/выходе. Legacy `.tmp`, `#ERROR:`
  source rewrite и autosave obsolete options не переносятся; compatible records
  сохраняются в памяти. Не добавлять blanket macro autoresave.

Подробные record families, gates и source references — в source contract.
Известный дефект `S`-record уже [выделен отдельно](sv-improvements.md#correct-legacy-s-keymap-record-field-parsing):
не исправлять его незаметно в C004 и не менять legacy. В C004 сохранить запись
и baseline compatibility disposition; effective keymap consumer принадлежит C005.

## Startup и ownership

`U` — shared SDL3 root, `S=U/sv`, `B` — selected game library.
Нормативный порядок:

1. Compiled defaults и shipped preference/mapping bootstrap.
2. Собственные global options.
3. Applicable system preferences.
4. Preferences выбранных effective map font/graphics.
5. Собственные system options.
6. Собственные character options в ранней pre-send точке.
7. Baseline macro layers по мере появления их orchestration в C052.

Shipped `pref.prf` загружается из B. Его option entrypoints направляются в S;
только предусмотренный shipped fallback читает B/user/options.prf. Не читать
автоматически legacy U/user/global.opt/options.prf. Свои `global.opt`,
`global-sv.opt`, `<character>.opt` сохраняют позиции; class OPT autoload не добавлять.
Explicit macro/resource sources используют U overlay с B fallback. Отсутствующий
overlay и повреждённый/недоступный overlay не смешивать.

Сейчас B/user/pref-sv.prf отсутствует, тогда как pref-sdl3.prf содержит baseline
system macro defaults. Предлагается SV-local выбор совместимого shipped
pref-sdl3.prf как fallback при отсутствии pref-sv.prf, с сохранением существующего
явного pref-sv override. В 003 зафиксировать exact source selection и фактические
trigger bytes; глобальный ANGBAND_SYS не менять. Это предложение реализации,
не утверждение, что raw SDL3 triggers уже исполняются native input SV.

Текущий text font не является effective map font. Не привязывать map PRF reload
к `ui/font.c` только потому, что он уже открывает шрифт. Отсутствующие resource
consumers получают настоящий hook и pending evidence, а не выдуманную identity.

## Границы и приёмка

C004 предоставляет production parser/state/effects и подключает существующих
consumers. Данные без consumer не считаются работающим gameplay. Решением
пользователя от 2026-11-09 C004 завершён и принят сейчас; прежнее условие ожидания
late flow evidence отменено. Конкретные доработки и проверки внесены прямо в
[планы этапов-владельцев](tasks/c004/README.md#самостоятельные-обязательства-следующих-этапов).
Они обязательны при реализации этих этапов и не переоткрывают C004 автоматически.
Повторное подтверждение переноса или завершения этапа не требуется.

| Поздний владелец | Что должен завершить на том же production-пути |
|---|---|
| C005 | Effective keymap application, включая disposition S; parser не принимает input outcomes |
| C006/C007 | Effective map font/graphics preferences, mapping/reset/reload и визуальный consumer |
| C011/C014 | Session preparation/handoff и lifecycle доставки session-derived effects |
| C016 | Полный message/chat route для load-time messages, без повторов при recall/redraw |
| C044/C054 | Settings/macros UI callers explicit load/save; сохранность mixed records |
| C051 | Parse/stage без исполнения; missing/cyclic/out-of-source include пропускает linked import group |
| C052 | Global/race/trait/class/character/form precedence, later character/form reloads и body contexts |
| C053 и этап конкретного action | Runtime macro bytes, waits/cancel и фактическое выполнение load-time commands |

Canonical allocation и девять exact obligations C004 сохраняются; работа по
оставшейся интеграции и её проверке передана следующим этапам. Source contract
сохраняет трассировку core и фактически неполученного late evidence. Для import all-or-skip не
применять rollback к ordinary loading. Пустая очередь, тестовый callback или
single-byte binding table не доказывают исполнение arbitrary PRF action.

## Минимум проверок

Только [Linux](agents/platform-validation.md); Windows/MinGW/Wine deferred.
Нужен необходимый минимум core cases через public production SV path: mixed
records/includes и U/S/B provenance; exact option/alias/mapping/macro bytes;
prepend effects/gates; missing/malformed/cycle и usable continuation;
character-before-send и retry lifetime; byte-identical sources после load/exit.
Переиспользовать существующий settings-options entrypoint и относящиеся login
packet checks, новый компактный core файл допустим только при необходимости.
Не навязывать число executables, новый harness, fault framework, полную visual
matrix, packaging rerun или реальные аккаунты. Расширение проверок согласовать.

Во время реализации создать `docs/sv-c004-evidence.md`: revision/build, команды,
expected/actual bytes/paths, real callers, все obligation dispositions и remaining
flows. Acceptance документ создаётся по фактическому результату. Этот план и
coverage snapshot не являются runtime evidence и не меняют canonical registry.

## Текущий результат — 2026-11-09

Current core и startup wiring реализованы по задачам 001–003, отчёт 004 готов.
Linux build, focused ASan/UBSan, existing first-session four-mode flow и native
startup/quit прошли. Выбор shipped pref-sdl3 fallback реализован; ресурсный hook
пока без map consumer. C004 принят пользователем; недостающие flows и проверки
выполняются в следующих этапах согласно [решению](sv-stage-c004-acceptance.md)
и [фактическому evidence](sv-c004-evidence.md). Source encoded-prefix1023 сохранён с
явной LIMIT/partial диагностикой и полным raw retention. Canonical JSON не менялся.

Планы C045–C049 дополнительно закрепляют доставку immediate X/Y occurrences
своим option consumers, включая unchanged values и alias effect order. Их
проверки не добавляются назад в C004. Поздние этапы этим решением не принимаются;
возврат evidence в C004 служит трассировке, не gate его завершения.

C050 и C055 закрепляют реальных alert/audio consumers, появляющихся после
option stages C045/C049. Их coupled проверки выполняются поздними владельцами,
не становятся обратным prerequisite ранних option stages или принятого C004.

# C004 — Linux core evidence

Дата: 2026-11-09. **C004 и задачи 001–004 accepted по решению пользователя;
оставшиеся доработки и проверки перенесены в следующие этапы.**
[Spec](sv-stage-c004-spec.md), [задачи](tasks/c004/README.md),
[техническая сверка](sv-stage-c004-acceptance.md).
C004 завершён отдельным решением пользователя, без ожидания late flows.
Фактические результаты ниже не переписаны как полный runtime PASS. Canonical JSON
не изменён; три exact rows и девять obligations сохранены в
[planning snapshot](tasks/c004/coverage.json).

База HEAD `dc14c4ad5d5a9aa202051de929cbc9b601ddadb7` плюс существовавшие до
этой задачи незакоммиченные C002/C003 и другие правки. Начальные hashes/status
сохранены в [.scratch/sv-c004](../.scratch/sv-c004/before-sha256.json),
финальные source/binary hashes — [after-sha256.json](../.scratch/sv-c004/after-sha256.json).
Linux build ID: `linux-ff24240d10a53db590cb`.

## Реализованный production путь

- `settings/preferences.[ch]` — SV-local owner PRF records, include stack,
  logical/physical provenance, mapping/macro definitions, текущей A-action,
  diagnostics, option/message/action intents и pending action bytes.
- `settings/options.[ch]` сохраняет C002 defaults, slot identities и exact aliases;
  wrappers вызывают тот же interpreter. Immediate option occurrences сохраняются
  даже при прежнем значении, включая порядок one-to-many alias effects.
- `login/native-run.c` создаёт global preference owner и вызывает настоящий
  bootstrap. `login/flow.c` создаёт свежий character overlay до первого options
  packet. `big_map` остаётся проекцией SV layout.
- Global effects не копируются в попытку. Character effects несут generation;
  teardown/retry уничтожает их вместе с attempt owner. Flow уничтожается до
  global owner. Данные не передаются в credential text fields.
- Startup: B pref.prf/options fallback, собственные S options/global/system,
  U/B pref-sv override либо shipped B pref-sdl3 fallback. Resource hook стоит
  перед own system OPT; настоящего map consumer пока нет.
- Ordinary load продолжает чтение после bad records/includes. Результат
  clean/partial/unavailable/fatal и contextual diagnostics не скрывают ошибки.
  Source writes/`#ERROR:`/OPT autosave не выполняются.

Изменения production ограничены SV и добавлением object в `makefile.sv`.
Legacy/common не менялись. В 13 существующих Python compile wrappers добавлены
линковочные зависимости нового production owner; это не расширение suites.
Focused cases добавлены в существующий `tests/sv/settings-options.c`.

## Проверки

| Проверка | Фактический результат / evidence |
|---|---|
| Linux production build | PASS, `make -f makefile.sv tomenet-sv -j4` из src; [build.log](../.scratch/sv-c004/build.log) |
| Existing options core + PRF cases, ASan/UBSan | PASS; [options.log](../.scratch/sv-c004/options.log), команды ниже |
| Existing first-session managed peer | PASS: success/disconnect/quit/retry, actual character-before-send и retry; [first-session.log](../.scratch/sv-c004/first-session.log) |
| Native startup/quit с S/global.opt, `#:` и `!:` | PASS current assembly: retained 1 message/1 action явно сообщены, payload не выводится, источники unchanged, CFG/OPT autosave отсутствует; [native-startup.log](../.scratch/sv-c004/native-startup.log) |
| Один поставляемый font PRF и fresh clone | PASS public production API: 1326 retained records, 1272 effective mappings, 0 diagnostics; исходный SHA-256 unchanged; [probe/log](../.scratch/sv-c004/resource-probe.log) |
| Wrapper syntax | Python AST syntax PASS для 13 изменённых wrappers; все suites не запускались |
| Независимая сверка agents | Baseline/record semantics и финальный ownership/lifecycle review; blockers в реализованном core не осталось |

Сначала first-session не мог создать listener в sandbox (`socket()` → assertion
`peer.listener >= 0`); [исходный лог](../.scratch/sv-c004/first-session-sandbox.log)
сохранён. Тот же существующий suite с разрешённым loopback прошёл все четыре
режима; код/сценарий для обхода ограничения не менялся.
ASan/UBSan активны. Default LeakSanitizer ограничен ptrace окружения;
использован `ASAN_OPTIONS=detect_leaks=0`, LSan PASS не заявлен.
Полная Linux production сборка имела прежние warnings common-кода; новый SV
код компилируется с `-Wall -Wextra -Werror`.

Минимальный focused reproduction:

```sh
clang -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Isrc/client/sv tests/sv/settings-options.c \
  src/client/sv/settings/options.c src/client/sv/settings/preferences.c \
  src/client/sv/files/files.c src/client/sv/protocol/login.c \
  -o .scratch/sv-c004/settings-options
ASAN_OPTIONS=detect_leaks=0 .scratch/sv-c004/settings-options
python3 tests/sv_first_session_checks.py
```

Последняя команда требует локального loopback listener. Font probe — временная
проверка production loader/clone на одном реальном shipped файле из-за найденного
риска слишком малого retention budget, не новый committed suite и не C006
renderer acceptance. Visual matrix, packaging, реальные аккаунты, performance
framework и Windows/MinGW/Wine не запускались.

## Точные core результаты и ограничения

Mixed U/S/B fixture подтверждает shared→bundled physical provenance и S ownership
вложенного OPT, aliases/inversion, A через include/последующий load, P/H/C/D,
macro-only/body/trigger gates, W без Term, mappings и cross-subset replacement.
R/K/F/U effective state индексируется family/index; glyph subset хранится отдельно,
I/m остаются subset-specific. Реальный consumer визуальных результатов не заявлен.

`\w` декодируется в 96, `\W` — в 30. Для последовательных `!:first`, `?:second`
pending bytes равны `second,29,first,29`. `#:` сохраняет colour escaping и chat
marker 253. Inspection не потребляет effects; `clear_effects` освобождает их.
Ни одно из этих утверждений не означает execution игрового action.

Baseline `c-files.c:1199` обрезает encoded record до byte1023 перед dispatch,
после includes/macro-only filtering. SV применяет тот же effective prefix,
но возвращает LIMIT/partial вместо молчаливого усечения; весь raw record остаётся
retained. Проверенный boundary: 1024-char raw `!:` record даёт 1021 action bytes
и marker29, не выполняемый хвост. Malformed escapes, включая повреждённый cut,
диагностируются безопасно вместо legacy out-of-bounds decode.

Лимиты: include depth16; retained dynamic budget64MiB; diagnostics256;
decoded action buffer1024 с NUL. Бюджет не равен total process RSS: roots/stack,
glyph table, diagnostic storage и transient reader имеют свои bounded allocations.
Старый 32KiB `SvSettingsOptions.records` — только compatibility view; полный
retained источник доступен через `SvPreferences`, и future Save использует его.

Shipped options.prf содержит `N:ascii_weather`/`N:no2mask_weather` на строках36/37.
Они получают baseline format warnings, valid tail применяется; файл не исправляется.
Alias diagnostics тоже являются фактическим startup output, а не startup failure.
Chronicle sources в offline smoke unavailable; это не проверка Chronicle/network.

## Все девять obligations

Префикс каждого имени — `obligation.preferences.`. Полные canonical descriptions,
conditions и prerequisites не сокращены в snapshot.

| Obligation | Current core evidence | Remaining evidence |
|---|---|---|
| `load.result` | Mixed production load, includes/options/mappings/macros/effect bytes | Реальный message recipient, action runtime, map consumers, explicit-load UI |
| `load.failure-lifecycle` | Partial/unavailable/cycle, bounded data, source unchanged; ASan/UBSan и current startup/attempt cleanup | Реальные delivery/cancel/relog для поздних callers |
| `load.allocation-checks` | Warnings, aliases и same-value occurrences, gates и точная pending queue | Отдельный actual C/D action caller regression, не primitive load |
| `bootstrap.result` | Current native bootstrap и character pre-send, S isolation/system fallback | Effective map font/graphics stage и полная C052 sequence |
| `bootstrap.failure-lifecycle` | Current retry/quit/disconnect и свежий character state; no autosave | Late handoff/resource enabled failures |
| `bootstrap.allocation-checks` | Различимые S/U/B layer values, versioned option bytes, retry | Macro precedence/reloads и реальные command results |
| `include-failure.result` | Missing/invalid/recursive normal includes, valid tail | C051 linked import group/no-execution |
| `include-failure.failure-lifecycle` | Resolved owner, diagnostics и cleanup; current read operation | C051 cancel/commit/group failures, failures настоящих writers |
| `include-failure.allocation-checks` | Normal include cases и unchanged source | Migration executes nothing; actual action regressions |

Для read-only loader write/replace injection неприменим именно к операции;
это не снимает будущие writer obligations и не создаёт новый Save framework.
Concurrent source read не обещает atomic filesystem snapshot; source audit не
выдаётся за проверку всех concurrent-edit branches. Windows evidence deferred
по текущему правилу платформ.

## Pending consumers и итог

`#:` и `!/?` производятся production owner, но текущий native milestone не имеет
полного message/game-action recipient: assembly сохраняет их и явно сообщает
pending counts, затем отменяет при close. Результат «сообщение показано / action
исполнен» **не реализован и не подтверждён runtime evidence**. Бесконечно ожидающая очередь или успешный
тестовый callback не объявлены работающим пользовательским flow.

C005 — effective keymaps/S disposition; C006/C007 — map resources и rendering;
C011/C014 — session preparation/handoff; C016 — message route;
C044/C054 — actual explicit-load/save UI; C051 — data-only linked import;
C052 — full macro precedence и later reloads; C053/action stages — gameplay
execution/completion/waits/cancel. Allocation C004 сохраняется.

001–003 имеют реализованный current core, 004 подготовил этот отчёт. Пользователь
завершил и принял C004/001–004 сейчас и поручил перенести дальнейшие доработки
и проверки в следующие этапы. Их самостоятельные документы перечислены в
[плане передачи работы](tasks/c004/README.md#самостоятельные-обязательства-следующих-этапов).
Они не блокируют и не переоткрывают C004 автоматически; найденные проблемы
исправляются в рамках этапа-владельца. Дополнение evidence C004 разрешено для
трассировки, но не является prerequisite его завершения. Canonical runtime
registration остаётся отдельным workflow. Дополнительные поздние функции
не включались в реализацию ради формального PASS. Обнаруженные legacy encoded
boundary и SV provenance memory opportunities записаны отдельно в
[список улучшений](sv-improvements.md).

## Обновление графа

`.graphify-venv/bin/graphify update .` выполнен успешно; [лог](../.scratch/sv-c004/graphify-update.log).
Code-only AST update, без LLM extraction (0 LLM tokens): 21500 nodes, 56820 edges,
894 communities. HTML создан как aggregated community view из-за размера графа.
Параллельный extraction недоступен в sandbox, выполнен sequential fallback.
Extractor сообщил 78 файлов с partial syntax extraction и 185 файлов без symbols;
это ограничения навигационного графа, не результаты C compiler/test failures.
Community labels частично обновлены по hubs; отдельный semantic re-extraction
Markdown и LLM labeling не выполнялись. Runtime/acceptance conclusions основаны
на исходниках и production checks выше, а не на полноте графа.

## Решение о переносе проверок — 2026-11-09

[Решение пользователя](sv-stage-c004-acceptance.md) заменяет прежний gate ожидания
late consumers. Таблица девяти obligations выше сохраняет фактические границы
evidence; столбец Remaining evidence теперь означает работу в последующих
этапах, а не незавершённость принятого C004. Immediate X/Y consumers C045–C049
также используют occurrence-aware adapter; их реальная реакция проверяется
в соответствующих этапах. Поздние этапы не приняты и не получают runtime PASS
из-за этого переноса. Сейчас менялись только планы/статусы, новые runtime
проверки и изменения production-кода этим решением не выполнялись.

Alert delivery C050 и live audio consumers C055 получили самостоятельные
planning addenda для завершения option-effect связок после C045/C049; это
передача future work, не новые runtime результаты данного отчёта.

## C006 integration — Linux core, 2026-11-09

C006 подключил production map-font owner и resource bootstrap hook.
[Фактические caller/команды/источники и ограничения](sv-c006-evidence.md).
`settings/map-font.c` использует C003 file owners и C004 parser/reset seam;
`login/native-run.c` активирует font roles, `login/flow.c` применяет character
visuals перед PLAY, `protocol/login.c` сериализует actual definitions/identity.
Минимальный map-font check подтверждает U/user custom F:3=(attr9,glyph81),
B/user custom F:3=(attr8,glyph80), затем B system F:5=(attr6,glyph82); invalid
custom directory не публикуется, missing system явно даёт zero overrides.
PLAY bytes содержат эти таблицы, live output — CLIENT_SETUP затем FONT.
Source full refs сохраняются локально; C003 reporting copy policy не менялась.
App PRF effects имеют generation/payload/context, repaint не генерирует их снова;
actual messages/actions delivery остаётся C016/C053. FI-01 core evidence
дополнено; FI-02 active graphics/coupled reload остаётся C007. Это не меняет
прежнюю пользовательскую приёмку C003/C004 и не регистрирует canonical PASS.


## C007 — возврат core integration evidence, 2026-11-09

Production graphics consumer реализован: coupled font/graphics PRF candidate,
actual base/subset/mask/raw preparation, effective reporting и atomic publication.
[Evidence C007](sv-c007-evidence.md) содержит callers/commands, real sources/generations,
PLAY/live FONT boundary observations и отдельную disposition 45 rendering
obligations. Для C003 это FI-02 resource-overlay/report-resource-identity trace;
для C004 — graphics preference/reset/reload trace. Полный late packet-derived
map flow, backend/budget и непроверенные failure branches остаются pending.
Это дополнение не переоткрывает принятую приёмку C003/C004 и не меняет их
canonical allocation/runtime statuses; сама приёмка C007 не выполнялась.


Последующее решение 2026-11-09: [C007 принят пользователем](sv-stage-c007-acceptance.md) в представленных
границах Linux core. Поздние flows и непроверенные branches остаются таковыми;
это не новая регистрация native-runtime PASS C003/C004.

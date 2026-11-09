# C005 — Linux core evidence

Дата: 2026-11-09. C005 core реализован и проверен на Linux; задачи 001–004 выполнены в
согласованных границах. [C005 и задачи 001–004 приняты пользователем](sv-stage-c005-acceptance.md). [Спецификация](sv-stage-c005-spec.md),
[задачи](tasks/c005/README.md), [точные rows](tasks/c005/coverage.json).
Пользовательская приёмка подтверждена; canonical runtime registration остаётся
отдельным workflow.

База HEAD `dc14c4ad5d5a9aa202051de929cbc9b601ddadb7` плюс существовавшее dirty
дерево C002/C003/C004 и соседних задач. Начальный status и hashes затронутых
файлов сохранены в [.scratch/sv-c005](../.scratch/sv-c005/initial-status.txt).
Изменения прежних задач не сбрасывались; canonical JSON не изменяется.

## Production результат

- `input/keymap.[ch]`: все 128 normal/roguelike mappings, command+direction,
  bypass/control states между проходами main loop. Bypass сохраняет последнее
  направление baseline; fresh/reset direction=0.
- `settings/preferences.[ch]`: bounded ordered typed INITIAL/OPTION/S occurrences,
  сохраняющиеся через clone независимо от pending effects. Option occurrence
  выбирает layout, но не вызывает несуществующий в baseline init. Explicit
  consumer rebuild задаёт baseline границу init и стирает S в manual reload.
- `input/quantity.[ch]`: неблокирующий core поверх production text-field,
  caller default/maximum/all policy, k/M/G/fraction, Escape/empty и single take.
  У single backquote baseline отмена; bulk paste сохраняет внутренние bytes.
- `app.[ch]`, `input/native-input.[ch]`: production physical/native input → keymap →
  semantic command либо raw packet; quantity begin/text/edit/take с собственным
  context и восстановлением после nested server key request. Pending command
  хранится однократно до take, чужой/stale generation его не получает.

Дефект S сознательно сохранён: command/direction читаются из первого поля с
baseline direction filtering. Intended syntax не включена. Backslash оставляет
последнее direction. Initial отрицательный default, оставленный видимым, — текст:
`-1` + Enter даёт 0; очищенная строка + Enter при negative predef даёт 2000000000.
Quantity overflow не определён baseline C; SV считает в int64 и ограничивает
результат representable int32 и caller maximum. Это не перенос artifacts UB.
[Legacy improvement](sv-improvements.md#define-safe-legacy-quantity-arithmetic-at-numeric-limits)
остаётся отдельным предложением.

## Проверки

Точные команды: [check-commands.txt](../.scratch/sv-c005/check-commands.txt).

| Проверка | Результат |
|---|---|
| Focused keymap, ASan/UBSan | PASS, [keymap.log](../.scratch/sv-c005/keymap.log): все remaps, directional groups, high-bit lookup, prefixes/no-op, reset, настоящие PRF load/reload/partial, S/init order, clone |
| Existing preference core, ASan/UBSan | PASS, [preferences.log](../.scratch/sv-c005/preferences.log): layers, aliases, mixed PRF/effects/includes, versioned options |
| Focused quantity, ASan/UBSan | PASS, [quantity.log](../.scratch/sv-c005/quantity.log): default/empty/negative/zero, shortcuts/policy, factors/fraction, width, reset, single take, backquote и bulk paste |
| Linux production build | PASS, [build.log](../.scratch/sv-c005/build.log), `linux-ff24240d10a53db590cb` |
| Existing request/app/native core, ASan/UBSan | PASS, [app-checks.log](../.scratch/sv-c005/app-checks.log): 372 existing split/HP/cancel/abort cases и компактный C005 app/native набор |

SV code собирается с `-Wall -Wextra -Werror`; common code сохраняет прежние
prototype warnings. ASan/UBSan активны в core checks, `ASAN_OPTIONS=detect_leaks=0`
из-за LSan/ptrace ограничения окружения; LSan PASS не заявлен. Семь существующих
app compile wrappers получили новые production dependencies; wrapper syntax
проверен Python AST, широкие suites не запускались. Старый отсутствовавший
`input/physical.c` в request wrapper заменён действительным `input/key-encoding.c`.


## Точный app/native результат

Existing `tests/sv/requests.c` вызывает production `sv_native_input` и
`sv_app_physical` с настоящим protocol output. Normal `4` даёт `;`/direction4,
rogue `h` — тот же semantic result после adoption preferences. Известные
baseline команды остаются semantic results; будущий отсутствующий executor не
подменяется raw отправкой. Unhandled ordinary `J` и rogue `X` дают packet
`{0x9c,key}`. Literal `^` ожидает следующий byte; normal `^z` даёт `{0x9c,26}`.
F1 normal binding также проходит настоящий sender. Reserved Esc/CR/hyphen не
дают packet; normal LF становится CR, bypass LF сохраняет raw byte. Space
выдаёт clear-actions command, `]` различает ordinary и master disposition.

Output заполняется до 1024 bytes: следующий двухбайтовый packet возвращает
BACKPRESSURE, не добавляет partial bytes и восстанавливает prefix для явного
retry. После освобождения output повтор даёт один packet. Pending semantic
command также возвращает backpressure без потери уже принятого результата.
Literal Escape завершает pending prefix через native route; backquote проходит
baseline Escape normalization. Resize/focus/network update не повторяют команду.

Quantity: native text `2k` и Enter дают 2000 единожды; nested request возвращается
в QUANTITY context, не меняет draft и отправляет свой отдельный reply packet.
COMMAND/HYBRID F1 bindings не редактируют quantity; NORMAL binding разрешён.
Cancel/backquote дают 0, result-pending не превращает следующий text в gameplay.
Open/close очищают quantity/prefix/command, старое generation отвергается.
`SvNativeInput.last_result` явно отделяет классификацию SDL event от app success;
retaining/retrying обязанность future live owner, а не скрытая очередь адаптера.
[Отдельное улучшение](sv-improvements.md#retain-native-input-events-when-live-dispatch-is-backpressured)
сохраняет этот integration gap диагностических event loops.

## Obligation disposition

Три exact canonical rows и девять obligations сохранены без переименования в
snapshot, source hashes сверяются отдельно. Здесь описывается core evidence,
а не автоматический PASS поздних gameplay callers.

| Obligation ID | Evidence текущего core / обязательный поздний route |
|---|---|
| `obligation.input.keymap.result` | PASS core: keymap/preference vectors и app physical command+direction; конкретные action outcomes остаются своим stages |
| `obligation.input.keymap.lifecycle` | PASS core: prefix retained/reset, app focus/resize/interleaved update и generation teardown. Actual login→session handoff — C011/C014 |
| `obligation.input.keymap.allocation-checks` | PASS core: normal/rogue own branches, typed PRF occurrences и настоящий physical route. Macro-derived runtime route — C053 |
| `obligation.input.quantity.result` | PASS core: production quantity/text-field defaults/empty/all/factors/limits/cancel; native text/edit и app result 2000 из 2k |
| `obligation.input.quantity.lifecycle` | PASS core: reset/single take, app nested-request parent restore, stale generation и native input. Actual session lifecycle — C011/C014 |
| `obligation.input.quantity.allocation-checks` | PASS core собственных primitive branches; реальные item/store/server/audio quantity interpretations и bytes — C022/C027/C017/C055; macro input — C053 |
| `obligation.input.raw-key.result` | PASS core: normal/rogue unhandled keys и physical binding → exact {0x9c,key}; prefix/no-op/known command не отправляются raw |
| `obligation.input.raw-key.lifecycle` | PASS core: output backpressure с prefix rollback/retry, pending prompt/quantity, stale/close/no-repeat; actual session routing — C011/C014 |
| `obligation.input.raw-key.allocation-checks` | PASS core: normal/rogue/physical/no-op separation, space как semantic action, ordinary/admin ] disposition; macro runtime — C053, store raw-key — C027 |

Поздние executors используют тот же production результат, а не новый parser.
Собственный success primitive не принимает их игровые действия. C011/C014,
C017/C022/C027/C055 и C053 выполняют свои combined flows в своих этапах без
обратного gate C005 и без переоткрытия принятого C004. Ни pending queue, ни
source audit не доказывают серверное действие.
Windows/MinGW/Wine, full visual matrix, packaging rerun и реальные аккаунты
не запускались по текущему scope.

## Итог и graph update

Задачи 001–004 выполнены в согласованном core scope и приняты пользователем
вместе с C005: [решение](sv-stage-c005-acceptance.md). Source/binary hashes: [after-sha256.json](../.scratch/sv-c005/after-sha256.json).
Canonical rows/hash проверка, локальные ссылки, syntax семи compile wrappers и
`git diff --check` прошли. Existing unrelated dirty changes сохранены.

Обязательная команда `.graphify-venv/bin/graphify update .` завершилась успешно:
21730 nodes / 57435 edges / 816 communities,
[лог](../.scratch/sv-c005/graphify-update.log). CLI обновил code graph; semantic
extraction изменённых документов этим code-only subcommand не выполняется.
Extractor предупредил о partial syntax extraction 78 файлов и отсутствии symbols
у 187 файлов, а parallel extraction недоступен в sandbox и заменён sequential.
Граф не является доказательством компиляции или correctness; результаты сборки
и production core проверок приведены выше. Граф больше 5000 nodes отображён CLI
агрегированно по communities, без запуска отдельной полной визуализации.

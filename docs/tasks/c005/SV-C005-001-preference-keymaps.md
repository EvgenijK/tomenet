# SV-C005-001 — effective keymap и PRF S compatibility

Статус: **accepted**, 2026-11-09, по решению пользователя.
Labels: enhancement
Owned IDs: `capability.input.keymap` (existing C005 allocation).
Зависимости: принятые C004 `SvPreferences` и текущий production input router;
C011/C014 и C053 подключают поздние routes в своих этапах и не блокируют C005.

## Проблема и production путь

C004 уже сохраняет option values и S compatibility records, но реального
normal/roguelike consumer этих данных ещё нет. `src/client/sv/input/input.[ch]`
имеет ограниченные single-byte/physical bindings и не заменяет baseline keymap.
C004 завершён и принят пользователем 2026-11-09; эта интеграция и её проверки
теперь обязательны в C005. Дефекты найденного consumer исправляются здесь,
C004 не переоткрывается; ссылка на evidence C004 — необязательная трассировка.

Baseline: `src/client/c-util.c:keymap_init` (1907), `request_command` (3583 onward),
`original_commands`/`roguelike_commands`; `src/client/c-files.c:1550` — S record.
Указанные строки — навигация по текущему дереву, перед реализацией пересверить.
Canonical `input.keymap` требует normal/roguelike mapping, raw backslash bypass,
control-prefix input и reserved Escape/CR/hyphen no-ops; остальные unhandled
keys сохраняют отдельный raw dispatch. Полные conditions/obligations из
`docs/capabilities/native-coverage.json` остаются без сокращения.

### Source contract, анализ 2026-11-09

Graphify query `C005 keymap request_command SvInput SvPreferences` нашёл owners,
но вернул усечённый общий граф; контракты ниже сверены по исходникам.

| Production owner | Что переносить / чего сейчас не хватает |
| --- | --- |
| `c-util.c:1729–1932`, `roguelike_commands`, `original_commands`, `keymap_init` | Все 128 entries, identity+direction=0 по умолчанию; normal digits→`;`+direction, `T`→`+`, comma/5→`,`+5, LF/CR→CR. Roguelike включает walk/run/tunnel и все недирекционные remaps из switch; несколько примеров движения не заменяют полный перенос таблицы. |
| `c-util.c:3583–3672`, `request_command`; `get_com:3516` | Обычный lookup использует `cmd & 0x7f`; `^` преобразует следующий byte через `KTRL` до lookup; backslash пропускает lookup, в том числе для `\\^<byte>`. Итоговый command=0 заменяется Escape. Отмена prefix следует реально возвращённому byte `get_com`, а не отдельной придуманной обработке текста. |
| `c-cmd.c:274–506`, `process_command` | Escape/CR/hyphen — no-op; space выполняет clear-actions и не является no-op keymap. Только unhandled command идёт в `cmd_raw_key`; неизвестная для SV, но известная baseline игровая команда не становится raw автоматически. Raw packet contract — отдельная задача C005. |
| `sv/input/input.c:89–164`, `input.h` | `sv_input_physical` уже разрешает physical trigger целиком; `sv_input_accept/key/sync` обслуживают server key request. Здесь ещё нет gameplay command+direction owner; не прогонять текстовые/login/birth/server prompts через gameplay keymap. |
| `sv/app.c:235–244,291–312,58–75` | Сейчас gameplay `sv_app_physical` сразу вызывает `sv_app_raw_key`; это действительный seam подключения command resolver. Lifecycle сбрасывает input router в `release_session`; новый prefix state должен войти в production reset, а не жить в UI/static. |
| `sv/settings/options.c:13`; `preferences.c:353–420,487–505`; `preferences.h` | `rogue_like_commands` — slot 0, default false. Mapping view сворачивает S одного index до последнего значения, option effects сохраняют occurrences, но S occurrences в этой очереди отсутствуют. Одного final mapping view недостаточно для точного межсемейного порядка. |

Порядок init — отдельный контракт, а не «применить все final S поверх defaults»:
`c-init.c:initialize_player_pref_files` загружает character `.opt`, вызывает
`keymap_init`, затем global/race/trait/class/character `.prf`; manual и class PRF
reload в `c-util.c:7413–7438` вызывает `keymap_init` **после** загрузки, стирая
S overrides. Выход из options (`c-util.c:17446`) также делает init; disabled
`cmd_redraw` init (`c-cmd.c:9086`) не является активным поведением. Не вводить
неявный init на каждую option occurrence: хранение значения и caller-owned init
являются разными операциями.

Disposition совместимости: backslash-ветвь
`request_command` записывает только `command_cmd`, оставляя прежний
`command_dir`. Обычный lookup, напротив, всегда заменяет оба значения. Нельзя
объявить direction=0 для bypass: по умолчанию SV сохраняет последнее direction
в явном состоянии gameplay keymap owner в пределах его lifetime; fresh/reset
state имеет direction=0. Изменение этого поведения требует отдельного решения,
а документированное сохранение baseline не требует нового согласования.

Архитектурная граница: локальный SV keymap owner с явным preference/input seam,
неблокирующими prefix states (`idle`, bypass, control, bypass-control) и
наблюдаемым command+direction result. Связи собираются явно, состояние
принадлежит главному потоку, reset отделён от reload. Это следует ADR-0001,
0002, 0003, 0005 и 0006; legacy `Term`/`inkey` loop в SV не подключать.

### Canonical coverage и распределение evidence

`docs/capabilities/native-coverage.json`, entry `capability.input.keymap`:
acceptanceStage C005; prerequisites пусты; все поддерживаемые cited owner
server versions и gameplay builds, с сохранением применимых compile guards.
Canonical platforms linux/windows сохраняются; текущая приёмка по правилу
платформ требует только Linux, Windows deferred.

| Existing obligation | Собственный результат C005 / поздний владелец |
| --- | --- |
| `obligation.input.keymap.result` | Normal/roguelike, bypass/control, reserved no-ops против unhandled raw; exact bytes и применимая caller cancellation. Concrete inventory outcomes сохраняют своего владельца, результат primitive не принимает их автоматически. |
| `obligation.input.keymap.lifecycle` | Production keymap с interleaved update и focus/resize; собственный reset/teardown не оставляет pending prefix и повторных effects. Session handoff/teardown integration — C011/C014. |
| `obligation.input.keymap.allocation-checks` | Все собственные ветви по pinned source, production early и physical route. Macro route явно закреплён за C053; поздние caller tests не заменяют evidence собственного C005 результата. |

Полные descriptions canonical остаются нормативными: эта таблица поясняет
границы текущего core evidence, не удаляет inventory/macro/lifecycle obligations
и не меняет их IDs либо allocation. Pending late routes указывать с владельцем,
без фиктивного PASS и без превращения их в gate C005.

## Реализация

1. Подключить существующий `SvPreferences` к production keymap owner/input router
   явным интерфейсом. Получать `rogue_like_commands` из его option state и
   учитывать option occurrence при повторной загрузке; не читать файлы из UI.
   Использовать обычный loader при PRF reload, второй parser не создавать.
   Сохранить порядок option/S application occurrences внутри load. Если текущих
   final mapping views недостаточно, добавить узкий typed occurrence seam в
   SV preference owner в C005; не восстанавливать исполнение вторым разбором
   raw records и не переоткрывать C004 ради этой интеграции.
2. Реализовать command/direction translation для normal и roguelike layouts,
   backslash bypass и control-prefix semantics. Undefined key по baseline
   остаётся собой с direction=0; no-op и raw packet dispatch различаются.
   Сохранить состояние незавершённого prefix между проходами main loop.
3. Зафиксировать S disposition перед effective application. Текущий baseline
   ошибочно читает command и direction из первого поля (`zz[0]`), C004 сохраняет
   этот compatibility result и полный source record. Default совместимости —
   воспроизвести это поведение. Не подменять его незаметно intended syntax
   `S:key:command:direction`; изменение трактовки оформляется отдельным явным
   решением в C005 по уже существующей записи
   [улучшения](../../sv-improvements.md#correct-legacy-s-keymap-record-field-parsing).
   Legacy не менять. Сохранить baseline load/keymap-init precedence: не применять
   S повторно после каждого init, если caller baseline его там не применяет.
4. Использовать полные retained records/mappings API, не ограниченный 32KiB
   `SvSettingsOptions.records` view. Parser уже применяет encoded prefix1023
   после includes/filtering и сохраняет полный raw record с LIMIT/partial;
   consumer не должен повторно интерпретировать отброшенный хвост.
5. Подключить существующий physical/core input path и его реальные command/
   direction results. Сохранить очистку prefix state и отсутствие повторной
   команды от focus/resize/redraw на production интерфейсе keymap. Предоставить
   тот же интерфейс последующим callers: C011/C014 подключают actual session
   handoff/teardown, C053 — macro-derived route и его проверку. WAIT96, XWAIT30
   и marker29 обрабатывает C053 runtime, keymap не переосмысляет control bytes.
   Эти поздние интеграции не являются gate готовности C005; gameplay cancellation
   остаётся у конкретного caller.

Изменяемые места: преимущественно `src/client/sv/input/` (допустим отдельный
локальный `keymap.[ch]`), узкий typed seam в `settings/preferences.[ch]` и сборка
связей в `app.[ch]`. Не расширять `SvInputBindings` в полный macro runtime и не
встраивать полный `process_command` всех будущих этапов. Контракт результата
должен различать ожидание prefix, no-op и готовую semantic command; политика
позднего unsupported action сохраняет owning stage вместо отправки неправильного
raw packet. При добавлении нового source включить его в текущий SV build target.
До разрешения gameplay prefix проверить generation, pause и активный input
context; server key request остаётся своим route. После decode проверить
итоговую command disposition: reserved bytes не доходят до raw encoder,
готовая mapped command не становится raw лишь из-за отсутствующего позднего
исполнителя. Raw-key задача использует этот production результат вместо повторного разбора
физического byte. Quantity остаётся отдельным input context: текст numeric prompt
не пропускается через gameplay command keymap.

## Минимальная Linux проверка и готовность

Использовать existing input/core entrypoint и настоящий production consumer.
В одном компактном наборе подтвердить normal/roguelike command+direction,
backslash/control prefix, no-op против raw dispatch, PRF reload/option toggle и
различающиеся три S поля с объявленной compatibility трактовкой. Проверить
действительный load/init order и существующий physical/core input path.
Наблюдать production command/direction result, а не сохранённый parser record.
Macro-derived route обязательно проверяется в
[SV-C053-001](../c053/SV-C053-001-preference-runtime.md), после появления runtime.

Для существующего production core caller проверить interleaved update,
focus/resize и reset между prefix и следующим key. C005 доказывает собственный
результат настоящего keymap module; успешный fake callback или stored mapping
этого не подтверждает. Actual session handoff/teardown и его protocol observation
обязательны в C011/C014, macro-derived execution — в C053. Отсутствие этих будущих
callers не является незавершённостью или readiness gate C005. Расширение набора
проверок согласовать; Windows/MinGW/Wine deferred.

Готовность: production keymap потребляет действительные preference values,
S disposition и порядок применения доказаны, невалидный/partial PRF не сообщает
ложный успех, production reset очищает незавершённый prefix. Все три existing
`obligation.input.keymap.*` получают фактическое evidence собственных результатов
и явную disposition поздних routes с владельцами C011/C014/C053 в отчёте C005;
их последующие integration checks не требуют переоткрытия C005.
`input.quantity/raw-key` и конкретные игровые outcomes имеют своих владельцев
в полном плане C005/других этапов и не принимаются этой задачей автоматически.

## Реализованный core и проверки, 2026-11-09

`src/client/sv/input/keymap.[ch]` реализует все normal/roguelike entries,
неблокирующие bypass/control states, mask `0x7f`, zero-command→Escape и
сохранение previous direction при raw bypass. `sv_keymap_init` создаёт свежий
owner/reset; `sv_keymap_rebuild` только перестраивает таблицы, как baseline
`keymap_init`, сохраняя незавершённый prefix и last direction.

В `settings/preferences.[ch]` добавлен retained typed stream INITIAL/OPTION/S,
независимый от pending effects и bounded compatibility records. Он сохраняет
order includes/reloads, исходный layout, обе BOOL occurrence и baseline S
interpretation; clone переносит stream, `clear_effects` его не потребляет.
Применение в `sv_keymap_preferences` идёт один раз по cursor, без повторного
парсинга source records и без неявного rebuild от option records.

Caller contract: fresh owner создаётся `sv_keymap_init`; после bootstrap или
character `.opt` caller применяет preferences и явно делает rebuild. Следующие
automatic PRF loads применяются без rebuild; manual PRF reload завершается
rebuild. Сейчас `sv_preferences_character` загружает только `.opt`; поздняя
автозагрузка character PRFs принадлежит своему этапу и не симулируется здесь.
App wiring и physical result/raw disposition реализуются совместно с задачей
003; этот раздел не заявляет их готовность до общей проверки C005.

Минимальный Linux core `tests/sv/keymap.c` прошёл ASan/UBSan: normal и три группы
roguelike directional mappings и все недирекционные remaps (data-only expected
vectors), normal/rogue digits, high-bit mask, prefix через отсутствие следующего
byte, prefix Escape/zero-command, bypass direction, reset, no-op bytes,
настоящий preference load/reload,
S compatibility, explicit init order, partial diagnostics, clone/clear-effects.
Из-за изменения preference owner повторно пройден существующий
`tests/sv/settings-options.c` под ASan/UBSan. Логи:
[keymap.log](../../../.scratch/sv-c005/keymap.log),
[preferences.log](../../../.scratch/sv-c005/preferences.log).
`ASAN_OPTIONS=detect_leaks=0` соответствует ограничению LSan окружения;
LSan PASS не заявлен. Ни legacy/common, ни граф этим агентом не изменялись.

```sh
clang -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Isrc/client/sv tests/sv/keymap.c src/client/sv/input/keymap.c \
  src/client/sv/settings/options.c src/client/sv/settings/preferences.c \
  src/client/sv/files/files.c -o .scratch/sv-c005/keymap
ASAN_OPTIONS=detect_leaks=0 .scratch/sv-c005/keymap
clang -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Isrc/client/sv tests/sv/settings-options.c src/client/sv/settings/options.c \
  src/client/sv/settings/preferences.c src/client/sv/files/files.c \
  src/client/sv/protocol/login.c -o .scratch/sv-c005/preferences
ASAN_OPTIONS=detect_leaks=0 .scratch/sv-c005/preferences
```

## Итог совместной реализации C005

Production integration и минимальные Linux проверки завершены; подробные
команды, результаты и границы late evidence — в [общем отчёте](../../sv-c005-evidence.md).
Задача выполнена и принята пользователем в составе C005:
[решение](../../sv-stage-c005-acceptance.md). C004 и canonical registry statuses сохраняются.

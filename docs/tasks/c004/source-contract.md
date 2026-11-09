# C004 — source contract и границы evidence

Дата исходной сверки: 2026-11-09. Ниже сохранён source-backed planning;
таблица «уже работает / gap» описывает базу до реализации C004. C004 принят
пользователем 2026-11-09 с переносом remaining work в следующие этапы. Current результат —
[Linux evidence](../../sv-c004-evidence.md), а не исходные gap observations.
Три read-only анализа: baseline PRF; current SV wiring; independent canonical
scope audit. Graphify query выполнены перед поиском исходников. Root query
`preferences bootstrap include settings macro` (слова из graph vocabulary)
дал 634 nodes, budget 1800 показал 52; граф дал навигацию к policy/C002 и
`process_pref_file`, точные утверждения проверены в текущих файлах. Граф не
доказывает новые незакоммиченные C002/C003 callers. При исходной сверке код не менялся.

## Canonical ownership

Snapshot [coverage.json](coverage.json) содержит exact rows без сокращения:

| ID | Canonical prerequisite | Ticket |
|---|---|---|
| `preferences.load` | `files.resource-overlay` | 001 |
| `preferences.bootstrap` | `settings.load`, `settings.defaults`, `settings.aliases`, `preferences.load` | 003 |
| `preferences.include-failure` | `preferences.load` | 002 |

Все ID имеют префикс `capability.`. У каждого ровно три obligations:
`.result`, `.failure-lifecycle`, `.allocation-checks`. Canonical implementation
и evidence pending, принятые C002/C003 используются как factual production база.
Policy override и Linux-only rule не переписывают exact historical rows.

## Проверенные production-пути SV

| Код | Уже работает / gap | План |
|---|---|---|
| `src/client/sv/settings/options.c:199–268` | Defaults, slot identities, exact aliases | Оставить один option apply seam, не дублировать alias algorithm |
| `settings/options.c:271–344` | Include/X/Y loader, stack 16, canonical path/owner escape guard; missing file считается success даже для includes | Единый PRF owner; distinguish purpose/results, сохранить защиты |
| `settings/options.c:288–297` | Shared fallback меняет physical path; logical owner остаётся shared | Отдельно хранить source role и `SvFilePath.owner`, не терять provenance |
| `settings/options.c:309–337` | 4096-byte line buffer, retained 32KiB; shared records ограничены X/Y/%; другие эффекты игнорируются | Не считать полной PRF implementation; определить исходные limits/retention без silent truncation |
| `settings/options.c:351–377` | Defaults → B pref.prf → own options.prf/shipped fallback → own global.opt → pref-sv.prf → own global-sv.opt | Сохранить callers/wrappers, добавить полную record semantics и resource stage hook |
| `login/native-run.c:528` | Реальный global settings startup caller | Создать/освободить preference owner в assembly |
| `login/flow.c:424–446` | Fresh character option overlay до первого options packet; layout projection big_map | Сохранить точку и retry semantics |
| `files/files.h:6–24` | Resolved path/owner, missing/invalid/IO и owner-aware open/stat | Переиспользовать, не создавать второй file framework |
| `input/input.h:6–28` | Macro kinds, однобайтовый action, небольшая physical table | Не подходит как arbitrary PRF macro store; не расширять runtime C053 случайно |
| `ui/font.c:61` | Text font consumer | Не считать map font PRF consumer |
| `docs/sv-architecture.md:115–120` | Native startup заканчивается live placeholder и cleanup | Real handoff/game execution пока отсутствует |

Ссылки на строки — координаты workspace исходной сверки до C004, не immutable revision.
При реализации пересверить источники; соседние незакоммиченные изменения не
сбрасывать и не заменять исторической Git-версией.

## Baseline records и gates

Owner: `src/client/c-files.c:1047`, `process_pref_file_aux_aux`.
Пустая строка, whitespace в первой позиции и `#` без следующего `:` — комментарий;
остальным нужен tag в первой колонке и `:` во второй. `my_fgets2` нормализует
line endings, tabs/printables; byte decoder — `c-files.c:135 text_to_ascii`.
Не добавлять Unicode normalization или guessed trigger charset.

| Records / source | Baseline semantics | C004 disposition |
|---|---|---|
| `%`, 1073 | Немедленный recursive load в позиции строки, ДО macro-only filter | Include stack и source provenance; explicit cycle guard |
| `X/Y`, 1092–1198, 1563–1589 | Exact aliases, inversion/one-to-many/discard; option slots и immediate application gates | Existing C002 values/aliases + current consumers; будущие option effects по своим этапам |
| `R/K/F/U`, 1211–1327 | Monster/kind/terrain/unaware mappings; index bounds; `CUSTOM_MAPPING_ALWAYS` zero semantics, graphics char offset | Семантические данные; C006/C007 выполняют реальное применение/reset/reload |
| `I`, 1329 | Под USE_GRAPHICS: raw picture definition; runtime !use_graphics и сборка без USE_GRAPHICS дают success без применения | Сохранить accepted inert branch; active consumer pending C007 |
| `r/@/Z`, 1360–1453 | Ветки реализации под `#if 0`, фактически format error | Не включать выключенную возможность по комментарию |
| `E/V`, 1455–1494 | Accepted inert, success | Сохранить disposition без fake presentation effect |
| `A`, 1496 | Decoded current action для следующих trigger records | State живёт через includes и последовательные loads одного owner, не статический global |
| `P/H/C`, 1507–1538 | Normal/hybrid/command trigger, decoded bytes, ныне no-op key_autoconvert, trigger-exclusive gate | Production macro definitions/store, runtime C053 отдельно |
| `D`, 1540 | Delete decoded trigger, без key_autoconvert | Сохранить trigger identity и отличие от P/H/C |
| `S`, 1550 | Известный дефект: все поля берутся из zz[0] | Сохранить source record/baseline disposition, effective keymap C005; исправление отдельно |
| `W`, 1591 | Legacy Term assignments | SV-approved inert compatibility data; Terms не создавать |
| `#:`, 1603 | Loud message, `{c`→colour byte, `{{`→literal brace; `{+`→chat route | Одноразовый production message intent; полный message flow pending до consumer |
| `!/?`, 1625 | Decode action, reset macro execution flags, prepend marker/action; `?` пропускается только bodymonster gate | Exact queue/gates сейчас, фактическое игровое исполнение pending C053/action owner |
| `m`, 1663 | SDL3 graphics mask RGB при USE_SDL3 && USE_GRAPHICS; без этих build gates — format error | Соответствующие SV build/semantic gates, C007 visual evidence |

Macro-only processing допускает A/P/H/C/D; остальные, включая !/?/#, отбрасываются.
Includes всё равно обходятся, потому что их branch раньше фильтра. Не вводить
новый blanket `in_game`/server gate для !. `macro_trigger_exclusive` относится
к trigger definitions и не заменяет macro-only filter.

Prepend в `src/client/z-term.c:3924` означает:

```text
load !:action1; then !:action2
pending bytes = action2, 29, action1, 29, previously queued bytes
```

Порядок bytes внутри action не меняется. Нужны исходные escapes (`\w`, `\W`,
hex/octal/control encodings), а не сохранённая текстовая строка вместо decoded
payload. Execution flags, completion и waits не считаются доказанными до runtime.
Store/effect API должен принадлежать production, не быть отдельным test sink.

## Load order, источники и отсутствующие consumers

`src/client/c-init.c:212 initialize_main_pref_files`: compiled defaults/mapping
reset → pref.prf → global.opt → pref-<sys>.prf → handle_process_font_file →
global-<sys>.opt. `c-init.c:4407–4432` применяет `<character>.opt` до Send_options.
`c-init.c:308 initialize_player_pref_files`, вызванный из `nclient.c:2867`, позже
повторяет character OPT, keymap init и global/race/trait/class/character PRFs.
Form reload — `nclient.c:2905 Receive_various` с processing_bodymonster_macros.
Эта поздняя orchestration относится C052, не переносится скрыто в bootstrap.

`c-util.c:20224 handle_process_font_file`: custom map/font PRF, иначе font-<sys>,
затем graphics overrides. Graphics owner `c-util.c:20137`: base и enabled subsets
с offset. C006/C007 должны выбирать effective resource, вызывать тот же loader
до system OPT в startup и сохранять дальнейший reload lifecycle. Нельзя
использовать выбранный text font как доказательство effective map font.

`lib/user/pref.prf:30` включает options.prf/window.prf и содержит действительные
macro definitions. Shipped pref.prf берётся из B; options entrypoint — S и только
разрешённый shipped fallback B. Прочие includes следуют source role; U legacy
options base не появляется из-за suffix PRF. Shared macro X/Y разрешены и
не меняют собственного OPT Save owner. Class OPT автоматически не загружается.

В B отсутствует pref-sv.prf. Proposed source selection в 003: existing explicit
U/B pref-sv source при наличии, иначе совместимый shipped pref-sdl3.prf. Сохранить
bytes и порядок без глобальной подмены ANGBAND_SYS; actual native trigger matching
проверяется C053. Это узкое SV-local решение, не изменение shared файла.
Missing-only fallback и диагностику invalid override не менять на «пропустить всё».

`process_pref_file` (`c-files.c:1840`) после обычного load при s_ARCADE делает
tail-load arcade-<sys>.prf, кроме самого arcade-файла; manual wrapper (`1860`)
сам не добавляет tail-load; его includes всё ещё используют normal wrapper.
Не добавлять tail-load к самому manual source. В 003 зафиксировать
applicability/server context и naming до wiring; до реального server/macro
consumer evidence pending C011/C052, без выдуманного enabled branch.

## Ошибки, source writes и lifecycle

`process_pref_file_aux` (`c-files.c:1705`) возвращает -1 при open failure и 2 при
allocation failure line reader. Ошибка record/include диагностируется, чтение
продолжается; aggregate обычно 0. Baseline не защищён от recursive cycle.
SV result делает partial/diagnostics явными, но не меняет normal load в atomic
transaction и не отменяет уже применённые option/message/action effects.
Optional automatic layer missing отличается от explicit source/include failure.
Current C002 protection от cycles/escape сохраняется и используется в новом owner.
Для explicit custom/absolute load не распространять автоматический basename-only
restriction без policy основания.

Legacy пишет `<source>.tmp`, комментирует bad records через `#ERROR:`, делает
rename и может autosave obsolete OPT/options.prf. Approved SV policy сохраняет
conversion/compatible records в памяти до explicit Save и не даёт blanket macro
autoresave. В C004 не переносить destructive load-time source rewrite.
Write/replace failures не требуют нового write framework для read-only loader;
future Save/import реализуют свои publication guarantees. Источники после
ordinary SV load не должны меняться.

Preference data приложения живут между attempts; character/session values и
effects привязаны к своему владельцу и generation. Retry не применяет overlay
предыдущего персонажа. Messages/actions не генерируются повторно на redraw,
observer read или повторное взятие уже доставленного occurrence. Закрытие
поколения освобождает payload и исключает stale session sends. Ни arbitrary
macro bytes, ни source records не попадают в credential buffers/history.

## Девять obligations и evidence disposition

Все имена ниже имеют префикс `obligation.preferences.`. Exact полные формулировки
остаются в snapshot; таблица — план их реализации, не сужение obligations.

| Obligation | Core evidence / owner | Что остаётся pending при отсутствии consumer |
|---|---|---|
| `load.result` | 001: реальный parser/store/effects, %/X/Y/mappings/bytes/gates | C006/C007 visuals; C053/action runtime; C044/C054 actual explicit-load UI |
| `load.failure-lifecycle` | 001–003: usable partial, invalid/unavailable, bounded payload, once-only/owner cleanup, source unchanged | Реальный cancel/relog/handoff C014 и late callers; Save errors проверяет Save owner |
| `load.allocation-checks` | 001/002: mixed records, warnings и точный prepend/gates | Отдельные C/D action caller regressions; enqueue не принимает gameplay |
| `bootstrap.result` | 003: actual native global/character caller и порядок, own S values, W без Terms | C006/C007 resource layers и C052 macro sequence |
| `bootstrap.failure-lifecycle` | 002/003: missing/invalid branch, fresh character attempt, cleanup, no autosave | Handoff/relog session runtime и enabled resource failures поздних owners |
| `bootstrap.allocation-checks` | 003: competing layer values, без legacy global.opt/options.prf leakage, first packet | Реальные macro precedence/command results C052/C053/action owner |
| `include-failure.result` | 002: normal missing/invalid/cyclic include и продолжение | C051 linked import group skips и no execution |
| `include-failure.failure-lifecycle` | 002/003: resolved source/diagnostics, bounded stack и cleanup без false clean | C051 group failure/cancel/commit; write/replace относится реальному writer |
| `include-failure.allocation-checks` | 002: normal-load include cases, valid tail и source bytes unchanged | C051 migration executes nothing; action caller regressions не заменяются primitive load |

Для read-only операции write/replace failure injection получает обоснованное
неприменимое operation-level disposition, но отсутствующий обязательный consumer
не получает not-applicable. Concurrent source edit не превращается в обещание
атомарного filesystem snapshot. Linux-only rule откладывает Windows evidence,
не меняя gameplay/bytes obligations.

C051 при появлении import должен parse/stage без effects/source writes и пропускать
всю linked group при missing/cyclic/out-of-source include; valid независимые groups
не страдают. Не использовать normal load с эффектами как import parser.
C052 использует тот же preference owner для trigger precedence/reloads/body gates;
C053 выполняет exact decoded actions и проверяет completion/waits/cancel.
C005/006/007/016/044/054 и конкретные action stages подключают своих consumers
по таблице спецификации; ID остаются у C004, pending evidence явно отмечается.

## Что не включать в текущую реализацию

Не исправлять monolithic legacy parser, не объединять SV/legacy ради дедупликации,
не внедрять Save/import UI, macro editor, полную macro engine, map renderer или
live handoff ради раннего PASS. Известный S parsing defect уже записан в
[список улучшений](../../sv-improvements.md#correct-legacy-s-keymap-record-field-parsing),
повторная запись не нужна. Намеренное SV-local дублирование само по себе не долг.
Новых отдельных legacy/common улучшений этот анализ не добавляет.

Минимальные production core проверки заданы tickets/spec и выполнены. C004
принят отдельным решением пользователя с переносом remaining implementation/checks
в этапы-владельцы. Canonical JSON и прежние фактические evidence не изменяются.

## Current implementation — 2026-11-09

Описанные gaps current core закрыты production `settings/preferences.[ch]` и
startup wiring; late consumers остаются pending. Добавлен occurrence-aware option
adapter, сохранены WAIT96/XWAIT30, prefix1023 и A lifetime. Ошибки prefix cut имеют
LIMIT/partial, unsafe malformed escapes отвергаются, raw source не переписывается.
Code graph обновлён `graphify update .`; его partial-extraction warnings и
фактические core checks записаны в [evidence](../../sv-c004-evidence.md).

## Завершение C004 и передача последующей работы — 2026-11-09

Пользователь решил завершить C004 сейчас и перенести недостающие доработки
и проверки на следующие этапы. [Самостоятельные планы владельцев](README.md#самостоятельные-обязательства-следующих-этапов)
содержат конкретную реализацию, минимальные Linux production-path проверки
и критерии завершения. Девять canonical obligations/IDs здесь сохранены для
трассировки; remaining rows не являются gate принятого C004. Поздние дефекты
исправляются в своём этапе без автоматического переоткрытия C004. Повторная
приёмка C004 или обязательный возврат evidence не требуются; поздние этапы
при этом остаются planned. Текущие отсутствующие consumers не объявлены
работающими и не получили выдуманный runtime PASS.

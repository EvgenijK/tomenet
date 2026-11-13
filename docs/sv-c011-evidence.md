# C011 — Linux production core evidence

2026-11-11. Реализован и проверен preparation core задач 001–006.
По дополнительному запросу пользователя закрыты оставшиеся собственные проверки
C011: native controls, birth/readout, initial audio и startup с настоящим сервером.
Исправлены найденные дефекты; поздние этапы не реализовывались.
C011 принят пользователем 2026-11-11: [решение](sv-stage-c011-acceptance.md).
Canonical runtime statuses остаются pending до отдельной регистрации evidence.
Полный live loop и непроверенные части obligations ниже не получают PASS.

## Revision, команды и артефакты

HEAD: `c29c37a153053779dddb659844315abc37dbd197`, рабочее дерево dirty до начала
C011: существовавшие C010 sources/evidence и другие изменения сохранены.
Начальный source snapshot этого прохода — `/tmp/sv-c011-start`;
список изменённых SV sources — `.scratch/sv-c011/changed-sources.txt`.

Финальные команды, обе exit 0:

```bash
python3 tests/sv_first_session_checks.py --case c011-preparation
make -C src -f makefile.sv -j4 shell
```

Сборка Linux amd64, clang, C99, `-O2 -g -Wall -Wextra -Werror`;
production object closure из `makefile.sv`, не отдельная реализация для тестов.
Check binary SHA-256:
`efe3166a29eb34f4b6dac8945fbe3715110301b4e8c21a8b5800ecbac715a75b`.
Runner сохраняет revision/dirty state, hashes 152 production/check sources и
headers, binary hash, environment, commands и exit codes. Root сверил hashes:
после финального прогона production/test sources не изменились.

Артефакты исходных шести cases: `.scratch/sv-c011/<case>/`; новых пяти —
`.scratch/sv-c011-followup/<case>/`. В каждом каталоге `command.json`, `build.log`,
`stdout.log`, `stderr.log`, `file-transcript.log`. Итоговые общий log и сборка —
`.scratch/sv-c011-followup/final-checks.log` и `build.log`; source reconciliation —
`final-reconciliation.json` в том же каталоге.
Реальные TCP round trips выполнялись с loopback managed peer; sandbox socket
restriction потребовала разрешённого escalated запуска. Dummy SDL software
renderer/audio, isolated profile/library; original `lib` hashes до/после
совпадают во всех одиннадцати случаях. Настоящий пользовательский аккаунт
и публичный сервер не использовались. Отдельный actual-server run описан ниже.
Sanitizer, accelerated/visual matrix и Windows evidence этим запуском не заявлены.

## Наблюдаемые результаты

| Case | Фактический результат |
|---|---|
| `c011-preparation` | Existing character → effective character options внутри точного PLAY → reply/END → post-END CHECK/SUM, INIT/DATA/END/ACK и точная публикация файла → prefix → accepted identity/INS → retained payload. Complete=1, active transfers=0, runtime ready |
| `c011-initial-repair` | Missing c-init на begin → FILE restore/reload; текущий runtime ready, первоначальная begin failure сохранена |
| `c011-transfer-failure` | Ошибка publication даёт ERR; успешный последующий reload не стирает transfer failure: complete=0 |
| `c011-startup-quit` | Split QUIT после END даёт видимую причину отказа, prepared payload не передан |
| `c011-cancel` | Отмена до END освобождает попытку и pending данные без передачи payload |
| `c011-retry` | Отказ до PLAY → retry → новая generation; новый aggregate, unrelated FILE END доказывает сохранение application-sticky Guide flag |
| `c011-controls` | Два native PAUSE, stale input rejection, printable/Return acknowledgement, сохранённый request; 12 FLUSH → 10 paced / 2 unpaced |
| `c011-birth` | Полный native birth UI → PLAY → MOTD acknowledgement → accepted identity на preparation surface без старого birth overlay |
| `c011-audio-enabled` | Actual dummy device, U sound / bundled music `.default` fallback, subsets, counts 1/1, exact PLAY/PKT_AUDIO и retained lifetime |
| `c011-audio-missing-pack` | PACK_FAILED, counts 0/0, requested CFG сохранён, complete=1 |
| `c011-audio-no-device` | NO_DEVICE, counts 0/0, diagnostic и requested values сохранены, complete=1 |

В том же существующем бинаре выполнены необходимые production core cases:

- Пять identity wire branches: partial frame без publication, absent defaults,
  old privilege bits, setup-index rejection, native accepted name `Accepted`
  вместо выбранного `Hero`, close/reopen и stale generation.
- Committed birth через production interaction/core → проверенный play draft →
  общий PLAY serializer с однократной постановкой. Дополнительно полный native
  birth UI и post-MOTD identity readout проверены managed peer и actual server.
- Global/character messages и prepend action queues имеют разные точные bytes;
  ack одного occurrence не очищает остальные. History, effective normal keymap,
  exact embedded PLAY option slots и два FLUSH с character disable/thin options
  сохранены; отдельный pre-PLAY OPTIONS не отправляется.
- INS most-specific selection, порядок/duplicates/partial duplicates, v1
  wildcard/force conversion и readback, I/O disposition/fallback.
- Actual initial audio snapshot удерживает diagnostic и requested
  rate/tracks/buffer `44100/32/1024` при disabled sound. Это не фиктивный нулевой
  snapshot; C002 audio implementation используется без второго backend.
- Ctrl-R в quantity context не redraw; output backpressure удерживает одну
  command occurrence, последующий drain даёт ровно `PKT_REDRAW,0`, одну revision;
  повторный local repaint не повторяет команду, старая generation отвергается.
- Payload после возврата native runner сохраняет app/Lua/history/preferences,
  renderer/font/audio/graphics lifetime. Полные requested/effective resource refs,
  mapfont source/result и definitions доступны после возврата; их сериализация
  MD5 совпадает с фактически отправленным PLAY. Public FILE/pump отвергает stale generation.

Managed startup prefix содержит 21 deferred frame / 557 bytes: PARTY/GUILD/CFG,
rendering/powers, SKILL_INIT с 120-byte description, skill updates и 80-column
RLE-map, ordinary/wide item frames, initial audio и unique-list frames. Glyph width выведен из фактически полученного полного PLAY: 2/2,
graphics 1/1. RLE и identity приходят раздельными частями. Наблюдение:
`startup=1 identity=1 profile=1 complete=1 unsupported=0 ingress=29`.
FIFO frame sequences 8–28 сохраняют exact bytes; applied CHECK/INIT/DATA/END,
controls/message и identity не replay. Probe не consume, drain однократен;
raw tail `PKT_INVEN,a1,b2,c3` доступен только после deferred FIFO и ровно один раз.

## Disposition 18 canonical obligations

`CORE CHECKED` относится только к указанному subflow. Descriptions, prerequisites,
conditions и statuses точной allocation сохраняет [snapshot](tasks/c011/coverage.json).
В pending части названы поздние owners; их наличие не превращает частичный
результат в full-obligation PASS.

| Obligation (`obligation.session.`) | Проверенный subflow и pending remainder |
|---|---|
| `load-profile-input.result` | CORE CHECKED: actual profile/options/history/keymap, два preference sources и retained occurrences; macro layers/execution C052/C053 pending |
| `load-profile-input.lifecycle` | CORE CHECKED: native cancel/retry и lifetime после runner; actual live adoption/close/relog C014 pending |
| `load-profile-input.allocation-checks` | CORE CHECKED: effective options внутри PLAY, generation, bytes/ack isolation; gameplay invocation и поздние recipients pending C014/C016/C052/C053 |
| `load-profile-input.startup-ins` | CORE CHECKED: actual identity→INS selection/rules/conversion; inventory application C021 и full management C059 pending |
| `load-profile-input.startup-audio-packs` | CORE CHECKED: actual disabled initial projection и удержание snapshot; actual enabled pack/subset/fallback и missing-pack branches C011 повторены; live C055/C056 pending |
| `load-profile-input.startup-audio-device` | CORE CHECKED: actual requested device settings/lifetime в disabled snapshot; actual enabled/no-device C011 branches повторены; live reconfiguration C056 pending |
| `transfer-startup-files.result` | CORE CHECKED: actual pre/post-END C010 path, results/readiness и сохранённый owner; будущие asynchronous updates после передачи C014 pending |
| `transfer-startup-files.lifecycle` | CORE CHECKED: preparation failure/cancel/retry; continuous live transfer/adoption C014 pending |
| `transfer-startup-files.allocation-checks` | CORE CHECKED: FILE→reload, repaired current runtime при сохранённой history, exact publication/ACK; dependent gameplay C014/поздних owners pending |
| `read-identity.result` | CORE CHECKED: actual native prefix→identity→readable view/INS/payload и 5 decoder branches; subsequent live updates C014 pending |
| `read-identity.lifecycle` | CORE CHECKED: partial/invalid/stale generation и destruction; полноценные live focus/resize/relog flows C014 pending |
| `read-identity.allocation-checks` | CORE CHECKED: accepted identity отличается от draft/list, preceding frames не теряются; late form/layer flow C052 pending |
| `startup-file-failure.result` | CORE CHECKED: initial script failure/recovery и publication ERR независимо от успешного reload; остальные I/O branches C010 здесь не повторялись |
| `startup-file-failure.lifecycle` | CORE CHECKED: quit/cancel/retry и раздельная readiness; post-adoption live failure C014 pending |
| `startup-file-failure.allocation-checks` | CORE CHECKED: false complete при failed publication исключён; no blanket app shutdown от optional resource degradation; combined live consumer outcomes pending |
| `redraw.result` | CORE CHECKED: production command executor, точные bytes/revision и coherent existing state; full HUD/map consumers C012/C013 pending |
| `redraw.lifecycle` | CORE CHECKED: backpressure/once-only/context/stale; integrated live resize/focus/teardown C014 и поздних surfaces pending |
| `redraw.allocation-checks` | CORE CHECKED: raw gameplay не подменяет redraw; preserved payload/view, без reload/effect replay; полные поздние projections pending |

## Review и границы

Три агента реализовали и взаимно проверили независимые части. Исправлены findings:
premature generic gameplay input, stale byte counts, partial duplicate INS loss,
pacing WAITING vs incomplete packet, failed-connection take guard, lifetime stack
callbacks, обязательный prefix, post-END FILE и порядок replies, потеря map
resources при возврате runner и export неполного native binding. Final independent
reviews integration и prefix decoder новых блокирующих дефектов не выявили.

Нет CHECK-batch terminator: readiness описывает текущую обязательную подготовку,
не обещает отсутствия будущего asynchronous INIT. C014 продолжает тот же C010
registry через public receive_file/pump/output API. Дополнительный неподдержанный
prefix удерживается с явным incomplete result; overflow явный, без recovery/drop.
Framing не принимает gameplay capabilities поздних consumers.

Native PAUSE acknowledgement и thin pacing проверены actual C011 runtime:
два PAUSE действительно представлены, два stale события отвергнуты, свежие
printable/Return gestures приняты один раз; request 7 не отвечен этим вводом.
Text input активен во время PAUSE и выключен до teardown; первые десять FLUSH
имеют actual monotonic deadline, два следующих не задерживаются политикой.
Это preparation caller C008, а не будущий C014 combined flow.
Full live C014, map/HUD C013/C012, macro runtime, effect delivery, INS application
и live audio остаются своими owners. Windows/MinGW/Wine отложены по текущему правилу.

Существующий code graph обновлён `.graphify-venv/bin/graphify update .`, exit 0.
Graphify сообщил ограничения своего AST extraction для части legacy sources;
они не являются результатом компилятора. Code graph обновлён без LLM extraction,
semantic graph документов этим CLI проходом не пересобирался.


## Закрытие оставшихся проверок C011 — 2026-11-11

Пользователь разрешил агентам закрыть оставшиеся проверки самого C011 и исправить
найденное, без работ следующих этапов. Существующий first-session harness получил
необходимые пять native variants и дополнительные assertions; нового общего
suite/decoder/test-only поведения нет. Test-only linker wrappers наблюдают real
calls, rendered semantic view и SDL events; production metrics не добавлялись.

Исправления:

1. Bytewise preparation assembly сбрасывал thin counter на каждом packet:
   12 FLUSH давали 12 paced. Opt-in `retain_batch` в прежнем native control owner
   удерживает ограниченный batch через decoder WAITING. Настоящий socket WAITING,
   budget exhaustion и terminal identity boundary заканчивают batch; deadline
   сохраняется. Default C008 caller не меняется. Итог: 10 paced / 2 unpaced.
2. Native PAUSE не включал SDL text input, хотя printable key acknowledgement
   требует TEXT_INPUT. Assembly временно включает ввод, показывает подсказку,
   выключает его после ack; старый paired tail не подтверждает следующий PAUSE.
3. Standalone pre-PLAY OPTIONS блокировал вход на настоящем сервере. В SV убран
   только ранний `sv_login_send_options`; character snapshot готов до PLAY и
   передаётся внутри него. Direct legacy `Net_start` очищает queued bytes перед
   PLAY; для interactive legacy путей возможность промежуточного flush отдельно
   не отрицается. Историческое C002 evidence не переписано.
4. Обязательные до identity startup frames EQUIP/INVEN, initial audio и unique
   list не все были поддержаны. Добавлен только version-aware framing/deferred
   retention, включая wide items и conditional SOUND/MUSIC_VOL. Модели инвентаря,
   playback и unique consumers не добавлялись. SUBINVEN не перенесён: его initial
   window update идёт после PR_BASIC/первой CHAR_INFO.
5. После native birth старый waiting overlay скрывал accepted identity, а handoff
   card показывала stub. Post-MOTD preparation flag подавляет завершённый birth
   и выводит identity в существующей карточке. Тот же context gate ограничивает
   native Ctrl-R; ранний PAUSE остаётся доступен. Failure status имеет приоритет
   над identity/PAUSE текстом; этот дефект последнего review также исправлен.

Дополнительно проверены повторный accepted identity/revision, redraw на coherent
старом view перед interleaved HP/message и последующий redraw нового view,
сохранение message delivery; старый RLE и modern dual-mask W4 partial/exact bounds.
Managed prefix содержит по одному современному frame новых типов; exhaustive
историческая packet matrix не заявлена.

Initial audio: requested 48000/4/128 → effective 48000/4/128, stereo=2,
counts=1/1, cache=1; fallback config/sample и subsets прошли через actual startup.
При missing pack и invalid device driver counts=0/0, complete=1, диагностические
результаты раздельны. CFG не переписан, audio owner жив после возврата runner и
освобождён с payload. Оба driver aliases SDL3/legacy заданы явно, inherited buffer
hint снят. Это настоящие SDL/mixer API с dummy provider, без audible playback.

Исторические development failures не объявлены успешными. Raw text-input failure
сохранён в `controls-before-text-input-fix/`; initial thin 12/0 доступен только как
извлечённое наблюдение в `controls-before-thin-fix/observation.txt`, исходный hash
не заявляется. Детали — [pacing report](../.scratch/sv-c011-followup/pacing-report.md)
и [audio report](../.scratch/sv-c011-followup/audio-report.md).

## Настоящий сервер и Linux visual check

Финальный probe `real-server-12`: существующий `src/tomenet.server`, wire version
4.9.3.0.2.0, серверный binary SHA-256
`7f3d1257cbee8acbf7f432eac01044afd301ab0554b645da288ff3cf88e080ba`.
Fresh library/profile/save/data внутри отдельного user/network namespace, только
loopback; original save/data не копировались. Использован только временный
локальный account/character, публичных контактов нет. Сервер после проверки
намеренно остановлен SIGTERM (server exit=-15); клиент exit=0.

Production `sv_endpoint_run` прошёл native birth (в том числе actual point-buy),
полный PLAY/reply/END, MOTD с native acknowledgement, accepted identity `Born`,
startup/runtime/profile readiness и возврат retained payload. Итог:
`phase=8, motd_ack=1, handoff_rendered=1, startup=1, identity=1, runtime=1,
complete=1, unsupported=0, deferred=666`. Примерно 30 KiB retained startup frames
укладываются в текущие bounds 2048 frames / 256 KiB. Actual gameplay не запускался
как проверяемый client flow; префикс сохранён для поздних owners.

[Server/client metadata](../.scratch/sv-c011-followup/real-server-12/command.json)
содержит commands, environment, binary hashes, captures и 149 production source/
header hashes; root независимо подтвердил их совпадение с текущими файлами.
[Полная команда и разбор попыток](../.scratch/sv-c011-followup/birth-report.md).
Actual probe использует отдельный одноразовый driver жестов/наблюдения, связанный
с теми же production objects; он не реализует игровой/протокольный simulator.

Просмотрены native birth SEX/STATS/MODE, MOTD и post-MOTD preparation на X11 с
software renderer при 1024×768. Root отдельно просмотрел
[финальную preparation карточку](../.scratch/sv-c011-followup/real-server-12/captures/prepared.png):
`Born` и все reported поля видимы, текст переносится без обрезания; completed
birth её не перекрывает. Это ограниченный actual visual check, не полная matrix.

Probe 05 ошибочно интерпретировался как draw failure: driver не подтверждал MOTD
и исчерпал frame limit. Explicit false draw не наблюдался; exact 2760-byte MOTD
успешно повторно отрисован production-функцией с теми же assets. SDL texture error
был устаревшим после успешного cleanup. Попытки 01–07 остаются неполным/failed
историческим evidence; ранние captures с caption prepared/MODE не используются
для доказательства финального readout. [Разбор](../.scratch/sv-c011-followup/motd-report.md).
Отдельные legacy/SDL diagnostic улучшения записаны в [список](sv-improvements.md)
и не включены в текущую реализацию.

## Итог текущего scope

Собственные перечисленные проверки C011 закрыты, известных блокирующих findings
в проверенном preparation scope не осталось. Все 11 cases и Linux production
build проходят на едином финальном snapshot; canonical 5/18 allocation и statuses
не менялись. Пользовательская приёмка не заявлена. Непроверенные поздние portions
каждого obligation остаются указанным owners в таблице выше. Physical audio,
physical keyboard, accelerated matrix и sanitizer coverage не заявлены этим
focused проходом; Windows/MinGW/Wine отложены действующим правилом.

Финальный code graph обновлён, exit 0: [log](../.scratch/sv-c011-followup/graphify-update.log).
28415 nodes / 70369 edges / 1342 communities; HTML использует aggregated view.
Это code-only update без LLM extraction; semantic graph документов не пересобирался.
Ограничения AST extraction части legacy files не являются compiler failures.


## Пользовательская приёмка — 2026-11-11

После этого отчёта пользователь явно принял C011 и задачи 001–006.
[Решение](sv-stage-c011-acceptance.md) фиксирует текущий accepted статус;
упоминания отсутствия приёмки в исторических разделах выше относятся к моменту
проверок. Технические наблюдения и ограничения этим решением не меняются.

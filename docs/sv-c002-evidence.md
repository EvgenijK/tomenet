# C002 — реализация и Linux evidence

Реализация и первоначальное evidence — 2026-11-05; birth follow-up — 2026-11-07.
Статус: accepted пользователем 2026-11-07;
[решение о приёмке](sv-stage-c002-acceptance.md).
Canonical native-runtime registration pending; технические наблюдения ниже
не переписываются как machine-readable registry PASS.
[Spec](sv-stage-c002-spec.md) · [задачи](tasks/c002/README.md) ·
[source contract](tasks/c002/source-contract.md) ·
[техническая сверка](sv-stage-c002-acceptance.md).

## Реализация

- `src/client/sv/settings/config.[ch]`: requested CFG snapshot, client defaults,
  exact keys, scalar validation, LF/CRLF, aliases совместимого формата,
  retained unknown/inert records, исключение `pass`, future-schema guard.
  `server host:port` применяется в порядке CFG/CLI, поэтому последующий port
  override действительно побеждает. `-i` не снимает запрет overwrite future schema.
- `settings/startup.[ch]`: загрузка S CFG и CLI reduction в production,
  без записи. Сохранены long options существующего SV и полный baseline CLI data union: rooted
  overrides, positional server/account/password state, request/protocol/DPI/log flags.
  [Пофлаговое disposition](tasks/c002/cli-disposition.md) связывает каждый
  applicable flag с действующим consumer, остальные — с named late owner.
  Сохраняются explicit root → environment → SDL/default library fallbacks;
  `-f` не меняет roots и не импортирует пароль. Session-only CLI password передаётся credential owner и сразу стирается из
  invocation; ни CFG, ни diagnostics его не удерживают. `-R` сохраняет existing
  selection/creation gates, включая15-byte identity без потери символа.
  `-C` применяется после каждого endpoint selection, `-s` использует общий
  display-scale owner, `-e` реально перенаправляет stderr. Late effects не
  объявляются готовыми по сохранённым invocation flags.
- `settings/options.[ch]`: 199 wire slots, 188 известных имён, compiled defaults,
  shipped bootstrap и собственные S option layers, exact alias branches,
  include ownership и retained records. Shared PRF interpreter C004/C052/C053
  не подменяется тестовой реализацией: этот reader обрабатывает option data.
- `login/native-run.c` применяет window/font/UI scale/FPS/endpoint settings до
  использования. Text selection доходит до body font shell; UI rendering и
  pointer routing используют один scale. Headings/MOTD сохраняют отдельные роли.
- `login/flow.c` загружает character overlay после authoritative selection,
  до первого `PKT_OPTIONS`, birth и play setup. Retry начинает со свежего global
  snapshot и перечитывает character file. `protocol/login.c` сохраняет wire
  counts/version gates; layout проецируется в big_map и66×44/66×22, с normal
  fallback для серверов без big-map geometry. Gameplay map outcome C013 не принят.
- `settings/audio.[ch]`: actual initial SDL3_mixer device/track allocation,
  shared overlay/bundled pack preparation, subset/event/sample identities,
  optional cache и безопасная деградация. Requested config не заменяется
  effective values. `audioBuffer` локально применяется через scoped SDL hint;
  actual frames читаются у устройства. Net_start и subsequent Send_audio
  получают actual counts/identity; Send_audio разрешён после play ACK.

Legacy/common source files не изменены. Build добавляет `sdl3-mixer` и четыре
settings objects. Existing Python check launchers получили только необходимые
link dependencies. Новый harness и полная matrix не создавались.
Два отдельных найденных улучшения вынесены в [список](sv-improvements.md).

## Среда и идентичность

Base revision `dc14c4ad5d5a9aa202051de929cbc9b601ddadb7` плюс C002 working tree.
Manjaro Linux x86_64, kernel6.18.49, glibc2.44, clang22.1.8.
SDL3 3.4.16; SDL3_ttf3.2.2; SDL3_mixer3.2.4; FreeType26.6.20
(pkg-config version); libpng1.6.58; libcurl8.22.0; libsecret0.21.7.
Это Linux development environment, не новая Fedora package acceptance.

Final binary SHA256:
`ce9409b5898946aba3bf2cd7a729c933bbdbbff448d97587db77f6b762d4946e`.
[Environment, binary и production source hashes](../.scratch/sv-c002/evidence/environment.json).
Build ID является toolchain key, поэтому одного build ID недостаточно для
идентичности source revision. Windows/MinGW/Wine deferred.

## Минимальные проверки

| Проверка | Результат и evidence |
|---|---|
| `make -C src -f makefile.sv tomenet-sv -j4` | PASS; [build log](../.scratch/sv-c002/evidence/build.log). SV компилируется с `-Werror`; прежние warnings common не выдаются за новые ошибки SV. |
| `tests/sv/settings-config.c` + production config | PASS; [команда/лог](../.scratch/sv-c002/evidence/config.log). Defaults, bounds, complete tokens, NUL/CRLF, records/schema/IO, byte-identical sources. |
| `tests/sv/settings-startup.c` + production config/startup | PASS; [команда/лог](../.scratch/sv-c002/evidence/startup.log). In-order `-f/-i`, inline host:port и поздний override, quiet, no writes, future-schema guard после `-i`. |
| `tests/sv/settings-options.c` + production options/login | PASS; [команда/лог](../.scratch/sv-c002/evidence/options.log). Exact aliases, owned layers/includes, reset, versioned first option/play bytes. |
| `tests/sv/settings-audio.c` + production audio/config/options/login | PASS; [команда/лог](../.scratch/sv-c002/evidence/audio.log). Actual dummy device, packs/subsets, failure/disabled/cache, Net_start/Send_audio bytes и identity limits. |
| Existing `python3 tests/sv_login_checks.py` | PASS; [raw log](../.scratch/sv-c002/evidence/login.log): прежние decoder/contact/MOTD checks и минимальные production `-R` request/15-byte cases; fixture не ослаблен. |
| Existing `python3 tests/sv_first_session_checks.py` | PASS, ASan+UBSan, local managed TCP peer; [final raw log](../.scratch/sv-c002/evidence/first-session-final.log), [ранний agent report](../.scratch/sv-c002/evidence/first-session.log). Проверены реальные S/Hero.opt bytes и перечитывание при retry; prefilling account заменяется через production Ctrl+A. |
| Обычный executable, missing CFG | PASS, dummy video/software + dummy audio; fullscreen/UI100/Cascadia/default FPS100; [лог](../.scratch/sv-c002/evidence/missing.log), [кадр](../.scratch/sv-c002/evidence/missing.png). S не создаётся. |
| Обычный executable, saved CFG + CLI overrides | PASS, window1280×960/UI125/PCF/small, FPS75→CLI90, audio disabled; [лог](../.scratch/sv-c002/evidence/configured.log), [кадр](../.scratch/sv-c002/evidence/configured.png). Unrelated U CFG не подмешивается. |

Дополнительный короткий native запуск новых applicable flags прошёл:
[`-l/-C/-s/-e` и retained deferred values](../.scratch/sv-c002/evidence/cli-native.log).
Private test argument отсутствует в stdout/redirected stderr и не записан в CFG;
`-s` достигает общего scale owner; `-e` создаёт только явно запрошенный
`tomenet.log` в изолированном cwd. Нет contact к реальному серверу.

Native command lines сохранены в логах; в обоих случаях server-list локальный,
contact не запускался. Файловые SHA256 до/после совпадают:
[native results](../.scratch/sv-c002/evidence/native-results.json).
Кадры просмотрены: default и configured layout показываются, масштаб применяется.
Это ограниченная startup проверка, не повторная B2/renderer/font matrix.

Audio device observations requested rate/tracks/buffer → effective:
4000/4/128 →44100/4/128; 44100/32/1024 →44100/32/1024;
48000/32/8192 →48000/32/8192. Output stereo, tracks не speaker count.
Requested4000 сохраняется при negotiated44100; внешний buffer hint восстановлен.
Initial native run также открыл реальный dummy mixer с bundled packs; это
проверка SDL device API, не evidence физического аудиоустройства или playback C055.

## Disposition всех 20 obligations

Prefix каждой строки — `obligation.settings.`. Exact canonical descriptions
не изменены и находятся в [snapshot](tasks/c002/coverage.json).
Observed означает локальный технический результат, не canonical PASS.

| Obligation | Технический результат / граница |
|---|---|
| `load.result` | Observed: S-owned load, actual character values перед первым packet; full mapping/macro effects остаются у своих consumers. |
| `load.failure-lifecycle` | Observed read/include errors и usable defaults; no writes, no redraw duplicates, fresh retry. Save/replace/concurrent-write faults неприменимы к этому read-only caller. |
| `load.allocation-checks` | Observed: distinct U/S/B fixtures, include provenance, unrelated owners unchanged, повторный character read. |
| `load.startup-audio-packs` | Observed: actual startup consumer, packs/subsets/sample registry и identity/reporting; missing pack/disabled counts0 без изменения requested config. |
| `load.startup-audio-device` | Observed: actual open, requested/effective rate/tracks/buffer/cache, stereo и no-device degradation; device failure не останавливает caller. Physical device coverage не заявляется. |
| `defaults.result` | Observed approved scalar defaults через production API и native startup; missing files создают только memory state. |
| `defaults.failure-lifecycle` | Observed missing/invalid/unreadable data; startup/quit без CFG/OPT publication. Другие write-provider failures отсутствуют в этом caller. |
| `defaults.allocation-checks` | Observed exact default values/bounds и отсутствие S/file creation. |
| `parse.result` | Observed grammar, defaults/duplicates, unknown records, secret exception и future schema guard. Future Save обязан использовать этот guard; writer C044 не реализован здесь. |
| `parse.failure-lifecycle` | Observed malformed/NUL/IO and safe diagnostics; read-only API не исполняет migration actions и не публикует файлы. |
| `parse.allocation-checks` | Observed LF/CRLF, full key tokens/prefix collisions, retained unknown, scalar failures, overwrite prohibition в том числе после `-i`. |
| `cli-overrides.result` | Observed полный CLI data union и действующие consumers по source-backed disposition. Поздние weather/transfer/export/cache effects остаются обязательными у своих owners; их отсутствие не маскируется parser PASS. |
| `cli-overrides.failure-lifecycle` | Observed invalid settings arguments, precedence и no writes; data-only invocation не делает import/vault publication. |
| `cli-overrides.allocation-checks` | Observed U/B selection/overrides, CFG неизменён; fingerprint production login продолжает получать U. Import/root relocation отсутствует. |
| `exit-unsaved.result` | Observed native normal exit с отличающимися memory/CFG values: нет CFG/OPT write и нового prompt. Полный settings edit UI ещё C044. |
| `exit-unsaved.failure-lifecycle` | Observed quit/cancel/disconnect/retry с production flow; независимый vault lifecycle не перенаправлен в settings writer. |
| `exit-unsaved.allocation-checks` | Observed no autosave; history/DNA/bookmarks не реализуются заново. Settings Guide entry не создаётся в C002, временный Guide placeholder не считается coverage Guide. |
| `aliases.result` | Observed exact rename/inversion/one-to-many/discard, только memory. |
| `aliases.failure-lifecycle` | Observed unknown/malformed records и source byte identity, без migration execution/write. |
| `aliases.allocation-checks` | Observed все различающиеся baseline alias branches в production loader; никакого guessed rename или source rewrite. |

## Ограничения и отдельные результаты

Исторический запуск 2026-11-05: `sv_birth_complete_checks.py` FAIL на ожидании финального `PKT_PLAY`:
старый `push_to_mode()` burst теряется при production owner input boundary.
Точно то же падение воспроизведено на неизменённом HEAD в изолированном дереве:
[raw baseline log](../.scratch/sv-c002/evidence/birth-baseline.log),
[current report](../.scratch/sv-c002/evidence/birth-complete.log).
Это не новый regression C002, но full birth handshake этим сценарием не доказан.
Assertions в историческом запуске не отключались; исправление тестового choreography
было вынесено отдельно.

Follow-up 2026-11-07: локальный test driver подаёт один gesture после настоящего
`sv_endpoint_draw`, читая borrowed semantic view только в owner thread. Смена
owner и `stats_order_count`/highlight управляют продвижением; MODE Enter отдельно
разрешает peer после проверки отсутствия преждевременного пакета. Production
input boundary и общий helper не изменены. Дополнительно исправлено устаревшее
ожидание exit code: явный `SDL_EVENT_QUIT` означает normal exit `0` во всех
трёх случаях (`sv_login_flow_command(EXIT)`), а успешность handshake проверяет
строгая phase, не process exit.

Команда: `SV_BIRTH_EVIDENCE="$PWD/.scratch/sv-c002/evidence/birth-fixed-2026-11-07" python3 tests/sv_birth_complete_checks.py`.
Linux, SDL dummy/software, ASan+UBSan (`detect_leaks=0`): PASS всех трёх существующих
cases: waiting и partial остались `play-pending`, ack достиг `live-session-handoff`.
В каждом случае проверены 16266 literal handshake bytes, early character
`PKT_OPTIONS`, отсутствие преждевременного/повторного `PKT_PLAY`, ping и финальные
birth values; ack проверил disabled `PKT_AUDIO`. Сохранены
[raw successful run](../.scratch/sv-c002/evidence/birth-fixed-2026-11-07/raw-04.log),
case logs, packet captures и reply fragments в том же каталоге. Новые matrix и
соседние suites не запускались. Историческое отсутствие birth evidence выше
относится к запуску 2026-11-05; этот follow-up закрывает ограничение для трёх
default cases текущего Linux workspace, без canonical registration или приёмки.

Resource mapping/font profiles, UI settings/Save,
preferences execution, audio playback и последующие consumers не объявляются
готовыми. В частности, PCF own-UI profile по-прежнему ограничен: недостающие
Unicode symbols видимо заменяются; это не evidence полной C006 glyph coverage.
CLI union сохранена как полный invocation data contract; source-backed
first-use boundary перечисляет действующих и поздних consumers. Полный semantic
sweep и registry runtime registration не выполнялись. Все шесть canonical rows
сохраняют pending. Для `-e` известно отличие diagnostic order: log открывается
один раз после успешной проверки argv, а не в каждой позиции `-e`; flags/CFG
precedence и игровая/протокольная семантика от этого не меняются.

Standalone audio sanitizer probe упёрся в LSan/ptrace restriction и не считается
sanitizer PASS. First-session ASan+UBSan использует существующий recipe с
`detect_leaks=0`. Initial root audio test launch с внешним SDL_AUDIODRIVER
конфликтовал с test-owned driver hint; после изоляции environment тот же test
прошёл, production code ради этого не менялся.

Graphify code update выполнен; [лог](../.scratch/sv-c002/evidence/graph-update.log).
Предупреждения об ограничениях AST legacy macros и отсутствии semantic refresh
документов сохранены. Graph не заменяет проверенные исходники и evidence.

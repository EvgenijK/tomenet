# C010 — Linux production core evidence

**Приёмка 2026-11-11:** пользователь принял C010 в границах Linux startup/core —
[решение](sv-stage-c010-acceptance.md). Отчёт ниже сохраняет observations и
ограничения своего запуска; прежние формулировки о незаявленной приёмке исторические.

Дата: 2026-11-10. Реализованы пять `capability.transfer.*` в границах текущего
startup/core. Это технический результат, не пользовательская приёмка C010 и не
автоматический PASS всех canonical obligations. [План](tasks/c010/README.md),
[baseline contract](tasks/c010/source-contract.md), [snapshot](tasks/c010/coverage.json).
Canonical ownership, prerequisites, conditions и registry statuses не изменены.

Базовая revision: `c29c37a153053779dddb659844315abc37dbd197`.
Workspace dirty: ранее подготовленные документы C010, новые SV transfer sources,
login/startup integration, build и одна связанная core-проверка. Результаты
относятся к этому рабочему дереву, не к чистой revision.

## Реализованный путь

`transfer/transfer.[ch]` владеет временными файлами, registry входящих и исходящих
передач, version/checksum, publication и bounded output. Linux пути сохраняют
baseline `ANGBAND_DIR + filename`; CHECK/source и INIT имеют разные validation
rules. END копирует temporary через `wb`, без export prompts и atomic-replace
гарантии. Legacy/common не изменены; используется существующий MD5 implementation.

`transfer/startup.[ch]` — production adapter существующего application Lua owner.
`protocol/login.c` dispatch принимает полные FILE events и вызывает adapter;
`login/flow.c` создаёт attachment каждой attempt, продвигает sender даже без
входящих bytes и возобновляет buffered input после output drain. Borrowed callbacks
сохраняют standalone linking login без обязательного Lua engine. Attachment
закрывается до Lua owner; copied transfer aggregate и отдельный begin result
сохраняются в endpoint outcome. Реальные decoded flags запускают begin один раз;
`-u` приходит из native invocation.

При END: close/publication → last reload → audio warning → Guide metadata → ACK/ERR.
Failed last close не отменяет reload, а успешный reload не меняет ERR на ACK.
Каждый completion потребляется до следующего END; counters удерживают ранние
transfer/runtime/Guide failures. Process-sticky flags переживают смену attempt.

## Команды и результаты

```sh
SV_TRANSFER_COMMAND_LOG=.scratch/sv-c010/transfer-core-command.json python3 tests/sv_transfer_core.py
make -C src -f makefile.sv -j4 tomenet-sv
```

Обе команды: **PASS**. [Core log](../.scratch/sv-c010/transfer-core.log),
[полные compile/link/run argv, compiler и source hashes](../.scratch/sv-c010/transfer-core-command.json),
[build log](../.scratch/sv-c010/build.log),
[build metadata](../.scratch/sv-c010/build-metadata.txt).
SV/test sources собраны с `-Wall -Wextra -Werror`, ASan и UBSan;
vendored Lua engine — отдельно без `-Werror`, как в C009.
`UBSAN_OPTIONS=halt_on_error=1`, `ASAN_OPTIONS=detect_leaks=0`;
leak detection не заявлена. Sanitizer diagnostics отсутствуют.

Одна проверка [transfer.c](../tests/sv/transfer.c) вызывает настоящий factory,
login decoder/tick, transfer owner, filesystem и Lua. Wire builders создают
только input/expected bytes, не реализуют parser, sender или Lua aggregate.
Все записи выполняются в собственном `/tmp/sv-transfer-core-*`, bundled `lib`
читается. Linux write failure создаётся реальным RLIMIT_FSIZE с восстановлением
limit/signal до последующих assertions, не test-only backend.

Проверены:

- Strict gate `>4.6.1.1.0.1`: old checksum ниже/на границе, MD5 выше;
  `abc`: old `02 4e 01 27`, MD5 `900150983cd24fb0d6963f7d28e17f72`;
  empty old=1/MD5 empty, missing=zero SUM, CHECK `name..txt` без INIT-only фильтра.
- Split SERVERDETAILS с непустыми flags и TEMP → actual Lua getters; partial
  INIT/DATA не фиксирует effects. Полный DATA 1030 bytes и chained INIT/END
  сохраняют publication, active count и last reload.
- Failed destination publication → ERR и reload; последующий successful END
  не стирает failure. Успешная передача invalid Lua или Guide getter → ACK
  при отдельном runtime/Guide failure и предшествующем audio warning.
- Fresh application owners доказывают sticky accepted INIT → disconnect → новая
  generation и отсутствие flags от rejected/`-u` INIT, включая обычный non-Lua файл.
- Реальный login output buffer заполнен: drain/tick возобновляет обработку без
  повторного INIT/publication/reload. Direct stale generation не даёт effects;
  zero ID/malformed length отклоняются, unknown DATA потребляется целиком с ERR.
- Upload: cancel/31-byte rejection, CHECK/SUM match release и mismatch→INIT,
  ACK-paced DATA 256/1024, exact chunk/empty EOF→END по ticks без новых bytes,
  final ACK/ERR cleanup. Реальная temporary write failure потребляет DATA,
  снимает transfer и не создаёт ложный ACK или повторную запись.

## Disposition 20 obligations

Таблица фиксирует исходный запуск 005. Последующее [evidence 006](sv-c010-006-evidence.md)
закрывает sender-specific output saturation и actual native startup flow;
прочие pending части сохраняются.

В таблице suffix дополняет `obligation.transfer.`. CORE CHECKED означает только
перечисленные наблюдения. Pending части не превращаются в PASS всего obligation.

| Exact suffix | Подтверждено / ограничения |
|---|---|
| `check.result` | CORE CHECKED: old/MD5 SUM bytes, missing/empty и original path rules. |
| `check.failure-lifecycle` | CORE CHECKED: split/stale/missing; pending EACCES и prefix read I/O failure, отдельный unrelated-owner inventory. |
| `check.wire` | CORE CHECKED: три положения относительно gate, split filename/header и adjacent input; exhaustive split positions не запускались. |
| `check.allocation-checks` | CORE CHECKED: checksum и mapping; pending дополнительные provider/read-error branches. |
| `init.result` | CORE CHECKED: accepted destination/temp ownership, validation и `-u` ERR. |
| `init.failure-lifecycle` | CORE CHECKED: rejected INIT, cancel/reconnect/stale и отсутствие sticky flags; pending actual temp-open ENOSPC/descriptor exhaustion. |
| `init.wire` | CORE CHECKED: split INIT, chained occurrences, ACK/ERR и zero ID. |
| `init.allocation-checks` | CORE CHECKED: wrong/stale input, ownership/cleanup; pending прочие temp-open failures. |
| `data.result` | CORE CHECKED: ordered full bytes и real temporary write failure. |
| `data.failure-lifecycle` | CORE CHECKED: partial input, stale, RLIMIT write failure и cleanup без false ACK; full native relog остаётся C014. |
| `data.wire` | CORE CHECKED: legal 1030 frame, malformed 1025 payload length, unknown ID body/ERR; exhaustive split matrix не запускалась. |
| `data.allocation-checks` | CORE CHECKED: length/ID, split/chained, disk failure, generation boundary. |
| `end.result` | CORE CHECKED: copy publication, ERR/reload independence и successful ACK с отдельными Lua/Guide failures. |
| `end.failure-lifecycle` | CORE CHECKED: failed open destination и cumulative results; pending mid-copy/fclose failures и concurrent external edits. |
| `end.wire` | CORE CHECKED: chained END order и ACK/ERR после synchronous Lua result. |
| `end.allocation-checks` | CORE CHECKED: multiple/last transfers, sticky flags и stale; actual readiness/teardown consumers C011/C014 не приняты. |
| `upload.result` | CORE CHECKED: callable production request, checksum match/mismatch и полный sender core flow. Admin menu остаётся C041. |
| `upload.failure-lifecycle` | CORE CHECKED: cancel, ERR, stale/cleanup; pending source read I/O fault и sender-specific saturated output branch. |
| `upload.wire` | CORE CHECKED: CHECK/INIT/DATA/END bytes, 256/1024 gate, ACK pacing и EOF без лишнего ACK. |
| `upload.allocation-checks` | CORE CHECKED: filename limit, cancel, checksum match и sender cleanup; actual C041 surface не проверялась. |

Export replace consent/atomic rename неприменимы к baseline transfer publication;
они не исполняются и не получают фиктивный PASS. Concurrent external edits не
защищены экспортным conflict policy и не проверялись в этом минимальном наборе.
Указанные дополнительные faults/матрицы остаются pending; расширение проверок
согласуется отдельно по AGENTS. Непроверенный actual native socket/flow runtime,
C011 readiness/handoff, C014 live adoption/teardown и C041 menu не подтверждены
этим core запуском. Build подтверждает их текущее подключение на уровне линковки,
а не выполнение полного live сценария. Принятый C009 не переоткрывается.

Только Linux; Windows/MinGW/Wine deferred. Canonical snapshot остаётся исходным
planning-only и не меняется автоматически вслед за техническим evidence.

## План дополнительного evidence — 2026-11-10

После read-only анализа трёх агентов подготовлен
[SV-C010-006](tasks/c010/SV-C010-006-targeted-validation.md): sender output
saturation и один actual native socket/login case в существующих проверках.
План не реализован и не запускался; приведённые выше PASS/pending observations
сохраняются. Новое evidence должно дополнить этот отчёт отдельным результатом.

## Граф проекта

`.graphify-venv/bin/graphify update .` завершился с exit 0:
[log](../.scratch/sv-c010/graphify-update.log). Граф обновлён; extractor сообщил
частичный разбор 78 файлов (включая legacy C), 202 файла без извлечённых symbols
и обновление community labels по hub. Это ограничения навигационного графа,
не compiler/runtime failures; точные исходники проверялись отдельно.

## Дополнение 006 выполнено — 2026-11-10

По отдельному поручению пользователя реализованы и прошли обе рекомендованные
проверки: собственная sender output queue при repeated BP и actual native
socket/login/FILE/Lua flow. [Evidence 006](sv-c010-006-evidence.md) содержит
отдельные logs, source/binary/library hashes, wire transcript и copied outcome.
Исходные observations 005 выше сохранены как история; pending на две указанные
части снят этим дополнением. Иные faults и поздние consumers не получают PASS.

# C010-006 — sender backpressure и native startup evidence

**Приёмка 2026-11-11:** пользователь принял C010 в границах Linux startup/core —
[решение](sv-stage-c010-acceptance.md). Отчёт ниже сохраняет observations и
ограничения своего запуска; прежние формулировки о незаявленной приёмке исторические.

Дата: 2026-11-10. Обе части [согласованного плана](tasks/c010/SV-C010-006-targeted-validation.md)
выполнены: **A PASS, B PASS**. Пользователь поручил «Запусти агентов отработать
по плану». Два агента реализовали и запустили scenarios, третий выполнил read-only
review; actionable defects в финальных проверках не обнаружены.

Это дополнение к [исходному evidence C010-005](sv-c010-evidence.md), не замена
истории его запусков и не пользовательская приёмка C010. Новых capability IDs нет,
canonical allocation/prerequisites/statuses не менялись.

Базовая revision: `c29c37a153053779dddb659844315abc37dbd197`. Проверялось dirty
рабочее дерево с ранее реализованным C010. В этой задаче изменены только
`tests/sv/transfer.c`, `tests/sv/first-session.c`, их existing native runner,
узкий target `src/makefile.sv` и документы. Production transfer/login/Lua
семантика и legacy/common в этом дополнении не менялись.

## A. Sender queue saturation — PASS

```sh
SV_TRANSFER_COMMAND_LOG=.scratch/sv-c010/006/sender-command.json python3 tests/sv_transfer_core.py
```

Exit 0. [Log](../.scratch/sv-c010/006/sender.log),
[compile/link/run argv, compiler и 32 source hashes](../.scratch/sv-c010/006/sender-command.json).
Существующий runner сохранил ASan/UBSan, SV/test `-Wall -Wextra -Werror`,
`ASAN_OPTIONS=detect_leaks=0`, `UBSAN_OPTIONS=halt_on_error=1`;
sanitizer diagnostics отсутствуют, leak detection не заявлена.
Исходные C010-005 logs/metadata не перезаписывались.

Новый helper `upload_backpressure()` в existing core test вызывает публичный
production `SvTransfer`, без private state mutations, fake parser/sender или
нового harness. Использован modern version `4.6.1.1.0.2`, source 2048 bytes:
1024 bytes `0x35`, затем 1024 bytes `0xca`.

| Наблюдение | Actual |
|---|---|
| READY sender после CHECK/SUM mismatch/INIT/ACK | 1 active transfer, handshake output drained |
| Наполнение собственной transfer output queue | 1587 SUM × 20 = 31740 bytes, осталось 1028 < DATA frame 1030 |
| Два `sv_transfer_pump` без drain | оба `SV_BACKPRESSURE`; queue size и summary неизменны |
| Drain с capacity меньше required | `SV_OUTPUT_TOO_SMALL`, bytes не потреблены |
| Полный drain | ровно ожидаемые SUM в исходном порядке, без partial DATA |
| Resume | DATA1030 с `0x35`; без ACK второго DATA нет; после ACK — DATA1030 с `0xca` |
| EOF/END | END на следующем pump pass без дополнительного incoming ACK |
| Final ACK | active=0, failures=0, output пуст |

Проверяется именно reservation перед sender `fread`, а не другая login queue.
Exact distinct chunks после repeated BP подтверждают отсутствие потери/повтора
source data без test-only ftell observer. Это bounded заполнение 32 KiB, не stress.
Trace: проверенная часть `obligation.transfer.upload.failure-lifecycle`, `.wire`,
`.allocation-checks`. Pending sender-specific saturated-output branch из 005 закрыт.
Kernel socket backpressure и source-read faults этой частью не подтверждаются.

## B. Native socket/login/FILE/Lua — PASS

```sh
python3 tests/sv_first_session_checks.py --case c010-transfer
```

Exit 0. Запущен только один новый mode existing managed peer, без запуска старых
success/disconnect/quit/retry modes. Focused target `sv-first-session-check`
линкует существующий test main с production `SV_OBJECTS` без `main.o`;
новый peer/build framework и production observation hooks не добавлены.
Native использует обычные production objects — sanitizer PASS этой части
не заявляется. Compile/link flags сохранены в [build log](../.scratch/sv-c010/006/native/build.log).

[Точные commands/environment, binary hash, 143 source hashes и library hashes](../.scratch/sv-c010/006/native/command.json),
[stdout](../.scratch/sv-c010/006/native/stdout.log),
[stderr](../.scratch/sv-c010/006/native/stderr.log),
[exact FILE transcript и copied outcome](../.scratch/sv-c010/006/native/file-transcript.log).
Root отдельно сверил все 143 hashes с финальными файлами и source hashes части A.

Путь: existing managed loopback peer → actual `sv_endpoint_run` → login flow/
contact socket → FILE dispatch → production startup adapter/transfer/Lua →
copied `SvEndpointOutcome` после штатного завершения из OVERVIEW.
Public fixture account/password вымышленные; реальные аккаунты не использовались.
Evidence не публикует raw credential conversation.

Полная bundled library скопирована в собственную writable temporary library,
`scpt` не symlink. Profile/shared roots также временные. Оригинальная `lib`
проверена по hashes до/после и **не изменилась**. Environment содержит SDL
dummy/software и dummy audio; actual SDL window/render/input loop исполняется,
но physical-display visual review этим запуском не заявлен.

Наблюдения одного соединения:

- SERVERDETAILS с flags/TEMP предшествует FILE. Begin result получен от actual
  startup binding, не direct test call `sv_lua_reload_begin`.
- CHECK copied Guide → exact modern MD5 SUM; INIT → ACK. Передано семь DATA,
  один full frame1030 разделён 9+1021 bytes с bounded 40 ms no-early-ACK barrier.
  До завершения FILE_END destination остаётся исходным.
- END разделён 3+1 bytes, также без early ACK/publication. После complete END
  peer получил exact ACK, destination точно совпал с 6977 transmitted bytes.
  Published SHA-256: `5c541ac87418fd537ef96cd3ff68d7f3417d2a39e3431f3b2ab69a15d5cb5ec4`.
- `_ALERT("C010 native transferred guide marker")` появился в captured stderr
  ровно один раз: опубликованный script действительно исполнился. Existing
  Lua diagnostic используется без новых hooks.
- После terminal character row отправлен PING; только после точного pong
  подан Q через existing SDL route. Нет произвольного readiness sleep,
  character selection, PLAY или live handoff.
- Copied outcome: generation `676613686`, phase OVERVIEW=2, Lua begin OK=0;
  accepted_init=1, completed_end=1, reloads=1, active=0; failures,
  runtime_failures и guide_failures равны 0. Completion того же generation:
  transfer_success, reload_attempted, runtime OK, guide_rebuilt и Guide status OK.
  Peer наблюдает EOF и штатный bounded exit.

FILE reply reader допускает отдельно только protocol KEEPALIVE и отвергает
неожиданные типы. Partial socket writes плюс no-early-ACK observations не
объявляются exhaustive матрицей всех TCP read boundaries.
Process timeout 30 s, socket/accept deadlines 8 s. Metadata getter values после
возврата `sv_endpoint_run` не читаются: application owner уже уничтожен.

Первый запуск остановился при создании AF_INET socket из-за sandbox network
restriction, до production contact. Та же локальная проверка затем выполнена
с sandbox escalation. Initial startup также запускал supplementary Chronicle;
первый успешный запуск прочитал публичные Chronicle sources. В финальном
окружении все proxy variables направлены на закрытый `127.0.0.1:1`, NO_PROXY
разрешает loopback: supplementary Chronicle возвращает unavailable/bytes=0,
actual contact socket работает локально. Production-код для этого не менялся.
Первый sandbox failure, pre-proxy run и prior proxy-isolated run сохранены в
отдельных подкаталогах evidence; финальный запуск сделан после стабилизации runner.
Исторические результаты не переписаны как успешные.

## Закрытые риски и оставшиеся границы

006 завершена: sender-specific bounded output saturation и actual native
startup socket/FILE/Lua flow подтверждены. Это дополнительно покрывает actual
startup portions `transfer.check/init/data/end.*` и integration trace Lua C010-001.
Не повышать целые canonical obligations автоматически по этому дополнению.

Прежние pending EACCES/read-prefix faults, temp-open ENOSPC, mid-copy/fclose
faults, concurrent external edits, exhaustive splits и kernel short writes
не включались и не получают PASS. C011 readiness/handoff, C014 live adoption/
relog/teardown, C041 menu остаются своими этапами. Только Linux;
Windows/MinGW/Wine отложены. Полная приёмка C010 требует фактического решения
пользователя и не подразумевается выполнением двух проверок.

## Финальная сверка

`git diff --check`, проверка source hashes обоих запусков, canonical snapshot/
hashes и локальных Markdown links прошли. `.graphify-venv/bin/graphify update .`
завершился с exit 0: [log](../.scratch/sv-c010/006/graphify-update.log).
Граф содержит 22718 nodes; HTML автоматически агрегирован по communities.
Сохранились extractor warnings частичного разбора 78 файлов (включая legacy C)
и отсутствия symbols в 202 файлах; это ограничения навигации, не compile/runtime
ошибки этих checks. Canonical данные не изменены, коммит этой работой не создавался.

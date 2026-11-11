# SV-C010-006 — sender backpressure и actual native startup flow

Статус: accepted — Linux startup/core, 2026-11-11.
Дата: 2026-11-10.
Labels: enhancement, ready-for-human
Owned IDs: 0 новых IDs; дополнительное evidence существующих C010 outcomes.
Зависимости: implemented C010-001–004 и исходное evidence C010-005.

## Цель и граница

После [исходного core evidence](../../sv-c010-evidence.md) рекомендовано точечно
проверить два оставшихся риска: резервирование output именно sender перед чтением
source и исполнение текущего native socket/login пути. План подготовлен по
read-only анализу трёх агентов; исходники сверены после Graphify query.
Первоначальный запрос пользователя касался подготовки плана. Последующим
поручением реализация и запуск разрешены (см. Comments); выполнение проверок
не означает пользовательской приёмки C010.

Завершённая 005 сохраняет статус и прежние результаты; эта задача дополняет их.
Не создаются новые peer/harness/framework, production test hooks или собственные
версии parser/sender/Lua. Только Linux; один modern protocol version достаточен:
old/new gates уже подтверждены существующим core check. Windows отложен.

## A. Upload при недостатке sender output capacity

**Изменяемые файлы при реализации:** одна добавка в `tests/sv/transfer.c`;
использовать существующий `tests/sv_transfer_core.py`, без нового runner.
Production-код менять только при обнаружении воспроизводимого дефекта.

**Почему текущей проверки недостаточно:** `output_retry()` заполняет login output.
`sv_login_tick()` сначала вызывает `sv_login_receive(NULL,0)` и `drain_transfer`,
поэтому может вернуть backpressure до sender pump. Новый scenario должен прямо
вызвать публичный production `sv_transfer_pump` у READY sender с заполненным
собственным `SvTransfer.output`.

1. Создать настоящий source во временном root: 2048 bytes, первый chunk 1024
   bytes `0x35`, второй — `0xca`. Создать owner с отдельным generation и version
   `4.6.1.1.0.2`; Lua может быть NULL, исходящий sender его не исполняет.
2. Через production `sv_transfer_upload → CHECK → mismatch SUM → INIT → ACK`
   подготовить READY sender. Полностью дренировать handshake output; pump пока
   не вызывать. Не выставлять private READY/state и не подменять FILE stream.
3. Настоящими CHECK отсутствующего unrelated filename наполнить transfer output
   SUM-ответами: modern SUM занимает 20 bytes, без allocation transfer record.
   Использовать публичные capacity/output-size. При capacity 32768 достаточно
   1587 SUM: занято 31740, осталось 1028 — на 2 bytes меньше DATA frame 1030.
   Проверять фактические размеры, а не зависеть только от этого вычисления.
4. Дважды вызвать pump: оба результата `SV_BACKPRESSURE`; output size, active
   count и failure counters неизменны. Очередь содержит только ожидаемые SUM,
   без частичного DATA. Не пытаться доказать эту ветку заполнением другого buffer.
5. `take_output` с недостаточной capacity возвращает `SV_OUTPUT_TOO_SMALL` и
   required size, не потребляя очередь. Полный drain даёт точные N SUM в порядке.
6. Следующий pump выдаёт ровно первый DATA1030 с `0x35`. Pump без ACK не выдаёт
   следующего блока. ACK → pump выдаёт ровно второй DATA с `0xca`.
   Exact payload/order и отсутствие лишних bytes доказывают, что repeated BP
   не потерял и не продублировал source data; private ftell hook не требуется.
7. ACK второго DATA → EOF pass → следующий pump выдаёт END без нового incoming
   packet. Final ACK освобождает sender: active=0, failures=0, output пуст.

Это конечное заполнение bounded 32 KiB очереди, не stress/soak. Не добавлять новую
версионную матрицу. Наблюдать queue sizes, repeated BP, exact DATA sequence и
cleanup в existing core result. Не заявлять kernel socket backpressure, actual
native flow или source-read fault PASS по этому scenario.

**Команда после реализации:**

```sh
SV_TRANSFER_COMMAND_LOG=.scratch/sv-c010/transfer-core-command.json python3 tests/sv_transfer_core.py
```

Использовать существующие ASan/UBSan flags и policy; расширять набор за пределы
этого scenario не требуется. Trace: проверенная часть
`obligation.transfer.upload.failure-lifecycle`, `.wire`, `.allocation-checks`.

## B. Actual native socket/login flow

**Основа:** `tests/sv/first-session.c` уже запускает настоящий `sv_endpoint_run`,
управляемый loopback peer и передаёт `.outcome=&outcome`. Добавить один mode
`c010-transfer`; использовать существующий `tests/sv_first_session_checks.py`
с focused selector, запускающим только этот mode.

Не брать старый CLI `sv_login_live_checks.py` за источник copied outcome:
CLI launcher его сейчас не экспортирует. Не добавлять ради проверки getters
или diagnostic API в production.

### Подготовка и build

- Отдельные temporary profile, shared SDL3 user root и writable library.
  Library — настоящая копия bundled ресурсов; `scpt` и transfer destinations
  обязательно копируются, не symlink на repository/user assets. Не передавать
  repository `lib` как writable transfer destination.
- `TOMENET_SDL3_USER_PATH` и `TOMENET_PATH` согласовать с fixture roots,
  чтобы внешний shared overlay не затенил проверяемый script. Базовые ресурсы
  репозитория читать только для подготовки копии; сохранить hashes до/после.
- Dummy/software SDL, dummy audio, отключённый sound в fixture config;
  loopback endpoint и фиктивная учётная запись, без реального сервера/аккаунта.
- Existing runner содержит устаревающий ручной SOURCES list. Для focused mode
  предусмотреть небольшой target в `src/makefile.sv`: существующий
  `tests/sv/first-session.c` main + production `SV_OBJECTS` без `main.o`.
  Использовать те же compile/link flags и текущий closure Lua/transfer/UI,
  не создавать второй build framework и не запускать старые modes пакетом.
  Native result с обычными production objects не объявлять sanitizer result;
  ASan/UBSan этой задачи относятся к части A, если native objects не собраны с ними.

### Сценарий и assertions

1. Пройти existing contact/verify/setup через реальный socket и login flow.
   Допустимы публичные fixture options account/password с вымышленными значениями,
   чтобы не проверять повторно ввод credentials. `.character=NULL` оставляет
   сценарий на OVERVIEW; PLAY и live handoff в эту задачу не входят.
2. Peer отправляет полный SERVERDETAILS с известными непустыми flags/TEMP.
   Только после него — FILE conversation: adapter обязан begin из actual decoded
   flags до INIT/END. Не вызывать Lua begin из peer/test напрямую.
3. Передать CHECK и проверить exact modern SUM исходного copied script. Затем INIT и DATA валидного
   `scpt/guide.lua` из собственной library, END. Script содержит полный исходный
   Guide с добавленной строкой `_ALERT("C010 native transferred guide marker")`,
   сохраняя валидность metadata. До передачи copied Guide marker не содержит.
   Existing `runtime.c:alert_message` направляет _ALERT в native diagnostic/stderr,
   не меняя success status; новых observer hooks не требуется.
   Хотя бы один DATA с payload 1024 (frame 1030) передать частями; neighboring
   frames/chained bytes сохраняют границы. Peer ждёт предусмотренный ACK каждого
   INIT/DATA и последний ACK/ERR, не исполняет клиентское поведение сам.
4. Последний END разделить 3+1 bytes. До его полного получения copied destination
   содержит прежние bytes, после final ACK — точно transmitted content.
   Сохранить exact FILE transcript и проверить published file length/content hash
   в isolated library. Для partial DATA наблюдать отсутствие раннего ACK; только
   итоговый файл после END не доказывает partial handling сам по себе.
5. Отправить character list с terminal row, затем existing PING barrier.
   Дождаться exact pong и только после него inject Q через SDL event по existing
   quit route: arbitrary `SDL_Delay(150)` не является readiness condition.
   Получение pong после списка подтверждает progress до OVERVIEW.
   Не проверять C011 readiness,
   C014 live adoption или C041 admin menu. Не завершать процесс сразу после END:
   final reply должен быть получен peer до native quit/teardown.
6. После `sv_endpoint_run` проверить copied `outcome.lua_begin.status == OK`,
   generation согласован, `transfers.accepted_init == 1`, `completed_end == 1`,
   `reloads == 1`, `active == 0`, failures/runtime_failures/guide_failures равны 0;
   copied completion: transfer_success, reload_attempted, runtime OK,
   guide_rebuilt/guide_status OK. Итог — OVERVIEW, без character selection/PLAY.
   Проверить фактические quit/result semantics существующего fixture.
7. Runner проверяет в captured stderr точный marker `_ALERT` ровно один раз:
   это подтверждает исполнение опубликованного script, а не только успешный
   reload старого содержимого. Одних exit 0 и существования файла недостаточно.
   Сопоставить wire ACK, file
   bytes, begin/reload results и copied outcome; inspection не вызывает новый
   reload. Значения Guide getters после уничтожения application owner недоступны:
   не обещать такие assertions через dangling owner.

Ограничить один case внешним timeout до 30 s, peer reads/accept — bounded deadline
8 s либо эквивалентом existing fixture, event wait — собственным deadline.
Не использовать долгий sleep как синхронизацию и не делать автоматический
повтор сценария до случайного PASS. При таймауте сохранить последний step и output.

**Команда focused selector/build target** (реализована в этой задаче):

```sh
python3 tests/sv_first_session_checks.py --case c010-transfer
```

Runner готовит roots/environment и сохраняет commands/env, binary hash,
stdout/stderr, FILE transcript, file hashes и copied outcome. Секреты и raw
credential conversation в evidence не публиковать.
Trace: проверенные actual startup portions `transfer.check/init/data/end.*`
и Lua integration C010-001, без новых IDs и без автоматического полного PASS.

## Результат и ограничения

Готовность 006: обе части реализованы и прошли выбранный production-путь,
отчёт связывает observations с двумя рисками, команды/dirty revision/hashes и
pending ограничения записаны в дополнение к `sv-c010-evidence.md`.
Снимать pending только с sender-specific output saturation и выполненного
native startup flow. Полную приёмку этапа или поздних consumers не объявлять.

EACCES/read-prefix faults, temp-open ENOSPC, midcopy/fclose faults, concurrent
edits, exhaustive splits, kernel short writes и live consumers остаются прежними
pending. Они не добавляются скрыто к плану двух проверок. Решения о legacy/common
refactor, новом harness, extra suites или расширении scope этой задачей не приняты.

## Comments

### 2026-11-10 — выполнение плана разрешено

Пользователь поручил: «Запусти агентов отработать по плану». Части A/B
реализуются и проверяются в ранее описанных границах. Исходный planning-only
текст выше сохраняет историю; новое поручение разрешает реализацию и запуск,
но не означает пользовательскую приёмку C010.

### 2026-11-10 — результат выполнения

Части A и B реализованы в существующих checks и прошли. A: 1587 SUM,
remaining1028 < DATA1030, repeated BP и точные два chunks после drain,
ACK/EOF/final cleanup; ASan/UBSan PASS. B: actual native loopback FILE transfer,
Guide publication6977bytes, marker1, copied begin/init/end/reload/Guide success
и OVERVIEW closure; ordinary production objects. Sandbox loopback permission
потребовал escalation; supplementary Chronicle изолирован proxy environment.
Read-only review не выявил actionable defects. Исходные evidence/logs сохранены.

[Дополнительное evidence с commands, hashes и pending limits](../../sv-c010-006-evidence.md).
Production/legacy/common семантика этой задачей не менялась. 006 завершена,
пользовательская приёмка C010 не заявлена.

### 2026-11-11 — приёмка пользователя

Пользователь подтвердил: «Этап принимаю». Задача принята в границах
[решения о приёмке C010](../../sv-stage-c010-acceptance.md). История реализации,
evidence и непроверенные условия сохраняются; canonical statuses не меняются.

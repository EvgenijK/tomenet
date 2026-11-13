# SV-C005-003 — raw-key fallback после разрешения команды

Статус: **accepted**, 2026-11-09, по решению пользователя.
Labels: enhancement
Owned IDs: `capability.input.raw-key` (existing C005 allocation).
Зависимости: результат production keymap из [SV-C005-001](SV-C005-001-preference-keymaps.md)
и существующие SV app/protocol/input interfaces. Поздние C011/C014/C053 не
блокируют готовность собственного core C005.

## Проблема и baseline

Serializer уже существует: `src/client/sv/protocol/protocol.c:sv_protocol_raw_key`
пишет ровно два байта `{PKT_RAW_KEY, key}`, где `PKT_RAW_KEY=156` (`0x9c`).
`sv_app_raw_key` проверяет generation, active/context/pause/request и вызывает
serializer. Но `sv_app_physical` без pending server request направляет любой
resolved byte непосредственно в raw-key seam: effective keymap и различение
игровой команды, незавершённого prefix, no-op и настоящего raw fallback отсутствуют.
Текущий пример `'^' → {156,'^'}` в `tests/sv/requests.c` подтверждает ранний
serializer seam, но не baseline command input: `^` начинает control-prefix.

Нормативные источники `source.reconciliation.input.raw-key` и
`source.protocol.items-send-raw-key` закреплены в manifest на revisions
`a3d99eeb4e2247ba0c23a17e57fa502ee931f6e3` и
`bbe7417e21d20470a7aeb075aa82650608f6e4ae`. В обоих `Send_raw_key` одинаков:
`Packet_printf(&wbuf, "%c%c", PKT_RAW_KEY, key)`, без server-version ветвей,
отсутствующих полей, slot/quantity или многобайтового текста.

Дополнительные baseline anchors текущего дерева: `c-util.c:request_command`
(3583), `original_commands` (1848), `roguelike_commands` (1729),
`c-cmd.c:process_command` (274), `cmd_raw_key` (10585),
`nclient.c:Send_raw_key` (8107), `common/pack.h:PKT_RAW_KEY` (198).
Default `process_command` отправляет raw key после command translation;
Escape, `-`, CR не отправляют его. Normal mapping превращает LF в CR.
Пробел вызывает `cmd_clear_actions`, поэтому исключение пробела из raw path
не означает готовность этого игрового действия или превращение его в no-op.
Клавиша, сохранившая своё значение в keymap, ещё может быть известной командой
`process_command`; identity mapping не является критерием raw fallback.
Пример context-sensitive ветви — `]`: admin/master action либо raw fallback
для обычного игрока. Не заменять отсутствующие будущие command owners отправкой
всех ещё не реализованных клавиш на сервер.

## Реализация

1. В SV связать production input resolution C005 с существующим raw sender.
   Различать незавершённый prefix, no-op, command/direction result и unhandled
   gameplay result. Только последний идёт в `sv_app_raw_key` и существующий
   serializer. Prefix/backslash/control translation выполняет один owner
   SV-C005-001; повторного keymap, parser или тестовой копии dispatcher не делать.
   Сохранить byte выбранной команды baseline, а не отправлять исходный physical
   trigger, UTF-8 text, scan code или PRF syntax.
2. Явно сохранить границу известных semantic actions и raw fallback. C005
   предоставляет production seam для поздних command/context owners; не
   переносить сюда все игровые команды, master UI, store routing или protocol
   receiver. No-op исключаются до отправки. Direct raw-key API сохраняет свои
   проверки; не расширять его фильтр предположением, что пробел или LF являются
   самостоятельными baseline no-op. Для bypass сверить именно baseline route.
3. Сохранить atomic output: при свободных >=2 bytes ставится один пакет, иначе
   `SV_BACKPRESSURE` без частичного пакета и ложного успеха. Не добавлять hidden
   retry, способный повторить команду, либо безлимитную очередь. Ошибки/closed/
   stale/busy сохраняют явный результат и не создают выходных bytes.
4. Сохранить отсутствие отправки в pending server-key request, pause и после
   teardown; key request остаётся отдельным `PKT_REQUEST_KEY` route. Main thread
   владеет изменяемым input state. Focus/resize/redraw/interleaved network update
   не воспроизводят уже разрешённый raw result. Reset/новое generation не
   возобновляют незавершённый ввод прошлой сессии.
5. Адаптировать ранний production physical/core caller и его существующий
   небольшой набор проверок к реальному C005 resolution. Исправить устаревший
   `'^' → raw` oracle, не меняя семантику baseline ради старого теста. Legacy и
   общие файлы менять только при доказанной необходимости; реализацию держать SV.

## Минимальная Linux проверка

Один компактный core набор через настоящий production input/app path с чтением
`sv_app_take_output`, а не успешный fake callback:

- По одному действительно unhandled результату normal/roguelike resolution и
  existing physical binding: ровно `{0x9c,key}`, один раз. Identity-mapped
  известная команда не попадает в raw fallback. Prefix/bypass результат
  проверять вместе с SV-C005-001, без второго набора тех же cases.
- Escape/CR/`-`, ожидающий prefix и known action не создают raw packet;
  normal LF идёт через CR semantics. Пробел не становится raw key; его
  конкретный clear-actions outcome принимает владелец игрового действия.
- Не хватает двух bytes output, повторный вызов после освобождения capacity,
  pending prompt/pause, stale generation и closed state: наблюдаемые return
  values и точное отсутствие частичных/лишних bytes.
- Interleaved update, focus/resize и production reset/teardown не повторяют
  packet и не отправляют старый ввод. Использовать существующий lifecycle
  harness, минимально дополнив его только при отсутствии нужного core case.

Serializer не содержит version branches: это фиксируется сверкой обоих pinned
owners, без искусственной server-version matrix. Windows/MinGW/Wine deferred
по [правилу платформ](../../agents/platform-validation.md). Расширение проверок
сверх необходимого core минимума предварительно согласовать с пользователем.

## Готовность и последующие владельцы

В отчёте C005 указать evidence для всех существующих obligations:
`obligation.input.raw-key.result`, `.lifecycle`, `.allocation-checks`.
Canonical conditions/IDs из [native-coverage](../../capabilities/native-coverage.json)
не менять: source/version/build semantics сохраняются, текущая обязательная
платформа — Linux. Заявлять готовность только при фактическом raw dispatch из
production C005 resolution; прежний ранний serializer PASS недостаточен.

C011/C014 принимают actual live-session command routing, context-sensitive
ветви и handoff/teardown на реальном session path; C053 принимает macro-derived
execution этого же primitive, включая runtime timing/control markers. C027
владеет отдельной `capability.store.raw-key`, server-owned store результатом и
своими wire/input obligations. Эти caller outcomes не принимаются C005.
`nserver.c:Receive_raw_key` подтверждает server authority: сообщение/действие
зависит от состояния игрока, здания и energy; постановка packet не означает
успех серверного действия. C005 не создаёт новый incoming raw-key decoder.

В evidence явно перечислить эти поздние routes и их владельцев, сохранив
полные obligations без фиктивного PASS. Отсутствие поздних callers не создаёт
backward gate C005; их проверка обязательна в своих этапах и не переоткрывает
принятый core. Изменения input keymap и preference loading не переоткрывают C004.

Нормативный контекст: [CONTEXT](../../../CONTEXT.md),
[ADR-0002](../../adr/0002-main-thread-state-ownership.md),
[ADR-0003](../../adr/0003-explicit-module-connections.md),
[ADR-0005](../../adr/0005-session-scoped-work.md),
[ADR-0006](../../adr/0006-errors-and-overload.md).

## Результат реализации — 2026-11-09

Production `sv_app_physical` теперь разрешает ввод через `SvKeymap` и отделяет
no-op, pending prefix, command/direction и raw fallback. Baseline command
classification находится в SV `app.c`; известные команды сохраняются в одном
generation-scoped результате `sv_app_take_command`, пока поздний исполнитель
не заберёт его. Занятый результат возвращает `SV_BACKPRESSURE`. Контекстная
ветвь `]` использует явный `sv_app_set_master_commands` (обычный игрок по
умолчанию), без переноса master UI. Raw packet сериализуется прежним модулем;
отказ output capacity восстанавливает prefix state для явного retry.

Literal backquote нормализуется в Escape после physical binding и перед
request/gameplay dispatch. LF сохраняется допустимым resolved raw byte для
backslash bypass; normal keymap превращает его в CR, roguelike — в tunnel
command, как baseline. Пробел остаётся local command result, не no-op/raw.
Preference owner подключён через `sv_app_keymap_preferences` с явным cursor и
rebuild flag; app не хранит borrowed preference pointer.

App также предоставляет generation-scoped quantity interaction из 002:
begin/text/edit/take/view, отдельный `SV_CONTEXT_QUANTITY`, normal-only physical
field bindings, сохранение parent context при вложенном server key request.
Active/completed quantity блокирует direct raw dispatch; stale request sequence
не используется как quantity input. Native adapter передаёт многобайтовый draft
в quantity editor, special/edit events — в соответствующий semantic путь.
Незабранный результат блокирует последующий ввод. Session reset очищает quantity,
prefix и pending command. Native Escape завершает незавершённый prefix.

`SvNativeInput.last_result` отделяет классификацию SDL event от успешного
выполнения; владелец обязан сохранить/повторить event при BACKPRESSURE/BUSY.
Скрытая очередь/retry не добавлены. Полный live sender/executor и macro runtime,
включая priority macro overrides поверх native edit keys, остаются у C014/C053.

Проверка: `ASAN_OPTIONS=detect_leaks=0 python3 tests/sv_request_checks.py` — PASS
на Linux с ASan/UBSan: 372 существующих request split/HP/cancel/abort cases и
один компактный C005 app/native набор. Проверены effective normal/roguelike
command/direction через app preference seam, истинный raw результат и bytes,
master branch, reserved/no-op/space, prefix rollback при полном output,
native result/retry, interleaved HP/focus/resize, quantity text/edit/physical
macro filtering, literal backquote, вложенный request, одноразовый take и reset.
Log: `.scratch/sv-c005/app-checks.log`. LeakSanitizer без `detect_leaks=0` не
работает в данном sandbox (`LeakSanitizer does not work under ptrace`), failure
сохранён в `.scratch/sv-c005/app-checks-lsan.log`; это не PASS проверки утечек.

`obligation.input.raw-key.result` подтверждена resolved production output;
`.lifecycle` — собственным reset/focus/resize/interleaved update;
`.allocation-checks` — собственными normal/roguelike/physical/input branches и
явной disposition late routes выше. Actual session callers C011/C014, macro
runtime C053 и store C027 по-прежнему требуют собственных integration outcomes;
их приёмка здесь не заявлена.

Задача принята пользователем в составе C005 2026-11-09: [решение](../../sv-stage-c005-acceptance.md).

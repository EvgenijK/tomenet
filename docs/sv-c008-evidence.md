# C008 — Linux implementation evidence

Дата: 2026-11-09. Статус: **core реализован и проверен на Linux**; late caller
integration evidence остаётся pending. Пользователь поручил
реализацию и затем [принял C008 в текущих границах core](sv-stage-c008-acceptance.md).
Поздние интеграции не блокируют завершение C008. Полный live-runtime PASS
и повышение canonical registry statuses не заявлены.

[Спецификация](sv-stage-c008-spec.md), [задачи](tasks/c008/README.md),
[source contract](tasks/c008/source-contract.md),
[точный planning snapshot](tasks/c008/coverage.json).
Шесть canonical outcomes и 18 obligations сохраняют allocation и исходные
conditions; текущая проверяемая платформа — [Linux](agents/platform-validation.md).
Windows/MinGW/Wine отложены. Snapshot остаётся planning-only; этот отчёт отдельно
фиксирует фактические результаты и ограничения.

## Что реализовано и что проверяется

1. **Packets/recovery, задача 001.** Существующие payload-free END/KEYPRESS,
   split CONFIRM и atomic unknown recovery сохранены. Successful recovery
   дополнительно публикует baseline diagnostic через `SvChange`/`SvMessage`,
   session feed и once-per-message effect delivery. До успешного резервирования
   всех 11 reply bytes diagnostic и очистка receive batch не выполняются.
   Gameplay END и отдельный login END lifecycle не объединялись.
2. **Pause/flush, задача 002.** Добавлен production native adapter
   `ui/native-controls.[ch]`: coherent draw → `SDL_RenderPresent`, свежий native
   input boundary и неблокирующий pacing. App delivery использует явный control
   sink, а не только inspection `pause_sequence`/`flush_due_ms`. После review
   сохранены отдельные acknowledgement повторных PAUSE и bounded outer budget
   через pacing yields; partial tail завершает batch. Финальный core run прошёл.
3. **Confirmation, задача 003.** Existing production decoder, app/input bounded
   FIFO, waiting-owner sequence и generation guards оказались достаточны для
   core; добавлена минимальная проверка недостающих ветвей. Настоящий macro
   WAIT/XWAIT consumer C053 этим не реализован.

Реализация остаётся SV-local. Изменения `tests/sv/diagnostics/launcher.c`
подключают тот же native control adapter к существующему synthetic launcher.
Это интеграция production adapter с diagnostic caller; она не создаёт live
session handoff C011/C014 или полный ingress карты C013.

## Выполненные команды и результаты

| Проверка | Фактический результат | Воспроизведение и evidence |
|---|---|---|
| 001 packet/core | PASS, Clang ASan/UBSan | Полные compile/run argv и environment в [network-packets-command.json](../.scratch/sv-c008/network-packets-command.json); [log](../.scratch/sv-c008/network-packets.log) |
| Existing architecture regression | PASS, ASan/UBSan с `detect_leaks=0` | `ASAN_OPTIONS=detect_leaks=0 python3 tests/sv_arch_checks.py`; [log](../.scratch/sv-c008/architecture.log) |
| 003 confirmation + existing contact controls | PASS, Clang ASan/UBSan | Compile argv в [confirmation-command.json](../.scratch/sv-c008/confirmation-command.json), запуск `ASAN_OPTIONS=detect_leaks=0 .scratch/sv-c008/contact-control`; [log](../.scratch/sv-c008/confirmation.log) |
| Linux native build | PASS после финальных fixes, `-Wall -Wextra -Werror` для SV | [build.log](../.scratch/sv-c008/build.log); exact top-level invocation ниже |
| Bounded native synthetic smoke | PASS после финальных fixes | [native-smoke.log](../.scratch/sv-c008/native-smoke.log): SDL dummy video/software renderer, одно системное окно, два submitted frames, active synthetic session; exact argv ниже |
| 002 native input/present/pacing | PASS, Clang ASan/UBSan и настоящий SDL software renderer | Compile argv в [network-controls-command.json](../.scratch/sv-c008/network-controls-command.json); [log](../.scratch/sv-c008/network-controls.log) |

Exact build и smoke commands из корня проекта:

```sh
make -C src -f makefile.sv -j4 tomenet-sv > .scratch/sv-c008/build.log 2>&1
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software src/tomenet-sv --synthetic --profile-root /tmp/sv-c008-native-smoke --library /home/svechnik/Projects/tomenet_modern_client/lib --fixture-window 1024x768 --frames 2 > .scratch/sv-c008/native-smoke.log 2>&1
```

Synthetic profile должен быть изолированным и разрешённым для synthetic mode;
production profile и сетевой аккаунт этими командами не используются.

В первой попытке existing architecture runner LeakSanitizer завершился из-за
ограничения ptrace в среде. Повтор с `ASAN_OPTIONS=detect_leaks=0` прошёл;
AddressSanitizer и UBSan остались включены. Это не подтверждает leak checking.
Existing output-capacity case минимально адаптирован: после каждой recovery
потребляется новый mandatory diagnostic event, чтобы прежний сценарий проверял
wire-output capacity, а не раньше достигал лимита message queue.

001 проверяет actual input/output через production protocol/app: END/KEYPRESS
не создают replies или input, flags sentinel остаётся следующим полным пакетом;
split CONFIRM не публикует partial effect и сохраняет byte 255; unknown после
KEYPRESS сохраняет previous type 147 и не меняет input/output при свободных
10 bytes. После drain recovery даёт ровно
`211,0,0,0,0,0,0,0,147,111,0` и diagnostic ровно один раз. Остаток receive batch
намеренно отбрасывается по baseline. Повторный step не повторяет сообщение или
ответ; close/open отвергает старый generation.

003 проверяет unsolicited confirmation до регистрации wait, split bytes 255/0,
FIFO order, busy/wrong owner, end/re-register, отсутствие replay при step/frame,
close/open generation и bounded overflow. 32 events принимаются, 33-е завершает
сессию с `SV_EVENT_OVERFLOW`; silent loss отсутствует. Это delivery/ownership
evidence, а не выполнение macro action или ожидания.

## Disposition всех 18 obligations

`CORE CHECKED` ниже означает результат перечисленных production core checks,
а не full live-runtime certification. Точные тексты obligations находятся в
неизменённом snapshot.

| Exact obligation ID | Текущий результат и оставшаяся граница |
|---|---|
| `obligation.network.unknown-packet.result` | CORE CHECKED: exact 11 bytes, previous type, diagnostic и intentional receive-batch discard. Full baseline receive-table integration с поздними packet owners не подтверждена. |
| `obligation.network.unknown-packet.lifecycle` | CORE CHECKED: однократная публикация, повторный step, close/open stale generation. Полный live nested UI/focus/resize transport flow ещё не подтверждён. |
| `obligation.network.unknown-packet.allocation-checks` | CORE CHECKED: capacity 10 → no partial reply/diagnostic/discard; drain → ровно одна recovery. Discard sentinel здесь нормативен; полный поздний поток не сертифицирован. |
| `obligation.network.keypress-stub.result` | CORE CHECKED: ровно один payload-free byte, NOOP, никаких invented input/replies. |
| `obligation.network.keypress-stub.lifecycle` | CORE CHECKED: sequential app ingress и generation guards; real live nested/focus/resize flow остаётся непроверенным. |
| `obligation.network.keypress-stub.allocation-checks` | CORE CHECKED: END/KEYPRESS + flags sentinel не теряет соседний packet; invented ack отсутствует. Полный late session caller evidence pending. |
| `obligation.network.pause.result` | CORE CHECKED: actual native presentation, discard accumulated input, fresh acknowledgement и adjacent network update. Реальная deferred caller queue отсутствует до своих late owners. |
| `obligation.network.pause.lifecycle` | CORE CHECKED: native epoch/request/quantity routes, repeated pause с отдельным ack и generation teardown; real live-session/nested caller lifecycle не подтверждён. |
| `obligation.network.pause.allocation-checks` | CORE CHECKED: ordered control submission, stale/gesture-tail rejection, adjacent network updates. Реальный baseline qbuf drain не заявлен выполненным. |
| `obligation.network.flush.result` | CORE CHECKED: actual SDL software Present независимо от disable option; 1 ms deadline и thin count/reset. Live game frame submission не подтверждён. |
| `obligation.network.flush.lifecycle` | CORE CHECKED: deadline/control generation reset и ordered submission; actual live lifecycle C011/C014 остаётся поздним. |
| `obligation.network.flush.allocation-checks` | CORE CHECKED: packet order, обязательный submission, pacing 10/11 и partial-tail batch reset; full live renderer/network caller integration pending. |
| `obligation.network.confirm.result` | CORE CHECKED: unsigned command bytes FIFO к зарегистрированному owner. Full outcome PENDING: реальный waiting macro C053 не исполнялся. |
| `obligation.network.confirm.lifecycle` | CORE CHECKED: wrong owner/generation, end/re-register, teardown и no replay. Real macro wait completion/cancel lifecycle pending C053. |
| `obligation.network.confirm.allocation-checks` | CORE CHECKED: split packet, ordered confirmations/sentinel, no invented ack, overflow явно завершает session. Actual nested macro caller evidence pending. |
| `obligation.network.end-marker.result` | CORE CHECKED: ровно один byte NOOP без invented frame counter; отдельный login END contract сохранён. |
| `obligation.network.end-marker.lifecycle` | CORE CHECKED: sequential ingress, повторные steps и session generation; full live focus/resize/nested flow ещё не подтверждён. |
| `obligation.network.end-marker.allocation-checks` | CORE CHECKED: соседний valid sentinel сохраняется, invented reply отсутствует. Full late session stream evidence pending. |

## Границы результата

- Native software adapter с реальным `SDL_RenderPresent` проверяет submission
  собственных coherent views; synthetic launcher smoke проверяет запуск и
  привязку adapter. Он не подтверждает control packets в настоящей сетевой
  игровой сессии или полный renderer/provider matrix.
- Текущий gameplay dispatcher не содержит полной baseline receive table.
  Known late packets должны получить настоящие decoders/consumers своих этапов.
  Отсутствующий decoder не доказывает unknown packet от сервера; длины не
  угадываются и fake packet consumers не вводятся.
- Полные preparation/handoff/map/lifecycle flows остаются C011/C013/C014.
  Реальная очередь deferred incoming caller packets из baseline `Flush_queue`
  не воспроизведена тестовой копией. Outgoing socket bytes не являются этой
  очередью; её интеграция остаётся отдельным неподтверждённым caller result.
- Реальный C053 macro WAIT/XWAIT продолжает получать обязательство подключить
  existing confirmation owner. API queue success не принимает macro outcome и
  не переносит canonical `network.confirm` allocation из C008.
- C008 core принят пользователем; B/C005/C007 не переоткрываются. Canonical
  native-runtime registration остаётся отдельным workflow. Самостоятельные
  интеграции C011-002/C013-001/C014-003/C053-001 обязательны своим owners,
  не блокируют и не переоткрывают C008 автоматически. Непроверенные flows
  остаются непроверенными; backlinks evidence служат трассировке.

## Финальная сверка

База Git `46b251d6405f3f1df4a488204a1835b26cfde661` плюс существующее dirty
дерево; пользовательские изменения сохранены. После интеграции повторно прошли
contact-control compile/run, Linux build и bounded native smoke. Source/binary
hashes — [after-sha256.json](../.scratch/sv-c008/after-sha256.json). Canonical rows,
их hashes, unique ownership и локальные ссылки проверены отдельно; registry
не менялся. Задача 004 завершила технический отчёт и принята пользователем
вместе с C008/001–003 в текущих границах core.

Обязательная команда `.graphify-venv/bin/graphify update .` завершилась успешно:
22352 nodes, 59437 edges, 923 communities;
[лог](../.scratch/sv-c008/graphify-update.log). CLI обновил code graph, сохранив
документальные semantic changes для отдельного `/graphify --update`. Extractor
предупредил о partial extraction 78 файлов и отсутствии symbols у 198 файлов;
parallel extraction недоступен в sandbox, использован sequential fallback.
Для графа больше 5000 nodes CLI создал агрегированное HTML-представление.
Это навигационный граф, не evidence корректности реализации.

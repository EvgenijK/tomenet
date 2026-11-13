# SV-C010-004 — requested/admin upload и sender lifecycle

Статус: accepted — Linux startup/core, 2026-11-11.
Labels: enhancement, ready-for-agent
Owned IDs: `capability.transfer.upload`.
Зависимости реализации: C003 resource-overlay/transient-owner; wire 002 и общий transfer owner 003.

## Основа и работа

Нормативны [план](README.md), [planning snapshot](coverage.json),
`cmd_script_upload` (`src/client/c-cmd.c:10369`), `remote_update`,
`check_return`/`check_return_new`, `do_xfers`, `local_file_ack/err`, `kill_xfers`
в `src/common/files.c` и `Send_file_*` в `src/client/nclient.c`.
Сохраняются все четыре `obligation.transfer.upload.*`, исходные build/version
branches и source/destination mapping. Наличие B ACK/ERR framing не доказывает
полный sender flow.

1. Реализовать SV-local production caller для requested/admin upload: filename
   input с исходным лимитом 30 bytes и cancel без отправки; не создавать весь
   поздний admin menu C041. При отсутствии actual menu оставить явный callable
   production action/interface и integration boundary к его будущему владельцу.
2. Сохранить strict gate `> 4.6.1.1.0.1`: chunksize 1024, иначе 256. Выполнить
   полный `remote_update → CHECK → SUM comparison`; совпадение освобождает
   check-only transfer, несовпадение открывает настоящий source и отправляет INIT.
   Сохранять baseline checksum, mapping и результаты missing/open/read failures,
   а не подменять flow безусловной отправкой файла.
   CHECK и source read имеют собственные baseline path rules, без автоматического
   применения INIT-only запрета leading `/` и substring `..`.
3. Перенести sender state transitions: ACK разрешает следующий DATA, `do_xfers`
   отправляет bytes по chunksize, EOF приводит к END, final ACK освобождает
   sender. ERR завершает исходный transfer. Сверить empty file, exact chunk
   boundary, unknown/repeated IDs, output capacity и порядок соседних transfers;
   не отправлять DATA до предусмотренного ACK и не replay уже отправленный chunk.
   Подключить sender pump к production tick текущего startup/transport owner,
   а не только к incoming ACK callback (`c-init.c:3179` вызывает `do_xfers`
   отдельно). При EOF baseline ставит DONE, сохраняя READY, а следующий проход
   отправляет END без дополнительного ACK. Empty file и exact chunk boundary
   должны завершаться даже при отсутствии новых входящих bytes. Бюджет pump
   сохраняет обслуживание UI/сети; backpressure удерживает тот же read chunk
   до enqueue, без потери bytes или продвижения source повторным fread.
   Резервировать весь DATA frame (header и payload) либо удерживать owned pending
   chunk; READY/state продвигаются после принятия output, не при неудачном enqueue.
4. Sender/receiver используют согласованный active-transfer count для last-END
   Lua решения 001. Generation/cancel/disconnect освобождают принадлежащие
   transfer ресурсы; delayed SUM/ACK/ERR старой сессии не воздействуют на новую.
   Не вводить новые permissions, filename policy или обязательные server changes.

## Минимальная проверка и завершение

В production-core подходе 005 наблюдать exact CHECK/INIT/DATA/END bytes и реальные
source reads: cancel/30-byte limit, gate 256/1024, equal/mismatch checksum,
ACK pacing/final release, ERR/missing source и stale/cancel cleanup.
Cases EOF без новых incoming packets и output drain/resume совместить с теми же
sender проверками; отдельный runtime/harness для этого не нужен. Совмещать
cases в минимальном Linux подходе, без отдельного peer/real-account/stress suite;
расширение проверок согласовать заранее. Полный C041 admin surface не принимается.
Завершение: настоящий upload caller и sender flow покрывают четыре obligations,
без test-only remote_update/state machine и без ложной sender coverage от decoder.

## Результат реализации — 2026-11-10

Реализован callable production upload через `sv_login_upload`, checksum negotiation и ACK-paced sender. `sv_login_tick` продвигает EOF→END без нового incoming packet; полный admin menu остаётся C041.

[Evidence, команды и ограничения](../../sv-c010-evidence.md).
Canonical statuses/ownership сохранены; pending flows не получают PASS.

## Comments

### 2026-11-11 — приёмка пользователя

Пользователь подтвердил: «Этап принимаю». Задача принята в границах
[решения о приёмке C010](../../sv-stage-c010-acceptance.md). История реализации,
evidence и непроверенные условия сохраняются; canonical statuses не меняются.

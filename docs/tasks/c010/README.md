# C010 — server file transfer

Статус: **accepted** — Linux startup/core, 2026-11-11.
Пользователь принял C010 и задачи 001–006: [решение о приёмке](../../sv-stage-c010-acceptance.md).
Ограничения evidence и pending canonical statuses сохраняются.
Labels: enhancement, ready-for-agent

C010 содержит ровно **5 outcomes и 20 obligations**: CHECK, INIT, DATA, END и
upload, у каждого result/failure-lifecycle/wire/allocation-checks.
[Canonical план](../../sv-functional-stage-plan.md), [source contract](source-contract.md),
[точный planning snapshot](coverage.json). Canonical allocation, prerequisites,
conditions и pending statuses сохраняются; snapshot не является PASS.

| Задача | Результат | Единственные owned IDs |
|---|---|---|
| [SV-C010-001](SV-C010-001-lua-reload-integration.md) | Actual INIT/END → Lua owner, раздельные results и ACK/ERR | 0 новых IDs; Lua остаётся C009 |
| [SV-C010-002](SV-C010-002-check-and-wire.md) | PKT_FILE framing, CHECK, checksum/version branches | `capability.transfer.check` |
| [SV-C010-003](SV-C010-003-incoming-transfer.md) | Incoming transfer owner, INIT/DATA/END, publication и lifecycle API | `capability.transfer.init`, `.data`, `.end` |
| [SV-C010-004](SV-C010-004-upload.md) | Requested/admin upload, SUM comparison, ACK-paced sender и teardown | `capability.transfer.upload` |
| [SV-C010-005](SV-C010-005-evidence.md) | Минимальное Linux core evidence и сверка 20 obligations | 0 IDs |
| [SV-C010-006](SV-C010-006-targeted-validation.md) | Sender backpressure и actual native startup: обе проверки PASS | 0 IDs; accepted |

Порядок реализации: 002 interfaces/framing → основа 003 (общий registry/incoming)
→ 004 sender → совместное завершение 003+001 → evidence 005.
Создание owner/API в 003 не ждёт связки 001; полный END outcome завершается
только с ней. Это не циклический implementation prerequisite.
Все пять canonical rows зависят от C003 `files.resource-overlay` и
`files.transient-owner`; END также от принятого C009 `lua.reload`.
C011 не prerequisite C010: transfer core предоставляет production preparation
interface, actual readiness/handoff принимает C011, live close/relog — C014.
Полный поздний admin menu остаётся C041, production upload caller/sender — C010.

## Границы реализации и решённые вопросы

На момент исходного планирования в SV не было полного PKT_FILE decoder/sender;
существующие B protocol/transport и C009 Lua API выбраны как основа.
Перенос локализуется в SV, без общего
refactor legacy/common. Изоляция, последовательная доставка и generation следуют
[CONTEXT](../../../CONTEXT.md), [архитектуре](../../sv-architecture.md) и ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).

Actual startup ingress/output и drain/resume подключаются в 002, borrowed Lua
attachment/begin из decoded features — в 001, sender pump с progress без новых
incoming bytes — в 004. C010 даёт работающий путь текущего login owner;
будущие C011/C014 используют этот seam без второго decoder или повторного begin.
Legal modern DATA frame занимает 1030 bytes, не 1024 bytes payload; bounded
buffering/commit и output retry не повторяют filesystem/runtime effects.

Q40 уже решён: original validation/mapping, без нового allow/deny-list или blanket
rebase в SV profile. Generic `sv_file_path` нельзя считать эквивалентом baseline
INIT validation (leading `/` и любое `..`): сверять точные ветви.
CHECK и upload source не наследуют эти INIT-only фильтры автоматически.
C003 `SvFilePublication` обслуживает export с consent/conflict/atomic rename;
эти prompts и гарантии не переносятся автоматически на server transfer.
Baseline END копирует temporary в destination через `wb`, не atomic replace.

Gate MD5 и upload 1024-byte chunks — strict `> 4.6.1.1.0.1`; старая ветвь
сохраняет broken Adler32 и 256-byte chunks. Полный DATA с disk-write failure
отличается от partial packet: baseline diagnostic без ACK/ERR не превращать
в автоматический retry/повторную запись. Детали зафиксированы в source contract.

Формулировка canonical END «does not falsely reload» читается вместе с явным
решением пользователя 2026-11-10: last END выполняет reload текущих файлов даже
после failed close, но transfer ERR не становится ACK, а runtime failure не
скрывается. Это решённая семантика, не новый вопрос на согласование.

Минимальная текущая проверка — Linux production core; Windows/MinGW/Wine deferred.
Расширение набора проверок согласовать заранее по AGENTS. Отдельные улучшения,
не нужные для C010, записывать в [список улучшений](../../sv-improvements.md).

## Открытые вопросы

Блокирующих пользовательских вопросов по scope и baseline при планировании не
выявлено. Конкретные SV API/файлы owner и preparation aggregate уточняются в
реализации без изменения canonical prerequisites; C011/C014/C041 consumer evidence
остаётся своим этапам. Любое предлагаемое изменение filename policy, DATA failure
recovery или publication semantics требует отдельного решения и не включено в C010.

Повторный анализ тремя агентами 2026-11-10 уточнил задачи 001–005: actual startup
attachment, input/output backpressure, полный размер DATA, operation-specific path
rules, checksum failure bytes, EOF/pump sender и разметку evidence. Новые задачи
не потребовались; число outcomes/obligations и canonical statuses не изменились.

C009 принят пользователем 2026-11-10 в границах Linux core. Обязательная связка
001 не блокирует и не переоткрывает C009 автоматически; evidence backlink служит
трассировке. Непроверенные actual transfer flows не получают PASS от C009 core.

## Обязательная интеграция C009

Использовать [production Lua owner C009](../c009/README.md) и
[completion contract 002](../c009/SV-C009-002-reload-lifecycle.md), без отдельной
реализации Lua reload в transfer decoder. Самодостаточный объём, API, baseline,
минимальные Linux checks и критерии завершения закреплены в
[SV-C010-001](SV-C010-001-lua-reload-integration.md). При полном планировании C010
сохранить эту связку с владельцем `capability.transfer.end`; evidence к трём
`obligation.lua.reload.*` служит трассировке принятого core.

C010 владеет CHECK/INIT/DATA/END/upload, version/checksum branches, temporary
ownership, публикацией и ACK/ERR. Передать C009 результат close, актуальное число
transfers, session generation. Сразу после успешного INIT передать accepted-INIT occurrence
в application Lua owner: updated_audio/guide отмечаются по baseline substring
имени `audio.lua`/`guide.lua`. Rejected INIT и `-u` их не меняют. Признаки sticky
в течение процесса: subsequent DATA/END failure, disconnect до END и новая session
их не сбрасывают. Generation проверяется до фиксации; передавать признаки только
из текущего session batch при END недостаточно.

После последнего END при нулевом числе transfers вызвать reload текущих файлов,
в том числе при неуспешном сохранении: так решил пользователь 2026-11-10.
Сохранить последовательность close/publication → reload → audio warning →
Guide metadata → ACK/ERR. Ошибка Guide getter не подавляет предшествующий warning. Ошибка передачи и результат reload независимы: ERR не становится ACK
от успешного чтения старых файлов, успешная передача не скрывает Lua failure.
Передать оба результата C011, без ложной startup readiness. Результат каждого
END синхронно потребить до обработки следующего END, включая chained packets:
completion slot — последний результат, а не очередь. Сохранять отказы в startup
aggregate, чтобы последующий successful END не маскировал предыдущую ошибку.

`-u` C002 отвергает все FILE_INIT как baseline, а не только `.lua`; он не запрещает
обычное чтение имеющихся локальных scripts. Частичный packet не даёт эффектов;
полные события сохраняют порядок, stale generation не вызывает file/runtime
эффектов. Не подавлять реальный новый wire END фиктивной deduplication по fnum;
отличать его от повторного внутреннего потребления того же события.

## Завершение интеграции

В существующих минимальных Linux core checks C010 наблюдать actual decoder →
close → reload/metadata/warning → ACK/ERR: несколько файлов, последний END,
failed last close, partial/chained input и stale generation, без повторных
эффектов от redraw/повторного внутреннего consumption. Прямой C009 API test не
доказывает эти flows. Дополнение не вводит новый harness или broad suite;
расширение проверок согласовать по AGENTS. Windows/MinGW/Wine deferred.
Этот раздел фиксирует исходные критерии; actual core результат и limits приведены ниже.

## Результат реализации — 2026-11-10

Реализованы SV-local transfer registry/receiver/sender, checksum branches,
actual login dispatch/tick и borrowed Lua startup adapter. Production Linux build
и одна связанная ASan/UBSan core-проверка прошли: [evidence](../../sv-c010-evidence.md).
20 obligations имеют отдельный disposition, включая pending faults и late consumers.
C011 readiness, C014 live lifecycle и C041 admin menu остаются своими этапами;
canonical statuses не менялись, пользовательская приёмка C010 не заявлена.

## План дополнительных проверок — 2026-11-10

По следующему запросу пользователя три агента подготовили узкий
[план SV-C010-006](SV-C010-006-targeted-validation.md): один sender saturation
case в существующем core test и один mode existing native managed peer.
На шаге планирования тестовый/production код не изменялся и новые проверки
не запускались; последующее выполнение описано ниже. Evidence 005 и прежние PASS сохраняются. Другие pending faults
и C011/C014/C041 не включаются скрыто в эти две проверки.

## Дополнительные проверки выполнены — 2026-11-10

По поручению пользователя агенты выполнили 006: sender output saturation
(ASan/UBSan) и один actual native socket/login/FILE/Lua case (ordinary production
objects) прошли. Независимый read-only review не выявил actionable defects.
[Результаты, commands/hashes и оставшиеся ограничения](../../sv-c010-006-evidence.md).
Два рекомендованных риска закрыты; canonical statuses и пользовательская
приёмка C010 автоматически не меняются.

## Приёмка — 2026-11-11

Пользователь явно подтвердил: «Этап принимаю». C010 и 001–006 приняты
в проверенных границах Linux startup/core. [Решение](../../sv-stage-c010-acceptance.md)
имеет приоритет над историческими формулировками о незаявленной приёмке выше.
Непроверенные I/O условия сохранены в evidence; C011/C014/C041 остаются
самостоятельной работой своих этапов. Canonical registry statuses не менялись.

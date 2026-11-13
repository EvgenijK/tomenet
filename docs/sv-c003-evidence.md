# C003 — текущее Linux evidence

Дата: 2026-11-09. Статус этапа: **accepted пользователем**, включая задачи 001–006.
[Решение о приёмке](sv-stage-c003-acceptance.md) отменяет прежний gate ожидания
late flow evidence. Нижеследующие проверки и ограничения сохранены фактически:
непроверенные flows не получают выдуманный PASS. Поздние интеграции/исправления
выполняются в своих этапах и не переоткрывают C003 автоматически. Возврат evidence
не является условием уже состоявшейся приёмки. Canonical runtime registration
остаётся отдельным workflow, registry не повышен этим документом.
[Spec](sv-stage-c003-spec.md), [задачи](tasks/c003/README.md).

База HEAD: `dc14c4ad5d5a9aa202051de929cbc9b601ddadb7` плюс текущее незакоммиченное
дерево. До начала уже были изменения C002/B2/архитектуры и планов; они сохранены.
Только SV-local production source; legacy/common production source не менялся.
Linux clang C99 `-Wall -Wextra -Werror`; standalone build
`linux-ff24240d10a53db590cb`. Windows/MinGW/Wine deferred по правилу платформ.
Новые файлы и собственные изменения не коммитились/не публиковались.

## Реализованный production путь

- `files/files.c`: resolved SHARED/PROFILE/BUNDLED paths; read/stat/open и
  owner-aware rename/remove; U→B resource lookup пропускает missing ENOENT,
  отличая ENOTDIR/invalid overlay. Bundled writer запрещён. Font candidate
  paths и audio pack reads/stat используют file owner, сохраняя consumer-specific
  fallback, `.default` read-only и отдельные requested/effective values.
- Publication operation пишет adjacent owned temp, flush/fsync/close до publication.
  Generated target использует no-clobber link/free suffix, включая занятые
  file/directory/symlink names. Explicit existing требует replace continuation;
  проверяются inode/size/mtime/ctime перед заменой. Cancel/failed write удаляет
  только собственный temp и сохраняет старое назначение. Реального экспортера
  не добавлено ради core проверки.
- `files/history.c`: private `U/sv/chathist-<account>.tmp`, 2000 записей × baseline
  256-byte slots, oldest-first persistence и duplicate move-to-newest. Secret
  input kind отвергается; endpoint history не используется. Authenticated
  `login/flow.c` создаёт/читает account owner, end-attempt сохраняет его независимо
  от CFG/OPT Save. Ввод chat пока отсутствует: C028 подключит accepted non-secret
  input до transformations/Send_msg. Corrupt/unread source не перезаписывается;
  внешняя версия привязана к открытому fd и не rebased при publication begin.
- `files/dna.c`: SV-local exact baseline roman suffix parser, shared U/user DNA
  с bundled fallback, previous-name lookup, bounded parsing и old male/bat flags
  conversion. Conversion привязана к версии прочитанного inode. Flow загружает
  данные до `sv_birth_begin`; существующий birth owner проверяет choices/hidden
  class mapping. Shared DNA сохраняется один раз после committed birth/успешного
  queue PLAY, до server ACK, как самостоятельный baseline lifecycle. Quit draft
  не сохраняется. File data и fixture injection не смешиваются в evidence.
- `files/message-sinks.c` + `app.c`: реальный production packet→session→occurrence
  вызывает effect ровно раз. Notes/continuation append, marker/colour stripping,
  timestamp; clone console/file off по умолчанию, append/flush, shared destinations.
  Есть typed assembly `sv_message_files_begin/options/sink` и app binding API;
  ошибки возвращаются отдельным semantic notice callback без recursive clone.
- `protocol/resource-report.h`: единые version/audio/font serializers для login
  и live protocol. Startup PLAY больше не выдаёт UI text font за map font:
  current missing map font пустой, graphics=0/NO_GRAPHICS. Font notification
  отправляется one-shot после ACK; audio C002 переиспользован. Version request
  обработан в login ACK stream и live app dispatcher, с backpressure-safe retry.
  Все `%s` reporting copies ≤79 bytes+NUL; full local names сохраняются.

## Что проверено

| Проверка | Фактический результат |
|---|---|
| Standalone `make -f makefile.sv -C src tomenet-sv -j4` | PASS, production sources связаны; [build log](../.scratch/sv-c003/evidence/build-final.log). |
| `tests/sv/files.c` + production files/history/DNA, clang ASan+UBSan | PASS: U/B priority, owner stat/rename/remove, bundled write denial, generated/explicit collision, replace/cancel, external edit conflict, `/dev/full` write failure/temp cleanup; ring wrap/dedup/private round-trip/CRLF/corrupt/account/secret exclusion; DNA normalized/previous/current/old/missing/invalid. |
| Existing `tests/sv/settings-audio.c` + production audio/config/options/login/files | PASS: real dummy device/packs and previous startup semantics after file routing. Test сам устанавливает dummy driver; внешний SDL_AUDIODRIVER мешает его SDL_SetHint и не используется в успешном запуске. |
| Existing `tests/sv_message_checks.py`, ASan/UBSan | PASS: production packet→app→sinks; repeated notes/continuation, chain reset, clone off/on, stripping/order, redraw без повторного append, `/dev/full` fclose failure и notices. Ordinary native session не имитируется. |
| Existing `tests/sv_contact_checks.py`, `tests/sv_login_checks.py` | PASS (005 executor): source-defined layouts/gates, 78/79/80 reporting boundaries, full local retention, ready/unavailable, malformed snapshot preservation, ping mean, retry, login/live font consistency. |
| Existing `tests/sv_lifecycle_checks.py`, ASAN_OPTIONS=detect_leaks=0 | PASS (005 executor): generation/stale state и reporting reset. |
| Existing `tests/sv_birth_complete_checks.py`, local native peer + SDL dummy | PASS: добавлен только необходимый cancel case перед final commit; затем waiting/partial/ack. [Cancel](../.scratch/sv-c003/evidence/cancel.log), [waiting](../.scratch/sv-c003/evidence/waiting.log), [partial](../.scratch/sv-c003/evidence/partial.log), [ack](../.scratch/sv-c003/evidence/ack.log). Actual `U/user/Carol.dna` проверен по fields/stats после completed birth; Ctrl-Q на MODE не создаёт DNA. Waiting/partial остаются PLAY_PENDING, только полный reply — LIVE_HANDOFF. |

Birth fixture хранит исходный literal oracle; тест корректирует только прежнее
ошибочное UI-font-as-map поле на empty missing identity. Остальные literal bytes
и ранние character options сохранены: теперь 16242 bytes вместо 16266. ACK case
дополнительно проверяет реальный missing-font notification и audio. Никакой
новой version/visual/packaging matrix не запускалось. Existing runner source lists
дополнены только новыми link dependencies; это не новые наборы проверок.

Первый sandbox birth запуск не мог создать listener; разрешённый локальный
loopback запуск это ограничение снял. Первый запуск со старым font oracle ждал
старое число bytes и получил bounded timeout; описанная корректировка oracle
сохраняет проверку остального handshake. Финальные четыре cases завершились PASS.
Default LSan в этой среде не работает из-за ptrace; ASan+UBSan включены,
успешные прогоны используют `ASAN_OPTIONS=detect_leaks=0`. LSan PASS не заявлен.

## Воспроизведение минимальных core checks

```sh
clang -Isrc/client/sv -std=c99 -Wall -Wextra -Werror -g \
  -fsanitize=address,undefined tests/sv/files.c \
  src/client/sv/files/files.c src/client/sv/files/history.c \
  src/client/sv/files/dna.c -o /tmp/sv-c003-files
ASAN_OPTIONS=detect_leaks=0 /tmp/sv-c003-files
clang -std=c99 -Wall -Wextra -Werror -Isrc/client/sv \
  tests/sv/settings-audio.c src/client/sv/settings/audio.c \
  src/client/sv/settings/config.c src/client/sv/settings/options.c \
  src/client/sv/protocol/login.c src/client/sv/files/files.c \
  -lSDL3_mixer -lSDL3 -o /tmp/sv-c003-audio
/tmp/sv-c003-audio
ASAN_OPTIONS=detect_leaks=0 python3 tests/sv_message_checks.py
python3 tests/sv_contact_checks.py
python3 tests/sv_login_checks.py
ASAN_OPTIONS=detect_leaks=0 python3 tests/sv_lifecycle_checks.py
SV_BIRTH_EVIDENCE=/tmp/sv-c003-birth python3 tests/sv_birth_complete_checks.py
make -f makefile.sv -C src tomenet-sv -j4
```

## Границы доказательств после пользовательской приёмки

Ordinary native startup сейчас заканчивается LIVE_HANDOFF; он ещё не создаёт
SvApp/не принимает live connection. Поэтому app packet tests доказывают production
core, но не ordinary native notes/clone binding, user error surface или gameplay.
Секреты конструктивно не доступны message API; тестовые credential/editor buffers
не используются как message source. C014/C016 должны подключить actual ordinary
session и notice presentation; source inspection не выдаётся за этот runtime proof.
Реальный server PKT_VERSION приходит после END и остаётся для live owner;
login parser не присваивает себе эти bytes. Version core PASS не подтверждает
отсутствующее native live adoption.

C006/C007 завершают enabled map/font/graphics activation/reinit evidence; C016 —
full recall и message effects integration; C028 — настоящий chat input/history;
C029/C032/C061 — реальные export/final-review/screenshot producers; C014 — полный
quit/relog parent и совместный exit-log child. Их самостоятельные требования
находятся в [документах этапов](tasks/c003/README.md), общий verification список
не требуется. Нет fake exporter, chat UI, full recall или renderer внутри C003.

Ограничение concurrent edits: выявленные изменения до commit дают CONFLICT и не
затираются. Shared legacy writers не участвуют в lock protocol; последнюю гонку
между финальным stat и rename и последующие записи legacy исключить нельзя.
Это принятое ограничение policy, не гарантия transactional межпроцессной записи.
Source-backed hardening corrupt history/DNA не имитирует unsafe legacy indexing;
ошибка не даёт false save. Cancel/empty history не запускают CFG/OPT autosave.

Ниже каждый canonical obligation сохранён; «core PASS; pending flow» не является
canonical evidenceStatus PASS. Windows ветви deferred отдельно, без фиктивной
приёмки. Полные late flows не приняты по helper/serializer snapshot.

| Canonical obligation | Текущее disposition |
|---|---|
| `obligation.files.history-load.result` | core/private load PASS; actual C028 input/relog flow pending; Linux only. |
| `obligation.files.history-load.failure-lifecycle` | core/private load PASS; actual C028 input/relog flow pending; Linux only. |
| `obligation.files.history-load.allocation-checks` | core/private load PASS; actual C028 input/relog flow pending; Linux only. |
| `obligation.files.history-save.result` | core save/account/secret/conflict PASS; actual C028/C014 flow pending; Linux only. |
| `obligation.files.history-save.failure-lifecycle` | core save/account/secret/conflict PASS; actual C028/C014 flow pending; Linux only. |
| `obligation.files.history-save.allocation-checks` | core save/account/secret/conflict PASS; actual C028/C014 flow pending; Linux only. |
| `obligation.files.resource-overlay.result` | core and existing font/audio paths PASS; late active consumers pending; Linux only. |
| `obligation.files.resource-overlay.failure-lifecycle` | core and existing font/audio paths PASS; late active consumers pending; Linux only. |
| `obligation.files.resource-overlay.allocation-checks` | core and existing font/audio paths PASS; late active consumers pending; Linux only. |
| `obligation.files.dna-shared.result` | current core/native completed+cancel PASS; wider DNA shortcut/version branches not claimed; Linux only. |
| `obligation.files.dna-shared.failure-lifecycle` | current core/native completed+cancel PASS; wider DNA shortcut/version branches not claimed; Linux only. |
| `obligation.files.dna-shared.allocation-checks` | current core/native completed+cancel PASS; wider DNA shortcut/version branches not claimed; Linux only. |
| `obligation.files.notes-append.result` | app production packet core PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.notes-append.failure-lifecycle` | app production packet core PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.notes-append.allocation-checks` | app production packet core PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.clone-console.result` | app core off/on/strip/flush PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.clone-console.failure-lifecycle` | app core off/on/strip/flush PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.clone-console.allocation-checks` | app core off/on/strip/flush PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.clone-file.result` | app core shared append/errors PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.clone-file.failure-lifecycle` | app core shared append/errors PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.clone-file.allocation-checks` | app core shared append/errors PASS; ordinary native C014/C016 binding pending; Linux only. |
| `obligation.files.export-collision.result` | publication core PASS; actual C029/C032/C061 producers pending; Linux only. |
| `obligation.files.export-collision.failure-lifecycle` | publication core PASS; actual C029/C032/C061 producers pending; Linux only. |
| `obligation.files.export-collision.allocation-checks` | publication core PASS; actual C029/C032/C061 producers pending; Linux only. |
| `obligation.files.transient-owner.result` | publication/history ownership+cleanup core PASS; late operation consumers pending; Linux only. |
| `obligation.files.transient-owner.failure-lifecycle` | publication/history ownership+cleanup core PASS; late operation consumers pending; Linux only. |
| `obligation.files.transient-owner.allocation-checks` | publication/history ownership+cleanup core PASS; late operation consumers pending; Linux only. |
| `obligation.files.report-resource-identity.result` | login/live core layouts and missing identity PASS; ordinary live adoption and enabled C006/C007 pending; Linux only. |
| `obligation.files.report-resource-identity.failure-lifecycle` | login/live core layouts and missing identity PASS; ordinary live adoption and enabled C006/C007 pending; Linux only. |
| `obligation.files.report-resource-identity.wire` | login/live core layouts and missing identity PASS; ordinary live adoption and enabled C006/C007 pending; Linux only. |
| `obligation.files.report-resource-identity.allocation-checks` | login/live core layouts and missing identity PASS; ordinary live adoption and enabled C006/C007 pending; Linux only. |

## Обновление существующего графа

Один финальный `.graphify-venv/bin/graphify update .` завершился exit 0:
21372 nodes, 56445 edges, 829 communities. [Лог](../.scratch/sv-c003/evidence/graph-update.log).
Code graph обновлён без новой генерации. CLI сообщил о частичном AST extraction
для 78 файлов с syntax errors, 184 code files без symbols и изменении community
labels. Документальные semantic changes этим code-only update не извлечены;
полнота Graphify не заявляется. Эти предупреждения относятся к навигационному
графу, не к успешной компиляции SV; отдельное rebuild/label не запускалось.

## C006 integration — Linux core, 2026-11-09

C006 подключил production map-font owner и resource bootstrap hook.
[Фактические caller/команды/источники и ограничения](sv-c006-evidence.md).
`settings/map-font.c` использует C003 file owners и C004 parser/reset seam;
`login/native-run.c` активирует font roles, `login/flow.c` применяет character
visuals перед PLAY, `protocol/login.c` сериализует actual definitions/identity.
Минимальный map-font check подтверждает U/user custom F:3=(attr9,glyph81),
B/user custom F:3=(attr8,glyph80), затем B system F:5=(attr6,glyph82); invalid
custom directory не публикуется, missing system явно даёт zero overrides.
PLAY bytes содержат эти таблицы, live output — CLIENT_SETUP затем FONT.
Source full refs сохраняются локально; C003 reporting copy policy не менялась.
App PRF effects имеют generation/payload/context, repaint не генерирует их снова;
actual messages/actions delivery остаётся C016/C053. FI-01 core evidence
дополнено; FI-02 active graphics/coupled reload остаётся C007. Это не меняет
прежнюю пользовательскую приёмку C003/C004 и не регистрирует canonical PASS.


## C007 — возврат core integration evidence, 2026-11-09

Production graphics consumer реализован: coupled font/graphics PRF candidate,
actual base/subset/mask/raw preparation, effective reporting и atomic publication.
[Evidence C007](sv-c007-evidence.md) содержит callers/commands, real sources/generations,
PLAY/live FONT boundary observations и отдельную disposition 45 rendering
obligations. Для C003 это FI-02 resource-overlay/report-resource-identity trace;
для C004 — graphics preference/reset/reload trace. Полный late packet-derived
map flow, backend/budget и непроверенные failure branches остаются pending.
Это дополнение не переоткрывает принятую приёмку C003/C004 и не меняет их
canonical allocation/runtime statuses; сама приёмка C007 не выполнялась.


Последующее решение 2026-11-09: [C007 принят пользователем](sv-stage-c007-acceptance.md) в представленных
границах Linux core. Поздние flows и непроверенные branches остаются таковыми;
это не новая регистрация native-runtime PASS C003/C004.

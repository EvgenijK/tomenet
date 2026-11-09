# SV-C003-004 — notes append и клонирование сообщений

Статус: accepted, 2026-11-09, в составе явно принятого пользователем C003.
Labels: enhancement
Зависимости: 001; settings options C002.
Owned IDs: `capability.files.notes-append`, `capability.files.clone-console`, `capability.files.clone-file`.
Контракт: [spec](../../sv-stage-c003-spec.md), [источники](source-contract.md), [полные rows](coverage.json).

1. Подключить effects к успешному production message occurrence; использовать существующий decoder/session sequencing, не snapshot/render. Не терять identical repeated occurrences при bounded recall.
2. Notes: baseline HCMSG_NOTE и continuation recognition, timestamp/stripping, append U/user/notes-account.txt. Non-note сбрасывает chain, partial packet не меняет её, новое поколение/account не наследует state.
3. Clone: обе опции исходно off; console и shared U/user/stdout.txt независимы; baseline colour stripping/newline/append/flush. Получают game message bytes, не credential/editor buffers; не добавлять recorder.
4. Ошибки sink сообщаются пользователю без recursive clone error loop/false success и без разрыва работоспособной сессии только из-за optional sink.
5. Минимальные core cases: два одинаковых notes/continuation occurrences по порядку, non-note reset, partial packet, relog/account reset, off/on sinks, stripped final colour marker, redraw без повторной записи, disk denied/flush failure и secret exclusion. Test вызывает production packet→session→effect, не отдельный baseline parser. C016 завершает evidence при полном recall; текущие 6 live строк не заменяют исторический источник C029 export.

Готовность: production wiring и применимые Linux core evidence записаны; все
owned obligations имеют честный disposition. Проверки расширяются только после
согласования. Пользовательская приёмка C003 и этого ticket явно дана 2026-11-09.

## Решение и отложенная интеграция — 2026-11-09

Утверждены core и существующие consumers; границы поздних этапов сохранены.
Самостоятельные integration/evidence требования закреплены непосредственно в
[C016](../c016/README.md), [C029](../c029/README.md). Общий verification список читать не требуется;
canonical IDs и obligations не перенесены. C003 принят пользователем 2026-11-09;
helper-only evidence не закрывает поздний flow.


## Core implementation — 2026-11-09

Реализован SV-local `files/message-sinks.{h,c}` через C003 resolved shared-user
paths и `sv_file_open`. `SvMessageFiles` отдельно хранит account/generation,
notes chain и исходно выключенные clone flags. Notes сохраняют baseline
recognition/continuation (включая previous-chain offset), local timestamp,
marker/colour stripping и append. Clone console/file сохраняют routing-prefix
и colour-pair stripping, newline и flush/close; destinations — `U/user`.

В `app.c/h` добавлен явный borrowed `SvMessageSink`: deliver вызывается ровно
после успешного message apply, reset при release поколения. Очередь доставки
не захватывается renderer или file sink; snapshot/redraw/ack не повторяют эффекты.
Ошибки всех независимых sinks получают semantic notice без текста сообщений
и устанавливают optional executor failure без разрыва рабочей сессии. Типизированный
sink принимает только server game-message fields, не credential/editor buffers.

Минимальные проверки добавлены в существующие `tests/sv/messages.c` и
`tests/sv_message_checks.py`: реальный packet→session→effect, partial packet,
одинаковые occurrences, note reset, account/generation change, default off/on,
broken colour marker, `sv_messages_prepare` redraw без replay, append-open failure,
console flush и notes fclose flush failure через Linux `/dev/full`.
`ASAN_OPTIONS=detect_leaks=0 python3 tests/sv_message_checks.py` — PASS с Address/
UndefinedBehaviorSanitizer. Default LeakSanitizer запуск ограничен ptrace политикой
среды; leak evidence этим результатом не заявлено. `git diff --check` затронутых
tracked файлов — PASS. Это core evidence, не доказательство runtime ветвей поздних consumers.
Пользовательская приёмка C003 и ticket дана отдельным явным решением.

Открыто: ordinary native login сейчас заканчивается `LIVE_HANDOFF`, не создавая
рабочий SvApp gameplay pump. Поэтому ordinary connection adoption, bind
roots/account/clone options и показ sink notices в игровом UI должны быть
подключены настоящим C014/C016 consumer; ради PASS новый pump здесь не создан.
Full recall/search lifecycle evidence также остаётся C016, exports — C029.
Проверенный public production app path не доказывает обычную native gameplay
сессию. Полная приёмка C003 и canonical статусы этим дополнением не меняются.

## Current core evidence — 2026-11-09

[Техническое evidence](../../sv-c003-evidence.md) различает ordinary native startup
и production app core, фиксирует минимальные Linux результаты и remaining flows.
[Пользовательская приёмка accepted](../../sv-stage-c003-acceptance.md); canonical runtime PASS не выставлен; пользовательская приёмка accepted. Поздние самостоятельные integration tasks остаются обязательными.

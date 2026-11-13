# SV-C014-001 — Exit/relog: history и родитель export interaction

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: implementation follow-up к плану C014; связка FI-06.
Основание: решение пользователя 2026-11-09 — C003 core и существующие consumers
сейчас; отсутствующие flow consumers/evidence завершаются в ответственных этапах.
Эта задача самодостаточна; общий индекс связок не нужен для её выполнения.
Это не полный план C014, не новые capability IDs и не перенос ownership.

C014 владеет quit/reconnect/применимым portal-relogin parent; история реализована
в [SV-C003-002](../c003/SV-C003-002-history.md), publication и operation cleanup —
в [SV-C003-001](../c003/SV-C003-001-file-owners.md). Затронуты C003
`capability.files.history-load`, `capability.files.history-save`,
`capability.files.export-collision`, `capability.files.transient-owner`.
C029 владеет formatter/export child, C016 — полным источником сообщений.

## Работа

1. В production quit/reconnect/portal-relogin owner подключить C003 finish-account
   и begin-account: save перед освобождением/сменой account state, load для нового
   подтверждённого account; общий путь не должен сохранять дважды при shutdown.
   Путь истории `S/chathist-<account>.tmp`, `S=U/sv`; это persistent history,
   не operation temp. Settings Save и CFG/OPT autosave не участвуют.
2. Создать явное продолжение parent→optional exit-log child→parent. Сохранить
   нужный message source/account/generation до завершения child; результат
   success/cancel/error возвращает к правильному shutdown/relogin, не запускает
   повторный shutdown и не отменяет самостоятельный history save.
3. Передавать реальные invocation `-v/-V/-x`, `quit_no_prompt` и baseline условия
   chat-only/all-messages/no-export в поздний consumer, не подменять выбор export
   новым settings prompt. Сверить `src/client/c-init.c:quit_hook/do_save_chat`,
   `settings/startup.c` и `login/native-run.c` внутри SV. Ожидание interaction
   обслуживает UI/network согласно состоянию parent; блокирующий legacy loop
   не переносить. Stale continuation не меняет новое поколение/account.
4. C014 реализуется раньше C016/C029: проверить доступный parent lifecycle
   отдельно. Отсутствующий child оставить явным pending; никакого обратного
   canonical prerequisite C014→C029 и никакого фиктивного exporter. После появления
   C029 его задача повторно проходит совместный flow с уже реализованным parent.

## Готовность и минимальные Linux core checks

Production exit/relog действительно сохраняет/загружает account history даже при
cancel/error export и без settings Save; смена account изолирована, write failure
виден, CFG/OPT не меняются, stale parent/child result не действует. Проверить
необходимые варианты quit/reconnect/portal parent через реальные lifecycle API,
actual history paths/bytes, cleanup только operation temp. Parent evidence без
реального C016+C029 child не закрывает полный exit-log flow: отдельно записать
pending и позднее actual CLI→child→publication→return evidence. Не выдавать
имитированный child за production export. Готовность этого follow-up не означает
готовность всех outcomes C014.

## Возврат evidence и ограничения

После реализации передать в [SV-C003-006](../c003/SV-C003-006-acceptance.md)
и `docs/sv-c003-evidence.md` конкретные production callers/revision/build,
применимые C003 obligation IDs из [coverage](../c003/coverage.json), команды Linux,
expected/actual paths/bytes/occurrences/lifecycle и remaining pending с причиной.
Закрывать только доказанную часть связки, не все C003 rows по наличию документа;
C003 уже принят пользователем 2026-11-09. Эти результаты нужны своему позднему
этапу; возврат evidence не prerequisite приёмки C003, проблемы исправляются здесь. Canonical JSON allocation/IDs/prerequisites/obligations/statuses
не менять этой задачей.

Добавлять лишь минимально необходимые core checks через production path;
расширение проверок согласовать заранее по [AGENTS](../../../AGENTS.md).
[Linux обязателен](../../agents/platform-validation.md), Windows/MinGW/Wine deferred.
Изменения SV-local, legacy/shared refactor не требуется. Main-thread ownership,
явные связи и generation-scoped continuation соответствуют ADR-0002/0003/0005/0006.
Сейчас изменены только планы: implementation/runtime evidence и PASS не заявлены.

## Уточнение после C003 core — 2026-11-09

Обычный native startup пока заканчивается LIVE_HANDOFF без создания SvApp и
adoption live connection. Завершить настоящий ordinary caller binding к уже
реализованным `sv_message_files_begin/options/sink` и `sv_app_set_message_sink`,
передав actual U/account/clone flags и separate semantic notice presentation.
Owner живёт дольше app binding; reset по account/generation. Не считать app peer
core PASS доказательством этой native связки; reporting PKT_VERSION после END
также должен достаться live owner. [C003 evidence](../../sv-c003-evidence.md).

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.

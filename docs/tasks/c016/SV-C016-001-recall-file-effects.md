# SV-C016-001 — Полный recall и однократная доставка файловых эффектов

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: implementation follow-up к плану C016; связка FI-03.
Основание: решение пользователя 2026-11-09 — C003 core и существующие consumers
сейчас; отсутствующие flow consumers/evidence завершаются в ответственных этапах.
Эта задача самодостаточна; общий индекс связок не нужен для её выполнения.
Это не полный план C016, не новые capability IDs и не перенос ownership.

C016 владеет full recall/important source и просмотром сообщений; файловые
эффекты принадлежат [SV-C003-004](../c003/SV-C003-004-message-sinks.md):
`capability.files.notes-append`, `capability.files.clone-console`,
`capability.files.clone-file`. C029 потребляет исторический источник для export.

## Работа

1. В production packet→session model path создать полноценный bounded baseline
   recall и important source с нужными occurrence/order/content. Существующие
   `session/session.h:SV_MESSAGE_LINES=6` и `SvMessages` — short live feed,
   а `SV_MESSAGE_PENDING` — delivery queue; ни один не является full recall.
   Не подменять историю экспорта шестью видимыми строками или input history.
2. Подключить существующий C003 effect/delivery owner после successful apply
   complete message occurrence в `sv_app_step`/явной app assembly. Сохранить
   одного владельца обязательной доставки; не конкурировать за `sv_app_take_message`
   между renderer и file sinks. Partial packet и повторный observer callback
   не создают effect. Два легитимных одинаковых сообщения остаются двумя events.
3. Recall/search/important view/redraw/UI recreation читают presentation state,
   не воспроизводят notes/clone append. Удаление строки из live feed не удаляет
   обязательную доставку и не изменяет правила full recall retention.
4. Дать C029 явный historical snapshot/source с сохранением baseline order,
   markers и repetition information для `dump_messages_aux` semantics; удерживать
   lifetime при export/exit child. Источник — semantic model, не Term/screen scrape.
   Форматирование/публикация файлов остаются C029/C003, не C016.

## Готовность и минимальные Linux core checks

Через реальные protocol/app интерфейсы доставить complete/partial сообщения,
включая повтор одинакового текста и private note continuation; проверить actual
notes bytes в `U/user/notes-<account>.txt`, console/file clone occurrence order
(`U/user/stdout.txt`, switches initially off). После recall/search/redraw/recreation
байты/число effects не меняются. Сообщения старше шести live строк остаются
доступны full source в рамках baseline bounds и доходят до будущего export.
Отказ sink виден без false success; generation/account change не продолжает
старую note chain. Файловые алгоритмы не дублировать. C029 позже подтверждает
формат actual export; готовность источника не закрывает его caller evidence.
Готовность этого follow-up не означает готовность всех outcomes C016.

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

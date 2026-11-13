# SV-C029-001 — Настоящие message/character exports и exit-log child

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: implementation follow-up к плану C029; связка FI-05; совместное завершение FI-06/FI-07.
Основание: решение пользователя 2026-11-09 — C003 core и существующие consumers
сейчас; отсутствующие flow consumers/evidence завершаются в ответственных этапах.
Эта задача самодостаточна; общий индекс связок не нужен для её выполнения.
Это не полный план C029, не новые capability IDs и не перенос ownership.

C029 владеет настоящими `exports.messages`/`exports.character` producers,
formatter и export child. Использовать
[SV-C003-001](../c003/SV-C003-001-file-owners.md):
`capability.files.export-collision`, `capability.files.resource-overlay`,
`capability.files.transient-owner`. C016 предоставляет full recall; C014 —
exit/relog parent; C032 — final-review/tomb parent. Их lifecycle не копировать
в exporter; C061 screenshots имеет собственного producer.

## Работа

1. Подключить реальные live message/character export actions к formatter и C003
   publication interface. Message source — full recall/important из C016,
   не шесть live строк, не input history и не искусственно созданная fixture.
   Сохранить baseline content/order/repetitions/filtering/colour/glyph conversion
   и выбранный message/chat mode по `src/client/c-xtra2.c:dump_messages_aux`,
   `dump_messages` и character dump production baseline.
2. Передать реальные owned bytes в C003 resolved owner/publication. Экспорты
   принадлежат shared `U/user`, не `S=U/sv` или bundled B. Generated filename
   получает свободный suffix; explicit existing destination требует replace/cancel.
   Продолжения используют C003 result/confirm/cancel, учитывают изменение target
   извне; failed write/replace сохраняет прежние bytes. Не повторять filesystem
   алгоритмы в formatter; temp/backup cleanup остаётся operation owner.
3. Реализовать exit-log child для существующих `-v/-V/-x`, `quit_no_prompt`
   и baseline условий chat-only/all/no-export из `c-init.c:do_save_chat/quit_hook`.
   Получить полный источник до destruction, вернуть success/cancel/error в
   сохранённый C014 parent без duplicate shutdown. History save самостоятельно
   выполняет C014/C003-002 и не зависит от export choice/error. C014 находится
   раньше C029: подключить уже готовый parent; обратный canonical prerequisite
   не добавлять. До реального child совместный exit-log evidence остаётся pending.
4. Дать C032 тот же export child для final-review/tomb: snapshot живёт до результата,
   возврат идёт в исходный parent; exporter не закрывает final-review или tomb сам.
   Generation/account cancellation освобождает только owned operation resources;
   поздний результат не публикует export/успех для новой сессии.

## Готовность и минимальные Linux core checks

Actual message/character actions создают baseline bytes в shared owner; message
экспорт включает применимые строки старше шести live строк. Минимально проверить
real producer→generated collision, explicit replace/cancel и write/replace failure
с неизменным прежним destination, stale continuation/temp cleanup. Для CLI exit-log
проверить actual flags→full source→child→publication→C014 return, независимость
history save и отсутствие CFG/OPT autosave. Для C032 caller финальный совместный
flow подтверждается при его появлении, до этого FI-07 pending. C003 helper PASS
не заменяет эти consumer проверки; formatting source validation не native evidence.
Готовность этого follow-up не означает готовность всех outcomes C029.

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

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.

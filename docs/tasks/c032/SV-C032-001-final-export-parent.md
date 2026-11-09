# SV-C032-001 — Final review/tomb: подключение export child

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: implementation follow-up к плану C032; связка FI-07.
Основание: решение пользователя 2026-11-09 — C003 core и существующие consumers
сейчас; отсутствующие flow consumers/evidence завершаются в ответственных этапах.
Эта задача самодостаточна; общий индекс связок не нужен для её выполнения.
Это не полный план C032, не новые capability IDs и не перенос ownership.

C032 владеет production final review, close-final-review и close-tomb parent
interactions. Formatter/export child реализует
[C029](../c029/SV-C029-001-real-export-publication.md), безопасную публикацию —
[SV-C003-001](../c003/SV-C003-001-file-owners.md), затронуто
`capability.files.export-collision` и применимые resource/transient obligations.
Не создавать второй exporter, файловый writer или shutdown owner в C032.

## Работа

1. Подключить настоящие final-review/tomb export actions к C029 child, передав
   актуальный final character/message snapshot, account, generation и parent token.
   Держать final data живыми до завершения дочернего interaction, даже если
   transport уже закрыт; message source — full C016 recall, не live feed.
2. Child использует C003 generated/explicit publication в shared `U/user`:
   generated collision→free suffix, explicit existing→replace/cancel; write/replace
   failure не портит прежний файл. C032 потребляет semantic result, не повторяет
   formatter/temp/rename/remove алгоритмы. Проверить применимые baseline final
   dump сценарии по `src/client/c-files.c` и final-scene owners при реализации.
3. Success/cancel/error child возвращает к тому же final-review/tomb parent;
   повторный redraw не запускает экспорт, отмена export не закрывает parent.
   Явные close-final-review/close-tomb завершают parent ровно один раз. При relog,
   account/generation change отменить owned continuation; запоздалый result
   не изменяет новую сессию, final data/temp освобождаются своими owners.

## Готовность и минимальные Linux core checks

Production final-review и tomb actions проходят C029→C003 с actual final bytes
и destination; проверить применимые generated/explicit collision, cancel/error
return в правильный parent, final snapshot lifetime и однократное закрытие.
При смене поколения нет stale publication/успеха; failed write сохраняет target,
cleanup не удаляет history или чужие файлы. Использовать существующие core paths,
переиспользовать C003 filesystem evidence и добавить только caller/lifecycle cases.
Без настоящего final/tomb caller FI-07 остаётся pending, тест helper не закрывает её.
Готовность этого follow-up не означает готовность всех outcomes C032.

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

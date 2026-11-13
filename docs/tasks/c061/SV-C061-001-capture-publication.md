# SV-C061-001 — Screenshot producers, safe publication и open-last

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: implementation follow-up к плану C061; связка FI-08.
Основание: решение пользователя 2026-11-09 — C003 core и существующие consumers
сейчас; отсутствующие flow consumers/evidence завершаются в ответственных этапах.
Эта задача самодостаточна; общий индекс связок не нужен для её выполнения.
Это не полный план C061, не новые capability IDs и не перенос ownership.

C061 владеет `screenshots.capture`, `screenshots.server-trigger` и
`screenshots.open-last`. Подключить настоящие capture producers к
[SV-C003-001](../c003/SV-C003-001-file-owners.md):
`capability.files.export-collision`, `capability.files.resource-overlay`,
`capability.files.transient-owner`. Screenshot renderer/format/trigger принадлежат
C061, resolved owner/publication/temp lifecycle — C003; C029 formatter не требуется.

## Работа

1. В native capture action и применимом server-trigger path получить реальный
   capture payload текущего renderer/semantic screenshot format. Сохранить
   baseline content/glyph conversion/trigger semantics; сверить
   `src/client/c-files.c:xhtml_screenshot` и production server-trigger owner.
   Не использовать fixture image или terminal fallback для доказательства caller.
2. Передать payload в C003 publication с destination shared `U/user`, generated
   либо explicit request. Generated collision выбирает свободный suffix;
   explicit existing destination ждёт replace/cancel через реальный interaction.
   Ошибка записи/замены оставляет прежний файл. Renderer/encoder не выполняет
   обходной direct overwrite, файловый алгоритм C003 не копировать.
3. Хранить last-successful screenshot reference только после подтверждённой
   публикации; open-last открывает именно его. Cancel, encode/write/replace failure
   не меняют reference и не дают ложного success. Capture operation и continuation
   привязаны к generation/owner; stale completion не становится новым last image.
4. На cancel/relog cleanup через C003 удаляет только operation-owned temp/helper;
   не удаляет ранее опубликованный screenshot, чужие файлы и persistent history.
   Асинхронный encoder возвращает owner-thread result без mutation session state.

## Готовность и минимальные Linux core checks

Реальные native и server-trigger capture paths публикуют ожидаемый output в shared
owner. Минимально проверить generated suffix и explicit replace/cancel, failed
encode/write/replace с сохранением прежнего target/reference, open-last после
успеха и после отказа, stale completion и cancel/relog cleanup. Проверять actual
file/content/reference через production API; helper-only publication PASS не
закрывает capture flow. Применимые renderer/format ветви не скрывать фиктивным
snapshot; unavailable ветвь фиксировать отдельно без false success.
Готовность этого follow-up не означает готовность всех outcomes C061.

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

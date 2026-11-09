# SV-C028-001 — Принятый chat input и account history

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: implementation follow-up к плану C028; связка FI-04.
Основание: решение пользователя 2026-11-09 — C003 core и существующие consumers
сейчас; отсутствующие flow consumers/evidence завершаются в ответственных этапах.
Эта задача самодостаточна; общий индекс связок не нужен для её выполнения.
Это не полный план C028, не новые capability IDs и не перенос ownership.

C028 владеет send/cancel/history chat interaction и применимыми chat
преобразованиями. Использовать production account history owner из
[SV-C003-002](../c003/SV-C003-002-history.md):
`capability.files.history-load`, `capability.files.history-save`.
C014 владеет quit/reconnect/portal parent, C028 завершает evidence реального ввода.

## Работа

1. Подключить настоящий native chat editor/action к C003 history read/accept API.
   Account history имеет baseline `MSG_HISTORY_MAX=2000`, wrap/order/search и
   duplicate move-to-newest; endpoint `SvTextHistory=16` не заменяет этот owner.
   Не переносить ring/file алгоритмы в chat UI и не сохранять любой text field.
2. Записывать принятый несекретный chat input ДО chat mode/item/slash преобразований
   и `Send_msg`, независимо от дальнейшей отправки: локальная команда или отказ
   сетевой отправки не убирает уже принятый ввод. Сверить
   `src/client/c-util.c:askfor_aux` и `c-cmd.c:cmd_message`; history содержит
   исходную baseline строку, не transformed wire payload. Public input здесь
   означает разрешённый non-secret chat field, не только глобальный канал.
3. Escape/cancel не добавляет запись. Endpoint/account/password/archive/credential
   fields не становятся chat history; секреты не попадают в export/diagnostics.
   Сохранить baseline empty/encoding/byte-boundary поведение, не вводить guessed
   charset или зависимость remember от успешного socket send.
4. Связать реальный flow с begin-account/load и C014 finish-account/exit/relog/save.
   Load и save используют `S/chathist-<account>.tmp`, `S=U/sv`; legacy history
   автоматически не читать. Смена account не смешивает rings; history `.tmp`
   не disposable temp. Settings Save не нужен, CFG/OPT autosave не добавлять.

## Готовность и минимальные Linux core checks

Реальный load→chat accept/cancel→exit/relog→reload через production interaction
и lifecycle APIs даёт ожидаемые bytes/path/order. Минимально проверить wrap и
move-to-newest, поиск по загруженной истории, два account, локальную команду/случай
неотправки после acceptance, cancel и secret exclusion. Отказ чтения/записи даёт
bounded результат/диагностику без ложного save; legacy file и CFG/OPT неизменны.
Не заменять эту связку вызовом history helper из fixture. Проверенные C003 core
cases не повторять без причины; добавить только недостающее consumer evidence.
Готовность этого follow-up не означает готовность всех outcomes C028.

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

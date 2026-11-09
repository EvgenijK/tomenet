# C005 — базовые механизмы игрового ввода

Статус: **accepted**, 2026-11-09, по решению пользователя. План подготовлен по запросу пользователя с анализом
тремя агентами: keymap, quantity и raw-key. Core реализован по поручению пользователя, [Linux evidence](sv-c005-evidence.md)
подготовлено; [пользовательская приёмка](sv-stage-c005-acceptance.md) подтверждена.
Canonical runtime registration остаётся отдельным workflow. [Задачи](tasks/c005/README.md),
[точный planning snapshot](tasks/c005/coverage.json).

## Результат и границы

Этап развивает production input/core SV поверх B и принятого C004. Ровно три
canonical outcomes и девять obligations сохраняют IDs, allocation, conditions и
prerequisites. Snapshot копирует canonical rows без изменения; текущая платформа
проверки — Linux, хотя исторические canonical conditions содержат Windows.

| Outcome | Результат | Единственный ticket owner |
|---|---|---|
| `capability.input.keymap` | Normal/roguelike command+direction, prefixes и preference mappings | SV-C005-001 |
| `capability.input.quantity` | Количество: defaults, shortcuts, пределы и результат отмены | SV-C005-002 |
| `capability.input.raw-key` | Unhandled gameplay key в exact raw-key packet, reserved no-ops исключены | SV-C005-003 |

Элементы UI, новые игровые действия, полноценный macro runtime и перенос всех
legacy callers не включаются скрыто в C005. Core primitives проверяются на своём
production-пути. Проверка конкретного item/store/request/action результата остаётся
в этапе его владельца; primitive evidence его не принимает. Поздние интеграции
не становятся обратными prerequisites C005, а их evidence не заявляется полученным.

## Нормативная основа и текущий код

Применяются [AGENTS](../AGENTS.md), [CONTEXT](../CONTEXT.md),
[settings policy](capabilities/settings-policy.md),
[ADR-0001](adr/0001-separate-session-interactions-and-ui.md),
[ADR-0002](adr/0002-main-thread-state-ownership.md),
[ADR-0003](adr/0003-explicit-module-connections.md),
[ADR-0005](adr/0005-session-scoped-work.md),
[ADR-0006](adr/0006-errors-and-overload.md) и
[правило платформ](agents/platform-validation.md).

`src/client/sv/input/input.[ch]` уже содержит request context, очередь и ограниченные
однобайтовые/physical bindings. Это не полный command keymap и не macro runtime.
`app.c:sv_app_physical` сейчас ведёт gameplay key в `sv_app_raw_key`; protocol
уже предоставляет `sv_protocol_raw_key`. `settings/preferences.[ch]` хранит options,
mappings и source records C004. Quantity primitive нужно выделить по baseline,
а не подменять существующим key-request reply. Source contract, точные ветви и
места минимального изменения указаны в задачах 001–003; строки исходников нужно
пересверить при реализации.

## Реализация и порядок

1. **001 — keymap.** SV-local owner с явным preference interface, command/direction
   result и незавершённым prefix state. Normal/roguelike translation, backslash
   bypass, control prefix и no-op различаются. PRF reload использует C004 loader,
   сохраняет option/S occurrence order и baseline load/init precedence.
2. **002 — quantity.** Независимый неблокирующий interaction core с caller default,
   ограничениями, shortcuts и однократным результатом. Escape возвращает 0,
   подтверждённое пустое поле использует default; значение 0 интерпретирует caller.
3. **003 — raw-key.** После результата keymap подключить существующий app/protocol
   путь. Проверить реальные bytes и отсутствие отправки no-op, prefix и клавиш
   чужого input context. Не считать каждую распознанную команду raw key.
4. **004 — evidence.** После 001–003 собрать компактный Linux отчёт по девяти
   exact obligations и явно указать evidence собственных результатов и late routes.

002 можно реализовывать независимо от 001; 003 зависит от согласованного результата
001. Не требуется общий framework ввода или общий refactor legacy/common.
Минимальные изменения и адаптации размещаются в SV; локальное дублирование допустимо.
Новые возможности улучшения legacy/common фиксируются отдельно в
[списке улучшений](sv-improvements.md), без включения в реализацию C005.

Известный дефект legacy `S` читает command/direction из первого поля. Default
C005 — совместимость с фактическим baseline. Intended `S:key:command:direction`
не включать незаметно: отдельное решение по [существующей записи](sv-improvements.md#correct-legacy-s-keymap-record-field-parsing).
Это не блокирует реализацию совместимого варианта. C004 остаётся accepted;
необходимый узкий occurrence API и исправления consumer выполняются в C005.

## Минимальная проверка и готовность

Только необходимые core cases через production SV: реальные command/direction,
quantity results и wire bytes, с interleaved updates и reset/teardown текущего
core. Focus/resize/redraw не исполняют команду повторно. Prefix/interaction не
останавливают сеть/UI, stale generation не отправляет команду. Подробный минимум
и baseline corner cases — в каждой задаче. Не создавать тестовую копию поведения,
новый harness, полную visual matrix или реальные аккаунты. Расширение набора
проверок согласовывать; Windows/MinGW/Wine deferred.

`result`, `lifecycle` и `allocation-checks` каждого из трёх IDs рассматриваются
отдельно. Общие формулировки obligations применяются к фактическому caller и его
операции; они не требуют файлового fault framework для чистого numeric core.
Не объявлять отсутствие позднего caller доказательством его успеха.

C011/C014 завершают actual session preparation/handoff/close, C053 — macro-derived
route. C017, C022 и C027 принимают соответственно server-request, item и store
интерпретацию quantity; другие callers сохраняют owners своего этапа. Их конкретные
проверки не являются core readiness gate C005. Отчёт сохраняет эти границы, не
повышая canonical registry statuses автоматически. Пользовательская приёмка и
native evidence registration — отдельные действия по фактическому результату.

## Результат реализации — 2026-11-09

Задачи 001–004 выполнены в границах этого плана. Production keymap/preferences,
quantity/text-field и app/native dispatch проверены минимальными core cases под
ASan/UBSan; Linux production build прошёл. Фактические команды, bytes, lifecycle
и disposition всех девяти obligations — в [evidence](sv-c005-evidence.md).
Canonical JSON не менялся, C004 остаётся accepted, late gameplay/macro/session
flows остаются обязанностью своих этапов. C005 и задачи 001–004 приняты пользователем: [решение](sv-stage-c005-acceptance.md).

# SV-LOGIN-ARCH-001 — Определить interface и владельцев состояния входа

**Статус:** implemented; адресные Linux checks passed, принято пользователем 2026-11-04.
**Дата:** 2026-11-04.
Labels: enhancement

**Зависимости:** нет; первый шаг плана [архитектуры входа](README.md).

## Результат и scope

По текущему production-пути `endpoint-run.[ch]`, `session/pregame.[ch]`,
`input/login-interaction.[ch]`, `input/native-endpoint.[ch]`,
`session/birth.[ch]` и `ui/endpoint-scene.[ch]` описать глубокий модуль входа:
commands, read-only view и смысловой результат, скрывающие orchestration.
Не закреплять заранее конкретные C-функции или новую enum всех состояний.

Для каждого изменяемого состояния указать владельца, lifetime и потребителей:
попытка соединения/generation, credentials и character drafts, birth interaction,
protocol state, input focus/editing, view и диагностика. Отдельно описать back,
cancel, retry/name recovery, ошибку и teardown; определить lifetime borrowed data.
Зафиксировать таблицу файлов к переносу/сохранению и минимальный набор checks.

## Текущий контракт и ownership

Контракт отражает реализацию от 2026-11-04; результаты адресной проверки — в [005](SV-LOGIN-ARCH-005-integration.md).

| Состояние | Владелец |
|---|---|
| Выбор endpoint, credentials buffers, retry и name recovery между попытками | login flow |
| Socket, wire buffers, generation попытки, protocol login, pregame, birth и запрос сохранения credentials | попытка внутри flow |
| Смысловой input context, редактируемые drafts и подтверждения | соответствующее interaction внутри flow |
| Hover, viewport, scroll, native keyboard latch и геометрия | native input/presentation |
| Window, renderer, font и общие ресурсы | приложение |

Публичные операции `login/flow.h` — create/destroy, begin попытки, poll,
command и view. Private native adapters в `flow-internal.h` доставляют результаты
своих interactions и завершают tick/attempt синхронно. Flow не получает
renderer или font object: если протоколу нужно имя font resource, передаётся имя.
Не вводить новую I/O абстракцию только ради этого переноса.

Различать всю flow и отдельную попытку: recovery draft может пережить teardown
попытки по текущим правилам; socket и устаревший ввод — нет. Заимствованное view
не переживает mutation/teardown. При смене редактора сохраняется retirement
queued native input; native слой исполняет Start/StopTextInput.

Будущий handoff должен учитывать socket, generation, version/setup, выбранного
персонажа, уже декодированные startup данные, unread bytes из login и wire,
а также pending sends. Нынешний startup milestone не равен передаче владения:
текущий cleanup закрывает socket. Реализацию передачи выполнять с реальным
runtime consumer отдельной задачей.

## Критерии завершения

- [x] Показан один production-путь от выбора сервера до текущего результата входа;
      caller не управляет внутренними переходами flow.
- [x] Main-thread ownership, отсутствие блокирующего ожидания, explicit connections
      и generation rules соответствуют ADR-0001…0006.
- [x] Семантика flow, protocol и UI focus не слита механически; старые переходы
      переносятся без второй state machine.
- [x] Описан будущий однократный handoff ресурсов и bytes без реализации live runtime.
- [x] Перечень переноса оставляет protocol/utilities/vault самостоятельными;
      C001 и текущая диагностика учтены как consumers, не дублируются.
- [x] Для 002…005 указаны адресные существующие Linux production-path checks и
      необходимые adapters; новые checks не добавлены без согласования.

## Проверка и exclusions

Это документальная задача: сопоставить описанные переходы и владельцев с текущими
production callers и существующими проверками, не требовать сборку для текста.
Исходники, CLI, server/legacy/common, новые I/O adapters, capability IDs и live
handoff не менять. ADR менять только при реальном изменении принятого решения;
группировка файлов сама по себе не отменяет разделение обязанностей.

## Реализация и evidence — 2026-11-04

- Mutable `SvLoginFlow` скрыт публичным forward declaration; его owned attempt
  хранится в `login/flow-internal.h` для внутренних adapters.
- `login/options.h` сохраняет существующий runner/result contract;
  startup outcome остаётся диагностическим результатом, не живым соединением.
- Renderer получает отдельный `SvEndpointView`, birth — opaque interaction.
  Публичная `SvLoginFlowView` содержит semantic status и character/birth projections;
  детализация private native interface не объявляется внешним контрактом.
- Linux build и адресные runtime/core checks прошли; команды, результаты,
  review и ограничения перечислены в интеграционном тикете 005.

Итоговая проверка и ограничения: [005 — интеграция](SV-LOGIN-ARCH-005-integration.md).

Приёмка: пользователь принял правки 2026-11-04 и поручил commit/push.

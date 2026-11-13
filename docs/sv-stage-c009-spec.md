# C009 — startup Lua reload

Статус: **accepted** — Linux core, 2026-11-10, по решению пользователя.
C009 и задачи 001–003 завершены: [приёмка](sv-stage-c009-acceptance.md).
План подготовлен по запросу пользователя с тремя
независимыми анализами: canonical scope, production baseline/SV, вопросы и задачи.
Production core реализован агентами по поручению пользователя:
[Linux evidence](sv-c009-evidence.md). Поздняя live integration обязательна C010/C011/C014 без обратного gate C009.
[Задачи](tasks/c009/README.md), [источники](tasks/c009/source-contract.md),
[точный planning snapshot](tasks/c009/coverage.json).

## Что входит

C009 владеет одним outcome `capability.lua.reload` и тремя obligations:
`obligation.lua.reload.result`, `.failure-lifecycle`, `.allocation-checks`.
Canonical prerequisite — `capability.files.resource-overlay` (C003).

1. Реальный Lua startup runtime в SV: загрузить действующие скрипты, передать
   server feature flags, подготовить данные школ/заклинаний; при reload освободить
   прежние данные и повторить штатную последовательность.
2. Startup loader и вложенные `pern_dofile` читают скрипты через shared user
   overlay с bundled fallback. Невалидный пользовательский скрипт сообщает
   ошибку, а не маскируется bundled версией. Native Lua `dofile`/`dostring`
   сохраняют семантику baseline, без перехвата этим resolver.
3. Предоставить production API для reload при завершении последней передачи
   файлов. API также принимает successful INIT для немедленного сохранения
   process-sticky audio/Guide flags, в том числе при disconnect до END.
   C010 вызывает этот API; повторная отрисовка не повторяет загрузку.
4. После обновления `guide.lua` получить новые race/class/skill/school/spell
   metadata через настоящий Lua runtime. Это внутренние данные, а не готовый Guide.
5. После обновления `audio.lua` показать рекомендацию перезапуска. Работающие
   audio mappings/backend автоматически не обновляются.
6. Возвращать явные результаты загрузки/ошибки и соблюдать ownership/lifecycle.
   Не выдавать transfer failure или stale completion за успешный startup.

До C009 в SV не было Lua runtime: `settings/audio.c` читает только буквальные таблицы.
Одного подключения `c-script.o` недостаточно: его bindings зависят от legacy globals.
Реализована SV-local адаптация над существующим vendored Lua engine,
без нового языка, regex-подмены исполнения или затягивания legacy UI.

## Три задачи

| Задача | Результат | Canonical owner |
|---|---|---|
| [SV-C009-001](tasks/c009/SV-C009-001-startup-runtime.md) | Lua runtime, startup scripts, overlay, flags, reload/cleanup и ошибки | `capability.lua.reload` |
| [SV-C009-002](tasks/c009/SV-C009-002-reload-lifecycle.md) | Completion seam, Guide metadata, audio warning и lifecycle | 0 новых IDs; часть результата 001 |
| [SV-C009-003](tasks/c009/SV-C009-003-evidence.md) | Минимальное Linux evidence и disposition трёх obligations | 0 новых IDs |

Порядок: 001 → 002 → 003. Во время 001 зафиксировать реальный closure scripts и
bindings, необходимый для штатных `init_lua/open_lua/reopen_lua`. Все вызываемые
startup bindings исполняют production-семантику; заглушки, фиктивные schools/spells
или пустые успешные callbacks не принимаются. Gameplay UI/actions остаются поздним
этапам; необходимые для startup данные входят в C009.

## Границы этапов

- C010 владеет CHECK/INIT/DATA/END/upload, checksums/version branches, временными
  файлами, публикацией и ACK/ERR. Он подключает реальный FILE_END к C009 и проверяет
  сквозной результат. C009 не создаёт второй transfer decoder.
- C011 передаёт реальные server features и использует результат Lua при session
  preparation. Он проверяет actual startup readiness и handoff; C014 — teardown.
- C002 хранит `-u`; C010 отвергает по нему все FILE_INIT как baseline. Флаг не
  запрещает чтение уже имеющихся локальных Lua scripts.
- C053 — macro runtime; C058 — явное исполнение локального Lua (`//` и другие
  существующие callers). Наличие engine не принимает эти capabilities.
- C055 — audio runtime. C062–C064 — настоящий Guide. Если текущий caller открывает
  Guide, он показывает ровно `The guide is in development`, без content,
  bookmarks, network/update операций, и возвращается с сохранённым interaction.

Manifest упоминает preference lifecycle; это не разрешение добавлять новую
кнопку Reload или исполнять PRF/локальные команды C058 в C009.

## Решение об ошибке последней передачи

Решение пользователя от 2026-11-10: «сохраняем поведение старого клиента».
После последнего FILE_END при нулевом числе transfers выполняется попытка reload
текущих файлов даже при ошибке сохранения. На диске может оказаться старый либо
частичный файл; reload также может завершиться ошибкой.
Transfer result и reload result учитываются отдельно: успешное перечитывание
не превращает ошибку передачи в успешное обновление или готовность startup.
C010 сохраняет ACK/ERR по результату передачи, C011 обрабатывает явные результаты.
[Источники и решение](tasks/c009/source-contract.md). Вопрос закрыт.

## Проверка и готовность

Только Linux build и необходимый минимум core cases через настоящий production
Lua owner/reload API. Минимум описан в 003; расширение проверок согласовывается
по AGENTS. Windows/MinGW/Wine отложены. Применяются CONTEXT и ADR-0001–0006:
явные связи, main-thread ownership, generation для session completion, смысловые
ошибки и отдельные lifetimes shared ресурсов/session данных.

Core evidence не доказывает отсутствующий FILE_END consumer C010 или startup
handoff C011. Для каждого из трёх obligations отдельно записать проверенное и
непроверенное; будущие flows не получают PASS от прямого API вызова.
Пользователь принял core-границу 2026-11-10: [решение](sv-stage-c009-acceptance.md).
Canonical JSON,
allocation, prerequisites и pending статусы не меняются от подготовки плана.
Изменения существующего клиента минимальны; улучшения вне C009 записываются
отдельно в [список улучшений](sv-improvements.md).

## Результат реализации — 2026-11-10

Реализованы `src/client/sv/lua/runtime.[ch]`, `constants.c`, `reload.[ch]`,
изолированная сборка engine в makefile.sv и application lifetime/early init в
native-run. Штатная bundled цепочка: 41 загрузка, 25 школ, 310 заклинаний.
Проверены overlay/getters, visible nested failures, typed data/retry/cleanup,
INIT/END/sticky generation и согласованный failed-last-close reload.
Linux build, одна focused ASan/UBSan core проверка и bounded native startup
прошли; [команды и disposition всех obligations](sv-c009-evidence.md).
Actual C010/C011/C014 consumers остаются самостоятельной обязательной работой
[задач следующих этапов](tasks/c009/README.md#самостоятельные-обязательства-следующих-этапов),
не блокируют и не переоткрывают C009 автоматически. C009 и задачи 001–003 приняты
пользователем в текущих границах core; canonical JSON не менялся.
Source contract сохраняет исходную сверку и actual closure.

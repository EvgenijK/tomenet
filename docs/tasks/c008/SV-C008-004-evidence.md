# SV-C008-004 — минимальное Linux evidence и сверка всех obligations

Статус: accepted пользователем 2026-11-09 в текущих границах Linux core.
Labels: enhancement
Owned IDs: 0 IDs; сводка шести outcomes без повторного allocation.
Зависимости реализации: 001–003; точный snapshot 6 outcomes/18 obligations.

## Основа и границы

Нормативны [спецификация C008](../../sv-stage-c008-spec.md),
[source contract](source-contract.md), [точный planning snapshot](coverage.json),
[CONTEXT](../../../CONTEXT.md), [архитектура](../../sv-architecture.md), ADR
[0001](../../adr/0001-separate-session-interactions-and-ui.md),
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).
Сохранять все canonical conditions/prerequisites/obligations. Все шесть rows
имеют пустой canonical prerequisite list; зависимости реализации ниже его не меняют.
Существующие production protocol/app/input owners дорабатываются в SV; второй
network engine, legacy Terms и общий refactor не нужны. Необходимые отдельные
улучшения legacy/common/SV записывать в [список улучшений](../../sv-improvements.md).

## Работа

1. Подготовить `docs/sv-c008-evidence.md`: actual callers, revision, команды,
   expected/actual wire bytes, control/input/frame ordering и session ownership.
   Для каждого из 18 obligations — отдельная disposition с evidence либо
   конкретным ограничением. Source review не является native runtime evidence.
2. Переиспользовать focused production-core cases 001–003 и применимую Linux
   сборку. Разделять decode/core, native presentation/input integration,
   настоящие caller flows и отсутствующий macro C053 consumer.
3. Сверить exact snapshot с canonical hashes/rows, unique ticket owners, локальные
   ссылки, границы B/C007/C011/C013/C014/C053 и unresolved questions. Canonical
   registry не менять, не приписывать PASS уже существующим primitive флагам.
4. Не выдавать full runtime PASS неподтверждённым caller queue либо waiting
   macro outcomes. Пользователь принял текущий core; late integration evidence
   обязателен следующим owners и не блокирует C008. Старые принятые этапы
   автоматически не открывать.

## Минимальная проверка и завершение

Завершение: reviewable отчёт с раздельным checked/pending по всем 18 obligations,
минимальными воспроизводимыми командами и явными late-flow limits. Не запускать
историческую cross-platform matrix и не добавлять новый verification процесс.

Проверки — только необходимый минимум core через production public path на
[Linux](../../agents/platform-validation.md), с переиспользованием существующих
entrypoints. Fixtures подают bytes, время и ввод, но не реализуют поведение
вместо production. Новый общий harness, широкая matrix и stress/soak не входят;
расширение набора согласуется заранее по AGENTS.md. Windows/MinGW/Wine deferred.
Implementation, runtime evidence, пользовательская приёмка и canonical registry —
отдельные состояния. Пользовательская приёмка core зафиксирована ниже;
непроверенные runtime flows сохраняют свой статус.

## Результат — 2026-11-09

Подготовлен [отчёт](../../sv-c008-evidence.md) с отдельной disposition всех 18
exact obligations, командами и Linux logs, core readiness и late caller gaps.
Build, focused production ASan/UBSan checks и bounded native smoke прошли;
canonical rows/hashes и локальные ссылки сверены. Кодовый граф обновлён.
Полный native-runtime PASS не заявлен. Core C008 принят пользователем
последующим решением ниже.

## Решение пользователя — 2026-11-09

[C008 и задачи 001–004 завершены и приняты](../../sv-stage-c008-acceptance.md)
в текущих границах core. Предыдущие ограничения полного runtime evidence
сохраняются как факты, но не являются gate приёмки C008. Оставшаяся работа
обязательна следующим owners:

- [C011-002](../c011/SV-C011-002-network-control-preparation.md): подготовка control owner/options и handoff contract.
- [C013-001](../c013/SV-C013-001-network-control-live-map.md): реальный map ingress и control-driven submission.
- [C014-003](../c014/SV-C014-003-network-control-lifecycle.md): actual handoff/lifecycle и deferred incoming caller queue.
- [C053-001](../c053/SV-C053-001-preference-runtime.md): настоящий macro WAIT/XWAIT confirmation consumer.

Их доработки и проверки не блокируют и не переоткрывают C008 автоматически;
evidence backlink служит трассировке. Полный live-runtime PASS не выдан, canonical
allocation/obligations/statuses не изменены. Плановые критерии выше читаются
с учётом этого более позднего явного решения пользователя.

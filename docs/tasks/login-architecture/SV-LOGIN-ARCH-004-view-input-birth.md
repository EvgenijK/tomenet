# SV-LOGIN-ARCH-004 — Разделить view и input; скрыть backing birth state

**Статус:** implemented; адресные Linux checks passed, принято пользователем 2026-11-04.
**Дата:** 2026-11-04.
Labels: enhancement

**Зависимости:** [003](SV-LOGIN-ARCH-003-flow.md).

## Результат и scope

Отделить read-only представление входа от mutable native input/editing state:
renderer больше не получает `SvEndpointInput` как контейнер всей сцены.
Hit testing остаётся в native input и использует ту же logical geometry, что draw;
focus и редактирование не переходят в протокол или renderer.

Заменить forwarding-only `input/birth-interaction.h` настоящим interface birth
interaction в размещении 002. Сохранить production birth implementation и правила;
caller не включает backing header и не читает mutable state для решений за модуль.
Скрыть только действительно внутренние детали: не создавать новый слой wrappers
или набор однооперационных getters ради формальной инкапсуляции.

## Критерии завершения

- [x] UI читает view с явным lifetime; borrowed pointers не переживают mutation,
      teardown или смену generation; между кадрами соблюдается ADR-0004.
- [x] Native input хранит editing/focus, передаёт смысловой ввод; draw и hit test
      сохраняют согласованную геометрию и существующее B2 поведение.
- [x] Birth interaction предоставляет выборы/view/результат через небольшой interface;
      production callers не зависят от его mutable backing state.
- [x] Правила допустимости, back/cancel, случайный выбор/DNA и final commit сохранены.
- [x] Runner и диагностика используют тот же interface; отдельной test implementation нет.

## Минимальная проверка

Linux SV build; адресные существующие production-path core checks birth/view-input
по выбранным в 001 затронутым переходам. Переиспользовать только необходимые
режимы `tests/sv/birth-single-submission.c` и native input проверки, выбранной
по фактическому diff; один targeted Linux visual/input smoke затронутого экрана,
если поменялся renderer/input contract. Без новой screenshot suite и широкой
геометрической matrix; расширение checks заранее согласовать.

## Вне scope

Новый UX, изменение birth rules, general input router redesign, split birth по
числу строк, CLI, live handoff, архитектура legacy/server/common.

## Реализация — 2026-11-04

- Renderer принимает `SvEndpointView` из `login/ui/view.h` отдельно от mutable
  `SvEndpointInput`; возвращает render metrics для viewport без изменения
  смысловых drafts. Borrowed frame projection не переживает mutation.
- `birth-interaction.h` содержит opaque `SvBirth`, прежние commands/view/validated
  committed result и create/destroy. Full struct находится в private `birth.h`,
  используемом core и native adapter внутри feature; внешние production consumers
  не читают backing state.
- `sv_birth_snapshot_prepare` копирует только существующие diagnostic/lifecycle
  значения и массивы в самостоятельный `SvBirthSnapshot`, без borrowed setup
  или возможности восстановить mutable interaction. Snapshot обнуляется перед
  копированием; renderer использует `SvBirthView`, transport — `SvBirthPlayDraft`.
- Native adapter сохранён рядом с core и может использовать private state;
  дополнительные getters/note-input wrappers ради запрета внутренних reads
  не вводятся. Алгоритмы birth не дробились и не дублировались.

Синтаксическая проверка `birth.c` с `-Wall -Wextra -Werror` прошла;
Linux build и адресные runtime/input проверки прошли. Полный перечень
результатов, исправлений и ограничений приведён в 005.

Итоговая проверка и ограничения: [005 — интеграция](SV-LOGIN-ARCH-005-integration.md).

Приёмка: пользователь принял правки 2026-11-04 и поручил commit/push.

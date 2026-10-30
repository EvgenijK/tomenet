# SV-B2-001 — Shell D, графика и шрифтовые роли

Статус: accepted, 2026-10-30, в составе принятого пользователем B2;
[итоговое решение](../../sv-stage-b2-acceptance.md).

## Реализация и требования до приёмки

Реализация: implemented, 2026-10-28. Linux build/checks и два независимых code review завершены; открытых замечаний нет.
Evidence: [B2-001](../../sv-b2-001-evidence.md). Полная приёмка B2 остаётся за B2-006.
Зависимости: —.
Существующие semantic consumers: B-001; все B startup consumers.
Новых capability IDs: 0; B ownership и protocol semantics сохраняются.

Общий контракт: [B2 spec](../../sv-stage-b2-spec.md),
[reference и screen mapping](reference.md), [DAG](README.md).
Linux обязателен; Windows/MinGW/Wine deferred.

## Пользовательский результат

Первый production endpoint screen уже выглядит как D: фон, большой animated ASCII
logo, panels/side areas и управляющие элементы составляют цельный интерфейс.
Resource-only результат без видимого production consumer не закрывает задачу.

## Работы

- Зафиксировать локально воспроизводимый reference по reference.md: actual hashes,
  финальный CSS cascade, selected backgrounds/font/capture state. Не требовать clean
  всего внешнего repo и не менять его.
- Перенести необходимые 19 PNG, cell-logo/torches data и provenance в SV-owned
  resources; выбрать существующий доступный image decode path либо минимальную
  зависимость по реальному consumer. Packaging не зависит от sibling web checkout.
- Ввести только необходимые palette/type/spacing/resources и concrete layout
  structs. Итоговый D body — mono; другие роли следуют computed final styles.
  Сохранить API/поведение A font callers или использовать B2-local text helper.
- Реализовать shell wide/≤1240/короткая высота, panels/Chronicle/realm links
  composition, clipping/scroll regions, анимацию на elapsed time. Hidden/minimized
  и reduced-motion не блокируют сеть/main loop. Loading/reset/resource teardown явны.
- Единая window/output/logical coordinate policy и измеренные rectangles для draw
  и hit testing; invalidation при geometry/font/scale/renderer/model изменениях.

## Production seam и ограничения

`ui/login-layout.[ch]`, конкретные B2 scene/resource/text/theme files внутри
`src/client/sv/ui/`, thin `endpoint-scene` facade, `endpoint-run.c`,
`src/makefile.sv` и SV resources. Имена новых файлов — предложение, не обязательный
каркас. Legacy/common, A game renderer и общий widget framework вне scope.

## Проверки и Definition of Done

Реальный endpoint scene отрисован на Linux software/OpenGL, visually сопоставлен
с тем же reference viewport; wide и minimum не теряют controls. Existing проверки
затронутых endpoint/font callers выбираются по diff. Layout/resources должны быть
доступны следующим tickets без второй coordinate/theme реализации.

Общие правила: минимальные SV-local changes; никакой отдельной реализации
поведения только для тестов. Дополнительные improvements фиксируются отдельно
в `docs/sv-improvements.md`, а не включаются автоматически в эту задачу.

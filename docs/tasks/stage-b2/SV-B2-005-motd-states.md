# SV-B2-005 — MOTD и общие wait/error/child states

Статус: accepted, 2026-10-30, в составе принятого пользователем B2;
[итоговое решение](../../sv-stage-b2-acceptance.md).

## Реализация и требования до приёмки

Реализация: implemented, 2026-10-29; Linux build/checks, software/OpenGL visual replay
и два независимых review завершены, открытых замечаний нет. [Evidence](../../sv-b2-005-evidence.md).
Зависимости: SV-B2-001.
Существующие semantic consumers: B-006, B-020, B-024, B-025.
Новых capability IDs: 0; B ownership и protocol semantics сохраняются.

Общий контракт: [B2 spec](../../sv-stage-b2-spec.md),
[reference и screen mapping](reference.md), [DAG](README.md).
Linux обязателен; Windows/MinGW/Wine deferred.

## Пользовательский результат

После server ACK игрок видит точный server-defined MOTD в отдельно утверждённой
композиции C; все native-only состояния используют согласованный visual language.

## Работы

- Реализовать MOTD variant C: полная 23-row композиция, decoded visible extent,
  исходные columns/colors/ASCII/unknown text; padding 24 logical, uniform maximum
  fit и центрирование. Без wrap/scroll/crop, клиентских headings и D overlays поверх.
- Existing close/any-key, skip, once-per-login, order после selection/creation и
  перед gameplay сохраняются; handoff не активирует gameplay actions.
- Общие loading/error/status/confirmation/Guide/quit компоненты, на которые
  ссылаются surface owners, не становятся новыми semantic owners. Разные caller
  cancel/back/retry не сводятся к universal reconnect.
- Native-only состояния, которых нет в HTML, получают конкретную documented
  adaptation в тех же controls/spacing/colors; не остаются plain text legacy scene.
- Reset/focus/resize/teardown не сохраняют stale overlay или секретный draft.

## Production seam и ограничения

SV MOTD presentation/layout внутри `ui/`, существующие pregame/login read views
и wiring `endpoint-run.c`. Никакого чтения legacy Term, нового packet decoder
или hardcoded welcome/news content.

## Проверки и Definition of Done

Сопоставить native MOTD с отдельным variant C на short/wide/colored/unknown
synthetic content, проверить selected complete composition и any-key/skip.
Использовать existing `sv_first_session_checks.py`, `sv_guide_motd_checks.py`
или `sv_birth_complete_checks.py` по изменённому caller. Existing outcomes не
выдаются за новый actual-provider или real-server evidence.

Общие правила: минимальные SV-local changes; никакой отдельной реализации
поведения только для тестов. Дополнительные improvements фиксируются отдельно
в `docs/sv-improvements.md`, а не включаются автоматически в эту задачу.

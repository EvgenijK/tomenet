# SV-B2-004 — Creation workspace, summary и choices

Статус: planned, 2026-10-28. Реализация не начата.
Зависимости: SV-B2-003.
Существующие semantic consumers: B-023, B-024.
Новых capability IDs: 0; B ownership и protocol semantics сохраняются.

Общий контракт: [B2 spec](../../sv-stage-b2-spec.md),
[reference и screen mapping](reference.md), [DAG](README.md).
Linux обязателен; Windows/MinGW/Wine deferred.

## Пользовательский результат

Полный native birth flow показан в композиции D: summary слева, выбор сверху,
detail снизу 60/40, настоящее выделение/доступность и рабочие stats/confirm controls.

## Работы

- Перенести summary и текущее состояние выбора, name/sex/race/class/trait/body/
  stats/mode views; отдельный step-list/progress widget не добавляется. Перенести
  choice cards/grids, descriptions, legal/disabled/selected/hover/focus states.
  Final CSS top/bottom split имеет приоритет над старым right-detail описанием.
- Native setup/interaction определяют order, skipped steps, legal/highlighted IDs,
  class/race build order, values и mode restrictions; JS creationData не model.
- Stats controls, unused-points confirmation, back/cancel/quit и actual random/DNA
  commands используют существующий production interaction path. DNA persistence
  C040 не добавляется; temporary Guide сохраняет exact text и действительный caller.
- Длинные choices/details/summary читаемы с independent scrolling и постоянными
  required controls; shared viewport/rectangles используются draw и input.
- Partial ACK/authoritative completion показываются как B2 waiting/completion
  states, без преждевременного MOTD или fake Play→gameplay перехода.

## Production seam и ограничения

B2 birth scene/layout, `input/native-birth.[ch]`, публичный
`input/birth-interaction.h`, read-only `SvBirthView` через существующую публичную interaction границу,
endpoint wiring. Прямой доступ UI к private backing state не расширяется.

## Проверки и Definition of Done

Reference/native capture каждой layout-distinct birth family; keyboard/mouse,
back/cancel и selected visibility при resize/scroll. Relevant subset existing
`sv_name_editor_submit_checks.py`, `sv_guide_motd_checks.py`,
`sv_birth_single_submission_checks.py`/`sv_birth_complete_checks.py` — по diff.
Не добавлять новую version matrix или отдельную test-only birth model.

Общие правила: минимальные SV-local changes; никакой отдельной реализации
поведения только для тестов. Дополнительные improvements фиксируются отдельно
в `docs/sv-improvements.md`, а не включаются автоматически в эту задачу.

# SV-B2-002 — Выбор сервера и account/auth forms

Статус: accepted, 2026-10-30, в составе принятого пользователем B2;
[итоговое решение](../../sv-stage-b2-acceptance.md).

## Реализация и требования до приёмки

Реализация: implemented, 2026-10-28. Реализация, code review и текущие Linux
проверки завершены; результаты и ограничения — в [evidence](../../sv-b2-002-evidence.md).
Зависимости: SV-B2-001.
Существующие semantic consumers: B-001, B-002, B-005, B-006, B-021.
Новых capability IDs: 0; B ownership и protocol semantics сохраняются.

Общий контракт: [B2 spec](../../sv-stage-b2-spec.md),
[reference и screen mapping](reference.md), [DAG](README.md).
Linux обязателен; Windows/MinGW/Wine deferred.

## Пользовательский результат

Игрок выбирает server card или direct address и входит/создаёт account через
видимые поля и кнопки макета, с корректными edit/focus/wait/error состояниями.

## Работы

- Карточки actual server list, selected/hover/focus, пустой список и factual ping
  pending/unavailable; manual host/port overlay использует существующий editor.
- Одновременно видимые account/password fields и visual buttons: Tab/click/focus,
  caret/selection, private mask/local reveal, primary submit/cancel/back; не создавать
  второй credentials editor или новый порядок protocol submissions.
- Read-only presentation для active field, draft/mask, validation и vault status;
  show password — private render-only, сброс при выходе, без retained secret copies.
- Contact/auth/new-account success/rejection/timeout/disconnect/provider unavailable
  используют общие B2 wait/error/status components. Actual baseline determines retry.
- Store/restore/session-only policy и server-confirmed save timing B сохраняются.
  Не добавлять credential management UI из external replace/delete test операций.
- Подключить мышь к endpoint/credentials через shared rectangles; любые look-alike
  действия HTML получают явную native availability, а не fake response.

## Production seam и ограничения

`input/native-endpoint.[ch]`, `input/credentials.[ch]`, существующие endpoint/text
field interactions, B2 server/account scene и `endpoint-run.c`. Protocol/vault/model
семантика не переписывается. Shared files интегрирует один назначенный владелец.

## Проверки и Definition of Done

Reference/native сравнение server, manual и account frames; native keyboard/mouse
submit/cancel/edit и representative scaled clicks. Переиспользовать по изменённому
пути existing `sv_endpoint_checks.py`, `sv_account_create_checks.py` или
`sv_account_failure_checks.py`; не запускать новый provider acceptance matrix.

Общие правила: минимальные SV-local changes; никакой отдельной реализации
поведения только для тестов. Дополнительные improvements фиксируются отдельно
в `docs/sv-improvements.md`, а не включаются автоматически в эту задачу.

## Результат

Production SV использует B2 server cards, manual overlay и одновременные поля
account/password с shared draw/hit geometry, caret/selection/focus, private reveal
и фактическими wait/error/vault состояниями. Protocol и server-confirmed save
порядок сохранены. Замечания Standards/Spec review исправлены; reference/native
кадры, команды и границы проверенного результата перечислены в evidence.

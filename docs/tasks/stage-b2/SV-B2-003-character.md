# SV-B2-003 — Character overview, slots и имя

Статус: accepted, 2026-10-30, в составе принятого пользователем B2;
[итоговое решение](../../sv-stage-b2-acceptance.md).

## Реализация и требования до приёмки

Реализация: implemented, 2026-10-29. Linux software/OpenGL, targeted production-path
проверки и независимые Standards/Spec review завершены;
[evidence](../../sv-b2-003-evidence.md).
Зависимости: SV-B2-002.
Существующие semantic consumers: B-006, B-022.
Новых capability IDs: 0; B ownership и protocol semantics сохраняются.

Общий контракт: [B2 spec](../../sv-stage-b2-spec.md),
[reference и screen mapping](reference.md), [DAG](README.md).
Linux обязателен; Windows/MinGW/Wine deferred.

## Пользовательский результат

Список с inline полями персонажей, ordinary/exclusive creation и редактор имени
соответствуют макету и продолжают использовать actual server slots и B actions.

## Работы

- Перенести character rows/cards, inline level/race/class/mode/location поля,
  selected/focus, slot placeholders и fixed actions/footer с inner scrolling.
  Итоговый D скрывает `.character-detail`; отдельная overview detail panel
  не добавляется. Native-only информация требует explicit adaptation.
- Отобразить empty/full/disabled/first-run/exclusive availability по native view.
  Fake roster или stock JS restrictions не применяются.
- Name entry — в creation visual family; существующий draft, caret/selection,
  encoding error, rejection recovery, cancel и возврат focus сохраняются.
- Клик по строке/кнопке и клавиатура вызывают существующий semantic intent;
  hit identity — server slot ID, а не индекс отрисованной отсечённой строки.
- Reorder не реализуется (C039). Его web affordances получают явную documented
  adaptation; mock-only поля не подставляются вместо отсутствующих server данных.

## Production seam и ограничения

B2 character/name scene, `session/login-view.[ch]` только для необходимой
read-only projection, `input/native-login.[ch]`, `native-name.[ch]`,
`ui/login-layout.[ch]`. Interaction остаётся semantic owner.

## Проверки и Definition of Done

Reference/native overview и name/edit/error кадры; selected row видна после scroll
и resize, keyboard/mouse сохраняют тот же slot/name outcome. Targeted existing
`sv_login_checks.py`, `sv_login_live_checks.py`, `sv_name_rejection_recovery_checks.py`
либо нужный subset existing name-editor scenarios выбирается по diff.

Общие правила: минимальные SV-local changes; никакой отдельной реализации
поведения только для тестов. Дополнительные improvements фиксируются отдельно
в `docs/sv-improvements.md`, а не включаются автоматически в эту задачу.

## Результат

Production overview и name editor используют B2-композицию, inline fields,
server-derived placeholders и availability, общий draw/hit layout и existing
semantic actions. Сохранены slot identity, draft/caret/selection, encoding error,
rejection recovery и возврат focus при отмене. Проверены настоящий overflow
с 12 wire rows и resize с сохранением видимого selected slot.

Замечания независимого ревью исправлены: невидимые row hits, составные mode bits,
scroll при переменной высоте placeholders, factual status, возврат focus и clip
длинного summary name. Reorder не добавлен; его web affordances явно исключены,
а отсутствующие server данные не заменяются mock значениями. Отличия от reference,
финальные кадры и команды проверок перечислены в evidence. Вся B2 acceptance
остаётся отдельной задачей B2-006.

# C006 — решение о приёмке

Статус: **accepted**, 2026-11-09, явно подтверждён пользователем.
C006 и задачи SV-C006-001–007 приняты в текущих согласованных границах.
[Linux evidence](sv-c006-evidence.md), [задачи](tasks/c006/README.md),
[спецификация](sv-stage-c006-spec.md).

## Решение пользователя

Пользователь подтвердил: «понял текущий этап принимаю» после запуска клиента,
исправления preview выбранной роли и обсуждения ограничений TTF и выбора шрифтов.

Приняты Linux font/resource core, независимые Text/Map роли, preview/Cancel/Save,
map preferences/reporting, resource preparation и представленное evidence.
Задачи SV-C006-001–007 имеют статус accepted. Фактические результаты и ограничения
проверок сохраняются в evidence; приёмка не добавляет непроведённых проверок.

## Известные ограничения и следующие этапы

- Встроенный numeric TTF profile сопоставляет только ASCII 32–126; остальные
  игровые IDs без explicit correspondence показывают replacement `?` (кроме
  procedural solid ID 2). Приёмка не означает полноту TTF соответствий PCF.
- F7 переключает роли Text/Map, а не список шрифтов. Временный F6 editor требует
  ручного ввода имени файла и не является продуктовым Settings UI.
- Продуктовый выбор ресурсов, preview и удаление временного F6 editor обязательны
  в [SV-C044-002](tasks/c044/SV-C044-002-product-font-settings.md).
- C007 сохраняет full graphics composition/lifecycle; C011/C013/C014 — свои
  session/map flows; C016/C053 — actual message/action delivery.

Canonical IDs, allocation, conditions и obligations не изменяются. Planning
snapshot остаётся историческим. Canonical native-runtime registration — отдельный
workflow; пользовательская приёмка не повышает registry statuses автоматически.
Непроверенные backend/profile conditions сохраняются; Windows/MinGW/Wine отложены
по текущему правилу платформ.

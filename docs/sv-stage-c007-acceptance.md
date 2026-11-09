# C007 — решение о приёмке

Статус: **accepted**, 2026-11-09, явно подтверждён пользователем.
C007 и задачи SV-C007-001–007 приняты в представленных границах Linux core.
[Evidence и ограничения](sv-c007-evidence.md), [задачи](tasks/c007/README.md),
[спецификация](sv-stage-c007-spec.md).

## Решение пользователя

Пользователь подтвердил: «этот этап принял». До этого явно уточнено, что на
реальный игровой сервер в рамках C007 не заходили и live-карту не видели.
Полная карта персонажа с сервера остаётся C013; сейчас F6 → F7 (Map) показывает
ресурсную сетку с настоящими шрифтами, тайлами и PRF definitions.
По просьбе пользователя пересобранный клиент был запущен; он создал fullscreen
окно с OpenGL renderer и завершился после отмены выбора сервера, без contact.
Это наблюдение запуска не доказывает accelerated rendering matrix.

Приняты graphics/preference lifecycle, semantic composition и geometry,
palette/animation/lighting/weather/effects core, native resource consumer и
представленное evidence. Задачи SV-C007-001–007 имеют статус accepted.

## Сохраняемые ограничения и последующие обязанности

- C011/C013/C014 сохраняют preparation, packet-derived live карту и session
  handoff; прямой semantic API/F6 projection не подменяет decode→state→surface.
- Непроверенные hardware/accelerated branches, submission budgets 20/50/200 ms,
  полная HTML/native visual acceptance и отдельный self-indicator reference
  остаются непроверенными. Приёмка не добавляет им фиктивный PASS.
- C016/C053 сохраняют реальные PRF message/action delivery и свои triggers;
  C055 — исполнение audio intents. C044/C056 — продуктовый Settings UX,
  включая обязательную замену и удаление временного F6 editor.
- Полный перечень branch/corpus/backend ограничений и disposition всех
  45 obligations сохраняется в evidence. Linux-only правило и отсрочка
  Windows/MinGW/Wine продолжают действовать.

Canonical IDs, allocation, conditions и obligations не изменяются. Planning
snapshot остаётся историческим; пользовательская приёмка не повышает canonical
runtime statuses автоматически. Native-runtime registration — отдельный workflow.

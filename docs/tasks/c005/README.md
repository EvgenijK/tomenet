# C005 — базовые механизмы игрового ввода

Статус: **accepted**, 2026-11-09. C005 и задачи 001–004 приняты пользователем. Задачи
реализованы агентами по поручению пользователя и проверены на Linux.
[Решение о приёмке](../../sv-stage-c005-acceptance.md),
[фактический evidence](../../sv-c005-evidence.md). Canonical runtime registration
остаётся отдельным workflow.
Предыдущее target addendum по preferences включено в задачу 001 без потери
обязательств. [Спецификация](../../sv-stage-c005-spec.md),
[exact canonical snapshot](coverage.json).

| Задача | Результат | Canonical owner | Зависимости |
|---|---|---|---|
| [SV-C005-001](SV-C005-001-preference-keymaps.md) | Effective normal/roguelike maps, prefixes, PRF S compatibility и production consumer | `capability.input.keymap` | Принятые B/C004 |
| [SV-C005-002](SV-C005-002-quantity.md) | Quantity core: defaults, shortcuts, limits, cancel и caller result | `capability.input.quantity` | Существующий input/text-edit core |
| [SV-C005-003](SV-C005-003-raw-key.md) | Keymap dispatch → app/protocol raw packet, reserved no-ops и lifecycle | `capability.input.raw-key` | 001 |
| [SV-C005-004](SV-C005-004-evidence.md) | Минимальное Linux evidence и disposition девяти obligations | 0 IDs | 001–003 |

Порядок: 001 и 002 независимы, затем 003; завершение — 004. Это порядок реализации,
не изменение canonical prerequisites. Snapshot содержит ровно три canonical rows
и девять obligations с hashes; canonical implementation/evidence статусы сохранены.

## Обязательства из C004

C004 завершён и принят пользователем 2026-11-09. C005 подключает реальный keymap
consumer к `SvPreferences`, включая `rogue_like_commands`, PRF reload, option/S
occurrence order и disposition legacy S records. Исправления consumer и необходимый
узкий typed seam делаются здесь, C004 не переоткрывается. Новый PRF parser,
legacy/common refactor и исправление S в legacy не требуются. Stored S record
или однобайтовая binding не доказывают command/direction result.

Default S compatibility сохраняет фактический baseline, включая его известный
defect; intended syntax требует отдельного явного решения. Новые улучшения
legacy/common записываются в [список улучшений](../../sv-improvements.md), отдельно
от текущей реализации. Намеренное SV-local дублирование допустимо.

## Границы готовности

C005 проверяет собственные effective keymap/quantity/raw results на production
пути. C011/C014 завершают actual session handoff/teardown, C053 — macro-derived
route и waits/control markers. Конкретные quantity actions принимают C017
(server requests), C022 (items), C027 (stores), C055 (audio) и другие stages
своих callers. Их будущие результаты не принимаются по core evidence; отсутствие
будущих callers не становится обратным gate C005. Source contracts и критерии
находятся в каждой задаче, общий отдельный verification процесс не вводится.

Нормативны [CONTEXT](../../../CONTEXT.md), [settings policy](../../capabilities/settings-policy.md),
ADR [0001](../../adr/0001-separate-session-interactions-and-ui.md),
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).
Только [Linux](../../agents/platform-validation.md), минимальные core cases через
production callers. Расширение проверок согласуется по AGENTS; Windows/MinGW/Wine
отложены. План и snapshot не являются runtime evidence.

## Результат реализации — 2026-11-09

001–003 реализованы на production SV пути, 004 подготовила Linux evidence.
Новые keymap/quantity modules, typed preference occurrences и app/native routing
не затрагивают legacy/common. Linux build, focused keymap/quantity/preference
ASan/UBSan и existing app/native request core прошли. Canonical allocation,
conditions и девять obligations сохранены; snapshot остаётся историческим
planning-only снимком, runtime evidence приведено отдельным отчётом выше.
Поздние callers и текущая Linux-only policy сохраняются. Этап принят пользователем:
[решение](../../sv-stage-c005-acceptance.md).

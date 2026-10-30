# Этап B2 — native startup UI по веб-макету

Статус: accepted, 2026-10-30. Пользователь явно принял этап B2 после запуска
клиента и исправления буквенных хоткеев выбора режима. Приёмка включает
SV-B2-001…006 и представленные корректировки. Текущая платформа — Linux;
Windows/MinGW/Wine остаются отложены по правилу платформ.
[Решение и evidence](../../sv-stage-b2-acceptance.md).
[Спецификация](../../sv-stage-b2-spec.md) · [Reference](reference.md) ·
[Результаты планирования агентов](../../../.scratch/sv-stage-b2-planning/map.md).

B2 выполняется после принятого B и перед C001. Это presentation follow-up
существующих B outcomes, **0 новых capability IDs**: canonical A8/B54,
B owners и C001–C064 allocation не меняются. Отдельный capability ledger или
новый `acceptanceStage` в JSON не создаётся. Все 54 IDs сохраняют B ownership;
таблица ниже распределяет visual deliverables, а не присваивает outcomes.

## Implementation DAG

| Ticket | Владеет visual deliverable | Зависимости |
|---|---|---|
| [SV-B2-001](SV-B2-001-shell.md) | Frozen reference, resources, typography, shell D, graphics/animation и shared logical geometry; первый production endpoint consumer | — |
| [SV-B2-002](SV-B2-002-server-account.md) | Server/direct connect и account/auth/create-account формы, edit/focus и factual errors | 001 |
| [SV-B2-003](SV-B2-003-character.md) | Character overview с inline fields, slots/name и rejection recovery | 002 |
| [SV-B2-004](SV-B2-004-birth.md) | Birth summary/choices/detail/stats и child controls | 003 |
| [SV-B2-005](SV-B2-005-motd-states.md) | Отдельный MOTD variant C и общие wait/error/confirmation/Guide/quit компоненты | 001 |
| [SV-B2-006](SV-B2-006-acceptance.md) | Сквозная интеграция, native/reference visual comparison и Linux input acceptance | 002, 003, 004, 005 |

Рабочая последовательность: 001 → 002 → 003 → 004 → 006; 005 идёт отдельной
ветвью после 001. Общие компоненты 005 могут подключаться при интеграции surface
owners, не меняя смысловых owners. Задачи не требуют отдельного framework.

При параллельной реализации один назначенный integration owner меняет общие
`endpoint-run.c`, scene facade, layout contracts и `makefile.sv`; surface owners
работают в своих конкретных routines/adapters. Один shared logical layout
применяется draw и hit testing, без копирования hit-coordinate constants.

## Что означает готовность

В native executable узнаваемо воспроизведён **полный** соответствующий макет:
backgrounds, animated ASCII logo/torches, panels, palette/type/spacing, формы,
списки и birth composition; MOTD следует собственному утверждённому варианту C.
Passing protocol tests и красивый первый экран не закрывают остальные surfaces.
Каждая строка [screen mapping](reference.md#screenstate--implementation-owner)
получает reference/native outcome и owner. Pixel-perfect rendering не требуется.

Linux software/OpenGL — текущие обязательные backend paths. Geometry review
ограничен affected layouts, не Cartesian matrix; minimum 1024×768 logical,
integer/fractional scales по существующему контракту. Проверки production path
переиспользуют существующие suites только по затронутой логике; расширение
проверок предварительно согласовывается по AGENTS.md.

Принятый B и снятые пользователем remaining checks не переоткрываются. Нет новых
actual-provider/IME/real-server/Windows gates, backend новостей, gameplay UI,
reorder C039, DNA persistence C040 или OS integration C060. Mock-only controls
получают explicit unavailable/adapted disposition, сохраняя visual composition.

## Статус планирования

Три агента исследовали reference, production seams и acceptance. Детальные
решения и hashes сохранены в planning map. Spec/tickets задают work plan;
B2-001…006 имеют production implementation и Linux evidence; итоговый
[visual/input review](../../sv-stage-b2-acceptance.md) подготовлен для пользователя.
Пользовательская приёмка B2 зафиксирована 2026-10-30.

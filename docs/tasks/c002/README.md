# C002 — bootstrap настроек

Статус: accepted, 2026-11-07. Пользователь явно принял C002 и задачи 001–004. [Спецификация](../../sv-stage-c002-spec.md).
Реализация и Linux evidence выполнены; [evidence](../../sv-c002-evidence.md),
[техническая сверка](../../sv-stage-c002-acceptance.md). Все шесть canonical rows pending.
Только Linux, Windows/MinGW/Wine deferred. Минимальные правки внутри SV и минимум
core tests через production-путь; расширение проверок согласовывается заранее.

| Ticket | Результат | Зависит от | Owned outcomes | Статус |
|---|---|---|---|---|
| [SV-C002-001](SV-C002-001-defaults-parser.md) | Defaults, CFG parser и узкая сверка контрактов | — | `settings.defaults`, `settings.parse` | accepted |
| [SV-C002-002](SV-C002-002-load-options.md) | Собственные settings layers, aliases и применение до первой отправки | 001; решение initial audio boundary | `settings.load`, `settings.aliases` | accepted |
| [SV-C002-003](SV-C002-003-startup-overrides-exit.md) | Production startup, CLI/environment precedence и выход без autosave | 001; интеграция с 002 | `settings.cli-overrides`, `settings.exit-unsaved` | accepted |
| [SV-C002-004](SV-C002-004-acceptance.md) | Сверка evidence и представление этапа к приёмке | 001–003 | 0 IDs | accepted |

Порядок: 001 → 002 → завершение 003 → 004. После 001 независимая часть 003
может выполняться до решения audio boundary в 002; финальная интеграция требует
общего production пути. Это порядок работ, не указание запускать подагентов.

Каждый ID имеет ровно одного owner. [Coverage snapshot](coverage.json) содержит
exact rows шести outcomes, полные obligations и hashes canonical sources;
это planning snapshot, не native evidence. Сверка границ в 001/002 не закрывает
общий SV-REALLOC-005/006/007. Старые [SV-B-003](../stage-b/SV-B-003-profile.md) и
[SV-B-004](../stage-b/SV-B-004-save.md) сохраняются как история.

Actual initial audio consumer реализован и проверен;
[вся CLI union](cli-disposition.md) сверена по existing/late consumers.
Ни один поздний outcome не принят по успешному parser. Follow-up 2026-11-07
исправил прежний birth fixture: Linux `waiting/partial/ack` прошли; историческое
падение и baseline comparison сохранены в evidence. Приёмка зафиксирована
[решением пользователя](../../sv-stage-c002-acceptance.md). Canonical native-runtime
registration остаётся отдельной работой; planning snapshot не переписывается.

# C053 — дополнение: preference macro runtime

Статус: planned, 2026-11-09. Target addendum к будущему полному плану C053;
реализация и приёмка C053 не заявлены.

C004 завершён и принят пользователем 2026-11-09; remaining runtime доработки и
проверки выполняются как обязательная работа C053. Parser/store/queue уже
существуют; actual trigger recognition, permitted `!/?` execution, completion,
waits и cancel ещё требуют настоящего consumer. Найденные при этом дефекты
исправляются в C053 без переоткрытия C004. Evidence backlink в C004 — optional
trace, не gate завершения.

| Задача | Результат | Canonical owners |
|---|---|---|
| [SV-C053-001](SV-C053-001-preference-runtime.md) | Macro recognition и actual execution одного production byte stream | `input.macro-match`, `input.macro-wait`, `input.macro-xwait` |

Все три ID имеют префикс `capability.` и сохраняют existing C053 allocation,
conditions, prerequisites и девять obligations в [ledger](../../capabilities/native-coverage.json).
Исторические `preferences.load/bootstrap/include-failure` остаются C004 IDs;
поздние runtime работа и evidence обязательны в C053, без duplicate outcome.
Macro-derived route через effective keymap C005 и actual execution/action
regression для C052 обязательны в этой задаче C053; они не являются backward
gates C005/C052 и не переоткрывают их приёмку.
[Полная layer orchestration C052](../c052/README.md) и session handoff/lifecycle
[C011](../c011/README.md)/[C014](../c014/README.md) сохраняют своих владельцев.

Production источник — `src/client/sv/settings/preferences.[ch]`; текущая
`SvInputBindings` с однобайтовой action не реализует arbitrary PRF macro.
Нужен реальный input-router/command path, исполняющий те же decoded bytes,
а не параллельный interpreter для тестов. Existing native diagnostic counts
сообщают retained intents и не являются runtime consumer.

Нормативны [CONTEXT](../../../CONTEXT.md), [settings policy](../../capabilities/settings-policy.md),
[ADR-0002](../../adr/0002-main-thread-state-ownership.md),
[ADR-0003](../../adr/0003-explicit-module-connections.md),
[ADR-0005](../../adr/0005-session-scoped-work.md),
[ADR-0006](../../adr/0006-errors-and-overload.md).
Только [Linux](../../agents/platform-validation.md); минимальные production core
checks из задачи, расширение набора проверок согласовать.

# C052 — дополнение: macro preference orchestration

Статус: planned, 2026-11-09. Target addendum, не полный план C052;
реализация и приёмка этого этапа не заявлены.

Пользователь 2026-11-09 завершил и принял C004, направив remaining consumer
доработки и проверки в соответствующие последующие этапы. Полная macro layer
precedence, поздние character/form reloads, body gates и фактический arcade
server selection теперь обязательны в C052. Найденные здесь дефекты исправляются
в C052 без переоткрытия C004; возврат evidence туда — optional trace, не gate.

| Задача | Результат | Canonical owner |
|---|---|---|
| [SV-C052-001](SV-C052-001-preference-orchestration.md) | Реальные layer/reload callers и действительная итоговая macro state | `capability.preferences.macro-precedence` |

Это единственный canonical ID C052. Его allocation, prerequisites, conditions и
четыре evidence obligations из [ledger](../../capabilities/native-coverage.json)
сохраняются, включая versioned `Receive_char_info` wire obligation. IDs
`preferences.load/bootstrap/include-failure` остаются исторически у C004;
интеграционные доработки и их проверки выполняет C052 в рамках своего результата.

Основа — stateful `src/client/sv/settings/preferences.[ch]`, уже подключённый
к global startup и fresh character OPT в `login/native-run.c`/`login/flow.c`.
Имеющиеся single-byte bindings не доказывают precedence и macro execution.
C052 доказывает реальную decoded-update orchestration и итоговую semantic state
через тот же parser/state; для его завершения input runtime C053 не требуется.
C053 обязательно выполняет macro runtime и actual action caller regressions;
[C011](../c011/README.md)/[C014](../c014/README.md) владеют session handoff/lifecycle.

Нормативны [CONTEXT](../../../CONTEXT.md), [settings policy](../../capabilities/settings-policy.md),
[ADR-0002](../../adr/0002-main-thread-state-ownership.md),
[ADR-0003](../../adr/0003-explicit-module-connections.md),
[ADR-0005](../../adr/0005-session-scoped-work.md),
[ADR-0006](../../adr/0006-errors-and-overload.md).
Нужны минимальные [Linux](../../agents/platform-validation.md) production-path
проверки из задачи, без нового общего harness или полного legacy refactor.

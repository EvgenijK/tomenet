# C005 — решение о приёмке

Статус: **accepted**, 2026-11-09, явно подтверждён пользователем.
C005 и задачи SV-C005-001–004 завершены и приняты в согласованных границах.
[Linux evidence](sv-c005-evidence.md), [спецификация](sv-stage-c005-spec.md),
[задачи](tasks/c005/README.md).

## Решение пользователя

Пользователь подтвердил: «считаем этап принятым».

Приняты production keymap и применение PRF mappings, quantity core,
app/native command/raw routing и итоговое Linux evidence. Linux build,
focused keymap/quantity/preference ASan/UBSan и existing app/native request
core прошли; фактические результаты и ограничения сохраняются в evidence.
LeakSanitizer был отключён из-за ограничения ptrace; его PASS не заявлен.

| Задача | Решение |
|---|---|
| SV-C005-001 | accepted — keymap, prefixes, preference occurrences и baseline S compatibility |
| SV-C005-002 | accepted — quantity defaults/shortcuts/limits/cancel и production input route |
| SV-C005-003 | accepted — semantic command против raw packet, native routing и core lifecycle |
| SV-C005-004 | accepted — Linux evidence и disposition всех девяти obligations |

## Сохраняющиеся границы

Поздние session preparation/handoff/teardown принадлежат C011/C014,
macro runtime — C053, конкретные quantity callers — C017/C022/C027/C055
и другим этапам своих actions; store raw-key — C027. Они не принимаются
этим решением и не являются обратным gate завершённого C005. Их обязательные
интеграции и проверки выполняются в своих этапах; результаты core C005 не
подменяют фактическое выполнение игровых действий.

Canonical IDs, allocation, conditions, prerequisites и девять obligations
сохранены. Исторический planning snapshot остаётся planning-only.
Canonical native-runtime registration — отдельный workflow: пользовательская
приёмка не повышает registry implementation/evidence pending автоматически.
Windows/MinGW/Wine проверки остаются отложенными по текущему правилу платформ.

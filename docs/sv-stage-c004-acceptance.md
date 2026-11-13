# C004 — решение о завершении и техническая сверка

Статус: **accepted**, 2026-11-09, явно подтверждён пользователем.
C004 и задачи SV-C004-001–004 приняты; оставшиеся доработки и проверки
закреплены в следующих этапах согласно предыдущему решению.
[Evidence](sv-c004-evidence.md), [spec](sv-stage-c004-spec.md),
[задачи и документы следующих этапов](tasks/c004/README.md).

## Решение пользователя — 2026-11-09

Пользователь: «перенести проверки и доработку на следующие этапы сейчас, чтобы
не оставлять этот этап незавершщённым», затем поручил агентам скорректировать
планы последующих этапов.

После переноса работ пользователь явно подтвердил приёмку:
«этот этап считаем принятым».

Current core C004 завершён и принят сейчас. Прежний gate ожидания реальных late
consumers отменён этим решением. Оставшиеся доработки и проверки включены как
обязательная работа в самостоятельные планы этапов-владельцев. Повторное
подтверждение переноса или завершения C004 не требуется.

При реализации своего этапа владелец подключает production preference owner,
проводит применимые проверки и исправляет найденные проблемы в рамках этого
этапа. Они не блокируют и не переоткрывают C004 автоматически. Возврат evidence
в сводку C004 полезен для трассировки, но не является условием его завершения.
Поздние этапы этим решением не принимаются.

## Техническая сверка

Реализован единый SV-local PRF owner поверх C002/C003: includes/provenance,
options/aliases, mapping/macro data, load-time effects, bootstrap и свежий
character overlay до отправки options. Linux build, focused ASan/UBSan,
existing first-session four-mode flow, native startup/quit и один shipped font
PRF/clone прошли. Legacy/common не менялись, CFG/OPT autosave не добавлен.

| Ticket | Текущее решение |
|---|---|
| SV-C004-001 | accepted; PRF core готов, поздние реальные consumers выполняются в своих этапах |
| SV-C004-002 | accepted; normal include core готов, transactional import завершается в C051 |
| SV-C004-003 | accepted; current startup wiring готов, resource/session/macro связки выполняются поздними owners |
| SV-C004-004 | accepted; evidence сведён, оставшиеся работы закреплены в следующих этапах |

Все три canonical outcomes, их allocation/prerequisites и девять obligations
сохранены. Непроверенные flows не получили runtime PASS: фактическое исполнение
команд, отображение PRF messages, map rendering и transactional import предстоит
проверить их реальным consumers. Доступный core и эти ограничения записаны в evidence.

Canonical native-runtime registration остаётся отдельным workflow, как для
C001–C003. Локальная пользовательская приёмка не повышает machine-readable
registry implementation/evidence pending и не подменяет фактические результаты.

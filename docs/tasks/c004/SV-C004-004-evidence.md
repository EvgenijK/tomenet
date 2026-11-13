# SV-C004-004 — Linux evidence и готовность к приёмке

Статус: **accepted**, 2026-11-09, в составе завершённого по решению пользователя C004.
Labels: enhancement
Зависимости: SV-C004-001–003.
Owned IDs: нет; сверка трёх outcomes, девяти obligations.

1. Проверить минимальный production diff и Linux build; переиспользовать только
   относящиеся core/login checks. Расширение набора проверок согласовывать по AGENTS.
2. Создать `docs/sv-c004-evidence.md`: revision/build, source roots, exact commands,
   expected/actual bytes/order/results, public production callers, filesystem
   before/after и known gaps. Не создавать evidence PASS только из source audit.
3. Для каждой exact obligation snapshot дать core evidence и late disposition
   по [source contract](source-contract.md#девять-obligations-и-evidence-disposition).
   Read/write/replace/concurrent-edit общие формулировки применять к настоящей
   операции; read-only load не оправдывает новый Save framework. Missing обязательный
   consumer — pending, не not-applicable.
4. Отдельно показать normal load против import C051, storage/queue против runtime
   C053, early bootstrap против C052 precedence, mapping against real C006/C007.
   Ни helper-only PASS, ни inert unsupported branch не закрывают gameplay evidence.
5. Сохранить C002/C003 acceptance и соседние незакоммиченные изменения. Canonical
   allocation/implementation/evidence JSON не менять без отдельного workflow.
6. Зафиксировать решение пользователя завершить C004 и перенести remaining
   доработки/проверки в следующие этапы. Их планы содержат самостоятельные
   обязательства и criteria; C004 не ожидает их evidence и не переоткрывается
   автоматически. Canonical runtime PASS не выводится из пользовательской приёмки.

Windows/MinGW/Wine deferred. Итог этой задачи — проверяемый отчёт, а не автоматическая
приёмка этапа и не команда расширить implementation scope поздних этапов.

## Результат реализации

Current Linux evidence и техническая сверка подготовлены; C004 и задачи 001–004
приняты по решению пользователя с переносом оставшейся работы в следующие этапы.
[Evidence и точные ограничения](../../sv-c004-evidence.md),
[сверка готовности](../../sv-stage-c004-acceptance.md). Canonical статусы не повышены.

## Решение пользователя и дальнейшая работа — 2026-11-09

Задача завершена и принята вместе с C004. Remaining integrations/checks относятся
к [самостоятельным планам следующих этапов](README.md#самостоятельные-обязательства-следующих-этапов).
Они не являются gate завершения этой задачи, не переоткрывают её автоматически;
найденные там проблемы исправляются в рамках этапа-владельца. Фактический
pending evidence не переписан как PASS. Возврат evidence в C004 — optional trace,
не prerequisite уже состоявшегося завершения.

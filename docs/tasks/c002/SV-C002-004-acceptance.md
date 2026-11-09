# SV-C002-004 — техническая сверка и приёмка этапа

Статус: accepted, 2026-11-07, в составе явно принятого пользователем C002.
Labels: enhancement
Зависимости: 001, 002, 003.
Owned IDs: нет (0 IDs).
Контракт: [spec](../../sv-stage-c002-spec.md), [ownership snapshot](coverage.json).

1. Сверить exact-once ownership шести IDs и актуальность canonical hashes;
   сохранить полный scope/obligations, различать implementation и evidence.
   Плановые статусы не являются результатом выполнения.
2. Свести `docs/sv-c002-evidence.md`: revision/build, Linux environment, команды,
   expected/actual values, paths, packet bytes, CFG/OPT unchanged observations,
   production callers и disposition каждой obligation. Недостающие и failed
   результаты оставить pending; Windows/MinGW/Wine — deferred.
3. Проверить решение initial audio boundary и его фактическое исполнение;
   не принимать settings.load только по parser/pack-directory observations.
   Если корректное закрытие требует изменения порядка этапов, сначала представить
   это решение, а не объявлять C002 готовым с исключёнными obligations.
4. Подтвердить минимальность SV diff, отсутствие test-only implementation и
   ненужного legacy/common refactor. Найденные отдельные улучшения записать
   в `docs/sv-improvements.md`, не включать автоматически в текущий этап.
5. Выполнить только согласованный минимум проверок из spec и затронутые
   существующие regressions. Новая matrix/suite/stress требует предварительного
   согласования. Повторная полная приёмка B/B2/C001 не является gate C002.
6. Представить пользователю результаты, ограничения и статус всех шести outcomes.
   Явное принятие фиксировать отдельным решением; canonical runtime status
   обновлять только по существующему evidence workflow, не по факту наличия отчёта.

Готово: пользователь получил проверяемый итог этапа и оставшиеся blockers;
отметка accepted возможна только после явного решения пользователя.

## Реализация 2026-11-05

Фактические файлы, команды, результаты и ограничения —
[общий evidence](../../sv-c002-evidence.md),
[техническая сверка](../../sv-stage-c002-acceptance.md).
Принят пользователем 2026-11-07 по решению в технической сверке.
Canonical rows остаются pending до отдельной native-runtime registration.

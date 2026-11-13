# SV-C005-004 — Linux evidence C005

Статус: **accepted**, 2026-11-09, по решению пользователя.
Labels: enhancement
Зависимости: SV-C005-001, SV-C005-002, SV-C005-003.
Owned IDs: нет; сверка трёх outcomes и девяти exact obligations.

## Работа

1. Сверить результат с [полным планом](../../sv-stage-c005-spec.md) и
   [snapshot](coverage.json): один owner на ID, ни один outcome/obligation не потерян.
   Сохранить canonical JSON, условия и статусы, принятую основу B/C004 и соседние
   незакоммиченные изменения.
2. Выполнить Linux build и только минимальные core checks задач 001–003 через
   production пути. Переиспользовать существующие entrypoints. Расширение проверок
   согласовать по AGENTS; Windows/MinGW/Wine отложены.
3. Создать `docs/sv-c005-evidence.md`: revision/dirty state, команды и результаты,
   production callers, expected/actual command+direction, numeric results и packet
   bytes; state/reset, отсутствие повторов и stale-send. Source audit и сохранённый
   mapping не выдавать за native execution. Ошибки/непроверенные ветви оставить явными.
4. Для каждого из девяти obligation IDs привести evidence своего core результата
   и отдельную disposition поздних routes. C011/C014 — actual handoff/teardown,
   C053 — macro-derived route; C017/C022/C027 — конкретные quantity callers,
   остальные action/audio/admin callers — в своих этапах. Future routes не gate
   готовности текущего core, но и не получают PASS от primitive tests.
5. Зафиксировать выбранную S compatibility семантику, option/S load/init order,
   retained prefix/reset и различие no-op/command/raw. Quantity cancel→0 не заменяет
   accepted-empty→default; actual caller решает смысл zero. Raw output проверяется
   через настоящий app/protocol путь, включая backpressure без повторной отправки.

## Завершение

Отчёт позволяет проверить все три core результата и каждую obligation disposition;
нет фиктивного полного gameplay PASS. Readiness выводится из фактических результатов,
не из одного source inspection или mock callback. Приёмку не заявлять от имени
пользователя; acceptance документ и canonical runtime registration делать только
по соответствующему фактическому решению/workflow. Реализация и evidence завершены: [Linux отчёт](../../sv-c005-evidence.md).

## Итог совместной реализации C005

Production integration и минимальные Linux проверки завершены; подробные
команды, результаты и границы late evidence — в [общем отчёте](../../sv-c005-evidence.md).
Задача выполнена и принята пользователем в составе C005:
[решение](../../sv-stage-c005-acceptance.md). C004 и canonical registry statuses сохраняются.

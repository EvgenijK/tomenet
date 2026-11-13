# SV-C002-003 — startup overrides и выход без autosave

Статус: accepted, 2026-11-07, в составе явно принятого пользователем C002.
Labels: enhancement
Зависимости: 001; итоговая интеграция с 002.
Owned IDs: `capability.settings.cli-overrides`, `capability.settings.exit-unsaved`.
Контракт: [spec](../../sv-stage-c002-spec.md), [полные rows](coverage.json).

1. Подключить settings core к обычному запуску до создания окна/загрузки
   ресурсов/contact. Применить CLI/environment/root overrides с baseline
   precedence поверх конфигурации для текущего запуска. Различать отсутствие
   аргумента и явное значение. Сохранить существующие production CLI contracts.
2. Ограничить адаптацию launcher необходимой передачей overrides; settings
   behavior разместить в SV production code, не в diagnostic scenario.
   Не объединять эту задачу с полным разделением launcher.
3. Применить конфигурацию к существующим startup consumers; согласовать
   с 002 один путь загрузки. Ни CLI, ни root selection, ни fallback, ни first-run
   detection не сохраняют CFG/OPT автоматически. Import UI не добавляется.
4. Проверить обычный quit, отмену входа и существующий retry/relog путь с
   изменёнными в памяти values: CFG/OPT неизменны, дополнительного Save prompt
   нет. Независимые записи vault и прочих owners сохраняют свои lifecycles.
5. Минимальные core cases проверяют precedence и actual root paths при
   различающихся file/env/CLI values. Короткий native запуск/выход подтверждает
   применение настроек к существующему UI. Для memory changes использовать
   production API, а не test-only behavior. Сравнить CFG/OPT bytes до/после,
   включая legacy unrelated owners. Не повторять всю visual/package matrix.

Готово: обычный executable использует тот же settings путь, что core tests;
override действует на запуск и не становится неявной записью. Evidence описывает
применимые failure/lifecycle cases и фактические результаты.

## Реализация 2026-11-05

Фактические файлы, команды, результаты и ограничения —
[общий evidence](../../sv-c002-evidence.md),
[техническая сверка](../../sv-stage-c002-acceptance.md).
Принят пользователем 2026-11-07 по решению в технической сверке.
Canonical rows остаются pending до отдельной native-runtime registration.

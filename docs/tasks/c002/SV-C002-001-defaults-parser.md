# SV-C002-001 — defaults и CFG parser

Статус: accepted, 2026-11-07, в составе явно принятого пользователем C002.
Labels: enhancement
Зависимости: —.
Owned IDs: `capability.settings.defaults`, `capability.settings.parse`.
Контракт: [spec](../../sv-stage-c002-spec.md), [полные rows](coverage.json).

1. Выполнить узкую source-backed сверку шести C002 IDs с manifest,
   reconciliation и baseline; сохранить в этом каталоге `source-contract.md`:
   keys/defaults/bounds, aliases, load order, CLI precedence, version gates и
   спорные mappings. Ничего не принимать по архивным SV-B-003/004.
2. Реализовать минимальный SV-local settings core. Defaults применяются до
   первого использования window/resource/connection values. Не читать server
   CFG как client defaults, legacy CFG как SV профиль и не создавать CFG/OPT
   при missing-file initialization.
3. Реализовать line-oriented parsing по settings policy: LF/CRLF, complete key
   tokens, flags/comments, duplicates и invalid-value defaults/diagnostic;
   NUL/непредставимые scalars, unknown compatible records, schema version и
   защита будущего Save от incompatible schema. `pass` исключён из загрузки и
   сохраняемых unknown records. Unsupported keys получают явный inert disposition.
4. Выделить requested values и результат валидации так, чтобы consumers не
   перезаписывали выбор эффективным fallback. Не создавать универсальный registry
   или settings UI ради будущих задач.
5. Минимальный core test вызывает production API: отсутствие CFG, утверждённые
   defaults, границы значений, LF/CRLF/duplicates, prefix collision,
   malformed scalar, supported unknown/future schema и исключение `pass`.
   Проверить отсутствие CFG/OPT writes; не добавлять отдельный parser для тестов.

Готово для следующих задач: core подключаем к настоящему startup, ошибки и
защита schema передаются caller. Полная stage acceptance требует 002–004.
Фактические результаты и obligation disposition добавить в evidence при реализации.

## Реализация 2026-11-05

Фактические файлы, команды, результаты и ограничения —
[общий evidence](../../sv-c002-evidence.md),
[техническая сверка](../../sv-stage-c002-acceptance.md).
Принят пользователем 2026-11-07 по решению в технической сверке.
Canonical rows остаются pending до отдельной native-runtime registration.

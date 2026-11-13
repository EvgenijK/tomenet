# C003 — решение о приёмке и техническая сверка

Статус: **accepted**, 2026-11-09, по явному решению пользователя.
Приёмка включает C003 и задачи SV-C003-001–006.
[Evidence и все 31 obligations](sv-c003-evidence.md),
[spec](sv-stage-c003-spec.md), [задачи](tasks/c003/README.md).

## Решение пользователя — 2026-11-09

Пользователь: «считаем этап принятым без откладывания; если будут проблемы с этим
функционалом на других этапах, то будем править в их рамках».

C003 и задачи 001–006 приняты сейчас. Это решение заменяет прежнее условие
ожидания late flow evidence. Интеграции и проверки C006/C007/C014/C016/C028/C029/
C032/C061 остаются работой этих этапов; обнаруженные там проблемы исправляются
в их рамках, не блокируют и не переоткрывают C003 автоматически. Возврат evidence
в сводку C003 полезен для трассировки, но больше не prerequisite его приёмки.
Поздние этапы этим решением не принимаются. Повторное подтверждение не требуется.

Пользовательская приёмка не превращает непроверенные flows в runtime PASS.
Canonical native-runtime registration остаётся отдельным existing workflow,
как для C001/C002; простое изменение Markdown не повышает registry статусы.

## Техническая сверка

Утверждённый текущий scope реализован: SV-local file owners и безопасная
публикация; история на 2000 записей с доступным authenticated account lifecycle;
shared DNA и реальный birth autosave/cancel; production app notes/clone effects;
version/audio/font serializers и truthful missing map identity. Linux standalone
build и минимальные affected core checks прошли. Legacy/common production source
не менялся, исходные незакоммиченные работы сохранены, canonical JSON не менялся.

Доказаны разные уровни: ordinary native startup/birth peer проверил actual DNA
save и cancel до commit, 16242-byte PLAY с исправленной identity и one-shot font
notification; app peer проверил packet→session→message effect и live reporting.
Ordinary native startup заканчивается LIVE_HANDOFF и не принимает live gameplay;
поэтому app core PASS не означает ordinary native sink/error-display/version
flow PASS. Активные map/graphics и настоящие chat/export consumers не создавались.

SV-C003-001–006 приняты пользователем по представленному текущему результату.
Самостоятельные задачи C006/C007/C014/C016/C028/C029/C032/C061 сохраняют реальные
integration/behavior obligations, условия завершения и проверки. Недоступные
consumers/evidence остаются фактически непроверенными; они не являются gate C003.

Windows/MinGW/Wine deferred, LSan в этой среде ограничен ptrace; успешные
ASan+UBSan прогоны не заявляют LSan PASS. Shared legacy writers не lock:
проверка изменений до replace реализована, последняя stat→rename гонка остаётся
принятым policy ограничением. Wider DNA shortcut/server-version branches не
проверялись отдельной matrix; native completed/cancel и local old conversion
подтверждены конкретными checks из evidence.

Canonical IDs/allocation/prerequisites/31 obligations сохранены без сокращения.
Пользовательская приёмка зафиксирована выше; этот документ не регистрирует
canonical runtime evidence и не ставит machine-readable registry PASS.

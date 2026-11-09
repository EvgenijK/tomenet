# SV-C003-001 — resolved file owners, publication и transient lifecycle

Статус: accepted, 2026-11-09, в составе явно принятого пользователем C003.
Labels: enhancement
Зависимости: C001/C002.
Owned IDs: `capability.files.resource-overlay`, `capability.files.export-collision`, `capability.files.transient-owner`.
Контракт: [spec](../../sv-stage-c003-spec.md), [источники](source-contract.md), [полные rows](coverage.json).

1. Уточнить actual production caller inventory по source-contract: read/stat/write/temp/rename/remove, startup font/audio, exports и diagnostics; записать владельца U/S/B и отсутствующие consumers.
2. Реализовать минимальный SV-local owner и явное подключение к существующим consumers. Сохранить resource-specific invalid/missing fallbacks; shared writes не отправлять в S или installation B.
3. Публикация generated export резервирует свободный suffix без гонки check→overwrite; explicit existing возвращает replace/cancel interaction, повторно проверяет destination перед commit. Failed write/replace сохраняет старый файл. Настоящий producer обязателен для полного outcome; решением 2026-11-09 publication core реализуется сейчас, integration/evidence C029/C032/C061 остаются pending. Не создавать фиктивный exporter.
4. Temp/backup/helper принадлежат операции и resolved owner; cleanup при cancel/relog и отказе. Не импортировать их как profile data, не создавать credential temp; history.tmp не disposable.
5. Минимальные core cases: U/B precedence, raw stat/temp/rename/remove в том же owner, collision/replace/cancel, denied write/failed publication, external edit и cleanup. Показать bytes прежнего destination и unrelated owners. Не строить general fault framework.

Готовность: production wiring и применимые Linux core evidence записаны; все
owned obligations имеют честный disposition. Проверки расширяются только после
согласования. Пользовательская приёмка C003 и этого ticket явно дана 2026-11-09.

## Решение и отложенная интеграция — 2026-11-09

Утверждены core и существующие consumers; границы поздних этапов сохранены.
Самостоятельные integration/evidence требования закреплены непосредственно в
[C029](../c029/README.md), [C032](../c032/README.md), [C061](../c061/README.md). Общий verification список читать не требуется;
canonical IDs и obligations не перенесены. C003 принят пользователем 2026-11-09;
helper-only evidence не закрывает поздний flow.

## Current core evidence — 2026-11-09

[Техническое evidence](../../sv-c003-evidence.md) различает ordinary native startup
и production app core, фиксирует минимальные Linux результаты и remaining flows.
[Пользовательская приёмка accepted](../../sv-stage-c003-acceptance.md); canonical runtime PASS не выставлен; пользовательская приёмка accepted. Поздние самостоятельные integration tasks остаются обязательными.

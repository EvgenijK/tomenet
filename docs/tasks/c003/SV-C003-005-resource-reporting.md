# SV-C003-005 — version/audio/font resource identity

Статус: accepted, 2026-11-09, в составе явно принятого пользователем C003.
Labels: enhancement
Зависимости: 001; C002 audio snapshot.
Owned IDs: `capability.files.report-resource-identity`.
Контракт: [spec](../../sv-stage-c003-spec.md), [источники](source-contract.md), [полные rows](coverage.json).

1. Сопоставить Send_version/Send_audio/Send_font и server receive layouts, точные %s byte limits (MAX_CHARS=80: до79 bytes+NUL), strict/inclusive version gates и actual production callers; сохранить таблицу в source-contract.
2. Дополнить существующий SV audio reporting, не дублируя его. Version/font handlers и текущий protocol lifecycle добавить в C003 через production serializers; missing/disabled features сообщают source-backed baseline values.
3. Requested local name и effective resource identity — разные данные. Только reporting copy ограничивается полем; local references остаются полными. Исправить существующую передачу UI text font из flow.c как map font в PLAY; не сообщать желаемый tileset как фактически активный renderer.
4. Сохранить Guide placeholder без Guide I/O; не получать guide length загрузкой контента. До реального map consumer сообщать source-backed missing/disabled. По решению 2026-11-09 active resource flow evidence закреплено за C006/C007, без переноса их реализации в C003.
5. Минимальное evidence existing peer/core: все source-defined layouts и boundaries limit−1/limit/limit+1, full local retention, missing packs/font, reinit и generation lifecycle. Реальный аккаунт не нужен; несовместимое расширение протокола запрещено.

Готовность: production wiring и применимые Linux core evidence записаны; все
owned obligations имеют честный disposition. Проверки расширяются только после
согласования. Пользовательская приёмка C003 и этого ticket явно дана 2026-11-09.

## Решение и отложенная интеграция — 2026-11-09

Утверждены core и существующие consumers; границы поздних этапов сохранены.
Самостоятельные integration/evidence требования закреплены непосредственно в
[C006](../c006/README.md), [C007](../c007/README.md). Общий verification список читать не требуется;
canonical IDs и obligations не перенесены. C003 принят пользователем 2026-11-09;
helper-only evidence не закрывает поздний flow.

## Current core evidence — 2026-11-09

[Техническое evidence](../../sv-c003-evidence.md) различает ordinary native startup
и production app core, фиксирует минимальные Linux результаты и remaining flows.
[Пользовательская приёмка accepted](../../sv-stage-c003-acceptance.md); canonical runtime PASS не выставлен; пользовательская приёмка accepted. Поздние самостоятельные integration tasks остаются обязательными.

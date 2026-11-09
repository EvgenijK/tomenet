# SV-C003-002 — приватная input history

Статус: accepted, 2026-11-09, в составе явно принятого пользователем C003.
Labels: enhancement
Зависимости: 001; capability.settings.load.
Owned IDs: `capability.files.history-load`, `capability.files.history-save`.
Контракт: [spec](../../sv-stage-c003-spec.md), [источники](source-contract.md), [полные rows](coverage.json).

1. Сохранить baseline MSG_HISTORY_MAX=2000, ring/order/dedup/move-to-newest; endpoint SvTextHistory=16 не является заменой. Выделить account chat owner с симметричным S/chathist-account.tmp; legacy history не загружается автоматически.
2. Подключить load к подтверждённому account lifecycle, accepted public chat input к dedup/move-to-newest до преобразований и Send_msg, независимо от успешной отправки, save к baseline exit/relog независимо от settings Save. Endpoint/password/archive/credential input не является chat history. Public chat caller отсутствует: по решению 2026-11-09 сейчас core и доступный account lifecycle, C028 подключает принятый ввод и завершает flow evidence; новой chat UI в C003 нет.
3. Empty history не переписывает файл по baseline; account switch не смешивает rings. Missing/corrupt input даёт bounded безопасный результат и диагностику; сохранить semantics строк и encoding, не вводить guessed charset.
4. Минимальные core cases: missing/empty/corrupt, ring wrap, duplicate relocation, два account, exit/relog round-trip, secrets exclusion, unavailable read/write. Проверять реальные paths/bytes и неизменность legacy file. Не сохранять history на redraw или произвольный editor destroy.

Готовность: production wiring и применимые Linux core evidence записаны; все
owned obligations имеют честный disposition. Проверки расширяются только после
согласования. Пользовательская приёмка C003 и этого ticket явно дана 2026-11-09.

## Решение и отложенная интеграция — 2026-11-09

Утверждены core и существующие consumers; границы поздних этапов сохранены.
Самостоятельные integration/evidence требования закреплены непосредственно в
[C028](../c028/README.md), [C014](../c014/README.md). Общий verification список читать не требуется;
canonical IDs и obligations не перенесены. C003 принят пользователем 2026-11-09;
helper-only evidence не закрывает поздний flow.

## Current core evidence — 2026-11-09

[Техническое evidence](../../sv-c003-evidence.md) различает ordinary native startup
и production app core, фиксирует минимальные Linux результаты и remaining flows.
[Пользовательская приёмка accepted](../../sv-stage-c003-acceptance.md); canonical runtime PASS не выставлен; пользовательская приёмка accepted. Поздние самостоятельные integration tasks остаются обязательными.

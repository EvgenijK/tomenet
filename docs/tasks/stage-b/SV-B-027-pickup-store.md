# SV-B-027 — Pickup и посещение read-only магазина

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Шаг/открытие может показать pickup confirmation или ordinary store stock/actions, затем вернуть игрока.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-007](SV-B-007-macros.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-025](SV-B-025-entry-complete.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Поздние обязательные проверки для primary owner этого тикета: [SV-B-028](SV-B-028-movement.md), [SV-B-031](SV-B-031-chat.md), [SV-B-048](SV-B-048-options-input.md), [SV-B-065](SV-B-065-options-pickup-1.md), [SV-B-066](SV-B-066-options-pickup-2.md), [SV-B-067](SV-B-067-options-pickup-3.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-009](SV-B-009-inventory.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.items.pickup-accept` | Accept the server pickup check with default-no confirmation and send Send_stay exactly once; observe authoritative pickup/inventory updates. | [nclient.c:395](../../../src/client/nclient.c#L395)<br>[nclient.c:5965](../../../src/client/nclient.c#L5965)<br>[c-inven.c:117](../../../src/client/c-inven.c#L117)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.items.pickup-decline` | Decline or Escape the pickup check: no Send_stay and no generic request reply; restore the movement owner. | [nclient.c:395](../../../src/client/nclient.c#L395)<br>[nclient.c:5965](../../../src/client/nclient.c#L5965)<br>[c-inven.c:117](../../../src/client/c-inven.c#L117)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.store.enter` | Stage stock/action updates before STORE_INFO opens shopping; metadata defines identity, owner, active stock extent and price multiplier defaults. Opening resets the view; shrinking stock hides stale rows outside stock_num. | [nclient.c:388](../../../src/client/nclient.c#L388) |
| `capability.store.read-stock` | Read normal and wide keyed stock, prices, quantities, pval width, attributed names and powers; only active stock extent is selectable, and local page letters resolve through store_top. Partial updates preserve other slots. | [nclient.c:387](../../../src/client/nclient.c#L387)<br>[nclient.c:425](../../../src/client/nclient.c#L425)<br>[nclient.c:388](../../../src/client/nclient.c#L388) |
| `capability.store.read-actions` | Read server-advertised action IDs, letters, flags, cost and restrictions independently of stock. Runtime action letters take priority over local keys; missing/disabled actions are not invented. | [nclient.c:409](../../../src/client/nclient.c#L409) |
| `capability.store.leave` | Escape/Ctrl-Q leaves the store, emits Send_store_leave through the store lifecycle, clears transient stock/last selection and restores the previous owner and macro state. | [c-store.c:16](../../../src/client/c-store.c#L16)<br>[nclient.c:7942](../../../src/client/nclient.c#L7942) |
| `capability.store.kicked` | A server STORE_LEAVE closes shopping, flushes safe macro execution and clears player-store visuals; store lifecycle cleanup restores the parent and invalidates pending transactions without duplicated effects. | [nclient.c:407](../../../src/client/nclient.c#L407) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Receive_pickup_check success/decline с actual slot/amount semantics, input/macro queue и replacement при item update; не вводить quantity prompt.
2. Store entry→stock/actions→leave/kick: versions/builds, slot identity/clear/replace, actual parent restoration; network updates продолжаются.
3. Client не заявляет buy/sell/service/item child C; store-specific key collisions и macro-in-store early paths сохраняются.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.items.pickup-accept` | Точная branch ID: normal/wide wire/slot identity/empty update, ENABLE_SUBINVEN и версия при применимости; live/final-review parents, без принятия transaction C. Точный проверяемый результат: Accept the server pickup check with default-no confirmation and send Send_stay exactly once; observe authoritative pickup/inventory updates. |
| `capability.items.pickup-decline` | Точная branch ID: normal/wide wire/slot identity/empty update, ENABLE_SUBINVEN и версия при применимости; live/final-review parents, без принятия transaction C. Точный проверяемый результат: Decline or Escape the pickup check: no Send_stay and no generic request reply; restore the movement owner. |
| `capability.store.enter` | Stock/action до INFO, resize/shrink/version defaults; ordinary entry/read/leave/kick, safe macro flush; special-canvas future boundary отдельно, не fallback для принятого B. Точный проверяемый результат: Stage stock/action updates before STORE_INFO opens shopping; metadata defines identity, owner, active stock extent and price multiplier defaults. Opening resets the view; shrinking stock hides stale rows outside stock_num. |
| `capability.store.read-stock` | Stock/action до INFO, resize/shrink/version defaults; ordinary entry/read/leave/kick, safe macro flush; special-canvas future boundary отдельно, не fallback для принятого B. Точный проверяемый результат: Read normal and wide keyed stock, prices, quantities, pval width, attributed names and powers; only active stock extent is selectable, and local page letters resolve through store_top. Partial updates preserve other slots. |
| `capability.store.read-actions` | Stock/action до INFO, resize/shrink/version defaults; ordinary entry/read/leave/kick, safe macro flush; special-canvas future boundary отдельно, не fallback для принятого B. Точный проверяемый результат: Read server-advertised action IDs, letters, flags, cost and restrictions independently of stock. Runtime action letters take priority over local keys; missing/disabled actions are not invented. |
| `capability.store.leave` | Stock/action до INFO, resize/shrink/version defaults; ordinary entry/read/leave/kick, safe macro flush; special-canvas future boundary отдельно, не fallback для принятого B. Точный проверяемый результат: Escape/Ctrl-Q leaves the store, emits Send_store_leave through the store lifecycle, clears transient stock/last selection and restores the previous owner and macro state. |
| `capability.store.kicked` | Stock/action до INFO, resize/shrink/version defaults; ordinary entry/read/leave/kick, safe macro flush; special-canvas future boundary отдельно, не fallback для принятого B. Точный проверяемый результат: A server STORE_LEAVE closes shopping, flushes safe macro execution and clears player-store visuals; store lifecycle cleanup restores the parent and invalidates pending transactions without duplicated effects. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.items.pickup-accept`, `capability.items.pickup-decline`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in item-reconciliation.md.; Receive_pickup_check: always; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Retain owner-specific ENABLE_SUBINVEN, ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN, DISCRETE_SPELL_SYSTEM, MIMIC_LUA, REGEX_SEARCH, USE_SOUND_2010 and server feature gates where present; a disabled branch is not an accepted enabled branch..
- `capability.store.enter`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in item-reconciliation.md.; Receive_store_info: > 4.7.4.2.0.0; Receive_store_info: > 4.4.4.0.0.0; Receive_store_info: older; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Retain owner-specific ENABLE_SUBINVEN, ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN, DISCRETE_SPELL_SYSTEM, MIMIC_LUA, REGEX_SEARCH, USE_SOUND_2010 and server feature gates where present; a disabled branch is not an accepted enabled branch..
- `capability.store.read-stock`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in item-reconciliation.md.; Receive_store: >= 4.9.3.0.0.3; Receive_store: >= 4.7.3.0.0.0; Receive_store: > 4.4.7.0.0.0; Receive_store: older; Receive_store_wide: >= 4.9.3.0.0.3; Receive_store_wide: > 4.7.0.0.0.0; Receive_store_wide: > 4.4.7.0.0.0; Receive_store_wide: older; Receive_store_info: > 4.7.4.2.0.0; Receive_store_info: > 4.4.4.0.0.0; Receive_store_info: older; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Retain owner-specific ENABLE_SUBINVEN, ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN, DISCRETE_SPELL_SYSTEM, MIMIC_LUA, REGEX_SEARCH, USE_SOUND_2010 and server feature gates where present; a disabled branch is not an accepted enabled branch..
- `capability.store.read-actions`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in item-reconciliation.md.; Receive_store_action: >= 4.9.3.0.0.3; Receive_store_action: >= 4.9.2.1.0.1; Receive_store_action: older; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Retain owner-specific ENABLE_SUBINVEN, ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN, DISCRETE_SPELL_SYSTEM, MIMIC_LUA, REGEX_SEARCH, USE_SOUND_2010 and server feature gates where present; a disabled branch is not an accepted enabled branch..
- `capability.store.leave`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in item-reconciliation.md.; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Retain owner-specific ENABLE_SUBINVEN, ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN, DISCRETE_SPELL_SYSTEM, MIMIC_LUA, REGEX_SEARCH, USE_SOUND_2010 and server feature gates where present; a disabled branch is not an accepted enabled branch..
- `capability.store.kicked`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in item-reconciliation.md.; Receive_store_kick: always; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Retain owner-specific ENABLE_SUBINVEN, ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN, DISCRETE_SPELL_SYSTEM, MIMIC_LUA, REGEX_SEARCH, USE_SOUND_2010 and server feature gates where present; a disabled branch is not an accepted enabled branch..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

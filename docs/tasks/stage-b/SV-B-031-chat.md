# SV-B-031 — Чат, история и локальные формы

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок отправляет public/private/channel текст или local-self, использует историю и item substitutions.

## Зависимости и граница

Завершить необходимые production части [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-030](SV-B-030-clipboard.md).

Prerequisites A: `capability.messages.read-occurrences`. См. [различие checkpoint и canonical claims](../../sv-stage-b-spec.md#foundation).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `map-chat`: producer [SV-B-029](SV-B-029-map-explore.md); полные owners [SV-B-031](SV-B-031-chat.md). 029 вводит actual native chat child с map/locate continuation.Реализовать все достижимые из этого child baseline branches: ordinary/private/channel transforms, own history, available item substitutions, local-self/forwarded slash и Escape без Send_msg.029 использует real editor/clipboard001 и histories010; это не fixed-response или test-only chat.031 расширяет и принимает полные chat outcomes во всех обязательных B callers. 029 выполняет map-specific success/cancel/transform overflow/history/provider failure и exact map selection/focus/queue return с interleaved network/resize.031 повторяет map branch; Guide/sheet/final callers добавляют integration checks033/035/036, результаты прикладываются к исходному owner031.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-033](SV-B-033-guide-tools.md), [SV-B-035](SV-B-035-sheet.md), [SV-B-036](SV-B-036-session-end.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-010](SV-B-010-history-profile.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-029](SV-B-029-map-explore.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.chat.send` | Submit ordinary, private and explicit-channel chat through cmd_message and Send_msg, preserving channel/address decoration and transformed bytes; editor limit is MSG_LEN minus cname length minus 17 including the final terminator allowance. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:6791](../../../src/client/nclient.c#L6791)<br>[nclient.c:366](../../../src/client/nclient.c#L366)<br>[c-util.c:1649](../../../src/client/c-util.c#L1649) |
| `capability.chat.history` | Recall/edit chat text using the production text editor and its chat mode/history rules; draft edits and input method/paste respect byte limits. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:6791](../../../src/client/nclient.c#L6791)<br>[nclient.c:366](../../../src/client/nclient.c#L366)<br>[c-util.c:1649](../../../src/client/c-util.c#L1649) |
| `capability.chat.substitute-items` | Expand inventory/equipment/floor/newest/bag/store shortcuts, colours, spacing and colon escaping in source order; preserve unavailable-slot and expansion-limit behavior, testing final transformed bytes. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:6791](../../../src/client/nclient.c#L6791)<br>[nclient.c:366](../../../src/client/nclient.c#L366)<br>[c-util.c:1649](../../../src/client/c-util.c#L1649) |
| `capability.chat.local-self` | Route %: to local messages and %%: to local chat, except doubled-colon escape forms; no Send_msg for consumed local forms. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:6791](../../../src/client/nclient.c#L6791)<br>[nclient.c:366](../../../src/client/nclient.c#L366)<br>[c-util.c:1649](../../../src/client/c-util.c#L1649) |
| `capability.chat.forward-slash` | Forward unmatched slash text, bare slash and malformed local recognizers through Send_msg once; server grammar and permission outcomes remain server-owned, with no invented local whitelist. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:6791](../../../src/client/nclient.c#L6791)<br>[nclient.c:366](../../../src/client/nclient.c#L366)<br>[c-util.c:1649](../../../src/client/c-util.c#L1649) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Final transformed bytes: address/colour/colon/spacing/item expansions, MSG_LEN−cname−17 allowance, unavailable slot и bounded expansion; one send only.
2. History load/edit/Save, macro→Enter after truncation; unmatched/bare/malformed slash forwards once без whitelist; consumed %:/%%: local forms не шлют Send_msg.
3. Escape в early gameplay/map/Guide/character/read-only-store caller не шлёт сообщение, восстанавливает focus/draft/queue; D chat.cancel whole union не принимается этим B handoff.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.chat.send` | Final transformed bytes после paste/item/colour/address/colon expansion, boundary+overflow; send once/cancel none; nested owner и reconnect. Точный проверяемый результат: Submit ordinary, private and explicit-channel chat through cmd_message and Send_msg, preserving channel/address decoration and transformed bytes; editor limit is MSG_LEN minus cname length minus 17 including the final terminator allowance. Уже существующий obligation.chat.send.lifecycle требует cancellation и actual caller return. cmd_message get_string guard, map child, sheet child. В каждом раннем gameplay/map/Guide/character/read-only-store caller: Escape без Send_msg, возврат focus/draft/queue, resize/network/relogin; D добавляет lore/document, C skill остаётся проверкой своего caller. Full deferred outcomes: capability.chat.cancel. |
| `capability.chat.history` | Final transformed bytes после paste/item/colour/address/colon expansion, boundary+overflow; send once/cancel none; nested owner и reconnect. Точный проверяемый результат: Recall/edit chat text using the production text editor and its chat mode/history rules; draft edits and input method/paste respect byte limits. |
| `capability.chat.substitute-items` | Final transformed bytes после paste/item/colour/address/colon expansion, boundary+overflow; send once/cancel none; nested owner и reconnect. Точный проверяемый результат: Expand inventory/equipment/floor/newest/bag/store shortcuts, colours, spacing and colon escaping in source order; preserve unavailable-slot and expansion-limit behavior, testing final transformed bytes. |
| `capability.chat.local-self` | Final transformed bytes после paste/item/colour/address/colon expansion, boundary+overflow; send once/cancel none; nested owner и reconnect. Точный проверяемый результат: Route %: to local messages and %%: to local chat, except doubled-colon escape forms; no Send_msg for consumed local forms. |
| `capability.chat.forward-slash` | Final transformed bytes после paste/item/colour/address/colon expansion, boundary+overflow; send once/cancel none; nested owner и reconnect. Точный проверяемый результат: Forward unmatched slash text, bare slash and malformed local recognizers through Send_msg once; server grammar and permission outcomes remain server-owned, with no invented local whitelist. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.chat.send`, `capability.chat.history`, `capability.chat.substitute-items`, `capability.chat.local-self`, `capability.chat.forward-slash`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in information-reconciliation.md.; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Preserve per-owner REGEX_SEARCH, ENABLE_GO_GAME, ENABLE_ITEM_ORDER, USE_GRAPHICS, TEST_RAWPICT, ANIM_SLOT_SPINALL, USE_SOUND_2010 and admin/DM/runtime privilege gates where applicable; disabled branches do not certify enabled behavior..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

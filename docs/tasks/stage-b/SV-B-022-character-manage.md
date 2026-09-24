# SV-B-022 — Создание slot и порядок персонажей

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Обзор позволяет создать ordinary/exclusive персонажа и переставить строки, сохраняя server identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-006](SV-B-006-login.md), [SV-B-020](SV-B-020-first-session.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.character.create-ordinary` | N chooses an available ordinary slot; name acceptance starts birth only when server permits creation. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.create-exclusive` | E selects dedicated creation when exclusive_ok and not first run; carry dedicated IDDC/PvP mode into completed birth. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.name` | Use 15-byte live-trim/plus-prefix editor; empty entry generates a random suggestion and remains in editor, nonempty accepts, plus prefix requests reincarnation. Server Trim_name and ownership checks remain authoritative. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[nserver.c:320](../../../src/server/nserver.c#L320)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.cancel-name` | Escape returns to character overview with no character-login submission. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.name-rejected` | Server rejects forbidden/duplicate/unowned character name with retry-login or exit policy; never expose a partially entered live session. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[nserver.c:320](../../../src/server/nserver.c#L320)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.swap` | Select two valid slots and send PKT_LOGIN with *** followed by first slot, second slot and mode 1. Wait for the refreshed server list before treating ordering as changed. | [nclient.c:702](../../../src/client/nclient.c#L702)<br>[nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.insert-before` | Select two valid slots and send PKT_LOGIN with *** followed by first slot, second slot and mode 2. Wait for the refreshed server list before treating ordering as changed. | [nclient.c:702](../../../src/client/nclient.c#L702)<br>[nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.append-after` | Select two valid slots and send PKT_LOGIN with *** followed by first slot, second slot and mode 3. Wait for the refreshed server list before treating ordering as changed. | [nclient.c:702](../../../src/client/nclient.c#L702)<br>[nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.cancel-reorder-first` | Escape at first slot selection discards the unfinished reorder and returns to overview without PKT_LOGIN reorder bytes. | [nclient.c:702](../../../src/client/nclient.c#L702)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.cancel-reorder-second` | Escape at second slot selection discards the unfinished reorder and returns to overview without PKT_LOGIN reorder bytes. | [nclient.c:702](../../../src/client/nclient.c#L702)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Name limits/rejection, ordinary/exclusive availability, server-owned constraints; do not infer success from send.
2. Swap/insert-before/append-after, cancellation at first/second selection, overview refresh while pending сохраняют correct stable identities.
3. Creation launch не пропускает birth steps, renderer resize не перезапускает запрос.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.character.create-ordinary` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: N chooses an available ordinary slot; name acceptance starts birth only when server permits creation. |
| `capability.character.create-exclusive` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: E selects dedicated creation when exclusive_ok and not first run; carry dedicated IDDC/PvP mode into completed birth. |
| `capability.character.name` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Use 15-byte live-trim/plus-prefix editor; empty entry generates a random suggestion and remains in editor, nonempty accepts, plus prefix requests reincarnation. Server Trim_name and ownership checks remain authoritative. |
| `capability.character.cancel-name` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Escape returns to character overview with no character-login submission. |
| `capability.character.name-rejected` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Server rejects forbidden/duplicate/unowned character name with retry-login or exit policy; never expose a partially entered live session. |
| `capability.character.swap` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Select two valid slots and send PKT_LOGIN with *** followed by first slot, second slot and mode 1. Wait for the refreshed server list before treating ordering as changed. |
| `capability.character.insert-before` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Select two valid slots and send PKT_LOGIN with *** followed by first slot, second slot and mode 2. Wait for the refreshed server list before treating ordering as changed. |
| `capability.character.append-after` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Select two valid slots and send PKT_LOGIN with *** followed by first slot, second slot and mode 3. Wait for the refreshed server list before treating ordering as changed. |
| `capability.character.cancel-reorder-first` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Escape at first slot selection discards the unfinished reorder and returns to overview without PKT_LOGIN reorder bytes. |
| `capability.character.cancel-reorder-second` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Escape at second slot selection discards the unfinished reorder and returns to overview without PKT_LOGIN reorder bytes. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.character.create-ordinary`, `capability.character.create-exclusive`, `capability.character.name-rejected`: versions — Character list: >4.5.7 includes location; >4.4.9.2 includes mode; older omits both. Reorder >=4.7.3. Dedicated capacity >4.5.8.1 and arcade/RPG changes at 4.9.0.5. Login initial address bytes >=4.9.2.1.0.2.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.character.name`, `capability.character.cancel-name`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.character.swap`, `capability.character.insert-before`, `capability.character.append-after`: versions — >=4.7.3 and allow_reordering; server flags and actual slot count.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.character.cancel-reorder-first`, `capability.character.cancel-reorder-second`: versions — >=4.7.3 and allow_reordering.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

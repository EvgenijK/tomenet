# SV-B-033 — Bookmarks, копирование и вставка из Guide

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок сохраняет/открывает/удаляет bookmarks, копирует строки и вставляет Guide line в разрешённый chat caller.

## Зависимости и граница

Завершить необходимые production части [SV-B-008](SV-B-008-guide.md), [SV-B-030](SV-B-030-clipboard.md), [SV-B-031](SV-B-031-chat.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-008](SV-B-008-guide.md), [SV-B-031](SV-B-031-chat.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.guide.bookmark-set` | Create/rename selected Guide bookmark with original bounded name and line semantics in private bookmark state. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.bookmark-open` | Open named bookmark including exact /? a through /? t under GUIDE_BOOKMARKS; invalid/missing bookmark keeps owner. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.bookmark-delete` | Remove selected Guide bookmark without affecting other bookmarks or source Guide. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.copy-lines` | Copy current Guide lines through source-profile decoding and formatting stripping, preserving logical source content. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.paste-line` | Send current Guide line reference to chat with baseline formatting exactly once; return to Guide caller. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Own bookmark load/save/conflict/failure, Guide replacement, missing topic и selected-line identity после scroll/resize.
2. Copy-lines/paste-line идут через actual clipboard/chat transform and byte limits, Escape восстанавливает Guide state без Send_msg.
3. Поздние skills/lore context-help callers D остаются отдельной полной приёмкой.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.guide.bookmark-set` | Create/rename bounded name сохраняет выбранную source line через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.bookmark-open` | /? a…t открывает точную line; invalid/missing bookmark сохраняет owner через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.bookmark-delete` | Удаляется только выбранный bookmark, Guide bytes не меняются через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.copy-lines` | FF/control stripped decoding выбранных current lines, secret-free clipboard через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.paste-line` | Ровно один chat send formatted Guide line reference, затем возврат через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.guide.bookmark-set`, `capability.guide.bookmark-open`, `capability.guide.bookmark-delete`, `capability.guide.copy-lines`, `capability.guide.paste-line`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; GUIDE_BOOKMARKS for bookmark operations, REGEX_SEARCH for regexp; SDL3_CURL_SSL for automatic download/checksum. No server required except Receive_Guide..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

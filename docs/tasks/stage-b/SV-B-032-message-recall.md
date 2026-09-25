# SV-B-032 — Полный recall и important history

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок читает, ищет и закрывает оба вида истории, включая final-review owner.

## Зависимости и граница

Завершить необходимые production части [SV-B-031](SV-B-031-chat.md).

Prerequisites A: `capability.messages.read-occurrences`. См. [различие checkpoint и canonical claims](../../sv-stage-b-spec.md#foundation).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Поздние обязательные проверки для primary owner этого тикета: [SV-B-034](SV-B-034-exports.md), [SV-B-036](SV-B-036-session-end.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.messages.recall-read` | Read recall message history with original ordering, channel/control markers and owner-specific filtering/bundling; do not replace stored occurrences with the display projection. | [c-xtra2.c:144](../../../src/client/c-xtra2.c#L144)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.messages.recall-navigate` | Navigate recall recall by line/page/top/bottom and horizontal offset while retaining the current history position and updates. | [c-xtra2.c:144](../../../src/client/c-xtra2.c#L144)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.messages.recall-search` | Search/highlight recall recall with the exact case/direction and optional REGEX_SEARCH rules; failed or canceled search preserves the recall owner. | [c-xtra2.c:144](../../../src/client/c-xtra2.c#L144)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.messages.recall-close` | Close recall recall and restore gameplay or final-review caller and its queue policy. | [c-xtra2.c:144](../../../src/client/c-xtra2.c#L144)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.messages.important-read` | Read important message history with original ordering, channel/control markers and owner-specific filtering/bundling; do not replace stored occurrences with the display projection. | [c-xtra2.c:594](../../../src/client/c-xtra2.c#L594)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.messages.important-navigate` | Navigate important recall by line/page/top/bottom and horizontal offset while retaining the current history position and updates. | [c-xtra2.c:594](../../../src/client/c-xtra2.c#L594)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.messages.important-search` | Search/highlight important recall with the exact case/direction and optional REGEX_SEARCH rules; failed or canceled search preserves the recall owner. | [c-xtra2.c:594](../../../src/client/c-xtra2.c#L594)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.messages.important-close` | Close important recall and restore gameplay or final-review caller and its queue policy. | [c-xtra2.c:594](../../../src/client/c-xtra2.c#L594)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Original ordering/control/channel/bundling, consecutive equal occurrences; scroll/search/repeat-search и REGEX_SEARCH guards, cancel/failed search restores anchor.
2. Updates while open, bounded retention/eviction anchors, resize/focus, disconnect и exact live/final parent restoration.
3. Не подменять сохранённые occurrences display projection; initial A message checks остаются регрессиями.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.messages.recall-read` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Read recall message history with original ordering, channel/control markers and owner-specific filtering/bundling; do not replace stored occurrences with the display projection. |
| `capability.messages.recall-navigate` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Navigate recall recall by line/page/top/bottom and horizontal offset while retaining the current history position and updates. |
| `capability.messages.recall-search` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Search/highlight recall recall with the exact case/direction and optional REGEX_SEARCH rules; failed or canceled search preserves the recall owner. |
| `capability.messages.recall-close` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Close recall recall and restore gameplay or final-review caller and its queue policy. |
| `capability.messages.important-read` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Read important message history with original ordering, channel/control markers and owner-specific filtering/bundling; do not replace stored occurrences with the display projection. |
| `capability.messages.important-navigate` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Navigate important recall by line/page/top/bottom and horizontal offset while retaining the current history position and updates. |
| `capability.messages.important-search` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Search/highlight important recall with the exact case/direction and optional REGEX_SEARCH rules; failed or canceled search preserves the recall owner. |
| `capability.messages.important-close` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Close important recall and restore gameplay or final-review caller and its queue policy. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.messages.recall-read`, `capability.messages.recall-navigate`, `capability.messages.recall-search`, `capability.messages.recall-close`, `capability.messages.important-read`, `capability.messages.important-navigate`, `capability.messages.important-search`, `capability.messages.important-close`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in information-reconciliation.md.; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Preserve per-owner REGEX_SEARCH, ENABLE_GO_GAME, ENABLE_ITEM_ORDER, USE_GRAPHICS, TEST_RAWPICT, ANIM_SLOT_SPINALL, USE_SOUND_2010 and admin/DM/runtime privilege gates where applicable; disabled branches do not certify enabled behavior..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

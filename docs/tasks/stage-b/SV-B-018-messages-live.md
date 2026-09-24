# SV-B-018 — Живая лента сообщений с точными occurrences

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Первый real session сохраняет incoming message bytes, channel/control markers и все одинаковые occurrences.

## Зависимости и граница

Завершить необходимые production части [SV-B-002](SV-B-002-contact.md).

Prerequisites A: `capability.messages.read-occurrences`. См. [различие checkpoint и canonical claims](../../sv-stage-b-spec.md#foundation).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.messages.read-live` | Read real-session messages with original channel/colour/control markers, clear-topline, live feed and full-history routing; identical incoming occurrences remain distinct even if recall bundles display. This B outcome needs live decode-to-view evidence beyond the synthetic A occurrence slice. | [nclient.c:366](../../../src/client/nclient.c#L366)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Два одинаковых пакета — два occurrences; routing/filter/bundling только projection, retained source bytes не изменяются.
2. Live feed/primary update при network burst, prompt, resize и replay snapshot; no duplicate notification on redraw.
3. Bounded queues/recall по owner policy, explicit overflow, no unbounded packet recorder; сообщения обеспечивают semantic message animation scope.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.messages.read-live` | Exact order/control/channel/filter/bundling, navigation/search+REGEX_SEARCH, cancel/failed search, updates while open и return live/final owner. Точный проверяемый результат: Read real-session messages with original channel/colour/control markers, clear-topline, live feed and full-history routing; identical incoming occurrences remain distinct even if recall bundles display. This B outcome needs live decode-to-view evidence beyond the synthetic A occurrence slice. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.messages.read-live`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in information-reconciliation.md.; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Preserve per-owner REGEX_SEARCH, ENABLE_GO_GAME, ENABLE_ITEM_ORDER, USE_GRAPHICS, TEST_RAWPICT, ANIM_SLOT_SPINALL, USE_SOUND_2010 and admin/DM/runtime privilege gates where applicable; disabled branches do not certify enabled behavior..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

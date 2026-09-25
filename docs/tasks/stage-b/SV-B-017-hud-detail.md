# SV-B-017 — Личность, цели и дополнительные HUD данные

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Экран сохраняет demographics, tracked monster health и служебные collections без смешения с поздними knowledge documents.

## Зависимости и граница

Завершить необходимые production части [SV-B-002](SV-B-002-contact.md), [SV-B-015](SV-B-015-hud-core.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.status.read-demographics` | Read height, weight, age, social class and source identity text. | [nclient.c:359](../../../src/client/nclient.c#L359) |
| `capability.status.read-study` | Read available study count. | [nclient.c:372](../../../src/client/nclient.c#L372) |
| `capability.status.read-blows` | Read blows/wraith probability attribute and versioned label. | [nclient.c:373](../../../src/client/nclient.c#L373) |
| `capability.status.read-monster-health` | Read tracked monster health and colour separately from player HP. | [nclient.c:402](../../../src/client/nclient.c#L402) |
| `capability.status.read-afk` | Read server AFK state, independent of local window focus. | [nclient.c:413](../../../src/client/nclient.c#L413) |
| `capability.status.read-extra` | Read exact opaque extra-status bytes with bounded display projection. | [nclient.c:419](../../../src/client/nclient.c#L419) |
| `capability.status.read-idle` | Read idle state and applicable status presentation. | [nclient.c:452](../../../src/client/nclient.c#L452) |
| `capability.status.read-indicators` | Read indicator bit meanings without losing unknown bits. | [nclient.c:456](../../../src/client/nclient.c#L456) |
| `capability.information.live-players` | Read separate live player-list snapshot: mode0 clear, mode1 full terminated replace, mode2 upsert, mode3 remove; preserve bounded source rows and rollback. | [nclient.c:457](../../../src/client/nclient.c#L457)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.information.unique-records` | Read keyed unique kill counts/names independently of server document navigation. | [nclient.c:421](../../../src/client/nclient.c#L421)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[c-files.c:2297](../../../src/client/c-files.c#L2297) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Каждый оставшийся Receive_*: old/new layouts, field defaults, keyed collection clear/upsert/remove, split/adjacent packet и reset.
2. Unique-records хранит только count/name и питает dump; здесь нет ASCII glyph mapping. Live players collection не равна SPECIAL_FILE_PLAYER document D.
3. На screen recreation никаких повторных эффекта/запроса; self identity/AFK отличается от window focus.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.status.read-demographics` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read height, weight, age, social class and source identity text. |
| `capability.status.read-study` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read available study count. |
| `capability.status.read-blows` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read blows/wraith probability attribute and versioned label. |
| `capability.status.read-monster-health` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read tracked monster health and colour separately from player HP. |
| `capability.status.read-afk` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read server AFK state, independent of local window focus. |
| `capability.status.read-extra` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read exact opaque extra-status bytes with bounded display projection. |
| `capability.status.read-idle` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read idle state and applicable status presentation. |
| `capability.status.read-indicators` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read indicator bit meanings without losing unknown bits. |
| `capability.information.live-players` | Live и final-review child; coherent data/pages/help/dump/capture/chat и точное восстановление parent; версия/missing values, без повторных sends. Точный проверяемый результат: Read separate live player-list snapshot: mode0 clear, mode1 full terminated replace, mode2 upsert, mode3 remove; preserve bounded source rows and rollback. |
| `capability.information.unique-records` | Live и final-review child; coherent data/pages/help/dump/capture/chat и точное восстановление parent; версия/missing values, без повторных sends. Точный проверяемый результат: Read keyed unique kill counts/names independently of server document navigation. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.status.read-demographics`, `capability.status.read-study`, `capability.status.read-monster-health`, `capability.status.read-afk`, `capability.status.read-extra`, `capability.status.read-idle`, `capability.status.read-indicators`, `capability.information.unique-records`: versions — always; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.status.read-blows`: versions — < 4.9.1.0.0.1; >= 4.9.1.0.0.1; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.information.live-players`: versions — always; player-list mode 1 record loop; empty name terminates; player-list mode 2 add/update; player-list mode 3 remove; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

# SV-B-016 — Состояния, голод и видимые ограничения

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок видит независимые серверные состояния, включая версионные отсутствующие значения.

## Зависимости и граница

Завершить необходимые production части [SV-B-015](SV-B-015-hud-core.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.status.read-confusion` | Read authoritative confusion condition. | [nclient.c:370](../../../src/client/nclient.c#L370) |
| `capability.status.read-poison` | Read authoritative poison condition. | [nclient.c:371](../../../src/client/nclient.c#L371) |
| `capability.status.read-food` | Read food amount and semantic hunger presentation. | [nclient.c:374](../../../src/client/nclient.c#L374) |
| `capability.status.read-fear` | Read authoritative fear condition. | [nclient.c:375](../../../src/client/nclient.c#L375) |
| `capability.status.read-speed` | Read authoritative speed value. | [nclient.c:376](../../../src/client/nclient.c#L376) |
| `capability.status.read-cut` | Read cut severity and corresponding presentation. | [nclient.c:377](../../../src/client/nclient.c#L377) |
| `capability.status.read-blind-hallucination` | Read blind/hallucination flags without collapsing their meanings. | [nclient.c:378](../../../src/client/nclient.c#L378) |
| `capability.status.read-stun` | Read stun severity and corresponding presentation. | [nclient.c:379](../../../src/client/nclient.c#L379) |
| `capability.status.read-encumbrance` | Read all fifteen encumbrance conditions with version-dependent absent flags cleared. | [nclient.c:414](../../../src/client/nclient.c#L414) |
| `capability.status.read-martyr` | Read martyr state and applicable status presentation. | [nclient.c:450](../../../src/client/nclient.c#L450) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Полные attribute/condition fields и15encumbrance flags с absent-version defaults; clear/reset каждого состояния и interleaved packet burst.
2. Проверить различие blind/hallucination, fear/stun/cut, food/speed/martyr; не выводить состояние из локального mouse/animation.
3. Ургентное изменение под prompt, software frame submission, redraw/focus и reconnect сохраняют authoritative state.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.status.read-confusion` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read authoritative confusion condition. |
| `capability.status.read-poison` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read authoritative poison condition. |
| `capability.status.read-food` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read food amount and semantic hunger presentation. |
| `capability.status.read-fear` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read authoritative fear condition. |
| `capability.status.read-speed` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read authoritative speed value. |
| `capability.status.read-cut` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read cut severity and corresponding presentation. |
| `capability.status.read-blind-hallucination` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read blind/hallucination flags without collapsing their meanings. |
| `capability.status.read-stun` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read stun severity and corresponding presentation. |
| `capability.status.read-encumbrance` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read all fifteen encumbrance conditions with version-dependent absent flags cleared. |
| `capability.status.read-martyr` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read martyr state and applicable status presentation. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.status.read-confusion`, `capability.status.read-poison`, `capability.status.read-food`, `capability.status.read-fear`, `capability.status.read-speed`, `capability.status.read-cut`, `capability.status.read-blind-hallucination`, `capability.status.read-stun`, `capability.status.read-martyr`: versions — always; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.status.read-encumbrance`: versions — >= 4.8.1.0.0.0; > 4.4.2.0.0.0; older; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

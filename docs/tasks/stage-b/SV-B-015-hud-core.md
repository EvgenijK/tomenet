# SV-B-015 — Живые HP, ресурсы и прогресс персонажа

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

На первом игровом экране видны actual HP/MP/ST/sanity и основные характеристики вместо синтетических значений.

## Зависимости и граница

Завершить необходимые production части [SV-B-002](SV-B-002-contact.md).

Prerequisites A: `capability.status.read-hp`. См. [различие checkpoint и canonical claims](../../sv-stage-b-spec.md#foundation).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.status.read-live-hp` | During a live session read authoritative current/max HP, boost and bar interpretation after each complete versioned packet, including absent-field defaults, interleaved gameplay, resize and reconnect. Stage A synthetic HP does not accept this live caller. | [nclient.c:354](../../../src/client/nclient.c#L354) |
| `capability.status.read-mana` | Read current/max mana and applicable bar/boost conventions. | [nclient.c:363](../../../src/client/nclient.c#L363) |
| `capability.status.read-stamina` | Read current/max stamina. | [nclient.c:417](../../../src/client/nclient.c#L417) |
| `capability.status.read-sanity` | Read sanity attribute/label/damage/current/max according to version; SHOW_SANITY must link with registered receiver. | [nclient.c:403](../../../src/client/nclient.c#L403) |
| `capability.status.read-experience` | Read level, max levels, experience and next/previous thresholds; preserve old-version defaults. | [nclient.c:361](../../../src/client/nclient.c#L361) |
| `capability.status.read-currency` | Read carried gold and bank balance separately. | [nclient.c:362](../../../src/client/nclient.c#L362) |
| `capability.status.read-stats` | Read each stat current/max/index/base/temp value with version defaults. | [nclient.c:353](../../../src/client/nclient.c#L353) |
| `capability.status.read-armour` | Read base armour class and armour bonus. | [nclient.c:355](../../../src/client/nclient.c#L355) |
| `capability.status.read-combat-bonuses` | Read melee/ranged/misc hit and damage values without conflating them. | [nclient.c:360](../../../src/client/nclient.c#L360) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Каждый Receive_* wire branch отдельно, marker/default values, paired maxima/current, incomplete/chained sentinel; no partial coherent-view publication.
2. Live HP отличается от accepted A synthetic HP; привязать к real session stream и actual displayed values, 50→10→50 не теряет effects.
3. Native software/accelerated render, resize/DPI/font change сохраняют values и critical HUD visibility; stale session update не протекает.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.status.read-live-hp` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: During a live session read authoritative current/max HP, boost and bar interpretation after each complete versioned packet, including absent-field defaults, interleaved gameplay, resize and reconnect. Stage A synthetic HP does not accept this live caller. |
| `capability.status.read-mana` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read current/max mana and applicable bar/boost conventions. |
| `capability.status.read-stamina` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read current/max stamina. |
| `capability.status.read-sanity` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read sanity attribute/label/damage/current/max according to version; SHOW_SANITY must link with registered receiver. |
| `capability.status.read-experience` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read level, max levels, experience and next/previous thresholds; preserve old-version defaults. |
| `capability.status.read-currency` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read carried gold and bank balance separately. |
| `capability.status.read-stats` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read each stat current/max/index/base/temp value with version defaults. |
| `capability.status.read-armour` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read base armour class and armour bonus. |
| `capability.status.read-combat-bonuses` | Все wire layouts указанного Receive_* и absent-field defaults, split+sentinel, coherent live projection, reconnect/reset; normal/big/resize и applicable build gates. Точный проверяемый результат: Read melee/ranged/misc hit and damage values without conflating them. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.status.read-live-hp`: versions — > 4.7.0.2.0.1; older; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.status.read-mana`, `capability.status.read-stamina`, `capability.status.read-currency`, `capability.status.read-armour`, `capability.status.read-combat-bonuses`: versions — always; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.status.read-sanity`: versions — >= 4.8.1.3.0.0; > 4.6.1.2.0.0 and < 4.8.1.3; <= 4.6.1.2; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.status.read-experience`: versions — > 4.5.6.0.0.1; older; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.status.read-stats`: versions — >= 4.7.4.6.0.0; older; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

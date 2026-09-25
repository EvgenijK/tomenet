# SV-B-055 — Опции освещения и видимости карты — часть 1

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-039](SV-B-039-map-effects.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-039](SV-B-039-map-effects.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.font-map-solid-walls` | Apply font_map_solid_walls: Certain fonts only: Walls look like solid blocks. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:214](../../../src/client/c-tables.c#L214)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:213](../../../src/client/c-tables.c#L213)<br>[nclient.c:2888](../../../src/client/nclient.c#L2888)<br>[nclient.c:2889](../../../src/client/nclient.c#L2889)<br>[c-util.c:12780](../../../src/client/c-util.c#L12780) |
| `capability.options.view-animated-light` | Apply view_animated_light: Animate lantern light, flickering in colour. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:216](../../../src/client/c-tables.c#L216)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:215](../../../src/client/c-tables.c#L215)<br>[cave.c:3338](../../../src/server/cave.c#L3338) |
| `capability.options.wall-lighting` | Apply wall_lighting: Generally enable lighting/shading for wall grids. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:218](../../../src/client/c-tables.c#L218)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:217](../../../src/client/c-tables.c#L217)<br>[cave.c:3724](../../../src/server/cave.c#L3724)<br>[cave.c:5449](../../../src/server/cave.c#L5449)<br>[cave.c:5453](../../../src/server/cave.c#L5453) |
| `capability.options.view-lamp-walls` | Apply view_lamp_walls: Use special colors for lamp-lit wall grids. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:220](../../../src/client/c-tables.c#L220)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:219](../../../src/client/c-tables.c#L219)<br>[cave.c:3768](../../../src/server/cave.c#L3768) |
| `capability.options.view-shade-walls` | Apply view_shade_walls: Use special colors to shade wall grids. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:222](../../../src/client/c-tables.c#L222)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:221](../../../src/client/c-tables.c#L221)<br>[cave.c:3796](../../../src/server/cave.c#L3796) |
| `capability.options.floor-lighting` | Apply floor_lighting: Generally enable lighting/shading for floor grids. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:224](../../../src/client/c-tables.c#L224)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:223](../../../src/client/c-tables.c#L223)<br>[cave.c:3317](../../../src/server/cave.c#L3317)<br>[cave.c:3520](../../../src/server/cave.c#L3520)<br>[cave.c:5448](../../../src/server/cave.c#L5448) |
| `capability.options.view-lamp-floor` | Apply view_lamp_floor: Use special colors for lamp-lit floor grids. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:226](../../../src/client/c-tables.c#L226)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:225](../../../src/client/c-tables.c#L225)<br>[cave.c:3330](../../../src/server/cave.c#L3330) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.font-map-solid-walls` | Apply font_map_solid_walls: Certain fonts only: Walls look like solid blocks — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.view-animated-light` | Apply view_animated_light: Animate lantern light, flickering in colour — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.wall-lighting` | Apply wall_lighting: Generally enable lighting/shading for wall grids — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.view-lamp-walls` | Apply view_lamp_walls: Use special colors for lamp-lit wall grids — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.view-shade-walls` | Apply view_shade_walls: Use special colors to shade wall grids — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.floor-lighting` | Apply floor_lighting: Generally enable lighting/shading for floor grids — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.view-lamp-floor` | Apply view_lamp_floor: Use special colors for lamp-lit floor grids — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.font-map-solid-walls` | `capability.world.read-map` |
| `capability.options.view-animated-light` | `capability.world.read-map` |
| `capability.options.wall-lighting` | `capability.world.read-map` |
| `capability.options.view-lamp-walls` | `capability.world.read-map` |
| `capability.options.view-shade-walls` | `capability.world.read-map` |
| `capability.options.floor-lighting` | `capability.world.read-map` |
| `capability.options.view-lamp-floor` | `capability.world.read-map` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.font-map-solid-walls`, `capability.options.view-animated-light`, `capability.options.wall-lighting`, `capability.options.view-lamp-walls`, `capability.options.view-shade-walls`, `capability.options.floor-lighting`, `capability.options.view-lamp-floor`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

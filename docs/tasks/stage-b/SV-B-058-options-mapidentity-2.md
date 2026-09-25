# SV-B-058 — Опции glyph identity и highlighting — часть 2

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-039](SV-B-039-map-effects.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-039](SV-B-039-map-effects.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.ascii-feats` | Apply ascii_feats: Disable font-specific mapping for all floor feats. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:570](../../../src/client/c-tables.c#L570)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:569](../../../src/client/c-tables.c#L569)<br>[go.c:1349](../../../src/server/go.c#L1349)<br>[c-util.c:12843](../../../src/client/c-util.c#L12843) |
| `capability.options.ascii-items` | Apply ascii_items: Disable font-specific mapping for all items. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:572](../../../src/client/c-tables.c#L572)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:571](../../../src/client/c-tables.c#L571)<br>[cave.c:3067](../../../src/server/cave.c#L3067)<br>[c-cmd.c:6004](../../../src/client/c-cmd.c#L6004) |
| `capability.options.ascii-monsters` | Apply ascii_monsters: Disable font-specific mapping for all monsters. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:574](../../../src/client/c-tables.c#L574)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:573](../../../src/client/c-tables.c#L573)<br>[cave.c:2337](../../../src/server/cave.c#L2337)<br>[c-cmd.c:6412](../../../src/client/c-cmd.c#L6412) |
| `capability.options.ascii-uniques` | Apply ascii_uniques: Disable font-specific mapping for unique monsters. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:576](../../../src/client/c-tables.c#L576)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:575](../../../src/client/c-tables.c#L575)<br>[cave.c:2337](../../../src/server/cave.c#L2337)<br>[c-cmd.c:6412](../../../src/client/c-cmd.c#L6412) |
| `capability.options.gfx-autooff-fmsw` | Apply gfx_autooff_fmsw: Auto-disable font_map_solid_walls for graphics. Preserve literal baseline default/build row F; E=F; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:585](../../../src/client/c-tables.c#L585)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:584](../../../src/client/c-tables.c#L584)<br>[nclient.c:2888](../../../src/client/nclient.c#L2888)<br>[c-util.c:15168](../../../src/client/c-util.c#L15168)<br>[c-init.c:4418](../../../src/client/c-init.c#L4418) |
| `capability.options.wide-scroll-margin` | Apply wide_scroll_margin: Scroll the screen further away from the edges. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:601](../../../src/client/c-tables.c#L601)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:600](../../../src/client/c-tables.c#L600)<br>[xtra2.c:153](../../../src/server/xtra2.c#L153)<br>[xtra2.c:155](../../../src/server/xtra2.c#L155)<br>[xtra2.c:157](../../../src/server/xtra2.c#L157) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.ascii-feats` | Apply ascii_feats: Disable font-specific mapping for all floor feats — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.ascii-items` | Apply ascii_items: Disable font-specific mapping for all items — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.ascii-monsters` | Apply ascii_monsters: Disable font-specific mapping for all monsters — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.ascii-uniques` | Apply ascii_uniques: Disable font-specific mapping for unique monsters — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.gfx-autooff-fmsw` | Apply gfx_autooff_fmsw: Auto-disable font_map_solid_walls for graphics — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.wide-scroll-margin` | Apply wide_scroll_margin: Scroll the screen further away from the edges — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.ascii-feats` | `capability.world.read-map` |
| `capability.options.ascii-items` | `capability.world.read-map` |
| `capability.options.ascii-monsters` | `capability.world.read-map` |
| `capability.options.ascii-uniques` | `capability.world.read-map` |
| `capability.options.gfx-autooff-fmsw` | `capability.world.read-map` |
| `capability.options.wide-scroll-margin` | `capability.world.read-map` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.ascii-feats`, `capability.options.ascii-items`, `capability.options.ascii-monsters`, `capability.options.ascii-uniques`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.gfx-autooff-fmsw`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=F; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.wide-scroll-margin`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

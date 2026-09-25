# SV-B-053 — Опции чисел и status bars — часть 1

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-017](SV-B-017-hud-detail.md), [SV-B-035](SV-B-035-sheet.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.exp-bar` | Apply exp_bar: Show experience bar instead of a number. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:242](../../../src/client/c-tables.c#L242)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:241](../../../src/client/c-tables.c#L241)<br>[c-xtra1.c:227](../../../src/client/c-xtra1.c#L227)<br>[c-util.c:12816](../../../src/client/c-util.c#L12816)<br>[c-xtra1.c:246](../../../src/client/c-xtra1.c#L246) |
| `capability.options.linear-stats` | Apply linear_stats: Stats are represented in a linear way. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:262](../../../src/client/c-tables.c#L262)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:261](../../../src/client/c-tables.c#L261)<br>[c-xtra1.c:87](../../../src/client/c-xtra1.c#L87) |
| `capability.options.exp-need` | Apply exp_need: Show the experience needed for next level. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:264](../../../src/client/c-tables.c#L264)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:263](../../../src/client/c-tables.c#L263)<br>[c-xtra1.c:228](../../../src/client/c-xtra1.c#L228)<br>[c-util.c:12816](../../../src/client/c-util.c#L12816) |
| `capability.options.depth-in-feet` | Apply depth_in_feet: Show dungeon level in feet. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:266](../../../src/client/c-tables.c#L266)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:265](../../../src/client/c-tables.c#L265)<br>[c-util.c:17621](../../../src/client/c-util.c#L17621)<br>[c-xtra1.c:1069](../../../src/client/c-xtra1.c#L1069)<br>[c-xtra1.c:4043](../../../src/client/c-xtra1.c#L4043) |
| `capability.options.hp-bar` | Apply hp_bar: Display hit points as bar instead of numbers. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:508](../../../src/client/c-tables.c#L508)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:507](../../../src/client/c-tables.c#L507)<br>[nclient.c:2195](../../../src/client/nclient.c#L2195)<br>[c-util.c:12822](../../../src/client/c-util.c#L12822)<br>[c-util.c:12825](../../../src/client/c-util.c#L12825) |
| `capability.options.mp-bar` | Apply mp_bar: Display mana pool as bar instead of numbers. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:510](../../../src/client/c-tables.c#L510)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:509](../../../src/client/c-tables.c#L509)<br>[nclient.c:3123](../../../src/client/nclient.c#L3123)<br>[c-util.c:12823](../../../src/client/c-util.c#L12823)<br>[c-util.c:12826](../../../src/client/c-util.c#L12826) |
| `capability.options.st-bar` | Apply st_bar: Display stamina as bar instead of numbers. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:512](../../../src/client/c-tables.c#L512)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:511](../../../src/client/c-tables.c#L511)<br>[nclient.c:2247](../../../src/client/nclient.c#L2247)<br>[c-util.c:12824](../../../src/client/c-util.c#L12824)<br>[c-util.c:12827](../../../src/client/c-util.c#L12827) |
| `capability.options.mp-huge-bar` | Apply mp_huge_bar: Also show mana pool as huge bar (big_map only). Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:543](../../../src/client/c-tables.c#L543)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:542](../../../src/client/c-tables.c#L542)<br>[nclient.c:162](../../../src/client/nclient.c#L162)<br>[nclient.c:182](../../../src/client/nclient.c#L182)<br>[nclient.c:191](../../../src/client/nclient.c#L191) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.exp-bar` | Apply exp_bar: Show experience bar instead of a number — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.linear-stats` | Apply linear_stats: Stats are represented in a linear way — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.exp-need` | Apply exp_need: Show the experience needed for next level — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.depth-in-feet` | Apply depth_in_feet: Show dungeon level in feet — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.hp-bar` | Apply hp_bar: Display hit points as bar instead of numbers — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.mp-bar` | Apply mp_bar: Display mana pool as bar instead of numbers — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.st-bar` | Apply st_bar: Display stamina as bar instead of numbers — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.mp-huge-bar` | Apply mp_huge_bar: Also show mana pool as huge bar (big_map only) — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.exp-bar` | `capability.world.read-map` |
| `capability.options.linear-stats` | `capability.world.read-map` |
| `capability.options.exp-need` | `capability.world.read-map` |
| `capability.options.depth-in-feet` | `capability.world.read-map` |
| `capability.options.hp-bar` | `capability.world.read-map` |
| `capability.options.mp-bar` | `capability.world.read-map` |
| `capability.options.st-bar` | `capability.world.read-map` |
| `capability.options.mp-huge-bar` | `capability.world.read-map` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.exp-bar`, `capability.options.depth-in-feet`, `capability.options.mp-huge-bar`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.linear-stats`, `capability.options.exp-need`, `capability.options.hp-bar`, `capability.options.mp-bar`, `capability.options.st-bar`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

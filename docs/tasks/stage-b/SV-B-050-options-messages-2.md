# SV-B-050 — Опции входящих сообщений и их представления — часть 2

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.keep-topline` | Apply keep_topline: Don't clear messages in the top line if avoidable. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:280](../../../src/client/c-tables.c#L280)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:279](../../../src/client/c-tables.c#L279)<br>[c-util.c:205](../../../src/client/c-util.c#L205) |
| `capability.options.taciturn-messages` | Apply taciturn_messages: Suppress server messages as far as possible. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:284](../../../src/client/c-tables.c#L284)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:283](../../../src/client/c-tables.c#L283)<br>[c-cmd.c:2095](../../../src/client/c-cmd.c#L2095)<br>[melee2.c:13217](../../../src/server/melee2.c#L13217)<br>[dungeon.c:689](../../../src/server/dungeon.c#L689) |
| `capability.options.topline-no-msg` | Apply topline_no_msg: Don't display messages in main window top line. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:478](../../../src/client/c-tables.c#L478)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:477](../../../src/client/c-tables.c#L477)<br>[c-util.c:4799](../../../src/client/c-util.c#L4799)<br>[c-util.c:4932](../../../src/client/c-util.c#L4932)<br>[c-util.c:17726](../../../src/client/c-util.c#L17726) |
| `capability.options.topline-first` | Apply topline_first: Top line shows first line of multi-line messages. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:607](../../../src/client/c-tables.c#L607)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:606](../../../src/client/c-tables.c#L606)<br>[c-util.c:4796](../../../src/client/c-util.c#L4796)<br>[c-util.c:5004](../../../src/client/c-util.c#L5004)<br>[c-util.c:5032](../../../src/client/c-util.c#L5032) |
| `capability.options.add-kind-diz` | Apply add_kind_diz: Inv/eq window only: Item-to-chat-paste extra info. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:592](../../../src/client/c-tables.c#L592)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:591](../../../src/client/c-tables.c#L591)<br>[util.c:2943](../../../src/server/util.c#L2943)<br>[slash.c:6934](../../../src/server/slash.c#L6934)<br>[slash.c:6935](../../../src/server/slash.c#L6935) |
| `capability.options.hide-lore-paste` | Apply hide_lore_paste: Hide artifact/monster lore pastes in public chat. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:594](../../../src/client/c-tables.c#L594)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:593](../../../src/client/c-tables.c#L593)<br>[util.c:2937](../../../src/server/util.c#L2937)<br>[slash.c:6943](../../../src/server/slash.c#L6943)<br>[slash.c:6944](../../../src/server/slash.c#L6944) |
| `capability.options.sunburn-msg` | Apply sunburn_msg: Show message if you start/stop burning in the sun. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:599](../../../src/client/c-tables.c#L599)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:598](../../../src/client/c-tables.c#L598)<br>[xtra1.c:6823](../../../src/server/xtra1.c#L6823)<br>[xtra1.c:6824](../../../src/server/xtra1.c#L6824) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.keep-topline` | Apply keep_topline: Don't clear messages in the top line if avoidable — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live |
| `capability.options.taciturn-messages` | Apply taciturn_messages: Suppress server messages as far as possible — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live |
| `capability.options.topline-no-msg` | Apply topline_no_msg: Don't display messages in main window top line — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live |
| `capability.options.topline-first` | Apply topline_first: Top line shows first line of multi-line messages — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live |
| `capability.options.add-kind-diz` | Apply add_kind_diz: Inv/eq window only: Item-to-chat-paste extra info — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live |
| `capability.options.hide-lore-paste` | Apply hide_lore_paste: Hide artifact/monster lore pastes in public chat — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live |
| `capability.options.sunburn-msg` | Apply sunburn_msg: Show message if you start/stop burning in the sun — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.walk capability.messages.read-live |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.keep-topline` | `capability.messages.read-live` |
| `capability.options.taciturn-messages` | `capability.messages.read-live` |
| `capability.options.topline-no-msg` | `capability.messages.read-live` |
| `capability.options.topline-first` | `capability.messages.read-live` |
| `capability.options.add-kind-diz` | `capability.messages.read-live` |
| `capability.options.hide-lore-paste` | `capability.messages.read-live` |
| `capability.options.sunburn-msg` | `capability.world.walk`, `capability.messages.read-live` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.keep-topline`, `capability.options.taciturn-messages`, `capability.options.topline-no-msg`, `capability.options.hide-lore-paste`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.topline-first`, `capability.options.add-kind-diz`, `capability.options.sunburn-msg`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

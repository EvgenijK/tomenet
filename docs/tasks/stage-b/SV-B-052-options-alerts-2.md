# SV-B-052 — Опции paging и предупреждений — часть 2

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-028](SV-B-028-movement.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.alert-offpanel-dam` | Apply alert_offpanel_dam: Beep when taking damage while looking elsewhere. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:239](../../../src/client/c-tables.c#L239)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:238](../../../src/client/c-tables.c#L238)<br>[nclient.c:2096](../../../src/client/nclient.c#L2096)<br>[nclient.c:2211](../../../src/client/nclient.c#L2211) |
| `capability.options.warn-unique-credit` | Apply warn_unique_credit: Beep on attacking a unique you already killed. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:247](../../../src/client/c-tables.c#L247)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:246](../../../src/client/c-tables.c#L246)<br>[monster2.c:5294](../../../src/server/monster2.c#L5294)<br>[spells1.c:10768](../../../src/server/spells1.c#L10768)<br>[cmd1.c:4718](../../../src/server/cmd1.c#L4718) |
| `capability.options.allow-paging` | Apply allow_paging: Allow users to page you (recommended!). Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:256](../../../src/client/c-tables.c#L256)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:255](../../../src/client/c-tables.c#L255)<br>[nclient.c:6290](../../../src/client/nclient.c#L6290) |
| `capability.options.ring-bell` | Apply ring_bell: Beep on misc warnings and errors. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:258](../../../src/client/c-tables.c#L258)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:257](../../../src/client/c-tables.c#L257)<br>[c-util.c:1943](../../../src/client/c-util.c#L1943) |
| `capability.options.alert-starvation` | Apply alert_starvation: Beep when taking damage from starvation. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:434](../../../src/client/c-tables.c#L434)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:433](../../../src/client/c-tables.c#L433)<br>[spells1.c:2503](../../../src/server/spells1.c#L2503) |
| `capability.options.flash-insane` | Apply flash_insane: Flash own character icon when going badly insane. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:484](../../../src/client/c-tables.c#L484)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:483](../../../src/client/c-tables.c#L483)<br>[cave.c:4811](../../../src/server/cave.c#L4811) |
| `capability.options.flash-starvation` | Apply flash_starvation: Flash 'Starved' food status by colour animation. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:633](../../../src/client/c-tables.c#L633)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:632](../../../src/client/c-tables.c#L632)<br>[c-xtra1.c:1114](../../../src/client/c-xtra1.c#L1114)<br>[c-xtra1.c:1116](../../../src/client/c-xtra1.c#L1116) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.alert-offpanel-dam` | Apply alert_offpanel_dam: Beep when taking damage while looking elsewhere — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live capability.audio.play-event |
| `capability.options.warn-unique-credit` | Apply warn_unique_credit: Beep on attacking a unique you already killed — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.walk capability.audio.play-event |
| `capability.options.allow-paging` | Apply allow_paging: Allow users to page you (recommended!) — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live capability.audio.play-event |
| `capability.options.ring-bell` | Apply ring_bell: Beep on misc warnings and errors — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live capability.audio.play-event |
| `capability.options.alert-starvation` | Apply alert_starvation: Beep when taking damage from starvation — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live capability.audio.play-event |
| `capability.options.flash-insane` | Apply flash_insane: Flash own character icon when going badly insane — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.flash-starvation` | Apply flash_starvation: Flash 'Starved' food status by colour animation — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.messages.read-live capability.audio.play-event |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.alert-offpanel-dam` | `capability.messages.read-live`, `capability.audio.play-event` |
| `capability.options.warn-unique-credit` | `capability.world.walk`, `capability.audio.play-event` |
| `capability.options.allow-paging` | `capability.messages.read-live`, `capability.audio.play-event` |
| `capability.options.ring-bell` | `capability.messages.read-live`, `capability.audio.play-event` |
| `capability.options.alert-starvation` | `capability.messages.read-live`, `capability.audio.play-event` |
| `capability.options.flash-insane` | `capability.world.read-map` |
| `capability.options.flash-starvation` | `capability.messages.read-live`, `capability.audio.play-event` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.alert-offpanel-dam`, `capability.options.allow-paging`, `capability.options.ring-bell`, `capability.options.alert-starvation`, `capability.options.flash-starvation`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.warn-unique-credit`, `capability.options.flash-insane`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

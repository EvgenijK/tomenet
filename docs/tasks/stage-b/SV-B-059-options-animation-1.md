# SV-B-059 — Опции анимации, weather и color scopes — часть 1

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-018](SV-B-018-messages-live.md), [SV-B-039](SV-B-039-map-effects.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-009](SV-B-009-inventory.md), [SV-B-039](SV-B-039-map-effects.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.subterm-flicker` | Apply subterm_flicker: Show animated text colours in sub-windows. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:305](../../../src/client/c-tables.c#L305)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:304](../../../src/client/c-tables.c#L304)<br>[z-term.c:1134](../../../src/client/z-term.c#L1134)<br>[c-xtra1.c:2945](../../../src/client/c-xtra1.c#L2945)<br>[c-xtra1.c:1921](../../../src/client/c-xtra1.c#L1921)<br>[c-xtra1.c:3098](../../../src/client/c-xtra1.c#L3098)<br>[nclient.c:356](../../../src/client/nclient.c#L356)<br>[nclient.c:6627](../../../src/client/nclient.c#L6627)<br>[nclient.c:420](../../../src/client/nclient.c#L420)<br>[nclient.c:423](../../../src/client/nclient.c#L423)<br>[c-inven.c:117](../../../src/client/c-inven.c#L117)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.options.misc-no-flicker` | Apply misc_no_flicker: Only animate colours for main screen and messages. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:641](../../../src/client/c-tables.c#L641)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:640](../../../src/client/c-tables.c#L640)<br>[z-term.c:1135](../../../src/client/z-term.c#L1135)<br>[c-xtra1.c:2945](../../../src/client/c-xtra1.c#L2945)<br>[c-xtra1.c:1921](../../../src/client/c-xtra1.c#L1921)<br>[c-xtra1.c:3098](../../../src/client/c-xtra1.c#L3098)<br>[nclient.c:356](../../../src/client/nclient.c#L356)<br>[nclient.c:6627](../../../src/client/nclient.c#L6627)<br>[nclient.c:420](../../../src/client/nclient.c#L420)<br>[nclient.c:423](../../../src/client/nclient.c#L423)<br>[c-inven.c:117](../../../src/client/c-inven.c#L117)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.options.no-flicker` | Apply no_flicker: Disable all fast colour animations. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:579](../../../src/client/c-tables.c#L579)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:578](../../../src/client/c-tables.c#L578)<br>[z-term.c:1133](../../../src/client/z-term.c#L1133) |
| `capability.options.no-weather` | Apply no_weather: Disable weather visuals and sounds completely. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:289](../../../src/client/c-tables.c#L289)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:288](../../../src/client/c-tables.c#L288)<br>[nclient.c:5591](../../../src/client/nclient.c#L5591)<br>[nclient.c:5613](../../../src/client/nclient.c#L5613)<br>[nclient.c:8442](../../../src/client/nclient.c#L8442) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.subterm-flicker` | При no_flicker=false и misc_no_flicker=false переключить subterm_flicker: true сохраняет анимацию, false применяет baseline anim2static к animated item attr в auxiliary inventory и к animated text в live message/chat content. Один и тот же main-map animated attr не меняет animation policy при этом переключении. Проверять фазы нескольких animation ticks через production SV; после no_flicker=true все три content roles становятся статичными. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Сохранить source colour identity и порядок сообщений. Это local display effect: не выдумывать server consumer. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Consumer fixtures: capability.items.read-inventory, capability.messages.read-live, capability.world.read-map. |
| `capability.options.misc-no-flicker` | При no_flicker=false и subterm_flicker=true переключить misc_no_flicker: true применяет baseline anim2static к animated item attr в auxiliary inventory, false сохраняет анимацию. Live message/chat content (PW_MESSAGE/PW_CHAT/PW_MSGNOCHAT semantic scopes) и main-map animated attr сохраняют animation policy при обоих значениях. При subterm_flicker=false auxiliary messages также статичны; исключение сообщений из misc suppression не обходит subterm_flicker или global no_flicker. Проверять фазы нескольких animation ticks через production SV. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Сохранить source colour identity и порядок сообщений. Это local display effect: не выдумывать server consumer. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Consumer fixtures: capability.items.read-inventory, capability.messages.read-live, capability.world.read-map. |
| `capability.options.no-flicker` | Apply no_flicker: Disable all fast colour animations — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |
| `capability.options.no-weather` | Apply no_weather: Disable weather visuals and sounds completely — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.read-map |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.subterm-flicker` | `capability.items.read-inventory`, `capability.messages.read-live`, `capability.world.read-map` |
| `capability.options.misc-no-flicker` | `capability.items.read-inventory`, `capability.messages.read-live`, `capability.world.read-map` |
| `capability.options.no-flicker` | `capability.world.read-map` |
| `capability.options.no-weather` | `capability.world.read-map` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.subterm-flicker`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.misc-no-flicker`, `capability.options.no-flicker`, `capability.options.no-weather`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

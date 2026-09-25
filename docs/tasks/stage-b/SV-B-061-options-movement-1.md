# SV-B-061 — Опции движения, running и disturbance — часть 1

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-028](SV-B-028-movement.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.always-repeat` | Apply always_repeat: Repeat obvious commands (eg search/tunnel). Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:337](../../../src/client/c-tables.c#L337)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:336](../../../src/client/c-tables.c#L336)<br>[cmd2.c:1797](../../../src/server/cmd2.c#L1797)<br>[cmd2.c:3308](../../../src/server/cmd2.c#L3308)<br>[cmd2.c:5277](../../../src/server/cmd2.c#L5277) |
| `capability.options.find-ignore-stairs` | Apply find_ignore_stairs: Run past stairs. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:356](../../../src/client/c-tables.c#L356)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:355](../../../src/client/c-tables.c#L355)<br>[cmd1.c:9161](../../../src/server/cmd1.c#L9161) |
| `capability.options.find-ignore-doors` | Apply find_ignore_doors: Run through open doors. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:358](../../../src/client/c-tables.c#L358)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:357](../../../src/client/c-tables.c#L357)<br>[cmd1.c:9146](../../../src/server/cmd1.c#L9146) |
| `capability.options.find-cut` | Apply find_cut: Run past known corners. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:360](../../../src/client/c-tables.c#L360)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:359](../../../src/client/c-tables.c#L359)<br>[cmd1.c:9316](../../../src/server/cmd1.c#L9316)<br>[cmd1.c:9348](../../../src/server/cmd1.c#L9348) |
| `capability.options.find-examine` | Apply find_examine: Run into potential corners. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:362](../../../src/client/c-tables.c#L362)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:361](../../../src/client/c-tables.c#L361)<br>[cmd1.c:9316](../../../src/server/cmd1.c#L9316)<br>[cmd1.c:9316](../../../src/server/cmd1.c#L9316) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.always-repeat` | Apply always_repeat: Repeat obvious commands (eg search/tunnel) — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.search capability.world.open capability.world.tunnel |
| `capability.options.find-ignore-stairs` | Apply find_ignore_stairs: Run past stairs — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.walk capability.world.run |
| `capability.options.find-ignore-doors` | Apply find_ignore_doors: Run through open doors — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.walk capability.world.run |
| `capability.options.find-cut` | Apply find_cut: Run past known corners — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.walk capability.world.run |
| `capability.options.find-examine` | Apply find_examine: Run into potential corners — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.world.walk capability.world.run |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.always-repeat` | `capability.world.search`, `capability.world.open`, `capability.world.tunnel` |
| `capability.options.find-ignore-stairs` | `capability.world.walk`, `capability.world.run` |
| `capability.options.find-ignore-doors` | `capability.world.walk`, `capability.world.run` |
| `capability.options.find-cut` | `capability.world.walk`, `capability.world.run` |
| `capability.options.find-examine` | `capability.world.walk`, `capability.world.run` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.always-repeat`, `capability.options.find-ignore-doors`, `capability.options.find-cut`, `capability.options.find-examine`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.find-ignore-stairs`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

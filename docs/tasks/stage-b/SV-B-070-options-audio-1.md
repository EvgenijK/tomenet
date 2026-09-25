# SV-B-070 — Опции audio incoming effects и suppression — часть 1

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок наблюдает эффект каждого option через его конкретного раннего consumer, сохраняя BOOL meaning, defaults и slot identity.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-028](SV-B-028-movement.md), [SV-B-040](SV-B-040-audio-controls.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.options.audio-paging` | Apply audio_paging: Use audio system for page/alert, if available. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:394](../../../src/client/c-tables.c#L394)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:393](../../../src/client/c-tables.c#L393)<br>[c-util.c:1986](../../../src/client/c-util.c#L1986) |
| `capability.options.paging-master-vol` | Apply paging_master_vol: Play page/alert sounds at master volume. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:396](../../../src/client/c-tables.c#L396)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:395](../../../src/client/c-tables.c#L395)<br>[snd-sdl3.c:1936](../../../src/client/snd-sdl3.c#L1936)<br>[snd-sdl3.c:1987](../../../src/client/snd-sdl3.c#L1987) |
| `capability.options.paging-max-vol` | Apply paging_max_vol: Play page/alert sounds at maximum volume. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:398](../../../src/client/c-tables.c#L398)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:397](../../../src/client/c-tables.c#L397)<br>[snd-sdl3.c:1935](../../../src/client/snd-sdl3.c#L1935)<br>[snd-sdl3.c:1986](../../../src/client/snd-sdl3.c#L1986) |
| `capability.options.no-ovl-close-sfx` | Apply no_ovl_close_sfx: Prevent re-playing sfx received after <100ms gap. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:400](../../../src/client/c-tables.c#L400)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:399](../../../src/client/c-tables.c#L399)<br>[snd-sdl3.c:2150](../../../src/client/snd-sdl3.c#L2150)<br>[snd-sdl.c:1948](../../../src/client/snd-sdl.c#L1948) |
| `capability.options.ovl-sfx-attack` | Apply ovl_sfx_attack: Allow overlapping combat sounds of same type. Preserve literal baseline default/build row T; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:402](../../../src/client/c-tables.c#L402)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:401](../../../src/client/c-tables.c#L401)<br>[snd-sdl3.c:2107](../../../src/client/snd-sdl3.c#L2107)<br>[snd-sdl.c:1905](../../../src/client/snd-sdl.c#L1905) |
| `capability.options.no-combat-sfx` | Apply no_combat_sfx: Don't play melee/launcher attack/miss sound fx. Preserve literal baseline default/build row F; E=T; —, boolean meaning and option slot; own global/character OPT, permitted shared macro effects, explicit Save. Final semantic display-option overlay in settings-policy.md overrides draft L/U no-op proposals. | [c-tables.c:404](../../../src/client/c-tables.c#L404)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-tables.c:403](../../../src/client/c-tables.c#L403)<br>[cmd1.c:49](../../../src/server/cmd1.c#L49)<br>[cmd1.c:3712](../../../src/server/cmd1.c#L3712)<br>[nserver.c:2860](../../../src/server/nserver.c#L2860)<br>[spells2.c:8238](../../../src/server/spells2.c#L8238)<br>[spells2.c:8241](../../../src/server/spells2.c#L8241)<br>[spells2.c:8244](../../../src/server/spells2.c#L8244)<br>[config.h:467](../../../src/config.h#L467) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Для КАЖДОГО ID таблицы выполнить false/true через указанные concrete consumer fixtures; default/slot, own global/character OPT, macro directive override, Preview→Cancel и Save→reload.
2. При server-owned значении: actual Send_options→sync_options→consumer→наблюдаемый ответ с version/inversion gates; local-only effect не получает выдуманного server consumer.
3. Проверить cancellation/error/interleaved update/resize/teardown у consumer; поздние C/D/E callers остаются своими pending outcomes и сохраняют option-aware regressions.
4. Особые границы: disturb_other только storage/wire без нового effect; subterm/misc — auxiliary inventory/message scopes и invariant main map; ASCII lore regressions не относятся к unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.options.audio-paging` | Apply audio_paging: Use audio system for page/alert, if available — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.audio.play-event capability.audio.music capability.audio.weather |
| `capability.options.paging-master-vol` | Apply paging_master_vol: Play page/alert sounds at master volume — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.audio.play-event capability.audio.music capability.audio.weather |
| `capability.options.paging-max-vol` | Apply paging_max_vol: Play page/alert sounds at maximum volume — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.audio.play-event capability.audio.music capability.audio.weather |
| `capability.options.no-ovl-close-sfx` | Apply no_ovl_close_sfx: Prevent re-playing sfx received after <100ms gap — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.audio.play-event capability.audio.music capability.audio.weather |
| `capability.options.ovl-sfx-attack` | Apply ovl_sfx_attack: Allow overlapping combat sounds of same type — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Повторить caller-specific проверки при C/D/E; поздний caller остаётся pending до его этапа. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. Consumer fixtures: capability.audio.play-event capability.audio.music capability.audio.weather |
| `capability.options.no-combat-sfx` | Apply no_combat_sfx: Don't play melee/launcher attack/miss sound fx — обе BOOL-ветви через production SV и указанный реальный consumer, не standalone parser. OPT default/slot, own global/character Save→reload, Preview→Cancel, macro directive override; применимые compile/version guards. Для server-owned значения: Send_options→sync_options→consumer→наблюдаемый ответ; сохранить инверсию slot и актуальную server-version ветвь. Для local-only значения не изобретать server effect. USE_SOUND_2010: sound build и disabled-feature ветка, production option→server source→ordered sound event; bump/autoret не заменять синтетическим проигрыванием sample. Consumer fixtures: capability.world.walk capability.audio.play-event |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

### Конкретные option consumer fixtures

| Option ID | Native callers из reconciliation |
|---|---|
| `capability.options.audio-paging` | `capability.audio.play-event`, `capability.audio.music`, `capability.audio.weather` |
| `capability.options.paging-master-vol` | `capability.audio.play-event`, `capability.audio.music`, `capability.audio.weather` |
| `capability.options.paging-max-vol` | `capability.audio.play-event`, `capability.audio.music`, `capability.audio.weather` |
| `capability.options.no-ovl-close-sfx` | `capability.audio.play-event`, `capability.audio.music`, `capability.audio.weather` |
| `capability.options.ovl-sfx-attack` | `capability.audio.play-event`, `capability.audio.music`, `capability.audio.weather` |
| `capability.options.no-combat-sfx` | `capability.world.walk`, `capability.audio.play-event` |

Это scenario references, а не новые prerequisite edges и не перенос ответственности за caller. Если fixture ссылается на поздний C/D/E outcome, используйте раннюю ветвь ровно в пределах B option contract, а полный поздний caller сохраняйте pending с собственными регрессиями.

## Версии, build gates и источники

- `capability.options.audio-paging`, `capability.options.no-ovl-close-sfx`, `capability.options.ovl-sfx-attack`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — T; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..
- `capability.options.paging-master-vol`, `capability.options.paging-max-vol`, `capability.options.no-combat-sfx`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — F; E=T; —; retain consumer feature/version/runtime gates and Send_options projection..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

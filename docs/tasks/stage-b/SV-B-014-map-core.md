# SV-B-014 — Карта, палитра и согласованный viewport

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Серверная карта отображается целиком с terrain/foreground, weather и корректным normal/big viewport.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-006](SV-B-006-login.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-013](SV-B-013-fonts-load.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `startup-tiles`: producer [SV-B-014](SV-B-014-map-core.md); полные owners [SV-B-039](SV-B-039-map-effects.md). 014 реализует штатную production tile/subset composition для approved graphics1 и selected16x24sv: resource/PRF mappings, terrain/foreground placement и masks по этому режиму, palette, scaled prepared assets1:1, normal/big viewport fit.039 остаётся полным owner всех tile/subset modes/gates/effects. Fresh own CFG/defaults с доступным16x24sv в014/020 действительно рисует тайлы; проверить selected/default subset и glyph/terrain placement по baseline reference. graphics0/font-only допустим только как предусмотренный failure для отсутствующего/непригодного ресурса, не обход незавершённого renderer.039 повторяет штатный M1 и достраивает mode2/прочие subsets и source branches.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-020](SV-B-020-first-session.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-039](SV-B-039-map-effects.md), [SV-B-041](SV-B-041-layout-settings.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-013](SV-B-013-fonts-load.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.rendering.glyph-identity` | Preserve numeric legacy glyph IDs, source profile mapping, colour/control roles and font default then visible one-cell fallback; no direct TTF raw-byte decoding or guessed PCF origin. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.terrain-stack` | Keep background, terrain and foreground identities independent through effects/weather/cursor; removing foreground reveals latest terrain state. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.palette` | Apply canonical palette roles and runtime tint without rebuilding neutral assets; honour lighterDarkBlue, monochrome and allowed custom entries. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.resize` | Logical UI units map to output pixels with current DPI; integer final-cell geometry fits complete66x22/66x44 centered grid. No crop, map zoom key or permanently scaled whole-window framebuffer. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.wipe` | Clearing a semantic rectangle removes previous glyph/tile/cursor content, preserves unaffected cells and reconstructs current state on repaint. | [main-sdl3.c:1730](../../../src/client/main-sdl3.c#L1730)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.weather` | Preserve weather parameters/type/start/stop/wind and redraw lifecycle, independent overlay and no2mask_weather branch; resize/recreate cannot replay start sounds or freeze packet application. | [c-xtra1.c:5457](../../../src/client/c-xtra1.c#L5457)<br>[nclient.c:422](../../../src/client/nclient.c#L422)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.world.read-map` | Display authoritative cells and line batches, overlays and player position, preserving glyph identity and protocol frame boundaries; normal/big-map resize never fabricates world movement. | [nclient.c:385](../../../src/client/nclient.c#L385)<br>[nclient.c:1672](../../../src/client/nclient.c#L1672)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Receive_char/line/map state через production decode: numeric glyph identity, masks/layers/palette, attr escapes, versions/optional gates и split/chained updates.
2. 66×22 и66×44 целиком fits при minimum1024×768,100/125/150/200%DPI; no crop/map scroll/user zoom/automatic layout switch. Resize/palette/weather не повторяют game commands.
3. Weather input producer и audio weather interaction, clear/wipe/restoration; current state survives focus/resize/resource generation changes.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.rendering.glyph-identity` | Numeric legacy IDs, encoding profile/default glyph и one-cell fallback; nonzero PCF origin and no TTF byte guess Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.terrain-stack` | Background/terrain/foreground identities независимы; foreground remove раскрывает latest terrain Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.palette` | Runtime palette/tint+lighterDarkBlue/monochrome values не rebuild neutral assets и no stale cache Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.resize` | 66x22/66x44 whole centered map with integer final cells, DPI/logical units and no crop/zoom Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.wipe` | Semantic rectangle clears old glyph/tile/cursor only within range; repaint reconstructs current state Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.weather` | Type/wind/start/stop/volume and no2mask branch; redraw/resize not duplicate start sound Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.world.read-map` | Один соответствующий intent; authoritative success/blocked result; normal/roguelike/macro; prompt/target cancel без action, resize/relogin без повторной отправки. Точный проверяемый результат: Display authoritative cells and line batches, overlays and player position, preserving glyph identity and protocol frame boundaries; normal/big-map resize never fabricates world movement. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.rendering.glyph-identity`, `capability.rendering.terrain-stack`, `capability.rendering.palette`, `capability.rendering.resize`, `capability.rendering.wipe`, `capability.rendering.weather`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; USE_GRAPHICS, GRAPHICS_BG_MASK (dual-mask), TEST_RAWPICT, EXTENDED_COLOURS_PALANIM and per-packet version rules where applicable; ASCII paths still required..
- `capability.world.read-map`: versions — Receive_char and Receive_line_info: legacy/32-bit glyphs, negotiated char_transfer_bytes, GRAPHICS_BG_MASK two-layer paths and RLE sentinels; preserve every row in session-reconciliation.md packet annex.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

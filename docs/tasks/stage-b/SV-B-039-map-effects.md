# SV-B-039 — Tiles, анимации, lighting и восстановление

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Карта отображает configured tiles/subsets/effects в правильном масштабе и возвращается к текущему состоянию.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-038](SV-B-038-font-choices.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `startup-tiles`: producer [SV-B-014](SV-B-014-map-core.md); полные owners [SV-B-039](SV-B-039-map-effects.md). 014 реализует штатную production tile/subset composition для approved graphics1 и selected16x24sv: resource/PRF mappings, terrain/foreground placement и masks по этому режиму, palette, scaled prepared assets1:1, normal/big viewport fit.039 остаётся полным owner всех tile/subset modes/gates/effects. Fresh own CFG/defaults с доступным16x24sv в014/020 действительно рисует тайлы; проверить selected/default subset и glyph/terrain placement по baseline reference. graphics0/font-only допустим только как предусмотренный failure для отсутствующего/непригодного ресурса, не обход незавершённого renderer.039 повторяет штатный M1 и достраивает mode2/прочие subsets и source branches.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-046](SV-B-046-config-render.md), [SV-B-055](SV-B-055-options-maplight-1.md), [SV-B-056](SV-B-056-options-maplight-2.md), [SV-B-057](SV-B-057-options-mapidentity-1.md), [SV-B-058](SV-B-058-options-mapidentity-2.md), [SV-B-059](SV-B-059-options-animation-1.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-038](SV-B-038-font-choices.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.rendering.tiles` | Prepare authored regions and decoded single/two-mask layers to final integer cell size, draw1:1; preserve recolour/background-hole/outline semantics and server gates. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.subsets` | Select independent subtileset/mapping overrides with baseline lookup precedence; preserve mixed tile/glyph content and missing-resource failures. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.animation` | Preserve animated palette categories, timing, no_flicker/subterm/misc scopes and controlled-RNG semantics, excluding pixel-identical random particles. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.lighting` | Preserve torch/wall/floor/shading semantics and full-map TERM_SRCLITE timing; fix repeated lower-half big-map pattern rather than preserving defect. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.cursor` | Present semantic self/target/cell indicators with approved HTML design and shared geometry; no required X11 XOR/SDL alpha pixel match. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.effects-restore` | After transient effect ends restore current semantic cell including updates underneath and remaining layers; never restore stale raster snapshot. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
| `capability.rendering.solid-wall` | Preserve solid wall and equal-foreground/background solid-block intent; FONT_MAP_SOLID_X11 is a drawing role rather than missing control glyph. | [main-sdl3.c:1730](../../../src/client/main-sdl3.c#L1730)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Terrain/foreground masks, lighting/cursor/solid-wall/subset and animation scopes; final-size prepared assets compose1:1, no forbidden Lanczos/XHTML.
2. Effect/wipe/restoration после resize/font/tileset/palette generation; no stale cache и repeated command/effect on redraw.
3. Анимация auxiliary inventory/messages не переопределяет main map scopes; точные option switches проверяют отдельные option-owner tickets.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.rendering.tiles` | Single/two-mask, recolour/background hole/outline decoded then final integer raster1:1; mixed glyph/tile Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.subsets` | Independent subtileset overrides precedence, missing subset fallback and mixed content identity Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.animation` | No_flicker/subterm/misc scopes, timing and controlled RNG preserve category behavior Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.lighting` | Torch/wall/floor shading; TERM_SRCLITE big-map нижняя половина не повторяет upper pattern Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.cursor` | Self/target/cell semantic indicators, geometry/hit aligned с approved UX Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.effects-restore` | Effect end restores latest semantic cell and remaining layers, not stale saved raster Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |
| `capability.rendering.solid-wall` | Solid walls/equal foreground-background intent is render role, not absent control glyph Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.rendering.tiles`, `capability.rendering.subsets`, `capability.rendering.animation`, `capability.rendering.lighting`, `capability.rendering.cursor`, `capability.rendering.effects-restore`, `capability.rendering.solid-wall`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; USE_GRAPHICS, GRAPHICS_BG_MASK (dual-mask), TEST_RAWPICT, EXTENDED_COLOURS_PALANIM and per-packet version rules where applicable; ASCII paths still required..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

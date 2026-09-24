# SV-B-013 — Рабочие шрифты и отказ ресурсов

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Текст и карта получают подходящие PCF/TTF profiles; отсутствующий ресурс показывает requested/effective fallback без скрытой смены настройки.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `live-font-replacement`: producer [SV-B-013](SV-B-013-fonts-load.md); полные owners [SV-B-038](SV-B-038-font-choices.md). 013 вводит production request→prepare→activate/reject path и минимальный native resource-change child для воспроизводимого отказа живого ресурса.Это рабочий SV child;038 достраивает полные selection/preview/Save/cancel paths и остаётся владельцем обоих полных chooser IDs. 013 проверяет failure retaining previous working stack и generation cleanup через real production child;038 повторяет failure и полный role-specific chooser matrix. Fixture подаёт resource fault, не заменяет chooser behavior.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-014](SV-B-014-map-core.md), [SV-B-020](SV-B-020-first-session.md), [SV-B-038](SV-B-038-font-choices.md), [SV-B-039](SV-B-039-map-effects.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.fonts.pcf` | Load custom/bundled PCF via FreeType encoded-ID/charmap/metrics/default semantics including nonzero origins and partial ranges; prepare glyphs at final geometry, never auto-replace successful PCF for aesthetics. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.ttf` | Load bundled v2407.24 CascadiaMono-Regular.ttf/profile with OFL/copyright and custom TTF via SDL_ttf final-metric rasterization; preserve one-cell identity and proportional-font contract. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.fallback` | Startup text falls back to bundled Cascadia then16x24x PCF; map to16x24x PCF; all fonts failing stops startup with resource error. Keep requested name, effective resource and failure distinct. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.live-failure` | Validate replacement before publish; failed live font/profile/tiles selection retains previous working stack and draft requested/effective distinction. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.tile-fallback` | Unusable selected tileset falls back to font-only map with message, preserving requested tileset/enabled preference for later startup. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.asset-validation` | Discover and validate every bundled font/tileset/profile; deep corpus includes9x15,9x15tg,16x24tg+16x24sv,12x24/8x16 origin and16x22 partial range. Separate Linux software/accelerated and Windows evidence. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.rendering.cache-generation` | Key prepared assets by source/profile/mask/subset/filter/outline/final-size generation in distinct SV namespace; atomically publish; live resize may briefly scale old assets while updates/hit testing continue. | [main-sdl3.c:1947](../../../src/client/main-sdl3.c#L1947)<br>[main-sdl3.c:4767](../../../src/client/main-sdl3.c#L4767)<br>[nclient.c:1](../../../src/client/nclient.c#L1) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Проверить весь утверждённый asset/profile corpus, numeric glyphs/control markers и font-mapping lifecycle; cache key включает profile/source generation.
2. Startup fallback text Cascadia→16x24xPCF, map16x24xPCF, broken tiles→font-only; если все font fallbacks не работают — явная startup error.
3. Live prepare-before-activate: injected parse/upload/resource disappearance failure сохраняет previous working stack; при cleanup нет dangling borrowed frame views.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.fonts.pcf` | Encoded-ID/default/charmap/metrics,12x24/8x16 origin1 and16x22 partial range, successful custom PCF not replaced Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.ttf` | Cascadia v2407.24 metadata/license and custom proportional TTF final-metric one-cell rendering Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.fallback` | Requested/effective names distinct; text Cascadia→16x24x and map16x24x; all failing means startup resource error Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.live-failure` | Invalid replacement cannot publish; previous working stack kept, request/draft failure visible Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.tile-fallback` | Missing/invalid selected tiles produces font-only map+message; requested preference survives for later recovery Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.asset-validation` | All bundled basic load/metadata/default/range; deep9x15/9x15tg/16x24tg+sv/origin/partial corpus Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.rendering.cache-generation` | Source/profile/mask/subset/filter/outline/size generation; replacement atomic, stale async discard; resize keeps update/hit Production decode→state→surface; Linux software/accelerated and Windows software, updates under overlay, focus/resize/reopen; stage budgets. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.fonts.pcf`, `capability.fonts.ttf`, `capability.fonts.fallback`, `capability.fonts.live-failure`, `capability.fonts.tile-fallback`, `capability.fonts.asset-validation`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..
- `capability.rendering.cache-generation`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; USE_GRAPHICS, GRAPHICS_BG_MASK (dual-mask), TEST_RAWPICT, EXTENDED_COLOURS_PALANIM and per-packet version rules where applicable; ASCII paths still required..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

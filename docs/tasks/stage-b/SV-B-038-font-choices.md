# SV-B-038 — Выбор шрифтов и фильтров в живой сессии

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок независимо меняет text/map font, graphics/PCF filters и outline с честными preview/save/failure.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-020](SV-B-020-first-session.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `live-font-replacement`: producer [SV-B-013](SV-B-013-fonts-load.md); полные owners [SV-B-038](SV-B-038-font-choices.md). 013 вводит production request→prepare→activate/reject path и минимальный native resource-change child для воспроизводимого отказа живого ресурса.Это рабочий SV child;038 достраивает полные selection/preview/Save/cancel paths и остаётся владельцем обоих полных chooser IDs. 013 проверяет failure retaining previous working stack и generation cleanup через real production child;038 повторяет failure и полный role-specific chooser matrix. Fixture подаёт resource fault, не заменяет chooser behavior.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-039](SV-B-039-map-effects.md), [SV-B-041](SV-B-041-layout-settings.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-013](SV-B-013-fonts-load.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.fonts.text-select` | Select text font/profile independently of map definitions; persist requested selection privately only on Save. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.map-select` | Select map font/profile preserving map-font→mapping reload→graphics override and required server visual-definition updates; text-only changes do not trigger this. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[nclient.c:9204](../../../src/client/nclient.c#L9204)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.graphics-filter` | Select Nearest/Linear/PixelArt for authored tiles/raw pictures; decode encoded masks first and use Nearest recolour coverage/authored outline. Unsupported PixelArt explicitly falls back to Nearest, preserving request. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.pcf-filter` | Independent Nearest/Linear/PixelArt PCF filter defaults Nearest; font successful identity remains unchanged and glyph boundaries do not bleed. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
| `capability.fonts.outline` | Generate configured outlines at final size within individual tile bounds; no subsequent steady-state scaling. | [main-sdl3.c:1065](../../../src/client/main-sdl3.c#L1065)<br>[main-sdl3.c:1](../../../src/client/main-sdl3.c#L1) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Text-only change не reloads map mappings; map font→mapping reload→graphics overrides, requested names full/local with safe reporting copy.
2. PCF/TTF role-specific scaling, Linear/Nearest/PixelArt allowed paths, unsupported PixelArt effective Nearest while retaining request; live replacement prepare-before-activate.
3. Preview→Cancel, Save→restart, missing profile/resource, fallback exhaustion и cache invalidation; full accepted early font/resource scope сохраняется.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.fonts.text-select` | Independent text role preview+Save, no map mapping/server definitions changed Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.map-select` | Map font→mapping reload→graphics override→server visual update, independent from text role Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.graphics-filter` | Nearest/Linear/PixelArt authored layers; unsupported PixelArt explicit Nearest effective fallback, requested retained Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.pcf-filter` | Independent PCF filter nearest default, no boundary bleed or glyph ID change Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |
| `capability.fonts.outline` | Final-sized per-tile outline inside bounds, no later steady scaling Font/resource disappearance/reappearance, fail before publish, recreate current scene; separate Linux/Windows evidence. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.fonts.text-select`, `capability.fonts.map-select`, `capability.fonts.graphics-filter`, `capability.fonts.pcf-filter`, `capability.fonts.outline`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

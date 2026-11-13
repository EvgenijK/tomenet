# SV-C006-005 — final-size resource preparation, filters, outlines и tile fallback

Статус: accepted, 2026-11-09; принят пользователем в согласованных границах.
Labels: enhancement
Stage outcomes: `capability.fonts.tile-fallback`, `capability.fonts.graphics-filter`,
`capability.fonts.pcf-filter`, `capability.fonts.outline`.
Зависимости: production font owners C006-003/004, active map consumer
[C006-001](SV-C006-001-map-resource-integration.md) и preference stack
[C006-002](SV-C006-002-map-preferences-integration.md).

## Контракт и границы

Нормативны [CONTEXT](../../../CONTEXT.md), [settings policy](../../capabilities/settings-policy.md)
(resource fallbacks и raster preparation), [canonical coverage](../../capabilities/native-coverage.json),
[reconciliation](../../capabilities/settings-reconciliation.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md) и
[0006](../../adr/0006-errors-and-overload.md).

C006 реализует свои resource outcomes через настоящий production preparation и
rendering consumer. Enum, loader без применения, injected effective snapshot,
отдельный test-only renderer и декларация поддержки backend не являются результатом.
Минимальный consumer выводит подготовленные glyph/tile/raw-picture assets тем же
production путём, который продолжит C007. Полная игровая композиция, animation,
weather, terrain/foreground lifecycle, active graphics reporting FI-02 и coupled
graphics PRF reset/overrides/reload принадлежат C007. Их отсутствие не переносит
четыре outcomes этой задачи из C006 и не требует ожидать приёмки C007.

## Работа

1. Реализовать SV-local подготовку исходных assets непосредственно к конечному
   целочисленному размеру клетки/output rectangle. Steady-state rendering — 1:1,
   без постоянного scaling font-sized raster или всей карты. Source/profile,
   mask/subset, final size, filter и outline policy участвуют в generation identity;
   публиковать готовую generation согласованно, не теряя предыдущую при ошибке.
2. Graphics filter — независимые requested/effective Nearest / Linear / PixelArt,
   default Linear. Authored-color tile regions фильтровать выбранным режимом;
   encoded mask keys сначала декодировать в source-defined роли, затем
   recolor coverage и authored outline готовить Nearest. Не интерполировать encoded
   mask colors. Сохранить background holes, независимые subsets и границы assets.
   Raw picture готовить из исходного rectangle к final size выбранным graphics
   filter, сохраняя anchors/extents/clipping и special-canvas transform; обычные
   tile masks не навязывать.
3. PCF filter — отдельный selector тех же режимов, default Nearest, применимый
   к обоим font roles. Фильтровать изолированную decoded glyph coverage: не менять
   encoded ID/default/metrics и успешно выбранный PCF, исключить bleed соседних
   glyphs. Не подменять шрифт другим семейством/размером/TTF ради вида.
4. Проверять действительную поддержку PixelArt на preparation/backend path.
   При неподдерживаемом пути effective Nearest с сообщением и сохранением requested
   PixelArt. Успешный `SDL_ScaleSurface` или наличие enum не доказывают алгоритм:
   CPU surface path способен молча заменить его Nearest. Diagnostics показывают
   фактический backend и requested/effective для каждого применимого selector.
5. Generated outline рассчитывать с выбранной baseline толщиной при подготовке
   final size внутри индивидуальных tile bounds. Соседние tiles не участвуют;
   последующего steady-state scaling нет. Authored outline сохраняет свою роль.
6. При startup непригодном выбранном tileset реально активировать font-only map
   с диагностикой, сохранив requested tileset и enabled preference для следующего
   запуска/восстановления. Requested graphics не выдавать за active graphics.
   До live resource replacement подготовить пригодный набор; failure сохраняет
   прежний рабочий stack и видимый неуспешный draft согласно C006-004. Missing glyph
   обрабатывается font-default/visible one-cell fallback, отдельно от отказа assets.
7. Частные font/filter/outline selections, необходимые этим outcomes, передавать
   собственному C006 selection/publication owner, сохранять только explicit Save;
   не переносить весь configuration registry C056 или общую Settings surface C044.
   Assets/caches остаются в принятых shared owners с отдельным SV cache namespace.

## Завершение и минимальная Linux проверка

Переиспользовать production core checks C006-003/004 и map consumer 001/002.
Добавить только необходимое доказательство различимых graphics/PCF selectors,
Nearest mask roles, final-size outlines и их bounds, no bleed, реального
PixelArt support либо объявленного fallback. Сверять semantic identity, geometry,
roles и lifecycle, без pixel-perfect gate. Через production output показать
font-only map и сообщение при invalid/missing tiles; requested tiles/enabled
сохраняются, successful live replacement применяется, failed replacement сохраняет
старый stack. Repaint/recreation не выполняют повторные preference effects.

Требуемый результат — фактическое применение подготовленных assets consumer,
а не только промежуточные buffers или screenshots без caller trace. Для каждого
из четырёх `fonts.<name>` закрыть `obligation.fonts.<name>.result`,
`obligation.fonts.<name>.failure-lifecycle` и
`obligation.fonts.<name>.allocation-checks`: всего 12 canonical obligations.
Зафиксировать caller/revision, Linux команды, requested/effective/backend,
применимые failure outcomes и reuse evidence. Complete C007 gameplay flows
остаются отдельными и не получают PASS из этой проверки.

Проверки — необходимый минимум Linux core через production-путь; новый harness,
полная visual/fault matrix или расширение набора заранее согласуются по AGENTS.md.
[Windows/MinGW/Wine deferred](../../agents/platform-validation.md). Canonical
IDs/conditions/obligations/allocation и runtime statuses этой задачей не меняются.

## Результат реализации — 2026-11-09

Production SV реализация и минимальные Linux core проверки выполнены.
[Фактическое evidence и ограничения](../../sv-c006-evidence.md) отделяют
software resource consumer от поздних gameplay flows и непроверенных backends.
Canonical runtime registration и пользовательская приёмка не выполнялись.

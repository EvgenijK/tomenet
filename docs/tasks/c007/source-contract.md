# C007 — проверенные источники и production seams

Дата сверки: 2026-11-09. Статус: planning, без нового runtime evidence.
Graphify query `C007 rendering terrain-stack weather lighting resize cache-generation`
выполнен до поиска исходников. Граф дал навигацию; факты ниже сверены с текущим
workspace после C006, включая незакоммиченные изменения. Строки — навигация,
перед реализацией проверить актуальные тела функций и version/build gates.

Нормативны [CONTEXT](../../../CONTEXT.md), [settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md), ADR [0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md), [0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md), [0006](../../adr/0006-errors-and-overload.md).
Canonical IDs/conditions/obligations и allocation сохраняются. Старые Stage B
rendering tickets — историческая навигация, не восстановленный acceptance gate.

## Baseline и принятые корректировки

| Источник | Контракт и существенные ветви |
|---|---|
| `src/client/main-sdl3.c:1730` `Term_wipe_sdl3`, `1745` `Term_curs_sdl3`, `1778` `Term_text_sdl3` | Очистка диапазона, индикаторы и font path; SV реализует semantic state и final-cell geometry, не виртуальный Term или SDL alpha pixel oracle |
| `main-sdl3.c:2050` `term_data_draw_graphics_tiles`, `2095` `Term_pict_sdl3`, `2147` `Term_pict_sdl3_2mask` | Background fill, masks/recolour, single/two-mask, foreground/background equality как solid block, glyph-indexed subset lookup; terrain/foreground identities остаются раздельными. Background `0`/space и несуществующий layer имеют отличающуюся семантику |
| `main-sdl3.c:5062` `animate_palette`; `src/client/z-term.c:289` `term2attr`, `1008`, `1133`, `1494` | Палитра, animated categories, no_flicker/subterm/misc scopes, lighting и TERM_SRCLITE. Терминальные scope переводятся в семантические surfaces; topology legacy не переносится |
| `z-term.c:1033` | Legacy повторяет TERM_SRCLITE в нижней половине big map. По policy SV рассчитывает полную геометрию без повторения этого дефекта |
| `src/client/c-util.c:20137` `handle_process_graphics_file` | Reset rawpic/masks → graphics base PRF → enabled subsets при graphics offset → initialization даже после missing PRF → pseudo-object definitions. Base failure и subset failure имеют различный порядок продолжения |
| `c-util.c:20224` `handle_process_font_file` | Общий font reset, font PRF при offset 0, затем graphics stack, server definitions/font notification и repaint. Graphics-only reset не должен стирать новый font base |
| `src/client/c-files.c` `process_pref_file_aux_aux`; `src/client/sv/settings/preferences.c` | R/K/F/U mappings, I raw-picture geometry и m masks потребляет единый production `SvPreferences`; filename/context offsets и shared glyph-subset identity не пересоздаются renderer parser |
| `src/client/nclient.c:3162` `Receive_char` | Numeric glyph/attrs, foreground/background, self indicator. Раздельные gates 4.8.1 (wide IDs), 4.9.2.1 (two-mask); negotiated 1/2/3/4-byte transfer; `PKT_CHAR_DIRECT` не равен обычному routed update |
| `nclient.c:4064` `Receive_line_info` | Версионный compressed row decode, partial rollback и semantic cells. В том же legacy handler есть minimap branches; C015 не включать скрыто в C007 |
| `nclient.c:6482` `Receive_weather` | Type/wind/generation/intensity/speed/panel/clouds, halt/pregenerate/stop и one-shot sound intent. Legacy rollback восстанавливает buffer pointer после частичного cloud loop, но уже изменённые globals не являются нормативным atomicity поведением: SV ничего не публикует до полного packet |
| `nclient.c:6637` `Receive_palette` | Пять bytes c/index/R/G/B, применимость palette_animation; palette update меняет runtime tint, а не source assets |
| `nclient.c:7456` `Receive_weather_colouring` | До 4.7.4.6 rain/snow, начиная с gate также sand colour/character; original byte roles и порядок с palette сохраняются |
| `src/client/c-xtra1.c:5463` `do_weather`, `src/client/nclient.c:8442` | Production weather timing и controlled RNG, rain/snow/sand/clouds; `no_weather` сохраняет применимые non-weather flash/lightning обязанности; ASCII и `no2mask_weather` имеют отдельные branches |
| `docs/capabilities/settings-policy.md:247–306` | Canonical raster pipeline, logical units/DPI, full 66×22/66×44 maximum fit, integer final cells, final-size direct preparation, runtime palette, atomic generations, effects restore latest semantic cell |

`USE_GRAPHICS`, `GRAPHICS_BG_MASK`, `TEST_RAWPICT`,
`EXTENDED_COLOURS_PALANIM` и packet-version branches нельзя заменить одним happy
path. Это не требует подключения всего legacy backend в SV: source semantic
rules локально адаптируются с минимальным изменением shared/legacy кода.
Pixel equality с X11/SDL3 не является критерием.

## Что уже предоставляет C006

[C006 evidence](../../sv-c006-evidence.md) и [приёмка](../../sv-stage-c006-acceptance.md)
описывают принятый core с явными ограничениями. Его source-contract был снимком
**до реализации**; использовать таблицу пробелов оттуда как текущую нельзя.

| Production seam | Текущее состояние и использование C007 |
|---|---|
| `ui/font.[ch]`: `sv_font_open_profile_file`, `sv_font_render_glyph`, `sv_font_metrics` | Numeric PCF IDs/default/presence и explicit TTF mapping, final-sized glyph surfaces, solid ID 2, visible fallback. Builtin TTF profile остаётся ASCII-only; unknown game IDs не декодировать как UTF-8 и не считать replacements parity |
| `ui/resource-preparation.[ch]` | PNG/BMP, `sv_resource_prepare_tile/raw/coverage`, mask decode до filtering, Nearest/Linear, явный CPU PixelArt→Nearest, runtime composite, failure сохраняет старый prepared result. Использовать для настоящих base/subset cells, не переписывать loader/filter core |
| `ui/font-selection.[ch]` | Independent Text/Map roles, requested/effective, preview/Cancel и private Save; resource selection и render activation можно связать узким существующим prepare callback |
| `settings/map-font.[ch]`: `sv_map_font_bootstrap/reload/definitions` | Реальный font PRF owner: reset, custom U→B/system, clone candidate с load effects только текущего поколения, `SvVisualDefinitions`. Сейчас bootstrap явно ставит graphics_enabled=false; C007 добавляет graphics phase после font и до own OPT |
| `settings/preferences.[ch]`: `sv_preferences_clone`, `reset_visuals`, mapping/glyph-subset accessors | Один PRF parser и retained effects. Existing reset удаляет R/K/F/U/I/m и glyph-subsets целиком; если нужен graphics-only reset после нового font base — узкое дополнение этого owner, не альтернативное состояние parser |
| `files/files.[ch]`, `settings/config.[ch]` | Owner-aware U-before-B, missing/invalid/IO, requested graphics 0/1/2, default 1 + 16x24sv, subsets, filters/outline. Requested выбор сам по себе не доказывает активацию |
| `protocol/visuals.h` `sv_visual_encode`; `protocol/login.c:290–308` | Shared SV semantic definitions, точные различные PLAY/live table gates. Live setup+font append атомарен при backpressure. Это serializer, не active graphics proof |
| `protocol/resource-report.h` | Единый C003 wire/reporting owner, bounded 79-byte copies; default graphics/tiles пока disabled/NO_GRAPHICS. C007 публикует actual mode/tiles/font согласованно, не изобретает wire fields для cache generation |
| `login/native-run.c:454` `apply_map_font`, `589–603` | Уже подключён реальный map font bootstrap hook; прежнее утверждение про NULL resource hook устарело. Live font candidate сначала сериализуется/проверяется и затем публикуется. Coupled font→graphics reinit расширяет этот lifecycle |
| `ui/font-editor.c:161` `prepare`; `login/native-run.c:695` | Production F6 resource preview использует реальные fonts, tile/raw preparation и software draw. Это ресурсный consumer C006, не игровая карта, не weather/terrain composition и не active tiles reporting |
| `protocol/login.c:377` | После `PKT_END` login удерживает остаток для будущего live owner; live map/weather decoder/transport consumer сейчас здесь отсутствует. Не перехватывать неизвестные packets в renderer и не выбрасывать stream remainder |

## Инженерная граница C007 и поздних этапов

C007 нужен reusable production semantic cell/geometry/composition owner и реальные
weather/palette packet adapters для собственных rendering paths
(weather дополнительно имеет отдельную wire obligation). Полный map row/char
ingress и `Receive_line_info` flow остаются C013; его позднее decode→state→surface
evidence сохраняется pending до настоящего consumer, а не становится hard scope C007. Он принимает
version-aware decoded changes в session presentation state; renderer не читает
legacy globals, packets или Term. Main thread владеет state, generations и
publication, UI caches производны. Resource generation и session generation —
разные lifetimes.

C013 остаётся владельцем полного `world.read-map` live flow и его native session
surface; C011/C014 — session preparation/lifecycle, C015 — overview/locate,
C018 — targeting interaction, C044/C056 — полные настройки, C055 — полный audio.
C007 не принимает эти outcomes и не требует предварительно завершить их целиком.
Проверки C007 должны вызывать созданный production decoder/model/renderer seam
через existing core/peer entrypoints; тот же код затем подключается поздними
consumers. Test-only map/effects implementation, инъекция готовой картинки или
snapshot вместо actual activation не доказывают C007. Если для native consumer
нужен узкий rendering host, он вызывает тот же production owner и явно не
объявляется завершённой live игровой сессией.

Weather start/stop/volume intent создаётся в semantic transition ровно один раз;
repaint/resize/reopen его не создают. Полный audio pack/device playback не включать
скрыто в C007. PRF message/action effects сохраняют payload/context/generation и
ownership без replay; actual delivery остаётся C016/C053 по existing 002.

## Geometry и вопросы

Maximum fit, отсутствие map zoom key, 66×22/66×44, integer output cells, DPI и
общий hit transform уже приняты; повторного выбора пользователя не требуется.
Точные resource/cache budgets и debounce выбираются по минимальному необходимому
измерению, без объявления неподтверждённой performance parity.

Для cursor действует [решение 31](../../../.scratch/single-window-sdl3-client/issues/31-prototype-cursor-and-cell-indicators.md):
пользователь отклонил варианты A/B/C и передал внешний вид основному HTML UX
prototype `/home/svechnik/Projects/github_site/tomenet_interface`.
`css/cursors.css` определяет theme-aware mouse cursors и targeting pointer;
`css/windows.css:325–354` содержит `.throw-targeting`/`.throw-target-cell`: cyan
cell border/fill и crosshair, привязанные к target width/height. Это разные роли
указателя мыши и клеточного индикатора. C007 использует основной HTML reference
и сохраняет semantic states/geometry; новый обязательный выбор формы сейчас
не требуется. Отклонённые формы/толщина/анимации из старого эскиза не становятся
default, полноценный targeting interaction остаётся C018.

Startup graphics failure следует уже принятой policy: явная диагностика и
font-based fallback с сохранённым requested выбором; failed live activation
сохраняет прежний whole stack. Missing PRF после пригодной загрузки sheet не
равен failed activation: baseline выполняет initialization/default masks после
load attempt. Invalid/unreadable U overlay не маскируется как missing B search.

## Минимальные Linux проверки

Новых проверок в этом анализе не запускалось. На реализации дополнять существующие
core/peer checks только необходимым доказательством собственных production paths:

- Один различимый base/subset corpus: actual resource resolve→PRF mappings→prepare→
  activate→render use→PLAY/live reporting; glyph/tile mix, masks, raw extents,
  offsets и glyph-subset sharing. Existing parser/loaders suites не дублировать.
- Semantic cell updates под effect/weather/cursor и wipe, затем remove/stop:
  latest terrain и оставшиеся layers восстанавливаются без stale raster.
- Normal/big complete grid, shared geometry/hit test, resize/DPI/recreation:
  final-size assets 1:1 после generation change; полный TERM_SRCLITE без half repeat.
- Controlled clock/RNG для lighting/animation/weather и no_flicker/scopes;
  applicable split/chained wire inputs не публикуют partial state. Retain порядок
  полного stream, gates/widths и невоспроизведение start/PRF effects при repaint.
- Actual enabled/disabled/fallback, failed live activation, source/profile/cache
  generations и session switch; requested/effective и непроверенные backend
  branches остаются раздельно в evidence. Existing C003 serializers/C006 primitives
  переиспользуются вместо новой общей matrix/harness.

Текущая обязательная платформа — [Linux](../../agents/platform-validation.md).
Windows/MinGW/Wine deferred; расширение набора проверок заранее согласуется по
AGENTS.md. Не запускать старые полные matrix автоматически. Нового runtime PASS,
приёмки C007 или переоткрытия C003/C004/C006 этот документ не создаёт.


## После реализации core — 2026-11-09

Таблицы текущего состояния выше — snapshot перед реализацией C007. Новые
production owners/callers и проверенные результаты перечислены в
[evidence](../../sv-c007-evidence.md); поздние C013/C014 flows и непроверенные
условия этим не принимаются. C003/C004/C006 остаются приняты пользователем.

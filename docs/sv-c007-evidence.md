# C007 — Linux rendering core и production resource consumer

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Пользователь поручил реализацию агентам. Canonical registry не изменялся.
[Спецификация](sv-stage-c007-spec.md), [задачи](tasks/c007/README.md),
[точный planning snapshot 15 IDs/45 obligations](tasks/c007/coverage.json).
База Git — `2eb1c652103b56c4c75b9863c50ad432f438359c`; рабочее дерево содержит
предшествующие незакоммиченные C006 и другие изменения, которые сохранены.

## Реальный production путь

- `settings/graphics.[ch]` использует существующие C003 file owners, C004 PRF
  parser и C006 preparation. Base/subsets/masks/raw definitions подготавливаются
  до candidate publication; failure сохраняет прежние preferences/resources.
  Startup bootstrap: font → graphics → own OPT. Requested остаётся в CFG;
  effective mode/name и согласованные definitions идут в existing C003 serializers.
- `session/map.[ch]` хранит независимые background/terrain/foreground, numeric IDs,
  revisions и session generation; wipe ограничен заданной областью. Renderer
  принимает semantic view без Term, wire packets или legacy globals.
- `ui/map-renderer.[ch]` переиспользует C006 glyph/default/fallback, рисует реальные
  prepared tiles и independent transient overlays, solid roles, current-state
  restore, normal/big complete grid, DPI-aware fit и общую hit geometry.
- `session/map-effects.[ch]` и `map-colours.inc` дают SV-local palette/animation,
  full-height searchlight, controlled clock/RNG, clouds/particles, transient stack,
  screenflash/lightning timelines и bounded one-shot sound intents. Queries не
  продвигают время/RNG. `protocol/rendering.[ch]` декодирует weather/palette/
  colouring/screenflash, сохраняя gates и whole-packet commit.
- `login/native-run.c` подключает graphics bootstrap и coupled live candidate,
  атомарные definitions/reporting, retained opening stack для Cancel, scalar filter
  preview/Save/Cancel через reprepare без PRF reload. Device reset очищает active
  и opening graphics textures; app resources отделены от session generation.
- На server selection F6 → F7 Map показывает существующий C006 resource preview
  и справа production C007 composition текущих PRF definitions/numeric glyph IDs.
  Hover и left-click target используют ту же geometry; right-click снимает target.
  Это настоящий native rendering consumer, **не live-world карта C013**.

Кэши bounded: glyph textures — 257 slots; graphics prepared entries — 1024 с
64 MiB CPU limit и производными bounded SDL layers. Tint меняется через texture
colour modulation без rebuild neutral source assets. Cache identity включает
source/profile/masks/subsets/filter/outline/final-size; session generation не
подменяет profile identity. `-T` запрещает reuse tile cache. Persistent tileset
cache сейчас не создаётся; `-t` честно отключает отсутствующий disk cache.
PixelArt на CPU сохраняет requested и явно использует effective Nearest.

## Проверки и воспроизведение

`make -C src -f makefile.sv -j4 tomenet-sv` — PASS, `-Wall -Wextra -Werror`.
[Build log](../.scratch/sv-c007/build.log). Новые production файлы находятся в SV;
legacy backends/common sources не изменялись в C007.

Три новых focused checks и расширенный существующий editor check вызывают
production API, compiled clang C99 с ASan/UBSan, `-fno-omit-frame-pointer`.
`ASAN_OPTIONS=detect_leaks=0`: LSan недоступен под ptrace среды, leak coverage не
заявляется. SDL software checks используют `SDL_VIDEODRIVER=dummy` и при
необходимости `SDL_RENDER_DRIVER=software`.

| Проверка | Наблюдение | Точный compile argv / лог |
|---|---|---|
| graphics | Actual base/subset BMP+PRF, shared glyph identity/offset, malformed valid tail, masks, raw extents и software draw; atomic failure/off/defaults/overlay; shipped 16x24sv; PLAY/FONT 78/79/80 boundaries; reprepare без PRF/effect replay | [argv](../.scratch/sv-c007/graphics-command.json), [log](../.scratch/sv-c007/graphics.log) |
| map-renderer | Terrain update под foreground, remove/restore/wipe/solid; numeric/default/visible fallback; indicator removal и overlay modes; normal/big fractional DPI/hits/reset | [argv](../.scratch/sv-c007/map-renderer-command.json), [log](../.scratch/sv-c007/map-renderer.log) |
| map-effects | Every partial weather prefix, cloud fields/atomicity, chained palette, 4.7.4.6 colouring, controlled time/RNG, full-height searchlight, actual particles, transient expiry, no replay и flash/lightning timeline | [argv](../.scratch/sv-c007/map-effects-command.json), [log](../.scratch/sv-c007/map-effects.log) |
| font-editor | Existing role/preview/Save/Cancel/reset checks плюс actual C007 compositor attachment, graphics mappings и различимый target draw | [argv](../.scratch/sv-c007/font-editor-command.json), [log](../.scratch/sv-c007/font-editor.log) |
| existing login | Regression исходных PLAY/font/default layouts после добавления coherent graphics snapshot API | [argv](../.scratch/sv-c007/login-command.json), [log](../.scratch/sv-c007/login.log) |

Отдельный bounded native smoke вызывает **тот же `sv_endpoint_run`**, связываясь
с production objects из текущей сборки. Fixture подаёт только SDL events/source
poll, не реализует renderer/resource поведение: F6/F7, Map font preview, F9/F10,
Map Save, несохранённая filter change/Cancel, Text Save и device reset. On-disk
CFG проверен: `9x15tg.pcf`, `nearest`, outline `0`. Server contact не выполнялся;
chronicle HTTP providers в этой среде недоступны. [Fixture](../.scratch/sv-c007/native-events.c),
[compile argv](../.scratch/sv-c007/native-events-command.json),
[log](../.scratch/sv-c007/native-events.log). Production objects в этом smoke —
обычная O2 сборка, отдельно от ASan/UBSan core binaries. Повторный запуск smoke
требует вернуть его isolated `native-events-profile/sv/tomenet.cfg` к
`svSchemaVersion 1` и `sound 0` перед запуском.

[Native resource composition capture](../.scratch/sv-c007/native-map.png)
осмотрен визуально: настоящие numeric glyphs и prepared definitions, cyan target
внутри fitted grid. Это resource QA, не full gameplay/UX acceptance.

## Конкретные source/identity/wire observations

В native smoke startup requested/effective graphics совпали: `1 / 16x24sv`,
generation `11420071573672862897`; map font — `lib/xtra/font/16x24x.pcf`.
Файловый owner разрешил bundled `lib/xtra/graphics/16x24sv.bmp` и
`lib/user/graphics-16x24sv.prf`; U overlay отсутствовал. Font system PRF
`font-sv.prf` отсутствовал, явно применены server-authoritative zero overrides
до graphics phase. Эти значения взяты из native log и production source lookup,
а не из requested setting как доказательства активации.

В focused graphics fixture реально загружены U `2x2test.bmp` и
`2x2test#0_green.bmp` с соответствующими `user/graphics-*.prf`:
`F[1].character=256`, `K[2].character=256`, glyph `256` использует subset `0`;
actual green channel больше red (выбран subset image). Raw `I[1]` source extent
4×2 при final cell 4×4 даёт actual output 8×4. Reprepare Nearest→Linear/outline2
меняет generation; возврат Nearest/-1 возвращает исходную generation, смена
только session context 7→8 её не меняет. Failed candidate сохраняет mappings и
прежнюю generation. Числовые fingerprints source files сохранены отдельно.

Production PLAY для fixture version 4.9.4 содержит mode bytes `00 02`, tiles
`32 78 32 74 65 73 74 00` (`2x2test` + NUL), font `map.pcf` + NUL. Live update
публикует `PKT_CLIENT_SETUP` и `PKT_FONT` одним append; FONT mode также `00 02`.
Для full local names длиной 78/79/80 bytes wire name length 78/79/79 плюс NUL;
полный локальный input сохраняет 78/79/80. Это assertions actual production
output из `tests/sv/graphics.c`, не новый serializer или источник игрового поведения.
Полный source-defined version/failure union остаётся pending по таблице ниже.

## HTML reference и ограничения

Read-only reference: `/home/svechnik/Projects/github_site/tomenet_interface`,
revision `1bb2d5d45b9001bf2a6f2084f78184c29a73fca7`.
`js/features/map.js:130–136` задаёт hover и его suppression при targeting;
`css/windows.css:325–354` — target border/fill/crosshair/contrast roles.
SHA256 map.js: `63cc0a7fbe39a50dd31ab160f5dc81762bfa9cea8285182b4d6ab0d4db2684e5`;
windows.css: `a3aa32d7a18e681fc607fc7596548ed8710abdab515c079091335d7a574d1ce8`.
Native следует этим состояниям/цветам/scale roles; pixel equality не требуется.
Pointer themes не подменяют cell indicators, отклонённые A/B/C не используются.
Была сверка исходников и просмотр native capture; side-by-side browser review,
полная contrast/scale human acceptance и неподтверждённый отдельный self-cursor
остаются pending.

Полный `Receive_char/Receive_line_info` и live transport/handoff остаются C013/
C014 (preparation C011). Прямой production semantic API и F6 projection **не
доказывают** позднее packet-derived `decode→state→surface`. Production weather/
palette/screenflash decoder и effects core существуют, но реальный server stream
пока не подключён к native live surface. C016 triggers/lamp-fainting и C055 audio
playback не принимаются по APIs/one-shot outputs. PRF effects сохраняют context/
payload/generation/ownership; actual delivery C016/C053 не hard gate по принятому
addendum и не объявляется выполненной. C044/C056 сохраняют полный Settings UX и
удаление временного F6 экрана. TTF high/control IDs требуют explicit profile C006.

Проверялся Linux software backend. Hardware/accelerated path, полный corpus
семантических combinations, все old animated-colour version branches, полный
graphics-weather pseudo-object draw/mask union, cloud_count<-1/negative-index/
long fractional motion и sound queue overflow/retry отдельно не исполнены.
Исходные условия сохранены, неподтверждённые branches не получают PASS.
Windows/MinGW/Wine deferred. Submission budgets 20/50/200 ms сохраняются, но
здесь не измерены: functional smoke не является latency evidence. Новые stress/
soak/общий memory ceiling и широкая matrix не вводились.

## Disposition 45 obligations

`core observed` означает ограниченное production-core evidence из таблицы выше;
это не canonical runtime PASS. Для map allocation-checks поздний настоящий
packet-derived C013 consumer остаётся pending. Полная приёмка C007 и отдельная
core-приёмка возможны только явным решением пользователя.

| Obligation | Disposition |
|---|---|
| `obligation.rendering.glyph-identity.result` | core observed; map-renderer: numeric/default/visible fallback; full game/profile corpus и C013 caller pending |
| `obligation.rendering.glyph-identity.failure-lifecycle` | core observed; map-renderer: numeric/default/visible fallback; full game/profile corpus и C013 caller pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.glyph-identity.allocation-checks` | core observed; map-renderer: numeric/default/visible fallback; full game/profile corpus и C013 caller pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.terrain-stack.result` | core observed; map-renderer: latest terrain под foreground/overlay и removal; packet-derived C013 pending |
| `obligation.rendering.terrain-stack.failure-lifecycle` | core observed; map-renderer: latest terrain под foreground/overlay и removal; packet-derived C013 pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.terrain-stack.allocation-checks` | core observed; map-renderer: latest terrain под foreground/overlay и removal; packet-derived C013 pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.tiles.result` | core observed; graphics + editor: prepared single/two-mask fixture, actual active draw/report; полный branch/backend union pending |
| `obligation.rendering.tiles.failure-lifecycle` | core observed; graphics + editor: prepared single/two-mask fixture, actual active draw/report; полный branch/backend union pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.tiles.allocation-checks` | core observed; graphics + editor: prepared single/two-mask fixture, actual active draw/report; полный branch/backend union pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.subsets.result` | core observed; graphics: base/subset precedence/shared glyph identity/reset; полный mixed/absent-subset union pending |
| `obligation.rendering.subsets.failure-lifecycle` | core observed; graphics: base/subset precedence/shared glyph identity/reset; полный mixed/absent-subset union pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.subsets.allocation-checks` | core observed; graphics: base/subset precedence/shared glyph identity/reset; полный mixed/absent-subset union pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.palette.result` | core observed; map-effects: base/custom/server palette, tint/monochrome/lighterDarkBlue; live stream pending |
| `obligation.rendering.palette.failure-lifecycle` | core observed; map-effects: base/custom/server palette, tint/monochrome/lighterDarkBlue; live stream pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.palette.allocation-checks` | core observed; map-effects: base/custom/server palette, tint/monochrome/lighterDarkBlue; live stream pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.animation.result` | core observed; map-effects: controlled clock/RNG и scopes; все old-version/category branches и late surfaces pending |
| `obligation.rendering.animation.failure-lifecycle` | core observed; map-effects: controlled clock/RNG и scopes; все old-version/category branches и late surfaces pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.animation.allocation-checks` | core observed; map-effects: controlled clock/RNG и scopes; все old-version/category branches и late surfaces pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.lighting.result` | core observed; map-effects: TERM_SRCLITE full44/lamp policy core; actual torch/wall/floor packet-derived caller pending C013 |
| `obligation.rendering.lighting.failure-lifecycle` | core observed; map-effects: TERM_SRCLITE full44/lamp policy core; actual torch/wall/floor packet-derived caller pending C013; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.lighting.allocation-checks` | core observed; map-effects: TERM_SRCLITE full44/lamp policy core; actual torch/wall/floor packet-derived caller pending C013; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.cursor.result` | core observed; map-renderer + editor: hover/target geometry/remove; self design и полная HTML/native visual acceptance pending |
| `obligation.rendering.cursor.failure-lifecycle` | core observed; map-renderer + editor: hover/target geometry/remove; self design и полная HTML/native visual acceptance pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.cursor.allocation-checks` | core observed; map-renderer + editor: hover/target geometry/remove; self design и полная HTML/native visual acceptance pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.resize.result` | core observed; map-renderer: normal/big/fractional DPI/hit/recreation; real OS display move/accelerated/budgets pending |
| `obligation.rendering.resize.failure-lifecycle` | core observed; map-renderer: normal/big/fractional DPI/hit/recreation; real OS display move/accelerated/budgets pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.resize.allocation-checks` | core observed; map-renderer: normal/big/fractional DPI/hit/recreation; real OS display move/accelerated/budgets pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.cache-generation.result` | core observed; graphics: stable source/profile/policy/final size, failure/reprepare/reset; full concurrent/accelerated/budget evidence pending |
| `obligation.rendering.cache-generation.failure-lifecycle` | core observed; graphics: stable source/profile/policy/final size, failure/reprepare/reset; full concurrent/accelerated/budget evidence pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.cache-generation.allocation-checks` | core observed; graphics: stable source/profile/policy/final size, failure/reprepare/reset; full concurrent/accelerated/budget evidence pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.effects-restore.result` | core observed; map-renderer + map-effects: current semantic layers/expiry/overlay modes; late live caller pending |
| `obligation.rendering.effects-restore.failure-lifecycle` | core observed; map-renderer + map-effects: current semantic layers/expiry/overlay modes; late live caller pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.effects-restore.allocation-checks` | core observed; map-renderer + map-effects: current semantic layers/expiry/overlay modes; late live caller pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.raw-picture.result` | core observed; graphics: real prepared raw extent/draw и reset; special-canvas live caller/clipping union pending |
| `obligation.rendering.raw-picture.failure-lifecycle` | core observed; graphics: real prepared raw extent/draw и reset; special-canvas live caller/clipping union pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.weather.result` | core observed; map-effects: decoder/particles/overlay/flash/audio intents; actual graphics weather union/live backend/audio playback pending |
| `obligation.rendering.weather.failure-lifecycle` | core observed; map-effects: decoder/particles/overlay/flash/audio intents; actual graphics weather union/live backend/audio playback pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.weather.wire` | core observed; map-effects: partial-prefix/cloud, chained и colouring gate fixtures; полный wire/version/live-stream union pending |
| `obligation.rendering.weather.allocation-checks` | core observed; map-effects: decoder/particles/overlay/flash/audio intents; actual graphics weather union/live backend/audio playback pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.wipe.result` | core observed; map-renderer: bounded rectangle/indicator clear/repaint; live C013 caller pending |
| `obligation.rendering.wipe.failure-lifecycle` | core observed; map-renderer: bounded rectangle/indicator clear/repaint; live C013 caller pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.wipe.allocation-checks` | core observed; map-renderer: bounded rectangle/indicator clear/repaint; live C013 caller pending; Linux accelerated/submission budgets и полный allocation flow pending |
| `obligation.rendering.solid-wall.result` | core observed; map-renderer: solid drawing role/current-state restore; all graphics equal-colour/live branches pending |
| `obligation.rendering.solid-wall.failure-lifecycle` | core observed; map-renderer: solid drawing role/current-state restore; all graphics equal-colour/live branches pending; core failure/reset/generation cases ограничены описанными fixtures, весь fault union не заявлен |
| `obligation.rendering.solid-wall.allocation-checks` | core observed; map-renderer: solid drawing role/current-state restore; all graphics equal-colour/live branches pending; Linux accelerated/submission budgets и полный allocation flow pending |

## Возврат интеграционного evidence и граф

FI-02 active resource consumer теперь существует; core commands/callers, actual
owned paths/IDs/generations и wire boundaries трассируются в C003, graphics PRF —
в C004. Это дополнение не переоткрывает их принятую пользователем приёмку и не
меняет canonical allocation. Непроверенные late branches остаются pending.

Существующий code graph обновлён `.graphify-venv/bin/graphify update .`;
[лог](../.scratch/sv-c007/graphify-update.log). CLI обновляет AST/code graph;
semantic extraction изменённых Markdown этим запуском не выполнялась. Игнорируемые
`graphify-out/` artifacts не являются runtime evidence. Source fingerprints:
[source-sha256.json](../.scratch/sv-c007/source-sha256.json).


## Приёмка пользователем — 2026-11-09

Пользователь явно подтвердил: «этот этап принял».
[C007 и задачи 001–007 приняты](sv-stage-c007-acceptance.md) в представленных границах Linux core,
с сохранением описанных ограничений. Исторические формулировки pending выше
относятся к моменту подготовки плана/evidence. Live карта остаётся C013;
непроведённые проверки не получают PASS, canonical registry не изменён.

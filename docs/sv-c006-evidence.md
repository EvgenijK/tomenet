# C006 — Linux core и software resource consumer

Статус: accepted пользователем 2026-11-09; [решение и известные ограничения](sv-stage-c006-acceptance.md).
Ниже сохранён технический отчёт на момент проверок; исторические формулировки
о pending приёмке заменены последующим решением пользователя. Registry не изменён.

## Исправление привязки preview к роли — 2026-11-09

Сообщение пользователя: образец не менялся после выбора другого шрифта.
Воспроизведено через existing `tests/sv/font-editor.c`: active Text font менялся,
map font оставался прежним, но hash actual rendered preview region оставался тем
же; assertion `preview_pixels(renderer)!=text_before` падал до исправления.
Это сравнение до/после одного события, не pixel-perfect oracle.

Причина: preparation, geometry и generation cache всегда использовали Map role,
даже когда editor показывал выбор Text. Теперь glyph preview использует выбранную
роль, cache учитывает role и generation, заголовок различает Text/Map; PRF cells и
tile/raw preview остаются только в Map. Text selection не меняет map definitions.
Existing focused ASan/UBSan test проверяет изменение actual pixels для обеих ролей,
неизменность другой роли и восстановление Text preview после Cancel.
[Лог](../.scratch/sv-c006/font-editor-role-fix.log),
[сборка](../.scratch/sv-c006/build-role-fix.log). Пользовательский процесс,
запущенный до пересборки, необходимо перезапустить для применения исправления.

2026-11-09. База: `2eb1c652103b56c4c75b9863c50ad432f438359c`, рабочее дерево
с незакоммиченными изменениями планирования и реализации C006. Existing unrelated
files сохранены. [Spec](sv-stage-c006-spec.md), [задачи](tasks/c006/README.md),
[exact planning snapshot](tasks/c006/coverage.json). Canonical allocation,
conditions, obligations, registry statuses и пользовательская приёмка не менялись.

## Что реально подключено

- `ui/font.[ch]`: FreeType PCF encoded IDs/charmap/metrics/default; numeric glyph
  API, native origin/partial range, solid cell ID 2 и visible one-cell replacement.
  PCF default metadata читается из encoding header, lookup/raster выполняет FT.
  TTF rasterize через SDL_ttf непосредственно к конечной клетке, proportional
  placement; profiles `sv-pcf-encoded-v1`, `sv-ttf-ascii-v1` и явные `SVFONT1`
  ID→Unicode tables. Shell Unicode text API сохранён.
- `ui/font-selection.[ch]` и `settings/config-save.c`: независимые роли,
  requested/effective, startup fallback и failed live replacement с прежним font;
  preview/Cancel/private explicit Save. Save перечитывает destination, сохраняет
  unknown/unrelated records, external-wins конфликт и прежний файл при failure;
  failure оставляет active dirty preview. Disk baseline отделён от CLI request.
- `settings/map-font.[ch]`, `preferences.c`: bootstrap resource hook до own OPT,
  reset R/K/F/U/modifier/subset и graphics resource definitions с сохранением
  options/macros/current A/records/effects; custom U→B, затем system `font-sv.prf`.
  PCF basename нормализован без `.pcf` для shipped custom PRF names. Отсутствие
  system source явно означает server-authoritative zero overrides; invalid/IO
  source отвергает candidate. Ordinary valid-tail PRF semantics сохраняются.
- `protocol/visuals.h`, `protocol/login.c`, `login/flow.c`: actual effective map
  identity и definitions в PLAY; character preferences применяются до финальных
  definitions. Live setup+font append атомарен. PLAY и live setup сохраняют свои
  различные version gates/table sizes; отсутствие override не превращается в
  glyph 0. Reporting сохраняет existing 79-byte copy policy, full refs локальны.
- `ui/resource-preparation.[ch]`: PNG/BMP decode, masks до filtering, authored
  Linear/Nearest, nearest coverage/outline, isolated PCF filter, final-size
  generated outlines внутри bounds, raw rectangles, generation и runtime tint.
  CPU PixelArt явно сообщает effective Nearest, сохраняя request.
- `ui/font-editor.[ch]` и `login/native-run.c`: настоящий production software
  consumer, без terminal/test-only renderer. На server selection F6 открывает
  resource editor; F7 роль, Tab поле font/profile, Enter preview, Ctrl+S Save,
  Esc Cancel, F8 PCF filter, F9 graphics filter, F10 outline -1…6. Numeric grid
  и PRF-defined cells используют active map font, attrs и фильтр; tile/raw preview
  использует selected shared sheet и final-size preparation. При missing/invalid
  startup sheet выводятся font-only resource map и диагностика, requested
  tiles/enabled не переписываются. Полная игровая карта здесь не заявляется.

Native Cancel восстанавливает сохранённый opening map stack, не перечитывая
исчезнувший PRF. Text-only операции не меняют map tables/report. Repaint/reset
не перечитывают PRF; device reset очищает editor textures и shell resources.
App resources живут отдельно от session generation. PRF load effects сохраняют
payload/context/generation и не объявляются доставленными messages/actions.

## Проверки и воспроизведение

Сборка: `make -C src -f makefile.sv -j4 tomenet-sv` — PASS с `-Wall -Wextra -Werror`.
[Лог](../.scratch/sv-c006/build.log). Все новые production objects SV-local;
legacy backends/common sources не менялись.

Пять минимальных checks вызывают production API, compiled clang C99 с
`-Wall -Wextra -Werror -g -fsanitize=address,undefined -fno-omit-frame-pointer`.
Точные argv всех пяти компиляций — [core-commands.json](../.scratch/sv-c006/core-commands.json).
Запуск binaries из `.scratch/sv-c006/` с `ASAN_OPTIONS=detect_leaks=0`;
для editor — `SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software`.
LeakSanitizer отключён из-за ограничения ptrace среды, leak coverage не заявляется.

| Production check | Фактический результат |
|---|---|
| [font-core](../tests/sv/font-core.c) | PASS: native origins/default IDs/presence, encoded ID vs FT slot, partial range, solid/replacement, TTF proportional и explicit high/control profile, invalid scalar rejection; 51/51 shipped PCF/TTF |
| [font-selection](../tests/sv/font-selection.c) | PASS: role independence, fallback, preview/cancel, failed live load, Save/reload, merge unrelated edit, same-key conflict и failed publication с active dirty |
| [resource-preparation](../tests/sv/resource-preparation.c) | PASS: mask roles, filter distinction, PixelArt fallback, bounds/no atlas bleed, retained generation при failure, BMP/PNG; actual graphics inventory 6/6 |
| [map-font](../tests/sv/map-font.c) | PASS: reset без потери S/macros/A/options, custom U/B/system, invalid directory rejection, missing system explicit policy, generation/effects, actual PLAY/live setup/font bytes и version lengths |
| [font-editor](../tests/sv/font-editor.c) | PASS: реальные events и SDL software draw, text/map preview/Cancel, filters/Save/reread, device reset закрытого/открытого editor и reopen |

Логи каждого check лежат в `.scratch/sv-c006/<name>.log`.
Дополнительно existing `tests/sv/login.c` compiled UBSan с production login/options/
preferences/files — PASS, чтобы сохранить default wire behavior после замены stub.
Двухкадровый native startup с isolated CFG, local server list и отключённым audio
прошёл, без server contact: [native.log](../.scratch/sv-c006/native.log).
Software editor [кадр](../.scratch/sv-c006/font-editor.png) осмотрен визуально;
это QA, а не pixel oracle или утверждённый полный Settings UX.

## Корпус и условия

Fonts: 50 PCF + Cascadia TTF = **51 valid, 0 invalid**; 50 FON — Windows deferred.
Representative результаты: `12x24`/`8x16` origin 1/default 32;
`16x22` range 0…127/default 0; `9x15tg`/`16x24tg`/`16x24x` full byte range/default 31.
Builtin Cascadia identity/license сохранены; custom proportional Adwaita и explicit
profile tested через тот же production loader. Игровые glyph IDs не выводятся из
UTF-8/metadata. Builtin TTF profile намеренно ASCII-only; произвольная high/control
correspondence требует явного profile. Полная bitmap-to-Unicode parity и покрытие
неизвестных пользовательских profiles массовыми replacements не заявляются.

Graphics: **6/6 BMP sheets**, включая 2 subsets; geometry/grid, base/subset PRF mask
associations, tile preparation и raw rectangle каждого sheet прошли; missing/partial
PRF = 0. PNG decoder дополнительно проверен на bundled login PNG.

Эта проверка выполнялась на Linux software SDL renderer. Hardware/accelerated
renderer, memory/performance matrix и реальные аккаунты не запускались; нет PASS
для непроверенных backend conditions. Windows/MinGW/Wine отложены по правилу
платформ и текущую Linux проверку не блокируют. Общий набор не расширялся matrix
или новым harness. Shipped options.prf содержит два baseline N records с parsing
diagnostics; они сохранены, предложение исправления отдельно в sv-improvements.

## Disposition 34 canonical obligations

Ниже PASS означает перечисленное Linux core/software evidence, не canonical
registration или blanket PASS всех платформ/backends/будущих consumers.
Для каждого ID сохранены result, failure-lifecycle, allocation-checks; у map-select
также wire. Полные IDs перечислены без объединения obligations.

| Obligation | Linux core/software evidence |
|---|---|
| `obligation.fonts.text-select.result` | PASS core: font-selection + font-editor: role preview/Cancel/private Save, unrelated map preserved |
| `obligation.fonts.text-select.failure-lifecycle` | PASS core: font-selection + font-editor: role preview/Cancel/private Save, unrelated map preserved |
| `obligation.fonts.text-select.allocation-checks` | PASS core: font-selection + font-editor: role preview/Cancel/private Save, unrelated map preserved; source/platform/backend limits above remain explicit |
| `obligation.fonts.map-select.result` | PASS core: map-font + font-editor + native bootstrap: PRF definitions, effective identity, PLAY/setup/font |
| `obligation.fonts.map-select.failure-lifecycle` | PASS core: map-font + font-editor + native bootstrap: PRF definitions, effective identity, PLAY/setup/font |
| `obligation.fonts.map-select.wire` | PASS core: map-font: exact actual PLAY tables, CLIENT_SETUP then FONT; distinct old/new gates and packet widths |
| `obligation.fonts.map-select.allocation-checks` | PASS core: map-font + font-editor + native bootstrap: PRF definitions, effective identity, PLAY/setup/font; source/platform/backend limits above remain explicit |
| `obligation.fonts.pcf.result` | PASS core: font-core + font-editor: encoded/default/origin/range/final glyph output |
| `obligation.fonts.pcf.failure-lifecycle` | PASS core: font-core + font-editor: encoded/default/origin/range/final glyph output |
| `obligation.fonts.pcf.allocation-checks` | PASS core: font-core + font-editor: encoded/default/origin/range/final glyph output; source/platform/backend limits above remain explicit |
| `obligation.fonts.ttf.result` | PASS core: font-core + font-editor: Cascadia, proportional custom face, explicit profile/scalar validation |
| `obligation.fonts.ttf.failure-lifecycle` | PASS core: font-core + font-editor: Cascadia, proportional custom face, explicit profile/scalar validation |
| `obligation.fonts.ttf.allocation-checks` | PASS core: font-core + font-editor: Cascadia, proportional custom face, explicit profile/scalar validation; source/platform/backend limits above remain explicit |
| `obligation.fonts.fallback.result` | PASS core: font-selection: declared text/map fallback, original request retained |
| `obligation.fonts.fallback.failure-lifecycle` | PASS core: font-selection: declared text/map fallback, original request retained |
| `obligation.fonts.fallback.allocation-checks` | PASS core: font-selection: declared text/map fallback, original request retained; source/platform/backend limits above remain explicit |
| `obligation.fonts.live-failure.result` | PASS core: font-selection + map-font: failed candidate/invalid PRF retains active stack |
| `obligation.fonts.live-failure.failure-lifecycle` | PASS core: font-selection + map-font: failed candidate/invalid PRF retains active stack |
| `obligation.fonts.live-failure.allocation-checks` | PASS core: font-selection + map-font: failed candidate/invalid PRF retains active stack; source/platform/backend limits above remain explicit |
| `obligation.fonts.tile-fallback.result` | PASS core: font-editor + preparation: missing/invalid resource result, font-only output and retained request |
| `obligation.fonts.tile-fallback.failure-lifecycle` | PASS core: font-editor + preparation: missing/invalid resource result, font-only output and retained request |
| `obligation.fonts.tile-fallback.allocation-checks` | PASS core: font-editor + preparation: missing/invalid resource result, font-only output and retained request; source/platform/backend limits above remain explicit |
| `obligation.fonts.graphics-filter.result` | PASS core: resource-preparation + font-editor: authored layers, nearest masks, explicit PixelArt fallback |
| `obligation.fonts.graphics-filter.failure-lifecycle` | PASS core: resource-preparation + font-editor: authored layers, nearest masks, explicit PixelArt fallback |
| `obligation.fonts.graphics-filter.allocation-checks` | PASS core: resource-preparation + font-editor: authored layers, nearest masks, explicit PixelArt fallback; source/platform/backend limits above remain explicit |
| `obligation.fonts.pcf-filter.result` | PASS core: resource-preparation + font-editor: independent coverage selector/no bleed |
| `obligation.fonts.pcf-filter.failure-lifecycle` | PASS core: resource-preparation + font-editor: independent coverage selector/no bleed |
| `obligation.fonts.pcf-filter.allocation-checks` | PASS core: resource-preparation + font-editor: independent coverage selector/no bleed; source/platform/backend limits above remain explicit |
| `obligation.fonts.outline.result` | PASS core: resource-preparation + font-editor: final-pixel radius/individual bounds |
| `obligation.fonts.outline.failure-lifecycle` | PASS core: resource-preparation + font-editor: final-pixel radius/individual bounds |
| `obligation.fonts.outline.allocation-checks` | PASS core: resource-preparation + font-editor: final-pixel radius/individual bounds; source/platform/backend limits above remain explicit |
| `obligation.fonts.asset-validation.result` | PASS core: font-core + resource-preparation: 51 supported fonts, 6 sheets, base/subset mask associations |
| `obligation.fonts.asset-validation.failure-lifecycle` | PASS core: font-core + resource-preparation: 51 supported fonts, 6 sheets, base/subset mask associations |
| `obligation.fonts.asset-validation.allocation-checks` | PASS core: font-core + resource-preparation: 51 supported fonts, 6 sheets, base/subset mask associations; source/platform/backend limits above remain explicit |

## Поздние flows и итог

C007 завершает full terrain/foreground/effects composition, actual graphics
FI-02 reporting и coupled graphics PRF overrides/reload. C011/C013/C014 принимают
session preparation, packet-derived game map и live-session handoff. Native
resource preview не доказывает эти flows. C016/C053 выполняют actual PRF
message/action delivery; C044/C056 — полный Settings/configuration UI. Их отсутствие
не становится обратным core gate, но и не получает PASS из текущих проверок.

Реализация C006 Linux core и технический отчёт подготовлены. Полная пользовательская
приёмка, непроверенные backend/profile conditions и canonical runtime registration
не объявляются завершёнными. Planning snapshot остаётся точной исторической копией
11 rows/34 obligations; canonical files не менялись. Существующий code graph
обновлён командой `.graphify-venv/bin/graphify update .`;
[лог](../.scratch/sv-c006/graphify-update.log) отдельно от runtime evidence.

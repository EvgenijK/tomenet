# C006 — проверенные источники и границы реализации

Снимок baseline/current SV **до реализации** C006, 2026-11-09.
Текущее состояние — в [Linux evidence](../../sv-c006-evidence.md); таблица
пробелов ниже сохранена как source trace планирования.

Дата сверки: 2026-11-09. Статус: planning; новых runtime PASS нет.
Анализ не запускал сборки и тесты. Graphify query
`font PCF TTF profiles filter outline fallback SV` выполнен до поиска исходников;
граф дал навигацию, факты ниже сверены в workspace. Координаты строк относятся
к этой сверке, при реализации их следует пересмотреть.

Нормативны [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md) и
[0006](../../adr/0006-errors-and-overload.md).
Окончательные policy и AGENTS.md выше исторических предложений inventories:
независимые text/map roles, FreeType PCF, отдельный PCF filter с default Nearest,
final-size preparation и явный requested/effective fallback уже приняты.
Повторно выбирать их или вводить pixel-perfect acceptance не требуется.

## Baseline и принятая адаптация

| Источник | Проверенный контракт для SV |
|---|---|
| `src/client/main-sdl3.c:505–605`, `Infofnt_init_ttf` | User font перед bundled; TTF metrics и monospace emulation для proportional fonts. SV rasterize в конечных метриках; legacy raw-byte UTF-8 вызов не переносить как glyph correspondence |
| `main-sdl3.c:627`, `Infofnt_init_pcf`; `5428`, `PCF_OpenFont`; `5860`, `PCF_RenderText` | PCF остаётся bitmap glyph resource. Legacy parser не годится для прямого переноса: policy требует корректный encoded-ID/origin/range/default lookup через FreeType |
| `main-sdl3.c:4701`, `term_force_font` | Новый ресурс загружается до замены; ошибка сохраняет старый. Успешная смена main font вызывает reload даже при прежних метриках |
| `src/client/c-util.c:20224`, `handle_process_font_file` | Reset R/K/F/U, modifier dictionaries и glyph subsets → font PRF при offset 0 → graphics PRF → client setup/font notifications → repaint |
| `settings-policy.md:99–111`, `355–361` | Resource/profile failure отличается от missing glyph. Startup: text → bundled Cascadia/profile → bundled 16x24x/profile; map → bundled 16x24x/profile; полный отказ font прекращает startup. Missing glyph: корректный font default, затем видимая one-cell замена с сохранением исходного ID |
| `settings-policy.md:268–288` | Authored tiles/raw pictures используют graphics filter; PCF — независимый filter; encoded masks сначала decode, recolour coverage/authored outline Nearest; generated outline строится внутри tile bounds в конечном размере. PixelArt требует фактической поддержки preparation/backend, иначе явный Nearest |

Подробные factual inventories:
[font/map lifecycle](../../capabilities/inventories/font-map-and-tiles-contract.md),
[PCF options](../../capabilities/inventories/pcf-loading-and-scaling-options.md),
[assets](../../capabilities/inventories/encoding-font-assets.md),
[renderer](../../capabilities/inventories/renderer-parity.md).
Исторические Windows requirements применяются с текущим
[Linux rule](../../agents/platform-validation.md).

## Текущий production SV и реальные пробелы

| Код | Уже есть | Что требуется C006 |
|---|---|---|
| `src/client/sv/ui/font.c:18–59`, `font.h` | Shell TTF/FreeType PCF loader; проверка ASCII, fixed-width TTF; готовый API для shell text | Numeric game-glyph API, explicit profiles, proportional TTF one-cell placement, PCF default/presence/metrics contract; не выдавать ASCII adapter за полную реализацию |
| `ui/font.c:61–105` | Startup text requested → U/B → declared bundled fallbacks; effective path отдельно | Profile validation и role-specific lifecycle; resolver C003 должен различать missing/invalid/IO. Текущий ручной перебор не является основанием для второго файлового алгоритма |
| `ui/font.c:132–225` | TTF Unicode text rendering; PCF shell ASCII projection; nearest raster; measure | PCF non-ASCII сейчас заменяется `?`; отсутствуют numeric map-ID dispatch, explicit default chain, final-size glyph cache и selectable PCF filtering |
| `lib/xtra/font/sv-shell-profiles.json` | Явный Stage A ASCII shell profile; Cascadia v2407.24 hash/license и 16x24x ASCII fallback | Не содержит полного game/map или field-specific byte correspondence. Нужны проверенные resource/role profiles; unknown mapping не угадывать |
| `settings/config.c:30–34`, `config.h:33–36` | Раздельные text/map references, profiles и filters, graphics/outline settings; load/parser без Save API | Реальные consumers и узкий private explicit Save в 004 поверх C002/C003. Общая Settings surface и configuration registry остаются C044/C056 |
| `files/files.h:8–20`, `files.c:42–53` | `SvFilePath.owner`, resolver U-before-B, missing/invalid/IO, owner-aware IO | Использовать в 001/003/004; не дублировать resolver |
| `settings/preferences.h:95–130` | Production PRF owner, clone, mappings, resource bootstrap hook, retained effects | 002 добавляет необходимый visual reset и реальный font PRF consumer, не второй parser |
| `login/native-run.c:528–534`, `603–620` | Text font загружается; map-resource bootstrap hook NULL | 001/002 должны создать настоящий font-based map consumer, независимый от text font |
| `protocol/login.h:34–46`, `protocol/resource-report.h:46–47` | Current missing map identity и `NO_GRAPHICS`, один wire serializer | 001 подключает actual map identity. UI text font и requested map name не подменяют реально активный resource |
| `src/makefile.sv:30`, `44`, `62` | SDL3, SDL_ttf, FreeType уже native dependencies; `ui/font.o` в production сборке | Новые SV-local core objects подключать минимально; legacy backend не затягивать |

В SV не найден действующий map raster/tiles/raw-picture consumer, resource
profile registry или filter/outline preparation owner. Следовательно, один
успешный `sv_font_open_file` не доказывает map rendering, reload или C007 flow.

## Seams и зависимости

003 владеет PCF/TTF decoding, glyph/profile identity, metrics и final-size glyph
preparation. 004 управляет independent role selection, candidate validation,
startup/live error policy и requested/effective generations; 001 активирует map
consumer/reporting; 002 связывает его с PRF owner. Это обязанности, а не требование
создать по новому framework на каждый ticket. Пригодный `ui/font.c` допустимо
адаптировать или оставить shell wrapper над SV-local core.

005 определяет SV-local preparation/filter/outline contract, реальное production
отображение prepared tile/raw-picture layers и font-only fallback result.
Полная игровая композиция active tiles, graphics PRF overlays/subsets и
raw-picture lifecycle завершает C007 на предоставленном seam. Его поздние
consumer evidence не выдаются за PASS C006 и не становятся обратным prerequisite
приёмки C006. Собственные resource outcomes 005 доказываются уже в C006 через
production consumer, пригодный для последующего C007. 006 инвентаризирует shipped assets/profiles и выполняет basic
validation через этот же core; 007 объединяет уже необходимые Linux checks.
Не добавлять отдельный test-only loader, новый общий harness или полную matrix.

Все новые изменения по возможности в SV. Возможные улучшения legacy и общей
организации заносятся отдельно в `docs/sv-improvements.md`; намеренная изоляция
и дублирование сами по себе дефектом не являются. Canonical IDs, allocation,
obligations и runtime statuses данным документом не меняются.

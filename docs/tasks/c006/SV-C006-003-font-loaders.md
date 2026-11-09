# SV-C006-003 — PCF/TTF core, profiles и numeric glyph rendering

Статус: accepted, 2026-11-09; принят пользователем в согласованных границах.
Labels: enhancement, ready-for-agent
ID owner: `capability.fonts.pcf`, `capability.fonts.ttf`.
Зависимости: production file owner C003; при интеграции map —
[001](SV-C006-001-map-resource-integration.md) и
[002](SV-C006-002-map-preferences-integration.md).
Consumers: 004 role lifecycle, 005 preparation/filter, 006 asset validation.

## Результат и границы

Production SV умеет загружать bundled/custom PCF и TTF с совместимым explicit
resource profile, сохранять numeric game glyph identity и готовить glyph в
конечной клетке. Тот же core вызывают действующие text/map consumers и минимальные
проверки; отдельной реализации поведения для тестов нет.

Нормативны [source contract](source-contract.md),
[CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md) и
[0006](../../adr/0006-errors-and-overload.md).
Этот ticket — единственный ID owner двух fonts capabilities; 001/002/004/005/006
дают consumer/integration evidence без повторного allocation. Canonical allocation
и obligations не меняются.

## Работа

1. Построить минимальный SV-local font core на существующих FreeType/SDL_ttf
   dependencies. Сверить `ui/font.c` и сохранить пригодные shell callers; не
   включать `main-sdl3.c`, legacy Term globals или большой набор зависимостей.
   Пути разрешать через C003 `sv_file_resource`/`SvFilePath` и сохранять owner,
   effective path и source generation. Ошибочный overlay не приравнивать к
   отсутствующему. Никакого второго file resolver.
2. PCF открывать через FreeType с явным encoded-ID/charmap и native strike.
   Сохранять presence отдельно от fallback; учитывать nonzero origin, partial
   ranges, bearings/ascent/advance и bitmap pitch. Default encoded character
   разрешать через таблицу/charmap, а не принимать за glyph index; если default
   непригоден, использовать видимую one-cell replacement, сохраняя original ID.
   Не подменять успешный PCF иным размером/семейством ради эстетики.
3. TTF rasterize через SDL_ttf в окончательных метриках. Поддержать bundled
   Cascadia Mono exact v2407.24 с OFL/copyright и custom TTF, включая proportional
   font с явным one-cell placement/clipping. Текущая fixed-width-only проверка
   `ascii_ttf` недостаточна. Unicode own UI text подавать напрямую; legacy byte
   glyph сначала разрешать по проверенному ID→Unicode profile. Не передавать
   raw high/control bytes в UTF-8 text API и не выводить charset из metadata.
4. Определить минимальный production profile seam: optional explicit
   `svTextProfile`/`svMapProfile`, иначе зарегистрированный profile resource/role;
   printable ASCII identity допустима, остальные glyphs требуют доказанной
   correspondence. Missing/unsupported profile возвращает resource failure,
   а не guessed mapping. `sv-shell-profiles.json` — только shell ASCII profile,
   не доказательство полного map/game corpus. Full bundled profile coverage
   проверяется в 006; массовые replacements не доказывают parity.
5. Предоставить production glyph/metrics/preparation API, принимающий numeric ID
   и конечную integer cell geometry. Поддержать NUL/control/high IDs независимо
   от C-string termination; не смешивать encoded ID, Unicode scalar и glyph slot.
   Solid-wall procedural dispatch остаётся filled cell, tile IDs направляются
   graphics consumer. Prepare coverage изолированно по glyph/cell, затем выводить
   1:1; не делать постоянное масштабирование готового raster. Generation identity
   учитывает resource/profile и конечные размеры, а filter seam связывается с 005.
6. Возвращать смысловые результаты load/profile/missing-glyph/preparation failure
   и пригодный candidate. Startup fallback, live load-before-replace, независимость
   text/map и requested/effective publication принадлежат 004; map PRF и server
   reporting — 001/002. Не активировать частичный stack внутри loader и не
   расширять settings UI/Save C056.

## Минимальная проверка и завершение

Только необходимый Linux core production path; переиспользовать checks 001/002,
004/006 и итоговый evidence 007. Один компактный representative corpus покрывает
ordinary `9x15`, full-byte `9x15tg`, origins `12x24`/`8x16`, partial `16x22`,
selected `16x24tg` и fallback `16x24x`; не создавать отдельный matrix harness.
Проверить различие ID/slot, границы/presence/default/replacement и метрики;
для TTF — bundled identity/license, custom proportional one-cell output,
явный high/control profile mapping и profile failure. Проверки вызывают тот же
production API, который реально обслуживает consumers, а не копию алгоритма.

Широкое basic discovery всех shipped assets — 006; active tiles/mixed composition
и graphics lifecycle — C007. Здесь не доказывать отсутствующие consumers
искусственным snapshot и не требовать pixel-perfect comparison. Completion:
два allocated outcomes имеют production core и применимое consumer evidence,
источники/ограничения/failures зафиксированы в C006 evidence; непроверенное остаётся
pending. Новых runtime PASS этот план не заявляет.

Расширение набора проверок заранее согласуется по AGENTS.md.
[Обязательная платформа Linux](../../agents/platform-validation.md);
Windows/MinGW/Wine deferred и не блокируют текущую приёмку.

## Результат реализации — 2026-11-09

Production SV реализация и минимальные Linux core проверки выполнены.
[Фактическое evidence и ограничения](../../sv-c006-evidence.md) отделяют
software resource consumer от поздних gameplay flows и непроверенных backends.
Canonical runtime registration и пользовательская приёмка не выполнялись.

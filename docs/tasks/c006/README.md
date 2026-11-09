# C006 — шрифты текста и карты, подготовка ресурсов

Статус: accepted, 2026-11-09; принят пользователем в согласованных границах.
Labels: enhancement

Полный план подготовлен 2026-11-09 по запросу пользователя с анализом трёх
агентов. [Спецификация](../../sv-stage-c006-spec.md),
[source contract](source-contract.md), [exact planning snapshot](coverage.json).
Ровно 11 canonical font outcomes и 34 obligations. Реализация и Linux evidence
выполнены; границы и непроверенные условия — в [отчёте](../../sv-c006-evidence.md).
Прежняя компактная часть плана включена в задачи 001/002 без потери решений.
Пользователь утвердил 2026-11-09:
файловый core и существующие consumers реализуются в C003; реальные поздние
consumers завершают зависимые flow evidence без переноса реализации в C003.
Повторное согласование этой развилки не требуется.

| Задача | Обязательный результат | Единственный ID owner / зависимости |
|---|---|---|
| [SV-C006-001 — active map font](SV-C006-001-map-resource-integration.md) | Реальный font-based map, definitions, reporting и FI-01 | `fonts.map-select`; C003, 003/004 и 002 |
| [SV-C006-002 — map preferences](SV-C006-002-map-preferences-integration.md) | Resource hook, visual reset, font PRF и server reload | 0 IDs, supporting 001; C004 и map owner 001 |
| [SV-C006-003 — font loaders](SV-C006-003-font-loaders.md) | PCF/TTF glyphs, profiles и final-size raster | `fonts.pcf`, `fonts.ttf`; existing C003 file owner |
| [SV-C006-004 — selection/lifecycle](SV-C006-004-font-selection-lifecycle.md) | Независимые роли, preview/cancel/Save, startup/live policy | `fonts.text-select`, `fonts.fallback`, `fonts.live-failure`; C002/C003 и 003 |
| [SV-C006-005 — preparation](SV-C006-005-resource-preparation.md) | Filters/outline и startup tiles fallback с реальным production output | `fonts.tile-fallback`, `fonts.graphics-filter`, `fonts.pcf-filter`, `fonts.outline`; 003/004 и map path 001/002 |
| [SV-C006-006 — assets](SV-C006-006-asset-validation.md) | Discovery/validation shipped resources и особые glyph ranges | `fonts.asset-validation`; 003–005 для runtime validation |
| [SV-C006-007 — evidence](SV-C006-007-evidence.md) | Минимальный общий Linux отчёт по 11 IDs/34 obligations | 0 IDs; 001–006 |

001 входит в production map-font путь; отдельный параллельный reporting или
файловый алгоритм не вводить. Map-specific fallback/live checks 001 подтверждают
общий policy owner 004, не создавая второго ID owner. Allocation IDs остаётся в canonical
[native coverage](../../capabilities/native-coverage.json).

Порядок реализации: 003 и discovery 006 → общий lifecycle 004 и map path 001/002 →
preparation/consumer 005 → завершение validation 006 → evidence 007.
001/002 реализуются совместно: resource owner → reset/font PRF → server definitions
и reporting. Seam graphics overrides остаётся для C007. Active graphics lifecycle завершает
[C007](../c007/README.md); отсутствие этого consumer остаётся явным pending,
не закрывается выбранным font или сериализованным snapshot.

Только минимальные SV-local изменения; проверки — необходимый минимум Linux core
через production path. Расширение набора согласуется по AGENTS.md; Windows/MinGW/
Wine deferred. План не означает реализацию или приёмку C006; C003 принят пользователем.

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.

## Интеграция C004 — дополнение 2026-11-09

Пользователь принял и закрыл C004, передав remaining consumers и проверки
следующим этапам. C006 обязан выполнить
[SV-C006-002 — map preference stack](SV-C006-002-map-preferences-integration.md):
effective map font → custom/system PRF sources → visual reset → font mappings →
server definitions/reload настоящего font-based map consumer. Настоящий resource
hook использует production `SvPreferences`; text font не подменяет map font.

Задача 002 дополняет existing C003/FI-01 задачу 001; обе входят в полный
план C006. Порядок map работ: actual map-font owner 001 и source selection 002 →
production preference reset/apply → минимальные Linux font consumer/peer checks
и evidence. Переиспользовать checks между задачами. C006 предоставляет seam для
graphics; graphics-enabled overrides/reset/coupled reload и actual flow проверяет
C007 по своей задаче, без зависимости приёмки C006 от C007. C006 доказывает
production generation/ownership PRF effects; actual message/action delivery
остаётся C016/C053 и не блокирует C006.

Late flow requirements остаются обязательными в C006; найденные дефекты
исправляются здесь без автоматического переоткрытия C004. На момент этого дополнения C006 оставался planned,
его реализация/приёмка не заявлялись; текущий результат приведён ниже. Canonical IDs/conditions/obligations/allocation
и runtime statuses не изменяются; [приёмка C004](../../sv-stage-c004-acceptance.md)
и [source trace](../c004/source-contract.md) сохраняются отдельно от новых checks.

## Полнота C006 и границы соседних этапов

Подготовка фильтрованных tile/raw-picture layers и outlines в 005 должна иметь
реальный production rendering consumer. Startup tiles failure обязан дать
font-only map с диагностикой и сохранёнными requested tiles/enabled. Это собственные
C006 результаты; их нельзя закрыть enum, loader или отложить целиком до C007.
C007 завершает полную игровую композицию, FI-02 active graphics reporting и
coupled graphics PRF lifecycle. Полная C006 приёмка не ожидает C007, но требует
наблюдаемого результата всех своих 11 возможностей.

004 включает минимальный production выбор/preview/cancel/explicit Save fonts через
existing private CFG owner. Общие Settings/configuration UI и orchestration остаются
C044/C056. 003 не заменяет успешный PCF другим шрифтом ради эстетики; profile и
glyph-ID semantics не выводятся из Unicode UI или shell-only font registry.
Source contract фиксирует реальные пробелы существующего SV, а не готовность.

Только минимальные Linux core проверки через production API; обязательный bundled
corpus сохраняется, набор проверок не расширяется новым harness/matrix без
согласования. Platform conditions и exact obligations скопированы без изменений
в planning snapshot. Канонический runtime status и пользовательская приёмка этим
планированием не изменяются.

## Реализация — 2026-11-09

003/004/005 реализованы агентами; 001/002 подключены к native bootstrap,
font PRF и PLAY/setup/font reporting. Минимальный native resource editor открывается
на server-selection экране клавишей F6: F7 роль, Tab font/profile, Enter preview,
Ctrl+S Save, Esc Cancel; F8/F9 filters, F10 outline. Это узкий font/resource
consumer, а не полный Settings UI C044 или игровая карта C013.
Замена этого временного экрана продуктовым Settings UI и его удаление закреплены
за [SV-C044-002](../c044/SV-C044-002-product-font-settings.md); это обязательный
критерий завершения C044, а не последующая необязательная уборка.

Linux build, пять focused production core/consumer ASan/UBSan checks, existing
login parser и двухкадровый native software startup прошли. Корпус: 51/51 PCF/TTF,
6/6 BMP sheets и их base/subset PRF associations; 50 FON — Windows deferred.
[Evidence](../../sv-c006-evidence.md) сохраняет отдельную disposition 34 obligations.
Аппаратный renderer и произвольная TTF correspondence не доказаны. Полная
пользовательская приёмка и canonical runtime registration не заявляются.

## Приёмка — 2026-11-09

Пользователь принял C006 и задачи 001–007 после обсуждения известных ограничений.
[Решение о приёмке](../../sv-stage-c006-acceptance.md) имеет приоритет над
историческими формулировками pending выше. TTF ASCII profile и ручной ввод имени
в F6 зафиксированы явно; продуктовый выбор и замена временного UI остаются C044.
Canonical runtime registration остаётся отдельным workflow.

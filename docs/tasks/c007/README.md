# C007 — игровой rendering, graphics stack и эффекты

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Labels: enhancement

Полный план подготовлен 2026-11-09 по запросу пользователя с анализом трёх
агентов. Ровно **15 rendering outcomes и 45 obligations**.
[Спецификация](../../sv-stage-c007-spec.md), [source contract](source-contract.md),
[точный planning snapshot](coverage.json). Реализация и evidence pending.

Первоначальная обязательная интеграционная часть сохранена ниже. Решение пользователя
2026-11-09: C003 реализует file/reporting core; поздний настоящий renderer
завершает active graphics flow evidence. Повторное согласование не требуется.

| Задача | Обязательный результат | Зависимость реализации |
|---|---|---|
| [SV-C007-001 — active graphics](SV-C007-001-graphics-resource-integration.md) | Actual mode/tiles/font identity, overlay, activation/reinit и возврат FI-02 evidence в C003 | Production core SV-C003-001/005; реальные font owners C006 и rendering C007 |

Задача входит в production renderer/resource lifecycle полного плана C007.
Порядок: actual load/preparation → activation → publication effective state →
baseline server exchange → checks и возврат evidence. C006 отвечает за
[FI-01 map font](../c006/SV-C006-001-map-resource-integration.md); совместный
font/graphics snapshot не заменяет доказательство применения обоих consumers.

Все canonical IDs/obligations/allocations остаются в
[native coverage](../../capabilities/native-coverage.json); эта задача не
добавляет новый capability и не принимает C007. C003 уже принят пользователем. Только SV-local минимум
изменений, только необходимые Linux production-path core checks; расширение
набора согласуется заранее, Windows/MinGW/Wine deferred.

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.

## Интеграция C004 — дополнение 2026-11-09

Пользователь принял и закрыл C004; оставшиеся graphics preference consumers и
проверки теперь обязательны в C007. Добавлена самостоятельная
[SV-C007-002 — graphics preference stack](SV-C007-002-graphics-preferences-integration.md):
base/subset PRFs, offsets, shared glyph-subset identity, masks/raw pictures,
reset/reload, фактическое применение renderer и server-visible definitions.

Задача 002 дополняет existing C003/FI-02 задачу 001 и использует
[C006 map preference stage](../c006/SV-C006-002-map-preferences-integration.md).
Порядок: font base C006 → graphics source/preparation 001/002 → activation и
единый reporting → минимальные Linux consumer/peer checks. Production
`SvPreferences`/C003 owners сохраняются; второй parser или pixel oracle не вводить.
Graphics-enabled coupled overrides/reset/reload и actual renderer flow завершаются
именно здесь; font-based map acceptance C006 не ожидает C007. Для PRF effects
C007 доказывает generation/ownership и отсутствие replay, actual message/action
delivery принадлежит C016/C053 по своим задачам и не является hard gate C007.

Найденные при интеграции дефекты исправляются в C007 без автоматического
переоткрытия C004. C007 остаётся planned, runtime PASS и приёмка не заявлены.
Canonical allocation/IDs/conditions/obligations/runtime statuses неизменны;
новое evidence трассируется к [C004 source contract](../c004/source-contract.md)
и [приёмке](../../sv-stage-c004-acceptance.md), не отменяя её.


## Полная декомпозиция и единственные owners

| Задача | Результат | Owned `capability.rendering.*` |
|---|---|---|
| [001 — graphics resources](SV-C007-001-graphics-resource-integration.md) | Base/subset images, masks, prepared generations, activation и FI-02 | `tiles`, `subsets`, `cache-generation` |
| [002 — graphics preferences](SV-C007-002-graphics-preferences-integration.md) | Coupled font/graphics PRF reset/reload и server definitions | 0 IDs; поддерживает 001/003/006 |
| [003 — semantic composition](SV-C007-003-semantic-composition.md) | Semantic state, glyph/default roles, terrain/foreground, wipe и solid blocks | `glyph-identity`, `terrain-stack`, `wipe`, `solid-wall` |
| [004 — palette/lighting](SV-C007-004-palette-lighting-animation.md) | Цветовые роли, animation scopes и full-map освещение | `palette`, `animation`, `lighting` |
| [005 — geometry/indicators](SV-C007-005-geometry-indicators.md) | Normal/big fit, DPI, hit testing и cell indicators | `cursor`, `resize` |
| [006 — effects/weather](SV-C007-006-effects-weather-raw-picture.md) | Restore текущего state, raw pictures и weather protocol/model/renderer | `effects-restore`, `raw-picture`, `weather` |
| [007 — evidence](SV-C007-007-evidence.md) | Минимальный Linux отчёт по 15 IDs/45 obligations | 0 IDs |

Порядок: semantic foundation 003 и candidate resources 001/002 → общая geometry
005 и cache integration → 004/006 → завершение production wiring/reporting → 007.
001/002 готовят одну coupled transaction; это совместная инженерная итерация,
а не циклический hard prerequisite. Canonical prerequisites не меняются.

## Текущее production состояние и границы приёмки

C006 уже реализовал `sv_map_font_bootstrap`, font PRF/reset, definitions/reporting,
font/preparation APIs и native F6 resource preview. Прежние упоминания NULL hook
и disabled stub в исходных addenda — исторические anchors до C006; реализация
C007 начинается с текущих production owners, а не со второго bootstrap/parser.
F6 preview не является полным игровым renderer C007.

C007 вводит reusable production semantic rendering state/ingress и composition;
renderer не читает wire packets, legacy globals или `Term`. Weather decode
принадлежит 006 по собственной wire obligation. Полный map packet ingress
`world.read-map` и live-session handoff остаются C013/C014 (preparation C011).
Core scene через production API доказывает core результат, но не заменяет
canonical `decode→state→surface` packet-derived map evidence. Этот поздний flow
остаётся pending до реального C013 consumer; полная приёмка C007 по одному
core результату не подразумевается и потребует явного решения пользователя.

Дизайн indicators делегирован основному HTML UX prototype. Наблюдаемые hover и
throw-target anchors описаны в source contract; старые A/B/C отклонены. Повторный
выбор рисунка сейчас не нужен. Native реализация сверяет фактический reference,
а отсутствующий self/target variant и непроверенные contrast/scale состояния
фиксирует явно; без reference нельзя выдумывать approved UX.

C016/C053 исполняют PRF message/action delivery; C007 доказывает их generation,
ownership и отсутствие replay, без backward gate по уже принятому addendum.
C044/C056 сохраняют полный Settings/configuration UX. C003/C004/C006 приняты;
consumer defects исправляются внутри C007 без автоматического переоткрытия.

Только SV-local минимум и необходимые Linux production-core checks; набор
не расширяется новым harness/matrix без согласования. Canonical allocation,
conditions, obligations и runtime statuses неизменны. Этот полный план не
заявляет реализацию, runtime PASS или пользовательскую приёмку C007.


## Реализация Linux core — 2026-11-09

Агенты реализовали graphics/preference candidate 001/002, semantic compositor и
geometry 003/005, palette/animation/lighting и weather/effects/flash 004/006.
Root подключил bootstrap/reporting, F6 Map consumer, filter reprepare без PRF
replay и coherent preview/Save/Cancel/reset. Сборка, три focused production-core
ASan/UBSan checks, existing editor/login и bounded native event smoke прошли.
[Evidence по 45 obligations и ограничения](../../sv-c007-evidence.md).

Полный packet-derived map flow, hardware/submission evidence и полная visual
приёмка не заявлены. Поздние обязанности сохраняются; плановый snapshot не
переписан как runtime PASS, пользовательская приёмка и canonical registry не менялись.


## Приёмка пользователем — 2026-11-09

Пользователь явно подтвердил: «этот этап принял».
[C007 и задачи 001–007 приняты](../../sv-stage-c007-acceptance.md) в представленных границах Linux core,
с сохранением описанных ограничений. Исторические формулировки pending выше
относятся к моменту подготовки плана/evidence. Live карта остаётся C013;
непроведённые проверки не получают PASS, canonical registry не изменён.

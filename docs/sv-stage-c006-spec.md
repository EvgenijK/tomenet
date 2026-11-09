# C006 — шрифты текста и карты, подготовка ресурсов

Статус: implemented Linux core, 2026-11-09; приёмка pending. План подготовлен по запросу пользователя с анализом
тремя агентами: canonical contract, baseline/current SV и integration/lifecycle.
Реализация и software Linux evidence подготовлены;
[текущие результаты и ограничения](sv-c006-evidence.md).
[Задачи и порядок](tasks/c006/README.md),
[source contract](tasks/c006/source-contract.md),
[exact planning snapshot](tasks/c006/coverage.json).

## Результат этапа

C006 реализует независимые text/map font roles, PCF/TTF/profile загрузку,
startup fallback и безопасную live смену, подготовку ресурсов с выбранными
filters/outline и проверку shipped assets. Ровно **11 canonical outcomes и 34
obligations** сохраняют allocation, conditions, prerequisites и runtime statuses.
Для каждого ID есть result, failure-lifecycle, allocation-checks; map-select
дополнительно имеет wire obligation. У каждого ID один ticket owner:

| `capability.fonts.*` | Результат | Owner |
|---|---|---|
| `map-select` | Реальный map font/profile, visual definitions и server reporting | 001 |
| `pcf`, `ttf` | Glyph/profile loaders и rasterization в конечном размере | 003 |
| `text-select` | Независимый text font/profile, preview/cancel/explicit Save | 004 |
| `fallback`, `live-failure` | Startup policy, load-before-replace и сохранение рабочего stack при live failure | 004 |
| `tile-fallback` | Непригодный startup tileset → настоящий font-only map и сообщение | 005 |
| `graphics-filter`, `pcf-filter`, `outline` | Реальные final-size prepared assets и применение production consumer | 005 |
| `asset-validation` | Discovery/validation shipped fonts/tilesets/profiles и особые PCF ranges | 006 |

002 завершает C004 preference integration внутри map outcome 001; 007 собирает
evidence всех IDs. Они не получают второй allocation тех же возможностей.
Первоначальные 001/002 сохраняются вместе с историей решений о C003/C004.

## Нормативная основа и существующий код

Применяются [AGENTS](../AGENTS.md), [CONTEXT](../CONTEXT.md),
[settings policy](capabilities/settings-policy.md),
[архитектура](sv-architecture.md), ADR
[0002](adr/0002-main-thread-state-ownership.md),
[0003](adr/0003-explicit-module-connections.md),
[0004](adr/0004-revision-based-ui-views.md),
[0005](adr/0005-session-scoped-work.md),
[0006](adr/0006-errors-and-overload.md) и
[правило платформ](agents/platform-validation.md).
Reconciliation и inventories дают source trace; принятая policy и canonical
raster pipeline имеют приоритет над историческими предложениями pixel oracle.

`ui/font.c` уже рисует shell text, но не реализует полный PCF/TTF/profile контракт
C006. В частности, shell-only ASCII handling и fixed-width TTF gate не доказывают
игровые glyph IDs, proportional font contract или role profiles. Текущий
`sv-shell-profiles.json` не заменяет map/game profiles.
`SvPreferences` и file/reporting core существуют, но resource hook startup пока
NULL, а PLAY setup definitions берутся из `sv_stub_play_visuals`. C003 уже
передаёт missing map identity вместо text font; C006 должен подключить настоящий
effective map owner и definitions. Точные seams и baseline — в source contract.

## Обязательные правила поведения

1. **Две роли.** Text font обслуживает текстовые surfaces, map font — игровые
   glyphs и клеточную геометрию. Text-only change не перезагружает mappings и не
   отправляет map font notification. Unicode UI не меняет numeric game IDs или
   wire encoding. PCF сохраняет encoded-ID/charmap/default/metrics, включая
   nonzero origins/partial ranges. TTF сохраняет one-cell/proportional contract.
2. **Requested/effective.** Полные resource/profile references сохраняются в
   private CFG только при явном Save. Preview/cancel и failed change не создают
   ложного сохранения. U-before-B lookup использует C003 file owner; missing,
   unreadable/invalid resource и profile failure различаются. Отсутствующий
   profile не исправляется выдуманной универсальной charmap.
3. **Startup и live.** Для текста fallback — bundled Cascadia/profile, затем
   bundled `16x24x.pcf`/profile, с пропуском повторных шагов; для карты — bundled
   `16x24x.pcf`/profile. Исчерпание вариантов завершает запуск с resource error.
   Непригодный startup tileset даёт font-only map и сообщение, сохраняя requested
   tileset/enabled. Failed live font/profile/tiles change сохраняет прежний
   рабочий stack и видимую ошибку draft. Успешный PCF сохраняется независимо от
   эстетических предпочтений реализации.
4. **Map reload.** Effective font → visual reset → custom/system font PRF → seam
   для graphics overrides → client setup/font notification → repaint. Loader,
   parser и serializer существующих owners переиспользуются. Ordinary PRF
   partial/continuation rules сохраняются; пригодность resource candidate решает
   resource owner. Resource/mappings/effective identity публикуются согласованно.
5. **Подготовка.** Шрифты, masks и outlines готовятся к конечной geometry, а не
   растягиваются постоянно как целый framebuffer. Graphics filter действует на
   authored tiles/raw pictures; encoded masks декодируются до фильтрации,
   recolour coverage/authored outline используют Nearest. PCF filter независим,
   default Nearest; graphics default Linear. Unsupported PixelArt даёт явный
   effective Nearest с сохранением requested. Glyph/tile boundaries не смешиваются.
6. **Lifecycle.** App resources живут независимо от session. Подготовленный
   candidate публикуется после проверки; cache identity учитывает source/profile,
   filter/outline/final geometry и поколение. Repaint/recreation не перечитывают
   PRF и не исполняют load effects снова; stale session generation не отправляет
   notifications. C006 доказывает generation/payload/context/ownership effects;
   actual messages/actions delivery принадлежит C016/C053.

## Границы с соседними этапами

**C007:** C006 обязан получить собственный наблюдаемый production результат
graphics-filter/outline/tile-fallback. Реальная подготовка и отображение authored
tile/raw-picture layers через production consumer входят в 005; enum, snapshot
или тестовый renderer их не заменяют. Это минимальная ресурсная часть, пригодная
для дальнейшего C007. Полная игровая композиция terrain/foreground/effects,
active graphics identity/FI-02 и coupled graphics PRF reset/override/reload
остаются C007. Его отсутствие не становится обратным gate C006; собственные
непроверенные результаты C006 остаются pending и препятствуют его полному завершению.

**C044/C056:** минимальный production выбор/preview/cancel/Save fonts и применение
их resource keys обязательны здесь. Общая Settings surface, orchestration всех
drafts/conflicts и весь configuration registry принимаются в своих этапах.
Узкая font операция использует existing C002 model/parser и C003 publication;
отсутствующий Save seam добавляется в рамках 004, новый CFG owner не нужен.
Собственный font Save уже соблюдает reread/merge changed records, external-wins
с диагностикой при конфликте и active+dirty preview при неудачной записи.

**C003/C004:** они приняты пользователем. Consumer defects и узкие недостающие
seams исправляются здесь без автоматического переоткрытия прежней приёмки.
FI-01 и preference obligations трассируются обратно, без изменения их allocation.
**C011/C013/C014:** полный session preparation, map packet flow и live session
не принимаются по локальному font output; C006 использует реальный ресурсный
consumer и definitions, а не ждёт реализации этих будущих flows.

## Порядок и минимальная проверка

Начать с 003 и discovery части 006; затем общий lifecycle/selection 004 и
map integration 001/002. Подготовку 005 строить на том же resource/glyph owner,
не создавая параллельный pipeline. Закончить validation 006 и evidence 007.
Внутри реализации допустимы короткие совместные итерации 001/002/004/005;
порядок задач не меняет canonical prerequisites.

Проверки — только необходимый минимум core через production путь на Linux.
Переиспользовать existing core/peer и ресурсные cases между задачами. Никакого
test-only поведения, нового общего harness или полной visual/fault matrix.
Обязательный shipped corpus остаётся требованием к discovery/validation;
расширение набора проверок согласуется по AGENTS. Canonical software/accelerated
Linux conditions сохраняются; непроверенный backend указывается явно.
Windows/MinGW/Wine отложены и текущую Linux приёмку не блокируют.

Готовность: все 11 собственных outcomes имеют production результат, для каждого
из 34 obligations есть отдельная disposition и фактическое Linux evidence;
fallback/reset/cancel/Save/recreation не дают stale state или повторов. Полный
font path и wire bytes соответствуют effective owner. Pixel-perfect legacy
compatibility не требуется. План и snapshot не являются native PASS;
пользовательская приёмка и canonical runtime registration выполняются отдельно.

## Результат реализации — 2026-11-09

Production loaders, selection/publication, map preference/reset/definitions,
resource preparation и native F6 consumer реализованы в SV. Linux build и
необходимые core проверки проходят. [Evidence](sv-c006-evidence.md) фиксирует
software backend, shipped corpus, wire bytes и отдельные ограничения;
аппаратный backend, полные поздние gameplay flows и универсальный TTF glyph
profile не принимаются по этим результатам. Приёмка пользователя и registry
статусы остаются отдельными действиями.

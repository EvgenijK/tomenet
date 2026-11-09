# SV-C007-001 — active graphics, overlay и reporting C003

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Flow evidence: Linux core выполнен; непроверенные поздние branches сохранены в evidence.
Labels: enhancement
Связка: FI-02.
Owned IDs полного плана: `capability.rendering.tiles`,
`capability.rendering.subsets`, `capability.rendering.cache-generation`;
это integration task внутри их нормативных контрактов, без новых IDs.
C003 obligations: `capability.files.resource-overlay`,
`capability.files.report-resource-identity`; владельцы —
[SV-C003-001](../c003/SV-C003-001-file-owners.md) и
[SV-C003-005](../c003/SV-C003-005-resource-reporting.md).

## Решение и production consumer

Пользователь утвердил 2026-11-09 сохранение границ этапов: C003 предоставляет
проверенный file/reporting core и сообщает source-backed missing/disabled до
реального graphics consumer. C007 реализует и проверяет активную ветвь. C003 принят пользователем 2026-11-09; canonical prerequisites и allocations не меняются.
Disabled branch, injected snapshot и requested setting не являются active evidence.

Нормативная база: [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md),
[ADR-0002](../../adr/0002-main-thread-state-ownership.md),
[ADR-0003](../../adr/0003-explicit-module-connections.md),
[ADR-0005](../../adr/0005-session-scoped-work.md),
[ADR-0006](../../adr/0006-errors-and-overload.md).

Production consumer — реально работающий renderer/preparation owner, который
применяет tiles/font/mapping stack. До его появления production login использовал
`src/temporary/sv/stubs/play-profile.h` disabled graphics. Текущее состояние и
C003 public interfaces проверить при реализации; не проектировать второй owner
по устаревшему stub и не считать имя tileset доказательством его использования.

## Работа

1. Использовать C003 resolved owners для чтения, stat и применимых изменений
   graphics assets/profiles/caches: shared user U перед bundled B; запись
   сохраняет нормативного owner и SV cache namespace. Не копировать файловый
   поиск, publication или transient algorithms в renderer. Missing и invalid
   overlay различать, сохраняя declared consumer failure/fallback policy.
2. Сохранять full requested names/references отдельно от actual effective
   graphics mode/tiles/font identities. До публикации нового effective state
   закончить необходимые загрузку, mapping/subset overrides и подготовку;
   подтвердить применение ресурса renderer. Поколение actual source/profile/
   prepared cache учитывать по C007 контракту, не только имя mutable файла.
3. Подключить activation/reinit к единому reporting owner C003-005 и font owner
   [C006](../c006/SV-C006-001-map-resource-integration.md). Первоначальный PLAY и
   последующие font/resource packets сообщают фактический согласованный stack;
   version/audio сохраняют применимые metadata через существующие serializers.
   Не изобретать новые wire fields для requested/effective identities или cache
   generation. Version/font handlers реализуются уже в C003, повторять их нельзя.
4. Failed live change сохраняет прежний рабочий stack и effective reporting;
   startup failure выполняет approved fallback с явной ошибкой. Reinit/recreation
   не выдаёт старые identities за новые и не повторяет эффекты. Session change
   исключает stale command, хотя app resources могут жить дольше соединения.
5. Enabled/disabled результаты держать раздельно. Не требовать настроек C056
   или полного audio C055 для доказательства graphics; не принимать остальные
   rendering outcomes по успеху этого integration task.

## Завершение и минимальные проверки

Через existing Linux core/peer entrypoints проверить реальное load/preparation/
activation→rendering use→wire; U-before-B и invalid/missing resource; enabled
graphics, disabled/fallback, успешную смену, отказ с прежним stack, reinit и смену
session generation. Evidence должно показать actual effective mode/tiles/font
и source/cache generation; snapshot-only serializer test этого не доказывает.

Captured production PLAY/font и применимые version/audio metadata соответствуют
resource owner и всем source-defined layouts/gates C003. `%s` reporting copy
ограничена 79 bytes + NUL (`MAX_CHARS=80`, включая font/tiles с большими server
buffers); минимальные boundary cases 78/79/80 bytes сохраняют full local reference.
Переиспользовать C003 wire checks, добавляя только необходимое доказательство
реального enabled consumer. Общий harness/широкую новую matrix не вводить.

Вернуть в `docs/sv-c003-evidence.md` и [SV-C003-006](../c003/SV-C003-006-acceptance.md)
FI-02: production caller/revision, Linux команды, expected/actual owned paths,
active/fallback identities/generations и wire bytes. Сопоставить применимые
`resource-overlay` obligations и все четыре
`report-resource-identity.{result,failure-lifecycle,wire,allocation-checks}`
obligations. FI-01 возвращается C006 отдельно; отсутствующие branches остаются
pending. C003 уже принят пользователем; C007 этим не принимается. Возврат evidence
не prerequisite C003, ошибки исправляются в C007.

Только минимальные production-path core checks; расширение согласуется заранее
по AGENTS.md. [Linux обязателен](../../agents/platform-validation.md), Windows/
MinGW/Wine deferred. Здесь нет заявления о выполненной реализации или runtime PASS.

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.


## Уточнение полного плана после C006 — 2026-11-09

Единственный ID owner tiles/subsets/cache-generation — эта задача; graphics
PRF integration 002 не дублирует allocation. Текущий map/font bootstrap уже
production (`sv_map_font_bootstrap`); disabled stub выше описывает прежнее
состояние. Использовать C006 selection/preparation/glyph APIs и C003 files/
reporting, расширяя единый coupled candidate. Semantic composition 003 и
geometry 005 — реальные consumers; preview C006 не принимает full C007.
Cache key учитывает source/profile/masks/subsets/filter/outline/final-size
поколения; publication атомарна. Resize может кратко показывать прежний scaled
набор при продолжающихся updates/hit testing, затем выводит новый 1:1.
Итоговый scope/границы позднего map evidence — в [спецификации](../../sv-stage-c007-spec.md).


## Результат реализации — 2026-11-09

Production Linux core реализован агентами и подключён к native resource consumer.
[Фактические callers, команды, результаты и ограничения](../../sv-c007-evidence.md)
содержат отдельную disposition каждого obligation. Этот результат не принимает
полный C007, C013 live-map flow, hardware backend или непроверенные branches;
canonical registration остаётся отдельным workflow.


## Приёмка пользователем — 2026-11-09

Пользователь явно подтвердил: «этот этап принял».
[C007 и задачи 001–007 приняты](../../sv-stage-c007-acceptance.md) в представленных границах Linux core,
с сохранением описанных ограничений. Исторические формулировки pending выше
относятся к моменту подготовки плана/evidence. Live карта остаётся C013;
непроведённые проверки не получают PASS, canonical registry не изменён.

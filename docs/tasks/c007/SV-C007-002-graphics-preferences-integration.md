# SV-C007-002 — graphics preference stack и renderer integration C004

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Labels: enhancement
Зависимости: production preference core C004, C003 file/reporting owners,
[SV-C007-001](SV-C007-001-graphics-resource-integration.md) и
[SV-C006-002](../c006/SV-C006-002-map-preferences-integration.md).

## Решение и границы

Пользователь 2026-11-09 принял и закрыл C004; remaining graphics PRF integration
и проверки выполняются в C007. Найденные здесь дефекты исправляются в C007 без
автоматического переоткрытия C004. C007 пока не принят. Canonical allocation,
IDs, conditions, obligations и runtime statuses сохраняются. Трассировка:
[C004 source contract](../c004/source-contract.md),
[evidence](../../sv-c004-evidence.md), [приёмка](../../sv-stage-c004-acceptance.md).
Задача дополняет FI-02/001, не заменяя active renderer/reporting работу C003.

Нормативны [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md). Legacy определяет игровую/протокольную
семантику; его raster output не становится pixel oracle.

## Production путь

1. Встроить graphics PRFs в C006 resource stage после font preferences и до own
   system OPT при startup. `login/native-run.c` пока передаёт NULL resource hook;
   отдельной автоматической загрузкой после character pre-send этот порядок не
   подменять. Использовать существующий `SvPreferences` из
   `settings/preferences.[ch]` и `sv_preferences_load`, C003 resolved file owners
   и reporting. Renderer получает подготовленные semantic mappings; он не читает
   PRF и не содержит второго parser.
2. По effective выбранному graphics resource загрузить `graphics-<tiles>.prf`,
   затем enabled subsets в baseline порядке с точным filename derivation,
   `SvPreferenceContext.character_offset` и filename/context subtileset rules.
   Requested/effective identity и source/profile generation держать раздельно;
   shared U overlay precedes B, invalid/unreadable не маскируются отсутствием.
   Base failure и subset failure сохраняют baseline продолжение/диагностику;
   пригодность fallback определяет resource owner 001. Graphics off не исполняет
   active raw-picture/mask branch и сохраняет font-based map path.
3. Вместе с C006 visual reset seam подготовить graphics candidate без остатков
   прежних base/subset definitions. Перед graphics reload очистить именно
   graphics-owned raw pictures и mask preferences, сохранив новый font base;
   общую glyph-indexed subset identity сбрасывать/перестраивать с mapping stack,
   а не хранить независимую копию по каждому mapping family. Потреблять
   `sv_preferences_mapping*` и `sv_preferences_glyph_subtileset`: R/K/F/U
   numeric IDs/attrs, I raw picture geometry, m base/subset RGBA mask data.
   Проверить текущие semantic bounds/gates и 0-value behavior; inactive legacy
   branches не включать по комментарию. Если для actual consumer нужен узкий
   reset/state accessor, добавить его production preference owner, не another
   decoding implementation.
4. Применить state к реально используемым base/subset images, masks и raw picture
   lookups, сохраняя mixed glyph/tiles, anchors/extents и applicable pseudo-object
   definitions. После load attempt выполнить baseline graphics initialization:
   tileset без PRF использует объявленные defaults, а не вечное ожидание retry.
   Смена tileset заново подготавливает image/flags; offset graphics loading не
   протекает обратно в font definitions. Source/profile/cache generation
   инвалидирует производные данные; отсутствующая в новом stack запись не
   остаётся из старого tileset.
5. Публиковать согласованные active images/mappings/glyph subsets/masks/rawpics
   и C003 effective reporting только после подготовки применимого candidate.
   Расширить готовый font-based путь C006 до graphics-enabled
   font→graphics→client setup/font notifications→repaint lifecycle. Coupled
   overrides/reset/reload и его actual flow evidence принадлежат C007;
   завершение C006 не ожидает их реализации.
   initial/live server-visible definitions отражают реально применённый stack
   через существующие protocol owners/gates. Failed live resource activation
   сохраняет прежний рабочий набор; malformed PRF отдельно сохраняет ordinary
   partial/valid-tail semantics и понятную диагностику. Repaint не повторяет
   reload/effects; reinit действительно восстанавливает semantic state и caches,
   а stale session generation не отправляет packets/actions в новое соединение.
   Сообщения/actions реального PRF load имеют сохранённый payload/context,
   generation и явного владельца; C007 проверяет их production generation,
   ownership и отсутствие replay. Actual delivery выполняют C016/C053 по своим
   задачам и не является hard gate приёмки C007. Renderer не исполняет pending
   bytes и не объявляет их доставленными.

Baseline sources: `src/client/c-util.c:handle_process_graphics_file` (20137:
rawpic/mask reset, base/subsets, offset, defaults initialization и pseudo-objects),
`handle_process_font_file` (20224), `src/client/c-files.c:process_pref_file_aux_aux`
mapping/I/m branches. SV seams: `settings/preferences.h:SvPreferenceContext`,
`SvPreferenceMapping`, mapping/glyph-subset accessors и bootstrap hook,
`files/files.[ch]`, `protocol/login-identity.[ch]`, `protocol/login.c`,
`login/native-run.c`, `login/flow.c`. Источники пересверить при реализации;
текущий `ui/font.c` text font не доказывает map/graphics readiness.

## Минимальная Linux проверка и завершение

Дополнить existing 001/C006 production checks одним различимым base/subset
fixture: real load→prepare→activation→renderer use, точные mappings/offset и
glyph-subset sharing между families, base/subset masks, raw-picture geometry.
Смена enabled subset/base должна убирать прежние definitions/masks/rawpics,
graphics off возвращает применимый font path. Проверить missing PRF defaults,
invalid/unreadable overlay, valid tail после malformed record и failed live
activation с прежним stack. Existing peer фиксирует соответствие actual
client setup/font bytes prepared state; reinit/session switch не отправляют
stale определения и не повторяют одноразовые load effects. Source bytes
остаются неизменны. Нужны только минимальные Linux core/consumer cases;
не дублировать parser suite, полную B/B2 matrix или C003 serializer framework.

Завершение: действующий renderer использует base/subset/mask/rawpic state того
же production preference owner; reset/reload, defaults/failure и server-visible
definitions доказаны. В evidence C007 сохранить callers/revision, команды,
expected/actual sources, IDs/attrs/subsets/masks/rawpic extents, generations и
wire bytes. Связать применимые `obligation.preferences.load.{result,failure-lifecycle}`,
`obligation.preferences.bootstrap.{result,failure-lifecycle,allocation-checks}`
и реально достигнутые include-failure branches из [C004 snapshot](../c004/coverage.json).
Непроверенное не получает PASS; эта трассировка не переоткрывает C004 и не меняет
его canonical allocation. Actual message/action delivery остаётся обязательством
C016/C053, без backward prerequisite для C007; здесь доказаны generation/ownership
эффектов без fake execution. Полный C007 и другие message/macro outcomes по одному
integration ticket не принимаются.

Проверки — по [Linux rule](../../agents/platform-validation.md), расширение
согласуется по AGENTS.md; Windows/MinGW/Wine deferred. Нового runtime evidence
этот план не создаёт.


## Уточнение production seam после C006 — 2026-11-09

Упоминание NULL hook выше фиксирует состояние до реализации C006. Сейчас
`sv_map_font_bootstrap` подключён к native startup, font reset/PRF и definitions
имеют production owner. Graphics hook встраивается именно в этот path; проверить
актуальные interfaces по [source contract](source-contract.md). Новый startup
parser/owner и повторная загрузка после character pre-send не нужны.
Задача не получает отдельные capability IDs; единственный owner tiles/subsets/
cache-generation — 001, raw-picture — 006. Совместная подготовка 001/002
публикует один согласованный candidate и не создаёт циклических prerequisites.


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

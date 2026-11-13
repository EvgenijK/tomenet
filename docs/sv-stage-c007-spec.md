# C007 — игровая композиция и graphics lifecycle

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
[Техническое evidence и ограничения](sv-c007-evidence.md).
План подготовлен по запросу пользователя с анализом тремя агентами.
[Задачи и порядок](tasks/c007/README.md),
[source contract](tasks/c007/source-contract.md),
[exact planning snapshot](tasks/c007/coverage.json).

## Результат этапа

C007 продолжает production font/resource core C006 и реализует полноценную
семантическую композицию игровых visual layers. Ровно **15 canonical outcomes
и 45 obligations** сохраняют allocation, prerequisites, conditions и runtime
statuses. У каждого ID один owner; 002 выполняет интеграцию внутри результатов
001, 007 собирает evidence без второго allocation.

| `capability.rendering.*` | Результат | Owner |
|---|---|---|
| `tiles`, `subsets`, `cache-generation` | Active base/subset graphics, decoded masks, actual identities и согласованные prepared generations | 001 |
| `glyph-identity`, `terrain-stack`, `wipe`, `solid-wall` | Numeric glyph identity, независимые слои карты, очистка и сплошные блоки | 003 |
| `palette`, `animation`, `lighting` | Runtime palette/tint, временные категории и освещение всей карты | 004 |
| `cursor`, `resize` | Семантические indicators и общая DPI-aware geometry целой карты | 005 |
| `effects-restore`, `raw-picture`, `weather` | Временные слои, восстановление актуальной клетки, raw-picture geometry и weather lifecycle/wire | 006 |

Все IDs имеют result и failure-lifecycle. Все, кроме raw-picture, дополнительно
имеют allocation-checks; weather имеет ещё wire obligation. Planning snapshot
содержит канонические строки дословно и SHA-256 исходных JSON; план не изменяет
их pending implementation/evidence statuses.

## Нормативная основа

Применяются [AGENTS](../AGENTS.md), [CONTEXT](../CONTEXT.md),
[settings policy](capabilities/settings-policy.md),
[архитектура](sv-architecture.md), ADR
[0002](adr/0002-main-thread-state-ownership.md),
[0003](adr/0003-explicit-module-connections.md),
[0004](adr/0004-revision-based-ui-views.md),
[0005](adr/0005-session-scoped-work.md),
[0006](adr/0006-errors-and-overload.md) и
[правило платформ](agents/platform-validation.md).
Behavior baseline определяет игровую и протокольную семантику; новый canonical
raster pipeline определяет композицию. Legacy captures не pixel oracle.
Намеренные решения и superseded proposals различаются по финальным Answer и
поздним уточнениям tickets
[23](../.scratch/single-window-sdl3-client/issues/23-choose-raster-references-and-defect-compatibility.md),
[20](../.scratch/single-window-sdl3-client/issues/20-classify-display-urgency-and-latency-budgets.md),
[09](../.scratch/single-window-sdl3-client/issues/09-design-parity-evidence-and-acceptance.md).

## Обязательные правила поведения

1. **Identity и state.** Числовые legacy glyph IDs, source profiles, цветовые и
   управляющие роли сохраняются. C006 loader/default/visible one-cell fallback
   переиспользуется без TTF raw-byte guesses. Background, terrain и foreground
   остаются раздельными IDs/attrs в session presentation state. Renderer получает
   version-aware semantic changes через явные interfaces, не читает Term или
   legacy globals. Удаление foreground открывает latest terrain; wipe затрагивает
   только заданную область. Solid wall — самостоятельная drawing role.
2. **Композиция.** Background/color role → terrain → foreground → effects/weather
   → indicators/cursor. Encoded masks декодируются до фильтрации; authored colour,
   recolour coverage, background holes и outline сохраняют свои роли. Смешанные
   glyph/tile cells, independent subtilesets и baseline override precedence
   обязательны. Ordinary tiles и raw-picture rectangles имеют разные pipelines;
   raw-picture сохраняет anchors/extents/clipping и special-canvas transform.
3. **Ресурсы и PRF.** C003 resolved owners дают U-before-B и SV cache namespace;
   существующий SvPreferences C004 даёт base/subset mappings, masks и raw-picture
   definitions. Font base C006 → graphics overrides → actual preparation →
   activation → publication effective state → existing server exchange.
   Reset/reload очищает старые graphics definitions без повреждения нового font
   base; glyph-subset identity общая для mapping families. Missing PRF defaults,
   malformed partial/valid-tail semantics и invalid overlay различаются.
4. **Publication и failure.** Requested references сохраняются отдельно от actual
   effective stack. Failed live activation оставляет прежние images/mappings,
   reporting и рабочий output; startup failure использует утверждённый font-only
   fallback с диагностикой, сохраняя requested graphics. C003 serializers сообщают
   actual mode/tiles/font, сохраняют gates/slots и 79-byte reporting boundary.
   Second file owner, PRF parser или reporting serializer не вводятся.
5. **Geometry и caches.** Map font задаёт пропорции; logical UI units и текущий
   DPI преобразуются в общую целочисленную final cell geometry в output pixels.
   Полная сетка 66×22/66×44 максимально вписана и центрирована без crop и map zoom
   key. UI scale независим. Layers и hit testing используют одну geometry.
   Assets готовятся непосредственно к final size и рисуются 1:1; cache identity
   включает source/profile/mask/subset/filter/outline/size generation.
   Publication атомарна. При live resize допустимо краткое масштабирование
   прежней generation до подготовки новой; model updates и hit testing работают.
6. **Цвет и время.** Canonical palette roles, lighterDarkBlue, monochrome и
   допустимые custom entries применяются runtime tint без rebuild neutral assets.
   Animation сохраняет categories, timing и scopes no_flicker/subterm/misc;
   content roles заменяют legacy terminal topology. Torch/wall/floor/shading и
   TERM_SRCLITE сохраняют смысл/timing; нижняя половина big map не повторяет
   ошибочно верхний узор. Controlled time/RNG не означает pixel-identical particles.
7. **Effects и lifecycle.** Effect end восстанавливает latest semantic cell и
   остальные активные слои. Weather сохраняет параметры/type/wind/start/stop,
   no2mask_weather и wire version/build branches. Partial frame не публикуется;
   complete changes применяются по порядку. Repaint/resize/recreation не повторяют
   start sounds, PRF effects или sends. App resources могут переживать session;
   stale session commands/results не попадают в новое соединение. Queues/caches
   конечны, заменённые ресурсы освобождаются, hidden archive не появляется.

## Границы и prerequisites

**C003/C004/C006 приняты пользователем.** Их production owners переиспользуются;
найденные consumer defects и необходимые узкие seams исправляются в C007 без
автоматического переоткрытия принятых этапов. FI-02 и graphics preference evidence
трассируются обратно. C006 font-only acceptance не ждёт coupled graphics flow.
TTF ASCII profile и временный F6 UI из C006 не доказывают полный glyph corpus C007.

**C011/C013/C014:** C007 владеет rendering outcomes и необходимым semantic
consumer/adapter. Полная live session, world.read-map и игровой handoff не
принимаются по rendering fixture. Отсутствующий production decoder/state path
нельзя заменять test-only реализацией или незаметно включать весь будущий этап:
C007 реализует production semantic ingress/core, а полный Receive_line_info и
world map ingress остаются C013. Позднее packet-derived evidence остаётся pending
до настоящего consumer; core checks его не заменяют. Weather wire obligation
принадлежит самому C007: его production decoder→state→surface реализуется здесь.
Техническая готовность core не равна полному acceptance. Отдельная core-приёмка
возможна только по будущему явному решению пользователя.

**C016/C053/C055:** C007 доказывает generation/payload/context/ownership PRF effects
и отсутствие replay; actual PRF message/action delivery принадлежит C016/C053.
Weather start/stop должен иметь явный одноразовый output к исполнителю; наличие
этого output не принимает полный audio C055. **C018/C019:** отображение indicators
не принимает gameplay targeting/direction interaction. **C044/C046/C047/C056:**
полные settings/options surfaces и registry остаются у своих этапов; C007
потребляет применимые значения и не объявляет значимые active options inert.

## Минимальная проверка и приёмка

Проверки — только необходимый минимум Linux core через production публичные
interfaces, existing consumer/peer и готовые C003–C006 cases. План не поручает
новый общий harness, дублирование parser suites, полную fault/visual matrix,
intensive benchmarks или длительные soak. Расширение проверок согласуется по
AGENTS. Software rendering обязателен; software/accelerated результаты и
непроверенные backend branches отражаются раздельно. Windows/MinGW/Wine deferred.

Обязательства проверяются на production decode/state/resource/renderer пути:
font-only и mixed cells; single/two-mask/subsets; update terrain под foreground
и effect; wipe/solid wall; runtime palette без rebuild; controlled time/RNG и
full-height lighting; geometry/resize/recreation; actual source/cache generations;
PRF reload/default/failure; weather partial/chained packets и exact bytes.
Одна компактная сцена может подтверждать несколько obligations; каждый получает
отдельную фактическую disposition. Captures нужны для диагностики и review,
не для pixel-equality gates. Source validation или injected reporting snapshot
не доказывает active rendering.

Существующие submission budgets 20/50/200 ms сохраняются как применимые
urgent/interactive/background gates. Visible 50/100/250 ms — manual targets,
не обязательное точное physical-presentation измерение. Это не поручение
расширять suite: применять existing instrumentation и короткие functional
concurrent cases; отсутствующее evidence отметить явно. Новый общий RAM/GPU
ceiling не вводится; reference host/backend и ограниченность ресурсов описываются
при реализации.

Готовность: все 15 собственных outcomes имеют реальный production результат;
для 45 obligations есть evidence/disposition, failures не дают ложного success,
повторов и stale output. Canonical runtime registration и пользовательская
приёмка остаются отдельными действиями. Этот документ не создаёт PASS.

## Уточнение UX для задачи 005

[Ticket 31](../.scratch/single-window-sdl3-client/issues/31-prototype-cursor-and-cell-indicators.md)
закрыт делегированием внешнего вида основному HTML UX prototype; варианты A/B/C
пользователь отверг. Read-only просмотр текущего
`/home/svechnik/Projects/github_site/tomenet_interface` нашёл hover rectangle в
`js/features/map.js:130–136`, target crosshair `.throw-target-cell` в
`css/windows.css:325–354` и pointer themes в `css/cursors.css`. Последние не
заменяют semantic cell indicators. Делегирование HTML уже принято: для 005
закрепить применимую HTML revision и перенести найденные hover/target состояния,
сохранив их наблюдаемый scope. Полное self/target/cell native visual evidence и
недостающие self states остаются pending; наличие CSS/JS не превращается в UX
approval. Новый выбор рисунка сейчас не требуется. Недостающие состояния
сверяются с актуальным основным prototype при реализации; конкретный
неразрешимый UX gap выносится пользователю по готовому визуальному результату.


## Реализация — 2026-11-09

Production Linux core и native resource consumer реализованы по задачам 001–006;
007 содержит техническую сводку. [Evidence](sv-c007-evidence.md) отделяет observed
core от поздних packet-derived callers, visual/backend/budget conditions и
непроверенных branches. Полная приёмка, отдельное решение о core-приёмке и
canonical runtime registration не выполнялись.


## Приёмка пользователем — 2026-11-09

Пользователь явно подтвердил: «этот этап принял».
[C007 и задачи 001–007 приняты](sv-stage-c007-acceptance.md) в представленных границах Linux core,
с сохранением описанных ограничений. Исторические формулировки pending выше
относятся к моменту подготовки плана/evidence. Live карта остаётся C013;
непроведённые проверки не получают PASS, canonical registry не изменён.

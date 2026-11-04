# B2: acceptance and stage integration

Type: decision
Status: resolved
Assignee: b2_acceptance_plan
Labels: enhancement, ready-for-agent

## Question

Как принять перенос login UX prototype в SV на Linux и включить presentation stage B2 между принятым B и C001 без новой capability allocation, повторного открытия B и разрастания проверок?

## Answer

### Граница нового этапа

B2 — отдельный presentation checkpoint после принятого B и перед C001, с **нулём новых capability IDs**. Он исправляет потерянное требование approved visuals из [acceptance contract](../../single-window-sdl3-client/acceptance-contract.md), а не переоценивает принятое поведение B. B54, A8, denominator 925, каталог C001–C064, canonical ownership, prerequisites и allocations остаются прежними. B2 tickets ссылаются на B owners как на consumers и источники semantic requirements, но не присваивают outcomes себе. Нельзя добавлять B2 в JSON stage catalog ценой миграции схемы/hashes или менять frozen B rows ради presentation checkpoint.

Решение пользователя «не ожидаем оставшихся тестов и принимаем» от 2026-10-28 сохраняется полностью: B/B-075 accepted. Не возвращать как prerequisites B2 незавершённые Secret Service unlock/cancel/refusal/full-caller persistence, старый human review, отдельную ping regression, обязательный ibus/fcitx или actual OS IME. Эти проверки не получают PASS и не превращаются в B2 долг. Новый native visual/input review нужен для **изменённого интерфейса B2**, а не для восстановления снятого B gate.

B2 не добавляет real-server integration, post-MOTD startup, profile/FILE/Lua, gameplay/map/HUD, поздние settings/Guide/exports capabilities. Controlled peer остаётся внешним исполнителем для production executable. Mock HTML не служит источником gameplay semantics; metadata/news/links, демонстрационные population/ping и декоративные affordances получают явный disposition по [решению о reference](01-web-reference-and-screen-scope.md), без выдуманных native функций. Уровни UX/поведения разрешаются по CONTEXT.md: confirmed appearance/layout из HTML, production outcomes из behavior baseline и принятого B.

### Зафиксированный visual target

До реализации сохранить доступный внутри проекта snapshot approved login reference со всеми используемыми local CSS/fonts/images/news assets и provenance: origin path/repository, revision, dirty/untracked state и SHA-256 фактических файлов. Одной ссылки на внешний `/home/.../github_site/` и одного commit недостаточно: фактические рабочие файлы могут отличаться от commit. Отдельно записать выбранный variant (сейчас login prototype D), query/options, viewport, browser/font setup и используемые synthetic display data. Снимок visual reference не является executable native mock или протокольной fixture.

На момент приёмки сравниваются один и тот же versioned reference и **текущий собранный production `tomenet-sv`**. Для каждой согласованной пары экран/состояние рядом доступны reference/native captures при одинаковом logical viewport и сопоставимых synthetic display data. Перечень selectors и адаптаций фиксируется решением 01 и переносится в spec; для native-only состояний указывается утверждённая адаптация в том же visual language, а не отсутствующий HTML screenshot.

Минимальная карта для checklist:

| Native family | Reference / state mapping | Что реально проверять |
|---|---|---|
| Endpoint/contact/manual | `server`, `#manualForm` overlay; selected/edit/error/loading/cancel | Shell, ASCII logo/background, server cards, manual host modal, действительные controls и статусы подключения; никакого фиктивного server discovery |
| Account/auth/create account | `account`; empty/edit/private/validation/rejected/wait | Центрирование формы, field/button geometry, password mask, focus/selection/caret, читаемые ошибки и навигация между существующими B actions |
| Character overview/name/slot | `character` и name/slot adaptation | Список/детали, selected/empty/full/disabled/rejected, реальные slot identity, уже существующие reorder/ordinary/exclusive actions и длинные значения без наложения |
| Birth | `character-create`; mode/race/trait/class/body/stats/confirm, где production B их предоставляет | Choice/list/detail/summary composition, ограничения доступности, backtracking/cancel, scroll и visible selected/focus states |
| MOTD | Отдельный approved MOTD prototype `?variant=C`, decision 28; не login shell | Полные 23 строки с исходными цветами/позициями, maximum uniform fit по обеим осям, padding 24 logical; без wrap/crop/scroll, прежний any-key/skip/once-only/order и handoff |
| Wait/error/return/quit/help | reference overlay/shell/quit либо явно документированная native adaptation | State-specific действия, возврат focus, native-only birth/DNA/help controls; Guide остаётся принятой заглушкой, конечная граница handoff без gameplay |

Итоговая карта не должна скрыть экран с текстовой заглушкой: каждый изменённый экран и layout-distinct state имеет reference/adaptation и отдельный результат. Не требуется screenshot каждой комбинации protocol version × build flag × selection × error: состояния, использующие одинаковую геометрию, можно объединить с указанием representant и основания.

### Критерии visual/input acceptance

Новый gate подтверждает узнаваемое соответствие согласованному макету: композиция и пропорции крупных областей, палитра/контраст/декор, иерархия шрифтов, spacing, формы и кнопки, карточки/списки, выделение и summary/details. Отдельно фиксируются утверждённые различия и их причина. Перечисление CSS tokens либо успешный compilation не доказывают перенос дизайна. Pixel-perfect comparison, raster golden threshold и новый screenshot framework не требуются.

На изменённом production UI проверить читаемость текста; отсутствие скрытых обязательных controls, overlap и unintended clipping; соответствие click rectangle видимому control; disabled/hover/selected/focus distinctions; caret/selection/password masking; переход focus клавиатурой и мышью, Enter/Escape/back/cancel по реальному caller, scrolling и возврат после overlays/errors. Screenshot подтверждает внешний вид, а краткий recorded input outcome — работающие controls; статичный кадр не доказывает hit testing или production wire path.

Linux software и OpenGL получают новые visual/input observations на одной актуальной сборке, с описанием реального video/backend/display. Software нельзя подменять software fallback неизвестного accelerated renderer. Windows/MinGW/Wine остаются deferred по [правилу платформ](../../../docs/agents/platform-validation.md).

Использовать существующие размеры [контракта](../../single-window-sdl3-client/acceptance-contract.md): 1024×768 logical minimum, 1920×1080 100%, 3840×2160 output при 200% (1920×1080 logical), fractional 125%/150%. Измерять actual logical/output dimensions и scale. Это bounded review geometry, не полный Cartesian product: все screen families сравниваются при одном основном размере; minimum и каждый scale применяются к representative affected forms/lists/birth, чтобы проверить новую геометрию и rounding. Software и OpenGL каждый получают основной сквозной сценарий и representative scaled hit check; не требуется повторять каждое состояние на каждом backend/scale. Отдельно сверить существующий перелом D при logical width ≤1240 и короткий layout при height ≤800: скрываемые decorations не должны убрать required actions; для birth selection сверху/details снизу и summary слева. Existing HTML breakpoints ниже native minimum не создают mobile/native scope. Virtual geometry допустима с честной маркировкой; она не доказывает physical display timing или настоящий OS IME.

### Проверки поведения и ограничения набора

Переиспользовать существующие production-path проверки изменённых modules: начать с `tests/sv_endpoint_checks.py`, `tests/sv_login_checks.py`/`sv_login_live_checks.py`, relevant account/character/birth suite, выбранного по реально затронутому пути. Список конкретных existing commands и причины выбора фиксирует implementer по diff. Не вводить новый обязательный полный прогон всех historical protocol/version/provider suites, новый checker, новый harness или дублированную test-only implementation. Если изменился только presentation, historical byte/protocol evidence остаётся ссылкой с ограничением freshness, а не результатом нового прогона. При неожиданном protocol изменении сначала вернуть presentation scope либо отдельно согласовать расширение задачи.

Сквозной runtime review B2 использует existing production executable/controlled peer для двух коротких routes: existing-account/existing-character → MOTD/handoff и account/character-create → birth/back/cancel/confirm → MOTD/handoff; действия раскладываются между keyboard и mouse, без повторения всей version matrix. Peer-only data setup допускается; прямое заполнение тестовой scene не заменяет runnable startup flow. Новые automated tests не предписываются этим планом: минимальный core regression допускается только при конкретной необходимости; расширение проверок заранее согласовывается с пользователем по AGENTS.md.

### Plain Markdown evidence schema

Достаточны обычные spec/checklist/report; validator не нужен. Пример записи в `docs/sv-stage-b2-acceptance.md`:

```markdown
Статус: planned / implemented-awaiting-review / accepted
Reference: snapshot path, origin revision + actual file hashes, variant/options
Build: SV revision + dirty diff/files + executable SHA-256, build command/options
Environment: Linux, SDL/video backend, software/opengl, fonts/resources
Scope: presentation only, B54 ownership unchanged; explicit adaptations

| Screen / state | Native entry/action | Reference selector/capture | Native capture | Logical/output/scale/backend | Expected → actual | Result / issue |
|---|---|---|---|---|---|---|

- [ ] Все screen families имеют reference или согласованную adaptation.
- [ ] Visual comparison проверяет composition, typography, spacing и controls.
- [ ] Production keyboard/mouse/edit-focus/hit geometry проходят на изменённом UI.
- [ ] Minimum, integer/fractional scale cases распределены по affected layouts.
- [ ] Software и OpenGL имеют actual evidence текущей сборки.
- [ ] Relevant existing production checks: exact commands, result, affected path.
- [ ] Все расхождения/limitations перечислены; missing checks не объявлены PASS.
- [ ] B remains accepted; waived checks и deferred Windows не стали B2 blockers.
- [ ] Stage ends at live-session handoff; no gameplay or real-server claim.

Acceptance decision: reviewer/user, date, accepted scope, explicit limitations.
```

Secrets/real passwords не попадают в кадры, logs или reference; используются synthetic данные, видимые private fields остаются masked. Evidence после последнего visual/input изменения обновляется по затронутым строкам, без механического повторения всего каталога. B2 не accepted лишь на основании закрытых implementation tickets: итоговая visual/input сверка и решение фиксируются явно. Этот planning ticket не утверждает runtime PASS.

### Documentation integration для оркестратора

1. Создать `docs/sv-stage-b2-spec.md`: цель, frozen reference, definitive screen/state/component map, native adaptations, scope/non-goals, критерии выше, dependencies и user-visible result. Один источник требований; tickets ссылаются на него.
2. Создать `docs/tasks/stage-b2/README.md` и local `SV-B2-*` implementation tickets в порядке dependencies: freeze reference → минимальная SV presentation foundation у первого consumer → screen slices → visual/input gate. Каждый ticket: статус planned, результат, конкретные production consumers, prerequisites, reference anchors, минимум existing checks и DoD. Это presentation work items с нулём owned IDs; не второй coverage ledger.
3. Добавить B2 row между B и after-B/C001 в `.scratch/single-window-sdl3-client/migration-sequence.md`; уточнить текст о fixed cumulative checkpoints, что zero-capability presentation checkpoint B2 не меняет frozen allocations.
4. В `docs/sv-functional-stage-plan.md` добавить prose/link о B2 перед C001; canonical table/counts/catalog/hash не менять. Тот же короткий navigation link добавить в index `docs/tasks/stage-reallocation/README.md`, если он остаётся входом в C sequence.
5. В `.scratch/single-window-sdl3-client/acceptance-contract.md` добавить узкое dated amendment/link на B2 visual target и current Linux rule; существующий общий visual requirement не переписывать как новый и исторические результаты не менять.
6. В `docs/tasks/stage-b/README.md`, `docs/sv-stage-b-spec.md`, `docs/sv-stage-b-acceptance.md` допустима только current navigation note: B accepted по прежнему решению, недостающий presentation перенос выделен в B2. Не открывать owners/B075, не переписывать их source/runtime evidence. Если достаточно B2 spec backlinks, массовое редактирование B tickets не нужно.
7. Создать `docs/sv-stage-b2-acceptance.md` как checklist planned, без PASS claims; либо отложить отчёт до реализации, сохранив обязательную schema в spec. Planning effort `.scratch/sv-stage-b2-planning/map.md` индексирует три decision tickets и связывает итоговый plan.

Не требуются новые ADR, изменения glossary/AGENTS.md, schema/JSON catalog/coverage manifests, planning validators или test framework. Нужные presentation foundations остаются локальными SV; общие/legacy улучшения относятся в `docs/sv-improvements.md` лишь при обнаружении конкретной проблемы, а не из-за допустимого duplication. Изменён только этот decision ticket, implementation/tests не запускались, graph не обновлялся.

# B2: web reference and screen scope

Type: decision
Status: resolved
Assignee: b2_ux_plan
Labels: enhancement, ready-for-agent

## Question

Какая зафиксированная web reference определяет visual scope B2 и как её screens/states/components отображаются на принятые native surfaces этапа B без изменения behavior baseline?

## Answer

B2 переносит presentation принятых B surfaces на **единственный текущий web-макет D (ASCII Chronicle)**. Нормативность: `CONTEXT.md` отделяет подтверждённый visual/layout от behavior baseline; ADR 0001 сохраняет ownership session/interaction/UI/protocol, ADR 0004 — presentation cache/layout invalidation. B остаётся принятым решением пользователя 2026-10-28; B2 не переоткрывает B-075, не меняет B54 ownership или C001–C064. Только Linux; Windows/MinGW/Wine deferred. Нового требования ibus/fcitx нет.

### Reference freeze

Read-only источник: `/home/svechnik/Projects/github_site/tomenet_interface`. На 2026-10-28 `git rev-parse HEAD` = `1bb2d5d45b9001bf2a6f2084f78184c29a73fca7`. Login HTML/CSS/data/background assets tracked и без tracked diff. Рабочее дерево всего web repo **не clean**: `M ../.gitigonre`, untracked `.playwright-cli/`, `.scratch/`, `AGENTS.md`, `HOTKEYS.md`, `MENUS.md`, `PLAYERS_ONLINE_DATA_CHECKLIST.md`, `README.md`, `SERVER_CONSTRAINTS.md`, `docs/`, `reference-full.png`, `world-reference.png`, `tools/update-login-news-snapshot.mjs`. Поэтому SHA HEAD недостаточно для воспроизведения документации. Ни один из этих файлов не изменён этим исследованием.

SHA-256 прочитанного reference:

| Relative path | SHA-256 |
| --- | --- |
| `login.html` | `038343aa9ee5aa8d1d2a8ca8fb5a87740730308a90ed7ea5dfd0b3cfa405e355` |
| `css/foundation.css` | `548a739c4be10bc9183feb11f91ba35a0b1585f25504669852cddf400c5ff368` |
| `css/theme.css` | `184d65e68b82a81eb26f9e1198efd0804c6ea96af250a74b8f1c765501ee3a06` |
| `js/data/creation-descriptions.js` | `e03790ba4d9a5ad16e855d58c1fd924f44a7a56a51e0077c65ba4df8da2b0fca` |
| `js/data/login-news-snapshot.js` | `fc7da1ee7b64f5d718b98f658518ececfc85876951a9665cd1123f342f8d63fe` |
| `README.md` | `3a491afa9fa76580994c6f0c88ba6064679442a6b22e10617311f3bf6e0b5886` |
| `SERVER_CONSTRAINTS.md` | `16b35475815007318a37710529178ed802729caf328317cd73d22a3af94cc76d` |

Backgrounds ниже — ровно 19 PNG, перечисленные в `login.html:2178–2196` (не все произвольные файлы assets). SHA-256 фиксирует каждый фактически читаемый файл:

| Asset under `assets/log_in_backgrounds/` | SHA-256 |
| --- | --- |
| `cirith_ungol_ascii_seed06.png` | `39c853c65294fe9907c6643386dafe49931322dc91095f133c3a7878bf500296` |
| `cloud_planes_ascii_seed14.png` | `8b4ac7df7c53cd17eeddeff78322322d560ae204f3572d463ddba2104e3c2eff` |
| `erebor_ascii_seed19.png` | `2a1d09395720428e78d28978a1d0f5b9c47862133d88d578d24173727da3fed9` |
| `halls_of_mandos_ascii_seed9.png` | `3f084173e26ee05ff246bf6f0b643697ecaf7e5ac1d8a8bdc29f3a80042f9795` |
| `heart_of_earth_ascii_seed17.png` | `21e5f55fdc370833b7683fbff208e61701edfa62a2f33f779143807a8e845f39` |
| `helcaraxe_ascii_seed31.png` | `66b9661b90ab06e0a4a816df5f12d220ca8afa92829e054761af936947873298` |
| `illusory_castle_ascii_seed16.png` | `2725e1b628b2f8098271db59e040c7b1439ef59249f5eed91e83535d54750ab1` |
| `land_of_rhun_ascii_seed36.png` | `7b777f13396e846f1e83056ba250728d4e9b3d2b876a05c444d51cd8cf09ff4a` |
| `mirkwood_ascii_seed01.png` | `91930ad54afe333aaaa4ab2e107f5e6697b819d924d25ec5d4ca97236031bcd7` |
| `moria_ascii_seed22.png` | `693885b12737466a502bd3f13cc80f5621d55411f588159f8b9321cb41fe2c53` |
| `mount_doom_ascii_seed24.png` | `e4324e912bd0e24edff8adfdae3b696d0ecfd85ee97354497d730a1d95820f2a` |
| `nether_realm_depth195_ascii_seed34.png` | `67856eb59b868b3c28c99a562ae256a58b938934522053dde03af5354088f919` |
| `old_forest_ascii_seed04.png` | `f4f6a31a22cadf8cfe3f7bd4cd07ed28e9682ef6ed0414da4b660a1e2a7ace95` |
| `orc_caves_ascii_3168x1584.png` | `3a404fe9b93854bc3019689c6746f12a3f7faa20c76c536146f2def904a28a96` |
| `sacred_land_of_mountains_ascii_seed70.png` | `3a8f351174d8df9d8c2b81a0bb2cd34626a9c246ca663c80441f0c6d4b5a3357` |
| `sandworm_lair_ascii_seed31.png` | `549ca59150fc9fddee81f27b2e0005a5996ce7b95941f3afe7dcb8a9f6133dbc` |
| `small_water_cave_ascii_seed44.png` | `9a36cd20ced8e9ef19eb2fd59590a50fa37c2a56b18b55287c59dc5a3ccf16c6` |
| `submerged_ruins_ascii_seed01.png` | `49ee55de6c97623b5df0c0073969f2e3bb9e5fa27a15f6eaac12c9ba889093ec` |
| `training_tower_ascii_seed404.png` | `c60bfec30473fc0bf9f01da8bd3ce929d74a21d127bc5929076fd8df96ec4b57` |

Reference capture при реализации должен записать именно эти hashes, logical size, font/scale, selected background и animation frame. Если upstream изменился — сверить изменение и явно обновить reference; не подменять эталон последним HEAD молча. PNG не являются pixel oracle для native glyph renderer.

### Что подтверждено и что остаётся только демонстрацией

- User запросил исправление native UI по существующему макету; принимаем текущую композицию D как источник visual remediation. README описывает её как единый сохранённый макет. Отдельной записи human approval каждого CSS размера/цвета в доступных sources не найдено; не называть все micro metrics индивидуально утверждёнными решениями.
- Правила whole-window sequential pregame, forms без automatic font shrink, minimum **1024×768 logical**, no gameplay HUD, preserved baseline transitions — подтверждённый answer [layout decision 21](../../single-window-sdl3-client/issues/21-specify-surface-layouts-and-responsive-rules.md).
- **MOTD не существует в `login.html`.** Его отдельный подтверждённый прототип — [decision 28](../../single-window-sdl3-client/issues/28-prototype-the-server-defined-motd-screen.md), `.scratch/single-window-sdl3-client/motd-prototype.html?variant=C`, human choice C, capture commit `e271c582cbf89d404deda6189881e531d3775770`. Использовать этот источник, не изобретать login-styled welcome screen.
- Servers, ping/auth/create/play в JS — локальные fixtures/simulations; Play переход в `index.html` не native outcome. Порядок/доступность/длины/байты/actions/race-class masks/returned setup data определяет принятый B production path и baseline, а не JS arrays.
- World Chronicle — автономный snapshot, не live service (`README.md`, `SERVER_CONSTRAINTS.md:150+`). Это источник внешнего вида левой области, **не требование нового news/backend/scraping subsystem**. Прототипные имена/даты/news/status не выдавать за фактически полученные runtime данные. Если отсутствует разрешённый источник наполнения, сохранить визуальную область с честным empty/unavailable state; поставка статического snapshot как product content не требуется этим решением.

### Actionable mapping

Native integration owner сегодня `src/client/sv/ui/endpoint-scene.c::sv_endpoint_render` с presentation `session/login-view.h`, birth interaction view и layout/hit geometry `ui/login-layout.h`; это точки сверки, а не требование увеличивать существующий renderer одним монолитом. Production interaction owners остаются прежними.

| Web source selector / asset | Native B2 surface / обязательный перенос |
| --- | --- |
| `body[data-variant="D"]`, `.login-background`, `.login-shell`, `.login-hero`, `#accountLogo` | Общий pregame shell: тёмная green-black/gold тема, ASCII backgrounds, большой клеточный логотип. Реальные окна и controls внутри единственного SDL_Window, а не raster screenshot веб-страницы. |
| `#serverStep`, `#serverGroups`, `.server-option`, `.server-enter`, `.server-health`, official/unofficial groups | Server select: карточки, выбор/hover/focus, отдельные ворота подключения, server details/player names/ping только из native view; pending/unavailable telemetry и пустой/ошибочный список остаются видимыми. Не фиксировать пять demo серверов или обновление каждые 5 секунд как protocol contract. |
| `#directBackdrop`, `#manualForm`, `#hostInput`, `#manualError` | Native manual endpoint: модальная карточка host:port, draft/caret/selection, inline encoding/address errors, Cancel/return focus; не делать hidden Q-action недоступным мышью там, где baseline/native требует способ доступа. |
| `#accountStep`, `.account-welcome`, `.account-form`, `#accountInput`, `#passwordInput`, `#passwordToggle`, `.account-torches` | Account/auth scene: centered form, labels, masked password/show control, primary Log in, original guidance, декоративные torches на wide. Native validation и фактические B store/lookup/status состояния сохраняются. Provider-проверки external replacement/delete не означают существующие UI-кнопки: B2 не добавляет replace/delete management. Password reveal допускается только внутри private render; раскрытое значение не копируется в logs/cache/captures. Credentials не попадают в screenshots/logs. |
| `.loading-mask`, `.loading-card`, `#loadingTitle`, `.field-error`, `#statusLine` | Connecting/auth waiting/rejection/transport failure: визуальное семейство loading/error/status; factual status и допустимые Cancel/Back определяются native state. Не вводить universal retry/reconnect. |
| `#characterStep`, `.character-list`, `.character-option`, `.character-level`, `.mode-badge`, `.character-location`, `.character-native-hints`, `#createCharacterButton`, `#createExclusiveButton` | Character overview: плотные строки level diamond/name/race/class/mode/location, selected/focused state, внутренний list scroll и фиксированные actions/footer. Обычное/slot-exclusive creation и quit сохраняют фактическую B semantics. Reorder S/I/A отложен B README/B-022 в **C039**: его реализация вне B2; web-подсказки визуально адаптировать как disabled/unavailable либо скрыть с явной записью intentional adaptation. Не добавлять glyph/winner/realm из mock data. |
| `#characterCreateStep`, `#creationSummary`, `#creationStageTitle`, `#creationContent`, `#characterNameInput`, `.creation-choice-grid`, `.creation-detail`, `.creation-stat` | Name → Sex → Class → Race → Trait → Body → Attributes → Mode как presentation принятых native interactions; summary слева с живыми значениями, legal/disabled options, scroll, details/native guidance, stat columns/colors. Trait skip/allowed choices берутся из native setup. Для ordinary name entry использовать тот же visual family без forced JS stage semantics. |
| `#attributeWarningBackdrop`, `#closeAttributeWarningButton`, `#continueWithUnspentButton`, `#creationGuideBackdrop` | Child confirmation/help surfaces: безопасный default focus, state-specific controls, baseline Escape/Backspace/Y/N semantics. Random и другие фактически доступные B actions не исчезают из presentation лишь потому, что HTML показывает меньше controls. DNA persistence относится к **C040** и вне B2; DNA/automatic-DNA controls отображаются только в фактически поддерживаемых B состояниях, неподдерживаемые — documented unavailable, без новой реализации хранения. |
| `.realm-links`, `.realm-link`, встроенные SVG и текст IRC | Шесть внешних ссылок: website, Discord, IRC, official guide, RU guide, GitHub — visual icons/labels/focus. Внешнее OS URI handling относится к **C060** и вне B2: ссылки могут получить visual-only disabled/unavailable adaptation, без новой интеграции запуска browser. Отсутствие browser integration не повод создавать backend requirement. Не переносить browser-only prototype navigation. |
| `#quitStep`, `#returnToServersButton` | Источник visual family завершения; только штатные native quit/end/relogin paths. Browser-safe return-to-list не является основанием заменить native process/session lifecycle. |
| Separate MOTD variant C | Whole-window centered full **23-row** server-defined composition, decoded visible width, original colors/positions/ASCII/unknown content; maximum uniform fit по обеим осям с 24 logical padding. No wrap/crop/scroll/new headings/background decoration over content. Any-key/skip/once-only/order сохраняются. |

### Tokens, components и responsive contract

Переносить **вычисленный cascade D**, а не ранние общие/старые варианты CSS. В `login.html:16–28` определены background `#030504`, surface rgba(8,12,10,.94), border `#443a20`, bright border `#977a36`, gold `#e6c04f`, green `#4add5a`, red `#ef5549`, text `#e4e2d8`, muted `#858b82`, dim `#565b55`; позднее D уточняет panels rgba(4,8,5,.91), border rgba(130,111,62,.52), selected gold translucent. Семейство: dark translucent panel/border, raised field, primary/secondary buttons, disabled/hover/focus, caret/selection/errors, modals and scrolling lists. Важен последний D override `login.html:1104–1110`: body **monospace** Consolas/Liberation Mono/DejaVu Sans Mono (он перекрывает промежуточный sans-serif stack `body.login-page` на строке850); choices/ASCII logo также terminal role. Конкретный установленный Linux font фиксируется в evidence, без требования install/download web fonts.

- Wide D: shell ≤1720 logical с наружными 15-unit margins; слева и справа равные колонки 180–340, центральная panel clamp(400, viewport−414,860). Large logo по центру над ними; decorative Chronicle/links не сдвигают центральную форму.
- **≤1240 logical width**: финальный override `login.html:1639–1663` делает hero/panel/links single-column, скрывает Chronicle/torches/link labels; шесть icons одной строкой ниже panel. Это pregame adaptation, не отмена принятого запрета auto-switch gameplay shell.
- **Height ≤800**, width >820: короткая hero row, меньшие intervals/server cards (72 вместо 88); text не shrink-to-fit. На minimum 1024×768 required actions доступны; long forms/lists scroll.
- Birth: финальный override `login.html:1665–1734` заменяет старую «detail справа» композицию: выбор **сверху**, description **снизу**, 60/40, independent scrolling, summary слева. README содержит исторические описания right detail — при конфликте приоритет actual final CSS + последнее описание верх/низ, а не случайный ранний абзац.
- ≤820/540 web mobile styles существуют, но не расширяют native minimum1024×768 acceptance или список обязательных runtime размеров. При UI scale/DPI effective available extent controls не должны обрезаться; выбрать overflow presentation вместо незаявленного font shrinking.
- Background list: random initial/30s switch/2s fade/9s gentle movement; pause hidden/minimized, reduced-motion static. Cell logo/torches около100ms cadence. Это decorative presentation, не сроки protocol/service. Выбор native scheduling не должен блокировать ввод, auth или resize; unsupported blur/shadow может воспроизводиться эквивалентной compositing treatment без изменения композиции.

### Узкие решения при реализации

Нет основания блокировать planning ради повторного выбора общей композиции: D уже задан задачей. Если renderer не способен воспроизвести конкретный эффект/поставляемый шрифт, предложить **конкретную** локальную замену и показать reference/native side by side; не объявлять отсутствующий визуальный слой «поздним этапом». Наполнение Chronicle требует отдельного решения только если захотят actual news content; B2 не строит его. Фактически доступные B password-store/lookup/status и native-only controls компоновать в той же visual family с минимальной адаптацией. Reorder C039, DNA persistence C040 и external OS URI C060 остаются у своих owners; B2 не получает их поведение через сходство web controls. Для неподдерживаемых controls записать intentional unavailable/hidden adaptation.

### Готовность результата B2

Каждый mapping row должен получить owning implementation ticket и reference/native Linux visual evidence с явно перечисленными intentional adaptations. Сопоставление должно охватывать server/manual/account/auth waiting+error/overview/name/birth+child overlays/MOTD; web reorder отображается только как documented unavailable/hidden adaptation и native-only actions; красивые shell/backgrounds без этих surfaces не закрывают B2. Ни новых automated suites, ни pixel-perfect golden gate этот planning answer не вводит; минимальный production-core test scope и предварительное согласование расширения остаются правилами AGENTS.md. B2 **планируется**, не реализован и не принят данным исследованием.

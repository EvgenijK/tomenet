# Specify surface layouts and responsive rules

Type: prototype
Status: resolved
Assignee: codex
Blocked by: 03, 22

## Question

Для independent text/map fonts, общей map glyph/tile grid и legacy document/store coordinates применить [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md). Макеты должны показывать разные text/map font sizes при font-only и mixed tile/glyph карте; concrete DPI/units/viewport thresholds и special-store raw-picture geometry остаются здесь. Не связывать text-only font change с map visual definitions или map zoom.

Какая конкретная per-capability компоновка single-window surfaces реализует согласованную interaction model: постоянная игровая основа и критический HUD, один primary с внутренними страницами/вкладками, дочерние prompts и map targeting, document viewers и special-store drawing области, pre-game/death/relogin состояния? Для каждой surface какие размеры, масштабы и density rules поддерживаются, как ведут себя длинное содержимое, прокрутка, focus и collapsed controls при resize, и как сохраняются видимость критических данных и все штатные точки входа без нового запоминания состояния разделов?

Опираемся на [Define the single-window interaction model](03-define-single-window-interaction-model.md), baseline inventories и подтверждённый HTML UX. Messages открывается primary, как Inventory/Abilities; независимая раскрываемая дополнительная лента не вводится. Проверить реальные макеты при различных размерах, а не только условный переключатель демо; обеспечить per-capability surface/entry-point evidence. Game command/chat/cancel semantics не перепроектируются. Итог — согласованные правила и linked prototype evidence для implementation-ready спецификации; runtime client остаётся вне scope.

После [Specify renderer parity](13-specify-renderer-parity.md) определить также пользовательский DPI/coordinate contract: logical UI/window units против surface pixels, font/cell/tile scaling, monitor-scale change и независимость server-visible map viewport от размера HUD/panels. Уточнить влияние UI scale/resize/primary overlays на доступную карту, поддерживаемые dimensions и normal/big-map fallback. Encoding и длины formatted content задаются в [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md); не считать legacy cell alphabet Unicode layout.

## Comments

### Session start — 2026-09-15

Claimed by codex как первый open/unblocked/unclaimed child карты. Применены wayfinder, prototype/UI, grilling и domain-modeling. Решений и human UX approval в этой сессии пока нет.

Spatial asset: [Layout prototype](../layout-prototype.html), три варианта на одном игровом shell, `?variant=A/B/C`: central primary, right primary с reserved map area, bottom primary с reserved map area. Standalone HTML; запустить из native repo командой `python3 -m http.server 8874 --bind 127.0.0.1 --directory .scratch/single-window-sdl3-client`, затем открыть `http://127.0.0.1:8874/layout-prototype.html`. Все thresholds/metrics в asset — proposals. Это синтетический spatial sketch; не per-capability coverage, не authoritative HTML UX, не native viewport/camera implementation. Branch capture и финальная резолюция ждут живого обсуждения и не выполнены.

Source facts собраны отдельным read-only агентом по инструкции grilling:

- Native map dimensions: normal 66×22, big 66×44; legacy terminal padding даёт 80×24/46 (`src/common/defines.h:350–379`, `src/common/common.c:1025–1053`). Padding не переносится как single-window UI requirement.
- SDL3 resize debounce 500 ms (`src/client/main-sdl3.c:1311–1339,1407–1431`); main term resize меняет screen_wid/hgt и отправляет `Send_screen_dimensions` (`src/client/main-sdl3.c:4874–4910`, `src/client/nclient.c:8072–8076`). Это baseline facts, не утверждённый новый layout/debounce contract.
- HTML runtime panel defaults задаёт `app.js:37–45`, inline vars в `js/features/right-panel.js:3–10`; shell min-width1180 и center min480 сохраняются (`css/foundation.css:27–34,410–412`). Primary Inventory centered с width clamp520/46vw/720 (`css/windows.css:2–21`). Пути HTML относительно `/home/svechnik/Projects/github_site/tomenet_interface`.
- HTML map fitting сохраняет aspect ratio (`js/features/map.js:153–164`), mock zoom допускает 3…66×3…44 (`js/features/map.js:17–30,234–242`); arbitrary mock viewport не является поддерживаемым native protocol behavior.

Browser verification: A/B/C отрисованы при 1920×1080,1280×720,1024×768,800×600; surface extents находятся внутри scene. Document содержит 60 исходных synthetic строк; text24/map16×24 применяются независимо; draft сохранён. Визуально просмотрен screenshot центрального варианта. Проверена runnable mechanics, не usability/critical visibility parity: clipping received map при недостатке места намеренно остаётся нерешённым. Per-capability pages, special-store raw-picture geometry, pre-game/death/relogin, focus restoration и DPI raster ещё не покрыты.

Первый human decision round pending: смысл видимости карты при обычном primary, minimum supported logical window dimensions, logical/pixel scale contract. Рекомендуется центральное размещение в духе HTML UX с разрешённым overlap карты вне targeting, минимум1024×600 logical units и отдельные OS DPI/text/map scales без network viewport changes из-за primary. Предложения не считаются принятыми; после ответов recompute frontier и уточнить layouts/viewport fallback.

### First round — user answers

Пользователь: «1 - может, как в html ux, 2 - минимальный 1024x768, 3 - уточни».

- Q1 принят: обычный primary может перекрывать карту, включая позицию игрока, как в HTML UX. Критический HUD и временное скрытие primary при targeting следуют уже согласованной interaction model. Central HTML UX placement — рабочая основа дальнейших макетов; B/C не выбираются как обязательное резервирование карты.
- Q2: минимум исправлен пользователем на 1024×768. Вопрос был задан в logical units; конкретное значение этого условия при OS DPI >100% поясняется в Q3 и ещё не считается явно согласованным. Предложение1024×600 отклонено.
- Q3 требует пояснения: OS DPI, logical window units, user text/map sizes и network viewport policy не приняты автоматически. До ответа не фиксировать их как normative contract.

### Expanded DPI contract — confirmed by user

Пользователь принял пояснённый Q3 ответом «принимаю».

- Минимум1024×768 именно logical window units; при150% OS scale physical client area1536×1152. Window decoration не входит в эти client-area dimensions.
- OS DPI масштабирует UI и карту физически; user text/map sizes независимы. Text-only change не меняет map setting/grid definitions; map-only change не меняет UI text setting. Конкретное fit behavior и возможное косвенное изменение доступной map area уточняются следующим round, не выводятся автоматически из этой формулировки.
- Primary open/close не изменяет server-visible viewport; underlying received map сохраняется. Normal/big selection остаётся отдельным решением.
- Monitor DPI change пересчитывает physical layout/raster без потери draft/interaction. Minimum support относится к доступной logical client area; поведение при forced smaller window/effective area ещё обсуждается.

Следующий frontier: full-grid fitting vs crop/normal fallback при недостатке места; отдельно coordinate-preserving document/special-store fitting. Shell density/thresholds следуют после определения map fitting.

### Map fitting and special stores — user answers

Пользователь: «4 - принимаем , 5 - какие документы? специальные магазины - без прокрутки, уменьшаем чтобы вписать».

- Q4 принят полностью: вся selected normal/big сетка proportionally fits без map scroll/crop; desired map cell size — максимум, actual displayed size может уменьшаться при недостатке места. Resize не переключает normal/big автоматически; primary overlay не меняет server viewport. Text setting не меняет map setting, однако уменьшение доступной map area из-за HUD может косвенно уменьшить actual fitted cells.
- Q5 special-store часть принята: canvas, raw pictures и animations уменьшаются proportionally для полного вписывания, без horizontal/vertical scroll. Geometry сохраняет anchors/relative positions; actual pixel rounding/raster oracle belongs to raster/acceptance contract. Предложенные отдельные semantic controls ещё нужно показать в макетах; отсутствие scroll canvas подтверждено явно.
- Q5 documents часть pending: пользователь попросил примеры. Разъяснить SPECIAL_FILE_* server-formatted documents (server Help, scores, uniques/artifacts/players, log/deaths/settings и произвольный OTHER) отдельно от typed UI lists/local Guide. Server documents обычно page-loaded, поэтому vertical movement должен сохранять штатный server request/search semantics, а не предполагать, что весь файл уже локально загружен. Generic unknown-format raw view обязателен даже при additional semantic projections. Не считать document horizontal/vertical layout принятым до ответа.

### Formatted document layout — confirmed by user

Пользователь принял пояснённое правило Q5 ответом «принимаю»: server-formatted Help, scores, uniques/artifacts, deaths/log/news/settings и arbitrary OTHER documents сохраняют исходные строки/колонки без automatic wrapping. Horizontal scrolling открывает длинные строки; navigation/search сохраняют штатную page request semantics. Весь длинный документ не уменьшается до размеров primary. Typed UI lists и local Guide не подчиняются этому правилу автоматически; их concrete layouts ещё уточняются.

Spatial prototype обновлён: full received synthetic map grid fits proportionally (desired cell dimensions — max), special-store canvas fits без scroll. Browser check при1024×768/text24/map24×36: displayed map594×594 внутри map area1022×594; special-store canvas161.2×214.9 внутри content718×230.8, overflow hidden; draft retained. Numeric metrics illustrative, не native renderer evidence.

Следующий frontier: responsive shell/collapse thresholds и critical HUD composition; primary sizes/density, typed list overflow, child prompt fitting и pre-game/system layouts. Принятые central overlap, минимум1024×768 logical, DPI и map/document/store fitting не пересматриваются.

### Responsive shell round — clarification requested

Предложены Q6 automatic collapse bands: width≥1440 both side panels;1100–1439 right collapsed;1024–1099 both collapsed, compact critical HUD above map. Q7 compact HUD composition и Q8 typed-list wrapping/scroll/focus retention заданы как отдельные вопросы. Пользователь ответил только «6 - распиши подробнее»; Q6/Q7/Q8 не приняты. Width bands ещё proposals; окончательная связь с пользовательскими panel widths, text metrics и available map area требует уточнения. Нужно пояснить реальные contents collapsed panels и доступность baseline entry points без разрешения сменить surface при активном request.

### Automatic layout changes rejected by user

Пользователь уточнил: «автоматически компановку не меняем».

- Automatic responsive shell switching, width bands1440/1100 и automatic panel collapse отклонены. Resize/DPI change не переключает выбранную компоновку, не скрывает боковые панели и не переносит HUD между областями автоматически.
- Ранее согласованное map fitting остаётся; surface extent adaptation и scroll/fit content не означают смену shell composition. Q1 overlap, минимум1024×768 logical, independent scales и document/special-store contracts сохраняются.
- Q7 compact HUD и Q8 typed-list rules не приняты. Automatic narrow-only compact HUD больше не предполагается.
- Следующий вопрос: как выбранная неизменная shell composition размещается при1024×768 — constrained panel widths/metrics или явный пользовательский layout setting. Это ещё открытый decision; наличие fixed composition не задаёт automatically constant pixel widths либо whole-UI downscaling.

### Panel widths — confirmed by user

Пользователь ответил «да» на уточнённый Q6: resize сохраняет выбранные пользователем ширины боковых панелей, карте отдаётся оставшееся место. Ширины изменяются вручную, как в HTML UX. Open primary подгоняется под available area без потери input. Automatic shell switching/collapse отсутствуют.

Конкретные default/min/max panel widths, minimum central area и placement primary относительно HUD/side panels ещё не заданы этим ответом. При1024×768 нужно проверить согласованную неизменную shell без скрытого auto-collapse/auto-width change.

### Primary placement and typed lists — confirmed by user

Пользователь ответил «принимаю» на Q7/Q8:

- Primary может занимать место поверх некритических частей боковых панелей и карты, размещаясь относительно всего окна в духе HTML UX. Не ограничен central map column; critical HUD остаётся видимым и не перекрывается primary. Конкретные safe rectangle/default dimensions обсуждаются дальше.
- Typed UI lists разрешают wrapping длинных названий внутри row, сохраняя видимость slot/key, count/price. Vertical scroll; selected row stays visible after resize. Header/navigation/required actions находятся вне scrolling content. Server-formatted documents сохраняют accepted no-wrap/page/horizontal semantics.

Следующий frontier: stable critical HUD placement и overflow при выбранных text metrics; desired dimensions primary families; fitting дочерних prompts. Перечень capabilities/entry points сверяется read-only отдельно, native behavior не перепроектируется.

### Desired-size facts from HTML UX

Read-only extraction from current CSS (dimensions observed in code, not all human-approved individually): Inventory clamp520/46vw/720; Abilities max720 with height cap690/72vh; Character width min54vw/918,height65.78vh; Guide min720/76vw,height min680/78vh; Skills min82vw/1080,height min76vh/720; Spellbook max1120,height min76vh/720; Messages max1180×760 with viewport96 margins. Sources: `css/windows.css`, `abilities-window.css`, `character-window.css`, `guide-window.css`, `skills-window.css`, `spellbook-window.css`, `messages.css` in external HTML repo. Own prototype metrics are synthetic and don't silently supersede these UX defaults.

Manual widths added to spatial asset. Browser check: panels260/320 retain widths from1920×1080 to1024×768; map area shrinks1330→434; draft retained. Numeric widths are illustrative, not normative defaults/min/max. Primary still requires update to accepted whole-window safe placement before final evidence.

### Panel fitting, primary dimensions and child prompts — user answers

Пользователь: «9 - боковая панель не прокручивается, если не хватает места - уменьшаем визуально, 10 -ок, 11 - ок».

- Q9 предложение о прокрутке прочих sections боковой панели отклонено. Side panel не прокручивается; если места не хватает, содержимое visually уменьшается для fitting. Critical HUD не переносится автоматически и не перекрывается primary. Нужно уточнить применение к left/right panels и finite feed widget extent: полный message recall остаётся primary Messages, fitting не означает уменьшение всей накопленной истории.
- Q10 принят: existing HTML surfaces сохраняют individual desired dimensions; absent simple menus/lists/normal stores720×690, list+details1120×720, generic document1180×760 logical desired maxima. Frame clamps к available safe area без automatic internal columns→tabs conversion или primary text shrinking. Typed wrapping/scroll и accepted map/store fitting остаются. Side-panel visual shrink — explicit user override для panels, не blanket primary font shrinking.
- Q11 принят: child prompts поверх current primary в safe area без перекрытия HUD; question wrapping, editor/required controls visible, длинный список scrolls. Resize preserves draft/selection/active step; responses/Enter/Esc/restoration следуют baseline.

Следующий frontier: panel fitting scope и extreme manual-width constraints при minimum1024×768; local Guide/structured detail layouts и pregame/system placement. Закрытие тикета ждёт coverage geometry evidence и итогового human confirmation.

### Small-window composition — new user specification

Пользователь уточнил Q12: «при малых размерах правая панель скрывается»; карта переходит в режим с выключенным big_map (меньше по высоте, как current client); left panel visually уменьшается для fitting по высоте; часть блоков игрок может скрыть сам в settings; над картой блок всех игровых сообщений небольшой высоты, примерно5 строк.

- Это конкретная small composition, уточняющая предыдущие general rules. Для неё right panel hidden, normal server viewport66×22, left panel fits по высоте, aggregate live message feed above map. Game feedback доступен через эту постоянную короткую ленту; full Messages recall остаётся отдельным primary. Лента не становится второй независимой рабочей поверхностью.
- Normal-mode activation использует штатный big_map/screen dimensions path; произвольные mock viewport sizes не вводятся. Прежнее Q4 «resize не переключает normal/big» уточняется этой схемой; точный trigger ещё pending.
- Manual per-block hiding разрешено пользователем как settings customization. Automatic hiding дополнительных left blocks не принято. Automatic font setting changes не выводятся из visual fitting.
- Нужно уточнить trigger small composition: explicit manual setting, сохраняющий прежнее no-auto-layout rule, или automatic size-based exception. Async question задан сразу; ответ не предполагается.
- Q13 dynamic minimum-window formula и Q14 pregame/system предложение пользователь не принимал. Q13 reconsider после trigger/small layout; proposed map minimum396 не adopted и не является runtime readability evidence.

### Small spatial sketch — pending trigger

Prototype scheme selector добавлен как test control, не определение product trigger. При1024×768/left260/text16/map12×18 small sketch:66×22=1452 cells; right display none; left overflow hidden; live feed758×116 над map area758×546. Primary720×376.6 располагается относительно safe whole-window area справа от left HUD и ниже feed (x283,y268.7), не зажат в map-only column. Draft retained. Side contents визуально fits без scroll; конкретный full left block corpus ещё нужно отрисовать. Font/tile modes остаются synthetic; runtime viewport negotiation, message queues и macro routing не выполняются.

### Small composition trigger — confirmed by user

Пользователь: «да, но можно вывести игроку предложение об изменении компановки при обнаружении малого размера экрана».

- Small composition включается вручную; automatic switching не вводится. Клиент может обнаружить малый доступный размер и предложить смену, но без выбора игрока сохраняет текущие panels/layout/map mode.
- Small mode choice явно скрывает right panel, включает normal66×22, показывает five-line aggregate feed; left panel visually fits height. Full Messages primary остаётся отдельным recall.
- Detection threshold, nonmodal presentation, safe activation во время command/request и dismissal lifetime ещё нужно согласовать. Monitor physical resolution и current logical usable client area не считаются одним и тем же размером.

### Small-layout offer and return to wide — user answers

Пользователь: «15 - да, 16 - возвращаем big_map, 14 - подробнее».

- Q15 принят: detection по logical available client area; small suggestion при width≤1280 OR height≤800, nonmodal «Включить малую компоновку»/«Не сейчас», once per app launch, не забирает focus; active command/macro/server request откладывают предложение до ordinary game input. No choice → no layout/map changes. Threshold adopted as initial UX contract, не runtime legibility proof.
- Q16 исправлен: explicit manual return wide включает big_map66×44, возвращает right panel и убирает small live feed above map. Предложение оставить normal при возврате отклонено. Resize сам wide/small/map mode не переключает. Нужно учитывать baseline BIG_MAP availability gates без unsupported dimensions; их source policy не переопределяется.
- Q14 требует подробностей; pregame/death/relogin geometry ещё не принята. Разъяснить whole-window sequential states и baseline-owned paths без универсального retry/reconnect либо восстановления unfinished commands.

### Q14 clarification and supplied MOTD reference

Пользователь спросил, является ли «MOTD и вход в игру» экраном сразу после входа до карты, и приложил screenshot `/tmp/codex-clipboard-bLM595.png` с server welcome/local settings/colored text and ASCII decoration. Q14 не считается принятым этим вопросом.

Для screenshot content важны original line positions/colors/ASCII decoration; его нельзя автоматически считать ordinary reflowable login form. Точный baseline order relative to auth/character selection/play/map проверяется отдельным read-only agent. Геометрия whole-window MOTD (no-wrap/cell coordinates и fit/scroll) требует уточнения после factual answer; user image reference не заменяет raw setup bytes и glyph mappings.

### Supplied screenshot identified in native flow

Read-only code подтверждает setup MOTD: server source `news.txt` (`src/server/nserver.c:471–483`), display `show_motd` рисует23 строк из120-byte strides (`src/client/c-files.c:1915–1928`). Account auth/character overview после `Net_setup`/`Net_login` (`src/client/c-init.c:4349–4363`, `src/client/nclient.c:1590–1618`); optional creation `get_char_info` раньше MOTD (`src/client/c-init.c:4434–4441`). Display MOTD (`:4463–4465`) → any-key ack/network serviced (`src/client/c-files.c:1936–1941`) → clear/Net_start/PKT_PLAY (`src/client/c-init.c:4471–4476`, `src/client/nclient.c:1678`) → gameplay packets/map. `-m` skip и RETRY_LOGIN once-only behavior (`src/client/client.c:1570`, `src/client/c-init.c:4467–4468`) сохраняются. Q14 не делает MOTD обязательным при baseline skip.

### MOTD content ownership — factual clarification

Пользователь спросил: «контент этого экрана целиком задаётся сервером?» Read-only follow-up confirms yes: server reads its `lib/text/news.txt` into Setup.motd23×120-byte buffer (`src/server/nserver.c:468–480`); file preprocessing/color markers (`src/server/util.c:412–461`); sends bytes (`src/server/nserver.c:2295–2296`); client copies bytes (`src/client/nclient.c:1369–1376`) and renders without extra heading/Press-any-key label (`src/client/c-files.c:1915–1941`). Welcome/links/settings prose/decorations are server-authored, not typed independent UI fields. Client owns font/palette/rendering and any-key acknowledgement. This clarification does not approve the pending MOTD proportional-fit proposal or complete Q14.

### Sequential screens and separate MOTD prototype — accepted by user

Пользователь: «тогда принимаю, уточняю, что для этого экрана будет сделан отдельный прототип».

- Q14 sequential whole-window scenes принимаются: server/connect, account/character overview/reorder, creation, setup MOTD, gameplay, death/end/relogin по baseline. Forms normal width, clamp client area, long forms scroll, required controls visible, no automatic form font shrinking. Session-ending events не восстанавливают старые commands и не вводят universal Retry.
- MOTD отображение принято после content-ownership clarification: full-window server-defined formatted scene, original rows/colors/ASCII layout retained, no automatic line wrap, proportionally shrink when needed; acknowledgement/skip/once-only baseline unchanged.
- Конкретный MOTD UI будет согласован отдельным prototype ticket. Current layout sketch не заявляет MOTD UX approval, missing MOTD layout не маскируется scripted screenshot recreation. User reference `/tmp/codex-clipboard-bLM595.png` — пример source content; fixture/reference capture принадлежит новому тикету.

### Final geometry evidence assembled

[Surface coverage / source owners](../layout-coverage.md) maps all36 baseline families and scoped/delegated prompt/input contexts to primary/child/canvas/source/form/target patterns. Geometry asset remains synthetic; atomic manifest/ledger and native behaviours not implemented.240 browser case pass covers20 surfaces×3 dimensions×2 layouts×text12/24; detail-pane shape/focus/draft/child/manual offer paths checked separately, console errors0.

Full-column screenshots: [Small1024×768](../layout-small-evidence.png), [Wide1920×1080](../layout-wide-evidence.png). Prototype selectors are test controls; forced transitions don't stand in for source router/gates. Final human confirmation pending, status remains claimed. Existing HTML individual sizes and new720/1120/1180 caps are source/confirmed policy; gallery pane fixtures not a claim that every cap-specific UX has been approved.

Remaining final-confirmation detail: readable local Guide/lore/details use applicable approved HTML UX, original/unknown source content remains available through source-preserving no-wrap viewer with local navigation/search ownership. Five-line small feed shows aggregate recent displayed lines (including chat), not whole accumulated history; wrapping raw message text doesn't affect server document line/column identity. These interpretations need final shared-understanding acceptance before resolution.

### Final confirmation — 2026-09-16

Пользователь ответил «принимаю» на Q17/Q18 и подтвердил итоговое общее понимание. Wide right-panel policy, local content policy и закрытие тикета приняты. Ниже находится каноническая резолюция; исторические предложения в Comments, которые ей противоречат, не являются действующими решениями.

## Answer

### Окно, единицы и масштаб

- Клиент использует один `SDL_Window`. Минимальная поддерживаемая client area — **1024×768 logical units**; OS DPI переводит logical units в surface pixels. Window decoration не входит в размер client area.
- Text font/UI scale и map font/cell scale независимы. Text-only change не меняет map setting/visual definitions; map-only change не меняет UI text. DPI/monitor change пересчитывает raster/layout без потери текущего interaction state.
- Карта всегда сохраняет полный выбранный server viewport: normal **66×22** или big **66×44**. Desired cell size — максимум; вся сетка proportionally fits с сохранением aspect ratio, без crop и map scroll. Primary open/close не меняет viewport.
- Точный glyph/raster reference, minimum legibility/profile bounds и rounding относятся к raster/acceptance owners. Предложенные в обсуждении dynamic minimum-window formula и minimum map width396 не приняты.

### Две выбираемые компоновки

**Wide layout** сохраняет left panel, map и right panel. Пользователь вручную меняет ширины panels; resize и DPI сами их не меняют и не перестраивают shell. Обычный primary может перекрывать карту и некритические части panels, но не видимый critical HUD. При явном возврате из small layout включается big_map66×44, возвращается right panel и убирается small live feed.

**Small layout** выбирается пользователем вручную. Он скрывает right panel, включает normal map66×22, visually fits left panel по высоте без outer scroll и показывает над картой общую ленту последних примерно пяти отображаемых строк всех игровых сообщений, включая chat. Это краткий live projection; полный recall остаётся Messages primary. Игрок может вручную скрывать отдельные left-panel blocks через settings. Компоновка не переключается автоматически.

При logical width≤1280 или height≤800 клиент один раз за app launch ненавязчиво предлагает small layout: «Включить малую компоновку» / «Не сейчас». Предложение не получает focus и не меняет state самостоятельно; active command, macro или server request откладывает его до ordinary game input. Manual Settings сохраняет тот же input/request contract. Persistence, defaults/panel constraints и startup precedence между SV layout preset и shared `CO_BIGMAP` принадлежат [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md).

В wide layout right panel сохраняет выбранные виджеты и размещение. Panel shell целиком не прокручивается и visually fits при нехватке высоты; списки внутри widgets могут прокручиваться. В small layout right panel hidden, но все его outcomes имеют primary entry points. Messages является primary, независимая раскрываемая feed surface не вводится.

### Primary surfaces и content rules

Одновременно открыт один primary. Он центрируется в safe area всего окна, может перекрывать карту/некритические данные и не ограничивается central map column. Resize clamps frame без закрытия interaction, сброса selection, draft, scroll/position или focus owner. После завершённого interaction раздел повторно инициализируется по baseline, без нового cross-open recall.

Existing HTML surfaces сохраняют свои подтверждённые desired sizes. Для новых surfaces применяются logical desired maxima:

| Семейство | Desired maximum |
|---|---:|
| Простые меню, typed lists, normal store |720×690 |
| List/tree + details, multi-column pages |1120×720 |
| Generic formatted document viewer |1180×760 |

Frame всегда clamps к safe area. Внутренние колонки автоматически не превращаются в tabs, primary text не уменьшается; overflowing region получает собственный scroll согласно типу содержимого.

- **Typed lists**: длинное имя wraps, semantic key/slot, count и price остаются видимыми; body scrolls vertically. Header, navigation и required actions pinned outside body. Selected row/focus остаётся видимым после resize.
- **Detail/Guide/lore**: применимый approved HTML UX даёт readable wrapped article/detail projection. Original source lines/markers и unknown content сохраняются и доступны через source-preserving no-wrap viewer; local search/bookmarks/navigation остаются у исходного owner.
- **Server-formatted documents** (`SPECIAL_FILE_*`) сохраняют original bytes, colors, rows/columns и unknown content. Automatic wrap отсутствует; horizontal scroll открывает wide lines, vertical navigation/search сохраняет штатные server page requests. Длинный документ не shrink-to-one-screen.
- **Special stores**: text/grid/raw pictures/animations/clear regions живут в одном reference canvas и проходят один uniform transform, complete fit без scroll. Text/tile anchors используют cell coordinates; raw picture extents могут занимать несколько или дробное число cells и не сжимаются в one-cell slot. Controls/prompts находятся вне canvas. Exact resource/profile/rounding oracle остаётся renderer/raster owner.
- **Map targeting** временно скрывает перекрывающий primary по принятой interaction model; HUD/live feed остаются. Finish/cancel/retry и возврат selection определяются baseline command, не layout.

### Children, focus и ввод

Quantity/text/name/password/key/confirm/item/source/slot/direction/target и packet-driven requests — children owning interaction. Child размещается поверх current primary в safe area; question wraps, editor/required controls остаются visible, long choice list scrolls. Resize сохраняет draft, selection, caret/focus и active step. Enter/Esc/answers/abort/restoration следуют конкретному baseline context; layout не вводит универсальный cancel/retry и не исполняет command повторно.

Input router/macros/chat сохраняют решения [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md). Mouse entry points разрешены как дополнительные bindings, но не заменяют keyboard/macro routes. ordinary primary или layout offer не расширяют availability chat/settings во время запроса.

### Pregame, MOTD и session end

Connect/server, account/character overview/reorder, creation, setup MOTD, gameplay и death/end/relogin — sequential whole-window scenes в baseline order. До active gameplay игровой HUD отсутствует; session teardown закрывает старые interactions. Long forms scroll с visible required controls, без automatic form-font shrinking. EOF/quit/death/relogin дают только штатные paths; universal reconnect/resume отсутствует.

Setup MOTD показывается после character selection/creation и до `Net_start`/карты. Его23×120-byte server-defined rows, colors и ASCII layout сохраняются без wrap и proportionally fit complete scene; any-key, `-m` skip и once-per-login behavior остаются baseline. Concrete UX вынесен в [Prototype the server-defined MOTD screen](28-prototype-the-server-defined-motd-screen.md), с [reference screenshot](../motd-reference.png).

### Coverage и проверка planning artifact

[Surface coverage and geometry evidence](../layout-coverage.md) распределяет все36 baseline family IDs и scoped/delegated prompts между persistent HUD/map, primary, child, source viewer, special canvas, target и whole-window form/system scenes. Это allocation для implementation spec, не runtime parity claim.

[Layout prototype](../layout-prototype.html) — throwaway geometry fixture. Browser pass:240 combinations (20 surfaces ×3 dimensions ×2 layouts × text12/24) без frame/map-fit/cell-count/default-draft failures. Отдельно проверены2/3-pane layouts при1024/1920, selected-row visibility/focus, form and child drafts, deferred offer→small22 rows/right hidden и explicit wide→44 rows. Console errors0. Captures: [small1024×768](../layout-small-evidence.png), [wide1920×1080](../layout-wide-evidence.png).

### Cross-ticket scaling amendment

[Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md) уточняет прежнюю формулировку `desired cell size — максимум`: полный viewport сначала получает maximum uniform fit в allocated map area, затем независимый user map fit zoom может только уменьшить и центрировать composition. Font/profile задаёт внутреннюю cell geometry и aspect ratio; zoom не меняет normal/big viewport или layout mode.

Последующее решение «Prepared assets and 1:1 composition» в том же тикете определяет вывод после fit/zoom: общий целочисленный final cell size, центрирующие поля от округления и подготовленные assets с композицией 1:1. Постоянно растягиваемый промежуточный raster canvas больше не является требованием layout; временное масштабирование допускается в live resize по контракту raster ticket.

Synthetic rows/forms/maps не являются capability implementation, native input/network proof или raster oracle. Atomic manifest/ledger evidence, exact field encoding, retention, persistence registry/defaults, reference raster profiles and platform acceptance остаются у существующих linked tickets. Runtime client и release artifacts не создавались.

Primary-source capture сохранён вне рабочей ветки: `prototype/single-window-surface-layouts`, commit `13b1f40fad2031cf1cf4a118c23904d56dffd303`. Commit содержит только layout prototype, coverage ledger, wide/small evidence captures и supplied MOTD reference; текущая ветка и пользовательский index не менялись.

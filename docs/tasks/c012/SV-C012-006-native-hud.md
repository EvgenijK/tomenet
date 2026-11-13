# SV-C012-006 — production caller и читаемый native HUD

Статус: **accepted — Linux status/core**, 2026-11-13, по решению пользователя.

[Решение о приёмке](../../sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются отложенными.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

Исторический статус до финальной проверки: **приёмка интерфейса отложена; собственные core/semantic обязательства C012 открыты**, 2026-11-13.

## История решения об отсрочке приёмки интерфейса — 2026-11-13

**Интерфейс в C012 больше не дорабатываем; его дальнейшая визуальная/пользовательская приёмка отложена.** [Реестр C012-DA-01–10](deferred-acceptance.md) задаёт принимающие этапы, условия закрытия и взаимные ссылки. Прежние visual correction gates ниже сохраняются как история; они не требуют продолжать оформление или снимки в C012 сейчас. Отсрочка не означает PASS, завершение C012 или перенос canonical 27/81.

В C012 остаются финальный минимальный existing focused core check последнего implementation candidate, semantic contract Study/stat index, сохранность данных/событий и точный disposition обязательств. Техническую часть можно завершать без ожидания поздней визуальной приёмки. Последний existing-slots freeze3 содержит исправления stat/encumbrance/Sanity/XP и resize/reflow; его финальные core и affected visual проверки после команды «стоп» не завершены. Старые snapshot PASS не подтверждают этот финальный кандидат. Новых блоков Character не добавлять; RESISTS unknown/DUMP unavailable остаются согласованными.

## История результата до отсрочки

Исторический статус: **Character visual/input correction checked in wide layout; C012 UI completeness open**, 2026-11-12. Пользовательская приёмка Character ожидается; общее окно ранее принято.
Labels: bug

Зависимости: 001–005. Cross-cutting consumer всех 27 outcomes, не второе владение IDs.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Предыдущий список «пять отсутствующих видимых статусов и все rawfields» был слишком широким. Показ реальных HTML снимков пользователю выполнен **до** этой корректировки задач. Источник HTML не менялся; эта волна меняет только план/документы, не production. Запрещены новый StatusDetails, новые блоки в Character и самовольно назначенные похожие иконки. Frozen5 visual/input PASS остаётся ограниченным проверенным wide состоянием; найденные ниже semantic gaps не закрыты им.

[Baseline/scope аудит](../../../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и реальные снимки](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md), [адресный план](../../../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md).

| Остаток | Класс и следующая работа |
|---|---|
| STAT-COLOR | Доказанный semantic defect существующих stat строк: drain>boost>normal base117/118; связанный owner003 |
| ENC-PRECEDENCE | Доказанный semantic defect существующего weapon-слота: easy>heavy>awkward; owner005 |
| Sanity current/max | Подтверждён существующий display mode exact/bar; проверить фактический SV режим/availability, без нового блока |
| Stats conditional max, level suffix, fractional XP | Проверить baseline-derived результат в существующих stat/Level/XP строках; raw поля не превращать в отдельные labels автоматически |
| Study/self AFK/opaque extra | Подтверждённых мест HTML нет; study дополнительно имеет canonical count/availability mismatch. Читаемые требования/mapping остаются open |
| Bank/misc combat | Отдельный readable requirement/owner и место не закрыты; bank baseline Merchant Guild conditional, не Character |
| Martyr/idle | State/message либо auto-mute/message, а не обязательные новые HUD labels. Сохранение delivery и выполнение позднего recipient различать |
| Unknown indicators/Probability Travel | Unknown u32 сохранить без обязательной hex-строки; known Probability Travel mapping ещё open |

Существующие места можно увидеть на реальных снимках: [Sanity exact](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/05-sanity-exact-region.png), [12encumbrance slots](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/06-encumbrance-existing-slots-region.png), [status grid](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/02-status-grid-region.png), [целый HUD](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/01-hud-default-whole.png). Эти снимки показывают существующий UI, а не расположение отсутствующих selfAFK/study/extra/bank. HTML исходники не менялись.

Следующий порядок: подтвердить source/contract dispositions → адресные существующие stat/weapon fixes → согласованное использование existing modes/conditional formats → необходимые scoped checks007. Если existing места нет, не начинать зависимое размещение до решения пользователя. **Новых блоков/списков в Character не делать.** Реализация этими planning edits не запускается.

## Текущий результат исправлений Character — freeze5, 2026-11-12

Исправлены реальные PROFILE/SKILLS/RESISTS: выбор страницы, active state, композиция, компактная высота RESISTS, связанный правый виджет, close/reopen retention и ограниченная фактическим содержимым прокрутка. Исправлены тон панели/шапки, подпись Level, заголовки и формат Blows/Round. Формы14 source-иконок перенесены из HTML; обнаруженный дефект blur-only исправлен чётким SVG foreground.

Пользователь одобрил RESISTS с неизвестными значениями и явно недоступный DUMP. Предложение `+ STATUS DETAILS` **отклонено** после показа снимков: переключатель, список и связанные state/hit regions полностью удалены. В SKILLS только секции макета; новый заменяющий блок не добавлен. Старые proposal/freeze3/freeze4 снимки исторические.

Независимая реальная проверка при1932×1208: три HTML/SV пары, переключения, DUMP unavailable notice, закрытие/повторное открытие, widget sync, fitting wheel/PageDown — без новых блокирующих presentation/input findings в этом состоянии. Native838frames/22PNG/exit0, frozen source/binary стабильны. [Отчёт и границы проверки](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/visual-review.md); пары: [Profile](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-profile.png), [Skills](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-skills.png), [Resists](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-resists.png).

Linux build и существующий focused production status check — exit0; [команды/логи/SHA](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/checks/final-checks.json). HUD SHA `56092ac705846c33e4ffb5d4350e9542b9f73861d323965f6a57238706526a91`, binary `710bd1d4679221b9b2c4ee598a22acb3a6e015d65d50c5a5d1c3f07d332c1fe6`. Обновлён существующий code graph:29331nodes/72309edges/1475communities, exit0. Новая suite/harness не добавлялась.

**Остаток C012 UI:** после удаления дополнительного списка нет самостоятельного видимого вывода study,AFK,extra,martyr,audio-idle; также больше не показан ряд расширенных значений bank/attributes/XP/sanity/concurrent encumbrance. Backend/derived rows сохранены, но это не видимое UI. [Точный список и семантические исключения](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/implementation.md). Нельзя объявлять все27outcomes/81obligations или весь Character принятыми. Canonical coverage не изменён; пользовательская приёмка остаётся открытой. Это не разрешение вернуть технический список в Character. Полные RESISTS data/export, live session, small paired capture и temporal QA текущим visual review не подтверждены.

## Исходная корректировка Character после пользовательской проверки — 2026-11-12

Пользователь принял общее окно и не принял Character. Прежний общий V3/appearance PASS отозван: были пропущены собственные расхождения Profile, а остальные страницы не открывались. [План исправлений006-C1–C9 и повторной проверки007-C1–C2](../../../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/correction-plan.md), [первичный списокCH-01…09](../../../.scratch/sv-c012/browser-visual/character-discrepancies-2026-11-12.md).

Новые реальные клики HTML/SV подтвердили: SKILLS/RESISTS в SV не переключаются, active PROFILE жёстко задан, DUMP инертен. В HTML SKILLS сохраняет общий верх и показывает DETAILS (Age/Height/Weight/SocialClass уже есть в C012 model; Sex пока недоступен); RESISTS имеет отдельные матрицы и компактное окно918×387.36@y410.31 вместоProfile 918×794.61@y206.69. Достаточно нарисованного tabstrip для приёмки нет.

Порядок: (1) field→slot и место supplementalC012 вне подменыBackground, (2) selected-tab/hit/active-state, (3) PROFILE/SKILLS/RESISTS-specific композиция/высота, (4) честный контрактDUMP без prototype-success, (5) CH-03…09 оформление/семантические роли, (6) close/reopen/widget-state/scroll/bindings, (7) frozen paired evidence каждой страницы и независимый review. Полный C030 не назначается автоматически; его data/actions остаются открытыми requirements для приёмки целогоCharacter, а не оправданием визуальногоPASS. Узкие вопросы о supplemental placement и DUMP/RESISTS scope записаны в плане; остальные адресные исправления готовы к работе.


## Текущий контракт: сравнение с реальным браузерным рендером — 2026-11-12

Предыдущие проверки R1–R4 подтверждали отдельные свойства production renderer, SVG и состояний, но **не соответствие интерфейса целиком отрисованному HTML**. Appearance-часть006/007 повторно открыта по запросу пользователя. Core001–005 и фактически выполненные R1–R4 checks сохраняются в своих границах; прежние формулировки о завершённой независимой визуальной проверке не являются текущим appearance PASS.

Локальный HTML теперь доступен в браузере через `http://127.0.0.1:8765/` (каталог `/home/svechnik/Projects/github_site/tomenet_interface`). Root подтвердил загрузку и реальный screenshot. Прежние записи о невозможности localhost относятся к истории и не ограничивают текущую проверку. Устаревший удалённый `reference-full.png` не использовать.

Последовательность: paired before captures → конкретный список расхождений и исправление задач → SV-local implementation → frozen paired after captures → независимый whole-frame visual review. Чтение DOM/CSS/SVG дополняет браузерный рендер, но не заменяет его.

| Пункт | Критерий исправления |
|---|---|
| **006-V1: геометрия и композиция HUD** | В normal wide состоянии сверить целый кадр и C012-области: ширины выбранных панелей, отступы, размеры/плотность карточек, вертикальные позиции, статусную сетку и применимый shortcut. Начальные defaults/selected widths, manual layout и сохранение выбора при resize/DPI остаются принятым контрактом; нельзя непрерывно менять ширины только ради совпадения одного кадра. Конкретные discrepancies фиксируются только после просмотра пары одинакового размера. |
| **006-V2: оформление и текст** | По реальной паре проверить и исправить применимые font roles/size/line-height, цвет/контраст, фон/градиенты, рамки/разделители, иконки и эффекты. Сопоставить браузерный effective font с выбранным SV text font. Пользователь подтвердил2026-11-12: UI font может отличаться от HTML, текущий CascadiaMono допустим; map font независим и не меняется. Font-family difference — согласованное исключение, geometry/size/line-height/density остаются в проверке. Не добавлять serif asset только ради HTML и не обходить выбранную UI-настройку. Состояния/числа и отсутствующие packet data не подменять ради appearance. |
| **006-V3: Character primary** | Отдельная пара с открытым `#characterWindowOverlay` и SV Character primary проверяет реальную композицию primary: положение и габариты, заголовок/закрытие, оформление, читаемость status-only content, scroll и backdrop. Правую Character widget и центральный Character primary нельзя сравнивать как один и тот же элемент. Подтверждённое правило видимости critical HUD имеет приоритет над буквальным перекрытием/затемнением в demo; существенный конфликт обозначить явно. Full C030 skills/resists/export и неподдерживаемые действия не имитировать. |
| **006-V4: согласованное состояние и сохранность данных** | Фиксировать одинаковые client viewport dimensions, CSS/logical vs physical scale, DPR/OS DPI, UI scale, layout, selected widths, fonts, active conditions/indicators/encumbrance, открытый primary и scroll. Все27 outcomes/81 obligations, availability и baseline meaning сохраняются. При различающихся scalar values проверять типографику/выравнивание с учётом длины; не выдавать такую пару за полностью идентичное состояние. |

Точные source/capture paths, исходные findings и их disposition ведутся в007/evidence. Пустая map area и отсутствующие live messages/inventory/bags — отдельные C013/C014/C015/C018/C020, полное character information — C030; они отмечаются на whole-frame comparison, но не заполняются фиктивными demo-данными. Внешний вид собственной status projection/Character primary остаётся обязанностью006 сейчас.

Исполнители исправляют подтверждённые visual findings до финального freeze. Наличие полного кадра, работающего build или одинакового SVG hash само по себе не закрывает V1–V4. Финальное appearance решение принимает независимый reviewer по снимкам **обоих** вариантов; пользовательская приёмка остаётся отдельной.

### Findings по реальным before кадрам — 2026-11-12

Просмотрены целиком `.scratch/sv-c012/browser-visual/html-wide-1932.jpg`, `html-character-1932.jpg` и `.scratch/sv-c012/rework/native-fullscreen-20261112.png`. Browser metrics сохранены в `html-wide-metrics.json`/`html-character-metrics.json`. Это actual1932×1208 browser/native pair, а не реконструкция макета по CSS. Native primary before снимается отдельно.

| Finding | Наблюдение и адресный план |
|---|---|
| **V1-a, origin/vertical rhythm** | Свежезагруженный HTML имеет selected default widths370/556, как SV; постоянного расхождения panel widths эта пара не показывает. HTML `levelCard` starts14,12, SV inset13,11: учесть1px panel border. HTML Ping row занимает43px (`y142..185`), в native отсутствует, поэтому stats и последующие блоки выше. Не копировать demo193ms: использовать existing production network data, если доступно, либо явно unavailable slot; C008 semantics не изобретать. |
| **V1-b, card extents** | Browser vitals block119px (`y364..483`), native вычисляет128px; после исходного Ping offset это даёт дополнительный сдвиг статусов. Сверить фактические resource extents/padding, browser status header y590 и grid y615, не компенсировать одним произвольным global offset. |
| **V2-a, typography** | Browser labels используют serif (`Georgia, Times New Roman`), numeric values — monospace; SV выбранный CascadiaMono применяется шире. Это реальное whole-frame отличие, для которого пользователь2026-11-12 согласовал исключение: UI font независим от map font, может отличаться от HTML; текущий CascadiaMono допустим. Не добавлять HTML serif asset и не менять map font. Geometry/size/line-height/density, frame/inset/icon/opacity остаются в scope после alignment. |
| **V3-a, primary identity/composition** | Browser Character primary находится x507,y206.6875,w918,h794.609375, head38px, tabstrip/close и backdrop blur1.5/saturate.72/dim.76. До этого показывавшийся native fullscreen имеет только правый Character widget; он не подтверждает соответствие primary. Нужна собственная before/after primary пара; full-window centered layout и применение backdrop должны учитывать видимый critical HUD. |
| **V4-a, known state/scope differences** | Raw native fixture имеет level20/XP120000/gold1234/HP75/100 и часть отсутствующих stats, browser demo level57/XP18432000/gold952358/HP1912/1912. Matched active icons не делают scalar values identical. Browser Ping, stance/dual controls, шесть shortcut controls, map/right messages/inventory/bags и full Character lore/skills отсутствуют либо принадлежат иным owners: отметить явно. Native supplemental27 status fields сохранить читаемыми, не заменять synthetic C030 lore. |

Нормативная closure проверяет применимый C012 результат после исправления, а не заявляет идентичность deferred surfaces или разного fixture content. Все active findings направлены implementation owner до final freeze.

## Исторический контракт корректировки R1–R4


Статус этой корректировки: **implemented; frozen-source Linux evidence checked**. Прежние заявления appearance PASS ниже — история; актуальны этот раздел и последующая проверка 007. Новых ticket IDs и изменений canonical allocation не требуется. Источник внешнего вида — текущие `index.html`, final CSS cascade, JS layout/state presentation и исходные SVG в `/home/svechnik/Projects/github_site/tomenet_interface`. Удалённый устаревший PNG не является источником; его нельзя восстанавливать ради сверки.

Приоритет источников: подтверждённые layout/session policies и behavior baseline сохраняют силу. HTML определяет применимый внешний вид; synthetic values/симуляция не подменяют значения пакетов. В частности, [item-policy](../../capabilities/item-policy.md) и принятое решение issue21 сохраняют выбранные logical panel widths при resize/DPI и manual wide/small choice.

| Исправление | Проверяемый результат |
|---|---|
| **006-R1: widths и fitting** | Убрать произвольные постоянные318/486. При отсутствии выбранной ширины использовать HTML **initial default**: `layoutScale=clamp(initial logical width/1672,.82,1.18)`, `round(320*layoutScale)`/`round(481*layoutScale)`. Fresh1280×800→262/394 logical; fresh1920×1080→367/552. Вычисление defaults выполняется один раз при initial selection/reset, не каждый draw/resize. Existing caller/profile selections имеют приоритет; rebuild/resize/DPI не сбрасывают selected logical widths. Не вводить automatic layout switch/collapse или новый width editor C046. Small left fit считать из фактических extents текущих видимых блоков/иконок/spacing и available height, а не фиксированного820; primary сохраняет requested text scale/scroll. |
| **006-R2: IDLE и activity** | Удалить `PKT_IDLE → Resting/REST` mapping. Baseline `Receive_idle` означает audio auto-mute idle, не gameplay rest/search/paralysis. Idle остаётся отдельным читаемым status field и occurrence metadata; поздние audio recipients не исполняются draw. Activity tile не показывает активную луну без соответствующего authoritative gameplay state; отсутствующий в C012 source обозначается unavailable. Не добавлять фиктивный Resting и не тянуть чужие packets/capabilities ради макета. |
| **006-R3: presentation palette** | Для conditions без переданного server attribute брать цвета соответствующего semantic state из актуальных `CONDITION_DEFINITIONS`, включая Blind `#e08137`, Blind+Hallu `#ee6246`, Hallucinating `#e4c641`, Confusion/Fear `#df8436`. Проверить также применимые Food/Poison/Wounds/Stun variants из того же source. Raw values/defaults/thresholds и meaning severity сохраняются по baseline. Sanity/blows и другие explicit authoritative attributes сохраняют свои semantic colour roles; их нельзя безусловно перекрасить synthetic palette. |
| **006-R4: final-size icons и filters** | Exact source SVG primitives сохраняются. Исправить raster path, где одна96px mask уменьшается SDL linear до15/18/20px с потерей stroke coverage. Подготовка/cache должны учитывать фактические physical target dimensions, scale и source generation; не допускать повторного грубого downscale готовой маски. CSS drop-shadow применять как blur/offset/alpha effect, не как восемь смещённых резких копий glyph. Не добавлять runtime dependencies без необходимости; existing source-derived assets допустимы. Cache lifecycle/eviction и colour/alpha modulation остаются renderer-owned. |

Полнота исходных27 outcomes сохраняется: Character supplemental fields, raw unknown bits/source spans, missing availability и поколения не теряются из-за визуальной коррекции. Geometry/source SHA equality не равны final-render PASS. Selected text font сохраняется; SVG/runtime comparison отделяется от browser pixel identity, которая не является контрактом.

### Сценарий одинаковых состояний для визуальной сверки

Для appearance comparison фиксировать viewport, logical/physical scale, effective selected font, layout, chosen widths, bar/huge options и semantic values. Нужен bounded profile с одинаковыми active states по обе стороны: например, только `heavy_wield` в encumbrance, остальные flags clear; одинаковый набор indicator bits и conditions. Existing all15-active fixture сохраняется как stress/core evidence, но не сравнивается с HTML default heavy_wield как будто состояния одинаковы. Fixtures доставляют raw bytes через existing production app→protocol→model→renderer; прямое присваивание semantic model запрещено. Отдельно проверить IDLE1 при отсутствии rest source: луна не становится активной. Не объявлять browser-rendered comparison, если reference получен только чтением DOM/CSS/SVG.

Эта задача готовит исправление; статус implemented/appearance checked возвращается только после frozen-source evidence 007 и независимого review всех пунктов R1–R4.

## Работа и критерий готовности

1. Подключить production status decoding → retained session model → read-only view → native renderer. Все 27 outcomes должны иметь читаемую проекцию; framing, synthetic fixtures или прямое присваивание model не заменяют работающий caller. Одного API будущему C014 недостаточно для завершения этой задачи.
2. Использовать утверждённые gameplay UX решения из item-policy/session-policy и применимый HTML reference. Synthetic docs/sv-shell.md не является финальным gameplay UX oracle. Не вводить automatic layout switch/collapse, новый primary или editor options C046. Существующий Character primary используется для status-only read subset; full C030 не переносится. Critical HUD не закрывается обычным primary; text/UI и map scales независимы.
3. Сохранить применимые numeric/bar/huge-bar preferences и их layout eligibility в соответствующих HUD elements. Учитывать read-only availability: неизвестное значение не превращать в авторитетный ноль. Отдельно отобразить player/monster HP, gold/bank и значения боевых бонусов.
4. Resize/focus/font/scale/redraw перестраивают только projection и не повторяют packets, startup effects или commands. Нельзя читать legacy terminal buffer ради HUD.
5. До active gameplay игровой HUD отсутствует. Для bounded native review можно использовать существующий явно development caller, не менять accepted pregame scenes и не объявлять enter-game. Полный socket adoption/close/relog — C014; требуемый status consumer должен быть реально вызываемым уже в C012, затем подключаться без второй реализации.

## UX reference

Gameplay HTML: `/home/svechnik/Projects/github_site/tomenet_interface/index.html`, стили `css/hud.css` того же проекта. Relevant selectors: `#hpText`, `#hpFill`, `#manaResource`, `#sanityArmorCard`, `#statusIconGrid`, `#characterWidget`, `#characterContent`. `app.js` задаёт synthetic demo behavior, не игровую семантику. При реализации зафиксировать SHA актуальных reference files и сверить с финальными policy решениями. Использовать `#characterShortcut` → `#characterWindowOverlay`/`#characterWindowContent` для status-only read subset, включая small layout со скрытой правой панелью. Полный `information.read-sheet`, skills/resists/Guide/export и другие C030 capabilities этим subset не принимаются.

## Production entry и доступность status данных

HUD renderer и semantic projection реализуются в `src/client/sv/ui/`, не в test-only UI. Существующий ограниченный native diagnostic entry — `tests/sv/diagnostics/launcher.c:90` (`--synthetic`/`--hp-check`), его draw caller — `tests/sv/diagnostics/ui.c:72`. Пример доставки packet bytes production-путём — `tests/sv/scenarios/hp-scenario.c:12`: `sv_app_receive` → `sv_app_step` → view. Допустимо узко расширить этот caller/fixtures для вызова production HUD renderer, без нового simulator/harness и второй реализации renderer для C014.

Демонстрационный caller подтверждает production status path, но не actual socket→live adoption. C011 сохраняет identity boundary; её снятие и полный live stream не требуются 006.

| Owned группа | Читаемое представление |
|---|---|
| 002: vitals | Critical HUD resources; полные значения/labels также в применимом status-only Character content |
| 003: character values | Существующий Character panel и тот же read-only status content через Character primary |
| 004: conditions | HUD status icons/labels, численные значения/severity доступны в status read projection |
| 005: extended status | Соответствующие HUD/encumbrance/tracked-health элементы; opaque text/дополнительные значения — readable status projection |

Все 27 outcomes доступны пользователю и в small layout: перенос status-only read content существующего Character primary и независимый от right panel `#characterShortcut` входят в 006. Не создавать full character information model/actions C030; не считать наличие полей в C snapshot доказательством пользовательской читаемости. Использовать существующий input-router/primary contract; не разрешать открытие Character поверх активного server request вопреки baseline.

Дополнительные UX sources в том же HTML проекте: `js/features/character.js` (динамический content), `js/features/character-window.js` (existing primary entry), `js/features/hud.js` и `css/character-window.css`. Зафиксировать актуальные SHA при реализации. Synthetic JS values, XP formulas и sanity labels не заменяют authoritative decoded baseline values.

## Необходимый минимум проверки

Focused native/core caller с production packets показывает updates всех групп 002–005, coherent projection, partial packet без публикации, resize/redraw без replay и generation reset. Существующий Linux native путь использовать для ограниченного просмотра читаемости; не вводить новую полную visual/input/provider matrix. Отдельно зафиксировать, что checked native consumer не доказывает actual C014 lifecycle. Если конкретный required UX choice отсутствует в утверждённых источниках, представить пользователю узкий вопрос до его реализации.

## Общие границы

[План](README.md), [source contract](source-contract.md) и [точные canonical записи](coverage.json) обязательны. Реализация SV-local; legacy/common менять только при необходимости текущей задачи. Модель/decoder не читает Term; UI не читает wire packets/legacy globals. Нужны атомарные complete-packet updates, baseline defaults, generation/reset и read-only views с revisions. Framing и test-only реализация поведения не заменяют production путь.

Проверки — только необходимый минимум Linux core production-пути, совместно с 007. Не создавать broad suite, новый simulator/harness или полную visual/provider matrix; расширение согласовать с пользователем. Windows/MinGW/Wine отложены. Canonical obligations/statuses сохраняются; actual adoption/close/relog проверяет C014. Собственный результат задачи нельзя заменить обещанием будущего C014 consumer.


## Реализация и проверка — 2026-11-11

Production `ui/hud.[ch]` подготавливает copied read-only rows из `SvAppView.hud`, рисует critical resources/conditions и status-only Character primary. Все 27 outcomes представлены; все 15 encumbrance flags имеют отдельные строки. Unknown availability не смешивается с нулём; отсутствующие old-version sanity numeric/damage обозначены как не передаваемые сервером. Сохраняются server attribute/severity color roles, source color spans sanity/blows/extra и raw unknown indicators в модели; известные indicator meanings выводятся по baseline compile branch/shield priority.

Numeric/bar/huge/solid/graphics/experience preferences принимаются из effective `SvSettingsOptions`, layout/UI scale — из `SvSettingsConfig`. Left HUD визуально вписывается по высоте, Character primary сохраняет requested text scale, wrapping и внутренний scroll. Left Character shortcut и Shift+C доступны независимо от right panel; Esc/X закрывают primary; server request/quantity/pause и stale generation не перехватываются. Rebuild/resize сохраняют primary/scroll и не потребляют effects.

Existing diagnostic launcher `--synthetic --status-check` доставляет независимо заданные packet bytes всех групп через `sv_app_receive`/`sv_app_step`, затем вызывает production renderer. `--status-small` и `--status-character` ограничивают review existing layouts; `SV_STATUS_SCROLL` выбирает scroll offset, `SV_STATUS_CAPTURE` сохраняет BMP до Present. Нового simulator/harness нет. Identity boundary C011 и actual C014 lifecycle не менялись.

Linux X11/software native runs по 2 кадра прошли; 1280×800 wide, 1024×768 small Character top/bottom и 150% UI scale с сохранённым primary после rebuild. Артефакты: `.scratch/sv-c012/{wide,small-character,small-bottom,scaled}.{bmp,png}`, `native-{wide,small,bottom,scaled}.log`. PNG — только конвертация BMP для просмотра, без правки кадров. Root выполняет финальную сборку/core checks на итоговом snapshot отдельно.

Exact native command (cwd — repository root):

```sh
SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=software \
SV_STATUS_CAPTURE="$PWD/.scratch/sv-c012/wide.bmp" \
src/tomenet-sv --synthetic --status-check \
  --profile-root "$PWD/.scratch/sv-c012/native-wide" \
  --library "$PWD/lib" --fixture-window 1280x800 --frames 2
```

Small top: те же env/args с capture `small-character.bmp`, root `native-small`, flags `--status-small --status-character`, размер `1024x768`. Bottom: тот же запуск с `SV_STATUS_SCROLL=100000` и capture `small-bottom.bmp`; scroll clamped production renderer. Scaled: root `native-scaled` с isolated CFG `svUiScalePercent 150`/`svLayout small`, flag `--status-character`, размер `1024x768`, capture `scaled.bmp`. Native run потребовал доступ к локальному X11 display вне sandbox; sandbox-only попытка получила `No available video device`, это не renderer failure.

Преференсы shipped library дали retained-usable diagnostics; caller явно инициализирует defaults перед загрузкой и сохраняет usable snapshot, о чём пишет warning. Optional preference diagnostic не превращает корректный HUD render в отказ. Ранний неуспешный native attempt из-за fatal handling этого boolean исправлен до final native captures.

### UX source hashes

Reference root: `/home/svechnik/Projects/github_site/tomenet_interface/`. HTML/JS задают confirmed presentation, не synthetic игровые значения/формулы.

| Reference | SHA-256 |
|---|---|
| `index.html` | `5a6616906da9072561044afe251c32fc0f161002fa61b21b11b73fce63cbbc9c` |
| `css/hud.css` | `76c1c916d0b12964093db3ae986f0589a595da11ef458a8c0ab97e9da52dfe63` |
| `css/character-window.css` | `d908bf092ac703ce144d3bc0597d94b09abfe618ed91b84f5c5e060ddd72695f` |
| `js/features/character.js` | `29ce142cd830e6baeb92bb9a3b50789dde2a694d9587b557842def6f3fc682e1` |
| `js/features/character-window.js` | `4ca618622d81fdb2e1627b14796d3cf65b23bcde71c4f91876d2d1657d6a27c1` |
| `js/features/hud.js` | `09c4685d30c6c74d35b2e8df2539487e97f1f6f94b851a179ff208b8a7394283` |
| `app.js` | `f513820e04b078ad8e85ed483d7ec864d7591f39070ddde5fe59e5de8bc1d421` |


## Итоговая проверка оркестратором — 2026-11-11

Linux production build, focused production status runner и bounded native HUD review прошли на финальных sources; собственный результат задачи проверен в описанных границах. Independent reviews backend/UI закрыли найденные дефекты. [Команды, артефакты и disposition всех 81 obligations](../../sv-c012-evidence.md). Actual C014 lifecycle, C013 карта и поздние recipients не получают PASS. Это технический результат, не пользовательская приёмка и не изменение canonical registry.


## Повторное открытие native UX — 2026-11-11

Пользователь указал, что предъявленный снимок не соответствует HTML-макету. Подтверждено: прежний native renderer использовал произвольную компоновку ресурсов/таблицы и не воспроизводил нормативные карточки и визуальную структуру макета. Прежние native captures подтверждают работоспособность renderer и читаемость тестовых значений, но **не подтверждают выполнение UX-критерия 006**. Утверждение о полной технической готовности 006/007 отозвано; core evidence 001–005 сохраняется. Исправление renderer и повторная визуальная сверка выполняются в существующих задачах 006/007.

### Критерии исправления по текущему DOM/CSS

- `#levelCard` / `.progress.xp`: XP с level diamond; `#currencyCard`: Gold в одну строку; `#encumbranceSlots`: компактные слоты.
- `#attributesCard`: шесть компактных icon/name/value строк; `#sanityArmorCard`: две компактные строки; `#vitalsCard`: framed resource cards с согласованными label/value и meter, без debug-маркеров.
- `#enemyHealthCard`: отдельный tracked meter; `#speedCard` / `#bprCard`: компактные metric rows; `#statusIconGrid`: grid состояний вместо текстового списка.
- `#characterWidget` / `#characterContent`: сгруппированные overview, resources/stats, combat и details; `#characterWindowOverlay`: framed primary с заголовком/закрытием и тем же status-only содержимым.
- Normative visual tokens: финальные слои `css/theme.css` (`--forged-surface`, `--forged-inset`), `css/hud.css`, `css/right-panel.css`, `css/character-window.css`. `reference-full.png` — более ранняя художественная иллюстрация, не точный снимок текущего HTML.
- Неперенесённые inventory/chat/map/Ping и full C030 действия не имитируются макетными значениями. Fonts/preferences, wire semantics, availability и input ownership сохраняются.

Browser-open локального `file://` макета запрещён URL policy; обход не выполнялся. Визуальная сверка опирается на прочитанные локальные DOM/CSS и новые native captures; browser-rendered reference не заявляется.


## Исправление native UX и повторная проверка — 2026-11-11

Renderer переработан по текущим DOM/CSS: XP/level, Gold, encumbrance, attributes, sanity/armour, resource cards, tracked health, speed/blows, fixed status grid; Character получил overview/combat/details и сгруппированные дополнительные сведения. Исправлены дробные raster scanlines, stat notation/linear_stats, No-Tele, Wraith/PbTrav, unknown huge values и потерянные supplemental поля. Изменены только production `ui/hud.c` и `hud.h`; protocol/model/core не менялись.

Сборка и existing focused core PASS. На финальном source snapshot standalone X11/software wide1280×800 и smallCharacter1024×768 прошли по2кадра. Offscreen dummy/software дополнительно подтверждает small без primary, Character/details и UI150%. Independent review подтвердил перечисленные structural criteria и отсутствие новых блокирующих findings. Точные команды/source SHA и ограничения — [обновлённый evidence](../../sv-c012-evidence.md).

Полное визуальное совпадение не заявляется: decorative attributes/encumbrance symbols упрощены, native texture/typography приближённые. Пользовательская визуальная приёмка остаётся открытой. Старые native captures не являются доказательством соответствия макету.


## Доработка оформления по запросу пользователя — 2026-11-11

Пользователь потребовал довести HUD до соответствия текущему HTML-макету, включая иконки и оформление. Прежние оговорки о приближённых decorative symbols/theme не закрывают эту работу. Требуются точные source-derived SVG shapes и применимые final CSS gradients/frames/insets/card geometry, независимая визуальная проверка новых кадров. Семантика и полнота данных C012 сохраняются; пользовательская приёмка остаётся открытой.


## Итог доработки иконок и оформления — 2026-11-11

Запрошенная доработка завершена. Прежние упрощённые symbols заменены 53 source-derived SVG icons; native renderer реализует применимые final CSS layers/frames/insets, resource gradients, XP diamond/drain, Gold coin, decorative separators, 12 encumbrance slots и 24 status slots. Critical tile/icon/shield mark используют общую фазу pulse. Character соответствует проверенным размерам/иерархии текста и overview/combat/details, полные status values доступны через scroll. Выбранный пользователем text font сохранён. Новых runtime-зависимостей нет.

Три агента выполнили icon implementation, оформление и независимый appearance review. Все известные findings закрыты. Финальные Linux production build, existing focused core runner и два standalone X11/software native runs прошли. Final images: `.scratch/sv-c012/html-fidelity/hud-current.png` и `character-current.png`. Root просмотрел их и сверил source snapshot; подробные команды, SHA, критерии и границы — в `docs/sv-c012-evidence.md`.

Сверка опирается на текущие DOM/CSS/SVG и native captures; browser-rendered side-by-side и попиксельное тождество не заявляются. Ранее записанная оговорка об оставленных упрощённых иконках/плоской теме относится к предыдущему snapshot и больше не описывает текущую реализацию. Пользовательская приёмка и canonical runtime registration остаются pending.


## Повторная сверка только по текущей вёрстке — 2026-11-11

После удаления устаревшего PNG root и отдельный read-only агент заново проверили текущие HTML/CSS/JS/SVG. Предыдущий общий appearance PASS отозван как слишком широкий. Исходные53 SVG и их SHA действительно совпадают; несоответствия обнаружены в runtime presentation и state mapping.

Открытые findings 006/007:

1. Native hardcodes panel widths318/486. Current `app.js:38–40` рассчитывает default widths от viewport: при1280px —262/394, при1920px —367/552. Динамическое правило JS пропущено предыдущим review. Учитывать documented user overrides и small fit при исправлении.
2. `hud.c` отображает `PKT_IDLE` активной луной Resting; baseline `Receive_idle` управляет audio auto-mute, не доказывает отдых персонажа. Сохранять Idle отдельно и не выдумывать activity state.
3. Native цвета Blind/Blind+Hallu/Confusion/Fear отличаются от актуальных `CONDITION_DEFINITIONS`. Для этих полей нет отдельного server attribute. Это не относится к sanity/blows, где authoritative attribute необходимо сохранять.
4. Embedded96px masks уменьшаются SDL linear до15/18/20px; source geometry equality не доказывает одинаковые stroke coverage и внешний вид. Одноразовое численное сравнение predicted bilinear alpha с direct rsvg raster at18px дало mean absolute alpha difference14.66/255 для Strength и16.32/255 для Sanity. Это анализ алгоритма, не browser/GPU pixel comparison. Native glow также реализован смещёнными копиями вместо CSS Gaussian drop-shadow.

Дополнительное различие fixtures: native включает все15 encumbrance flags, HTML default — только heavy_wield; active conditions также различны. Визуально сравнивать выбор иконок следует при одинаковых состояниях. Это не отменяет перечисленных ошибок.

Runtime code при этой перепроверке не изменялся, прежние core/build evidence остаются действительными только в своих границах. 006/007 снова открыты; пользовательская приёмка не заявлена. Удалённый PNG не использовался.


## Результат корректировки R1–R4 — 2026-11-11

Задачи были скорректированы до начала реализации и прошли независимый review. Затем два implementation owner исправили HUD presentation и SVG backend; отдельный агент проверил задачи, production sources и финальные кадры. Пользовательская приёмка pending.

- **R1:** defaults1280→262/394,1920→367/552 выбираются один раз по initial logical viewport. Explicit overrides и выбранные widths сохраняются при resize/DPI/generation/rebuild. Small fit использует фактическую высоту видимых блоков; main HUD показывает current stat в baseline notation, полные значения сохраняются в Character. Selected primary/UI scale не теряется. XP/resource text shadows восстановлены по CSS для читаемости белой XP-полосы.
- **R2:** PKT_IDLE остаётся отдельным Audio idle/readable field; activity unavailable и не получает Resting-луну. Семантика protocol/model/occurrences не менялась.
- **R3:** conditions без server attribute используют state-specific colours/severity/critical из текущего JS; sanity/blows и прочие explicit attributes сохраняются.
- **R4:** original SVG stroke commands растеризуются existing FreeType непосредственно под physical size/phase;96px texture downscale убран. Gaussian filter chain и whole-tile opacity применяются корректно. OVERLAP glyph raster устранён обнаруженный дефект пересекающихся штрихов. Новых runtime dependencies нет.

Final production build и existing focused core runner PASS. Existing diagnostic получил только narrow raw-packet matched-state overlay и one-frame icon-gallery mode; отдельного harness/suite нет. Все53 SVG выполнены production API в15/18/20px, все159 glyph renders непустые. Средняя alpha MAE против direct SVG-at-size:1.257/0.985/1.027 из255; это не browser pixel comparison.

Восемь финальных dummy/software offscreen scenarios прошли; затем два обычных X11/software запуска без stdbuf прошли по2кадра. Ранее были три timeout-попытки со stdbuf wrapper, их причина не установлена и они сохранены как unsuccessful attempts, не переписаны в PASS. Independent review финальных ordinary/small native captures не выявил новых findings. Источник/артефакты/SHA/commands и границы — в последнем разделе `docs/sv-c012-evidence.md`; canonical runtime registry не изменён.


### Согласованное исключение font family — 2026-11-12

Пользователь уточнил: клиент имеет независимый map font с legacy выбором и UI font; UI font может отличаться от HTML. Текущий CascadiaMono допустим. Поэтому HTML serif labels vs native CascadiaMono не являются open finding. Размеры/плотность/выравнивание UI и сохранность independent map/UI preferences остаются критерием006-V2. Добавление serif assets, смена map font или подмена выбранного UI font ради screenshot не требуются.


## Итог сравнения с реальным браузерным макетом — 2026-11-12

Задачи006/007 исправлены и проверены по реальным парам HTML/SV, а не только по исходникам макета. Финальные эталоны `html-wide-final.jpg` и `html-character-final.jpg` сняты после завершения CSS transition (opacity1), при1932×1208/DPR1; SV normal/primary сняты в настоящем X11 fullscreen при том же output, UI100%, widths370/556. Независимый reviewer просмотрел обе HTML серии и финальные normal/primary/details/small кадры. Новых блокирующих findings в применимой C012 части нет; пользовательская приёмка остаётся pending.

Исправлены origin/вертикальный ритм карточек и Ping slot, heights ресурсов, положение/размер Character widget и primary, фон/рамки/backdrop, shortcut и сохранность scroll/input. Дополнительный small1024×768 кадр обнаружил clipping боевых подписей; дефект исправлен и кадры пересняты. CascadiaMono UI согласован пользователем; map font не менялся. Отсутствующие C030 данные, map/chat/inventory/bags и другие deferred controls не имитируются demo-данными. Весь интерфейс не объявляется идентичным HTML.

Linux shell build, existing focused production core и четыре финальных bounded native runs прошли; source snapshot оставался неизменным. Подробные пары, commands/env/SHA и границы — в [финальном evidence](../../sv-c012-evidence.md#итог-реального-парного-сравнения-htmlsv--2026-11-12).


Финальное уточнение2026-11-12: после первого review root нашёл трёхсекционный header правого Character widget вместо HTML `PROFILE / SKILLS` + `RESISTS`. Widget header/click extent исправлены, три секции primary сохранены. Прежний snapshot сохранён в `browser-visual/pre-widget-header-fix/`; build/core и четыре native capture повторены, независимый reviewer заново просмотрел все кадры и не оставил новых блокирующих C012 findings. Актуальные SHA, exitcodes и граф — в последнем разделе evidence.

# C012 — live HUD/status

## Приёмка — 2026-11-13

Пользователь подтвердил закрытие C012 в проверенных границах Linux status/core: [решение о приёмке](sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются открытыми в реестре отсрочек; полный canonical runtime PASS не заявлен.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](tasks/c012/final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

Статус: **приёмка интерфейса отложена; собственные core/semantic обязательства C012 открыты**, 2026-11-13.

## История решения об отсрочке приёмки интерфейса — 2026-11-13

**Интерфейс в C012 больше не дорабатываем; его дальнейшая визуальная/пользовательская приёмка отложена.** [Реестр C012-DA-01–10](tasks/c012/deferred-acceptance.md) задаёт принимающие этапы, условия закрытия и взаимные ссылки. Прежние visual correction gates ниже сохраняются как история; они не требуют продолжать оформление или снимки в C012 сейчас. Отсрочка не означает PASS, завершение C012 или перенос canonical 27/81.

В C012 остаются финальный минимальный existing focused core check последнего implementation candidate, semantic contract Study/stat index, сохранность данных/событий и точный disposition обязательств. Техническую часть можно завершать без ожидания поздней визуальной приёмки. Последний existing-slots freeze3 содержит исправления stat/encumbrance/Sanity/XP и resize/reflow; его финальные core и affected visual проверки после команды «стоп» не завершены. Старые snapshot PASS не подтверждают этот финальный кандидат. Новых блоков Character не добавлять; RESISTS unknown/DUMP unavailable остаются согласованными.

## История результата до отсрочки

Исторический статус: **freeze5 Character presentation/input checked in wide layout; complete C012 UI acceptance open**, 2026-11-12. Общее окно принято пользователем; приёмка исправленного Character ожидается.

[Исходный план Character006-C1–C9/007-C1–C2](../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/correction-plan.md) основан на реальных кликах всех вкладок до исправлений. На момент этого аудита SV SKILLS / RESISTS / DUMP были инертны; PROFILE имел visual findings. Текущий итог freeze5 ниже. ПолныеC030 data/actions не принимаются этим аудитом и не превращают собственные Character findings вPASS.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Текущий остаток уточнён по baseline и реально показанному HTML: прежние пять отсутствующих «labels» и rawfields не являются автоматически пятью новыми блоками/строками. **Доказанные адресные дефекты:** stat colour priority/base118 и weapon encumbrance precedence в существующих слотах. Sanity numeric имеет существующий режим; stat/level/XP используют условные/derived значения. Self AFK/study/opaque extra и bank не получили подтверждённого места HTML; studycount требует source/contract разрешения. Idle/martyr — реакции/сообщения, не новые Character labels. Unknownbits сохраняются без обязательнойhexстроки. Canonical27/81неизменны; полнаяUIприёмка не заявлена.

[План правок задач](../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md), [scope audit](../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и снимки](../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md). Задачи003–007уточнены,002дополнена existingSanitymode. Production в этой planningволне не менялся. Новый Character StatusDetails отклонён и не возвращается; implementation пока не запускается. Минимальные Linux small/resize/temporal и необходимые existingcorecases записаны как предложение после согласованных изменений; это не разрешение расширить suite/harness.

## Текущий результат исправлений Character — freeze5, 2026-11-12

Исправлены реальные PROFILE/SKILLS/RESISTS: выбор страницы, active state, композиция, компактная высота RESISTS, связанный правый виджет, close/reopen retention и ограниченная фактическим содержимым прокрутка. Исправлены тон панели/шапки, подпись Level, заголовки и формат Blows/Round. Формы14 source-иконок перенесены из HTML; обнаруженный дефект blur-only исправлен чётким SVG foreground.

Пользователь одобрил RESISTS с неизвестными значениями и явно недоступный DUMP. Предложение `+ STATUS DETAILS` **отклонено** после показа снимков: переключатель, список и связанные state/hit regions полностью удалены. В SKILLS только секции макета; новый заменяющий блок не добавлен. Старые proposal/freeze3/freeze4 снимки исторические.

Независимая реальная проверка при1932×1208: три HTML/SV пары, переключения, DUMP unavailable notice, закрытие/повторное открытие, widget sync, fitting wheel/PageDown — без новых блокирующих presentation/input findings в этом состоянии. Native838frames/22PNG/exit0, frozen source/binary стабильны. [Отчёт и границы проверки](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/visual-review.md); пары: [Profile](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-profile.png), [Skills](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-skills.png), [Resists](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-resists.png).

Linux build и существующий focused production status check — exit0; [команды/логи/SHA](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/checks/final-checks.json). HUD SHA `56092ac705846c33e4ffb5d4350e9542b9f73861d323965f6a57238706526a91`, binary `710bd1d4679221b9b2c4ee598a22acb3a6e015d65d50c5a5d1c3f07d332c1fe6`. Обновлён существующий code graph:29331nodes/72309edges/1475communities, exit0. Новая suite/harness не добавлялась.

**Остаток C012 UI:** после удаления дополнительного списка нет самостоятельного видимого вывода study,AFK,extra,martyr,audio-idle; также больше не показан ряд расширенных значений bank/attributes/XP/sanity/concurrent encumbrance. Backend/derived rows сохранены, но это не видимое UI. [Точный список и семантические исключения](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/implementation.md). Нельзя объявлять все27outcomes/81obligations или весь Character принятыми. Canonical coverage не изменён; пользовательская приёмка остаётся открытой. Это не разрешение вернуть технический список в Character. Полные RESISTS data/export, live session, small paired capture и temporal QA текущим visual review не подтверждены.

## Текущая корректировка внешнего вида по браузерному рендеру

006/007 повторно открыты в части Character по результату пользовательской проверки2026-11-12: общее окно принято, Character не принят. Последующий аудит Profile выявил незакрытые расхождения композиции и оформления; прежняя проверка не покрывала переключение и содержимое остальных вкладок. Источник визуального сравнения — целиком отрисованный локальный HTML через работающий `http://127.0.0.1:8765/`; чтение DOM/CSS/JS/SVG больше не заменяет paired browser/native comparison. R1–R4 завершены в границах прошлых проверок; общий appearance PASS ими не подтверждён.

Актуальный [контракт006-V1–V4](tasks/c012/SV-C012-006-native-hud.md): whole-frame geometry/spacing HUD, font/appearance, отдельно Character primary и согласованное состояние. [007](tasks/c012/SV-C012-007-evidence.md) требует одинакового client viewport/scale/layout, before+after HTML/SV screenshots, source/state manifest и независимого review обеих серий после freeze. Первоначальный target1932×1208, UI100%, wide, normal и Character primary. Не растягивать снимки задним числом ради совпадения.

Approved policies сохраняются: selected widths не меняются при resize/DPI, small layout ручной, critical HUD остаётся доступен, выбранный text font не подменяется. Пользователь2026-11-12 подтвердил: UI font может отличаться от HTML, CascadiaMono допустим; map font независим. Serif/mono difference — согласованное исключение, UI geometry/size/line-height/density проверяются; serif assets и map-font changes не требуются. Baseline/packet values и availability сохраняются; отсутствующие data не изобретаются. C013/C014 и остальные deferred surfaces отмечаются как scope differences; полный C030 не переносится, но внешний вид status-only Character primary входит в006. Удалённый устаревший PNG не используется. Broad suites/provider matrix не добавляются. Пользовательская приёмка и canonical statuses остаются pending.

Этап даёт игроку актуальные серверные сведения о персонаже: HP, mana, stamina, sanity; характеристики и броню, демографические данные, боевые бонусы, опыт и деньги; состояния и ограничения, дополнительные индикаторы и здоровье отслеживаемого монстра. Это 27 canonical outcomes и 81 obligation из stages/native-coverage, [точная allocation](tasks/c012/coverage.json).

| Группа | Возможности |
|---|---|
| Vitals — 4 | live HP, mana, stamina, sanity |
| Character — 6 | stats, armour, demographics, combat bonuses, experience, currency |
| Conditions — 10 | confusion, poison, study, blows/wraith, food, fear, speed, cut, blind/hallucination, stun |
| Extended — 7 | tracked monster health, AFK, encumbrance, extra status, martyr, idle, indicators |

Единственный canonical prerequisite: read-live-hp → status.read-hp этапа A; synthetic HP не принимает live caller. Реализационные зависимости на существующие SV protocol/session/resources не меняют canonical prerequisites.

Путь: version-aware packet decoder → атомарное изменение session presentation model → read-only view с revisions → native HUD. Сохраняются все baseline wire branches/defaults, unknown bits, opaque source bytes, bar/boost conventions и применимые build guards. UI не читает wire/Term/globals, incomplete packet не публикуется. Все session данные привязаны к generation; reset не оставляет старых значений.

Реализация и адаптация размещаются в SV. Используются retained SvApp/preparation и existing HP/identity; расширение supported packets должно сохранять ordering, exact bytes и once-only применение applied/deferred occurrences. Source details и риски — [contract](tasks/c012/source-contract.md).

Native projection соблюдает утверждённые [layout constraints](capabilities/item-policy.md) и [HUD preferences](capabilities/session-policy.md), включая independent text/map scale, critical HUD и применимые numeric/bar/huge-bar режимы. Предыгровой HUD не вводится. Synthetic shell — демонстрационный, не финальный UX reference. Редакторы options и full information surfaces относятся к следующим этапам.

C013 принимает карту, C014 — actual socket adoption/close/relog/full live lifecycle. Собственный status decoder/model/native caller C012 обязателен сейчас; combined evidence следующих owners остаётся pending. C011 не переоткрывается. Непроверенный canonical obligation не получает PASS от успеха общего primitive.

Проверки: необходимый минимум Linux production-core и работающего native consumer, описанный в [007](tasks/c012/SV-C012-007-evidence.md). Broad suite, новый simulator/harness, полная visual/provider matrix и Windows/MinGW/Wine не входят в текущий план; расширение согласовывается заранее. На этапе планирования runtime tests не запускались.


Повторный review 2026-11-11 уточнил задачи без изменения allocation: HP occurrence и HUD revision разделены; body/idle reactions сохраняются отдельно от latest snapshot; native diagnostic caller вызывает production renderer из src. Для small layout C012 включает status-only read subset существующего Character primary с независимым от right panel shortcut; full C030 остаётся своим этапом. Точные изменения — [итоги review](tasks/c012/README.md). Блокирующих вопросов не выявлено.


Реализация и необходимый минимум Linux проверки завершены: [evidence](sv-c012-evidence.md). Задачи 001–007 технически выполнены в границах production status core/native consumer. Actual C014 adoption/close/relog, C013 карта, поздние recipients и canonical runtime registration остаются отдельной работой.


## Повторное открытие native UX — 2026-11-11

Пользователь указал, что предъявленный снимок не соответствует HTML-макету. Подтверждено: прежний native renderer использовал произвольную компоновку ресурсов/таблицы и не воспроизводил нормативные карточки и визуальную структуру макета. Прежние native captures подтверждают работоспособность renderer и читаемость тестовых значений, но **не подтверждают выполнение UX-критерия 006**. Утверждение о полной технической готовности 006/007 отозвано; core evidence 001–005 сохраняется. Исправление renderer и повторная визуальная сверка выполняются в существующих задачах 006/007.


## Исправление native UX и повторная проверка — 2026-11-11

Renderer переработан по текущим DOM/CSS: XP/level, Gold, encumbrance, attributes, sanity/armour, resource cards, tracked health, speed/blows, fixed status grid; Character получил overview/combat/details и сгруппированные дополнительные сведения. Исправлены дробные raster scanlines, stat notation/linear_stats, No-Tele, Wraith/PbTrav, unknown huge values и потерянные supplemental поля. Изменены только production `ui/hud.c` и `hud.h`; protocol/model/core не менялись.

Сборка и existing focused core PASS. На финальном source snapshot standalone X11/software wide1280×800 и smallCharacter1024×768 прошли по2кадра. Offscreen dummy/software дополнительно подтверждает small без primary, Character/details и UI150%. Independent review подтвердил перечисленные structural criteria и отсутствие новых блокирующих findings. Точные команды/source SHA и ограничения — [обновлённый evidence](sv-c012-evidence.md).

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


## Финальная проверка реального HTML/SV — 2026-11-12

На frozen source проверены normal и Character primary пары1932×1208: текущий локальный HTML в Chromium/DPR1 и SV в X11/software fullscreen, UI100%, selected widths370/556. Исправлены карточки/Ping geometry, resource spacing, Character widget/primary layout, оформление/backdrop, shortcut и scroll/input; сохраняются27 outcomes и baseline availability. Независимый reviewer просмотрел обе браузерные серии и четыре native кадра, включая details и дополнительный offscreen small regression. Найденный small label clipping исправлен и перепроверен. Новых блокирующих findings в применимой C012 части нет.

Linux production build/core и четыре native runs PASS; source SHA не менялся при capture. UI CascadiaMono допускается пользователем, map font не менялся. Deferred map/chat/inventory/bags/full C030 и неподтверждённые сервером значения не заполнялись HTML demo. Полное совпадение интерфейса, pixel identity, temporal animation acceptance и пользовательская приёмка не заявляются. [Финальные снимки/команды/границы](sv-c012-evidence.md#итог-реального-парного-сравнения-htmlsv--2026-11-12).


Финальное уточнение2026-11-12: после первого review root нашёл трёхсекционный header правого Character widget вместо HTML `PROFILE / SKILLS` + `RESISTS`. Widget header/click extent исправлены, три секции primary сохранены. Прежний snapshot сохранён в `browser-visual/pre-widget-header-fix/`; build/core и четыре native capture повторены, независимый reviewer заново просмотрел все кадры и не оставил новых блокирующих C012 findings. Актуальные SHA, exitcodes и граф — в последнем разделе evidence.

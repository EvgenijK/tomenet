# C012 — live HUD и статусы персонажа

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

Исторический статус: **freeze5 Character presentation/input checked in wide layout; complete C012 UI acceptance open**, 2026-11-12. Общее окно принято пользователем; приёмка исправленного Character ожидается.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Текущий остаток уточнён по baseline и реально показанному HTML: прежние пять отсутствующих «labels» и rawfields не являются автоматически пятью новыми блоками/строками. **Доказанные адресные дефекты:** stat colour priority/base118 и weapon encumbrance precedence в существующих слотах. Sanity numeric имеет существующий режим; stat/level/XP используют условные/derived значения. Self AFK/study/opaque extra и bank не получили подтверждённого места HTML; studycount требует source/contract разрешения. Idle/martyr — реакции/сообщения, не новые Character labels. Unknownbits сохраняются без обязательнойhexстроки. Canonical27/81неизменны; полнаяUIприёмка не заявлена.

[План правок задач](../../../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md), [scope audit](../../../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и снимки](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md). Задачи003–007уточнены,002дополнена existingSanitymode. Production в этой planningволне не менялся. Новый Character StatusDetails отклонён и не возвращается; implementation пока не запускается. Минимальные Linux small/resize/temporal и необходимые existingcorecases записаны как предложение после согласованных изменений; это не разрешение расширить suite/harness.

## Текущий результат исправлений Character — freeze5, 2026-11-12

Исправлены реальные PROFILE/SKILLS/RESISTS: выбор страницы, active state, композиция, компактная высота RESISTS, связанный правый виджет, close/reopen retention и ограниченная фактическим содержимым прокрутка. Исправлены тон панели/шапки, подпись Level, заголовки и формат Blows/Round. Формы14 source-иконок перенесены из HTML; обнаруженный дефект blur-only исправлен чётким SVG foreground.

Пользователь одобрил RESISTS с неизвестными значениями и явно недоступный DUMP. Предложение `+ STATUS DETAILS` **отклонено** после показа снимков: переключатель, список и связанные state/hit regions полностью удалены. В SKILLS только секции макета; новый заменяющий блок не добавлен. Старые proposal/freeze3/freeze4 снимки исторические.

Независимая реальная проверка при1932×1208: три HTML/SV пары, переключения, DUMP unavailable notice, закрытие/повторное открытие, widget sync, fitting wheel/PageDown — без новых блокирующих presentation/input findings в этом состоянии. Native838frames/22PNG/exit0, frozen source/binary стабильны. [Отчёт и границы проверки](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/visual-review.md); пары: [Profile](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-profile.png), [Skills](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-skills.png), [Resists](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-resists.png).

Linux build и существующий focused production status check — exit0; [команды/логи/SHA](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/checks/final-checks.json). HUD SHA `56092ac705846c33e4ffb5d4350e9542b9f73861d323965f6a57238706526a91`, binary `710bd1d4679221b9b2c4ee598a22acb3a6e015d65d50c5a5d1c3f07d332c1fe6`. Обновлён существующий code graph:29331nodes/72309edges/1475communities, exit0. Новая suite/harness не добавлялась.

**Остаток C012 UI:** после удаления дополнительного списка нет самостоятельного видимого вывода study,AFK,extra,martyr,audio-idle; также больше не показан ряд расширенных значений bank/attributes/XP/sanity/concurrent encumbrance. Backend/derived rows сохранены, но это не видимое UI. [Точный список и семантические исключения](../../../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/implementation.md). Нельзя объявлять все27outcomes/81obligations или весь Character принятыми. Canonical coverage не изменён; пользовательская приёмка остаётся открытой. Это не разрешение вернуть технический список в Character. Полные RESISTS data/export, live session, small paired capture и temporal QA текущим visual review не подтверждены.

## Исходная корректировка Character — 2026-11-12

[План006-C1–C9/007-C1–C2 и матрица](../../../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/correction-plan.md) основаны на новых реальных кликах и снимках всех страниц: [HTML](../../../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/html/report.md), [SV](../../../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/sv/report.md). В SV SKILLS / RESISTS / DUMP инертны; в HTML SKILLS меняет нижнийблок,RESISTS — всю композицию/высоту. Общийвид Profile также имеет CH-01…09. Исправления production в этом аудите не выполнялись; новые ticketIDs/coverage не нужны. Исторические итоги ниже не являются текущим Character PASS.


## Порядок выполненного исправления

1. Снять реальные локальные HTML и SV при одинаковом client viewport, scale/layout и сопоставимых состояниях — normal и Character primary. Локальный браузерный доступ через `http://127.0.0.1:8765/` проверен и работает.
2. По whole-frame paired before captures зафиксировать concrete findings и выполнить [006-V1–V4](SV-C012-006-native-hud.md): геометрия HUD, оформление/текст, отдельный Character primary, сохранность semantic values и policies.
3. После SV-local исправления выполнить [007 paired-browser evidence](SV-C012-007-evidence.md) на frozen snapshot и независимый review **обоих** after вариантов; показать пользователю HTML+SV.

Пользователь2026-11-12 разрешил UI font, отличный от HTML: текущий CascadiaMono допустим, map font остаётся независимым. Font-family exception не отменяет сверку геометрии, размеров/плотности и читаемости.

Новых ticket IDs/owned outcomes не добавляется. Current browser-rendered HTML — визуальный reference; DOM/CSS/JS/SVG объясняют layout и effects, approved policy и baseline определяют поведение. Selected widths сохраняются при resize/DPI, layout выбирается вручную. Deferred map/messages/inventory/full C030 отмечаются отдельно, не имитируются demo data и не скрывают C012 findings. Удалённый устаревший PNG не используется. Исторические reports и прежние source/native checks ниже не заменяют текущий corrective contract.

C012 переносит полное чтение live HUD/status поверх synthetic HP этапа A: **27 outcomes / 81 obligations**. Точные IDs, conditions, prerequisites и pending statuses сохранены в [planning snapshot](coverage.json); [спецификация](../../sv-stage-c012-spec.md), [baseline/production seams](source-contract.md). Три агента независимо проверили canonical scope, production seams и открытые вопросы.

| Задача | Результат | Owned outcomes |
|---|---|---:|
| [SV-C012-001](SV-C012-001-status-foundation.md) | Модель статусов, decoder contract и generation | cross-cutting |
| [SV-C012-002](SV-C012-002-vitals.md) | HP, мана, выносливость и sanity | 4 |
| [SV-C012-003](SV-C012-003-character-values.md) | Характеристики, боевые значения и прогресс | 6 |
| [SV-C012-004](SV-C012-004-conditions.md) | Состояния персонажа | 10 |
| [SV-C012-005](SV-C012-005-extended-status.md) | Расширенные статусы и tracked monster | 7 |
| [SV-C012-006](SV-C012-006-native-hud.md) | Production native caller и HUD | cross-cutting |
| [SV-C012-007](SV-C012-007-evidence.md) | Минимальное Linux evidence и 81 obligations | cross-cutting |

Порядок: 001 → 002–005 → 006 → 007. 002–005 используют один согласованный owner/contract; их можно разбирать независимо после 001. У каждого capability ровно один первичный владелец, 006/007 проверяют общий consumer.

C011 уже принят: resources/preparation/accepted identity переиспользуются. C013 владеет live map, C014 — actual socket adoption, complete dispatch/lifecycle, close/relog. C012 обязан дать работающий production status caller и читаемую native projection; нельзя заменить собственный результат обещанием C014. Combined C014 flows при этом не получают PASS из C012 core evidence. C015/C018/C020/C030, live collections information, C046 option editor, C050 alerts, C055 audio и C053 macros сюда не включаются.

Блокирующих вопросов пользователю по результатам анализа не выявлено. C API/структуры, coarse revisions, размещение SV-local decoders и baseline limits — решения реализации. Synthetic shell не задаёт финальный gameplay UX. Если обнаружится конкретный пробел утверждённого UX или потребуется изменение baseline, вынести узкий вопрос до зависимой реализации; полный redesign не подразумевается.

Обязателен Linux, Windows/MinGW/Wine deferred. Только необходимый минимум core проверок production-пути; расширение suites требует согласования. Canonical catalog/coverage не изменены. При реализации обнаруженные независимые улучшения записывать отдельно в docs/sv-improvements.md; намеренное SV-local дублирование само по себе не дефект.


## Повторный критический review — 2026-11-11

По запросу пользователя три агента повторно проверили scope, production feasibility и acceptance. Семь задач сохранены: новых ticket IDs не требуется, canonical 27 outcomes / 81 obligations и их единственное ownership не изменены. Уточнения внесены непосредственно в задачи:

- 001/002: HP occurrence отделён от общей HUD revision; обновление mana/AFK не повторяет предупреждения, одинаковые HP packets сохраняют baseline reactions. Существующие HP effects не отключаются.
- 001/003/005: значимые body/idle transitions имеют occurrence-aware generation/sequence и явный delivery/ack либо bounded retention/overflow; latest state не заменяет промежуточные реакции.
- 006: production HUD renderer находится в src/client/sv/ui, existing diagnostic caller лишь вызывает его через production packet/app путь. Identity boundary C011 не расширяется в live loop.
- 006: status-only read subset существующего Character primary доступен через left shortcut даже в small layout. Full C030 не принимается; добавлены динамические HTML reference files.
- 004/007: минимальный production DEPTH consumer обеспечивает speed/no-tele и обновление projection при обоих порядках DEPTH/SPEED, без расширения приёмки до карты C013.
- 002–005/007: минимальный core набор проверяет semantic interpretation branches наряду с wire layouts. Новая suite или broad matrix не назначается.

Блокирующих вопросов пользователю не осталось: доступ в small layout обеспечивается существующим утверждённым Character primary entry, новое UX решение не требуется. Если конкретное обязательное поле нельзя представить в утверждённой surface, узкий вопрос возникает до зависимой реализации. В этом проходе изменены только planning документы, runtime tests и реализация не запускались.


## Реализация и проверка — 2026-11-11

Три агента реализовали SV backend, reusable native HUD и focused core checks, затем выполнили перекрёстный review. Оркестратор подтвердил финальную Linux production build и status runner. Все 27 outcomes проходят production protocol→model→view; native renderer размещён в src и вызывается existing diagnostic entry. X11/software wide/small/Character/150% просмотрены; финальный small native run повторён после последней input correction.

Проверены exact defaults/branches/split+sentinel, HP-only per-packet reactions, body/idle FIFO/overflow, DEPTH/SPEED и generation/reset. Один existing C011 preparation case подтвердил actual socket applied-once + deferred FIFO. При review исправлены sticky decode errors, stat bounds, semantic colors/thresholds, source spans, empty labels, preference fallback и повторный Shift+C/TEXT_INPUT leak. Legacy/shared исходники и identity boundary C011 не менялись.

Для каждого из 81 obligations evidence различает checked core и pending combined flows. Canonical snapshot остаётся исходным planning snapshot с pending registry statuses; full live socket lifecycle C014, map C013 и macro/audio recipients не принимаются этим результатом. Пользовательская приёмка не заявлена.


## Повторное открытие native UX — 2026-11-11

Пользователь указал, что предъявленный снимок не соответствует HTML-макету. Подтверждено: прежний native renderer использовал произвольную компоновку ресурсов/таблицы и не воспроизводил нормативные карточки и визуальную структуру макета. Прежние native captures подтверждают работоспособность renderer и читаемость тестовых значений, но **не подтверждают выполнение UX-критерия 006**. Утверждение о полной технической готовности 006/007 отозвано; core evidence 001–005 сохраняется. Исправление renderer и повторная визуальная сверка выполняются в существующих задачах 006/007.


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


## Финальная парная проверка — 2026-11-12

Реальные HTML/SV normal и Character primary кадры1932×1208 проверены после исправления006-V1–V4. Независимый reviewer не оставил новых блокирующих замечаний в C012 scope; Linux build/core и четыре bounded runs PASS. Small1024×768 — дополнительный offscreen fit regression, не отдельная браузерная пара. CascadiaMono UI согласован; неизвестные данные и deferred surfaces не подменены demo. [Снимки, точные команды и ограничения](../../sv-c012-evidence.md#итог-реального-парного-сравнения-htmlsv--2026-11-12). Пользовательская приёмка и canonical runtime statuses pending.


Финальное уточнение2026-11-12: после первого review root нашёл трёхсекционный header правого Character widget вместо HTML `PROFILE / SKILLS` + `RESISTS`. Widget header/click extent исправлены, три секции primary сохранены. Прежний snapshot сохранён в `browser-visual/pre-widget-header-fix/`; build/core и четыре native capture повторены, независимый reviewer заново просмотрел все кадры и не оставил новых блокирующих C012 findings. Актуальные SHA, exitcodes и граф — в последнем разделе evidence.

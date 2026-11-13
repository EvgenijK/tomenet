# C012 — Linux production core evidence

## Приёмка — 2026-11-13

Пользователь подтвердил закрытие C012 в проверенных границах Linux status/core: [решение о приёмке](sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются открытыми в реестре отсрочек; полный canonical runtime PASS не заявлен.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](tasks/c012/final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

## История решения об отсрочке приёмки интерфейса — 2026-11-13

**Интерфейс в C012 больше не дорабатываем; его дальнейшая визуальная/пользовательская приёмка отложена.** [Реестр C012-DA-01–10](tasks/c012/deferred-acceptance.md) задаёт принимающие этапы, условия закрытия и взаимные ссылки. Прежние visual correction gates ниже сохраняются как история; они не требуют продолжать оформление или снимки в C012 сейчас. Отсрочка не означает PASS, завершение C012 или перенос canonical 27/81.

В C012 остаются финальный минимальный existing focused core check последнего implementation candidate, semantic contract Study/stat index, сохранность данных/событий и точный disposition обязательств. Техническую часть можно завершать без ожидания поздней визуальной приёмки. Последний existing-slots freeze3 содержит исправления stat/encumbrance/Sanity/XP и resize/reflow; его финальные core и affected visual проверки после команды «стоп» не завершены. Старые snapshot PASS не подтверждают этот финальный кандидат. Новых блоков Character не добавлять; RESISTS unknown/DUMP unavailable остаются согласованными.

## История результата до отсрочки

2026-11-12. **Freeze5: исправления Character проверены независимыми парными снимками и вводом при1932×1208; пользовательская приёмка и полнота C012 UI остаются открытыми.** Ниже сохранена история прежних проверок и пропущенных расхождений; текущий результат указан отдельно.

Canonical allocation/statuses не изменены: **27 outcomes / 81 obligations**, exact snapshot — [coverage](tasks/c012/coverage.json). Этот документ различает проверенные core subflows и непроверенные combined flows. Полный runtime PASS canonical obligation не заявляется.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Текущий остаток уточнён по baseline и реально показанному HTML: прежние пять отсутствующих «labels» и rawfields не являются автоматически пятью новыми блоками/строками. **Доказанные адресные дефекты:** stat colour priority/base118 и weapon encumbrance precedence в существующих слотах. Sanity numeric имеет существующий режим; stat/level/XP используют условные/derived значения. Self AFK/study/opaque extra и bank не получили подтверждённого места HTML; studycount требует source/contract разрешения. Idle/martyr — реакции/сообщения, не новые Character labels. Unknownbits сохраняются без обязательнойhexстроки. Canonical27/81неизменны; полнаяUIприёмка не заявлена.

[План правок задач](../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md), [scope audit](../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и снимки](../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md). Задачи003–007уточнены,002дополнена existingSanitymode. Production в этой planningволне не менялся. Новый Character StatusDetails отклонён и не возвращается; implementation пока не запускается. Минимальные Linux small/resize/temporal и необходимые existingcorecases записаны как предложение после согласованных изменений; это не разрешение расширить suite/harness.

## Текущий результат исправлений Character — freeze5, 2026-11-12

Исправлены реальные PROFILE/SKILLS/RESISTS: выбор страницы, active state, композиция, компактная высота RESISTS, связанный правый виджет, close/reopen retention и ограниченная фактическим содержимым прокрутка. Исправлены тон панели/шапки, подпись Level, заголовки и формат Blows/Round. Формы14 source-иконок перенесены из HTML; обнаруженный дефект blur-only исправлен чётким SVG foreground.

Пользователь одобрил RESISTS с неизвестными значениями и явно недоступный DUMP. Предложение `+ STATUS DETAILS` **отклонено** после показа снимков: переключатель, список и связанные state/hit regions полностью удалены. В SKILLS только секции макета; новый заменяющий блок не добавлен. Старые proposal/freeze3/freeze4 снимки исторические.

Независимая реальная проверка при1932×1208: три HTML/SV пары, переключения, DUMP unavailable notice, закрытие/повторное открытие, widget sync, fitting wheel/PageDown — без новых блокирующих presentation/input findings в этом состоянии. Native838frames/22PNG/exit0, frozen source/binary стабильны. [Отчёт и границы проверки](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/visual-review.md); пары: [Profile](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-profile.png), [Skills](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-skills.png), [Resists](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/comparison-resists.png).

Linux build и существующий focused production status check — exit0; [команды/логи/SHA](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/checks/final-checks.json). HUD SHA `56092ac705846c33e4ffb5d4350e9542b9f73861d323965f6a57238706526a91`, binary `710bd1d4679221b9b2c4ee598a22acb3a6e015d65d50c5a5d1c3f07d332c1fe6`. Обновлён существующий code graph:29331nodes/72309edges/1475communities, exit0. Новая suite/harness не добавлялась.

**Остаток C012 UI:** после удаления дополнительного списка нет самостоятельного видимого вывода study,AFK,extra,martyr,audio-idle; также больше не показан ряд расширенных значений bank/attributes/XP/sanity/concurrent encumbrance. Backend/derived rows сохранены, но это не видимое UI. [Точный список и семантические исключения](../.scratch/sv-c012/browser-visual/character-fix-2026-11-12/implementation.md). Нельзя объявлять все27outcomes/81obligations или весь Character принятыми. Canonical coverage не изменён; пользовательская приёмка остаётся открытой. Это не разрешение вернуть технический список в Character. Полные RESISTS data/export, live session, small paired capture и temporal QA текущим visual review не подтверждены.

## Production путь

`tests/sv/status.c` подаёт независимые baseline packet fixtures через `sv_app_receive` → `sv_app_step` → production protocol → session model → `SvAppView.hud`; semantic state не присваивается из тестов. Production `sv_hud_prepare` вызывается для граничных semantic presentation cases. Shared `tests/sv/status-fixture.h` содержит 27 wire fixtures для core и native diagnostic caller, без test-side decoder/semantic behavior.

Renderer и подготовка HUD находятся в `src/client/sv/ui/hud.[ch]`; bounded diagnostic entry из `tests/sv/diagnostics` использует тот же production API. Bounded native visual/input evidence приведено ниже; actual C014 lifecycle им не подтверждается. C011 identity boundary сохранена, полный socket→live adoption/close/relog остаётся C014.

## Выполненные команды и результаты

- `make -C src -f makefile.sv sv-status-check -j4` — **PASS**. Binary: `src/.sv-build/linux/bafdc534e11c1a7cb47f/checks/status`; log: `.scratch/sv-c012/status-build.log`. Link использует production closure.
- `src/.sv-build/linux/bafdc534e11c1a7cb47f/checks/status` — **PASS**. `.scratch/sv-c012/status.stdout.log`, `.scratch/sv-c012/status.stderr.log`.
- `make -C src -f makefile.sv sv-first-session-check -j4` — **PASS**, `.scratch/sv-c012/preparation-build.log`.
- Один existing `first-session <isolated-profile> <copied-lib> c011-preparation` — **PASS**. Exact command/environment: `.scratch/sv-c012/preparation/command.json`; stdout/stderr и protocol transcript в том же каталоге. Использованы loopback managed peer, dummy SDL/software renderer, отдельные profile/shared и временная копия lib. Обычный sandbox запрещал создание socket; тот же разрешённый loopback check выполнен с escalation. Остальные 10 cases C011 и broad suite не запускались.

В текущей Linux конфигурации `src/config.h` определяет `SHOW_SANITY`; SV semantic decoder зарегистрирован и линкуется, sanity branches вызваны через app. Матрица иных compile guards этим не заявляется. Historical common Lua/core compile warnings не изменялись; новых runtime failures в описанных focused checks нет.

## Проверенные core cases

| Case в status.c | Наблюдаемый результат |
|---|---|
| `latest_all_fields` | Все 27 availability bits; точные independent expected значения ресурсов, stats, armour, body/demographics, combat/experience/currency, conditions/indicators; view содержит копии |
| `split_and_adjacent` | Каждый из 27 receivers: незавершённый packet не публикуется; завершение вместе с GOLD sentinel потребляет ровно свои bytes, две атомарные updates |
| `version_defaults` | HP 2, stats 2, armour 3 interpretation, experience 2, sanity 3, encumbrance 3, blows 2 branches; old absent defaults, split+adjacent |
| `depth_speed_and_semantics` | 4 DEPTH layouts, old sector green/default preposition, оба порядка DEPTH/SPEED, no-tele clear без нового speed packet, negative speed; encoded poison/blind/stun и clear |
| `baseline_presentation_boundaries` | Production HUD rows для food999/1000/1999/2000/2999/3000, cut799/800, blind3, known indicator labels и unknown bit raw u32; resize/reprepare без model mutation |
| `hp_occurrences_and_projection` | Low-HP warning не повторяется от mana/AFK или UI rebuild, одинаковый полный HP повторяет baseline occurrence; HP revision не coarse HUD revision; marker equality boundaries |
| `effects_reset_and_errors` | Player→Wolf→Player сохраняется при нескольких inputs до draw, повторная Wolf не дублирует body change; idle1/0 FIFO; take consumes один occurrence, overflow explicit; open/reset удаляет availability/queue, stale generation rejected; invalid stat index/overlong text не публикуются |
| Scoped existing `c011-preparation` | Actual socket → retained app применяет INDICATORS sequence8 ровно один раз; следующие 21 exact deferred frames/557 bytes выдаются FIFO с sequence9 onward; drain не меняет HUD revision1; общий ingress30, raw tail сохраняется |

Проверены coherent read-only model/projection и core reset/new generation. Actual OS focus/monitor lifecycle, полная normal/big-map композиция и actual C014 relog/teardown не следуют из этих fixtures и остаются непроверенными. Pixel-perfect acceptance не входит в контракт.

## Disposition всех 81 obligations

Ниже каждый ID сохранён явно. **CORE CHECKED** означает только приведённые subflows. Для каждого `.lifecycle` core reset/stale/interleaved/repeated-read проверены общим production-path case; actual C014 socket lifecycle остаётся **PENDING**. Для каждого `.allocation-checks` matching listed wire/semantic/default/split branches проверены; native combined normal/big/focus/resize и поздние recipient flows не получают автоматического PASS. Их собственное native C012 evidence добавляется отдельно; remaining C014/C013/recipient combined flows остаются pending своих owners.

### `capability.status.read-stats`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; stat boost/default/index, armour semantic branches, exact combat/currency, experience old previous-threshold default.

- `obligation.status.read-stats.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-stats.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-stats.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-armour`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; stat boost/default/index, armour semantic branches, exact combat/currency, experience old previous-threshold default.

- `obligation.status.read-armour.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-armour.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-armour.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-demographics`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; stat boost/default/index, armour semantic branches, exact combat/currency, experience old previous-threshold default.

- `obligation.status.read-demographics.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-demographics.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-demographics.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-combat-bonuses`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; stat boost/default/index, armour semantic branches, exact combat/currency, experience old previous-threshold default.

- `obligation.status.read-combat-bonuses.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-combat-bonuses.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-combat-bonuses.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-experience`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; stat boost/default/index, armour semantic branches, exact combat/currency, experience old previous-threshold default.

- `obligation.status.read-experience.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-experience.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-experience.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-currency`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; stat boost/default/index, armour semantic branches, exact combat/currency, experience old previous-threshold default.

- `obligation.status.read-currency.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-currency.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-currency.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-mana`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; HP per-packet/default/boost/bar, sanity registration + numeric/damage availability, mana/stamina markers.

- `obligation.status.read-mana.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-mana.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-mana.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-confusion`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-confusion.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-confusion.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-confusion.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-poison`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-poison.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-poison.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-poison.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-study`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-study.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-study.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-study.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-blows`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-blows.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-blows.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-blows.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-food`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-food.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-food.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-food.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-fear`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-fear.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-fear.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-fear.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-speed`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-speed.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-speed.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-speed.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-cut`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-cut.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-cut.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-cut.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-blind-hallucination`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-blind-hallucination.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-blind-hallucination.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-blind-hallucination.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-stun`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `depth_speed_and_semantics`, `baseline_presentation_boundaries`; encoded state/clear, speed marker/no-tele, blows old/new labels, food/cut thresholds.

- `obligation.status.read-stun.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-stun.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-stun.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-monster-health`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`, `effects_reset_and_errors`; monster separate HP, AFK, all15 encumbrance/version defaults, exact opaque bytes/u32 unknown bits, occurrence retention.

- `obligation.status.read-monster-health.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-monster-health.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-monster-health.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-sanity`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; HP per-packet/default/boost/bar, sanity registration + numeric/damage availability, mana/stamina markers.

- `obligation.status.read-sanity.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-sanity.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-sanity.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-afk`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`, `effects_reset_and_errors`; monster separate HP, AFK, all15 encumbrance/version defaults, exact opaque bytes/u32 unknown bits, occurrence retention.

- `obligation.status.read-afk.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-afk.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-afk.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-encumbrance`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`, `effects_reset_and_errors`; monster separate HP, AFK, all15 encumbrance/version defaults, exact opaque bytes/u32 unknown bits, occurrence retention.

- `obligation.status.read-encumbrance.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-encumbrance.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-encumbrance.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-stamina`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; HP per-packet/default/boost/bar, sanity registration + numeric/damage availability, mana/stamina markers.

- `obligation.status.read-stamina.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-stamina.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-stamina.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-extra`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`, `effects_reset_and_errors`; monster separate HP, AFK, all15 encumbrance/version defaults, exact opaque bytes/u32 unknown bits, occurrence retention.

- `obligation.status.read-extra.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-extra.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-extra.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-martyr`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`, `effects_reset_and_errors`; monster separate HP, AFK, all15 encumbrance/version defaults, exact opaque bytes/u32 unknown bits, occurrence retention.

- `obligation.status.read-martyr.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-martyr.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-martyr.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-idle`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`, `effects_reset_and_errors`; monster separate HP, AFK, all15 encumbrance/version defaults, exact opaque bytes/u32 unknown bits, occurrence retention.

- `obligation.status.read-idle.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-idle.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-idle.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-indicators`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`, `effects_reset_and_errors`; monster separate HP, AFK, all15 encumbrance/version defaults, exact opaque bytes/u32 unknown bits, occurrence retention.

- `obligation.status.read-indicators.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-indicators.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-indicators.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

### `capability.status.read-live-hp`

Core evidence: `latest_all_fields`, `split_and_adjacent`, `version_defaults`; HP per-packet/default/boost/bar, sanity registration + numeric/damage availability, mana/stamina markers.

- `obligation.status.read-live-hp.result` — CORE CHECKED: production decode → semantic view с точными значениями/availability; native readable consumer подтверждён общим C012 HUD/Character path и ограниченным X11/software review ниже; actual C014 остаётся pending.
- `obligation.status.read-live-hp.lifecycle` — CORE CHECKED: interleaved complete inputs, read-only projection/rebuild, reset/new generation и stale rejection; actual socket adoption/close/relog и полный focus lifecycle PENDING C014.
- `obligation.status.read-live-hp.allocation-checks` — CORE CHECKED: соответствующие baseline wire/semantic/default ветки и split+adjacent packet; normal/big live-map combined layout и actual reconnect flows PENDING C013/C014. Applicable current Linux build linked; иные macro/provider/platform matrix не заявлены.

## Границы и оставшееся evidence

Late macro/audio recipients C052/C053/C055 остаются pending: C012 сохраняет occurrence-aware данные, но не заявляет исполнение этих возможностей. Обязательное сохранение existing HP alerts проверено. Full information C030 и live map C013 не принимаются status-only read projection. Windows/MinGW/Wine по правилу пользователя отложены. Canonical registry registration и пользовательская приёмка — отдельные действия.

Верхняя core-сводка проверена агентом checks. Финальная Linux production build/native review и source snapshot сверены оркестратором; результаты приведены ниже.


## Финальная проверка оркестратором

- `make -C src -f makefile.sv -j4 shell sv-status-check sv-first-session-check` — **PASS**, `.scratch/sv-c012/final-build.log`. Production binary `src/tomenet-sv`, build `linux-bafdc534e11c1a7cb47f`.
- `src/.sv-build/linux/bafdc534e11c1a7cb47f/checks/status` — **PASS** после финальной input correction. `.scratch/sv-c012/final-status.stdout.log`/`stderr.log`; stderr пуст. Tiny native input case вызывает production prepare/event: Shift+C opens, repeat stays open, TEXT_INPUT consumed, PgDn/Esc, quantity/PAUSE/server request ownership, stale generation/reset.
- Финальный двухкадровый X11/software native run — **PASS**: production binary, shared raw fixtures → app → renderer, small + Character, 1024×768. Exact command/env: `.scratch/sv-c012/final-native-command.json`; `.scratch/sv-c012/final-native.log`, `.bmp`/`.png`. Снимок просмотрен оркестратором; values доступны, critical HUD не перекрыт, borders/text readable.
- Дополнительно просмотрены агентом и оркестратором bounded native captures `.scratch/sv-c012/{wide,small-character,small-bottom,scaled}.png`. Wide1280×800, small1024×768, Character top/bottom и UI150%; right hidden small, numeric/condition/encumbrance/extra/known+unknown indicators доступны через scroll, rebuild не теряет primary/scroll. Exact reproduction и UX SHA приведены в [006](tasks/c012/SV-C012-006-native-hud.md). Это ограниченный native review, не полная visual/provider matrix.

Backend/model/app и `tests/sv/first-session.c` final SHA совпадают с проверенным preparation snapshot; повтор всей C011 suite не требовался. Финальные source/artifact hashes: `.scratch/sv-c012/final-source-hashes.json`. Independent backend review выполнен UI и checks агентами, UI review — backend агентом и оркестратором. Новых незакрытых findings собственного C012 scope нет.

Исправлены найденные при реализации/review sticky decoder error, invalid stat bounds/precedence, starvation/cut thresholds, semantic color roles, known indicator meanings/build guard, formatted source spans, empty text drawing, usable preference fallback, critical height fit и Shift+C/TEXT_INPUT leak. Только scoped SV changes; legacy/shared implementation не менялась. Отдельный legacy stat-index hardening записан в sv-improvements и не включён в эту реализацию.

Задачи 001–007 имеют статус implemented/Linux core checked; пользовательская приёмка не заявлена. Все late combined flows в disposition выше сохраняют pending. Canonical allocation/conditions/implementation/evidence statuses не изменены.


Code graph обновлён командой `.graphify-venv/bin/graphify update .` — exit0, 29081 nodes / 71725 edges; log `.scratch/sv-c012/graph-update.log`. CLI выполнил code AST update; semantic doc rebuild не запускался. Предупреждения extractor о частично разбираемых legacy/preprocessor sources не являются ошибками production compiler.


## Корректировка UX после замечания пользователя — финальный snapshot

Предыдущее заявление о технической готовности native UX было слишком широким: старые снимки подтверждали запуск/читаемость, но не соответствие нормативной структуре HTML. Исправление ограничено `src/client/sv/ui/hud.c` и `hud.h`; SHA остальных source файлов совпадают с ранее проверенным snapshot, включая backend и C011 preparation.

Нормативные DOM/CSS источники расширены: `index.html`, `css/foundation.css`, **финальные** слои `css/theme.css`, `css/hud.css`, `css/right-panel.css`, `css/character-window.css` и связанные character/hud JS. Их SHA — `.scratch/sv-c012/ux-correction/reference-hashes.json`. Более ранний `reference-full.png` не является текущим DOM screenshot. Browser-open file:// был запрещён URL policy; обход не выполнялся, browser-rendered reference не заявляется.

Проверены structural criteria: порядок левых карточек, XP/level diamond, Gold/encumbrance/attributes, sanity/armour, framed resource meters, tracked health, speed/blows, fixed8-column active/inactive status grid, structured Character overview/combat/details и supplemental values через scroll. Сохранены baseline stat notation/linear_stats, No-Tele/Wraith, colour roles/source spans, unknown indicator bits, absence/default semantics. st_huge увеличивает горизонтальную stamina; применимые HP/MP/SN huge показывают выбранные vertical meters с центральными значениями. Unknown resources и old sanity без numeric data не получают authoritative0/0.

Финальные проверки:

- `make -C src -f makefile.sv -j4 shell sv-status-check` — PASS, `ux-correction/build-final.log`; existing `checks/status` — PASS, `status-final.stdout.log`, stderr пуст. Новая suite не создавалась.
- Standalone X11/software current-source smallCharacter1024×768 и wide1280×800, по2кадра — PASS: `x11-final.log/png`, `x11-wide.log/png`. Root просмотрел оба финальных кадра.
- SDL dummy/software **offscreen** по2кадра: wide/small/Character/UI150%/scrolled details — PASS, `offscreen-*.log/png`; независимый агент просмотрел все5 кадров и исходники. Это дополнительное renderer evidence, не подмена X11 результата.
- Source/artifact SHA: `.scratch/sv-c012/ux-correction/final-source-hashes.json`; точные commands/env/exitcodes: `final-commands.json` в том же каталоге.
- В одном повторном batch шесть X11 процессов зависли и были завершены TERM/KILL; `*-final.log` и `native-commands.json` сохраняют этот неуспешный повтор. Из отсутствия buffered stdout нельзя локализовать зависание. Последующие одиночные bounded X11 запуски с построчным логом прошли exit0; первопричина batch hang не установлена. Итоговый source snapshot подтверждают только `x11-*`/`offscreen-*` captures, более ранние `*-final.png` к нему не относятся.
- `git diff --check` PASS; code graph update exit0, 29098 nodes/71756edges, `ux-correction/graph-update.log`; semantic doc rebuild не выполнялся.

Независимый final review не выявил новых блокирующих findings по указанным structural/semantic criteria. **Оставшиеся визуальные различия:** декоративные attributes/encumbrance symbols упрощены относительно SVG, texture/typography приближённые. Полного визуального совпадения с HTML и пользовательской приёмки не заявляем. C013/C014 и late-recipient combined obligations по-прежнему pending своих owners.


## HTML fidelity — итоговая проверка иконок и оформления

2026-11-11, после прямого запроса пользователя довести HUD до соответствия текущему HTML, включая icons/theme. Три отдельных owner: exact SVG assets, native CSS styling и независимый appearance audit. Root собрал и проверил frozen final sources. Этот итог заменяет прежние оговорки об оставленных упрощённых symbols/flat theme; historical captures выше сохранены как история.

### Source-derived иконки

В production добавлены `src/client/sv/ui/hud-icons.[ch]`, generated `hud-icons-data.h` и source snapshots/generator в `ui/hud-icons/`. Все53 иконки: 6 attributes, 15 encumbrance, 29 status, sanity/armour/Character. Path/primitive geometry сохранена из текущих HTML/JS; внешние SVG stroke widths применены согласно CSS. Источники и SHA: `ui/hud-icons/sources.json`. Production использует embedded96×96 alpha masks с renderer-owned texture cache; авторские rsvg-convert/Pillow не нужны во время сборки или запуска клиента.

Независимый агент сравнил все53 SVG primitives/tag attributes с оригиналами, проверил source/snapshot SHA и побайтное равенство compiled masks свежей rasterization оригиналов. Повторная генерация `python3 src/client/sv/ui/hud-icons/generate.py` без prototype аргумента дала byte-identical generated header. Новых runtime-зависимостей нет.

### Проверенные appearance criteria

| Область | Подтверждено source audit и native review |
|---|---|
| Панели/карточки | Final forged-surface radial/repeating/linear layers, forged-inset, frame/inset roles и 5px brass separators |
| XP/Gold | Radial level diamond/inner frames, white/high/drain presentation, borderless amber jagged drain segment, CSS radial Gold coin |
| Компактная геометрия | Attributes132px без лишнего заголовка, six exact18px SVG и индивидуальные цвета; 15 encumbrance states в12 grouped slots |
| Ресурсы | Framed enamel meters с actual palette: ordinary mana cyan; huge mana blue/sanity green/HP red; coloured empty backgrounds, reflections и central gradient badges |
| Состояния | 24 slots/8 columns, compact22px tiles, exact source glyph roles, active/inactive styling, radial/critical diagonal/glow; explicit critical flag отдельно от severity |
| Анимация | Один presentation frame phase для critical background/border/SVG/shield mark; 1.25s alternate timing. Time behaviour проверено по production code, а не полной frame-by-frame matrix |
| Character | Widget9px/primary12px roles, gold name/overview1.15:1/combat/details, dotted rows, responsive detail columns, scroll и requested UI scale |
| Семантика | Baseline authoritative values/colours, unknown availability, Wraith/No-Tele, source spans и все owned supplemental values сохранены; long source strings wrap в profile, label clip width ограничен |

CSS/SVG source paths и SHA — `.scratch/sv-c012/html-fidelity/reference-hashes.json`; snapshot за время работы не изменился. Нормативные final CSS layers включают foundation/theme/hud/right-panel/character-window/shortcuts и связанные character/hud/data sources. Source identity не подменяет игровой baseline демонстрационными значениями.

### Финальный frozen source snapshot

Из ранее проверенных production/build source файлов изменены только `src/client/sv/ui/hud.c` и `src/makefile.sv` (регистрация icons object); добавлены icon module/assets. Backend/protocol/model/preparation не менялись, предыдущий core/C011 evidence сохраняется. Полный SHA snapshot, включая generator/SVG/masks и binary, — `.scratch/sv-c012/html-fidelity/final-source-hashes.json`.

- `make -C src -f makefile.sv -j4 shell sv-status-check` — **PASS**, `html-fidelity/build-final.log`.
- Existing `checks/status` — **PASS**, `status-final.stdout.log`, stderr пуст; production-path27 outcomes/default/wire/occurrence/reset checks сохранены. Новая suite не создавалась.
- Final standalone X11/software two-frame `hud-current` wide1280×800 и `character-current` small1024×768 — **PASS**, соответствующие `.log/.bmp/.png`. Exact commands/env/exitcodes — `html-fidelity/final-commands.json`. Они выполнены после всех известных fixes и относятся к final SHA snapshot.
- На предшествующих промежуточных frozen snapshots просмотрены ordinary1920×1080, huge1280×800, small1024×768, Character, UI150% и scrolled details (`*-v2.png` и ранние `*-final.png`). Последующие изменения касались critical glyph animation и декоративных details; не выдаём эти промежуточные кадры за final-source captures.
- `git diff --check` — PASS. Final code graph update после последней правки — PASS (exit0), log `html-fidelity/graph-final.log`; semantic doc/image rebuild не заявляется.

Независимый агент просмотрел обе последние native images, проверил все известные source findings и сообщил: **открытых appearance findings C012 нет**. Root также просмотрел final frames. Первый capture attempt с новым isolated profile был отклонён из-за отсутствия synthetic marker; marker добавлен только в созданные root scratch profiles, после этого все описанные fidelity X11 runs прошли. Это setup correction, не production renderer failure.

### Границы результата

Сверка выполнена по текущим DOM/CSS/SVG и native captures. Browser-rendered side-by-side отсутствует: ранее локальный file:// open был запрещён URL policy, обход не выполнялся. Попиксельное тождество браузерному raster не является контрактом; native blur/antialias сохраняют source roles/parameters и geometry, выбранный пользователем text font не подменяется. Это не оговорка об оставленных generic icons или плоской теме — они заменены. Не заявляется полная visual/provider/input/version matrix.

Пользовательская приёмка остаётся отдельным решением; canonical81 obligations не получают blanket runtime PASS. C013 map, C014 actual lifecycle и late recipients сохраняют свои pending owners.


## Повторная сверка после удаления устаревшего PNG

2026-11-11. Предыдущий общий appearance PASS оказался слишком широким. Root и новый независимый read-only агент использовали только текущие index.html/CSS/JS/SVG, повторно проверили source SHA и native mapping. Удаление PNG не меняет renderer: он не загружает этот файл.

Подтверждены четыре группы открытых findings: пропущенное dynamic panel width правило `app.js:38–40` (native318/486 вместо HTMLdefault262/394 при1280px); ошибочный PKT_IDLE→Resting icon mapping; несовпадающие condition RGB для states без server attribute; отдельная96px raster→small SDLlinear цепочка и shifted-copy glow вместо SVG-at-size/CSS shadow. Подробности и границы численного анализа — в `docs/tasks/c012/SV-C012-006-native-hud.md`, последний раздел. Само совпадение всех53 SVG primitives/sourceSHA остаётся подтверждённым. Native fixture и HTML default также используют разные active states; их нельзя считать равнозначными appearance scenarios.

Задачи006/007 снова открыты; текущая реализация не получает общий appearance PASS. Core/build проверены на неизменившихся production sources; канонические runtime statuses и пользовательская приёмка не менялись. В этом review код не правился, новые runtime suites/harness не запускались.


## Корректировка R1–R4 — финальный frozen-source результат

2026-11-11. По запросу пользователя агенты сначала переписали актуальные требования006/007, независимый reviewer подтвердил их приоритет/границы, затем выполнена реализация и повторная проверка. История прежних слишком широких appearance PASS сохранена выше; текущий результат относится только к указанному snapshot и evidence.

### Что исправлено

| Пункт | Production результат | Доказательство |
|---|---|---|
| R1 widths/fitting | JS formula применяется к initial logical viewport один раз, selected logical widths/overrides сохраняются; small fit по actual content extents; current stat в main HUD и полные значения в Character | Existing status production prepare cases:1280→262/394,1920→367/552, resize/DPI/UI-scale/generation retention и override280/420; final matched1280/1920/small/UI150 frames |
| R2 semantic mapping | Нет PKT_IDLE→Resting; отдельный Audio idle; activity unavailable | Incoming IDLE1→production view→prepared condition unavailable/notREST; matched native log `activity_available=0 idle=1`; нет активной луны |
| R3 palettes | State-specific Food/Vision/Confusion/Fear/Poison/Cut/Stun colours/critical/severity из current JS; explicit authoritative attributes сохранены | Existing status case concretevision/confusion/fear/poison expectations и independent source review; stress frame |
| R4 icons/filter | Generated original SVG strokes→FreeType at actual physical size/phase,1:1texture copy; Gaussian chain, cache effects/source identity, whole-group alpha | Production53×3 icon gallery, originalSVG/genprovenance, numeric independent reference; source/error/resource audit |
| Readability | CSS Gaussian shadows для XP/resource overlay text; bounded cache учитывает font_revision | Source audit + final UI150 frame, читаемость белого текста на белой XP fill |

### SVG production coverage

53 original SVG snapshots не менялись. Их path/circle primitives компилируются standard-library authoring generator в108 stroke primitives; generated data теперь vector commands, не96px alpha assets. Runtime использует уже linked FreeType (`FT_Stroker`, final-size glyph bitmap raster) с round caps/joins и сохранёнными stroke widths. Размер, fractional placement1/64px, source generation и effects учитываются в renderer-owned caches. Gaussian drop-shadow chain использует alpha предыдущего composited stage; opacity применяется один раз к tile+glyph+shield mark.

Gallery обнаружила реальный overlap-artifact у Food/других intersecting strokes; он исправлен через `FT_OUTLINE_OVERLAP` и glyph bitmap renderer. После исправления все159 рендеров53icons×15/18/20px непустые, формы и ориентация просмотрены независимо. `python3 src/client/sv/ui/hud-icons/generate.py` воспроизводит generated header побайтно; новых runtime или authoring package dependencies не добавлено.

Сравнение `.scratch/sv-c012/rework/icon-comparison-final.json` читает pixels **production gallery capture**, white255-on-black0, и сравнивает с original SVG, непосредственно растеризованным `rsvg-convert` в том же physical size. Средняя по53 glyphs alpha MAE на шкале0–255:

| Physical size | Production FT | Прежняя аналитическая96px bilinear model |
|---|---:|---:|
|15px|1.257|17.434|
|18px|0.985|13.913|
|20px|1.027|12.221|

Прежняя колонка — аналитическая модель, не старый GPU capture. Direct SVG reference — rsvg/Cairo, **не browser-rendered pixels**. Эта проверка не выдаётся за pixel-perfect HTML acceptance. Script `rework/compare-icons.py` анализирует existing production capture, не реализует test-side runtime renderer.

### Final commands/artifacts

- `make -C src -f makefile.sv -j4 shell sv-status-check` — PASS, `.scratch/sv-c012/rework/build-final.log`. Existing status runner — PASS, `status-final.stdout.log`, stderr пуст. Несколько необходимых assertions добавлены в существующий production case; новая suite не создана.
- Existing diagnostic additions: `--status-matched-icons` подаёт raw packet overlay через app→protocol→model, не присваивает semantic state; `--status-icons` рисует53 glyphs через production icon API в existing single window.
- Final SDL dummy/software: `offscreen-{icons,matched-1280,matched-1920,small,character,scaled,stress,huge}-final.log/.bmp/.png` — все по2кадра PASS. Это offscreen evidence, не доказательство X11 окна.
- Final ordinary **X11/software**: `native-matched-final`1280×800 и `native-character-final`1024×768 — по2кадра, exit0. PNG просмотрены root и независимым агентом, новых visual findings нет.
- Exact final commands/env/exitcodes: `.scratch/sv-c012/rework/final-commands.json`. Source/binary/input-profile/artifact SHA: `final-source-hashes.json` там же. Prototype current HTML/CSS/JS SHA: `reference-hashes.json`; за время работы не изменились.
- Three earlier X11 attempts со stdbuf wrapper зависли до startup line и завершены timeout/KILL137; logs `icons-iteration1.log`, `matched-1280-iteration2.log`, `x11-final.log`. Они остаются unsuccessful, summary `unsuccessful-x11-attempts.json`. Затем тот же binary штатно завершился под gdb и в двух обычных запусках без stdbuf. **Первопричина тайм-аутов не установлена**, нельзя объявить её доказанным дефектом wrapper или renderer.

### Matched-state границы

Matched overlay фиксирует onlyheavy_wield, indicator bits0..4+6 (`0x5f`), normal food, clear confusion/poison/fear/cut/blind/stun; IDLE1 оставлен намеренно. Это контролируемое равенство **icon states**, не всех scalar demo values HTML (HP/XP/stats/identity остаются независимыми packet fixtures). Baseall15-active fixture/core expectations сохранены отдельно. Нет браузерного side-by-side: current DOM/CSS/JS/SVG — единственный visual authority; удалённый PNG не использовался.

Backend/protocol/model/preparation behavior не менялся; source SHA прежних owncore/C011 файлов совпадают. Изменения SV-local в HUD/icons и narrow existing diagnostic/core cases. KnownR1–R4/source/visual findings закрыты в проверенных Linux пределах; full provider/input/version matrix не заявляется. Пользовательская приёмка и canonical81 runtime PASS не заявляются, C013/C014/late recipients сохраняют свои owners.


Финальная сводка сверена 2026-11-12 (Europe/Moscow). Указанные final native runs выполнены 2026-11-11 в23:57; источник после них не менялся. Final code graph update — exit0,29207nodes/72028edges, log `.scratch/sv-c012/rework/graph-final.log`; semantic doc/image rebuild не запускался. `git diff --check` чист.


## Браузерная визуальная корректировка — 2026-11-12, исходный план

Пользователь потребовал сравнивать SV с реальным визуалом целого локального HTML-макета при одинаковом размере окна, затем исправить задачи/реализацию и независимо проверить со снимками обоих вариантов. Прежнее source-to-native сравнение не является этой проверкой. Root успешно открыл локальный макет через HTTP; историческая запись о невозможности localhost больше не является блокером.

Контракты обновлены **до implementation**:006-V1–V4 и paired-browser007. Артефакты текущего прогона — `.scratch/sv-c012/browser-visual/`. Планируются normal и Character primary пары при client area1932×1208, UI100%, wide; manifest фиксирует фактические scales/fonts/widths/state/source hashes. Конкретные findings и итог добавляются после просмотра реальных before/after captures; пока appearance PASS не заявляется. Core/backend и canonical allocation не изменены этой корректировкой.


### Парные before observations

Agent `visual_tasks` лично просмотрел full real-browser normal/Character JPG1932×1208 и native normal PNG того же размера. Browser source/metrics: `.scratch/sv-c012/browser-visual/html-wide-1932.jpg`, `html-character-1932.jpg`, `html-wide-metrics.json`, `html-character-metrics.json`; native before: `.scratch/sv-c012/rework/native-fullscreen-20261112.png`. Fresh reload браузера выбирает370/556, совпадающие с SV; прежний screenshot после resize без reload сохранял262/394 и не годился для вывода о mismatch defaults.

006 обновлена конкретными V1-a/origin+Ping43px, V1-b/vitals119vs128px, V2-a/font roles, V3-a/primary composition и V4-a/state+deferred differences. До просмотра native primary before не выдаётся его appearance disposition. Font-role решение уточнено пользователем2026-11-12: UI font может отличаться от HTML, текущий CascadiaMono допустим, map font независим. Font-family difference — согласованное исключение; geometry/density/size/line-height остаются в scope. Scalar fixture values не равны browser demo, даже при совпадающих active icon states; это записано как ограничение сравнения.


### Уточнение пользователя о шрифтах — 2026-11-12

Пользователь подтвердил независимый выбор map font (legacy options) и UI font; UI font может отличаться от HTML, текущий CascadiaMono допустим. Следовательно, browser serif/native mono не блокирует closure и не требует нового serif asset или map-font change. Оно фиксируется в paired manifest/review как agreed exception. UI geometry/density/text size/line-height/alignment/readability остаются проверяемыми свойствами. Задачи006/007, README и спецификация обновлены до final review.


## Итог реального парного сравнения HTML/SV — 2026-11-12

Исправление006-V1–V4 завершено на frozen source. В отличие от прежних source/native checks, этот результат опирается на лично просмотренные реальные браузерные и native кадры одинакового размера. Независимый агент `visual_independent` просмотрел два стабильных HTML эталона и все четыре финальных SV кадра; новых блокирующих findings в применимой C012 части не осталось. Пользовательская приёмка остаётся pending. Полное совпадение всего интерфейса или попиксельное тождество не заявляется.

### Условия и снимки обеих реализаций

Локальный reference: `/home/svechnik/Projects/github_site/tomenet_interface`, URL `http://127.0.0.1:8765/`, Chromium через IAB. Browser client area1932×1208, DPR1, fonts loaded, fresh reload после установки viewport; selected initial widths370/556. Финальные HTML кадры сняты при opacity1: сразу после reload был замечен промежуточный opacity0.698669, поэтому переходный кадр не использован для финального alpha/colour judgement.

SV main пары: настоящее Linux X11/software fullscreen, output1932×1208, display scale1, UI100%, wide, selected widths370/556. Использован существующий bounded caller с `--status-check --status-html`, по2кадра; personal profile не открывался. Исходники и binary оставались неизменными между build, core, capture и финальной сверкой. Manifest: [comparison-manifest.json](../.scratch/sv-c012/browser-visual/comparison-manifest.json).

| Состояние | Реальный HTML | Финальный SV |
|---|---|---|
| Normal HUD,1932×1208 | [html-wide-final.jpg](../.scratch/sv-c012/browser-visual/html-wide-final.jpg) | [sv-wide-final.png](../.scratch/sv-c012/browser-visual/sv-wide-final.png) |
| Character primary,1932×1208 | [html-character-final.jpg](../.scratch/sv-c012/browser-visual/html-character-final.jpg) | [sv-character-final.png](../.scratch/sv-c012/browser-visual/sv-character-final.png) |
| Supplemental details scroll,1932×1208 | Сравнивается применимая C012 часть, отдельного demo-equivalent content нет | [sv-character-details-final.png](../.scratch/sv-c012/browser-visual/sv-character-details-final.png), actual X11 |
| Small Character,1024×768 | Не отдельная HTML/native пара | [sv-small-character-final.png](../.scratch/sv-c012/browser-visual/sv-small-character-final.png), SDL dummy/software offscreen supplement |

Полные парные полотна без изменения размеров оригиналов (HTML слева, SV справа, добавлен32px заголовок): [HUD](../.scratch/sv-c012/browser-visual/comparison-hud-final.png), [Character](../.scratch/sv-c012/browser-visual/comparison-character-final.png). Это удобная композиция исходных кадров, не новый render и не преобразование для сокрытия различий. Оригиналы сохранены.

### Исправления и независимый disposition

| Критерий | Исправленный результат и проверка |
|---|---|
| V1: HUD geometry | Учтён1px panel border: initial block origin14,12. Добавлена43px строка Ping из уже доступного `SvPingTelemetry`; при отсутствии ответа показано unavailable `--`, а не demo193ms. Stats начинаются на y185, vitals block119px, status header y590/grid y615; сохранены chosen widths и manual layout. Character shortcut расположен в своей нижней cell; остальные deferred actions не имитируются. Независимый normal whole-frame review новых блокирующих замечаний не оставил. |
| V2: appearance | Уточнены XP/level origin, card extents, фон/insets/frames и применимые эффекты; SVG53 и production icon pipeline сохранены. CascadiaMono UI оставлен по явному ответу пользователя, без нового serif asset и map-font change. Шрифт, baseline colours и неизбежно отличающиеся данные явно отделены от geometry/fit. |
| V3: Character | Правый widget занимает предусмотренную нижнюю область и имеет независимый scroll; его header повторяет две секции HTML: `PROFILE / SKILLS` и `RESISTS`. У primary отдельно сохранены три секции `PROFILE`, `SKILLS`, `RESISTS`. Primary центрирован относительно всего окна: около x507,y206.7,w918,h794.6 при main target; head38px, close target38px, оформление tabstrip и dim/saturate/blur backdrop. Profile использует две верхние колонки и три combat колонки; current stat notation не дублируется как current/max, полные поля сохранены ниже. Недоступные C030 данные обозначены, отключённые вкладки не объявляются реализованными capabilities. Title/shortcut открывают primary, modal pointer events не уходят в gameplay. |
| V4: state/data | Новый узкий `--status-html` overlay доставляет raw bytes через production app→protocol→model; independent27-field base fixture и прежний matched-icons stress остаются прежними. Сопоставлены представимые HP/MP/ST, опыт/level/gold, stats, armour, speed/blows/combat/body и active icons. Никакой test-side renderer/model implementation не введён. Missing availability, extra status details, unknown data и generation semantics сохранены. |

При проверке найдено дополнительное clipping в combat labels small primary: искусственный48% cap ограничивал подпись, хотя справа оставалось место. Ограничение заменено расчётом фактической ширины значения/подписи; final small кадр переснят и независимо просмотрен. Предыдущие кадры/manifests сохранены в `browser-visual/pre-label-fix/`, их appearance finding не переименован в PASS.

После первоначального independent review root заметил ещё одно реальное расхождение: правый Character widget имел три раздельные вкладки вместо двух HTML секций `PROFILE / SKILLS` + `RESISTS`. Исправлены только widget header и его click extent; трёхсекционный primary tabstrip сохранён. Предыдущий snapshot/manifest остаётся в `browser-visual/pre-widget-header-fix/`. После этого исправления повторены build/core, все четыре native captures и обе composite пары на новом source SHA. Независимый reviewer заново просмотрел все четыре native кадра против стабильных HTML эталонов: header исправлен, small labels полные, новых блокирующих V1–V4 findings нет. Это уточнение не скрывает дефект, пропущенный первым review.

Backdrop cache учитывает coherent view/generation/redraw/font/geometry/preferences и independent scroll. Для присутствующего critical condition/pulse используется100ms time bucket, чтобы backdrop не замораживал активный визуальный сигнал. Final frozen matched scene не содержит critical pulse; temporal animation acceptance этим статическим review не заявляется.

### Проверки frozen snapshot

- `make -C src -f makefile.sv -j4 shell sv-status-check` — PASS, [build-final.log](../.scratch/sv-c012/browser-visual/build-final.log), без compiler warning/error diagnostics.
- Build/core commands и явные exit0 сохранены в [final-checks.json](../.scratch/sv-c012/browser-visual/final-checks.json).
- Existing production status runner — PASS, [status-final.stdout.log](../.scratch/sv-c012/browser-visual/status-final.stdout.log), stderr пуст. В существующем case добавлены минимальные assertions production event path для независимого widget scroll/title opening/modal pointer ownership; прежняя точная small scale assertion заменена проверкой фактического вписывания с сохранёнными выбранными widths. Новая suite/harness не создавалась.
- Все четыре финальных runs — exit0, по2кадра. Точные команды, env/providers и capture paths: [final-commands.json](../.scratch/sv-c012/browser-visual/final-commands.json). Primary details использует existing `SV_STATUS_SCROLL=1800`; small1024×768 — offscreen supplement, не подмена основной X11/browser пары.
- Source/binary/artifact SHA: [final-source-hashes.json](../.scratch/sv-c012/browser-visual/final-source-hashes.json). HUD source SHA256 `3cd73266ce2ed40b38e101dfc5d6661c98aba0db89d27116aee47772faed5bd5`; binary SHA256 `85962baca929f65685f13511e77628aa569c2ff065ad653138edd668aed48b9d`. `sourceStableDuringCaptures=true`, `sourceStableAfterGraphUpdate=true`; зафиксированы75 source files. Все116 reference HTML/CSS/JS/SVG hashes сохранились, [reference-hashes.json](../.scratch/sv-c012/browser-visual/reference-hashes.json).
- В этой корректировке изменены только `src/client/sv/ui/hud.c`, `hud.h`, `tests/sv/diagnostics/launcher.c`, `tests/sv/status-fixture.h`, `tests/sv/status.c`; backend/protocol/session/C011 и исходные53 SVG не менялись относительно предыдущего rework snapshot.
- Existing code graph обновлён `.graphify-venv/bin/graphify update .`: exit0,29235 nodes/72074 edges/1456 communities, [graph-final.log](../.scratch/sv-c012/browser-visual/graph-final.log). Известные partial legacy parser/no-symbol warnings и sequential fallback не являются compiler errors; semantic doc/image rebuild не заявляется. `git diff --check` чист.

Первый windowed before attempt1932×1208 завершился exit1 с пустым SDL error; причина не установлена. Запись сохранена в `sv-character-before-windowed-failed.log` и manifest. Последующие fullscreen X11 before/final runs успешны; это не доказательство первопричины раннего отказа.

### Согласованные различия и остающиеся границы

- Пользователь разрешил CascadiaMono для интерфейса; serif HTML не копируется. Map font остаётся независимым выбором legacy/options и этой работой не менялся.
- У недоступных name/identity/trait/mode/skills/other full C030 данных остаются честные placeholders; HTML MaiaMage/lore/demo skill values не подставлены. C012 supplemental status details отличаются от HTML lore и сохранены доступными через scroll.
- Raw protocol monster health передаётся десятью ступенями: native7/10, HTML demo72%. Historical XP maximum сохраняет wire semantics (в этом matched fixture равен current18432000), а HTML demo MaxExp показывает next threshold19200000. Baseline state/colour roles сохраняются там, где demo palette не является игровой семантикой. Ping остаётся unavailable без настоящей telemetry sample.
- Main map, messages/inventory/bags, stance/dual controls, остальные shortcuts и full C030 остаются за соответствующими owners; whole-frame comparison явно показывает эти различия, но не заполняет их фиктивными UI/game data. Actual C014 socket/live adoption/close/relog и late recipients этим bounded consumer не подтверждены.
- Проверен применимый C012 визуальный результат и ограниченный production input/core путь. Полное visual/provider/version matrix, pixel-perfect identity и временная анимационная приёмка не заявляются. Windows/MinGW/Wine отложены по текущему правилу платформ. Пользовательская приёмка и canonical27/81 runtime registration/statuses остаются pending.


## Повторное открытие Character и аудит вкладок — 2026-11-12

После приведённого выше review пользователь принял общее окно, но не принял Character. Повторный аудит выявил [9расхождений оформления и5расхождений данных](../.scratch/sv-c012/browser-visual/character-discrepancies-2026-11-12.md); прежний общий V3/Character appearancePASS слишком широк и отозван. Исторические прогоны/команды/SHA выше сохраняются как фактически выполненные, без приписывания им проверки всех вкладок.

Новая независимая проверка использовала реальные клики: [HTML CLI Playwright report,9 новых PNG 1932×1208/DPR1](../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/html/report.md) и [SV X11/XTest report,17новых PNG 1932×1208](../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/sv/report.md). Existing native renderer capture,450 кадров,exit0; production/launcher/tests/профиль не менялись. CUA в этом запуске недоступен; localhost черезCLI работает, запретаlocalhost нет. Первоначальный Pillow root capture failed записан вnative отчёте, вместо него успешно использован существующий renderer readback.

- **SKILLS/RESISTS FAIL:** SV клики не меняют active PROFILE/content. HTML SKILLS имеет DETAILS с demographics; RESISTS — отдельные grids/legend/equipmenticons и высоту387.359px@y410.3125, вместо794.609px@y206.6875.
- **DUMP FAIL отклика:** SV кнопка инертна. HTML только имитирует success chat,0 downloads; её mock feedback не является семантикой реального файла.
- **Close/open и scroll:** SV X/Escape close,Shift+C/shortcut open,PageDown/PageUp/wheel подтверждены. Сохранение selected SKILLS/RESISTS недостижимо до реализации выбора; HTML reopen сохраняетRESISTS. ТриHTML страницы на этомviewport помещаются без scroll; native Profile длинный из-за supplemental technical lists.
- **Profile остаётся open:** Background/location/пустая нижняя область подменены StatusDetails/Conditions; фон/шапка и отдельные роли текста отличаются. Недоступные данные и C030 не обосновывают иной layout или Character PASS.

[Обновлённый план006-C1–C9/007-C1–C2, точные пары и открытые вопросы](../.scratch/sv-c012/browser-visual/tabs-audit-2026-11-12/correction-plan.md). 006/007 снова ready-for-agent в части Character. Core001–005, Linux build/core и canonical27/81 неизменны; полный C030/реальныйэкспорт не назначены и не приняты этим аудитом. Пользовательская приёмка Character остаётся открытой.

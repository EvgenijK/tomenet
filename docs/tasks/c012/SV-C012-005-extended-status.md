# SV-C012-005 — Расширенные статусы и tracked monster

Статус: **accepted — Linux status/core**, 2026-11-13, по решению пользователя.

[Решение о приёмке](../../sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются отложенными.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

Исторический статус до финальной проверки: **Linux core checked; weapon precedence correction and status mapping open**, 2026-11-12; пользовательская приёмка pending.

Labels: bug

Зависимости реализации: 001; native projection — совместно с 006.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Предыдущий список «пять отсутствующих видимых статусов и все rawfields» был слишком широким. Показ реальных HTML снимков пользователю выполнен **до** этой корректировки задач. Источник HTML не менялся; эта волна меняет только план/документы, не production. Запрещены новый StatusDetails, новые блоки в Character и самовольно назначенные похожие иконки. Frozen5 visual/input PASS остаётся ограниченным проверенным wide состоянием; найденные ниже semantic gaps не закрыты им.

[Baseline/scope аудит](../../../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и реальные снимки](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md), [адресный план](../../../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md).

**Адресное исправление существующего места:** ENC-PRECEDENCE. Baseline `c-xtra1.c:1733–1735` для общего weapon-слота: easy_wield > heavy_wield > awkward_wield. SV `hud.c:776` из-за last-write порядка даёт heavy > awkward > easy. Исправить приоритет существующего слота/tooltip; shooter heavy>awkward уже совпадает.15wireflags отображаются baseline и HTML в12слотах с конфликтующими группами;15-строчный технический список не требуется. HTML controls сами исключают одновременные флаги группы; существующий tooltip показывает выбранный слот.

**Пользовательские результаты без подтверждённого места:** self AFK в baseline — server AFK на координатной строке, opaque extra — ограниченная текстовая строка. Их existing slots в HTML не обнаружены. AFK в Players Online — статус другого игрока, не доказательство selfHUD; нельзя использовать OS focus/Rest или новую Character строку. Эти UX mapping gaps открыты.

**Реакции и сохранение, а не новые labels:** martyr baseline сохраняет state и печатает сообщение; idle управляет music/weather auto-mute/unmute с сохранением прежней настройки и уведомлением. Нельзя требовать для обоих новую HUD-метку или считать queue/model доказательством работы аудио. C012 сохраняет versioned state/occurrence delivery; фактические поздние message/audio recipients и их owners описываются отдельно без самовольного принятия C055 либо переноса всей интеграции в C012.

Indicators: unknown bits должны сохраняться без выдуманных meanings, baseline не требует hex-строку. Known Probability Travel ещё требует существующего места/mapping; grid activity-иконки не назначать по внешнему сходству. Полные C030 Resists matrices не равны C012 status-indicators.

## Owned outcomes и baseline
- `capability.status.read-monster-health` — Read tracked monster health and colour separately from player HP. Baseline: `src/client/nclient.c:Receive_monster_health`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-afk` — Read server AFK state, independent of local window focus. Baseline: `src/client/nclient.c:Receive_AFK`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-encumbrance` — Read all fifteen encumbrance conditions with version-dependent absent flags cleared. Baseline: `src/client/nclient.c:Receive_encumberment`; версии: >= 4.8.1.0.0.0; > 4.4.2.0.0.0; older. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-extra` — Read exact opaque extra-status bytes with bounded display projection. Baseline: `src/client/nclient.c:Receive_extra_status`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-martyr` — Read martyr state and applicable status presentation. Baseline: `src/client/nclient.c:Receive_martyr`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-idle` — Read idle state and applicable status presentation. Baseline: `src/client/nclient.c:Receive_idle`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-indicators` — Read indicator bit meanings without losing unknown bits. Baseline: `src/client/nclient.c:Receive_indicators`; версии: always. Три obligations — result/lifecycle/allocation-checks.

## Работа и критерий готовности

Monster health и его colour не смешивать с player HP; tracked entity data не требуют переноса targeting C018. AFK — серверное состояние независимо от OS focus; idle audio reactions сохранить occurrence-aware для C055 согласно 001: несколько значимых входов до draw не схлопываются в latest idle, delivery/ack/overflow явные; из draw реакции не воспроизводятся; martyr/idle сохраняют baseline presentation. Encumbrance: все 15 флагов, три layouts (>=4.8.1; >4.4.2; older), absent heavy_swim/heavy_tool/rogue_heavyarmor очищаются. Extra сохраняет точные opaque source bytes, display — bounded projection; не терять control/high bytes и не угадывать структуру. Indicators сохраняет весь u32, включая unknown bits, и отображает известные значения без выдуманных meanings.

Реализовать version-aware production decoding → session presentation model → читаемый native view 006 для каждого owned ID. Повторное чтение view не исполняет effects. Сохранить exact packet consumption и порядок с соседним packet; неполный packet не публикует часть состояния.

## Необходимый минимум проверки

В общем focused production-core наборе 007 проверить matching wire layouts и absent defaults, fragmented packet + следующий packet, точные семантические значения и снятие/обновление состояния. Связанный caller 006 подтверждает interleaved updates и читаемость; reset/generation проверяется совместно с 001. Отметить checked subflows и непроверенные combined flows отдельно для каждого obligation.

Проверять не только wire layouts, но и перечисленные в задаче semantic branches на компактных baseline-derived случаях того же core набора: markers/defaults/encoded states и соответствующие граничные значения. Это проверка собственной реализации, не отдельная suite или новая matrix.

## Общие границы

[План](README.md), [source contract](source-contract.md) и [точные canonical записи](coverage.json) обязательны. Реализация SV-local; legacy/common менять только при необходимости текущей задачи. Модель/decoder не читает Term; UI не читает wire packets/legacy globals. Нужны атомарные complete-packet updates, baseline defaults, generation/reset и read-only views с revisions. Framing и test-only реализация поведения не заменяют production путь.

Проверки — только необходимый минимум Linux core production-пути, совместно с 007. Не создавать broad suite, новый simulator/harness или полную visual/provider matrix; расширение согласовать с пользователем. Windows/MinGW/Wine отложены. Canonical obligations/statuses сохраняются; actual adoption/close/relog проверяет C014. Собственный результат задачи нельзя заменить обещанием будущего C014 consumer.


## Реализация — 2026-11-11

Backend реализован внутри SV: `protocol/status-update.[ch]`, typed semantic view `session/status-state.h`, session-owned HUD/occurrences и app publication. Existing HP path и его отдельная revision сохранены; новые статусы не повторяют HP effects. Complete packet публикуется атомарно, absent defaults и bounded source strings сохраняются.

Production renderer/native consumer и focused evidence выполняются связанными задачами 006/007. Этот промежуточный статус не является PASS canonical obligation или пользовательской приёмкой.


## Итоговая проверка оркестратором — 2026-11-11

Linux production build, focused production status runner и bounded native HUD review прошли на финальных sources; собственный результат задачи проверен в описанных границах. Independent reviews backend/UI закрыли найденные дефекты. [Команды, артефакты и disposition всех 81 obligations](../../sv-c012-evidence.md). Actual C014 lifecycle, C013 карта и поздние recipients не получают PASS. Это технический результат, не пользовательская приёмка и не изменение canonical registry.

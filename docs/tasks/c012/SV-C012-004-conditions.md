# SV-C012-004 — Состояния персонажа

Статус: **accepted — Linux status/core**, 2026-11-13, по решению пользователя.

[Решение о приёмке](../../sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются отложенными.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

Исторический статус до финальной проверки: **Linux core checked; study source/contract and UX mapping clarification open**, 2026-11-12; пользовательская приёмка pending.

Labels: enhancement

Зависимости реализации: 001; native projection — совместно с 006.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Предыдущий список «пять отсутствующих видимых статусов и все rawfields» был слишком широким. Показ реальных HTML снимков пользователю выполнен **до** этой корректировки задач. Источник HTML не менялся; эта волна меняет только план/документы, не production. Запрещены новый StatusDetails, новые блоки в Character и самовольно назначенные похожие иконки. Frozen5 visual/input PASS остаётся ограниченным проверенным wide состоянием; найденные ниже semantic gaps не закрыты им.

[Baseline/scope аудит](../../../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и реальные снимки](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md), [адресный план](../../../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md).

**Открытый source/contract conflict study:** `Receive_study` сохраняет byte; baseline `prt_study` (`c-xtra1.c:1291`) показывает `Skill` как признак available learning, ноль очищает. Canonical description этой задачи требует `available study count`. В реальном HTML нет подтверждённого existing HUD/Character слота для self study. Нужно явно разрешить формулировку count/availability и место, не менять canonical в bool молча и не возвращать technical Character list. Сохранение byte и уже проверенные wire/default branches остаются; читаемый результат и окончательная трактовка пока open.

Activity-grid search/rest/paralysis не является study/AFK/audio-idle слотом. Другие condition icons сохраняют свой подтверждённый semantic mapping.

## Owned outcomes и baseline
- `capability.status.read-confusion` — Read authoritative confusion condition. Baseline: `src/client/nclient.c:Receive_confused`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-poison` — Read authoritative poison condition. Baseline: `src/client/nclient.c:Receive_poison`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-study` — Read available study count. Baseline: `src/client/nclient.c:Receive_study`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-blows` — Read blows/wraith probability attribute and versioned label. Baseline: `src/client/nclient.c:Receive_bpr_wraith_prob`; версии: < 4.9.1.0.0.1; >= 4.9.1.0.0.1. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-food` — Read food amount and semantic hunger presentation. Baseline: `src/client/nclient.c:Receive_food`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-fear` — Read authoritative fear condition. Baseline: `src/client/nclient.c:Receive_fear`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-speed` — Read authoritative speed value. Baseline: `src/client/nclient.c:Receive_speed`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-cut` — Read cut severity and corresponding presentation. Baseline: `src/client/nclient.c:Receive_cut`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-blind-hallucination` — Read blind/hallucination flags without collapsing their meanings. Baseline: `src/client/nclient.c:Receive_blind_hallu`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-stun` — Read stun severity and corresponding presentation. Baseline: `src/client/nclient.c:Receive_stun`; версии: always. Три obligations — result/lifecycle/allocation-checks.

## Работа и критерий готовности

Серверные значения authoritative. Blind и hallucination различны; food/cut/stun сохраняют численное значение/severity и baseline semantic presentation, speed не превращать в произвольную UI шкалу. Study — count, blows/wraith — version-dependent attribute/label (граница 4.9.1.0.0.1). Poison и blind/hallucination — encoded states, не bool. Speed boost bit 0x100 извлекается только для неотрицательного значения; сохранить baseline no-tele dependency от depth/sector colour через минимально необходимый semantic input (без принятия полной карты/location outcomes). Для старого blows packet label явно empty. Снятие состояния обязано убирать прежний indicator. Не выводить AFK из focus и не переносить actions, вызывающие состояния.

Реализовать version-aware production decoding → session presentation model → читаемый native view 006 для каждого owned ID. Повторное чтение view не исполняет effects. Сохранить exact packet consumption и порядок с соседним packet; неполный packet не публикует часть состояния.

## Ancillary DEPTH → speed/no-tele

Минимальный production version-aware `PKT_DEPTH` consumer и semantic no-tele state принадлежат этой задаче как зависимость `read-speed`, без новых owned IDs и без принятия полной map/location capability. Сейчас packet отсутствует в `protocol/protocol.c` supports и `protocol/startup-prefix.c`; direct test-only no-tele injection не удовлетворяет задаче. Baseline: `src/client/nclient.c:3624` — четыре layouts (>4.6.1.2.0.0, >4.5.9.0.0.0, >4.4.1.6.0.0, older), включая old sector colour default green; `src/client/c-xtra1.c:1085` — изменение depth/no-tele переотображает speed. Decoder полностью потребляет согласованный wire frame, даже если C012 использует только semantic subset; не реализовать остальные поля отдельной test моделью.

В том же focused core наборе проверить DEPTH→SPEED и SPEED→DEPTH: оба порядка дают согласованную speed projection, depth update не требует нового speed packet. Полный C013 ingress и C014 socket lifecycle остаются своими owners.

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

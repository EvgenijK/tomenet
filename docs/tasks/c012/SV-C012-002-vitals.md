# SV-C012-002 — HP, мана, выносливость и sanity

Статус: **accepted — Linux status/core**, 2026-11-13, по решению пользователя.

[Решение о приёмке](../../sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются отложенными.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

Исторический статус до финальной проверки: **implemented, Linux core checked**, 2026-11-11; пользовательская приёмка pending.

Labels: enhancement

Зависимости реализации: 001; native projection — совместно с 006.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Предыдущий список «пять отсутствующих видимых статусов и все rawfields» был слишком широким. Показ реальных HTML снимков пользователю выполнен **до** этой корректировки задач. Источник HTML не менялся; эта волна меняет только план/документы, не production. Запрещены новый StatusDetails, новые блоки в Character и самовольно назначенные похожие иконки. Frozen5 visual/input PASS остаётся ограниченным проверенным wide состоянием; найденные ниже semantic gaps не закрыты им.

[Baseline/scope аудит](../../../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и реальные снимки](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md), [адресный план](../../../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md).

Текущий обязательный результат sanity следует проверять в существующем Sanity-слоте/настройке display mode, а не дополнять Character числовой таблицей. В HTML реальные `exact`/bar/percent/word режимы дают current/max; в SV numeric возможен в имеющемся configured huge bar. Проверить доступность фактического SV режима и version-unavailable, не считать отсутствие отдельного числа в ordinary word-mode само по себе дефектом. `damage` в baseline управляет warning reaction, не является отдельной числовой строкой. Сохранение поля/metadata и выполнение позднего alert recipient — разные claims; C050 не получает PASS из модели C012. Wire layouts/defaults и выполненный core corpus не пересматриваются как непроверенные.

Следующий scoped runtime/check — предложение для согласованной реализации/верификации, не одобрение расширить набор тестов или создать suite.

## Owned outcomes и baseline
- `capability.status.read-live-hp` — During a live session read authoritative current/max HP, boost and bar interpretation after each complete versioned packet, including absent-field defaults, interleaved gameplay, resize and reconnect. Stage A synthetic HP does not accept this live caller. Baseline: `src/client/nclient.c:Receive_hp`; версии: > 4.7.0.2.0.1; older. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-mana` — Read current/max mana and applicable bar/boost conventions. Baseline: `src/client/nclient.c:Receive_mp`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-stamina` — Read current/max stamina. Baseline: `src/client/nclient.c:Receive_stamina`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-sanity` — Read sanity attribute/label/damage/current/max according to version; SHOW_SANITY must link with registered receiver. Baseline: `src/client/nclient.c:Receive_sanity`; версии: >= 4.8.1.3.0.0; > 4.6.1.2.0.0 and < 4.8.1.3; <= 4.6.1.2. Три obligations — result/lifecycle/allocation-checks.

## Работа и критерий готовности

Сохранить обе HP wire branches (>4.7.0.2.0.1 и older), drain default, max>10000 bar и cur>5000 boost semantics, включая отрицательные HP после снятия boost encoding. Mana/stamina имеют собственные bar/boost conventions — не переносить HP threshold без сверки Receive_mp/Receive_stamina. Sanity: три wire layouts, attribute/label/damage/current/max и соответствующая SHOW_SANITY receiver registration/link. Missing fields не наследуются от старого пакета/поколения. Полная интеграция новых alert/audio recipients принадлежит C050/C055. Существующие production HP reactions сохраняются; не отключать их. HP occurrence отличается от coarse HUD revision: mana/AFK updates не повторяют low-HP warning, а два одинаковых полных HP packet сохраняют baseline per-packet semantics. Для будущих recipients сохранять occurrence-aware metadata согласно 001.

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

# SV-C012-003 — Характеристики, боевые значения и прогресс

Статус: **accepted — Linux status/core**, 2026-11-13, по решению пользователя.

[Решение о приёмке](../../sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются отложенными.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

Исторический статус до финальной проверки: **Linux core checked; existing stat/level presentation correction open**, 2026-11-12; пользовательская приёмка pending.

Labels: bug

Зависимости реализации: 001; native projection — совместно с 006.

## Актуальный остаток после проверки макета и baseline — 2026-11-12

Предыдущий список «пять отсутствующих видимых статусов и все rawfields» был слишком широким. Показ реальных HTML снимков пользователю выполнен **до** этой корректировки задач. Источник HTML не менялся; эта волна меняет только план/документы, не production. Запрещены новый StatusDetails, новые блоки в Character и самовольно назначенные похожие иконки. Frozen5 visual/input PASS остаётся ограниченным проверенным wide состоянием; найденные ниже semantic gaps не закрыты им.

[Baseline/scope аудит](../../../.scratch/sv-c012/c012-remainder-2026-11-12/scope-audit.md), [HTML mapping и реальные снимки](../../../.scratch/sv-c012/c012-remainder-2026-11-12/html-locations/report.md), [адресный план](../../../.scratch/sv-c012/c012-remainder-2026-11-12/task-adjustments-draft.md).

**Готовые адресные пункты для последующей реализации в существующих слотах:**

- STAT-COLOR: baseline `c-xtra1.c:145–157` сначала drained→YELLOW, затем boosted→L_BLUE, затем normal base<118→L_GREEN/иначеL_UMBER. SV `ui/hud.c:81` использует другой приоритет и всегда green normal. Исправить правило, не hardcode пять демонстрационных ochre строк. Предыдущее freeze5 объяснение о полностью сохранённых semantic colours неполно.
- В существующем Level/XP проверить baseline reduced/max-player-level suffix и fractional bar result. Previous/next/fractional участвуют в полосе; их отсутствие отдельными подписями не требует нового списка.
- В существующих stat строках различить формат одного `18 / xxx` и baseline условное current/max при drain/boost. Base/tmp участвуют в цвете/формате; index — внутренний индекс таблиц, не подтверждённое печатное поле.

**Открытая граница контракта:** canonical literal `read current/max/index/base/temp` и прочие rawread descriptions сохраняются. Нельзя выдать PASS только по decoder, но и нельзя без baseline потребовать пять новых строк. Disposition должен явно записать decoded/derived/readable и source/wording conflict. Это уточнение не меняет canonical требования автоматически.

Carried Gold уже имеет существующее место HTML; bank там отсутствует. Baseline bank показан отдельно в Merchant Guild store при nonzero (`c-store.c:1085`), а не на Character. Merchant/store UI не назначается этой задачей самовольно; отдельное чтение bank остаётся открытым requirement/mapping. Misc combat values не смешивать с melee/ranged; отдельный baseline consumer/HTML slot ещё не установлен.

## Owned outcomes и baseline
- `capability.status.read-stats` — Read each stat current/max/index/base/temp value with version defaults. Baseline: `src/client/nclient.c:Receive_stat`; версии: >= 4.7.4.6.0.0; older. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-armour` — Read base armour class and armour bonus. Baseline: `src/client/nclient.c:Receive_ac`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-demographics` — Read height, weight, age, social class and source identity text. Baseline: `src/client/nclient.c:Receive_various`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-combat-bonuses` — Read melee/ranged/misc hit and damage values without conflating them. Baseline: `src/client/nclient.c:Receive_plusses`; версии: always. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-experience` — Read level, max levels, experience and next/previous thresholds; preserve old-version defaults. Baseline: `src/client/nclient.c:Receive_experience`; версии: > 4.5.6.0.0.1; older. Три obligations — result/lifecycle/allocation-checks.
- `capability.status.read-currency` — Read carried gold and bank balance separately. Baseline: `src/client/nclient.c:Receive_gold`; версии: always. Три obligations — result/lifecycle/allocation-checks.

## Работа и критерий готовности

Stats: current/max/index/base/temp, legacy boost marker 0x10 и version defaults; проверка stat index до доступа к массиву. Armour имеет один wire layout, но три semantic branches: >=4.7.3.1 unknown >15000 снимается вычитанием 20000 перед boost >5000/10000; >=4.7.3.0 используется bit 0x1000; older — без этих markers. Не смешивать base AC и bonus, melee/ranged/misc hit/damage, carried gold и bank balance. Demographics сохраняет height/weight/age/social class и source identity text; это не подмена accepted identity C011. Experience сохраняет level/max levels/experience/previous-next thresholds и old-version defaults. Body/form source bytes сохраняются; значимые form-macro transitions сохранить occurrence-aware для будущего C052/C053 owner согласно 001, без исполнения из draw и без coalescing нескольких переходов в один latest body. Для combat bonuses сверить фактическое имя handler `Receive_plusses` в исходнике: canonical source anchor в snapshot сохраняется дословно. Читаемость полных status данных не означает перенос всей information character sheet C030.

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

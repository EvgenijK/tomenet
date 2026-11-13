# C012 — итог технической части и disposition 27/81

Статус: **accepted — Linux status/core**, 2026-11-13. [Приёмка подтверждена пользователем](../../sv-stage-c012-acceptance.md); интерфейсные и поздние combined приёмки остаются отложенными. Полный runtime PASS canonical outcomes/obligations и пользовательская приёмка интерфейса не заявлены.

## Закрытый невизуальный остаток

1. Linux `make -C src -f makefile.sv -j4 shell sv-status-check` и existing production status runner завершились exit0 на окончательном existing-slots freeze3, source/binary stable. [Команды, SHA и логи](../../../.scratch/sv-c012/final-core-2026-11-13/final-checks.json). HUD SHA `80ac4b283a7510c2330e3bdf23bbd63c0407ce237ecd155ef2dcde11efb3d423`, shell SHA `58554f922710f3f0629089e3cab8f73817d4ddbbb4aaf524f3c7ae4830a76c6b`.
2. [Semantic contract](../../../.scratch/sv-c012/semantic-closure-2026-11-13/report.md) разрешён без canonical изменений: Study хранит exact unsigned source byte, availability определяется отдельно как nonzero; byte не выдуманное количество skill points. Stat table index сохраняется отдельно от проверяемого slot ID0..5; новая строка raw index не требуется. Пользовательский результат остаётся DA-05 и DA-02/04.
3. Production source review и существующий runner подтверждают version-aware decoding/defaults, complete-packet atomic application, copied views, generation/reset и отсутствие stale updates; exact opaque bytes/unknown u32 и различные bank/carried/combat/stat fields не теряются. Ни тесты, ни production код этой финальной волной не менялись.
4. Все81 IDs ниже сопоставлены с [неизменённым planning snapshot](coverage.json); проверенные subflows отделены от отложенных. Технические задачи001–007 не ждут поздней визуальной приёмки. Canonical registration/statuses остаются отдельным workflow.

## Данные и события: точные границы

- `protocol/status-update.c` публикует только complete local Cursor result, zero-initialized update очищает отсутствующие version fields; `session/session.c:83–151` проверяет update до изменения модели. `app.c:548–638` применяет и публикует один complete input до следующего.
- **HP:** отдельная HP revision/occurrence; два одинаковых полных HP packet дают две baseline реакции, non-HP update/rebuild их не повторяет. Existing HP alert delivery реально проверен.
- **Body/form и idle:** bounded FIFO64, generation/sequence, consume одного события, явный overflow; Player→Wolf→Player и idle1→0 не схлопываются. Исполнение macro/audio recipient не проверено.
- **Sanity damage и martyr:** authoritative latest payload сохраняется; отдельной очереди этих occurrences сейчас нет. Snapshot и coarse revision **не гарантируют** сохранение всех damage/message событий между inputs. C050/C016 должны подключить настоящий per-input consumer до следующего decode либо своё bounded retention, проверить повторы/порядок/reset и не replay сообщения из draw. Эта поздняя интеграция не имеет PASS из текущего хранения поля; старые события не обещаны к replay при позднем подключении.
- Mana warnings и новые outputs также не исполнены C012; существующий HP путь сохранён. Только body/idle имеют `sv_app_take_status_effect`, не все27 полей.

Новых data/event дефектов в собственном текущем read-status контракте не найдено. Общий event bus, полные recipients и поздний socket owner не добавляются ради закрытия C012.

## Как читать матрицу

Каждая строка — один точный obligation ID. **CORE CHECKED R/L/A — частичный проверенный scope, не полный PASS обязательства.**

- **R:** independent `latest_all_fields` через app→protocol→model→copied HUD; точные source/default значения. Дополнительно для stats/encumbrance/Sanity/XP проверены production derived preparation cases `stat_colour_boundaries`/`status_slot_formats`. Semantic Study/stat-index ограничены контрактом выше.
- **L:** общий production-path `split_and_adjacent`, `effects_reset_and_errors`, `hp_occurrences_and_projection`: interleaved inputs, read-only rebuild, reset/new generation, stale rejection. Только явно перечисленные HP/body/idle occurrence flows проверены; actual socket adoption/close/relog/focus остаются C014.
- **A:** каждый receiver split+GOLD sentinel, matching version branches/defaults в `version_defaults`, contextual DEPTH/SPEED branches в `depth_speed_and_semantics`; текущая Linux build с SHOW_SANITY. Прочие compile/provider/normal-big-map combined матрицы не заявлены. Windows/MinGW/Wine отложены по правилу платформ.

Общий backend corpus не изменился после [исторического scoped C011 preparation](../../sv-c012-evidence.md#проверенные-core-cases): actual retained app once-only applied INDICATORS и FIFO deferred exact frames были проверены там, заново не запускались. Это не полный C014 handoff PASS. Ссылки DA-NN означают строки [реестра отсрочек](deferred-acceptance.md); все они остаются открыты.

| Obligation ID | Core evidence | Не принятая поздняя часть |
|---|---|---|
| `obligation.status.read-stats.result` | CORE CHECKED R | DA-01, DA-02, DA-04 |
| `obligation.status.read-stats.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-02, DA-04 |
| `obligation.status.read-stats.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-02, DA-04 |
| `obligation.status.read-armour.result` | CORE CHECKED R | DA-01, DA-02 |
| `obligation.status.read-armour.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-02 |
| `obligation.status.read-armour.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-02 |
| `obligation.status.read-demographics.result` | CORE CHECKED R | DA-01, DA-02 |
| `obligation.status.read-demographics.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-02 |
| `obligation.status.read-demographics.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-02 |
| `obligation.status.read-combat-bonuses.result` | CORE CHECKED R | DA-01, DA-02 |
| `obligation.status.read-combat-bonuses.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-02 |
| `obligation.status.read-combat-bonuses.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-02 |
| `obligation.status.read-experience.result` | CORE CHECKED R | DA-01, DA-02, DA-04 |
| `obligation.status.read-experience.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-02, DA-04 |
| `obligation.status.read-experience.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-02, DA-04 |
| `obligation.status.read-currency.result` | CORE CHECKED R | DA-01, DA-07 |
| `obligation.status.read-currency.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-07 |
| `obligation.status.read-currency.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-07 |
| `obligation.status.read-mana.result` | CORE CHECKED R | DA-01, DA-04, DA-08 |
| `obligation.status.read-mana.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-04, DA-08 |
| `obligation.status.read-mana.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-04, DA-08 |
| `obligation.status.read-confusion.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-confusion.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-confusion.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-poison.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-poison.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-poison.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-study.result` | CORE CHECKED R | DA-05 |
| `obligation.status.read-study.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-05 |
| `obligation.status.read-study.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-05 |
| `obligation.status.read-blows.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-blows.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-blows.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-food.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-food.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-food.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-fear.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-fear.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-fear.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-speed.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-speed.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-speed.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-cut.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-cut.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-cut.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-blind-hallucination.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-blind-hallucination.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-blind-hallucination.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-stun.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-stun.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-stun.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-monster-health.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-monster-health.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-monster-health.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-sanity.result` | CORE CHECKED R | DA-01, DA-04, DA-08 |
| `obligation.status.read-sanity.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-04, DA-08 |
| `obligation.status.read-sanity.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-04, DA-08 |
| `obligation.status.read-afk.result` | CORE CHECKED R | DA-01, DA-06 |
| `obligation.status.read-afk.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-06 |
| `obligation.status.read-afk.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-06 |
| `obligation.status.read-encumbrance.result` | CORE CHECKED R | DA-01 |
| `obligation.status.read-encumbrance.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01 |
| `obligation.status.read-encumbrance.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01 |
| `obligation.status.read-stamina.result` | CORE CHECKED R | DA-01, DA-04 |
| `obligation.status.read-stamina.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-04 |
| `obligation.status.read-stamina.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-04 |
| `obligation.status.read-extra.result` | CORE CHECKED R | DA-01, DA-06 |
| `obligation.status.read-extra.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-06 |
| `obligation.status.read-extra.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-06 |
| `obligation.status.read-martyr.result` | CORE CHECKED R | DA-09 |
| `obligation.status.read-martyr.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-09 |
| `obligation.status.read-martyr.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-09 |
| `obligation.status.read-idle.result` | CORE CHECKED R | DA-09, DA-10 |
| `obligation.status.read-idle.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-09, DA-10 |
| `obligation.status.read-idle.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-09, DA-10 |
| `obligation.status.read-indicators.result` | CORE CHECKED R | DA-01, DA-06 |
| `obligation.status.read-indicators.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-06 |
| `obligation.status.read-indicators.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-06 |
| `obligation.status.read-live-hp.result` | CORE CHECKED R | DA-01, DA-04, DA-08 |
| `obligation.status.read-live-hp.lifecycle` | CORE CHECKED L | C014 live lifecycle; DA-01, DA-04, DA-08 |
| `obligation.status.read-live-hp.allocation-checks` | CORE CHECKED A | C014 live lifecycle; C013/C014 combined; DA-01, DA-04, DA-08 |

Итого:27 capability IDs /81 уникальный obligation ID,81 scoped core dispositions;0 заявлений full-outcome PASS. Отсрочки не меняют canonical acceptanceStage, ownership, conditions и statuses.

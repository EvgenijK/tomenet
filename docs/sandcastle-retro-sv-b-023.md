# Ретро Sandcastle: SV-B-023 и сопровождение в чате

Дата: 2026-10-09. Run: `7b133980`. Задача: [SV-B-023](tasks/stage-b/SV-B-023-birth-choices.md).

Ретро составлено по этому чату, журналу событий, state, tickets, review и final-audit artifacts, истории Git и текущему controller. Два независимых агента разобрали controller/recovery и продуктовые дефекты/качество проверки; выводы сверены перед объединением. Здесь предлагаются изменения процесса; они не считаются реализованными только потому, что включены в документ.

## Итог и главный вывод

Run завершился штатной интеграцией с verdict `ready`, scope `implementation`: 89 tickets, 0 открытых дефектов, 42 current criteria passed. `P1-AC30` остаётся deferred; полная native runtime-приёмка не завершена. Проверенный production HEAD: `2053455cb2c93b3f9a72b3294243be2c8a02128f`. Feature/report интегрированы до `0ff7dd720`; controller repairs сохранены отдельным maintenance-коммитом `3f2d5977a8ad9dc5b797efa2c90d9d5ddd8bde84`. После согласования controller suite прошёл 155/155. Каноническое evidence реализации — [отчёт run](tasks/sandcastle/7b133980-report.md).

Работа затянулась из-за сочетания двух причин:

1. Исправления часто закрывали локальный симптом birth flow, а не полный lifecycle и соседние переходы. Production seam был правильным, но fixtures и ожидаемые результаты иногда воспроизводили ошибочное предположение реализации.
2. Controller, run-ветка, immutable contract, рабочие копии и Docker runtime не имели единой проверяемой версии. Исправление в одном месте не означало, что следующий запуск использует его. Восстановление превращалось в ручное согласование этих состояний.

Review и защитные gates были полезны: они ловили пропущенные ветви, сохраняли требования и останавливали небезопасную интеграцию. Проблема — позднее обнаружение и дорогие повторные циклы, а не сама необходимость review.

## Подтверждённые показатели и границы интерпретации

| Показатель | Значение |
|---|---:|
| Период journal, МСК | 7 октября 13:44:10 — 9 октября 23:00:36 |
| Elapsed между первым и последним событием | 57 ч 16 мин 26 с; это не время активной работы |
| Completed tickets / ticket batches | 89 / 25 |
| Собранные waves | 65; последний sequence 67 — идентификатор, не количество |
| Total build / test attempts | 61 / 33 |
| Сохранённые review rounds | 41 |
| Finding appearances / distinct defectKey в review artifacts | 74 / 34 |
| Keys, появлявшиеся более одного раза | 15 |
| Отдельные focused-test-coverage tickets и waves | 28 / 28 |
| Дополнительные waves проверки/покрытия | Ticket 12 и Ticket 83 |
| Recovery decisions | 17: 8 retry, 7 repair, 2 stop |
| Категории recovery | 4 agent_failure, 13 other |
| События workflow paused | 49; это не 49 независимых аварий |
| Final acceptance criteria | 43: 42 current passed, 1 deferred |
| Финальная позиция journal | seq 710, phase done |

34 defect keys не равны 34 подтверждённым продуктовым багам: здесь есть metadata, policy/documentation findings и новые facets одной lifecycle-проблемы. 74 appearances также не равны числу ремонтов. 41 review round нельзя называть «41 циклом исправления»: часть повторов связана с response validation, disposition и повторной оценкой.

28 focused waves — waves с отдельным ticket проверки/покрытия; это не утверждение, что все они меняли только документацию. Некоторые добавляли tests или fixes. Сокращать отдельную assembly предлагается только для чистого evidence без code/test changes.

Сохранены два final-audit файла. Один блокировал aggregate DNA lifecycle, поздний финальный audit дал ready. В state также сохранён архив более раннего audit о legacy hidden-class decoding. Текущая acceptance history содержит один audit после amendment; текущий ledger не воспроизводит всю историю run. Для ретро нужны и архивы, и events.

Все 33 сохранённые controller artifacts `check-sv-core-*.json` имеют `passed: true`. Это показывает, что зелёный gate не предотвращал последующие review findings; это не доказательство отсутствия ошибок между gates и не свидетельство native acceptance.

### Квота и стоимость

В `allowanceHistory` записано 15 grants, сумма разрешений — 125 процентных пунктов. Финальное поле budget — 84/91; aggregated reported input/output tokens — 190 016 499. Ни одно из этих значений нельзя выдавать за полную уникальную или оплачиваемую стоимость run.

История cap содержит переход 85 → 25. Generic `SANDCASTLE_CONTINUE=1` в [workflow](../.sandcastle/workflow.mjs) до сих пор увеличивает build/test/review limits, очищает `recoveryAttempts`, сбрасывает `consumedPercent` и `tokens` и устанавливает cap 25. Это возможное объяснение перехода; точная команда reset не записана в journal, поэтому причинная связь остаётся предположением. Budget после reset не является lifetime accounting. Account window usage, доля выделенной run квоты и reported token usage должны учитываться раздельно.

## Проблемы продукта и проверки

| Проблема | Что позволило ей пройти проверки | Принятое решение / evidence |
|---|---|---|
| Stats после Backspace невозможно пройти повторно | Проверяли вернувшегося owner, затем quit; consumed state не проверяли при повторном продвижении | Ticket 15, review 1: восстановление stats при re-entry |
| PvP Maia после mode зацикливался или возвращался в неверный граф | Временный post-mode trait phase сохранялся при глубоком Backspace; тест завершался после первого возврата | Tickets 32/43/54 и последующие facets; восстановление pre-mode state и проверка нескольких обратных переходов |
| Automatic DNA и ordered stats | Исправления отдельно теряли pending continuation и принимали новый DNA command там, где baseline его игнорирует | Tickets 19/34/38/75; разделение pending continuation и новой команды, ordered-stats no-op semantics |
| DNA fragments принимались после общей invalidation | Независимые availability flags и частичные success fixtures вместо complete valid DNA | Ticket 71, final audit и reviews 30/32: aggregate validity lifecycle; Ticket 73 обновил success fixtures |
| Hidden classes, race/class highlight и буквы выбора | Identity смешивали с row position; fixtures не имели дополнительных hidden/derived вариантов | Tickets 16/21/27/52/53/58/69/79; authoritative identities, compatibility и stable selection |
| Shift/Caps, punctuation/layout/AltGr | Fixtures подставляли готовый symbol и тем самым обходили реальный native translation | Tickets 44/48 и соседние repairs; production SDL gestures/layout semantics |
| Scroll/viewport выбирал не видимую строку | Hit-testing и rendering использовали разные offset/identity | Tickets 29/60; проверка выбора после scroll/resize |
| first_run, dedicated entry и capacity | first_run выводили из roster; dedicated flow доступен через test options; capacity fixtures всегда включали оба feature flags | Tickets 22/23/85: отдельный authoritative first_run, production exclusive entry, четыре комбинации IDDC/PvP flags и capacity boundaries |
| Unknown creation setup: deadlock или удалённый body | `creation_flags > 1` использовали как признак отсутствия нескольких owners; тесты утверждали этот skip | Пользователь принял explicit fail-fast; Ticket 83 и amendment P1-AC31 проверили unchanged selection, exact message, отсутствие name/birth/draft/send |
| Unavailable-body и потерянный Fruit Bat outcome | Repair 86 проверял skip/Backspace и завершался до mode; потеря полного outcome выявлена review 40 | Ticket 88 восстановил legacy-mode outcome; Ticket 89 проверил покрытие; review 41 и final audit дали ready |

Канонические примеры: [birth state machine](../src/client/sv/session/birth.c), [login interaction](../src/client/sv/input/login-interaction.c), [endpoint fixtures](../tests/sv/name-editor-submit.c), [runner](../tests/sv_name_editor_submit_checks.py), [Wayfinder history](../.scratch/sandcastle-7b133980/map.md).

### Почему production tests давали ложную уверенность

Production seam действительно проходил controlled TCP peer → `sv_endpoint_run` → native SDL adapter → production interaction → input router. Но наличие правильного seam не доказывает правильность oracle и полноту сценария.

В этой работе повторялись четыре ошибки:

- Сценарий проверял owner/текст и прерывался до полного результата. Поздний mode, повторный stats или второй Backspace оставались непроверенными.
- Название сценария обещало skip/DNA, но setup не создавал нужное условие. После fail-fast некоторые availability fixtures всё ещё получали 0x10 и останавливались у guard. Alternate trait fixture выбирал race с доступным trait.
- Expected assertion был основан на том же неверном предположении, что implementation. Повторный запуск подтверждал согласованность двух ошибок.
- Проверяли одну удобную конфигурацию: point-buy вместо ordered stats, оба dedicated flags вместо всех сочетаний, один compile order или простой hidden-class inventory.

Для каждого регрессионного сценария нужны assertions его исходных условий, достигнутой ветви и конечного outcome. Например, DNA-success начинается с доказательства aggregate validity; unavailable-trait заканчивается подтверждением реально пропущенного owner; Backspace проходит `forward → back → edit → forward → READY`, а сложный post-mode flow — несколько обратных переходов.

### Неточность historical body availability

В чате прозвучало «сервер сообщил, что отдельного этапа тела нет». Эту формулировку нужно исправить: отдельного wire-поля body availability нет. Repair 86 вывел `body_step_available` из версии в [contact decoder](../src/client/sv/protocol/contact.c), а затем fixture моделировал получившееся правило.

Review 39 показал недостижимый `body_skipped` и пробел покрытия accepted P1-AC19. Это само по себе не доказывает выбранную границу версии. Текущий [baseline](../src/client/c-birth.c) вызывает `choose_body_modification()` без такого version guard. Исторический commit `3ccc2ddd8cb1dd3db14583c9d7116f7c7ddfe001` («Fixes for 4.4.1.a») переименовывает уже существовавшую modification-функцию; его parent уже имел отдельный body flow. Поэтому этот historical evidence не устанавливает тезис «до 4.4.1a отдельного body нет».

`MODE_FRUIT_BAT_OLD = 0x20` существует, но его backward compatibility также связана с client/DNA encoding до 4.7.1b/4.7.1.2: [defines](../src/common/defines.h), [DNA loading](../src/client/c-files.c), [server compatibility](../src/server/nserver.c). Это не равнозначно server UI capability. Последний repair записывает legacy mode и body outcome; окончательный packet send принадлежит SV-B-024 и этим run не проверен.

Это source ambiguity и основание для отдельного независимого compatibility audit. Ретро не объявляет новый подтверждённый runtime defect без воспроизведения. Final verdict остаётся записанным ready в implementation scope, но historical version rule требует более сильного независимого доказательства, чем синтетический fixture с тем же правилом.

## Проблемы процесса, причины и решения

| Инцидент | Причина | Решение и состояние после run |
|---|---|---|
| Focused tickets многократно возвращались без Labels/Tracker role | Чинили generated artifact/run branch, но launch controller продолжал генерировать старый формат | Генератор и validateTicketCompletion исправлены; проверка metadata распространяется на новые completed tickets. Исправления сохранены в maintenance |
| Повторные stale assessments и некорректные finding responses | Ошибочные criterion IDs, reused identity, одновременно open/resolved; ручные checkpoint repairs разносили current state и history | Validators остановили invalid acceptance; старые assessments архивировали, review повторяли. Однако done state ещё содержит stale pointers |
| Manual repair занял integration branch внешним worktree | У controller и ручного агента не было явной передачи ownership | Worktree detached; `4a2ccc5e4` сохранён как worker, `3b64d8330` — assembly merge. Работа не потеряна |
| Fail-fast spec/code обновились, immutable contract остался прежним | Требование изменили после фиксации контракта, но не провели controller amendment | Independent semantic amendment сохранил прежние 42 criteria/checks и добавил P1-AC31; planner перестал предлагать rollback |
| Verifier не мог подтвердить preservation контракта | Получал digest вместо полного previousContract | Передача полного previous contract proposer/verifier и regression test; amendment независимо одобрен |
| Scheduler переиспользовал wave-60 без Ticket 83 | Manual repair уже занял branch name; existing ref приняли за рабочую основу нового ticket | Collision-aware allocator и bounded reprovisioning; новая wave-62 от принятого plan HEAD. Три recovery решения до исправления были избыточны |
| Worker падал с status 1 и пустой причиной | parseAgentOutput терял JSON error/turn.failed в stdout | Sanitized diagnostics в agent-output; конкретная startup причина теперь сохраняется |
| GPT-6.1 доступен на host, но отказал в Docker | Host CLI 0.161.0, image CLI 0.157.1; dry-run argv/model cache не доказали actual dispatch совместимость | После HTTP 400 обновлён pinned CLI/image до 0.161.0; verified bounded runtime-upgrade resume; actual worker gpt-6.1-sol/medium подтвердил запуск |
| Recovery давал startup advice тому же не запускающемуся worker | Recovery умеет выбирать repair, но worker не может исправить среду до собственного запуска | Остановлены бесполезные повторы, исправлен launcher/runtime; исторические попытки сохранены |
| Финальный integrate остановился на dirty main | Незакоммиченные controller repairs и пользовательские docs пересекались с run diff | Durable stash/ref, archive, verified Git bundle; отдельный maintenance commit; все 17 файлов SHA256 verified, 2 user docs возвращены uncommitted |
| Heartbeat instructions содержали старый cursor | Prompt не был источником actual run cursor; cursor сохранялся вручную в /tmp | Проверки only-after-seq и run-specific cursor file; significant notifications; heartbeat удалён после done. Durable monitoring остаётся предложением |
| Пользователь многократно спрашивал «остановилось?» и «какие блокеры?» | Compact status не содержит конкретного pause reason/доступной команды продолжения; многословные state dumps, тишина и ручные переходы | Причины извлекались из state, но это ручной workaround. Structured blocked status ещё нужен |

Current controller evidence: [tickets](../.sandcastle/tickets.mjs), [parallel scheduler](../.sandcastle/parallel.mjs), [contract amendments](../.sandcastle/contract-amend.mjs), [runtime policy](../.sandcastle/runtime.mjs), [worker upgrade](../.sandcastle/worker-runtime.mjs), [diagnostics](../.sandcastle/agent-output.mjs), [Dockerfile](../.sandcastle/Dockerfile), [workflow](../.sandcastle/workflow.mjs).

### Ошибки ведения работы в чате

- После разрешения на ремонт агент коммитил прямо в run-ветку и оставил её занятой внешним worktree. Это создало предсказуемый ownership stop. Следовало зарегистрировать manual worker и штатную assembly до запуска repair.
- После уточнения `creation_flags > 1` был изменён код/spec, но не весь authoritative contract. Resume начал планировать отмену согласованного поведения. Изменение requirement должно атомарно предшествовать implementation dispatch.
- Полный тестовый прогон остановили ради скорости, честно указав ограничение. Но ранняя передача «готово» не заменяла полный gate; workflow впоследствии должен был всё равно его выполнить. Нужно различать implementation patch ready и gate complete.
- При миграции модели подтвердили cache, CLI flags и unit tests, но не совместимость actual worker. Правильная конфигурация argv не гарантирует принятие модели сервисом через выбранный runtime/account.
- Промежуточные объяснения иногда превращали вывод агента в установленный server fact. Для спорной compatibility причины требуется ссылка на wire/source и независимая проверка, а не только уверенный review.
- Увеличение квоты, разрешение на новый ремонт, approval файловой/Docker границы и изменение требований обсуждались похожим словом «разрешаю». Следующее действие должно называться точно: сколько quota points, какие attempts, какой checkpoint или какой requirement меняется. Сохранённая авторизация обычного ремонта не должна требовать повторного вопроса на каждом шаге; исчерпание явно заданной квоты остаётся реальной причиной остановки.

## Что сработало

- Сохранение checkpoints, branches и ancestry позволило завершить один run после длительных пауз.
- Контракт и audited HEAD guards не дали молча ослабить требования или принять неаудированный production code.
- Production endpoint seam позволял воспроизводить ошибки без test-only implementation поведения.
- Review/final audit нашли существенные пропущенные состояния; final audit остановил aggregate DNA defect после зелёного test gate.
- Fail-fast стал явной принятой divergence, а не скрытым отклонением от baseline.
- Clean-checkout guard сохранил пользовательскую работу; финальная backup/reconciliation восстановила все исходные файлы.
- Controller changes в конце сохранены отдельным maintenance commit; heartbeat завершил свой lifecycle после done.

## Предлагаемые системные решения

Приоритеты ниже — порядок внедрения, а не severity каждого исторического дефекта. Owner обозначает модуль/роль будущей работы, а не назначенного человека. Это новые предложения, кроме явно описанных уже установленных repairs.

| Приоритет / owner | Изменение | Проверяемый результат |
|---|---|---|
| P0 · quota/controller | Убрать reset semantics generic CONTINUE; разделить resume, quota grant, attempt grant. Append-only usage/grant history с идентификатором account window | Attempts grant не изменяет consumption/token history; quota grant не меняет attempt counters; terminal/pause/resume ничего не обнуляют. Подсказка команды зависит от pause category |
| P0 · runtime/controller | Единый execution manifest: controller revision/hash, runner digest, CLI version, image digest, model/effort, HEAD, contract digest | Каждый dispatch/evidence содержит manifest; worker preflight проверяет branch base, ticket readable, image/CLI и актуальный controller; несовместимость выявляется до implementation attempt |
| P0 · compatibility/test owner | Независимый compatibility ledger: server/client version ranges, wire flags, source revision, compile order, accepted divergences | Каждая version branch имеет source-backed oracle; спорная body boundary проверена независимым audit. Server version, client packet encoding и UI owner не подменяют друг друга |
| P0 · birth/test owner | Проверять lifecycle state machine, а не отдельные owners; общий valid-DNA fixture constructor | Required edges покрыты forward/back/edit/forward до READY, включая несколько Backspace. Assertions фиксируют setup prerequisites, draft/identity/validity/send; bounded transitions ловят циклы |
| P1 · controller/Git | Зарегистрированный manual-repair entry и отдельный maintenance change set | Repair получает уникальную worker branch от saved HEAD; integration ref controller-owned; maintenance не меняет audited product scope и не остаётся незакоммиченным в launch checkout |
| P1 · acceptance/controller | Atomic requirement amendment до code dispatch; previousContract обязателен для independent verifier | Все прежние обязательства сохранены или explicit semantic delta принят отдельно; spec/state/artifact/digest обновлены одной операцией; pending evidence не наследуется как passed |
| P1 · validation/controller | End-to-end generator→claim→resolve→assembly→report validation | Actual launch generator создаёт валидный ticket; metadata/map invariant проверяется при публикации каждого результата. Unit tests из другой checkout не считаются доказательством runtime upgrade |
| P1 · recovery/status | Structured failures и resume plan; stale current pointers отделены от immutable history | Startup/auth/model/schema/test/Git/contract/quota различаются; status содержит sanitized reason, next action, quota и last activity; неизменная deterministic причина не вызывает AI retries |
| P1 · tests/performance | Scenario descriptors и build-once/run-many с digest cache; unknown selector/zero cases — error | Один согласованный registry setup/input/assertions; critical pairwise/version matrix видна; per-case duration и executed count доступны; после patch нет лишней повторной компиляции неизменного artifact |
| P2 · verification/controller | Verification-only результат хранить как evidence stage, без обязательного нового ticket/worker/assembly, когда нет code changes | Один exact-HEAD evidence record подтверждает покрытие; 28 focused verification waves не воспроизводятся автоматически в следующей аналогичной задаче. Full gate и independent review сохраняются |
| P2 · monitoring/controller | Durable run monitor cursor и notification acknowledgements отдельно; terminal cleanup | Restart не дублирует notifications по (runId,seq); cursor не зависит от /tmp; done удаляет heartbeat по заранее сохранённой политике |

### Первые четыре работы

1. Зафиксировать независимый audit historical body/Fruit Bat и expected outcomes: это снимает риск неверного compatibility oracle, который зелёные synthetic tests не разрешают.
2. Сделать usage history монотонной и убрать универсальную подсказку CONTINUE: следующий длительный run должен иметь объяснимые расходы и отдельные allowances.
3. Добавить execution preflight/manifest и registered manual repair: предотвращает повторение controller skew, branch collision и CLI/model startup failure.
4. Перевести критические birth scenarios в descriptors с prerequisite assertions и полным lifecycle; затем измерить и сократить повторные verification stages.

Внедрять controller меры отдельными maintenance changes, проверяя их на небольшом smoke run до новой продуктовой задачи. Для regression harness полезны сохранённые incidents: Ticket83 missing on stale branch, generator без Labels, previousContract отсутствует у verifier, startup400 только в Docker, dirty-main integrate, ordered-stats re-entry и unavailable-body до завершения mode.

### Критерии улучшения следующего run

- Ноль повторов одной deterministic инфраструктурной причины без изменения manifest/checkpoint.
- Ноль metadata-only repairs из собственного генератора controller.
- Полная audit trail grants без reset lifetime usage; blocked status содержит причину и разрешённую команду.
- Для каждого high/medium transition repair — соседний lifecycle counterexample и assertions достигнутой ветви.
- Длительность compile, tests и model calls измеряется отдельно; доля verification-only waves сравнивается с этим baseline.
- Каждый отчёт показывает implementation/native/downstream scope отдельно.

Не задаём произвольную цель «не более N review»: число review зависит от scope. Полезная цель — снижение рецидивов и поздних findings при сохранении обязательной независимой проверки.

## Связь с прошлым ретро и незакрытые вопросы

[Ретро SV-B-021](sandcastle-retro-sv-b-021.md) уже описывало root/worktree runner skew, metadata/ledger recurrence, invalid reviewer responses и попадание controller repairs в feature diff. Здесь часть защит стала долговременной maintenance-частью. Однако чтение repository checks script из controller root остаётся в текущем workflow: это подтверждённый риск исполнения checks не из проверяемого HEAD, а не новый воспроизведённый инцидент SV-B-023.

Старые утверждения «validator снова допускает external checks» и другие описания состояния в [журнале проблем](sandcastle-issues.md) и предыдущем ретро датированы ранней конфигурацией. Их нельзя копировать как актуальный факт без сверки с текущим runner inventory/amendment кодом. Эти пользовательские документы в рамках ретро не изменялись.

Остаются отдельные работы: native acceptance P1-AC30 с исходным owner; проверка packet/outcome compatibility в SV-B-024; независимый historical body audit; lifetime accounting и terminal state cleanup. После done в state ещё есть stale pendingFailure/assessment references; их следует хранить как history, не как current failure.

## Источники и восстановление

- [Канонический run report](tasks/sandcastle/7b133980-report.md) и [SV-B-023](tasks/stage-b/SV-B-023-birth-choices.md).
- [Run state](../.sandcastle/runs/7b133980/state.json), [journal seq1–710](../.sandcastle/runs/7b133980/events.jsonl), review/final-audit/recovery artifacts в той же директории. Эти runtime-файлы игнорируются Git; для переноса ретро с полным evidence нужен отдельный экспорт.
- [Wayfinder map](../.scratch/sandcastle-7b133980/map.md) и issues01–89; resolved Answers фиксируют tested HEAD и ограничения.
- [ADR0001](adr/0001-separate-session-interactions-and-ui.md), [ADR0002](adr/0002-main-thread-state-ownership.md), [ADR0003](adr/0003-explicit-module-connections.md), [ADR0005](adr/0005-session-scoped-work.md), [AGENTS](../AGENTS.md).
- Backup финальной интеграции: `/tmp/sv-b-023-preserve-ZJy3iJ`, archive/patch/complete Git bundle/README. Named stash `78f7c6c808119cdebe097b91670775994be9f557` закреплён ref `codex/preserve-sv-b-023-local`; /tmp не долговременное хранилище.
- SHA256 сверка подтвердила все 17 исходных локальных файлов. Пользовательские `docs/sandcastle-issues.md` и `docs/sandcastle-retro-sv-b-021.md` восстановлены побайтово и оставлены uncommitted.

При переносе evidence исключать credentials, auth cache, environment secrets и необработанные request logs. Этот документ не содержит секретов и не создаёт автоматические задачи на внедрение предложений.

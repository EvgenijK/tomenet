# Ретро Sandcastle: SV-B-021

Дата: 2026-10-06  
Успешный run: `f5fbf91a`  
Предыдущий остановленный run: `b5cc0b14`  
Итог: implementation verdict `ready`; 27 current criteria passed, 3 native/downstream criteria pending.

Это ретро консолидирует опыт запуска, восстановления и проверки
[SV-B-021](tasks/stage-b/SV-B-021-account-manage.md). Канонический результат
самой задачи находится в [отчёте run](tasks/sandcastle/f5fbf91a-report.md).
Здесь зафиксированы причины процессных сбоев, принятые решения и состояние
защит в текущей ветке.

## Краткий итог

Sandcastle довёл SV-B-021 до готовой implementation-границы, не выдав
headless evidence за Linux/Windows native acceptance и не присвоив задаче
downstream acceptance SV-B-025. Финальный результат содержит production SV
путь создания аккаунта, controlled-peer проверки, три review-раунда и свежий
final audit без открытых дефектов.

Путь к результату оказался существенно длиннее самой feature-реализации:

- первый run `b5cc0b14` остановился на проверке полноты контракта;
- второй run `f5fbf91a` несколько раз безопасно останавливался на
  инфраструктурных противоречиях;
- для восстановления были разработаны четыре controller fixes;
- review потребовал убрать три recovery fixes из feature diff как не относящиеся
  к SV-B-021;
- поэтому только fix contract convergence остался долговременной частью
  текущей ветки, а три остальных решения пока являются проверенными, но не
  установленными системными улучшениями.

## Результат в цифрах

| Показатель | Значение |
|---|---:|
| Завершённые Wayfinder tickets | 18 |
| Собранные waves | 13 |
| Build attempts | 11 |
| Test attempts | 6 |
| Review rounds | 3 |
| Fresh final audits | 1 |
| Current criteria | 27 passed, 0 failed |
| Deferred criteria | 3 pending |
| Финальный verdict | `ready`, scope `implementation` |

Оставшиеся pending criteria не являются дефектами реализации:

- Linux native input и реальный credential provider;
- Windows native input и Credential Manager;
- включение результата в сквозной flow SV-B-025.

## Хронология и решения

### 1. Контракт не сходился по полноте

**Симптом.** Run `b5cc0b14` повторял contract verification и не приходил к
стабильному полному контракту.

**Причины.** Generic prompts содержали инструкции из другой задачи Guide /
SV-B-011; verifier видел неограниченный набор источников; отображаемая команда
`sv-core` скрывала передаваемый controller через stdin script и интерпретировалась
как неполная; замечания предыдущего раунда перезаписывались, а не накапливались;
одинаковое замечание не имело стабильной identity и не останавливало бесполезный
цикл.

**Решение.** Коммит `fec73b88c` добавил source-bounded prompts, фиксированный
authoritative source set, накопительный feedback ledger со стабильными ID и
остановку после трёх одинаковых open sets.

**Состояние.** Решение сохранено в текущей ветке и покрыто Sandcastle tests.

**Урок.** Completeness review должен быть функцией фиксированного набора
источников и накопленной истории, а не очередным независимым мнением модели.

### 2. Core gate выполнял checks не из проверяемого HEAD

**Симптом.** Интеграционная ветка уже содержала исправленный
`.sandcastle/checks.sh`, но `sv-core` продолжал запускать старый manifest со
ссылками на удалённые после сужения Stage B тесты.

**Причина.** Controller читал script из своей root checkout, а не из
`sandbox.worktreePath`. Поэтому worker не мог исправить реально выполняемый
gate.

**Решение для run.** Коммит `338de1039` заставил controller брать checks script
из integration worktree и добавил regression test с разными root/worktree
scripts.

**Состояние.** Решение позволило продолжить run, но затем было удалено из
feature diff при возврате SV-B-021 к account scope. Текущий
`.sandcastle/workflow.mjs` снова читает root `.sandcastle/checks.sh`.

**Урок.** Код, manifest и runner каждого gate должны происходить из одного
audited HEAD. Controller runtime может управлять выполнением, но не должен
подменять repository-owned script своей checkout-версией.

### 3. Implementation contract содержал обязательные external checks

**Симптом.** Build и `sv-core` прошли, после чего test-gate остановился на
`P1-implementation-review`: check был обязательным и current, но имел kind
`external` и не имел runner.

**Причина.** Validator разрешал такое сочетание, prompt прямо предлагал
«explicit external blocker», а test-gate закономерно запрещал считать его
пройденным. Контракт был валиден по schema, но невыполним по pipeline semantics.

**Решение для run.** Коммит `3c6a4b69b` запретил mandatory/current external
checks для implementation scope, уточнил prompts и добавил bounded contract
amendment с независимой повторной проверкой.

**Состояние.** Решение восстановило run, но было удалено из финального feature
diff. Текущий validator снова допускает эту конструкцию, а prompt снова
упоминает external blocker.

**Урок.** Валидность acceptance contract включает достижимость: каждый current
mandatory criterion implementation scope должен иметь controller-executable
evidence. Code/diff inspection принадлежит review/final audit, а не external
check test-gate.

### 4. Контракт сослался на несуществующий runner

**Симптом.** Пересобранный контракт указал
`python3 -B tests/test_sv_account_create.py`, хотя production-path suites
назывались `tests/sv_account_create_checks.py` и
`tests/sv_account_failure_checks.py`.

**Причина.** Contract validation проверял форму команды, но не существование
статически определимого repository runner. Обычный repair изменил committed
artifact, но не controller-owned contract digest, после чего immutable guard
правильно остановил run.

**Решение для run.** Коммит `6c6e0426a` добавил runner preflight и bounded
command-only repair: только замена команды, без изменения criteria, scope,
mandatory/deferred semantics или check metadata; с новой независимой
verification и атомарным обновлением digest/evidence.

**Состояние.** Решение проверено red-to-green и успешно использовано, но удалено
из финального feature diff. В текущей ветке существование command runner при
принятии контракта не проверяется.

**Урок.** Accepted contract metadata принадлежит controller. Worker может
предложить correction, но не может сделать новый artifact истинным простым
коммитом. Исправление требует повторного controller acceptance.

### 5. Recovery infrastructure попала в feature diff

**Симптом.** Первый review справедливо заблокировал SV-B-021: кроме account
implementation в three-dot diff находились общие Sandcastle amendment,
runner-validation и recovery changes.

**Причина.** Инфраструктурные fixes понадобились во время активного feature
run и были влиты в его integration branch. Технически это восстановило процесс,
но расширило scope проверяемой feature-задачи.

**Решение.** Repair ticket вернул feature diff к account scope; общие controller
changes были удалены перед повторным review.

**Состояние.** Feature scope чистый, но три системные защиты из пунктов 2–4
нужно вернуть отдельной Sandcastle maintenance-задачей.

**Урок.** Controller repair и product feature должны иметь разные change sets.
Исправление orchestrator сначала принимается в base отдельным review, затем
feature run синхронизируется с новым base и повторяет gates.

### 6. Controlled-peer evidence было ошибочно deferred

**Симптом.** Task record объявлял controlled-peer acceptance внешним и pending,
хотя новые suites уже запускали production `sv_endpoint_run` через настоящий
loopback TCP peer и покрывали success, rejection, cancel, error, retry и
fragmentation.

**Причина.** Было смешано понятие controlled deterministic peer с реальным
внешним TomeNET server и native platform acceptance.

**Решение.** P1-10 переведён в current/passed и связан с исполняемыми suites.
Linux native, Windows native и SV-B-025 integration остались deferred.

**Состояние.** Исправление сохранено в task record и финальном отчёте.

**Урок.** Тип evidence определяется production seam и контролируемой границей,
а не тем, что peer является fixture. Controlled peer достаточен для
детерминированной protocol/state implementation acceptance, но не заменяет
native/provider/platform acceptance.

### 7. Wayfinder ledger снова устарел после repair

**Симптом.** Первый repair создал map и восстановил metadata, но последующий
focused-test ticket 14 снова отсутствовал в `Decisions so far`.

**Причина.** Проверка ledger была выполнена как одноразовый repair, хотя
Sandcastle продолжал создавать новые child tickets.

**Решение.** Новые tickets были добавлены в map; review подтвердил итоговую
целостность.

**Состояние.** Run artifacts корректны.

**Урок.** Wayfinder index — инвариант публикации каждого resolved ticket, а не
финальная ручная уборка. Controller должен обновлять или валидировать map при
каждом resolve и непосредственно перед report.

### 8. Reviewer вернул schema-invalid finding

**Симптом.** Review response содержал Acceptance finding без criterion ID.

**Причина.** Это был дефект response contract, а не кода или acceptance
evidence.

**Решение.** Recovery сохранил тот же HEAD и повторил review с требованием
ссылаться на конкретные criteria. Повтор прошёл без расходования product repair.

**Состояние.** Решено в run штатным retry.

**Урок.** Invalid reviewer output нельзя превращать в product defect. Retry
должен сохранять HEAD, workers, budgets и идентичный acceptance contract.

### 9. Review обнаружил две локальные точки drift

**Симптом.** После функционального прохождения оставались дублированные
rejection-code mappings и две почти одинаковые terminal-transition functions.

**Решение.** В SV введены единые локальные helpers при сохранении публичных
семантических entry points; regression coverage расширено.

**Состояние.** Оба finding закрыты в третьем review.

**Урок.** Разрешённое AGENTS.md дублирование ради SV isolation не означает, что
нужно сохранять drift-prone duplication внутри одного SV-модуля. Граница
изоляции и локальная глубина модуля — разные решения.

### 10. Heartbeat жил дольше завершённого run

**Симптом.** После `done/ready` монитор продолжал просыпаться каждые пять минут.
Автоматическое удаление было отклонено, потому что пользователь явно разрешил
создание heartbeat, но не его удаление.

**Решение.** Монитор оставался тихим до явной команды пользователя, после чего
был удалён.

**Урок.** При создании мониторинга стоит сразу согласовывать lifecycle:
«остановить и удалить после terminal state» либо «оставить до ручной команды».

## Что сработало хорошо

- Checkpointed state сохранил workers, commits, budgets и counters после каждой
  остановки; запуск не пришлось начинать заново.
- Immutable contract digest и evidence guard остановили unsafe metadata drift,
  а не позволили молча принять изменённый artifact.
- Recovery отделял infrastructure failure, invalid agent output и product
  defect; это предотвратило тестовые заглушки и ложные deferrals.
- Red-to-green regression tests сопровождали каждый controller fix.
- Independent review поймал не только behavior issues, но и scope pollution,
  неверную классификацию evidence и нарушение Wayfinder процесса.
- Значимые heartbeat notifications давали фазу и прогресс без шума на каждом
  неизменившемся poll.

## Что следует изменить в процессе

1. **Сначала исправлять controller в отдельной maintenance-задаче.** Не включать
   orchestrator changes в feature run, даже если они нужны для его recovery.
2. **Проверять достижимость контракта до acceptance.** Implementation current
   checks обязаны иметь runner; статически определимые repository runners должны
   существовать в accepted HEAD.
3. **Исполнять repository gates из audited worktree.** Root checkout controller
   не должна подменять `.sandcastle/checks.sh` проверяемого HEAD.
4. **Считать accepted metadata controller-owned.** Любая коррекция criteria,
   checks или command требует bounded re-acceptance с новой verification и
   очисткой evidence старого digest.
5. **Поддерживать planning ledger инкрементально.** Новый resolved ticket сразу
   индексируется в map; final report повторно проверяет полноту.
6. **Фиксировать terminal policy heartbeat при создании.** Это устраняет
   отдельное ручное действие после успешного завершения.

## Безопасный recovery runbook

1. Зафиксировать точный phase, HEAD, contract digest, failing check и выполненные
   workers; не удалять run и не редактировать `state.json` вручную.
2. Классифицировать сбой: product, controller, contract metadata, agent response
   или external environment.
3. Построить минимальный deterministic red test на точный симптом.
4. Для controller defect выполнить fix в отдельном maintenance change set и
   принять его в base независимо от feature.
5. Синхронизировать feature integration branch с исправленным base, сохранив
   ancestry и уже выполненные workers.
6. Если меняется принятый контракт, архивировать старый digest/review, выполнить
   bounded re-acceptance, независимую verification и удалить evidence старого
   digest. Обычный retry недостаточен.
7. Повторить build, core и focused commands на одном committed HEAD.
8. Запустить новый review и fresh final audit; не считать recovery успешным
   только потому, что исходная ошибка больше не появилась.

Нельзя использовать pass-through test, test-only behavior shim, ручную правку
controller state, общий skip external checks или сброс counters: такие действия
маскируют проблему и делают evidence недостоверным.

## Долговременные действия

| Приоритет | Действие | Состояние |
|---|---|---|
| P0 | Отдельно вернуть integration-worktree source для `sv-core` и regression test из `338de1039` | Требуется maintenance-задача |
| P0 | Запретить mandatory/current external checks для implementation scope и исправить prompts | Требуется maintenance-задача |
| P0 | Добавить preflight существования repository command runners | Требуется maintenance-задача |
| P1 | Спроектировать общий controller-owned contract re-acceptance вместо feature-local recovery patch | Требуется design/review |
| P1 | Автоматически поддерживать Wayfinder map при resolve/publication | Требуется controller improvement |
| P2 | Добавить terminal cleanup policy в heartbeat creation prompt | Требуется automation convention |

До выполнения P0 текущий Sandcastle остаётся уязвим для повторения трёх
инфраструктурных остановок из пунктов 2–4. Успех `f5fbf91a` подтверждает
работоспособность подходов восстановления, но не означает, что эти защиты уже
присутствуют в текущем controller.

## Evidence

- [Финальный Sandcastle report](tasks/sandcastle/f5fbf91a-report.md)
- [SV-B-021 implementation record](tasks/stage-b/SV-B-021-account-manage.md)
- [Sandcastle process](sandcastle.md)
- [Журнал известных проблем](sandcastle-issues.md)
- Сохранённый durable fix: `fec73b88c Fix Sandcastle contract convergence`
- Проверенные recovery fixes, позднее удалённые из feature diff:
  `338de1039`, `3c6a4b69b`, `6c6e0426a`
- Финальный report commit: `c03e3ff33`

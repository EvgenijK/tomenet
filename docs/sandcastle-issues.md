# Проблемы настройки Sandcastle, Docker и Codex

Состояние на 2026-10-06. Это журнал **наблюдавшихся** проблем при настройке
Sandcastle и выполнении SV-B-007 и SV-B-021. Для каждой проблемы указано, что
удалось проверить, как её обошли и что ещё остаётся сделать. Планируемые проверки
отделены от уже произошедших сбоев. Секреты, коды входа и содержимое
`.sandcastle/auth/` сюда не записываются.

Основная конфигурация и команды описаны в [docs/sandcastle.md](sandcastle.md).
Подробное ретро второго запуска находится в
[ретро SV-B-021](sandcastle-retro-sv-b-021.md).

## Состояние после изменения процесса

- Пункты 6–8 ниже описывают исходные сбои SV-B-007. Новый
  `.sandcastle/workflow.mjs` сохраняет ID и фазу запуска, возобновляет ветку,
  передаёт замечания review в следующий раунд тикетов и после успешных gates
  переносит коммиты fast-forward в `modern_interface`.
- Конфигурация из пункта 9 теперь отслеживается Git. `auth/`, `runs/`,
  `worktrees/`, `reviews/`, `artifacts/` и `.env` остаются исключёнными.
- Пункт 5 частично остаётся ограничением внешнего процесса: права хостового
  Codex на Docker определяются его собственным sandbox, а не параметрами
  агента внутри контейнера. Один запуск orchestrator сокращает число ручных
  переходов через эту границу, но не отменяет возможную проверку прав.
- Недельный лимит читается из Codex app-server перед и после каждого агента.
  Порог 25% контролируется между агентами; один выполняющийся агент может
  превысить его до следующего контрольного чтения.

Sandcastle создаёт отдельные Git worktree и Docker-контейнер; установленный в
образе Codex CLI выполняет `$implement` и `$code-review`. Образ собирается из
`node:22-trixie-slim` по `.sandcastle/Dockerfile`; внутри также есть Clang,
SDL3/SDL3_ttf, Python-окружение для проверок и Git.

## Авторизация Codex

### 1. Подписка ChatGPT без API-ключа

- **Симптом:** первоначальный сценарий входа Codex в контейнере не был готов к
  работе с подпиской; у пользователя нет `OPENAI_API_KEY`.
- **Выяснили:** API-ключ не требуется для Codex CLI с входом через аккаунт
  ChatGPT. Для входа по коду устройства пользователь включил соответствующую
  настройку аккаунта.
- **Решение:** `npm run sandbox:login` запускает **хостовый** Codex CLI с
  отдельным `CODEX_HOME=.sandcastle/auth`. Этот каталог монтируется как
  `/home/agent/.codex` в dev-контейнер. `npm run sandbox:login -- status`
  подтвердил, что контейнер видит вход через ChatGPT; реальный запрос Codex
  после настройки сети тоже прошёл.
- **Состояние:** работает на текущем хосте. Это отдельная авторизация от
  обычного хостового Codex. `.sandcastle/auth/auth.json` игнорируется Git и
  должен оставаться вне коммитов, образов и отчётов.

### 2. Вход и доступ к сервису — разные проверки

- **Симптом:** успешный `codex login status` не гарантировал, что агент может
  начать запрос. При сетевых неполадках встречались `403` при авторизации из
  контейнера и сообщение `workspace routing discovery failed`.
- **Решение:** device/browser login выполняется хостовым CLI, после чего
  проверяется **реальный** запрос Codex из dev-контейнера. `login status`
  используется только для проверки доступа к кешу учётных данных.
- **Состояние:** рабочий запрос проверен. При повторении ошибки сначала
  проверять прокси и сетевой путь, не заменять подписку API-ключом без нужды.

### 3. Кратковременные ошибки соединения Codex

- **Симптом:** во время длительной работы появлялись WebSocket handshake EOF,
  переподключение и timeout обновления списка моделей.
- **Наблюдение:** рабочие сессии продолжались после таких ошибок; они сами по
  себе не доказывали потерю входа или изменений в Git.
- **Состояние:** перемежающийся сетевой сбой; при повторении проверять реальный
  запрос, состояние контейнера и сохранённые коммиты отдельно.

## Сеть и контейнеры Docker

### 4. Прокси хоста был недоступен из bridge-сети

- **Симптом:** локальный прокси слушал `127.0.0.1` хоста. В обычной bridge-сети
  Docker тот же адрес внутри контейнера указывает на сам контейнер; Codex не
  получал нужный сетевой путь.
- **Решение:** для `sandbox:dev` в `.sandcastle/main.mjs` установлено
  `network: "host"`, а `all_proxy`/`ALL_PROXY` хоста передаётся в контейнер.
  После этого запрос GPT-6 Sol с effort `high` прошёл. Контейнеры build/test/
  registry сохраняют bridge-сеть.
- **Ограничение:** dev-контейнер с сетью хоста видит локальные сервисы хоста.
  Не использовать его как сетевую изоляцию от официального игрового сервера.
  Для будущих клиент-серверных проверок нужен отдельный private test network,
  описанный в [плане](sandcastle.md#planned-local-server-and-visual-acceptance).

## Запуск, подтверждения и восстановление

### 5. Подтверждения на действия с Docker

- **Симптом:** внешний Codex несколько раз запрашивал разрешение на `docker
  exec` при сборке, тестах и ревью.
- **Причина:** это доступ **внешнего Codex к Docker на хосте**. Параметры
  `--ask-for-approval never --sandbox danger-full-access` относятся к агенту
  **внутри** контейнера и не меняют права внешнего процесса. Внутренний агент
  не запрашивал отдельного разрешения на запуск тестовых скриптов.
- **Что усугубило ситуацию:** после прерывания приложения проверки запускались
  отдельными прямыми `docker exec` вместо одной операции восстановления через
  Sandcastle. Поэтому внешняя граница прав проверялась повторно.
- **Состояние:** границу прав обойти настройкой внутреннего Codex нельзя.
  Нужна штатная команда продолжения/проверки существующего Sandcastle worktree,
  которая выполняет сборку, тесты и ревью одним управляемым запуском. Даже она
  может потребовать подтверждения доступа к Docker на хосте.

### 6. Прерывание приложения не остановило контейнер, но оборвало управление

- **Симптом:** первоначальный `npm run sandbox:dev` и его терминальная сессия
  пропали после прерывания приложения. Контейнер, отдельный worktree и коммиты
  SV-B-007 сохранились.
- **Восстановление:** работу продолжили в том же контейнере и ветке, вручную
  запуская Codex, сборку, проверки и review. Это сохранило production-код, но
  обошло обычную последовательность `.sandcastle/main.mjs`.
- **Состояние:** у `sandbox:dev` нет команды `resume`: повторный запуск создаёт
  **новую** ветку и worktree от текущего `HEAD`. Нужны сохранённый идентификатор
  запуска, возобновление существующего sandbox и идемпотентный финальный gate.

### 7. Отчёт review лежал вне worktree агента

- **Симптом:** повторный `$code-review` внутри worktree не смог прочитать
  прежний файл из `.sandcastle/reviews/` и работал по переданным в prompt
  замечаниям. Каталог отчётов расположен у orchestrator на хосте и игнорируется
  Git.
- **Состояние:** для возобновления надо передавать историю замечаний в prompt
  или монтировать/копировать отчёт как read-only вход. Итоговый отчёт должен
  соответствовать финальному `HEAD`, а не промежуточному коммиту.

### 8. Отдельная ветка Sandcastle не переносится автоматически

- **Симптом:** реализация SV-B-007 была закоммичена в worktree
  `codex/sandcastle-dev-4288c03b`, но сначала отсутствовала в
  `modern_interface`. Незакоммиченные изменения основной рабочей копии не
  копируются в новый sandbox.
- **Решение:** после проверки пересечений пять коммитов перенесены fast-forward
  в `modern_interface`; две независимые записи в `docs/sv-improvements.md`
  объединены, локальная запись сохранена отдельным коммитом `464417461`.
- **Состояние:** перенос SV-B-007 выполнен. Для будущих задач нужен явно
  описанный этап интеграции ветки после review.

## Воспроизводимость конфигурации и проверки

### 9. Скрипты Sandcastle пока не отслеживаются Git

- **Наблюдение:** `package.json`, `package-lock.json`, `.sandcastle/Dockerfile`,
  `.sandcastle/main.mjs`, `.sandcastle/login.mjs`, `.sandcastle/checks.sh` и
  `.sandcastle/.gitignore` на 2026-09-27 остаются untracked. Изменения
  `docs/sandcastle.md` тоже ещё не закоммичены.
- **Следствие:** коммиты SV-B-007 перенесены, но их Sandcastle-процесс нельзя
  восстановить только из Git checkout на другом хосте.
- **Действие:** отдельно проверить и закоммитить конфигурацию и lockfile;
  исключить `.sandcastle/auth/`, `worktrees/`, `reviews/`, артефакты и `.env`.
  Содержимое auth-кеша никогда не включать в Git.

### 10. Registry gate сообщает устаревший digest `src/makefile.sv`

- **Симптом:** две проверки capability registry сообщают `source-digest` для
  `src/makefile.sv` после добавления SV source в Makefile.
- **Разделение gate:** штатный `.sandcastle/checks.sh core` (сборка, SV
  production checks и dummy-video smoke) проходил. Более широкий прогон
  `tests/sv_*.py` дал 28/31: два падения registry и одно SDL lifecycle ниже.
- **Состояние:** manifest ссылается на закреплённую историческую ревизию;
  менять только SHA на текущие байты нельзя без пересмотра provenance.
  Отдельная задача записана в [списке улучшений](sv-improvements.md).

### 11. SDL dummy не поддерживает проверку сворачивания окна

- **Симптом:** `sv_lifecycle_native.py` в широком headless-прогоне получает
  ошибку `SDL_MinimizeWindow` под `SDL_VIDEODRIVER=dummy`.
- **Состояние:** это ограничение выбранного video backend для данного
  сценария. Запускать lifecycle/визуальные сценарии с настоящим дисплеем или
  Xvfb и оконным менеджером; dummy smoke не считать визуальной приёмкой.

## Контракт и recovery SV-B-021

### 12. Contract verification не сходился

- **Симптом:** run `b5cc0b14` повторял completeness verification без устойчивого
  прогресса.
- **Причины:** generic prompts содержали контекст другой задачи; authoritative
  sources не были ограничены; отображаемая команда `sv-core` скрывала stdin
  script controller; verifier feedback перезаписывался между раундами.
- **Решение:** `fec73b88c` добавил source-bounded prompts, фиксированный source
  set, накопительный feedback ledger и guard трёх одинаковых open sets.
- **Состояние:** исправление сохранено в текущей ветке.

### 13. Core gate читал manifest не из integration worktree

- **Симптом:** исправленный в run `.sandcastle/checks.sh` не влиял на реально
  выполняемый `sv-core`; controller продолжал запускать stale root script.
- **Решение для run:** `338de1039` переключил источник на
  `sandbox.worktreePath` и добавил regression test.
- **Состояние:** исправление помогло завершить run, но было удалено из feature
  diff как общий framework change. В текущей ветке проблема снова возможна;
  требуется отдельная maintenance-задача.

### 14. Валидный implementation contract оказался невыполнимым

- **Симптом:** test-gate остановился на mandatory/current external check без
  runner после успешных build и core checks.
- **Решение для run:** `3c6a4b69b` запретил такое сочетание и добавил bounded
  independently verified contract amendment.
- **Состояние:** recovery fix позднее удалён из feature diff. Текущий validator
  и prompts снова допускают противоречие; требуется отдельная maintenance-задача.

### 15. Контракт принял несуществующий command runner

- **Симптом:** accepted check ссылался на отсутствующий
  `tests/test_sv_account_create.py`; последующий worker commit изменил artifact,
  но не controller digest, и immutable guard остановил run.
- **Решение для run:** `6c6e0426a` добавил runner preflight и bounded
  controller-owned command-only repair с новой независимой verification.
- **Состояние:** подход проверен red-to-green и восстановил run, но fix удалён
  из feature diff. В текущей ветке preflight отсутствует.

### 16. Recovery fixes расширили feature scope

- **Симптом:** review SV-B-021 обнаружил в three-dot diff общие изменения
  Sandcastle controller, не относящиеся к account creation.
- **Решение:** controller changes удалены из feature diff; account implementation
  повторно прошла build, tests, три review-раунда и final audit.
- **Состояние:** SV-B-021 завершена с implementation verdict `ready`. Общие
  fixes из пунктов 13–15 должны возвращаться только отдельным change set, чтобы
  controller maintenance не смешивался с product feature.

## Startup preflight SV-B-024

### 17. Host networking скрывал Docker ID в hostname

- **Симптом:** запуск `1bd0d014` остановился до model dispatch с
  `IMAGE_UNVERIFIABLE`: probe возвращал hostname компьютера вместо Docker ID.
- **Причина:** при `network: host` Docker наследует UTS hostname хоста;
  проверка ошибочно требовала hex container ID в `os.hostname()`.
- **Решение:** probe получает фактический container ID из hostname bind mount
  в `/proc/self/mountinfo`, затем host Docker inspect проверяет точный образ.
  Для контейнеров с обычным hostname сохранён прежний путь проверки.
- **Состояние:** исправлено отдельным maintenance change перед новым запуском
  SV-B-024; regression test воспроизвёл сбой и прошёл после исправления.
  Execution/dispatch suites проверяют сохранение preflight до quota/inference.

## Запланированные, но ещё отсутствующие gates

- **Локальный сервер:** отдельный тестовый сервер и контент на private Docker
  network пока не запускаются из Sandcastle. Это нужно, чтобы не обращаться к
  официальному серверу при интеграционных проверках.
- **Визуальная проверка:** display-backed запуск клиента, ввод, снимки экрана
  и сравнение кадров пока не автоматизированы. Текущий dummy-video smoke
  проверяет запуск, но не внешний вид интерфейса.
- **Платформенная матрица:** текущий образ и команды покрывают Linux SV;
  MinGW/Wine, actual Windows и accelerated renderer остаются отдельными
  приёмочными проверками.

Подробный контракт будущего локального сервера и визуального gate находится в
[docs/sandcastle.md](sandcastle.md#planned-local-server-and-visual-acceptance).

## Systemic acceptance repair SV-B-024

### 18. Worker completion depended on its own post-assembly acceptance

- **Problem:** generated verification ticket 09 required independent final-HEAD
  controller gates before completion, while assembly admitted only completed
  workers. `blockedBy` did not expose the implicit phase cycle.
- **Affected code:** `.sandcastle/tickets.mjs`, `parallel.mjs`, `workflow.mjs`,
  execution binding and delivery validation modules.
- **Resolution:** typed worker ownership/phase and local delivery predicates;
  independently verified criterion-to-producer binding; controller artifact gates
  after assembly; versioned incident migration with a separate reviewed delivery
  receipt. Historical blocked status and acceptance finding remain unchanged.
- **Status:** implemented in the separate framework branch; not applied to actual
  run `0241e0da`. Review and real-run migration/resume remain separate actions.
- **Checks:** cyclic local predicates rejected before dispatch; ordinary blocked
  commits remain unmergeable; validated delivery permits assembly without fake
  completion; fixture migration preserves budgets, counters and open finding.

### 19. Container-local captures disappeared and proof aged with commits

- **Problem:** `/home/agent/sv-b024-final-audit/` was not host-mounted; teardown
  removed captures. Evidence/receipt commits changed the target HEAD again.
- **Affected code:** sandbox mounts, new controller-owned publisher/gates,
  acceptance closure and report descendant validation.
- **Resolution:** host-backed inbox, private controller snapshot validation,
  fsync/atomic seal, identity-bound durable receipts and restart readback;
  frozen target and outside-Git evidence. Strict report-only publication checks
  inspect every intervening commit, including change/revert sequences.
- **Status:** implemented and tested with safe synthetic Docker teardown and
  real collector/Git fixtures; no claim of completed SV feature acceptance.
- **Checks:** orphan/failed attempts, target mismatch, tamper, symlink/secret
  rejection and collector restart; actual Docker ephemeral loss versus retained
  sealed bytes. B-024 case counts use per-session logs, including no-send cases.


### 20. Installing controller maintenance diverges the stopped feature branch

- **Problem:** framework installation advances main while saved feature ancestry
  stays at the historical checkpoint; successful migration preparation could still
  reach an impossible final fast-forward after expensive acceptance gates.
- **Affected code:** `.sandcastle/verification-gates.mjs`, workflow freeze and
  publication; stopped-incident migration documentation.
- **Resolution:** read-only typed `INTEGRATION_BASE_DIVERGED` preflight before
  runtime/freeze/gate attempts and again before publication; separately reviewed
  maintenance provenance/scope reconciliation while stopped. Original task base,
  contract, history and counters are preserved; no automatic merge/scope exemption.
- **Status:** framework guard implemented; actual incident migration and its
  maintenance reconciliation remain unapplied and require separate review.
- **Checks:** real Git divergent main/feature blocks freeze unchanged; explicit
  reviewed merge allows final FF; main advancement after freeze blocks again.

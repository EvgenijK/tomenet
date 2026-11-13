# C003 — файловая основа и ресурсы SV

Статус: accepted, 2026-11-09, по явному решению пользователя; задачи 001–006 приняты.
[Evidence](sv-c003-evidence.md), [техническая сверка](sv-stage-c003-acceptance.md).
Ниже сохранён нормативный scope; late flow evidence остаётся pending.
[Задачи и порядок](tasks/c003/README.md), [сверка источников](tasks/c003/source-contract.md),
[полные canonical rows](tasks/c003/coverage.json).
C002 и его задачи 001–004 [приняты пользователем](sv-stage-c002-acceptance.md).

## Результат

Production SV разрешает владельца файла до операции, сохраняет собственную
историю ввода и общие DNA/notes, выполняет необязательное клонирование сообщений,
защищает публикацию экспорта и сообщает серверу фактическую resource identity.
Все десять `capability.files.*` outcomes C003 имеют одного владельца в задачах.
Snapshot сохраняет conditions/prerequisites/31 evidence obligations без сокращения;
это план, не native evidence и не изменение canonical registry.

Нормативная база: [AGENTS](../AGENTS.md), [CONTEXT](../CONTEXT.md),
[settings policy](capabilities/settings-policy.md), [архитектура](sv-architecture.md),
[ADR-0002](adr/0002-main-thread-state-ownership.md),
[ADR-0003](adr/0003-explicit-module-connections.md),
[ADR-0005](adr/0005-session-scoped-work.md),
[ADR-0006](adr/0006-errors-and-overload.md).
Старые allocation-B формулировки — источник обязательств, не повод повторять
приёмку B или внедрять прежний Sandcastle процесс.

## Ownership и минимальный путь

`U` — общий SDL3 user root; `S=U/sv` — client profile; `B` — game library.
История: `S/chathist-<account>.tmp`. DNA, notes и clone file: общий `U/user`.
Ресурсы читаются из U до B; изменения сохраняют shared owner. CFG/OPT остаются
собственными, без autosave. Временный суффикс истории не делает её disposable temp.

Минимальный SV-local file owner (предлагаемый каталог `src/client/sv/files/`)
передаёт consumers resolved path/owner и результаты операций. Он покрывает
read/stat/create/temp/rename/remove и publication, не только fopen. Формат DNA,
history ring и message semantics принадлежат соответствующим consumers.
Не подключать `c-files.c`/`c-util.c`/`nclient.c` целиком и не вводить общий legacy
refactor. Локальное дублирование допускается ради изоляции.

Существующие font/audio readers C002 подключать узко, сохраняя их declared fallback
и requested/effective state. Ошибочный overlay и missing overlay различаются;
конкретные fallbacks задаёт consumer, а не универсальное «пропустить все ошибки».
Для отсутствующих Lua/map consumers достаточно контракта paths; считать реальный
startup coverage только по работающим consumers. Guide в C003 остаётся approved
placeholder: `The guide is in development`, без чтения содержимого, bookmarks,
сетевых запросов и обновления. Guide length reporting не оправдывает загрузку Guide.

## Границы

- C004: preference loading; здесь только разрешение и операции с файлами.
- C040: отдельные birth restore/save outcomes не принимаются. C003 всё же обязан
  подключить shared DNA load/conversion/autosave к существующим production birth
  transitions: canonical scope прямо требует их. Передача fixture DNA через
  options не является файловой реализацией.
- C059: полный INS; C063: bookmarks. Их файлы не добавляются к history lifecycle.
- C044/C051: settings Save/import UI. Здесь не строить import или настройки UI;
  transient файлы не становятся импортируемыми личными данными.
- C006/C007/C055/C056: полные rendering/audio flows. Здесь точная identity и
  работа с ресурсными путями, без объявления отсутствующего renderer готовым.
- Полные chat/message/export surfaces и transfer protocol не подтягиваются.
  Решением пользователя 2026-11-09 утверждены core и существующие production
  consumers сейчас; отсутствующие flow evidence завершают поздние owners по
  [документам этапов](tasks/c003/README.md). Повторное
  согласование этой границы не нужно. Все canonical obligations сохраняются,
  непроверенные rows остаются pending в factual/canonical evidence.
  Последующим решением пользователь принял C003 сейчас; эти rows не gate приёмки.

History core сохраняет baseline MSG_HISTORY_MAX=2000 и move-to-newest, а не лимит
endpoint SvTextHistory=16. Записывается принятый несекретный chat input до
преобразований и Send_msg, без зависимости от успешной отправки. C028 подключает
этот путь; C016 строит full recall независимо от live feed шести строк; C029
публикует настоящие exports. C014/C032 доводят exit/final-review consumers по
собственным планам без переноса их outcomes в C003.

Version/font protocol handlers относятся к текущей C003 реализации; C002 audio
переиспользуется. Прежний UI text font ошибочно попадал в map-font поле PLAY;
C003 исправил false identity и сообщает source-backed missing/disabled до
появления map consumer. C006/C007 дополняют active identity/fallback/reinit evidence.
%s reporting copies ограничены 79 bytes+NUL (MAX_CHARS=80); local references
не сокращаются. Snapshot-only тест не принимает enabled resource branches.

## Реализация и проверка

Порядок: 001 → 002 → 003 → 004 → 005 → 006; после 001 независимые части
002–005 можно делать отдельно, сохраняя общую интеграцию. Это порядок работ,
не автоматическая команда запуска других агентов.

Обязательная платформа — [Linux](agents/platform-validation.md).
Windows/MinGW/Wine deferred; canonical условия не переписываются.
Добавлять только необходимый минимум core cases через production SV interfaces,
по возможности в existing entrypoints; число новых test executables заранее
не навязывается. Расширение набора проверок согласовать с пользователем.
Новый harness, general filesystem fault framework, полная B/B2 visual matrix,
packaging rerun и реальные аккаунты не требуются этим планом.

Минимальные группы связаны с tickets: resolved owners и safe publication;
history bounds/dedup/account/exit/secrets; DNA naming/conversion/cancel/autosave;
notes/clone occurrence order и ошибки; source-defined wire layouts и byte bounds.
Общие failure-lifecycle obligations получают применимый disposition по реальному
caller, а не исчезают: read/write/replace denied, изменение файла извне,
отмена/смена поколения, без false success и secret leakage. Существующий legacy
writer не участвует в новом lock protocol; не обещать межпроцессную гарантию,
которой нет в принятой policy.

При реализации создать `docs/sv-c003-evidence.md`: revision/build, команды,
expected/actual bytes и paths, production callers, все obligation dispositions,
known pending/deferred. 006 отдельно сводит core и поздние flow evidence;
проверенный core разрешает следующую реализацию без заявления полного PASS.
Пользователь уже явно принял C003 и задачи 001–006 без ожидания late flows;
повторное предложение/подтверждение приёмки не требуется. Текущие runtime/core результаты и pending приведены в evidence; canonical registry не изменён.

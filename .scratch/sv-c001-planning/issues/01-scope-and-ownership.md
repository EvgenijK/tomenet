# Границы C001 и владение outcome

Type: decision
Status: resolved
Assignee: agent c001_scope
Labels: enhancement, ready-for-agent

## Question

Как спланировать C001, сохранив одно outcome, Linux scope и границы соседних этапов?

## Answer

Дата: 2026-10-31. Это подготовка плана, без реализации, повышения статусов ledger или runtime acceptance.

C001 владеет только `capability.platform.package-launch`: реальный запуск извлечённого архива SV с изолированным client profile, разрешением поставляемых/пользовательских ресурсов и полным замыканием runtime-зависимостей. В catalog это order 3, block `platform`, expectedOutcomeCount 1. У строки ledger нет prerequisites; `implementation` и `evidenceStatus` — `pending`, `evidenceIds` пуст. Завершение B2 принято 2026-10-30, но его development runtime не доказывает готовность дистрибутива.

### Что относится к C001

- Определить воспроизводимый Linux amd64 архив, его состав, точку запуска и минимальную поддерживаемую Linux runtime-среду. Минимальная версия Linux/libc сейчас не выводится из слова amd64; её нужно явно зафиксировать в package contract до заявления dependency closure.
- Упаковать production SV binary, необходимые динамические библиотеки либо явно установить допустимые системные prerequisites; включить все обязательные для запуска ресурсы и лицензии. Отсутствие обязательной библиотеки/ресурса не маскировать успешным стартом из checkout.
- Обеспечить запуск из извлечённого архива вне репозитория, из независимого working directory, с изолированным профилем. Проверить реальные пути чтения/записи и отсутствие затрагивания чужих профилей.
- Подтвердить доступность bundled startup resources и применимый user-resource lookup production-путём. Одного списка ресурсов, population registry или логирования предполагаемых путей недостаточно.
- Разобрать применимость двух evidence obligations: `.result` и `.failure-lifecycle`. Обязателен запуск production SV и явный результат применимых missing/invalid resource и loader failures. Generic текст obligation не превращать в выдуманные cancel/concurrent-edit сценарии, если операция ими не владеет; неприменимость мотивировать в spec.

### Границы соседних этапов

| Область | Владелец | Граница C001 |
|---|---|---|
| CFG/OPT defaults, parse, CLI/environment overrides, no autosave | C002 | Не принимать settings outcomes под видом упаковки. Использовать существующий startup contract; обнаруженные semantic gaps передавать C002. |
| Полная ресурсная overlay/persistence семантика и file operations | C003 | Показать package-facing lookup на необходимых startup resources; не объявлять принятым общий files outcome и его остальные consumers. |
| Fonts/rendering corpus и управление font roles | C006/C007 | Включить необходимые startup bytes/profiles; не расширять C001 до полного raster/font parity. |
| Optional image/archive/download/sticky/gameplay integrations | C057 | Сохранить поддерживаемую конфигурацию сборки и необходимый runtime closure; не реализовывать и не принимать новые integrations. |
| Итоговый пакет полного клиента и cumulative live-entry review | verification gates | C001 доказывает запуск текущего SV, не заменяет финальную интеграционную приёмку всех последующих возможностей. |

Decision 38 разрешает вспомогательную инфраструктуру ранее завершённых блоков, но запрещает скрыто реализовывать prerequisite другого блока. Если package launch реально потребует ещё отсутствующего полного C002/C003 outcome, это конфликт порядка/metadata: сначала зафиксировать source-backed решение; не добавлять его молча в C001 и не принимать частично старый ID. Простая упаковочная адаптация существующего SV startup-пути не является новым acceptance ownership.

### Декомпозиция

Рекомендована последовательность из трёх implementation работ и отдельного нулевого gate:

1. Зафиксировать C001 spec, минимальный Linux package contract, startup resource inventory и local semantic sweep C001. Coverage snapshot должен содержать ровно один owned ID; A/B/B2 не переоткрываются.
2. Подготовить или адаптировать SV-local packaging/runtime wiring. Только эта работа владеет `capability.platform.package-launch`; необходимые исправления production startup входят сюда или оформляются subtasks без повторного ownership. Правки legacy/shared минимальны; дублирование ради изоляции допустимо.
3. Собрать evidence извлечённого Linux архива через реальный public production entrypoint и выполнить только согласованный минимальный набор проверок. Это supporting evidence task без собственного повторного ownership; запросить согласование до расширения проверок сверх пользовательского минимума.
4. Нулевой C001 acceptance gate сверяет результат, evidence и exact-once ownership; не получает capability ID. Реестр обновляется по фактам после evidence, не из-за создания плана.

Количество tickets можно сократить объединением 1 и 2, если это не смешивает decision и runtime результат. Не создавать тестовую копию behavior; минимальный core test нужен только для нового core поведения и вызывает production SV путь. Планирование не запускает runtime matrix. Windows/MinGW/Wine остаются deferred, их отсутствие не блокирует Linux acceptance и не означает PASS.

### Незавершённый общий reallocation

`SV-REALLOC-005` и `006` ещё planned, `007` in progress. Формально 006 зависит от 005, однако README разрешает готовить specs/tickets по уже стабилизированным slices после локального semantic sweep. Поэтому текущее исследование и локальный план C001 допустимы. Перед публикацией нормативных C001 spec/tickets нужно явно завершить C001 часть semantic sweep (включая связанные reconciliation/scenario ссылки) и связать её с частичным результатом 005/006. Это не разрешение отметить весь 005/006/007 completed и не глобальный блокер исследования C001.

Старые широкие C–G specs — historical research; их старые gates и cross-block требования нельзя механически переносить в C001. Canonical JSON сейчас не меняется.

## C001 semantic sweep — 2026-10-31

Объём: все 9 `reconciliation.rows`, содержащие `capability.platform.package-launch`, их точные inventory строки, все `reconciliation.scenarios`, текущие catalog/ledger, native evidence и пометки старых broad specs. Это локальный sweep; общий REALLOC-005/007 не закрывается.

| JSON row index (0-based) | Inventory и строка | Вывод для C001 |
|---:|---|---|
| 35 | `baseline:101` | Linux/Windows SDL3 production/test builds. Build prerequisites релевантны пакету, но frozen A ownership не меняется; Windows deferred. |
| 281 | `single-window-input-loops:322` | **Конфликт mapping:** строка Admin build (wall/tree/floor/sign actions), а mapping перечисляет только linux-build, package-launch, windows-build. Это не package-launch behavior. |
| 681 | `single-window-packet-state:466` | **Конфликт mapping:** USE_SOUND_2010/Windows меняет playback/timing effects, а не packet meaning. Package-launch не владеет audio семантикой. |
| 959 | `persisted-settings-and-files:24` | Legacy CFG смешивает connection/audio/graphics/Term layouts/credentials. Startup settings dependency контекст допустим, но полный split/parse/config contract принадлежит C002 и поздним owners. |
| 976 | `persisted-settings-and-files:41` | **Конфликт mapping:** Windows pack-specific audio override files и compatibility import. Windows deferred; audio/import не C001. |
| 1204 | `platform-deltas-and-packaging:18` | Прямая package row: Linux tar.bz2/tomenet/.so и Windows zip/exe/DLL. Применить к отдельному SV package identity, Windows deferred. |
| 1252 | `source-text-and-server-field-byte-contracts:88` | **Конфликт mapping:** item/spell/power/technique/ability name lookups, E79/E49/E40 и numeric ID. Это не startup/package behavior. |
| 1805 | `sv-file-ownership-draft:39` | **Конфликт mapping:** old SDL/Windows audio overrides, explicit compatibility import; сами не образуют C001 contract. |
| 2074 | `bundled-ttf-candidates:44` | Релевантно actual licensed CascadiaMono-Regular.ttf v2407.24 bytes/profile и OFL. Строка candidate report говорит «not selected here», но approved settings policy выбирает default и имеет приоритет; research metadata не shipping evidence. |

Inventory строки прочитаны из `docs/capabilities/inventories/<inventory>.md`, а не из возможной изменённой research-копии. Строки 281/681/976/1252/1805 выглядят семантически ошибочными mapping связями, а не новым требованием расширить C001 на admin/audio/import/item lookup. Их rationale («retain exact inventory contract») нельзя автоматически переносить в spec как package requirements. Удалять/переназначать mappings без общего owner audit в этой подготовке не предлагается.

**Локальный вывод:** catalog, manifest и ledger согласованы: C001 = 1 platform.package-launch, order 3, no prerequisites, pending. Границу можно явно описать и опубликовать план с этим scope, но sweep не является полностью clean: пять inventory→capability mappings требуют отдельного source-backed решения. До acceptance C001 нужно разрешить их applicability либо исправить reconciliation с нормативным обоснованием; нельзя молча считать их закрытыми package launch evidence. Это точечный дефект metadata, не доказательство отсутствующего игрового prerequisite и не основание передвинуть этап.

Связанные scenarios/evidence: ни один объект `reconciliation.scenarios` не содержит package-launch; `native-evidence.json` содержит 0 records и 0 fallbackRoutes, ledger evidenceIds пуст. В tests нет C001 runtime scenario: найденное `C001` в `sv_capabilities_checks.py` — отрицательная проверка stage/block metadata. Существующие B/B2 runtime доказательства не объявляют package-launch принятым.

Historical/stale references: broad C spec имеет явную superseded-пометку с C001–C064 и historical C615; broad G spec явно superseded, Guide перенесён C062–C064. Их внутренние C–G/A–F формулировки сохраняются как история, не local blocker. У package-launch ledger старые Linux+Windows builds/platforms; current Linux-only policy имеет приоритет, переписывать historical metadata в PASS нельзя. Research platform packaging:50 предупреждает о прежнем independent profile proposal; нынешний settings policy уточняет U/shared resources и S=U/sv, именно его применять.

Финальная декомпозиция по согласованию с ведущим агентом: `SV-C001-001` — единственный owner outcome, archive/launcher/minimal SV fixes и применимое evidence; `SV-C001-002` — нулевой acceptance gate, owns=[], зависит от 001. Отдельные infrastructure/evidence tickets не требуются. Spec и snapshot — документы этапа.

## Sources

- [CONTEXT](../../../CONTEXT.md): Client profile, Shared SDL3 resources, Capability, Coverage ledger.
- [Stage catalog](../../../docs/capabilities/stages.json), [manifest](../../../docs/capabilities/manifest.json), [native ledger](../../../docs/capabilities/native-coverage.json): точная строка `capability.platform.package-launch` и соседние stage IDs.
- [Functional stage plan](../../../docs/sv-functional-stage-plan.md): C001/C002/C003/C006/C007/C057 и slice rules.
- [Decision 38](../../single-window-sdl3-client/issues/38-reallocate-post-b-by-functional-block.md): one-block ownership, prerequisites, нулевые verification gates.
- [Reallocation README](../../../docs/tasks/stage-reallocation/README.md): текущие статусы 005–007 и параллельная работа по стабилизированным slices.
- [Settings policy](../../../docs/capabilities/settings-policy.md): Final persistence contract, Ownership and paths, Resource identity, Completion/evidence boundary. Manifest связывает package-launch с этой policy и baseline `src/makefile.sdl3`.
- [SV architecture](../../../docs/sv-architecture.md), [ADR-0001](../../../docs/adr/0001-separate-session-interactions-and-ui.md), [ADR-0003](../../../docs/adr/0003-explicit-module-connections.md), [ADR-0005](../../../docs/adr/0005-session-scoped-work.md), [ADR-0006](../../../docs/adr/0006-errors-and-overload.md): lifecycle, явное wiring и ownership ошибок/ресурсов.
- [Platform validation](../../../docs/agents/platform-validation.md), [issue tracker](../../../docs/agents/issue-tracker.md), [domain docs](../../../docs/agents/domain.md), [triage labels](../../../docs/agents/triage-labels.md).
- [B2 acceptance](../../../docs/sv-stage-b2-acceptance.md).

Graphify query `C001 package-launch platform` выполнен перед поиском; общий code graph не дал достаточного semantic контекста, выводы сверены с нормативными документами. Production код и canonical JSON не менялись.

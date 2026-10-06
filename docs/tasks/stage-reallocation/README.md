# Перераспределение этапов после B по функциональным блокам

Статус: in progress; каталог этапов, registry v3 и exact-once reallocation
выполнены, semantic sweep и детальные implementation tickets остаются
отдельной последующей работой. Нормативное решение —
[decision 38](../../../.scratch/single-window-sdl3-client/issues/38-reallocate-post-b-by-functional-block.md#answer).

Цель работ — заменить текущие широкие C–G на упорядоченные post-B этапы, каждый
из которых владеет outcomes только одного функционального блока. Текущие A8 и
B54, их ownership, prerequisites, specs и implementation tickets заморожены и
не входят в миграцию. Перераспределению подлежат 863 active outcomes из 46
post-B namespaces.

Каждый новый post-B этап владеет не более чем 50 outcomes. Замороженный B54 —
явное исключение и не перераспределяется. Поэтому крупные `items` и `options`
реализуются несколькими последовательными slices одного блока.

Новый этап может использовать infrastructure и regressions ранее завершённых
блоков, но не владеть их outcomes. Cross-block milestones и итоговые проверки
живут в `docs/tasks/verification/` и не являются acceptance stages.

## Работы

| Ticket | Результат | Зависит от | Статус |
|---|---|---|---|
| `SV-REALLOC-001` | Аудит prerequisites и проект упорядоченного block DAG | — | completed |
| `SV-REALLOC-002` | Regression lock неизменности A/B | — | completed |
| `SV-REALLOC-003` | Registry v3 и machine-readable stage catalog | SV-REALLOC-001, SV-REALLOC-002 | completed |
| `SV-REALLOC-004` | Exact-once reallocation всех 863 post-B outcomes | SV-REALLOC-003 | completed |
| `SV-REALLOC-005` | Semantic sweep stage-зависимых текстов и scenarios | SV-REALLOC-004 | planned |
| `SV-REALLOC-006` | Specs, coverage snapshots и implementation tickets новых этапов | SV-REALLOC-004, SV-REALLOC-005 | planned |
| `SV-REALLOC-007` | Полная structural/semantic validation миграции | SV-REALLOC-003…006 | in progress |

### SV-REALLOC-001 — prerequisite audit

Построить capability DAG для нынешнего C–G scope и его проекцию на 46 blocks.
Для каждого outcome зафиксировать необходимые более ранние outcomes, реальный
production consumer и минимальную точку, после которой блок можно проверить
целиком. Выдать:

- ordered block DAG и список допустимых параллельных ветвей;
- отчёт о межблочных циклах;
- решения, где один блок требуется разбить на несколько этапов;
- отдельно ранний minimum-playable путь и late-feature tail;
- подтверждение, что late blocks из decision 38 не были подтянуты вперёд общим
  primitive или parser-only зависимостью.

Аудит не меняет canonical JSON. Цикл нельзя устранять удалением нормативного
prerequisite без source-backed решения.

### SV-REALLOC-002 — A/B regression lock

Сохранить reviewed semantic projection всех A8/B54 rows: stage, prerequisites,
conditions, obligations, implementation/evidence state и B ticket ownership.
Добавить автоматическую проверку, что reallocation не меняет эту проекцию.

Если полный digest `native-coverage.json` в `stage-b/coverage.json` устаревает
только из-за post-B строк, обновить snapshot после доказательства неизменности
B-проекции либо заменить full-file freshness на отдельный projection digest.
Набор B IDs, DAG и owners при этом не меняются.

### SV-REALLOC-003 — registry v3 и stage catalog

Создать versioned machine-readable каталог этапов с stable stage ID, explicit
order, block, title и lifecycle. Обновить coverage/evidence schemas и validators:

- membership stage ID проверяется по каталогу;
- prerequisite/replacement order определяется explicit order, не сравнением
  строк;
- post-B stage содержит ровно один block;
- post-B stage содержит не более 50 outcomes;
- A/B разрешены как замороженные historical aggregate stages;
- ledger связывается с точной версией/digest каталога;
- negative fixtures покрывают неизвестный stage, duplicate order, mixed block,
  later prerequisite и изменённую A/B projection.

Сценарий Stage A checkpoint и Stage B plan checker должны продолжить работать.

### SV-REALLOC-004 — ledger reallocation

Перенести exact-once все 863 outcomes нынешних C–G в утверждённые block stages.
Обновить только относящиеся к ним prerequisites, replacement-stage metadata и
stage-dependent conditions/obligations. Не менять A/B rows и не повышать
`implementation` или `evidenceStatus` из-за самого планового переноса.

Результат обязан сохранять полный denominator 925, manifest identity и history
deprecated IDs. Каждый post-B stage имеет один block, от 1 до 50 outcomes и
ненулевой owned scope.

### SV-REALLOC-005 — semantic text sweep

Перепроверить вручную stage-ссылки в `native-coverage.json`,
`reconciliation.json`, policy/reconciliation Markdown и current planning docs.
Заменять нужно смысл consumer/gate, а не букву регулярным выражением. Особое
внимание требуется текстам `early`, `late`, `C/D/E caller`, Guide placeholder,
fallback retirement и full-option effect.

Исторические acceptance archives и research observations не переписываются.
Старые broad specs получают заметную пометку `superseded` и ссылку на decision
38, но сохраняют исходное содержание.

### SV-REALLOC-006 — stage specs и tickets

Создать единый current stage index и для каждого нового stage:

- spec с одним block, owned IDs, входными prerequisites, evidence boundary и
  explicitly deferred behavior;
- каталог `docs/tasks/<stage-id>/` с README и coverage snapshot;
- implementation tickets с exact-once ownership;
- нулевой stage acceptance gate без собственного capability ownership.

Старые `docs/tasks/stage-b/` остаются на месте и не копируются. Их moved-to-C
описания можно использовать только как historical research для новой
декомпозиции. Старые `docs/tasks/stage-g/` помечаются superseded in place после
появления новых owners. Cross-block live-entry и final-package gates создаются в
`docs/tasks/verification/`.

### SV-REALLOC-007 — validation

Прогнать и сохранить результаты:

1. schema metaschema и negative fixtures;
2. canonical registry + inventory + reconciliation validation;
3. exact 925 coverage, exact A8/B54 lock и exact 863 post-B allocation;
4. one-block-per-post-B-stage и ordered-prerequisite checks;
5. новый stage/task exact-once checker;
6. существующие Stage A checkpoint и Stage B plan checks;
7. локальные Markdown links и отсутствие active current документов,
   объявляющих C615/D162/E64/F1/G21 действующим планом.

Missing, failed или stale observation остаётся pending. Плановая reallocation не
является runtime evidence и ничего не принимает автоматически.

## Параллельное выполнение

После публикации результата SV-REALLOC-001 можно параллельно готовить catalog
contract (003) и A/B fixtures (002). После 004 semantic sweep удобно делить по
группам blocks, а stage specs/tickets создавать параллельно только по уже
стабилизированным slices. SV-REALLOC-007 выполняется на объединённом результате.

## Исторические документы

- `docs/sv-stage-c-spec.md` и `docs/sv-stage-g-spec.md` сохраняются как
  superseded snapshots.
- Decisions 29 и 36 сохраняют историю A–G; decision 38 имеет приоритет только
  для post-B allocation.
- Decision 37, Stage A evidence и весь активный Stage B plan остаются
  нормативными в своих прежних границах.

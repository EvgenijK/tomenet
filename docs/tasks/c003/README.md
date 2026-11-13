# C003 — задачи и план работ

Статус: accepted, 2026-11-09, C003 и задачи 001–006 приняты пользователем. [Спецификация](../../sv-stage-c003-spec.md).
C002 принят 2026-11-07. [Current core evidence](../../sv-c003-evidence.md),
[техническая сверка](../../sv-stage-c003-acceptance.md). Late flow evidence pending.

| Ticket | Результат | Зависит от | Owned outcomes | Статус |
|---|---|---|---|---|
| [SV-C003-001](SV-C003-001-file-owners.md) | Resolved owners, export collision и transient lifecycle | C001/C002 | `files.resource-overlay`, `files.export-collision`, `files.transient-owner`  accepted |
| [SV-C003-002](SV-C003-002-history.md) | Собственная bounded input history и exit/relog save | 001, `settings.load` | `files.history-load`, `files.history-save`  accepted |
| [SV-C003-003](SV-C003-003-dna.md) | Shared DNA и production birth integration | 001 | `files.dna-shared`  accepted |
| [SV-C003-004](SV-C003-004-message-sinks.md) | Private notes и необязательные message sinks | 001; settings options C002 | `files.notes-append`, `files.clone-console`, `files.clone-file`  accepted |
| [SV-C003-005](SV-C003-005-resource-reporting.md) | Version/audio/font resource reporting | 001; initial audio C002 | `files.report-resource-identity`  accepted |
| [SV-C003-006](SV-C003-006-acceptance.md) | Сверка evidence и представление к приёмке | 001–005 | 0 IDs  accepted |

Рекомендуемый порядок: 001 → 002 → 003 → 004 → 005 → 006.
После 001 независимые части 002–005 не зависят друг от друга. Исторический порядок
работ сохранён; пользователь принял C003 сейчас, без ожидания поздних consumers.
Helper-only PASS по-прежнему не заменяет фактическое lifecycle/wire evidence.

[Source contract](source-contract.md) фиксирует базу и открытые технические швы.
[Coverage snapshot](coverage.json) содержит ровно десять exact canonical rows,
31 obligation и hashes источников; каждый ID имеет одного ticket owner.
Canonical статусы, ownership других этапов и существующее evidence не меняются.

## Утверждённое решение — 2026-11-09

Пользователь утвердил: сохранить границы этапов, реализовать файловую основу и
существующие production consumers сейчас; зависимые flow evidence завершать
при появлении consumers на поздних этапах. Последующим явным решением того же
дня пользователь принял C003 без ожидания этих проверок.
Повторное согласование этой развилки не требуется. Canonical allocation, десять
IDs, 31 obligation и owners не меняются; pending не превращается в not-applicable.

Обязательства integration/evidence записаны непосредственно в документы этапов:
[C006](../c006/README.md), [C007](../c007/README.md), [C014](../c014/README.md),
[C016](../c016/README.md), [C028](../c028/README.md), [C029](../c029/README.md),
[C032](../c032/README.md), [C061](../c061/README.md). Каждый документ содержит
самостоятельные требования и условия завершения своей связки; читать прежний
общий verification список для планирования не требуется. Это целевые дополнения
планов этапов, не исчерпывающие полные планы всего их scope. Поздние владельцы
сохраняют evidence для своего этапа; сводку C003 можно дополнять для трассировки,
но возврат evidence не prerequisite его уже состоявшейся приёмки. Canonical
IDs/obligations остаются у C003.

Конкретный порядок текущей реализации:

1. 001: resolved owners, safe publication/replace continuation, transient cleanup;
   подключить существующие font/audio readers. Не добавлять exporter ради helper PASS.
2. 002: production history core на 2000 записей и account load/save lifecycle;
   типизированный accepted-input seam. Chat UI/submission связывается в C028.
3. 003: shared DNA load/conversion до birth setup и autosave по baseline transition.
4. 004: notes/clone delivery на message occurrence. Полный recall создаётся C016;
   текущая шестистрочная лента не служит источником полного export C029.
5. 005: version/font handlers и serializers текущего production protocol, reuse
   C002 audio. Исправить передачу UI text font как map font в PLAY: до появления
   map consumer использовать source-backed missing/disabled identity. Active
   map/font/graphics lifecycle evidence завершат C006/C007.
6. 006: свести текущий core результат и явные pending flow rows. Поздние этапы
   используют принятый C003 core; late flow evidence остаётся в своих этапах;
   это не canonical prerequisite PASS и не изменение порядка allocation.

Текущий core реализован и проверен на Linux, C003 принят пользователем.
Поздние реальные consumers/evidence и исправления относятся к своим этапам;
непроверенные flows не считаются доказанными по helper/snapshot тестам.
Canonical runtime registration остаётся отдельной работой.

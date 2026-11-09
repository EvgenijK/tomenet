# SV-C003-006 — Linux evidence и представление к приёмке

Статус: accepted, 2026-11-09, в составе явно принятого пользователем C003.
Labels: enhancement
Зависимости: 001–005.
Owned IDs: 0 IDs; только приёмка.
Контракт: [spec](../../sv-stage-c003-spec.md), [источники](source-contract.md), [полные rows](coverage.json).

1. Свести docs/sv-c003-evidence.md по всем десяти IDs и 31 obligations из coverage.json: production caller, expected/actual paths/bytes/wire, команды/revision/build, применимые failure dispositions.
2. Проверить отсутствие helper-only closure: late export/chat/active-resource consumer остаётся pending с owner и completion condition из собственного документа позднего этапа. Source validation и fixture injection не заменяют production evidence.
3. Только необходимый минимум core tests на Linux и затронутая build/integration проверка. Расширение набора проверок согласовывать заранее. Windows/MinGW/Wine deferred, не PASS. Не повторять полные B2/C001 suites.
4. Свести текущий core результат отдельно от полной приёмки. Пользователь явно принял C003 2026-11-09 без ожидания поздних consumers/evidence. Подготовить docs/sv-stage-c003-acceptance.md с техническим результатом и pending связками; приёмка уже дана, повторно её не запрашивать; не менять canonical JSON и не регистрировать evidence простым наличием документа.
5. Проверить сохранность соседних незакоммиченных изменений, изоляцию SV, CFG/OPT no-autosave и невключение C004/C040/C059/C063 outcomes.

Готовность: production wiring и применимые Linux core evidence записаны; все
owned obligations имеют честный disposition. Проверки расширяются только после
согласования. Пользовательская приёмка C003 и этого ticket явно дана 2026-11-09.

## Решение и отложенная интеграция — 2026-11-09

Утверждены core и существующие consumers; границы поздних этапов сохранены.
Самостоятельные integration/evidence требования закреплены непосредственно в
[C006](../c006/README.md), [C007](../c007/README.md), [C014](../c014/README.md),
[C016](../c016/README.md), [C028](../c028/README.md), [C029](../c029/README.md),
[C032](../c032/README.md), [C061](../c061/README.md). Общий verification список читать не требуется;
canonical IDs и obligations не перенесены. C003 принят пользователем 2026-11-09;
helper-only evidence не закрывает поздний flow.

## Техническая сводка — 2026-11-09

[Evidence](../../sv-c003-evidence.md) сводит все десять IDs/31 obligations без
canonical PASS. [Техническая сверка](../../sv-stage-c003-acceptance.md) готова;
late integration/evidence остаётся работой поздних этапов. Пользователь принял
C003 и задачи 001–006 без отлагательного условия. Native startup и app core различаются явно.

# C003 — историческая навигация отложенной интеграции

Статус: superseded as planning source, 2026-11-09.
Labels: enhancement
Тип: историческая запись переноса, не acceptance stage и не prerequisite чтения.

Решением пользователя 2026-11-09 файловый core и существующие production consumers
реализуются в C003, поздние consumers завершают flow evidence в своих этапах.
По последующему поручению пользователя все восемь обязательств перенесены
непосредственно в самостоятельные документы этапов. Для работы читать документ
своего этапа и его tickets; возвращаться к этому списку не требуется.

| Прежняя связка | Актуальный документ владельца | Смысл перенесённого обязательства |
|---|---|---|
| FI-01 | [C006](../c006/README.md) | Active map font, effective identity и reporting |
| FI-02 | [C007](../c007/README.md) | Active graphics, preparation/activation/reinit и reporting |
| FI-03 | [C016](../c016/README.md) | Полный recall и occurrence-safe notes/clone |
| FI-04 | [C028](../c028/README.md) | Принятый несекретный ввод и account history 2000 записей |
| FI-05 | [C029](../c029/README.md) | Реальные character/message exports через safe publication |
| FI-06 | [C014](../c014/README.md), [C029](../c029/README.md) | Quit/relog parent, independent input history save и exit-log child |
| FI-07 | [C032](../c032/README.md) | Final-review/tomb callers и сохранность данных до завершения child |
| FI-08 | [C061](../c061/README.md) | Настоящий screenshot producer, safe publication и last-successful reference |

Содержательные условия, dependencies, production callers и возврат evidence
находятся в указанных документах; эта таблица сохраняет только историю навигации.
Документы являются целевыми дополнениями планов, а не исчерпывающими спецификациями
всех outcomes этапа. Перенос требований не означает их реализацию или PASS.

C003 сохраняет десять capability IDs, 31 canonical obligation и своих ticket owners.
Integration owners поздних этапов не получают C003 IDs; canonical allocation,
prerequisites и registry статусы не меняются. Все отсутствующие flow evidence
остаются pending. [SV-C003-006](../c003/SV-C003-006-acceptance.md) сводит evidence
непосредственно из документов владельцев; C003 принят пользователем 2026-11-09. Эти поздние результаты
не блокируют и не переоткрывают C003 автоматически.

[План C003](../c003/README.md), [spec](../../sv-stage-c003-spec.md),
[индекс этапов](../../sv-functional-stage-plan.md).

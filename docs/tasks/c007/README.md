# C007 — план интеграции active graphics с C003

Статус: planned, 2026-11-09.
Labels: enhancement

Это компактная обязательная часть будущего полного плана C007 (15 rendering
outcomes), а не исчерпывающая спецификация этапа. Решение пользователя
2026-11-09: C003 реализует file/reporting core; поздний настоящий renderer
завершает active graphics flow evidence. Повторное согласование не требуется.

| Задача | Обязательный результат | Зависимость реализации |
|---|---|---|
| [SV-C007-001 — active graphics](SV-C007-001-graphics-resource-integration.md) | Actual mode/tiles/font identity, overlay, activation/reinit и возврат FI-02 evidence в C003 | Production core SV-C003-001/005; реальные font owners C006 и rendering C007 |

Включить задачу в production renderer/resource lifecycle полного плана C007.
Порядок: actual load/preparation → activation → publication effective state →
baseline server exchange → checks и возврат evidence. C006 отвечает за
[FI-01 map font](../c006/SV-C006-001-map-resource-integration.md); совместный
font/graphics snapshot не заменяет доказательство применения обоих consumers.

Все canonical IDs/obligations/allocations остаются в
[native coverage](../../capabilities/native-coverage.json); эта задача не
добавляет новый capability и не принимает C007. C003 уже принят пользователем. Только SV-local минимум
изменений, только необходимые Linux production-path core checks; расширение
набора согласуется заранее, Windows/MinGW/Wine deferred.

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.

## Интеграция C004 — дополнение 2026-11-09

Пользователь принял и закрыл C004; оставшиеся graphics preference consumers и
проверки теперь обязательны в C007. Добавлена самостоятельная
[SV-C007-002 — graphics preference stack](SV-C007-002-graphics-preferences-integration.md):
base/subset PRFs, offsets, shared glyph-subset identity, masks/raw pictures,
reset/reload, фактическое применение renderer и server-visible definitions.

Задача 002 дополняет existing C003/FI-02 задачу 001 и использует
[C006 map preference stage](../c006/SV-C006-002-map-preferences-integration.md).
Порядок: font base C006 → graphics source/preparation 001/002 → activation и
единый reporting → минимальные Linux consumer/peer checks. Production
`SvPreferences`/C003 owners сохраняются; второй parser или pixel oracle не вводить.
Graphics-enabled coupled overrides/reset/reload и actual renderer flow завершаются
именно здесь; font-based map acceptance C006 не ожидает C007. Для PRF effects
C007 доказывает generation/ownership и отсутствие replay, actual message/action
delivery принадлежит C016/C053 по своим задачам и не является hard gate C007.

Найденные при интеграции дефекты исправляются в C007 без автоматического
переоткрытия C004. C007 остаётся planned, runtime PASS и приёмка не заявлены.
Canonical allocation/IDs/conditions/obligations/runtime statuses неизменны;
новое evidence трассируется к [C004 source contract](../c004/source-contract.md)
и [приёмке](../../sv-stage-c004-acceptance.md), не отменяя её.

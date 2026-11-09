# C006 — план интеграции ресурсов карты с C003

Статус: planned, 2026-11-09.
Labels: enhancement

Это компактная обязательная часть будущего полного плана C006 (11 font outcomes),
а не исчерпывающая спецификация этапа. Пользователь утвердил 2026-11-09:
файловый core и существующие consumers реализуются в C003; реальные поздние
consumers завершают зависимые flow evidence без переноса реализации в C003.
Повторное согласование этой развилки не требуется.

| Задача | Обязательный результат | Зависимость реализации |
|---|---|---|
| [SV-C006-001 — active map font](SV-C006-001-map-resource-integration.md) | Реальный map font consumer, overlay, truthful identity и возврат FI-01 evidence в C003 | Production file/reporting core SV-C003-001/005; font lifecycle C006 |

При составлении полного плана C006 включить эту задачу в production map-font
путь; отдельный параллельный reporting или файловый алгоритм не вводить.
Задача относится к `fonts.map-select`, `fonts.fallback`, `fonts.live-failure`;
allocation всех capability IDs остаётся в canonical
[native coverage](../../capabilities/native-coverage.json).

Внутренний порядок: проверить доступные C003 interfaces → загрузка и активация
map font → mapping/graphics overrides и server definitions → reporting →
production-path checks и возврат evidence. Active graphics lifecycle завершает
[C007](../c007/README.md); отсутствие этого consumer остаётся явным pending,
не закрывается выбранным font или сериализованным snapshot.

Только минимальные SV-local изменения; проверки — необходимый минимум Linux core
через production path. Расширение набора согласуется по AGENTS.md; Windows/MinGW/
Wine deferred. План не означает реализацию или приёмку C006; C003 принят пользователем.

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.

## Интеграция C004 — дополнение 2026-11-09

Пользователь принял и закрыл C004, передав remaining consumers и проверки
следующим этапам. C006 обязан выполнить
[SV-C006-002 — map preference stack](SV-C006-002-map-preferences-integration.md):
effective map font → custom/system PRF sources → visual reset → font mappings →
server definitions/reload настоящего font-based map consumer. Настоящий resource
hook использует production `SvPreferences`; text font не подменяет map font.

Задача 002 дополняет existing C003/FI-01 задачу 001; обе входят в будущий полный
план C006. Порядок работ: actual map-font owner 001 и source selection 002 →
production preference reset/apply → минимальные Linux font consumer/peer checks
и evidence. Переиспользовать checks между задачами. C006 предоставляет seam для
graphics; graphics-enabled overrides/reset/coupled reload и actual flow проверяет
C007 по своей задаче, без зависимости приёмки C006 от C007. C006 доказывает
production generation/ownership PRF effects; actual message/action delivery
остаётся C016/C053 и не блокирует C006.

Late flow requirements остаются обязательными в C006; найденные дефекты
исправляются здесь без автоматического переоткрытия C004. C006 остаётся planned,
его реализация/приёмка не заявлены. Canonical IDs/conditions/obligations/allocation
и runtime statuses не изменяются; [приёмка C004](../../sv-stage-c004-acceptance.md)
и [source trace](../c004/source-contract.md) сохраняются отдельно от новых checks.

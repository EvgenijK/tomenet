# Defer Guide to stage G

Type: decision
Status: resolved
Assignee: user

## Question

Должны ли Guide и связанные с ним данные оставаться распределёнными между
этапами B, D и E либо быть реализованы отдельным этапом после существующих?

## Comments

### Решение пользователя — 2026-10-05

Пользователь потребовал полностью убрать Guide из существующих этапов и вынести
его в новый этап после них. Во всех существующих этапах вместо Guide должна
оставаться только заглушка с точным текстом `The guide is in development`.

## Answer

Добавлен этап G после A–F. Все постоянные `capability.guide.*`, загрузка и
сохранение Guide bookmarks, Guide update/management, а также атомарные
provider/build outcomes, включающие Guide, принимаются только в G (21 outcome).
В A–F каждая доступная по behavior baseline точка входа показывает нативную
заглушку `The guide is in development` и возвращает пользователя к фактическому
caller без чтения Guide, доступа к bookmarks, Guide-specific network/file
действий или terminal fallback.

F становится полной приёмкой non-Guide core без terminal fallback, но не
финальной приёмкой всего продукта. G заменяет заглушку полноценным Guide и
завершает общий coverage/platform/human acceptance. Заглушка является временным
stage behavior и не выдаётся за реализацию Guide capability.

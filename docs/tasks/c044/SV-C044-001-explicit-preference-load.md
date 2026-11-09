# SV-C044-001 — Явная PRF load, dirty settings и Save ownership

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: обязательная поздняя интеграция принятого C004; не новые capability IDs.

C044 связывает settings interface и явное preference loading с принятым
production core C004. Macro management caller — [C054](../c054/README.md),
normal messages/runtime — [C016](../c016/README.md)/[C053](../c053/README.md).
Это компактное дополнение, не полный план 22 outcomes C044. Здесь завершаются
C004 explicit caller `load.result/failure-lifecycle/allocation-checks`.

## Production seam и работа

1. Actual settings/named-load action вызывает `sv_preferences_load` живого
   stateful owner с `SV_PREF_EXPLICIT` и правильным logical source role. Старый
   options-only wrapper создаёт временный owner и уничтожает macros/mappings/
   effects; он не заменяет этот UI caller. Показать missing/unavailable/partial/
   fatal результат с contextual diagnostics; optional-file success не подходит
   к выбранному пользователем отсутствующему файлу.
2. Обычная PRF load сохраняет baseline effects и usable partial continuation;
   это не import C051 и не atomic data-only операция. Передавать messages/actions
   реальным recipients C016/C053 с source-defined gates/order, не скрывать их
   из-за settings UI. Закрытие/отмена UI не откатывает уже выполненное действие.
3. Изменённые compatible settings участвуют в preview/dirty state C044. Shared
   PRF X/Y могут изменить текущую сессию, но не выбирают destination Save:
   обычная игровая option сохраняется в own character scope, explicit global/
   class/named commands — в соответствующий own S OPT; class save не вводит
   class OPT autoload. `S/options.prf` остаётся собственным option entrypoint.
4. Реализовать Save/Cancel/dirty-close и conflicts по settings-policy: no exit
   autosave, changed-record merge, external value wins с сообщением conflict,
   failed publication сохраняет старый destination и active dirty edits. Не
   перезаписывать shared macro source лишь потому, что он дал X/Y значение.
   Retained mixed records/include/execution записи сохраняются для владельца
   Save, без flattening load chain в общий PRF; macro Save принадлежит C054.

## Минимальные Linux проверки и завершение

Через настоящий settings explicit-load action загрузить mixed PRF с X/Y и
message/action, проверить actual source/path, state/dirty changes, доставку
доступным recipients и pending bytes для отсутствующего C053 runtime;
missing и partial results не выглядят clean success.
Один production Save→reload с собственным destination подтверждает scope и
unchanged shared source; cancel/dirty-close/failed Save/conflict проверяются
существующим C044 production путём по применимой операции, без общего fault
framework и расширения suite сверх необходимого core.

Готовность C044 этой связки: работающий UI→stateful load→current recipients→
explicit Save flow с baseline partial behavior и ownership. Helper/parser PASS
не заменяет actual settings UI и message delivery. До появления C053 commands
сохраняют точные pending bytes/ownership, без объявления исполнения; C053 обязан
проверить combined explicit-load UI→runtime flow при своей реализации. Это
позднее evidence не gate C044 и не новый prerequisite C044→C053.

## Статус C004 и ограничения

C004 и задачи 001–004 приняты и завершены решением пользователя 2026-11-09;
эта поздняя интеграция и её необходимые исправления обязательны в текущем этапе,
но не блокируют и не переоткрывают C004 автоматически. [C004 evidence](../../sv-c004-evidence.md)
сохраняет границу проверенного core и отсутствовавших consumers. Три canonical
C004 IDs, conditions и девять obligations остаются в [snapshot](../c004/coverage.json);
эта задача не переносит allocation и не повышает canonical runtime statuses.
Результаты нужны приёмке своего этапа; обратная ссылка в C004 evidence допустима
для трассировки, не является gate его состоявшейся приёмки.

Только минимально необходимые Linux core checks через production путь; расширение
проверок согласовать по [AGENTS](../../../AGENTS.md). Windows/MinGW/Wine deferred.
Не создавать новый harness, general fault framework или полную visual matrix.
Изменения SV-local; соблюдать main-thread ownership, явные связи и generation
lifetime из ADR-0002/0003/0005/0006. Это план: реализация и PASS этой связки не заявлены.

# SV-C051-001 — Data-only preference import и linked-group failure

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: обязательная поздняя интеграция принятого C004; не новые capability IDs.

C051 завершает import часть C004
`preferences.include-failure.result/failure-lifecycle/allocation-checks`.
Canonical C004 IDs и descriptions остаются у C004; это обязательный production
import consumer своего позднего этапа, не повтор ordinary load.

## Production seam и работа

1. Применить действующий settings-policy import contract: явный source и preview,
   без silent import/source mixing. В `settings/preferences.[ch]` есть grammar,
   decoded data, raw records и provenance, но `sv_preferences_load` — обычный
   loader с действиями. Создать узкий SV-local data-only parse/stage путь,
   переиспользующий нужные production primitives без отдельного поведения для
   тестов; не вызвать normal load на live owner и затем очистить effects.
   `macro_only` фильтрует типы записей и не является import/no-execution режимом.
2. Во время preview/stage не применять X/Y к running session, не доставлять
   `#:/!/?`, не запускать macros/ресурсные запросы и не писать source. Includes
   и executable records сохраняются как данные. Привязать linked macro/include/
   stage group к выбранному source и его provenance; ordinary S option routing
   не должно незаметно подмешать destination в анализ legacy source.
3. Missing, cyclic и out-of-source include препятствуют всей связанной group,
   не unrelated valid groups. Диагностика указывает конкретный source/record;
   partial ordinary-load continuation не является разрешением импортировать
   половину связанной group. Destination wins default; explicit replacement
   применяется по согласованным scope/key, trigger/stage или whole-file identities.
4. Commit выполняется после review по полным independent groups с C003 resolved
   owners/publication; failure сохраняет прежний destination group, partial success
   сообщается по независимым groups. Cancel до commit не меняет sources или
   destinations и не оставляет pending commands. Уже shared данные не копировать
   на себя; source unchanged сохраняется для внешнего import. Credentials имеют
   отдельный explicit vault contract, без секретов в preview/log/temp files.

## Минимальные Linux проверки и завершение

Через actual import preview/stage/commit API пройти один linked fixture с
`#:/!/?`, mixed X/Y/macros/includes и различающимися source/destination. Проверить
zero delivery/command/resource effects, byte-identical source, whole-group skip
при missing/cycle/out-of-source и независимый valid-group commit. Cancel/conflict/
applicable publication failure сохраняют старый destination и честный per-group
результат. Использовать production import path и минимум его core checks; не
создавать общий filesystem failure framework.

Связка завершена при реальном reviewed import flow с изоляцией staging и
group guarantees. Source validation, вызов normal loader с подавленным UI либо
последующее `clear_effects` не доказывают data-only migration. Ordinary load
сохраняет прежние load-time effects и не переводится на import semantics.

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

# SV-C016-002 — Реальная доставка load-time messages

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: обязательная поздняя интеграция принятого C004; не новые capability IDs.

C016 подключает `#:` preference effects к настоящему normal/chat presentation
и historical occurrence source. Существующая задача C003 recall/file effects
сохраняется и используется для общей однократной доставки.
C004 `preferences.load.result/failure-lifecycle/allocation-checks` получают здесь
message caller evidence; loading/parser allocation остаётся у C004.

## Production seam и работа

1. В `settings/preferences.h` эффект `SV_PREF_MESSAGE` содержит уже подготовленные
   bytes, `chat`, source/line и generation. Подключить его из application assembly
   к семантическому локальному message ingress и существующему C016 recall/live
   маршруту, не к renderer и не к `Send_msg`. Marker253 задаёт локальный chat
   route; colour255 и literal braces после decoding не преобразовывать повторно.
2. Принять startup/global эффекты из owner `login/native-run.c` и character/reload
   эффекты из session owner, связанного [C011](../c011/README.md)/
   [C014](../c014/SV-C014-002-preference-lifecycle.md). Сейчас startup только
   сообщает retained counts и затем отменяет effects; это не показ сообщения.
   До session handoff нужен пригодный application recipient либо явная передача
   pending occurrence, без потери bootstrap messages и без replay на retry.
3. Создавать одну semantic occurrence на одну load-time message запись. Два
   одинаковых легитимных `#:` остаются двумя occurrences. Recall/search/redraw,
   перестройка UI и inspection `sv_preferences_effect` не повторяют сообщение
   или C003 clone/notes effects. Source bytes/order и normal/chat routing
   сохраняются независимо от шести строк live feed.
4. Согласовать acknowledge с другими recipients: `clear_effects` удаляет весь
   batch и action queue, поэтому message completion не уничтожает ещё не принятые
   option/actions. Старое поколение не добавляет сообщения в новую сессию.

## Минимальные Linux проверки и завершение

Production startup и существующий file-load→message ingress доставляют normal
`#:` с colour/literal brace и chat `{+` payload в реальный message model;
проверить exact route/content, порядок двух одинаковых occurrences и последующее чтение recall. Redraw/observer
и retry не увеличивают число occurrences/sink writes. Отмена до доставки и stale
generation не меняют новый model; action/option эффекты соседнего batch сохраняются
для их получателя. Не создавать отдельный test-only message consumer.

Готовность C016 этой связки: native application/session assembly действительно
показывает/сохраняет сообщения доступного production load и проходит lifecycle
проверку. Pending counter, тестовый callback и parser byte snapshot не являются
таким evidence. C044/C054 обязаны позже проверить свой actual explicit-load UI
вместе с этим recipient; C052 — сообщения реальных later/form reloads. Эти
будущие caller regressions не являются gate C016, и новый обратный prerequisite
на macro orchestration/runtime не вводится.

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

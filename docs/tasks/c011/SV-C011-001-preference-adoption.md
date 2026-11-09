# SV-C011-001 — Session preparation и adoption preferences

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: обязательная поздняя интеграция принятого C004; не новые capability IDs.

C011 завершает production preparation preferences для live owner; lifecycle после
handoff принадлежит [C014](../c014/SV-C014-002-preference-lifecycle.md), actual
messages — [C016](../c016/SV-C016-002-preference-messages.md), macro layers/runtime —
[C052](../c052/SV-C052-001-preference-orchestration.md) /
[C053](../c053/SV-C053-001-preference-runtime.md). Canonical C004
`preferences.bootstrap/load` result, allocation-checks и failure-lifecycle получают
здесь actual preparation evidence; stage allocation не меняется.

## Production seam и работа

1. `login/native-run.c` создаёт global `SvPreferences` через bootstrap и передаёт
   borrowed pointer в `SvEndpointOptions.preferences`. `login/flow.c` создаёт
   `attempt.preferences` через `sv_preferences_character`, который клонирует
   данные с `copy_effects=false`. Global startup intents остаются в исходном owner;
   нынешний cleanup уничтожает их. Создать явную передачу/удержание global данных
   и ещё не доставленных startup effects в preparation/handoff API; actual
   application/session delivery завершают C014/C016/C053 по ответственности.
   Одного character clone недостаточно: нельзя потерять global `#:/!/?/X/Y` intents.
2. Подготовить реальный preparation/handoff payload с owned character preferences
   и pending effects до attempt teardown, явно описать передачу lifetime через API.
   C014 связывает actual connection и live owner и проверяет integrated handoff;
   это не gate C011. App-scoped данные и session-derived effects имеют
   разных владельцев. Generation0 bootstrap intent не превращать автоматически
   в повторную команду каждого retry; сохранять identity исходной load occurrence.
   Pre-login message направляется приложению, command readiness/gates определяет
   C053 по baseline, без нового blanket запрета startup actions.
3. Дать C052 явный hook/store contract того же stateful preference owner для
   approved macro layers, сохранив current character values до первого options
   packet. Реальная orchestration C052 и её later reload evidence не gate C011.
   Effective map resource hooks задают [C006](../c006/README.md)/[C007](../c007/README.md); text font не
   подменяет map resource. При отсутствии позднего consumer явно удерживать
   данные/результат и pending evidence, не создавать фиктивную отправку.
4. Указать в handoff/assembly contract обязательных получателей option/message/
   action effects; actual delivery поздним recipients не требуется для C011.
   `sv_preferences_effect` — borrowed inspection, не consume;
   `sv_preferences_clear_effects` удаляет все виды effects и action queue сразу.
   Подготовить ownership/ack contract по occurrence: общий batch освобождается
   после обработки всех получателей либо явной отмены; message consumer не вправе
   очистить ещё не принятые commands/options. Никакой общей event bus ради этой связки.

## Минимальные Linux проверки и завершение

Пройти реальный native preparation до его production handoff API с различающимися
global и character `#:/!:/X/Y` records. Проверить actual payload/owner lifetime,
сохранность startup и character bytes/order, ранние option packets и отсутствие передачи в credential
fields. Actual socket→live-owner adoption проверяет C014. Повтор inspection или
подготовки того же handoff не повторяет effects; отказ/отмена preparation
освобождает данные, не оставляя stale commands. Использовать
существующий managed peer, без реального аккаунта и нового runtime simulator.

Готовность C011 этой связки: production preparation передаёт оба источника
preferences в явный handoff contract с evidence paths/bytes/generation/ownership,
без потери startup intents. Не ждать C014/C016/C053 для завершения preparation
части: C014 реализует actual live adoption/close, C016 и C053 обязаны позже
доказать combined flow от этого handoff до своих настоящих recipients. Это
их обязательства, не обратные prerequisites C011. Сохранённая queue не доказывает
игровое выполнение и не принимает outcomes поздних владельцев.

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

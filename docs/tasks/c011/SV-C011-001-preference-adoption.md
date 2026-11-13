# SV-C011-001 — Session preparation и adoption preferences

Статус: **accepted** — Linux preparation/core, 2026-11-11, по решению пользователя.
Labels: enhancement
Owned ID: `capability.session.load-profile-input` (6 obligations).
Также обязательная интеграция принятого C004; новых canonical IDs нет.

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
   approved macro layers, сохранив current character values до options snapshot внутри PLAY. Реальная orchestration C052 и её later reload evidence не gate C011.
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
сохранность startup и character bytes/order, exact options внутри PLAY и отсутствие передачи в credential
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

## Полный profile scope — уточнение анализа 2026-11-11

001 владеет общим production preparation payload/API для 002/003/004: owned
character preferences, app preferences с явным lifetime, pending occurrences,
accepted identity, copied results и readiness. `SvEndpointOutcome` сейчас лишь
копирует наблюдения; он не переносит ownership socket/preferences/Lua.
Сохранять baseline порядок effects и action queue: queue формируется prepend,
её порядок нельзя вывести только из порядка effect records.

Кроме C004 integration выше, сверить действующие C002/C003/C005 startup пути:
независимые SV CFG/OPT/history, normal/roguelike keymap, requested/effective
font/tiles/audio references и early character options. Не переписывать принятые
cores; доказать actual preparation caller и удержание данных до payload.

- `startup-ins`: подготовить ordered rules из первого доступного shared INS:
  character → class → trait → race → global (`c-init.c:initialize_player_ins_files`).
  Старые rules очистить; не объединять файлы. Identity берётся из 004, а не draft.
  Missing/old format и допустимая conversion сохраняют baseline. Runtime apply
  на inventory update остаётся C021, полное управление/Save — C059; закрепить
  получателя и pending combined evidence, не ставить успешный no-op.
- `startup-audio-packs` / `startup-audio-device`: использовать существующие
  `settings/audio.[ch]`, actual initial open и `sv_login_send_audio`, сохранив
  requested/effective identities, device values, disabled/no-device/failure
  degradation и lifetime snapshot. Startup не пишет CFG; no-device не закрывает
  login. C055/C056 выполняют later live playback/configuration.
- Macro precedence, waits/flush/raw execution и form layers остаются C052/C053;
  C011 удерживает данные и обязательный hook, не объявляет execution проверенным.

Необходимый минимум случаев объединить с существующим managed-peer preparation
check, различая подтверждённые startup projections и pending recipients.
Полные descriptions шести obligations сохраняются в [snapshot](coverage.json).

Завершение 001 требует наблюдаемого результата всех startup projections:
effective CFG/OPT/history/keymap и source identities; выбранного most-specific
INS, ordered rules и disposition missing/conversion; actual audio snapshot с
requested/effective values и degradation; pending occurrences без replay.
Одной передачи двух preference pointers недостаточно. Отсутствующее применение
INS к inventory или macro execution отмечается отдельно за поздними owners;
собственные startup read/selection/retention этой задачи не откладываются.

## Порядок подготовки

Разделить ранние и identity-dependent данные, чтобы не получить цикл 001→004→PLAY:

1. App bootstrap CFG/OPT/preferences/resources не требует accepted identity.
2. После принятого выбора имени загрузить ранний `<character>.opt` через
   существующий `sv_preferences_character` и сохранить effective options
   для включения в PLAY. Отдельный pre-PLAY
   `PKT_OPTIONS` сервер этой фазы не обрабатывает. Это pre-initialization, не заявление
   о подтверждённых сервером race/class/trait. Не ждать PKT_CHAR_INFO для PLAY.
   Повтор preparation/poll не загружает тот же OPT и не ставит PLAY
   заново; character effects уже на этой фазе имеют owner и явную доступность.
3. После PLAY/startup END 004 принимает authoritative identity. INS selection и
   identity-dependent hooks используют её; предварительное имя и birth draft
   не подменяют отсутствующие поля. Full macro layers выполняет C052.
4. Payload явно показывает готовность каждого участка: transport/startup,
   identity и identity-dependent profile. Переносимый owner может удерживать
   ещё не готовые данные, но их состояние не называется complete preparation.
   First-load occurrence и повторное обновление identity различаются, чтобы
   одна запись не исполнялась повторно от poll/view/repaint.

## Comments

2026-11-11 — реализация и проверка по запросу пользователя. `login/preparation.[ch]` удерживает оба preference sources/effects с ack по occurrence, history, effective keymap/audio и full resource refs. Native bundle принимает fonts/audio/graphics; owned MapFont содержит actual character definitions. Startup INS реализован `settings/startup-ins.[ch]`, выбор идёт по accepted identity. Snapshot и реальные resources переживают возврат native runner.

[Финальное evidence, команды и ограничения](../../sv-c011-evidence.md).
Пользовательская приёмка не заявлена; Windows проверки отложены.


2026-11-11 — follow-up по запросу пользователя закрыть оставшиеся проверки самого C011. Проверены actual initial audio enabled/subset/fallback, missing pack и no-device через native C011; exact PLAY/PKT_AUDIO, сохранность CFG и lifetime ресурсов после return/destroy. Native birth пройден до accepted identity. Character options остаются ранним snapshot, но standalone pre-PLAY OPTIONS убран по actual-server finding: значения передаются внутри PLAY.

Результаты и границы — [обновлённое evidence](../../sv-c011-evidence.md). Работы следующих этапов не выполнялись.


2026-11-11 — пользователь явно принял C011 и задачи 001–006 в проверенных границах Linux preparation/core. Задача завершена; [решение о приёмке](../../sv-stage-c011-acceptance.md). Исторические записи выше отражают состояние до приёмки; поздние obligations и canonical runtime statuses не повышаются автоматически.

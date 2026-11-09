# SV-C014-002 — Preferences: handoff, retry и once-only effects

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: обязательная поздняя интеграция принятого C004; не новые capability IDs.

C014 завершает live lifecycle уже принятого C004 preference core. Preparation
owner — [C011](../c011/SV-C011-001-preference-adoption.md); actual recipients —
[C016](../c016/SV-C016-002-preference-messages.md) и
[C053](../c053/SV-C053-001-preference-runtime.md). Это отдельная связка рядом
с существующей C003 history/export задачей; её требования сохраняются.
Связаны C004 `load/bootstrap/include-failure.failure-lifecycle` и caller evidence.

## Production seam и работа

1. Сейчас `login/flow.c:sv_login_flow_end_attempt` уничтожает `attempt.preferences`,
   а global owner живёт в `login/native-run.c` до cleanup. Вместе с настоящим
   enter-game/handoff явно передать owner либо необходимое принадлежащее live
   сессии состояние до teardown; не оставлять borrowed указатель на освобождённый
   owner. App-scoped данные сохраняются отдельно от session generations.
2. Закрепить generation/occurrence для ещё не доставленных effects и action queue.
   Retry, reconnect, portal relogin и quit отменяют старые session commands,
   освобождают payload и continuation; свежий character overlay строится из base,
   не из прошлого персонажа. Global startup intent не копировать заново в каждый
   attempt и не терять из-за `sv_preferences_clone(..., false)` в character path.
3. Доставка проходит через явную assembly на main thread. Повторный step/redraw,
   observer read и повтор acknowledgement не исполняют уже доставленное.
   `sv_preferences_clear_effects` очищает одновременно messages/options/actions
   и queue: вызывать после общего согласованного consume/cancel, а не после
   первого recipient. Cancel не откатывает ранее выполненное игровое действие.
4. Command start/wait/completion определяет C053; C014 сохраняет parent/session
   continuation и отбрасывает stale результаты. Не добавлять direct raw key send
   в login editor или новое `in_game` правило для допуска PRF actions.

## Минимальные Linux проверки и завершение

Через существующие production flow/app API пройти handoff, retry/смену character,
reconnect и quit с различающимися pending и уже доставленными occurrences.
Проверить actual bytes/generations у live owner и уже доступных получателей,
отсутствие повторного adoption и допуска stale commands, освобождение abandoned
owners, отзыв borrowed bindings до destroy.
Использовать минимальные относящиеся managed-peer/lifecycle cases, без повторения
полной login/visual matrix. CFG/OPT не сохраняются на exit.

Готовность C014 этой связки: actual enter-game→live preference owner→close/retry
lifecycle с generation/cancel и доступными recipients. Placeholder handoff и
один снимок queue не заменяют реальное adoption/teardown. Отсутствие C016/C053
не gate C014: actual delivery/execution combined flow, включая stale-send и
повторные acknowledgements у реального получателя, обязаны завершить C016/C053
при появлении своих consumers. Новые обратные canonical prerequisites не вводятся.

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

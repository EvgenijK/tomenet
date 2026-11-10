# SV-C014-004 — actual Lua runtime adoption, close и relog lifecycle

Статус: planned, 2026-11-10.
Labels: enhancement, ready-for-agent
Owned IDs: 0 новых IDs; integration obligation принятого C009.
Зависимости реализации: production Lua runtime/completion owner C009,
C011 session preparation/handoff contract; actual transfer caller C010.

## Основа и production путь

[C009 и задачи 001–003 приняты пользователем](../../sv-stage-c009-acceptance.md)
2026-11-10 в текущих границах Linux core. Эта самостоятельная работа обязательна
C014; она не блокирует и не переоткрывает C009 автоматически.
[Evidence C009](../../sv-c009-evidence.md),
[completion contract](../c009/SV-C009-002-reload-lifecycle.md),
[источники](../c009/source-contract.md), [CONTEXT](../../../CONTEXT.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).

Использовать `src/client/sv/lua/runtime.h` и `lua/reload.h`. `SvLuaReload` —
application-scoped owner; он владеет runtime и Guide metadata. Внешний
diagnostics context должен жить всё время существования binding. Ранний ordinary startup уже
создаёт owner в `login/native-run.c:sv_endpoint_run`; он пока уничтожается на
выходе из этого caller. Реальный live handoff должен закрепить ownership до
pregame cleanup, без borrowed указателя на уничтоженный owner и без новой
копии Lua/transfer поведения для live consumer.

## Работа

1. Связать C011 preparation с actual live connection/session owner: передать
   тот же Lua owner либо явно сохранить application owner и передать live binding,
   вместе с актуальным generation, server features и результатами подготовки.
   Diagnostics context живёт дольше binding; delivery не re-enter runtime.
   Adoption выполняется один раз. Не повторять `sv_lua_reload_begin` для уже
   подготовленного поколения только из-за handoff/redraw: begin очищает runtime
   и metadata. Новый session generation получает собственные feature inputs
   через согласованный C011 preparation путь.
2. Перед вызовом session-specific API проверять живой generation. Close, retry,
   reconnect/portal relogin и отказ handoff прекращают приём старых INIT/END,
   provider completions, callbacks и session commands до освобождения их payload.
   Отозвать borrowed bindings/continuations прежде, чем уничтожается их owner.
   Запоздалый результат не загружает scripts, не меняет server flags/metadata,
   не повторяет audio warning и не воздействует на новое соединение.
3. На закрытии текущего поколения вызвать `sv_lua_reload_close` и освободить
   session-owned данные/continuations ровно по ownership. Runtime schools/spells,
   metadata и borrowed pointers не переживают clear/reload/close; данные для
   следующего кадра/поколения копируются только по явному контракту.
   Повтор cleanup и cleanup старого generation не повреждают новый runtime.
4. Сохранить application owner между sessions: `updated_audio/updated_guide`
   имеют process-sticky lifetime, включая accepted INIT → disconnect до END →
   последний END следующей session. Не сбрасывать их при relog и не пересоздавать
   owner для скрытого batch/session reset. Shared script files живут независимо
   от session; close не удаляет и не откатывает их. Полное
   `sv_lua_reload_destroy` выполняется один раз при application shutdown после
   отключения всех bindings/callbacks.
5. Принять от C010/C011 отдельные transfer/runtime/Guide results, не терять failure
   при handoff и не объявлять stale scripts успешной подготовкой. C010 потребляет
   completion каждого END до следующего END; один latest-result slot не является
   очередью. C014 не заменяет этот wire caller и не повторяет его reload/ACK.
   Согласованный reload после failed last close сохраняется; успешный reload не
   отменяет transfer failure. Решение readiness/продолжения обязательного startup
   остаётся у actual session lifecycle с явными результатами, по контракту C011.

## Минимальные Linux проверки и готовность

В существующей минимальной проверке actual live lifecycle пройти preparation →
adoption/handoff → close/relog с доступным production Lua owner; проверить
сохранение владельца и результатов, отзыв borrowed bindings до destroy и отсутствие
повторного begin/adoption/effect от step/redraw/повторного cleanup. Placeholder
`LIVE_HANDOFF` или isolated C009 API test не подтверждает этот flow.

В том же необходимом lifecycle case проверить accepted audio/Guide INIT →
disconnect до END → новое поколение с другими server features → last END:
process-sticky flags сохранены у application owner; session-derived Lua данные
и server feature flags принадлежат новому соединению. Поздний
callback/completion предыдущего поколения не действует; cancel/ошибка handoff
освобождает принадлежащие payload/runtime данные и не удаляет shared files.
Отдельно зафиксировать, какой участок actual C010/C011 consumer был выполнен;
если его ещё нет, указать конкретный pending flow без фиктивного PASS.
Wire framing, publication/ACK bytes и transfer failure checks завершаются C010;
готовность preparation — C011, а эта задача принимает actual adoption/close/relog.

Готовность связки: реальный live consumer с доказанными ownership, generation,
cancel/cleanup и sticky lifetime. Только необходимый минимум production-path
Linux checks; расширение согласовать по [AGENTS](../../../AGENTS.md).
Новый harness, отдельная Lua suite, broad visual matrix и Windows/MinGW/Wine
проверки не требуются; Windows deferred по
[правилу платформ](../../agents/platform-validation.md).

## Evidence и статус принятого C009

В evidence C014 указать actual callers/revision/build, команды, expected/actual
lifecycle/results и pending limits. Trace — к `obligation.lua.reload.result`,
`obligation.lua.reload.failure-lifecycle`, `obligation.lua.reload.allocation-checks`.
Canonical owner/acceptance allocation C009, prerequisites, условия и pending
registry statuses не менять. Core evidence C009 не даёт PASS отсутствующим
C010/C011/C014 flows.

Найденные здесь необходимые исправления выполняются в C014. Они не блокируют
и не переоткрывают C009 автоматически; evidence backlink в C009 служит только
трассировке и не является обратным gate его состоявшейся приёмки.
Сейчас подготовлена задача; реализация, runtime evidence и приёмка C014 не заявлены.

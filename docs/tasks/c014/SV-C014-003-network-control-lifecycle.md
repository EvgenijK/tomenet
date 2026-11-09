# SV-C014-003 — live lifecycle и deferred caller queues controls

Статус: planned, 2026-11-09.
Labels: enhancement, ready-for-agent
Owned IDs: 0 новых IDs; integration obligation принятого C008.
Зависимости реализации: C008 production controls, C011 preparation contract;
live map evidence согласовать с C013 без дублирования suites.

## Основа и production путь

[C008 принят](../../sv-stage-c008-acceptance.md) в границах core. Эта работа
обязательна C014, не блокирует и не переоткрывает C008 автоматически.
[Evidence](../../sv-c008-evidence.md), [source contract](../c008/source-contract.md),
[CONTEXT](../../../CONTEXT.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).

Actual login/live owner связывает C011 handoff с `SvApp`, production protocol,
`SvNativeControls` и `SvNativeInput`. Sink callbacks не re-enter app; resume
уведомляет owner для последующей обработки deferred incoming caller work.

## Работа и минимальная Linux проверка

1. Выполнить actual handoff socket/generation/version/setup/unread bytes/pending
   sends ровно один раз. На переходе не терять и не повторять controls/replies.
2. Если текущий caller откладывает incoming packets, после свежего pause ack
   реально обработать его очередь по baseline `Flush_queue`, сохранив order и
   nested input context. Не очищать outgoing socket bytes и не заменять результат
   счётчиком resume callback. При очередной PAUSE очередной owner ждёт свой ack.
   Если применимого caller ещё нет, зафиксировать конкретного позднего owner
   и неподтверждённый flow, не создавать тестовую копию queue behavior.
3. Close/relog/retry отменяет pause, pacing deadlines, confirmation waits и
   callbacks старого generation; borrowed native resources не переживают owner.
   Pending SDL events и late ack/frame/confirm не меняют новую сессию.
4. Через тот же production lifecycle проверить handoff с partial control,
   nested pause и interleaved updates, deferred caller drain при наличии caller,
   close/reopen и stale rejection/no replay. Проверки игрового map result — C013,
   настоящего macro wait completion — C053.

Готовность: actual live lifecycle evidence и явно разобранные caller queues,
trace к `obligation.network.*` C008, с точными pending limits отсутствующих flows.
Дефекты, обнаруженные здесь, исправляются в C014 без автоматического переоткрытия
C008; backlink evidence служит трассировке. Canonical allocation/statuses
сохранены. Только минимальные production-core checks на Linux; расширение
проверок согласовать по AGENTS, Windows/MinGW/Wine deferred.

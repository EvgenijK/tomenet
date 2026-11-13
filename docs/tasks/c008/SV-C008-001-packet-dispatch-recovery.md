# SV-C008-001 — dispatch, unknown recovery и payload-free controls

Статус: accepted пользователем 2026-11-09 в текущих границах Linux core.
Labels: enhancement
Owned IDs: `capability.network.unknown-packet`, `capability.network.keypress-stub`, `capability.network.end-marker`.
Зависимости реализации: Существующие B protocol/transport owners; interface к 002/003.

## Основа и границы

Нормативны [спецификация C008](../../sv-stage-c008-spec.md),
[source contract](source-contract.md), [точный planning snapshot](coverage.json),
[CONTEXT](../../../CONTEXT.md), [архитектура](../../sv-architecture.md), ADR
[0001](../../adr/0001-separate-session-interactions-and-ui.md),
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).
Сохранять все canonical conditions/prerequisites/obligations. Все шесть rows
имеют пустой canonical prerequisite list; зависимости реализации ниже его не меняют.
Существующие production protocol/app/input owners дорабатываются в SV; второй
network engine, legacy Terms и общий refactor не нужны. Необходимые отдельные
улучшения legacy/common/SV записывать в [список улучшений](../../sv-improvements.md).

## Работа

1. Сверить существующий `sv_protocol_next` с baseline dispatch/NULL entries и
   применимыми supported packet branches. Не объявлять реально поддерживаемый
   пакет неизвестным из-за неполной локальной таблицы; неперенесённый поздний
   packet consumer фиксировать как границу, без угадывания длины и fake decode.
2. При unknown/NULL атомарно поставить `PKT_UNKNOWNPACKET(type, prev_type)` и
   `PKT_REDRAW(0)`, сохранить диагностический outcome, очистить весь текущий
   receive batch по baseline. Не пытаться сохранить sentinel после unknown:
   именно этот recovery намеренно очищает batch. При недостаточной output
   capacity не выполнять частичную recovery и не терять вход до retry.
3. `PKT_KEYPRESS` и `PKT_END` потребляют ровно один byte каждый; никаких injected
   keys, invented ack, frame counters или discard следующего валидного packet.
   Обновлять previous successful packet identity по фактической baseline ветви.
   Gameplay END нельзя механически переносить в login parser: его отдельная
   обработка END завершает `play_stream` и сохраняет собственный контекст.
4. Все control packets декодируются атомарно, в порядке потока; partial CONFIRM
   ждёт второй byte и не меняет state. Поддержать lifecycle/generation и teardown,
   не изменяя уже принятый B transport контракт и не переоткрывая B автоматически.

## Минимальная проверка и завершение

Проверить через настоящий protocol/app ingress: END/KEYPRESS + валидный sentinel
сохраняют соседний пакет и не создают replies/input; split CONFIRM не даёт partial
эффекта. Unknown после известного packet даёт точные 11 bytes recovery с верным
previous type и очищает остаток batch; output backpressure сохраняет atomicity.
Повторный step/resize и новый session generation не повторяют recovery.

Проверки — только необходимый минимум core через production public path на
[Linux](../../agents/platform-validation.md), с переиспользованием существующих
entrypoints. Fixtures подают bytes, время и ввод, но не реализуют поведение
вместо production. Новый общий harness, широкая matrix и stress/soak не входят;
расширение набора согласуется заранее по AGENTS.md. Windows/MinGW/Wine deferred.
Implementation, runtime evidence, пользовательская приёмка и canonical registry —
отдельные состояния. Core evidence не объявляет полную live-session готовность
или пользовательскую приёмку.

## Результат реализации — 2026-11-09

Существующий atomic decoder/recovery сохранён. Successful unknown recovery
теперь публикует baseline diagnostic через semantic `SvMessage`; app применяет
и доставляет сообщение до завершения recovery step. При output backpressure
нет частичной отправки, очистки input или diagnostic. Legacy/common и отдельная
login END обработка не менялись.

Минимальный `tests/sv/network-packets.c` вызывает production protocol/app:
payload-free END/KEYPRESS с соседним flags sentinel, split CONFIRM, exact
11-byte unknown recovery после known packet при output capacity 10, однократную
diagnostic publication и close/open stale generation. Clang ASan/UBSan compile
и run прошли; exact commands — `.scratch/sv-c008/network-packets-command.json`,
log — `.scratch/sv-c008/network-packets.log`.

Граница: текущий gameplay dispatcher переносит только реализованные packet
owners, не полный baseline receive table. Поздние известные packets должны
получить настоящих consumers в C011/C013/C017 и своих этапах; отсутствие
decoder не доказывает, что сервер прислал unknown packet. Этот core evidence
не сертифицирует полноценный live поток и все поздние packet branches. Длины
не угадывались, fake consumers не добавлялись, canonical JSON не менялся.

## Решение пользователя — 2026-11-09

[C008 и задачи 001–004 завершены и приняты](../../sv-stage-c008-acceptance.md)
в текущих границах core. Предыдущие ограничения полного runtime evidence
сохраняются как факты, но не являются gate приёмки C008. Оставшаяся работа
обязательна следующим owners:

- [C011-002](../c011/SV-C011-002-network-control-preparation.md): подготовка control owner/options и handoff contract.
- [C013-001](../c013/SV-C013-001-network-control-live-map.md): реальный map ingress и control-driven submission.
- [C014-003](../c014/SV-C014-003-network-control-lifecycle.md): actual handoff/lifecycle и deferred incoming caller queue.
- [C053-001](../c053/SV-C053-001-preference-runtime.md): настоящий macro WAIT/XWAIT confirmation consumer.

Их доработки и проверки не блокируют и не переоткрывают C008 автоматически;
evidence backlink служит трассировке. Полный live-runtime PASS не выдан, canonical
allocation/obligations/statuses не изменены. Плановые критерии выше читаются
с учётом этого более позднего явного решения пользователя.

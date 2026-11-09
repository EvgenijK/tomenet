# SV-C008-003 — упорядоченное подтверждение команды и owner lifecycle

Статус: accepted пользователем 2026-11-09 в текущих границах Linux core.
Labels: enhancement
Owned IDs: `capability.network.confirm`.
Зависимости реализации: 001; существующие app/input confirmation APIs; настоящий macro consumer принадлежит C053.

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

1. Сверить и при необходимости доработать текущий decode `PKT_CONFIRM` + один
   unsigned command byte → app → зарегистрированный confirmation owner.
   Частичный packet ничего не публикует; порядок complete confirmations
   сохраняется независимо от redraw и количества inputs за step.
2. Использовать существующие `sv_app_begin_confirmation`,
   `sv_app_take_confirmation`, `sv_app_end_confirmation` и input owner queue.
   Owner регистрируется до ожидания; чужой/устаревший sequence или generation
   не забирает confirmations. Не выдумывать reply/ack серверу.
3. Проверить overflow/backpressure, завершение ожидания и session teardown;
   повторный frame/resize не повторяет delivery. Контракт конкретных macro
   wait/xwait markers сверить с baseline и отдать настоящему owner C053.
4. Зафиксировать integration obligation C053: реальный waiting macro должен
   получить нужный command byte в правильном порядке и завершить свой wait.
   C008 canonical allocation `network.confirm` остаётся прежним; owner API check
   не сертифицирует полный macro outcome. Если actual consumer ещё отсутствует,
   полный result/lifecycle evidence остаётся gap, а не автоматически PASS или
   перенесённой canonical обязанностью. Core принят отдельным явным решением
   пользователя от 2026-11-09; late consumer evidence обязателен C053 без gate C008.

## Минимальная проверка и завершение

Через production app ingress зарегистрировать owner, подать split CONFIRM и
несколько соседних confirmations/sentinel; получить ровно исходные bytes по
порядку, без partial delivery и replay. Проверить нет owner, wrong owner,
end/re-register и close/open generation, bounded overflow. Это core evidence;
actual C053 macro wait/xwait flow обозначить pending до настоящего consumer.

Проверки — только необходимый минимум core через production public path на
[Linux](../../agents/platform-validation.md), с переиспользованием существующих
entrypoints. Fixtures подают bytes, время и ввод, но не реализуют поведение
вместо production. Новый общий harness, широкая matrix и stress/soak не входят;
расширение набора согласуется заранее по AGENTS.md. Windows/MinGW/Wine deferred.
Implementation, runtime evidence, пользовательская приёмка и canonical registry —
отдельные состояния. Этот план не предполагает автоматической приёмки.

## Результат реализации — 2026-11-09

Сверены existing `protocol/protocol.c`, app confirmation APIs и
`input/input.c`: production decode, unsigned byte delivery, bounded FIFO,
waiting-owner sequence и session-generation guards уже реализуют core контракт.
Дополнительная production реализация не потребовалась. В existing
`tests/sv/contact-control.c` добавлены два компактных сценария через настоящий
app ingress: split CONFIRM с bytes 255/0, unsolicited confirmation до wait,
busy/wrong owner, end/re-register с очисткой непрочитанных событий, отсутствие
replay при step/frame, session close/open и bounded overflow. 32 события
принимаются, 33-е явно завершает сессию с `SV_EVENT_OVERFLOW`, без silent loss.

Linux focused production check прошёл под ASan/UBSan вместе с существующими
contact-control cases. Exact compile command:
[`confirmation-command.json`](../../../.scratch/sv-c008/confirmation-command.json),
результат: [`confirmation.log`](../../../.scratch/sv-c008/confirmation.log).
Выполнение: `ASAN_OPTIONS=detect_leaks=0 .scratch/sv-c008/contact-control`.

Это evidence core delivery/ownership; настоящего consumer macro WAIT/XWAIT здесь
нет. Его подключение принадлежит [C053](../c053/README.md), а полный caller
result/lifecycle для `network.confirm` остаётся неподтверждённым. Allocation и
canonical registry не изменены; full runtime outcome не заявлен. Core принят
пользователем последующим решением ниже.

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

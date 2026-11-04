# SV-B-075 — Cumulative acceptance Stage B

Проверки платформ: [текущее правило](../../agents/platform-validation.md); Windows/MinGW/Wine отложены и не блокируют текущую приёмку.

Статус: accepted — принято пользователем 2026-10-28 вместе с этапом B.
Оставшиеся проверки сняты с текущего gate прямым решением пользователя;
они не объявлены выполненными. [Решение о приёмке](../../sv-stage-b-acceptance.md#решение-пользователя--2026-10-28).

Исторический статус до решения пользователя: Linux cumulative verification выполнена частично (2026-10-28);
gate pending — остаются применимые Linux provider/input/evidence требования.
[Текущий сводный отчёт](../../sv-stage-b-acceptance.md) фиксирует выполненное
и незакрытые обязательства; historical evidence не повышено до acceptance.

## Пользовательский результат

Подтвердить весь Stage B как полностью рабочий native startup screen flow для
подключения, аутентификации, выбора и создания персонажа через production path.

## Зависимости и граница

Зависит от [SV-B-020](SV-B-020-first-session.md) и
[SV-B-025](SV-B-025-entry-complete.md), а транзитивно от всех восьми owning
tickets. Gate не владеет capability IDs и не расширяет границу этапа.

Реальный TomeNET server, startup profile/FILE/Lua, gameplay session, map, HUD,
messages, chat, audio, settings и другие игровые surfaces проверяются в C или
последующих этапах. Controlled protocol peer является нормативным B executor.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
<!-- Нет: это cumulative acceptance gate без собственного capability ownership. -->
<!-- owned-capabilities:end -->

## Production SV проверки

1. Все 54 B IDs имеют current evidence по каждому canonical obligation;
   missing/failed/stale записи не повышают acceptance.
2. M1 и M2 проходят одним production executable без fallback: success, cancel,
   back, retry, rejection, provider failure, disconnect и stale generation.
3. Текущая приёмка: Linux software/accelerated. MinGW i686 build, Wine
   intermediate и actual Windows 10/11 software/accelerated отложены по решению
   пользователя и не блокируют gate; они не считаются выполненными.
4. Exact protocol bytes и fingerprints для source/fixture/build/peer; controlled
   peer покрывает version branches, fragmentation/chaining и deterministic faults.
5. Human native review подтверждает focus, keyboard/input method, mouse,
   resize/minimize/restore, private fields и понятные ошибки.
6. Cumulative A regressions проходят; B не заявляет real-server integration или
   первый gameplay screen и не создаёт скрытую зависимость от перенесённых тикетов.

## Definition of Done

[coverage.json](coverage.json) и `check-plan.py` valid, все implementation owners
и оба milestones приняты, evidence актуально и не содержит secrets. Любой
непроверенный текущий Linux build/runtime или version branch оставляет gate
pending. Windows/MinGW/Wine остаются deferred и не блокируют текущий gate.

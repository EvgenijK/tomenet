# SV-LOGIN-ARCH-003 — Перенести orchestration входа в глубокий модуль

**Статус:** implemented; адресные Linux checks passed, принято пользователем 2026-11-04.
**Дата:** 2026-11-04.
Labels: enhancement

**Зависимости:** [002](SV-LOGIN-ARCH-002-locality.md), interface из 001.

## Результат и scope

Перенести существующие смысловые переходы и lifetime попытки из `endpoint-run.c`
в модуль входа. Flow скрывает выбор следующего шага, восстановление после
отклонённого имени, back/cancel/retry и владение drafts; runner доставляет события
и время, вызывает flow и отрисовку. Protocol продолжает владеть wire state.
Сохранить текущие socket/login adapters и единственный production-путь.

Переносить старое orchestration, удаляя его из runner в том же изменении;
не оставлять два взаимно синхронизируемых flow. Диагностика получает сведения
из существующего пути, без новой системы evidence и без изменений CLI.

## Критерии завершения

- [x] Смена шага и смысл восстановления имени не вычисляются независимо runner и flow.
- [x] Back/cancel не теряют предусмотренные drafts; retry создаёт правильное
      generation, устаревшие результаты не меняют новое соединение.
- [x] Existing selection/create requests, подтверждения и порядок отправки не изменены.
- [x] Сохранены main-thread ownership, сеть/UI обслуживаются во время ожидания ввода.
- [x] Результат flow передаётся текущему consumer; будущий live handoff остаётся
      документированным контрактом, не заявлен реализованным.

## Минимальная проверка

Linux SV build и адресные существующие проверки production runner/flow из 001:
выбор/создание персонажа, back/cancel и retry/name recovery/generation.
Кандидаты для выбора — `tests/sv/first-session.c`,
`tests/sv/name-rejection-recovery.c`, `tests/sv/birth-single-submission.c`;
выбрать только режимы, проверяющие изменённый core, без автоматического запуска
всех вариантов. Если необходим новый core check, заранее согласовать расширение;
проверка вызывает тот же production interface, а не копию переходов в tests.

## Вне scope

Изменения behavior baseline, UI redesign, новый transport/event bus, CLI,
packaging, real live runtime/handoff и архитектура legacy/server/common.

## Реализация — 2026-11-04

`login/flow.[ch]` содержит перенесённые pump/login orchestration, переходы после
interaction results, name recovery, generation и teardown. `SvLoginAttempt`
внутри private `flow-internal.h` владеет connection/login/pregame/birth/wire/save;
flow хранит recovery draft между попытками. `login/native-run.c` содержит SDL
loop и вызывает flow для poll/interaction results/завершения попытки.

Native adapters имеют private synchronous доступ к принадлежащим flow
interactions; renderer его не получает. Публичная flow view формируется самим flow и используется native runner;
это не требует второй state machine. Protocol и реальная live передача
владения не меняются. Linux build, адресные проверки создания/восстановления
имени и двухосное review прошли; результаты и ограничения — в 005.

Итоговая проверка и ограничения: [005 — интеграция](SV-LOGIN-ARCH-005-integration.md).

Приёмка: пользователь принял правки 2026-11-04 и поручил commit/push.

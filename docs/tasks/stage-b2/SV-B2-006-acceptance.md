# SV-B2-006 — Интеграция и visual/input приёмка B2

Статус: planned, 2026-10-28. Реализация не начата.
Зависимости: SV-B2-002, SV-B2-003, SV-B2-004, SV-B2-005.
Существующие semantic consumers: Все B2 presentation consumers; ноль новых IDs.
Новых capability IDs: 0; B ownership и protocol semantics сохраняются.

Общий контракт: [B2 spec](../../sv-stage-b2-spec.md),
[reference и screen mapping](reference.md), [DAG](README.md).
Linux обязателен; Windows/MinGW/Wine deferred.

## Пользовательский результат

Один production executable проходит оба startup routes с дизайном по reference;
визуальные отличия и actual input результат явно представлены для принятия B2.

## Работы

- Собрать reference/native side-by-side screen/state evidence по reference.md:
  shell/server/manual/account/overview/name/birth/children/MOTD/wait/error/handoff.
  Same logical viewport и сопоставимые synthetic display data; reference и build
  fingerprints, environment/fonts/backend/resources записаны в Markdown report.
- Проверить композицию/пропорции/palette/type/spacing/art/animation, readable text,
  hover/selected/focus/disabled distinctions и отсутствие overlap/clipping.
- Два коротких production controlled-peer routes: existing character и account/new
  character creation; распределить keyboard/mouse, edit/focus, cancel/back,
  scroll/resize и handoff suppression. Separate preview scene не итоговый executor.
- Linux software и OpenGL: основной route и representative scaled hit check для
  каждого backend. Не повторять весь state×backend×scale product.
- Размеры/scale bounded: все families при одном основном размере, minimum
  1024×768; representative affected layouts при 1920×1080 100%, 3840×2160 output
  200%, fractional 125%/150%. Virtual geometry честно маркировать; physical display
  timing и реальный OS IME из неё не следуют.
- Existing checks выбрать по фактическому diff; новые automated suites/harness
  не предписаны. Минимально необходимое расширение core tests согласовать заранее.
- Свести deviations и итог visual review в `docs/sv-stage-b2-acceptance.md`.
  Приёмка отражает реальный внешний вид; исторический B waiver не переписывается.

## Production seam и ограничения

Production `src/tomenet-sv`, existing peer/fixture seams, обычные local captures
и Markdown acceptance report. Новый runtime/test framework и canonical JSON
evidence/schema migration вне scope.

## Проверки и Definition of Done

Все reference mapping rows имеют результат и owner; существенные неперенесённые
элементы не скрыты как optional future work. Visual результат представлен и
решение о B2 recorded. B остаётся accepted; B2 не заявляет real server/gameplay,
не возобновляет снятые проверки B и не меняет canonical allocation.

Общие правила: минимальные SV-local changes; никакой отдельной реализации
поведения только для тестов. Дополнительные improvements фиксируются отдельно
в `docs/sv-improvements.md`, а не включаются автоматически в эту задачу.

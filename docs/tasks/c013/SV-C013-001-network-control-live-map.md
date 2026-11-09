# SV-C013-001 — network controls на настоящем live map ingress

Статус: planned, 2026-11-09.
Labels: enhancement, ready-for-agent
Owned IDs: 0 новых IDs; integration obligation C008 внутри live map C013.
Зависимости реализации: C007 rendering, C008 controls, C011 preparation contract;
actual connection/handoff связывается с C014 без второго session runtime.

## Основа и production путь

[C008 принят](../../sv-stage-c008-acceptance.md) в границах core. Работа
обязательна C013, не блокирует и не переоткрывает C008 автоматически.
[Evidence](../../sv-c008-evidence.md), [source contract](../c008/source-contract.md),
[CONTEXT](../../../CONTEXT.md), ADR
[0002](../../adr/0002-main-thread-state-ownership.md),
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md),
[0005](../../adr/0005-session-scoped-work.md),
[0006](../../adr/0006-errors-and-overload.md).

Production ingress обновляет `session/map.[ch]` и другие session owners;
`ui/map-renderer.[ch]` получает semantic state. Submission использует
`SvControlSink`/`ui/native-controls.[ch]`, не terminal buffers или F6 projection.

## Работа и минимальная Linux проверка

1. Подключить реальные применимые map decoders к presentation state и renderer.
   Проверить map update → PAUSE/FLUSH → adjacent update: правильное промежуточное
   состояние действительно submitted до следующего эффекта, без coalescing.
2. На том же production live consumer проверить свежий pause ack, продолжение
   network/UI, separate ack повторных PAUSE и отсутствие gesture-tail leakage;
   проверить flush submission независимо от disable и pacing/reset thin budget.
3. Проверить применимые END/KEYPRESS + соседний valid map packet без потери или
   injected input. Unknown/NULL recovery даёт exact reply/redraw, diagnostic и
   нормативный discard batch; partial packet не создаёт partial effects.
   Известный baseline packet с неперенесённым decoder не считать настоящим
   unknown outcome; отсутствующие поздние consumers указать отдельно.
4. Коротко проверить resize/focus/repaint на том же реальном map/control path
   без replay и с сохранением packet order. Не считать synthetic shell smoke
   доказательством live map. Lifecycle teardown — C014, macro waits — C053.

Готовность: фактическое ingress→state→surface/control evidence C013 с точными
bytes и branch limits, backlink к применимым `obligation.network.*` C008.
Canonical allocation остаётся прежним. Минимум production-core checks на Linux;
расширение набора согласовать по AGENTS, Windows/MinGW/Wine deferred. Evidence
не становится обратным gate завершённого C008.

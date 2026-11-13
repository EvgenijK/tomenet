# SV-C007-004 — палитра, освещение и цветовая анимация

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Labels: enhancement
Owned IDs: `capability.rendering.palette`, `capability.rendering.animation`, `capability.rendering.lighting`.
Зависимости реализации: Semantic state/composition 003; resources/cache 001; clock/lifecycle 006.

## Основа и границы

Нормативны [спецификация C007](../../sv-stage-c007-spec.md),
[source contract](source-contract.md), [точный snapshot](coverage.json),
[CONTEXT](../../../CONTEXT.md), [архитектура](../../sv-architecture.md),
[settings policy](../../capabilities/settings-policy.md), ADR 0002–0006.
Сохраняются все conditions/prerequisites/obligations принадлежащих IDs;
таблица задачи не сужает canonical контракт. Legacy задаёт игровую и протокольную
семантику; canonical raster pipeline не требует pixel-perfect legacy output.
Реализация преимущественно SV-local; существующие C003/C004/C006 owners
переиспользуются. Изменения legacy/общих файлов — необходимый минимум;
отдельные улучшения фиксируются в `docs/sv-improvements.md`.

## Работа

1. Подключить production palette packet adapter к semantic owner: пять
   исходных bytes/index/R/G/B, применимые gates, partial-frame atomicity и
   порядок complete updates; renderer wire не читает. Сохранить canonical
   palette/color roles, допустимые custom entries,
   monochrome и lighterDarkBlue. Runtime tint/recolour применяются к нейтральным
   prepared assets без полного resource reload на каждом tick.
2. Перенести применимые animated colour categories и timing/scopes, включая
   no_flicker, бывшие subterm/misc scopes в соответствующих semantic surfaces.
   Отсутствие legacy terminal topology не отменяет различимые animation rules.
3. Torch/wall/floor/shading остаются отдельной игровой семантикой. TERM_SRCLITE
   сохраняет цвета и timing, но рассчитывается на полную normal/big geometry:
   нижняя половина big map не повторяет верхний legacy pattern.
4. Время и RNG передаются production owner явно; redraw не продвигает случайность
   или clock повторно. Pixel-identical flicker/particles не требуется.
   Значимые start/stop/reactions сохраняют порядок, UI/network продолжают работу.
5. Resource/palette/session generation инвалидируют только зависимые данные;
   overlay/focus/resize не сбрасывают animation state и не воспроизводят эффекты.
   Поздние C046/C047 владеют своими option UI/actions и full caller flows;
   C007 реализует renderer policy и явное применение применимых значений.

## Минимальная проверка и завершение

Один компактный production case с controlled time/RNG различает palette tint,
animated category и disabled scope, torch/shading и TERM_SRCLITE в нижней половине
66×44. Проверить update под overlay и repaint без повторного tick/effect.
Зафиксировать semantic colour roles, timing/coordinates и renderer output;
успех одной animated категории не принимает остальные branches.

Проверки — только необходимый минимум core через production public path на Linux;
fixtures подают данные/время/RNG/ресурсы, а не реализуют поведение renderer.
Переиспользовать cases соседних задач и existing core/peer entrypoints.
Новый общий harness, stress/soak и широкую matrix не вводить; расширение набора
согласуется по AGENTS.md. Windows/MinGW/Wine deferred по
[правилу платформ](../../agents/platform-validation.md). Непроверенные Linux
backends/branches и поздние flows отмечать явно, без фиктивного PASS.
Implementation, пользовательская приёмка и canonical runtime registration —
разные состояния; этот ticket их не объявляет.


## Результат реализации — 2026-11-09

Production Linux core реализован агентами и подключён к native resource consumer.
[Фактические callers, команды, результаты и ограничения](../../sv-c007-evidence.md)
содержат отдельную disposition каждого obligation. Этот результат не принимает
полный C007, C013 live-map flow, hardware backend или непроверенные branches;
canonical registration остаётся отдельным workflow.


## Приёмка пользователем — 2026-11-09

Пользователь явно подтвердил: «этот этап принял».
[C007 и задачи 001–007 приняты](../../sv-stage-c007-acceptance.md) в представленных границах Linux core,
с сохранением описанных ограничений. Исторические формулировки pending выше
относятся к моменту подготовки плана/evidence. Live карта остаётся C013;
непроведённые проверки не получают PASS, canonical registry не изменён.

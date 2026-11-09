# SV-C007-006 — временные эффекты, погода и raw pictures

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Labels: enhancement
Owned IDs: `capability.rendering.effects-restore`, `capability.rendering.raw-picture`, `capability.rendering.weather`.
Зависимости реализации: Semantic composition 003, geometry 005, graphics mappings 001/002, palette/time 004.

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

1. Представлять transient effects/weather независимо от terrain/foreground.
   После завершения слоя видны последние applied cell data и оставшиеся эффекты;
   stale raster snapshot не восстанавливать. Изменение карты под overlay сохраняется.
2. Подготовить authored raw-picture rectangle через C006 preparation и 002
   definitions: anchors/extents/clipping, special-canvas transform и выбранный
   graphics filter. Обычные tile masks не навязывать; TEST_RAWPICT/applicable
   build gates и ASCII paths сохраняются.
3. Реализовать production weather protocol decode→model→renderer path:
   тип/параметры, ветер, start/stop, volume intent, independent overlay и
   no2mask_weather. Сохранить source-defined version branches/signedness/control
   bytes, split/chained packets; partial frame ничего не применяет, включая
   незавершённый cloud loop. Weather-colouring adapter сохраняет разницу
   rain/snow и sand по gate 4.7.4.6; точные layouts пересверить с source contract.
4. Clock/RNG и timing — production параметры, не поведение тестового renderer.
   Resize/recreate/repaint не повторяют start sound/one-shot reactions и не
   замораживают packet application. Generation ограждает новую session от старых
   particles/callbacks/commands. Звуковые намерения имеют явного владельца;
   full audio/device/pack consumer C055 не переносится в C007.
5. Rendering-specific protocol ingress входит сюда без подключения legacy Term.
   Общий session handoff/world.read-map/target gameplay остаётся C011/C013/C014/C018.
   Actual PRF message/action delivery остаётся C016/C053; renderer их не исполняет.

## Минимальная проверка и завершение

Минимальный production case: обновление terrain под эффектом и корректное
restore при remaining layer, raw-picture anchor/clipping, weather start/change/
stop с controlled clock/RNG. Existing peer проверяет применимые version layouts,
split/chained weather и отсутствие partial commit/replay после resize/session reset.
Сохранять expected/actual wire fields, semantic layers, timing и generations;
один screenshot или только вызов serializer не доказывает weather wire obligation.

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

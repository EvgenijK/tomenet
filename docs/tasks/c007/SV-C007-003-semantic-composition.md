# SV-C007-003 — семантические слои и базовая композиция

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Labels: enhancement
Owned IDs: `capability.rendering.glyph-identity`, `capability.rendering.terrain-stack`, `capability.rendering.wipe`, `capability.rendering.solid-wall`.
Зависимости реализации: C006 font/profile/preparation API; resource interfaces 001/002, geometry interface 005.

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

1. Ввести или расширить production session presentation state для background,
   terrain и foreground с независимыми identities/attrs/subsets, revision и
   session generation. Renderer получает semantic view; не читает `Term`,
   legacy globals или wire packets. Не создавать virtual terminal.
2. Принимать version-aware decoded changes через явный production ingress.
   Семантический apply и renderer пригодны будущему C013: локальный fixture не
   становится отдельным map engine. Полный map packet ingress, movement/target/live session flow
   C013/C014 здесь не принимается и не переносится целиком в C007. Semantic
   ingress принимает decoded changes; protocol owner сохраняет partial-frame
   atomicity и порядок complete changes. Packet-derived map evidence остаётся
   pending до C013, прямой semantic core call его не подменяет.
3. Разрешать numeric glyph ID через существующий C006 map font/profile owner,
   font default и видимый one-cell fallback. Не передавать raw high/control
   bytes в TTF API, не угадывать PCF origin и не менять outgoing charset.
4. Составлять background → terrain → foreground → временные слои/indicators;
   удаление foreground открывает последнее terrain, включая обновление под ним.
   Поддержать font-only и mixed tiles/glyphs через 001 без второй resource loader.
5. Wipe очищает ровно semantic rectangle, включая применимые cursor/content
   roles; соседние клетки сохраняются. Repaint восстанавливает актуальное
   состояние, а не saved bitmap. Solid walls/equal foreground-background blocks
   рисуются по смысловой роли, включая FONT_MAP_SOLID_X11.
6. Session reset исключает старые cells/revisions; resize/repaint не повторяют
   входные изменения и side effects. Caches производны от state и ресурсов.

## Минимальная проверка и завершение

Минимальный различимый production case: numeric/default/missing glyph,
terrain update под foreground, удаление foreground, bounded wipe и solid block;
тот же state проходит настоящий renderer, затем repaint/resize/session reset.
Проверить mixed content вместе с 001, не дублируя font suite C006.
Результат — наблюдаемая композиция и expected/actual IDs/layers/regions/generations;
наполнение test-only структуры без production consumer недостаточно.

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

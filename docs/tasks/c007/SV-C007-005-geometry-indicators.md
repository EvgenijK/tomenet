# SV-C007-005 — геометрия карты, DPI и индикаторы

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Labels: enhancement
Owned IDs: `capability.rendering.cursor`, `capability.rendering.resize`.
Зависимости реализации: C006 map font metrics; semantic state 003; resources/cache 001.

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

1. Draw и hit testing используют одну logical geometry. OS/display scale
   переводит logical UI units в output pixels; UI scale и map fit сохраняют
   независимые роли. Вписывать полную centered grid 66×22/66×44 с целочисленной
   final-cell geometry и пропорциями клеток. Crop, map zoom key и постоянный
   whole-window framebuffer scaling не входят в контракт.
2. Resize/DPI/layout/font change пересчитывают map area, final size, placement,
   clipping и caches. Пока готовится новый набор, допустим краткий показ старого
   scaled результата по policy; semantic updates и hit testing не останавливать.
   При завершении generation показывать prepared assets 1:1 без второго scaling.
3. Рисовать self/target/cell indicators отдельным слоем общей сетки. Сохранять
   underlying layers, корректное исчезновение/перемещение/focus и отсутствие
   накопления alpha. Keyboard/mouse targeting actions принадлежат своим этапам;
   renderer принимает semantic indicator state.
4. Источник конкретного рисунка — подтверждённый основной HTML UX prototype.
   Отклонённые варианты A/B/C из decision ticket 31 не использовать как approved.
   Делегирование HTML уже принято; нового выбора формы сейчас не требуется.
   Наблюдаемые hover/throw-target anchors — в source contract. Если для нужного
   состояния reference отсутствует, явно записать остаток и уточнить только
   недостающую часть; geometry/lifecycle работу продолжать.
5. C013 получает готовую geometry/composition API; полный игровой layout/HUD
   соседних этапов не принимается по renderer fixture. Renderer recreation
   сохраняет semantic state и не повторяет PRF load или server notifications.

## Минимальная проверка и завершение

Минимальный production case покрывает normal/big grid, нецелый DPI, resize и
cell hit на границе; target move/remove поверх меняющегося terrain сохраняет
актуальный underlying state. Cursor appearance evidence связано с фактическим
approved reference, не с legacy pixel oracle. Зафиксировать map bounds,
final-cell pixels, transform, hit coordinates и generation.

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

# SV-C007-007 — минимальное Linux evidence и сверка этапа

Статус: accepted пользователем 2026-11-09 в представленных границах Linux core.
Labels: enhancement
Owned IDs: 0 IDs; сводка всего этапа без повторного allocation.
Зависимости реализации: 001–006; полный snapshot 15 outcomes/45 obligations.

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

1. Собрать единый отчёт `docs/sv-c007-evidence.md` с caller/revision, командами,
   resource/profile/PRF sources, source/cache/session generations, enabled/disabled
   и requested/effective состояниями. Для всех 45 obligations — отдельная
   disposition с конкретным evidence или явным ограничением.
2. Переиспользовать минимальные production cases задач 001–006; build и проверки
   actual protocol owners отделить от renderer observations. Никакого test-only
   поведения, нового общего harness или автоматического запуска старых matrix.
3. Проверить generation/lifecycle короткими различимыми cases: resource failure
   сохраняет stack, resize/recreation не повторяют effects/PRF/notifications,
   stale session не меняет новое соединение. Сохранить применимые submission
   budgets 20/50/200 ms; stress/soak и новый численный memory ceiling не добавлять.
4. Linux software/accelerated conditions сохраняются; реально непроверенный
   backend указывается явно. Пользовательское решение о границах приёмки нельзя
   подменять source review или успехом software-only fixture.
5. Возврат FI-02 и preference trace в C003/C004 — по 001/002; прежняя приёмка
   не переоткрывается автоматически. C006 TTF/profile/backend limits не скрывать
   и не объявлять исправленными без собственной проверки. Canonical files не менять.
6. Сверить 15 unique owners и 45 obligations с canonical rows/hashes, все локальные
   ссылки, scope соседних stages и открытые вопросы. Подготовить reviewable итог;
   принятие пользователем и runtime registration — отдельные действия.

## Минимальная проверка и завершение

Завершение: собственные outcomes имеют production результат, evidence честно
разделяет проверенное и pending; нет потери allocation или условий. Открытые
UX/backend/late-flow ограничения перечислены явно. Сам этот ticket не владеет
capability IDs и не принимает C007 по успеху одной интеграционной задачи.

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

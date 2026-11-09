# SV-C006-006 — discovery и validation bundled/custom resource corpus

Статус: accepted, 2026-11-09; принят пользователем в согласованных границах.
Labels: enhancement
Stage outcome: `capability.fonts.asset-validation`.
Зависимости: production PCF/TTF/profile owners C006-003, selection/lifecycle C006-004,
[preparation C006-005](SV-C006-005-resource-preparation.md),
[map consumer C006-001](SV-C006-001-map-resource-integration.md) и
[preferences C006-002](SV-C006-002-map-preferences-integration.md).

## Контракт и работа

Нормативны [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[font asset inventory](../../capabilities/inventories/encoding-font-assets.md),
[reconciliation](../../capabilities/settings-reconciliation.md),
[canonical coverage](../../capabilities/native-coverage.json), ADR
[0003](../../adr/0003-explicit-module-connections.md),
[0004](../../adr/0004-revision-based-ui-views.md) и
[0006](../../adr/0006-errors-and-overload.md).

1. Через production file/resource owners обнаруживать и проверять каждый
   поставляемый font, tileset и profile: basic load, metadata, диапазоны encoded
   IDs, default glyph, совместимые profile/mapping/mask/subset associations и
   явный failure. Не заменять exhaustive basic validation несколькими ASCII
   примерами. Перечень сверить с фактическими bundled files; historical inventory
   сам по себе не доказывает текущую комплектацию или загрузку.
2. Применимый полный baseline glyph corpus должен покрываться shipped profiles;
   массовый visible replacement не доказывает соответствие. Учитывать PCF encoded
   origin/default/charmap, TTF explicit ID correspondence, metrics и one-cell
   rendering. Bundled CascadiaMono-Regular.ttf — exact v2407.24 с profile,
   copyright и OFL; custom PCF/TTF/resources также поддерживаются production путём.
3. Глубокие semantic/lifecycle проверки сохраняют representative corpus:
   `9x15.pcf`, full byte-ID `9x15tg`, mixed `16x24tg` + `16x24sv`,
   `12x24`/`8x16` с origin 1, partial-range `16x22`. Проверять numeric identities,
   правильный default/missing и границы без OOB; custom successful PCF не подменять.
   Reuse checks 001–005 для font switching, independent text-only changes,
   profile/resource failure, исчезновения/возвращения assets и recreate текущего
   scene. Полные graphics overrides/masks/subsets/weather/raw-picture gameplay
   flows продолжает C007; это не отменяет собственного basic resource validation
   и production preparation результатов C006-005.
4. Basic discovery/load/metadata validation и глубокие scenarios должны вызывать
   настоящие production loaders/preparation/consumers, без отдельной реализации
   поведения для тестов. Отдельно различать исходный source audit, текущую
   installation, Linux software/accelerated path и native execution evidence.
   Недоступная ветвь фиксируется явно без фиктивного PASS.

## Завершение и минимальная Linux проверка

Использовать минимальные существующие core checks и общий перечень assets,
переиспользуя evidence 001–005. Требования corpus сохраняются полностью; это
не поручение строить новый harness, benchmark, screenshot corpus или широкую
visual/fault matrix. Необходимое расширение набора проверок согласуется заранее
по AGENTS.md. [Текущая платформа Linux](../../agents/platform-validation.md);
Windows/MinGW/Wine отложены и не блокируют Linux приёмку.

Закрыть три canonical obligations:
`obligation.fonts.asset-validation.result`,
`obligation.fonts.asset-validation.failure-lifecycle`,
`obligation.fonts.asset-validation.allocation-checks`.
Зафиксировать revision/caller, фактический resource inventory, expected/actual
load/metadata/range/default outcomes, применимый deep corpus и reuse evidence;
обнаруженные failures остаются видимыми. Весь применимый bundled basic corpus
проверен production путём, обязательные representative scenarios подтверждены;
metadata/source counts без исполнения не закрывают задачу.

Canonical IDs/conditions/obligations/allocation и runtime statuses не меняются;
эта задача не заявляет implementation PASS или приёмку C006.

## Результат реализации — 2026-11-09

Production SV реализация и минимальные Linux core проверки выполнены.
[Фактическое evidence и ограничения](../../sv-c006-evidence.md) отделяют
software resource consumer от поздних gameplay flows и непроверенных backends.
Canonical runtime registration и пользовательская приёмка не выполнялись.

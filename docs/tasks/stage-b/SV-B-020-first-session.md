# SV-B-020 — M1: первый сквозной вход существующим персонажем

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Получить реальный login→existing character→required startup→MOTD→map/live HUD→clean disconnect в production SV до завершения рождения.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-006](SV-B-006-login.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-016](SV-B-016-hud-conditions.md), [SV-B-017](SV-B-017-hud-detail.md), [SV-B-018](SV-B-018-messages-live.md), [SV-B-019](SV-B-019-first-import.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

У этого checkpoint **нет собственных capability IDs**. Он выполняет существующий branch `capability.session.enter-game`, primary owner которого — [SV-B-025](SV-B-025-entry-complete.md). Рабочий branch нужен сейчас; полный ID остаётся pending до обоих entry paths и всех canonical prerequisites. Это не замена baseline приёма тестовым кодом.

## Production subsets и поздние integration checks

- `initial-audio`: producer [SV-B-003](SV-B-003-profile.md); полные owners [SV-B-012](SV-B-012-audio-start.md). Profile consumer003 вводит production initial device/pack configuration application и неблокирующий init/result bridge.010 использует именно этот путь для восьми retained startup CFG fields;012 расширяет events/music/weather и принимает полный device-failure ID. Bounds/default/requested/effective device/pack/subset реально доходят до initial open, stereo не путается с track count. No-device leaves session responsive;012/020 повторяют startup failures с actual executors, а не parser-only fixture.
- `startup-tiles`: producer [SV-B-014](SV-B-014-map-core.md); полные owners [SV-B-039](SV-B-039-map-effects.md). 014 реализует штатную production tile/subset composition для approved graphics1 и selected16x24sv: resource/PRF mappings, terrain/foreground placement и masks по этому режиму, palette, scaled prepared assets1:1, normal/big viewport fit.039 остаётся полным owner всех tile/subset modes/gates/effects. Fresh own CFG/defaults с доступным16x24sv в014/020 действительно рисует тайлы; проверить selected/default subset и glyph/terrain placement по baseline reference. graphics0/font-only допустим только как предусмотренный failure для отсутствующего/непригодного ресурса, не обход незавершённого renderer.039 повторяет штатный M1 и достраивает mode2/прочие subsets и source branches.
- `existing-entry`: producer [SV-B-020](SV-B-020-first-session.md); полные owners [SV-B-025](SV-B-025-entry-complete.md). 020 реализует existing-character ветвь production entry transition;025 сохраняет единственного полного владельца, добавляет completed-birth alternative и все readiness prerequisites. Real auth/FILELua/MOTD/Net_start/map/HUD→disconnect; prior birth.complete не заставляет runtime создавать персонажа.Полный ID pending до025.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-025](SV-B-025-entry-complete.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-003](SV-B-003-profile.md), [SV-B-006](SV-B-006-login.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-019](SV-B-019-first-import.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
Нет собственных IDs; это integration milestone.
<!-- owned-capabilities:end -->

## Production SV проверки

1. На реальном поддерживаемом сервере и отдельном профиле пройти цель M1 по спецификации; actual scene order, Net_start after MOTD, server replies наблюдаемы.
2. Интегрировать уже начатый session.enter-game production transition как ранний existing-character branch; ID и полная приёмка принадлежат позднему ticket entry-complete, который также доказывает birth branch. Не менять registry prerequisite birth.complete.
3. No device/missing optional resource, canceled credentials, FILE failure, incomplete packet и teardown проверяются соответствующими ранними owners. Stage B не считается завершённым, а полный font/config/import и birth scope не отменяется.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.


[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники


Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

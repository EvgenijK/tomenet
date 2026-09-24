# SV-B-075 — Приёмка B на всех обязательных платформах

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Завершить cumulative stage B с реальными server round trips, native evidence и ручным review, сохранив честные C–F limits.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-002](SV-B-002-contact.md), [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-005](SV-B-005-vault.md), [SV-B-006](SV-B-006-login.md), [SV-B-007](SV-B-007-macros.md), [SV-B-008](SV-B-008-guide.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-013](SV-B-013-fonts-load.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-015](SV-B-015-hud-core.md), [SV-B-016](SV-B-016-hud-conditions.md), [SV-B-017](SV-B-017-hud-detail.md), [SV-B-018](SV-B-018-messages-live.md), [SV-B-019](SV-B-019-first-import.md), [SV-B-020](SV-B-020-first-session.md), [SV-B-021](SV-B-021-account-manage.md), [SV-B-022](SV-B-022-character-manage.md), [SV-B-023](SV-B-023-birth-choices.md), [SV-B-024](SV-B-024-birth-dna.md), [SV-B-025](SV-B-025-entry-complete.md), [SV-B-026](SV-B-026-target.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-028](SV-B-028-movement.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-030](SV-B-030-clipboard.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md), [SV-B-033](SV-B-033-guide-tools.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md), [SV-B-036](SV-B-036-session-end.md), [SV-B-037](SV-B-037-reconnect.md), [SV-B-038](SV-B-038-font-choices.md), [SV-B-039](SV-B-039-map-effects.md), [SV-B-040](SV-B-040-audio-controls.md), [SV-B-041](SV-B-041-layout-settings.md), [SV-B-042](SV-B-042-local-settings.md), [SV-B-043](SV-B-043-screenshots-platform.md), [SV-B-044](SV-B-044-message-clones.md), [SV-B-045](SV-B-045-config-identity.md), [SV-B-046](SV-B-046-config-render.md), [SV-B-047](SV-B-047-config-audio.md), [SV-B-048](SV-B-048-options-input.md), [SV-B-049](SV-B-049-options-messages-1.md), [SV-B-050](SV-B-050-options-messages-2.md), [SV-B-051](SV-B-051-options-alerts-1.md), [SV-B-052](SV-B-052-options-alerts-2.md), [SV-B-053](SV-B-053-options-hud-1.md), [SV-B-054](SV-B-054-options-hud-2.md), [SV-B-055](SV-B-055-options-maplight-1.md), [SV-B-056](SV-B-056-options-maplight-2.md), [SV-B-057](SV-B-057-options-mapidentity-1.md), [SV-B-058](SV-B-058-options-mapidentity-2.md), [SV-B-059](SV-B-059-options-animation-1.md), [SV-B-060](SV-B-060-options-animation-2.md), [SV-B-061](SV-B-061-options-movement-1.md), [SV-B-062](SV-B-062-options-movement-2.md), [SV-B-063](SV-B-063-options-movement-3.md), [SV-B-064](SV-B-064-options-target.md), [SV-B-065](SV-B-065-options-pickup-1.md), [SV-B-066](SV-B-066-options-pickup-2.md), [SV-B-067](SV-B-067-options-pickup-3.md), [SV-B-068](SV-B-068-options-session-1.md), [SV-B-069](SV-B-069-options-session-2.md), [SV-B-070](SV-B-070-options-audio-1.md), [SV-B-071](SV-B-071-options-audio-2.md), [SV-B-072](SV-B-072-options-audio-3.md), [SV-B-073](SV-B-073-options-audio-4.md), [SV-B-074](SV-B-074-options-integration.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

У gate **нет новых capability IDs**. Он агрегирует evidence всех508 owners и регрессии A; никакое отсутствие platform evidence не превращается в accepted.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
Нет собственных IDs; это cumulative acceptance gate.
<!-- owned-capabilities:end -->

## Production SV проверки

1. Все508 B owners и A foundation regressions завершены; полный matrix Linuxsoftware/accelerated, independent MinGWi686 build+Wine и realWindows10/11software/accelerated. Ни Wine, ни valid allocation не заменяют actual Windows evidence.
2. Актуальные manifests/allocation hashes, source/fixture/resource/build closure, safe runtime fallback counters, scoped obligation records и dependency freshness; acceptance не повышается при missing/failed/stale checks.
3. Собрать stage report: exact outcomes, all failures/limits, clean disconnect/reconnect/death and concurrency checks, human native UX review; no B accepted caller reaches fallback. Реальные C–E callers остаются pending, release archive closure F.

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

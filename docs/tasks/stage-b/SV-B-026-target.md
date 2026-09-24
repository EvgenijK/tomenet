# SV-B-026 — Направление и выбор цели

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок выбирает направление/hostile/position и читает описание цели, сохраняя реальный caller.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-007](SV-B-007-macros.md), [SV-B-025](SV-B-025-entry-complete.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Поздние обязательные проверки для primary owner этого тикета: [SV-B-028](SV-B-028-movement.md), [SV-B-064](SV-B-064-options-target.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.direction.choose-direction` | Return mapped direction 1..9 to its caller; supplied command_dir bypasses prompt without reading another key. | [c-util.c:3672](../../../src/client/c-util.c#L3672)<br>[c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
| `capability.direction.use-acquired` | Minus returns code 10, plus code 11, ordinary 5 returns 5; do not conflate target-required, acquired/manual and self-capable routes. | [c-util.c:3672](../../../src/client/c-util.c#L3672)<br>[c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
| `capability.direction.choose-target` | Star opens hostile target child; successful selection returns 5, child cancellation returns no direction and caller must not send its action. | [c-util.c:3672](../../../src/client/c-util.c#L3672)<br>[c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
| `capability.direction.cancel` | Escape/invalid direction returns false, with no enclosing action packet; parent owner decides whether to retry or exit. | [c-util.c:3672](../../../src/client/c-util.c#L3672)<br>[c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
| `capability.target.select-hostile` | Initialize Send_target(0), navigate candidates with mapped direction, accept t/5 using Send_target(5). Server target info supplies result. | [c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[nclient.c:5515](../../../src/client/nclient.c#L5515)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
| `capability.target.select-position` | p toggles manual mode and sends target 128; navigation sends 128+direction and acceptance sends 133. | [c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[nclient.c:5515](../../../src/client/nclient.c#L5515)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
| `capability.target.cancel` | Escape/q or any unmapped non-command key exits false without a target-confirm or invented cancel packet; clear target prompt and return to caller. | [c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[nclient.c:5515](../../../src/client/nclient.c#L5515)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
| `capability.target.read-description` | Retain original target x/y and description, render server target updates while another request is active, and clear old session target data on disconnect. | [nclient.c:5515](../../../src/client/nclient.c#L5515)<br>[c-cmd.c:2040](../../../src/client/c-cmd.c#L2040)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-util.c:3672](../../../src/client/c-util.c#L3672) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Normal/roguelike physical input и macro routes, acquired direction/target, cancel/retry, target description versions; overlays скрываются/возвращаются по policy.
2. Interleaved map/target change и resize не выбирают stale entity; no command emitted on canceled local child unless baseline defines a reply.
3. Quantity C не требуется; успешный target primitive не принимает fire/throw/spell callers C.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.direction.choose-direction` | command_dir bypass; mapped1..9/5, -→10/+→11, * target success/cancel; каждый movement caller без packet при отказе. Точный проверяемый результат: Return mapped direction 1..9 to its caller; supplied command_dir bypasses prompt without reading another key. |
| `capability.direction.use-acquired` | command_dir bypass; mapped1..9/5, -→10/+→11, * target success/cancel; каждый movement caller без packet при отказе. Точный проверяемый результат: Minus returns code 10, plus code 11, ordinary 5 returns 5; do not conflate target-required, acquired/manual and self-capable routes. |
| `capability.direction.choose-target` | command_dir bypass; mapped1..9/5, -→10/+→11, * target success/cancel; каждый movement caller без packet при отказе. Точный проверяемый результат: Star opens hostile target child; successful selection returns 5, child cancellation returns no direction and caller must not send its action. |
| `capability.direction.cancel` | command_dir bypass; mapped1..9/5, -→10/+→11, * target success/cancel; каждый movement caller без packet при отказе. Точный проверяемый результат: Escape/invalid direction returns false, with no enclosing action packet; parent owner decides whether to retry or exit. |
| `capability.target.select-hostile` | init0, directional sends, toggle128, confirm5/133; Escape/q/unmapped false без выдуманного cancel packet; interleaved description/reset. Точный проверяемый результат: Initialize Send_target(0), navigate candidates with mapped direction, accept t/5 using Send_target(5). Server target info supplies result. |
| `capability.target.select-position` | init0, directional sends, toggle128, confirm5/133; Escape/q/unmapped false без выдуманного cancel packet; interleaved description/reset. Точный проверяемый результат: p toggles manual mode and sends target 128; navigation sends 128+direction and acceptance sends 133. |
| `capability.target.cancel` | init0, directional sends, toggle128, confirm5/133; Escape/q/unmapped false без выдуманного cancel packet; interleaved description/reset. Точный проверяемый результат: Escape/q or any unmapped non-command key exits false without a target-confirm or invented cancel packet; clear target prompt and return to caller. |
| `capability.target.read-description` | init0, directional sends, toggle128, confirm5/133; Escape/q/unmapped false без выдуманного cancel packet; interleaved description/reset. Точный проверяемый результат: Retain original target x/y and description, render server target updates while another request is active, and clear old session target data on disconnect. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.direction.choose-direction`, `capability.direction.use-acquired`, `capability.direction.choose-target`, `capability.direction.cancel`, `capability.target.select-hostile`, `capability.target.select-position`, `capability.target.cancel`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.target.read-description`: versions — >=4.9.0.1 uses %S MSG_LEN; older %s MAX_CHARS.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

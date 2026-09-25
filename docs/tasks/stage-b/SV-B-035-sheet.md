# SV-B-035 — Character sheet и ранние knowledge данные

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок читает и переключает character pages/history/equipment flags, открывает Guide и закрывает sheet.

## Зависимости и граница

Завершить необходимые production части [SV-B-008](SV-B-008-guide.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-031](SV-B-031-chat.md), [SV-B-033](SV-B-033-guide-tools.md), [SV-B-034](SV-B-034-exports.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `map-capture`: producer [SV-B-029](SV-B-029-map-explore.md); полные owners [SV-B-034](SV-B-034-exports.md), [SV-B-043](SV-B-043-screenshots-platform.md). 029 вводит реальный native composed-frame capture child и production encoder/file-owner/collision path для доступных map bindings: PNG с SDL3_image, BMP без него, no silent overwrite, requested filename semantics и no notification inside saved frame.035/036 используют тот же production child из sheet/final scene.034 и043 сохраняют полных owners file-collision/capture/platform outcomes. 029 проверяет real image pixels/extension и success/cancel/write/encode/collision failures с возвратом map caller.034 расширяет collision tests на все exports,035/036 добавляют реальных callers,043 проверяет все aliases/server triggers и platform configurations и повторяет029/035/036. Никакого test-only screenshot callback.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-036](SV-B-036-session-end.md), [SV-B-043](SV-B-043-screenshots-platform.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-008](SV-B-008-guide.md), [SV-B-031](SV-B-031-chat.md), [SV-B-034](SV-B-034-exports.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.information.read-sheet` | Read current character abilities, identity and stat sheet from decoded state, preserving glyphs, colours and missing/versioned values. | [c-cmd.c:382](../../../src/client/c-cmd.c#L382)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2310](../../../src/client/c-cmd.c#L2310) |
| `capability.information.read-history` | Read server history lines at their original indices; updates replace only the addressed line and session reset clears old character content. | [c-cmd.c:382](../../../src/client/c-cmd.c#L382)<br>[nclient.c:364](../../../src/client/nclient.c#L364)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.information.read-equipment-flags` | Read equipment resistance/ability flag matrix and its horizontal/vertical view without changing equipment. | [c-cmd.c:382](../../../src/client/c-cmd.c#L382)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
| `capability.information.navigate-sheet` | h/H cycles the three character pages; 2/8 selects help topics (not pages), 4/6 changes selection, v toggles equipment orientation only on page 2. | [c-cmd.c:382](../../../src/client/c-cmd.c#L382)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2310](../../../src/client/c-cmd.c#L2310) |
| `capability.information.close-sheet` | q/Q/Escape/C closes the sheet and restores its exact live or final-review parent without issuing a gameplay command. | [c-cmd.c:382](../../../src/client/c-cmd.c#L382)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Live/final-review data coherent, versions/absent fields, exact paging/current-state redraw и context topic.
2. Actual character export/Guide/chat/capture children возвращают правильный sheet owner; repeat redraw не повторяет send/dump.
3. Полная D lore/context-help union и glyph lookup не приписываются B unique-records.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.information.read-sheet` | Live и final-review child; coherent data/pages/help/dump/capture/chat и точное восстановление parent; версия/missing values, без повторных sends. Точный проверяемый результат: Read current character abilities, identity and stat sheet from decoded state, preserving glyphs, colours and missing/versioned values. B caller-specific Guide topics from each birth step, final/live sheet and server opening tested through existing B caller/result obligations; full skills/lore caller-union remains D. For race/trait/class/stats/mode/body and sheet context, topic search matches exact baseline argument; close/cancel restores same caller/pending selection. Missing Guide/search failure/resize/network update preserve early owner; C skill and D lore callers require separate full-context regressions. Full deferred outcomes: capability.guide.context-help. |
| `capability.information.read-history` | Live и final-review child; coherent data/pages/help/dump/capture/chat и точное восстановление parent; версия/missing values, без повторных sends. Точный проверяемый результат: Read server history lines at their original indices; updates replace only the addressed line and session reset clears old character content. |
| `capability.information.read-equipment-flags` | Live и final-review child; coherent data/pages/help/dump/capture/chat и точное восстановление parent; версия/missing values, без повторных sends. Точный проверяемый результат: Read equipment resistance/ability flag matrix and its horizontal/vertical view without changing equipment. |
| `capability.information.navigate-sheet` | Live и final-review child; coherent data/pages/help/dump/capture/chat и точное восстановление parent; версия/missing values, без повторных sends. Точный проверяемый результат: h/H cycles the three character pages; 2/8 selects help topics (not pages), 4/6 changes selection, v toggles equipment orientation only on page 2. B caller-specific Guide topics from each birth step, final/live sheet and server opening tested through existing B caller/result obligations; full skills/lore caller-union remains D. For race/trait/class/stats/mode/body and sheet context, topic search matches exact baseline argument; close/cancel restores same caller/pending selection. Missing Guide/search failure/resize/network update preserve early owner; C skill and D lore callers require separate full-context regressions. Full deferred outcomes: capability.guide.context-help. |
| `capability.information.close-sheet` | Live и final-review child; coherent data/pages/help/dump/capture/chat и точное восстановление parent; версия/missing values, без повторных sends. Точный проверяемый результат: q/Q/Escape/C closes the sheet and restores its exact live or final-review parent without issuing a gameplay command. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.information.read-sheet`, `capability.information.read-history`, `capability.information.read-equipment-flags`, `capability.information.navigate-sheet`, `capability.information.close-sheet`: versions — All supported versions at the pinned owner and serializer; exact incoming wire branches mapped in information-reconciliation.md.; builds — Linux amd64 / Windows i686 SV; normal and roguelike keys. Preserve per-owner REGEX_SEARCH, ENABLE_GO_GAME, ENABLE_ITEM_ORDER, USE_GRAPHICS, TEST_RAWPICT, ANIM_SLOT_SPINALL, USE_SOUND_2010 and admin/DM/runtime privilege gates where applicable; disabled branches do not certify enabled behavior..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

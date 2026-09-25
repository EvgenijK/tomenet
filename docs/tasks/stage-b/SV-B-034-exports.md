# SV-B-034 — Экспорт персонажа/сообщений и notes

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок сохраняет character/message content и notes в правильные shared owners без незаметной перезаписи.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-032](SV-B-032-message-recall.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `map-capture`: producer [SV-B-029](SV-B-029-map-explore.md); полные owners [SV-B-034](SV-B-034-exports.md), [SV-B-043](SV-B-043-screenshots-platform.md). 029 вводит реальный native composed-frame capture child и production encoder/file-owner/collision path для доступных map bindings: PNG с SDL3_image, BMP без него, no silent overwrite, requested filename semantics и no notification inside saved frame.035/036 используют тот же production child из sheet/final scene.034 и043 сохраняют полных owners file-collision/capture/platform outcomes. 029 проверяет real image pixels/extension и success/cancel/write/encode/collision failures с возвратом map caller.034 расширяет collision tests на все exports,035/036 добавляют реальных callers,043 проверяет все aliases/server triggers и platform configurations и повторяет029/035/036. Никакого test-only screenshot callback.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-035](SV-B-035-sheet.md), [SV-B-036](SV-B-036-session-end.md), [SV-B-043](SV-B-043-screenshots-platform.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-032](SV-B-032-message-recall.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.exports.character` | Export character sheet text from live or final-review caller via f/F; retain baseline content/glyph conversion, owned destination and explicit replace/cancel errors. | [c-files.c:2297](../../../src/client/c-files.c#L2297)<br>[c-util.c:137](../../../src/client/c-util.c#L137)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[c-files.c:2297](../../../src/client/c-files.c#L2297) |
| `capability.exports.messages` | Explicit/exit-selected chat or all-message export retains baseline occurrence order/content and shared U/user destination; no implicit recorder. | [c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[c-util.c:137](../../../src/client/c-util.c#L137)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[c-files.c:2297](../../../src/client/c-files.c#L2297) |
| `capability.files.notes-append` | Automatically append received private notes to shared U/user/notes-account.txt; preserve original text/order and report disk failure. | [c-files.c:424](../../../src/client/c-files.c#L424)<br>[nclient.c:366](../../../src/client/nclient.c#L366)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
| `capability.files.export-collision` | Generated export filenames choose free suffix; explicit existing destination requires replace/cancel, never silent overwrite. | [c-files.c:424](../../../src/client/c-files.c#L424)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Actual raw names/source bytes, final character dump includes kill counts/names only, messages preserve correct selected occurrence content.
2. Collision policy, explicit destination, read/write/rename failure, cancellation и no unrelated/shared secret writes.
3. Redraw/reopen/teardown не дублируют append/export; delayed completion освобождает payload без command в новой session.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.exports.character` | Tomb/live f/F dump contains final identity/stats/unique kills with correct glyph conversion, exact filename replace/cancel Disk fault/invalid byte filename preserves previous file and caller; no secret buffers leak. |
| `capability.exports.messages` | Explicit/exit-selected all/chat export preserves duplicate occurrences/order, destination shared and no implicit recorder Disk fault/invalid byte filename preserves previous file and caller; no secret buffers leak. |
| `capability.files.notes-append` | Два одинаковых notes/continuation packets записаны по порядку один раз; non-note сбрасывает chain; disk denied и secret exclusion. |
| `capability.files.export-collision` | Generated name находит free suffix; explicit existing file replace/cancel; failed write сохраняет previous destination. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.exports.character`, `capability.exports.messages`, `capability.files.notes-append`, `capability.files.export-collision`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

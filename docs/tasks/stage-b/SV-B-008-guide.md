# SV-B-008 — Guide и ранний контекст при startup

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Ранние формы и sheet могут открыть Guide по точному topic и вернуться с тем же выбором.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Поздние обязательные проверки для primary owner этого тикета: [SV-B-011](SV-B-011-transfer.md), [SV-B-023](SV-B-023-birth-choices.md), [SV-B-033](SV-B-033-guide-tools.md), [SV-B-035](SV-B-035-sheet.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.guide.read` | Read local Guide through U/TomeNET-Guide.txt override then bundled B/G fallback; preserve source bytes, colour formatting and logical line identity, separate from server help. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462)<br>[c-cmd.c:2310](../../../src/client/c-cmd.c#L2310) |
| `capability.guide.navigate` | Navigate page/line/top/bottom/chapter and horizontal offset; preserve caller and scroll across resize/reopen. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.search` | Search Guide with baseline forward/reverse/case/regexp and failed/canceled search behavior; source byte limits remain per field. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462)<br>[c-cmd.c:2310](../../../src/client/c-cmd.c#L2310) |
| `capability.guide.close` | Close Guide and restore caller, pending selection and baseline macro/flush policy. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462)<br>[c-cmd.c:2310](../../../src/client/c-cmd.c#L2310) |
| `capability.guide.server-open` | Receive_Guide decodes search_type byte, signed32 line and source search string; publish only complete packet, open requested local Guide mode and restore caller after close. Preserve ordinary/strict/chapter/line search modes. | [nclient.c:455](../../../src/client/nclient.c#L455)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.mark-results` | Mark old/new search results in visible Guide content without changing source or search origin. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.restore-search` | Return to Guide position before search without resetting current caller. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.guide.help` | Show Guide navigation help then restore existing position/search state. | [c-cmd.c:2310](../../../src/client/c-cmd.c#L2310)<br>[c-birth.c:512](../../../src/client/c-birth.c#L512)<br>[c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) |
| `capability.files.bookmarks-load` | Load private S/bookmarks.tmp with original format and Guide line validity; legacy bookmarks require explicit import. | [c-init.c:3393](../../../src/client/c-init.c#L3393)<br>[c-init.c:3419](../../../src/client/c-init.c#L3419)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
| `capability.files.bookmarks-save` | Save Guide bookmarks at baseline lifecycle to same private path; failure is visible and preserves prior data. | [c-init.c:3393](../../../src/client/c-init.c#L3393)<br>[c-init.c:3419](../../../src/client/c-init.c#L3419)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. User override/bundled Guide: lossless source lines/markers, navigate/search/failed search/REGEX_SEARCH gates, missing file и server-directed opening; no universal context fallback.
2. Race/trait/class/stats/mode/body и live/final sheet topic arguments проверяются на соответствующих later B callers; close сохраняет exact parent/pending selection.
3. Bookmarks use own S files, load/save failure и teardown; viewer caches обновляются после FILE/Lua replacement. D context-help union не принят ранним viewer.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.guide.read` | User Guide override, bundled fallback, missing/empty file и logical line identity через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. B caller-specific Guide topics from each birth step, final/live sheet and server opening tested through existing B caller/result obligations; full skills/lore caller-union remains D. For race/trait/class/stats/mode/body and sheet context, topic search matches exact baseline argument; close/cancel restores same caller/pending selection. Missing Guide/search failure/resize/network update preserve early owner; C skill and D lore callers require separate full-context regressions. Full deferred outcomes: capability.guide.context-help. |
| `capability.guide.navigate` | Page/line/top/bottom/chapter/horizontal offset сохраняются после resize/reopen через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.search` | Forward/reverse/case/regexp, missing/cancel search и exact byte limits через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. B caller-specific Guide topics from each birth step, final/live sheet and server opening tested through existing B caller/result obligations; full skills/lore caller-union remains D. For race/trait/class/stats/mode/body and sheet context, topic search matches exact baseline argument; close/cancel restores same caller/pending selection. Missing Guide/search failure/resize/network update preserve early owner; C skill and D lore callers require separate full-context regressions. Full deferred outcomes: capability.guide.context-help. |
| `capability.guide.close` | Esc возвращает конкретный birth step/previous view и его pending input через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. B caller-specific Guide topics from each birth step, final/live sheet and server opening tested through existing B caller/result obligations; full skills/lore caller-union remains D. For race/trait/class/stats/mode/body and sheet context, topic search matches exact baseline argument; close/cancel restores same caller/pending selection. Missing Guide/search failure/resize/network update preserve early owner; C skill and D lore callers require separate full-context regressions. Full deferred outcomes: capability.guide.context-help. |
| `capability.guide.server-open` | Receive_Guide type byte + signed32 line + string: split/chained, source modes, restore caller через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.mark-results` | Old/new matches меняют только highlight, не source/origin через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.restore-search` | Восстановить presearch position без сброса caller через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.guide.help` | Открыть navigation help и восстановить search/scroll точно через production viewer из birth и повторное открытие. Неполный server input/отмена/resize не создают вторичный send или потерю caller. |
| `capability.files.bookmarks-load` | Read S/bookmarks.tmp bounded slots/names; invalid line, missing file, no automatic legacy import. |
| `capability.files.bookmarks-save` | Изменить bookmark, exit без settings Save, reload; disk fault сохраняет прежний файл и видимый failure. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.guide.read`, `capability.guide.navigate`, `capability.guide.search`, `capability.guide.close`, `capability.guide.server-open`, `capability.guide.mark-results`, `capability.guide.restore-search`, `capability.guide.help`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; GUIDE_BOOKMARKS for bookmark operations, REGEX_SEARCH for regexp; SDL3_CURL_SSL for automatic download/checksum. No server required except Receive_Guide..
- `capability.files.bookmarks-load`, `capability.files.bookmarks-save`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

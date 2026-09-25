# SV-B-030 — Clipboard и URL extraction без нарушения приватности

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок копирует/вставляет допустимый текст и открывает извлечённый URL, не передавая секреты в общую историю.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-005](SV-B-005-vault.md), [SV-B-020](SV-B-020-first-session.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `field-clipboard`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-030](SV-B-030-clipboard.md). Первое native поле получает настоящий platform clipboard read/error bridge и передачу bounded bytes в production editor.001 реализует общий bridge,005 подключает private-field exclusions;030 остаётся полным owner clipboard, включая остальные поля/платформенные ветви. В001 проверить real address-field paste и provider unavailable, selection replacement и итоговую capacity; в005 — private draft/history/diagnostics; в030 повторить обе интеграции на полном platform matrix. До030 не заявлять полный clipboard outcome.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-001](SV-B-001-endpoint.md), [SV-B-005](SV-B-005-vault.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.clipboard.copy` | Copy decoded visible text through source profile, strip FF formatting/control markers, preserve permitted text; exclude secrets and retain exact caller selection. | [c-util.c:2074](../../../src/client/c-util.c#L2074)<br>[c-util.c:2073](../../../src/client/c-util.c#L2073) |
| `capability.clipboard.failure` | Report unavailable OS clipboard/provider failure without fabricating success or replacing original selection. | [c-util.c:2074](../../../src/client/c-util.c#L2074)<br>[c-util.c:2073](../../../src/client/c-util.c#L2073) |
| `capability.clipboard.paste` | Accept OS UTF-8 text through actual target-field byte contract, including unencodable/overflow/control handling from session-policy; test full transformed serialization limit, never infer outgoing charset from chosen font. | [c-util.c:2385](../../../src/client/c-util.c#L2385)<br>[c-util.c:2073](../../../src/client/c-util.c#L2073) |
| `capability.clipboard.extract-url` | Repeated copy invokes baseline URL extraction from message text; preserve escaped colon handling and no-URL fallback without including private fields. | [c-util.c:47](../../../src/client/c-util.c#L47)<br>[c-util.c:2073](../../../src/client/c-util.c#L2073) |
| `capability.os.open-url` | Open selected URL through SDL OS integration with explicit errors; retain caller and never claim external program success from request alone. | [c-cmd.c:6992](../../../src/client/c-cmd.c#L6992)<br>[c-cmd.c:7003](../../../src/client/c-cmd.c#L7003) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Field capacity после transforms, signed-char/locale, multiline/sanitization по concrete owner, selection replacement и exact resulting bytes.
2. Private input clipboard/history/diagnostic exclusions, unavailable provider/tool failure и no stale paste after context change.
3. URL extraction/open system handoff success/error, no fabricated success; earlier text field acceptance повторяется с real platform clipboard.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.clipboard.copy` | Visible selection decoded по source profile, FF/control stripped; clipboard содержит ровно выбранный public text, secret selection excluded Test login/chat/Guide native calls on Linux and Windows, with synthetic Unicode/legacy profiles. |
| `capability.clipboard.failure` | Unavailable/locked OS clipboard returns visible failure, original selection/draft preserved, no reported success Test login/chat/Guide native calls on Linux and Windows, with synthetic Unicode/legacy profiles. |
| `capability.clipboard.paste` | UTF-8 OS draft→actual target bytes with transformed wire limits, unencodable/overflow/NUL/control rejection preserves draft Test login/chat/Guide native calls on Linux and Windows, with synthetic Unicode/legacy profiles. |
| `capability.clipboard.extract-url` | Repeated copy extracts http/www/escaped-colon URL; no-URL fallback keeps baseline copied text, unrelated private data absent Test login/chat/Guide native calls on Linux and Windows, with synthetic Unicode/legacy profiles. |
| `capability.os.open-url` | SDL OS handler receives exact public URL once, unavailable/failure visible; request success не выдаётся за success external browser. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.clipboard.copy`, `capability.clipboard.failure`, `capability.clipboard.paste`, `capability.clipboard.extract-url`, `capability.os.open-url`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

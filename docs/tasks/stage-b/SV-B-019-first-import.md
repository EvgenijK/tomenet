# SV-B-019 — First-launch data-only импорт

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Первый запуск без S CFG предлагает найденные legacy settings и явно переносит выбранные независимые группы.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-005](SV-B-005-vault.md), [SV-B-013](SV-B-013-fonts-load.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Поздние обязательные проверки для primary owner этого тикета: [SV-B-020](SV-B-020-first-session.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-003](SV-B-003-profile.md), [SV-B-005](SV-B-005-vault.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.imports.offer` | On absence of SV CFG offer known SDL3/X11 source only; no whole-disk discovery, silent import or source mixing. Skipping without Save permits offer next launch. | [c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.imports.select` | Explicitly select source and preview CFG/OPT, linked macros, INS/DNA, resource preferences, history/bookmarks/notes/credentials groups. | [c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.imports.conflicts` | Destination wins by default; explicit replacements use scope+key, trigger/stage or whole ordered file/document identity, preserving duplicate legitimate history events. | [c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.imports.commit` | Stage without executing records or changing source. Commit each complete independent group or retain previous destination; report partial success per group and skip malformed linked groups. | [c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.imports.resources` | Import references without fetching resources; main Term font maps to both roles once; numeric legacy Lanczos2 maps to Linear with report, never PixelArt. | [c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.imports.credentials` | Only explicit credential import may read legacy password; write exact bytes to destination OS vault, keep existing secret by default, report provider failure and never create secret-bearing config/temp/backup. | [c-files.c:1045](../../../src/client/c-files.c#L1045) |
| `capability.imports.cancel` | Cancel import leaves all sources and destinations unchanged, with no queued macro execution. | [c-files.c:1045](../../../src/client/c-files.c#L1045) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Offer only with missing own CFG+known legacy source; preview CFG/OPT/linked macro/INS/DNA/resources/history/bookmarks/notes/credentials, никакого исполнения PRF/script.
2. Destination wins by default; explicit replace per conflict, staged group commit and partial group failures, cancel unchanged all source/destination hashes.
3. Main Term font maps to both roles once, credential bytes/private preview/provider failure и no plaintext fallback; full reimport management E отдельно.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.imports.offer` | Нет S CFG+known SDL3/X11→offer; skip leaves no CFG, next launch offers; no discovery whole disk/no source mixing Synthetic source tree на Linux/Windows; immutable source hashes и staged ownership, no queued !/? execution. |
| `capability.imports.select` | Preview каждого data group CFG/OPT/linked macro/INS/DNA/resources/history/bookmarks/notes/credentials без execution Synthetic source tree на Linux/Windows; immutable source hashes и staged ownership, no queued !/? execution. |
| `capability.imports.conflicts` | Destination wins default; scope+key/file/stage identities, explicit replace; duplicate history occurrences remain duplicates Synthetic source tree на Linux/Windows; immutable source hashes и staged ownership, no queued !/? execution. |
| `capability.imports.commit` | Whole independent groups staged+committed; linked missing/cycle/out-of-source skip group; partial success reported per group Synthetic source tree на Linux/Windows; immutable source hashes и staged ownership, no queued !/? execution. |
| `capability.imports.resources` | Main Term font maps to both roles once; Lanczos2→Linear report; missing resource retains reference/fallback, no download Synthetic source tree на Linux/Windows; immutable source hashes и staged ownership, no queued !/? execution. |
| `capability.imports.credentials` | Legacy pass only via explicit import; existing vault wins; exact-byte replacement+failure, no secret CFG/temp/backups Synthetic source tree на Linux/Windows; immutable source hashes и staged ownership, no queued !/? execution. |
| `capability.imports.cancel` | Cancel all preview/selection paths leaves all destination/source hashes and queued actions unchanged Synthetic source tree на Linux/Windows; immutable source hashes и staged ownership, no queued !/? execution. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.imports.offer`, `capability.imports.select`, `capability.imports.conflicts`, `capability.imports.commit`, `capability.imports.resources`, `capability.imports.credentials`, `capability.imports.cancel`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

# SV-B-011 — Startup FILE и Lua reload

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Серверные startup files проходят CHECK/INIT/DATA/END и становятся доступны Lua/Guide до соответствующего игрового сценария.

## Зависимости и граница

Завершить необходимые production части [SV-B-002](SV-B-002-contact.md), [SV-B-003](SV-B-003-profile.md), [SV-B-008](SV-B-008-guide.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Поздние обязательные проверки для primary owner этого тикета: [SV-B-020](SV-B-020-first-session.md), [SV-B-023](SV-B-023-birth-choices.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-008](SV-B-008-guide.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.transfer.check` | Preserve original server file CHECK validation, destination mapping and checksum reply; no new resource-only allow-list and no blanket rebasing onto S. | [nclient.c:351](../../../src/client/nclient.c#L351)<br>[nclient.c:477](../../../src/client/nclient.c#L477)<br>[files.c:157](../../../src/common/files.c#L157) |
| `capability.transfer.init` | Preserve FILE_INIT IDs, destination validation and temporary ownership; missing/failed open reports baseline failure. | [nclient.c:351](../../../src/client/nclient.c#L351)<br>[nclient.c:477](../../../src/client/nclient.c#L477)<br>[files.c:157](../../../src/common/files.c#L157) |
| `capability.transfer.data` | Receive FILE_DATA chunks in order with original length/ID/version rules; partial packets publish no partial application and disk failure is not success. | [nclient.c:351](../../../src/client/nclient.c#L351)<br>[nclient.c:477](../../../src/client/nclient.c#L477)<br>[files.c:157](../../../src/common/files.c#L157) |
| `capability.transfer.end` | Complete FILE_END and original publish/reload/ack behavior exactly once; failed transfer retains required error semantics and does not falsely reload. | [nclient.c:351](../../../src/client/nclient.c#L351)<br>[nclient.c:477](../../../src/client/nclient.c#L477)<br>[files.c:157](../../../src/common/files.c#L157) |
| `capability.lua.reload` | Reload scripts at established preference/server-transfer lifecycle and regenerate Guide metadata; failure remains visible without inventing a second interpreter. | [c-script.c:533](../../../src/client/c-script.c#L533)<br>[nclient.c:351](../../../src/client/nclient.c#L351)<br>[nclient.c:547](../../../src/client/nclient.c#L547) |
| `capability.session.transfer-startup-files` | Process server file init/data/end/check/ack/error and reload required Lua/data before dependent play; preserve file ownership and failure rather than using stale scripts as successful startup. | [nclient.c:351](../../../src/client/nclient.c#L351)<br>[nclient.c:18](../../../src/client/nclient.c#L18)<br>[c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.startup-file-failure` | File-transfer checksum mismatch, write/read/open failure or malformed transfer fails explicitly with the defined error/ack path; dependent Lua/game setup cannot claim readiness. | [nclient.c:351](../../../src/client/nclient.c#L351)<br>[nclient.c:18](../../../src/client/nclient.c#L18)<br>[c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Real server FILE round trip и deterministic peer fragmentation: success/ACK/ERR, version/checksum/filename boundaries, existing server mapping — без нового resource-only allow-list.
2. Disk/temp owner failures, canceled/partial transfer, teardown while write pending; no partial publication/repeated commit/reply.
3. Final END production reopen_lua/Guide metadata refresh; parse/load failure сохраняет baseline recovery и старое working state где предусмотрено.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.transfer.check` | Preserve original server file CHECK validation, destination mapping and checksum reply; no new resource-only allow-list and no blanket rebasing onto S. Wrong/zero ID, split/chained packet, disk failure and relog cleanup; source/destination mapping same baseline, no new allow-list. |
| `capability.transfer.init` | Preserve FILE_INIT IDs, destination validation and temporary ownership; missing/failed open reports baseline failure. Wrong/zero ID, split/chained packet, disk failure and relog cleanup; source/destination mapping same baseline, no new allow-list. |
| `capability.transfer.data` | Receive FILE_DATA chunks in order with original length/ID/version rules; partial packets publish no partial application and disk failure is not success. Wrong/zero ID, split/chained packet, disk failure and relog cleanup; source/destination mapping same baseline, no new allow-list. |
| `capability.transfer.end` | Complete FILE_END and original publish/reload/ack behavior exactly once; failed transfer retains required error semantics and does not falsely reload. Wrong/zero ID, split/chained packet, disk failure and relog cleanup; source/destination mapping same baseline, no new allow-list. |
| `capability.lua.reload` | Last transfer completion reload once; updated guide getters rebuild; failed/missing script visible; updated_audio emits restart warning only, no false fresh audio mapping. |
| `capability.session.transfer-startup-files` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Process server file init/data/end/check/ack/error and reload required Lua/data before dependent play; preserve file ownership and failure rather than using stale scripts as successful startup. |
| `capability.session.startup-file-failure` | File-transfer checksum mismatch, write/read/open failure or malformed transfer fails explicitly with the defined error/ack path; dependent Lua/game setup cannot claim readiness. Exercise this exact owner through native production input/model/rendering with success or rejection as applicable, resize and focus changes, and disconnect during the flow. Restore its parent and macro policy; stale session input must not submit. Record actual Linux software/accelerated and Windows 10/11 results; Wine alone is intermediate evidence. No synthetic Stage A result certifies this flow. Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: File-transfer checksum mismatch, write/read/open failure or malformed transfer fails explicitly with the defined error/ack path; dependent Lua/game setup cannot claim readiness. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.transfer.check`, `capability.transfer.init`, `capability.transfer.data`, `capability.transfer.end`, `capability.lua.reload`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..
- `capability.session.transfer-startup-files`: versions — File checksum protocol >4.6.1.1.0.1 branches; server filename and transfer framing retain field-specific bounds.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.session.startup-file-failure`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

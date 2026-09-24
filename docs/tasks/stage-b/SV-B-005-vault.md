# SV-B-005 — Приватный ввод и системное хранилище для входа

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Для выбранных server/port/account игрок использует точные secret bytes или остаётся в session-only режиме при отказе provider.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-003](SV-B-003-profile.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `field-clipboard`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-030](SV-B-030-clipboard.md). Первое native поле получает настоящий platform clipboard read/error bridge и передачу bounded bytes в production editor.001 реализует общий bridge,005 подключает private-field exclusions;030 остаётся полным owner clipboard, включая остальные поля/платформенные ветви. В001 проверить real address-field paste и provider unavailable, selection replacement и итоговую capacity; в005 — private draft/history/diagnostics; в030 повторить обе интеграции на полном platform matrix. До030 не заявлять полный clipboard outcome.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-006](SV-B-006-login.md), [SV-B-019](SV-B-019-first-import.md), [SV-B-021](SV-B-021-account-manage.md), [SV-B-030](SV-B-030-clipboard.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.credentials.lookup` | Lookup namespace tomenet-sv/v1 by collision-free length-prefixed raw server spelling/effective port/account tuple; Windows lowercase hex, Linux same identity. No account case fold or DNS/IP identity merge; enforce provider length bounds. OS vault is approved SV replacement for legacy storage; acceptance policy supplies this new contract, not the legacy config implementation. | [client.c:236](../../../src/client/client.c#L236)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.credentials.provider-linux` | Use current-user Secret Service/libsecret binary values on Linux; provider unlock prompt remains provider-owned and startup/input stay responsive. OS vault is approved SV replacement for legacy storage; acceptance policy supplies this new contract, not the legacy config implementation. | [client.c:236](../../../src/client/client.c#L236)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.credentials.provider-windows` | Use current-user generic Credential Manager CRED_TYPE_GENERIC/CRED_PERSIST_LOCAL_MACHINE on Windows10/11 i686; no roaming or own master password. OS vault is approved SV replacement for legacy storage; acceptance policy supplies this new contract, not the legacy config implementation. | [client.c:236](../../../src/client/client.c#L236)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.credentials.private-input` | Passwords retain exact source bytes, field limits and protocol-star rejection; exclude history/logs/clipboard exports/dumps/reversible encodings; temporary archive passwords also private. | [client.c:236](../../../src/client/client.c#L236)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.credentials.change-write` | After accepted password-change send immediately update vault per approved policy; handle failure visibly with session-only fallback. Settings Save/exit does not defer or repeat this write. OS vault is approved SV replacement for legacy storage; acceptance policy supplies this new contract, not the legacy config implementation. | [client.c:236](../../../src/client/client.c#L236)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.secret-provider-failure` | Unavailable, locked, refused or failed credential provider leaves an explicit unsaved session-only credential; do not report storage success or fall back to legacy password files. | [c-birth.c:265](../../../src/client/c-birth.c#L265)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.restore-secret` | Look up the exact server spelling/effective port/account byte tuple in Linux Secret Service or Windows Credential Manager; invalid or missing secret returns to private manual entry with no hidden legacy-file fallback. | [c-birth.c:265](../../../src/client/c-birth.c#L265)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.save-secret` | After successful authentication save exact credential bytes in the OS provider; independent SV identity metadata contains no secret or reversible representation. | [nclient.c:1426](../../../src/client/nclient.c#L1426)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Linux Secret Service/libsecret binary values и Windows generic Credential Manager на реальных Windows10/11: exact namespace/length-prefixed identity, aliases и account case не сливаются.
2. No record/external delete/locked/refused/oversized provider result → ручной private draft, никакого plaintext fallback; callback старого session generation освобождается без применения.
3. Успешный login сохраняет raw bytes, rejection не удаляет record; private paste/history/diagnostics исключают секрет. Пароли не меняют Unicode normalization/font projection.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.credentials.lookup` | Collision-free raw server spelling/port/account tuple; Windows lowercase hex; no casefold/DNS merge; provider length overflow visible Synthetic secrets only; output/artifact scan и late completion session identity checks. |
| `capability.credentials.provider-linux` | Current-user Secret Service binary bytes; locked/unavailable/unlock canceled, asynchronous late result after relog, responsive UI Synthetic secrets only; output/artifact scan и late completion session identity checks. |
| `capability.credentials.provider-windows` | Windows10/11 i686 CRED_TYPE_GENERIC/CRED_PERSIST_LOCAL_MACHINE exact blob; native read/write errors, no roaming/master password Synthetic secrets only; output/artifact scan и late completion session identity checks. |
| `capability.credentials.private-input` | Password byte boundaries and star rejection; no normalization/history/log/export/reversible data; temporary archive input protected Synthetic secrets only; output/artifact scan и late completion session identity checks. |
| `capability.credentials.change-write` | After accepted password-change send write vault immediately once; provider failure session-only, no deferred Save/exit repeat Synthetic secrets only; output/artifact scan и late completion session identity checks. |
| `capability.account.secret-provider-failure` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Unavailable, locked, refused or failed credential provider leaves an explicit unsaved session-only credential; do not report storage success or fall back to legacy password files. |
| `capability.account.restore-secret` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Look up the exact server spelling/effective port/account byte tuple in Linux Secret Service or Windows Credential Manager; invalid or missing secret returns to private manual entry with no hidden legacy-file fallback. |
| `capability.account.save-secret` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: After successful authentication save exact credential bytes in the OS provider; independent SV identity metadata contains no secret or reversible representation. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.credentials.lookup`, `capability.credentials.provider-linux`, `capability.credentials.provider-windows`, `capability.credentials.private-input`, `capability.credentials.change-write`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..
- `capability.account.secret-provider-failure`, `capability.account.restore-secret`, `capability.account.save-secret`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

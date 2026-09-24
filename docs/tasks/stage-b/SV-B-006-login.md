# SV-B-006 — Вход и выбор существующего персонажа

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Существующий аккаунт получает подтверждённый overview, выбирает персонажа, читает MOTD; disconnect закрывает текущую сессию.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-002](SV-B-002-contact.md), [SV-B-005](SV-B-005-vault.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `field-clipboard`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-030](SV-B-030-clipboard.md). Первое native поле получает настоящий platform clipboard read/error bridge и передачу bounded bytes в production editor.001 реализует общий bridge,005 подключает private-field exclusions;030 остаётся полным owner clipboard, включая остальные поля/платформенные ветви. В001 проверить real address-field paste и provider unavailable, selection replacement и итоговую capacity; в005 — private draft/history/diagnostics; в030 повторить обе интеграции на полном platform matrix. До030 не заявлять полный clipboard outcome.
- `sticky-input`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-043](SV-B-043-screenshots-platform.md). С первым SDL physical-key consumer реализовать enabled next-key modifier latch с одноразовым consume/cancel/focus reset и disabled normal modifiers.043 сохраняет полный platform/build acceptance owner. 001 проверяет production SDL adapter в enabled/disabled конфигурациях;043 повторяет ранние поля и игровые contexts на обязательных платформах.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-020](SV-B-020-first-session.md), [SV-B-025](SV-B-025-entry-complete.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-001](SV-B-001-endpoint.md), [SV-B-005](SV-B-005-vault.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.account.enter-name` | Interactive account name has 15 payload bytes and live trimming; CLI/config producers retain their own boundaries and server validation. | [c-birth.c:66](../../../src/client/c-birth.c#L66)<br>[nserver.c:320](../../../src/server/nserver.c#L320)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.cancel-name` | Escape from account-name entry exits startup; it does not silently authenticate a stored identity. | [c-birth.c:66](../../../src/client/c-birth.c#L66)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.enter-password` | Accept private password bytes up to 15 interactive payload bytes; empty input retries, no normalization, history or diagnostic capture. | [c-birth.c:265](../../../src/client/c-birth.c#L265)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.cancel-password` | Escape from password entry returns to account-name selection without a login packet. | [c-birth.c:265](../../../src/client/c-birth.c#L265)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.reject-unencodable-password` | For server_protocol >= 2 reject login or password change containing star before XOR42 creates a NUL; preserve private draft for correction and send no partial packet. | [nclient.c:1426](../../../src/client/nclient.c#L1426)<br>[nclient.c:8154](../../../src/client/nclient.c#L8154)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.authenticate` | Verify credentials and receive account/character overview; success is server-confirmed, never inferred from sending login. | [nclient.c:1426](../../../src/client/nclient.c#L1426)<br>[nclient.c:762](../../../src/client/nclient.c#L762)<br>[nserver.c:320](../../../src/server/nserver.c#L320)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.login-rejected` | Wrong password, invalid name or server rejection shows failure and returns to the appropriate retry owner or exits when RETRY_LOGIN is disabled; clear invalid defaults as baseline requires. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[nserver.c:320](../../../src/server/nserver.c#L320)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.read-information` | Read account validation and account information from the account-options surface; Escape returns to the options parent. | [c-util.c:13174](../../../src/client/c-util.c#L13174)<br>[nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.read-overview` | Render the terminated server character list with names, modes, levels, race/class and available location; preserve server flags, total/dedicated limits and first-run restrictions. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.select-existing` | Select an owned slot by letter and complete login status handling before starting play; command-line/default character selection must resolve identically. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.character.quit-overview` | Q or Ctrl-Q exits the overview through the baseline connection cleanup without selecting a character. | [nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.read-motd` | Present received MOTD; acknowledge via a nonzero key after the baseline wait, returning to the startup/play-handshake owner before Net_start; gameplay input is not yet enabled. | [c-files.c:1935](../../../src/client/c-files.c#L1935)<br>[c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.read-identity` | Read current race/class/trait/sex/mode/lives/name from the server with explicit defaults for absent older fields; do not confuse birth draft with accepted identity. | [nclient.c:358](../../../src/client/nclient.c#L358)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.disconnect` | Invalidate session requests, macros and stale input; present reason and take baseline reconnect/exit branch without dispatching pending gameplay or fabricating replies. | [nclient.c:344](../../../src/client/nclient.c#L344)<br>[c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Реальный auth round trip: interactive name/password15 payload bytes, server-owned rejection, empty-password retry, cancel-name exits startup, cancel-password возвращает account selection.
2. При server_protocol>=2 reject star до XOR42/NUL без partial send и потери private draft; protocol<2 не получает лишний запрет. Auth success только после server confirmation.
3. Existing character selection → setup MOTD23×120-byte rows, colors/no wrap/complete fit, any-key, -m skip и once-per-login; до gameplay HUD отсутствует.
4. Disconnect с pending input/network/provider completion прекращает commands, закрывает requests, освобождает owned resources; новым session generation старые ответы недоступны.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.account.enter-name` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Interactive account name has 15 payload bytes and live trimming; CLI/config producers retain their own boundaries and server validation. |
| `capability.account.cancel-name` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Escape from account-name entry exits startup; it does not silently authenticate a stored identity. |
| `capability.account.enter-password` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Accept private password bytes up to 15 interactive payload bytes; empty input retries, no normalization, history or diagnostic capture. |
| `capability.account.cancel-password` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Escape from password entry returns to account-name selection without a login packet. |
| `capability.account.reject-unencodable-password` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: For server_protocol >= 2 reject login or password change containing star before XOR42 creates a NUL; preserve private draft for correction and send no partial packet. |
| `capability.account.authenticate` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Verify credentials and receive account/character overview; success is server-confirmed, never inferred from sending login. |
| `capability.account.login-rejected` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Wrong password, invalid name or server rejection shows failure and returns to the appropriate retry owner or exits when RETRY_LOGIN is disabled; clear invalid defaults as baseline requires. |
| `capability.account.read-information` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Read account validation and account information from the account-options surface; Escape returns to the options parent. |
| `capability.character.read-overview` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Render the terminated server character list with names, modes, levels, race/class and available location; preserve server flags, total/dedicated limits and first-run restrictions. |
| `capability.character.select-existing` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Select an owned slot by letter and complete login status handling before starting play; command-line/default character selection must resolve identically. |
| `capability.character.quit-overview` | Каждая допустимая/недоступная slot/name/reorder ветвь; точные login bytes и подтверждённый refresh; cancel без отправки. Точный проверяемый результат: Q or Ctrl-Q exits the overview through the baseline connection cleanup without selecting a character. |
| `capability.session.read-motd` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Present received MOTD; acknowledge via a nonzero key after the baseline wait, returning to the startup/play-handshake owner before Net_start; gameplay input is not yet enabled. |
| `capability.session.read-identity` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Read current race/class/trait/sex/mode/lives/name from the server with explicit defaults for absent older fields; do not confuse birth draft with accepted identity. |
| `capability.session.disconnect` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Invalidate session requests, macros and stale input; present reason and take baseline reconnect/exit branch without dispatching pending gameplay or fabricating replies. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.account.enter-name`, `capability.account.cancel-name`, `capability.account.enter-password`, `capability.account.cancel-password`, `capability.account.authenticate`, `capability.account.read-information`, `capability.character.quit-overview`, `capability.session.disconnect`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.account.reject-unencodable-password`: versions — server_protocol >= 2; protocol <2 retains its explicit transport path.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.account.login-rejected`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — RETRY_LOGIN / SIMPLE_LOGIN; runtime rl_password and invalid-account-name retry branches..
- `capability.character.read-overview`, `capability.character.select-existing`: versions — Character list: >4.5.7 includes location; >4.4.9.2 includes mode; older omits both. Reorder >=4.7.3. Dedicated capacity >4.5.8.1 and arcade/RPG changes at 4.9.0.5. Login initial address bytes >=4.9.2.1.0.2.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.session.read-motd`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Initial login when skip_motd is false; RETRY_LOGIN sets skip_motd after first display so character switches skip the presentation..
- `capability.session.read-identity`: versions — >=4.9.2.1.0.1 i32 mode; >=4.7.3 lives; >4.5.2 name; >4.4.5.10 trait; older sex/mode only.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

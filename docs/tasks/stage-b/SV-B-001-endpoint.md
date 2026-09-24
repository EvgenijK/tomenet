# SV-B-001 — Выбор сервера и редактирование адреса

Статус: implementation-ready для следующих B-задач на Linux; полное acceptance pending до поздних integration checks и платформенного matrix. [Evidence текущей реализации](../../sv-b001-evidence.md).

## Пользовательский результат

Игрок выбирает endpoint или вводит адрес в native whole-window форме; Escape завершает именно startup caller, input method и физические клавиши сохраняют байтовые границы.

## Зависимости и граница

Предыдущего B owner не требуется; используется принятый synthetic foundation A.

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `field-clipboard`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-030](SV-B-030-clipboard.md). Первое native поле получает настоящий platform clipboard read/error bridge и передачу bounded bytes в production editor.001 реализует общий bridge,005 подключает private-field exclusions;030 остаётся полным owner clipboard, включая остальные поля/платформенные ветви. В001 проверить real address-field paste и provider unavailable, selection replacement и итоговую capacity; в005 — private draft/history/diagnostics; в030 повторить обе интеграции на полном platform matrix. До030 не заявлять полный clipboard outcome.
- `sticky-input`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-043](SV-B-043-screenshots-platform.md). С первым SDL physical-key consumer реализовать enabled next-key modifier latch с одноразовым consume/cancel/focus reset и disabled normal modifiers.043 сохраняет полный platform/build acceptance owner. 001 проверяет production SDL adapter в enabled/disabled конфигурациях;043 повторяет ранние поля и игровые contexts на обязательных платформах.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-006](SV-B-006-login.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-030](SV-B-030-clipboard.md), [SV-B-043](SV-B-043-screenshots-platform.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.connection.select-server` | Select a metaserver entry and preserve its endpoint; ping results may update without changing the selected server. | [c-birth.c:2482](../../../src/client/c-birth.c#L2482)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.connection.enter-host` | Enter host and effective port manually or from startup arguments; no hostname-to-IP canonicalization of credential identity. | [c-birth.c:2482](../../../src/client/c-birth.c#L2482)<br>[c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.connection.cancel-host` | Escape cancels manual entry and follows the startup caller exit path without contacting a guessed/default host. | [c-birth.c:2482](../../../src/client/c-birth.c#L2482)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.input.physical-keys` | Preserve printable, function, keypad, navigation, modifier and lock key byte sequences on Linux/Windows; X11-compatible macro triggers remain usable. | [main-sdl3.c:1173](../../../src/client/main-sdl3.c#L1173) |
| `capability.input.prompt-navigation` | Decode navigation and caller-specific macro bypass without changing parent input context. | [c-util.c:855](../../../src/client/c-util.c#L855) |
| `capability.input.text-edit` | Edit bounded byte fields with cursor, history/search, clipboard and private masking; preserve caller-specific accept/cancel/empty behavior and approved field safety policy. | [c-util.c:1649](../../../src/client/c-util.c#L1649) |
| `capability.input.confirm` | Preserve distinct binary/default/retry/ternary confirmation contracts and queue flush; caller decides whether cancel aborts an action. | [c-util.c:3816](../../../src/client/c-util.c#L3816) |
| `capability.input.raw-key` | Unmapped gameplay key follows baseline raw-key packet dispatch; reserved no-op keys remain excluded from that dispatch. | [nclient.c:8107](../../../src/client/nclient.c#L8107) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Запустить native scene; выбрать metaserver row, обновить ping и проверить неизменность выбора/host spelling/effective port; отменить ручной ввод без guessed/default connect.
2. Через SDL adapter и headless production router проверить normal/roguelike physical keys, raw bypass, navigation, selection replacement, paste/default/macro text at limit−1/limit/limit+1; неизвестное Unicode↔byte соответствие оставляет draft с encoding error.
3. Enter/Escape/confirm идут к текущему owner один раз; focus/resize и pending server event не сбрасывают caret/draft. Наличие этих primitives не принимает всех B/C/D callers.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.connection.select-server` | Выбор/ввод/отмена endpoint; negotiated versions; DNS/socket/timeout/ban/setup failures, без ложного gameplay. Точный проверяемый результат: Select a metaserver entry and preserve its endpoint; ping results may update without changing the selected server. |
| `capability.connection.enter-host` | Выбор/ввод/отмена endpoint; negotiated versions; DNS/socket/timeout/ban/setup failures, без ложного gameplay. Точный проверяемый результат: Enter host and effective port manually or from startup arguments; no hostname-to-IP canonicalization of credential identity. |
| `capability.connection.cancel-host` | Выбор/ввод/отмена endpoint; negotiated versions; DNS/socket/timeout/ban/setup failures, без ложного gameplay. Точный проверяемый результат: Escape cancels manual entry and follows the startup caller exit path without contacting a guessed/default host. |
| `capability.input.physical-keys` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Preserve printable, function, keypad, navigation, modifier and lock key byte sequences on Linux/Windows; X11-compatible macro triggers remain usable. |
| `capability.input.prompt-navigation` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Decode navigation and caller-specific macro bypass without changing parent input context. |
| `capability.input.text-edit` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Edit bounded byte fields with cursor, history/search, clipboard and private masking; preserve caller-specific accept/cancel/empty behavior and approved field safety policy. |
| `capability.input.confirm` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Preserve distinct binary/default/retry/ternary confirmation contracts and queue flush; caller decides whether cancel aborts an action. |
| `capability.input.raw-key` | Каждая собственная primitive ветвь по pinned source/инвентарю; production early caller, normal/roguelike/physical/macro, focus/resize/network/teardown; поздние caller tests не засчитываются. Точный проверяемый результат: Unmapped gameplay key follows baseline raw-key packet dispatch; reserved no-op keys remain excluded from that dispatch. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.connection.select-server`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — EXPERIMENTAL_META; META_PINGS; Windows/POSIX ping process handling..
- `capability.connection.enter-host`, `capability.connection.cancel-host`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.input.physical-keys`, `capability.input.prompt-navigation`, `capability.input.text-edit`, `capability.input.confirm`, `capability.input.raw-key`: versions — All baseline versions supported by the cited owner; retain its version branches; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

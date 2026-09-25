# SV-B-002 — Контакт, negotiation и обслуживание соединения

Статус: Linux production contact/control subset реализован и проверен;
полное acceptance pending по оставшимся caller и platform obligations.
[Evidence текущей реализации](../../sv-b002-evidence.md).

## Пользовательский результат

Реальный выбранный сервер отвечает negotiation/setup, а фрагментация и control packets не останавливают native loop.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Открытые ниже caller checks дополняют исходные acceptance SV-B-002; они не
меняют canonical readiness prerequisites.

### Открытые follow-up после Linux contact/control subset (2026-09-24)

Это незакрытые части **собственного acceptance SV-B-002**, а не передача его
capability IDs следующим тикетам. Текущие наблюдения и ограничения — в
[evidence](../../sv-b002-evidence.md). Завершать каждый пункт production-путём
SV, с точными входными и исходящими bytes и актуальным session generation.

1. `capability.network.server-flags`: сохранённые четыре слова должны влиять на
   доступность соответствующих команд и режимов. Сопоставить биты с baseline
   (`SFLG1_BIG_MAP`, `SFLG1_CIPD`/`SFLG1_SIPD`, server type/limits), проверить
   enabled/disabled и обновление флагов в live session. Первые потребители:
   [SV-B-006](SV-B-006-login.md) для обзора персонажей и лимитов,
   [SV-B-014](SV-B-014-map-core.md) для big map и
   [SV-B-031](SV-B-031-chat.md) для item-paste ветвей; прочие команды
   проверять при появлении их caller. Одного хранения `server_flags[4]`
   недостаточно.
2. `capability.network.confirm`: проверить связку с реально ожидающим макросом
   [SV-B-007](SV-B-007-macros.md): регистрация waiter, ровно одно получение
   confirmation byte в порядке пакетов, продолжение wait, stale generation и
   повторный redraw. Наличие input API без macro executor не закрывает пункт.
3. `capability.network.flush`: повторить `disable_flush`/`thin_down_flush`
   и сроки frame submission в игровом presentation caller
   [SV-B-014](SV-B-014-map-core.md). Contact view уже выполняет submit, но
   не доказывает отправку изменённой карты/HUD во время gameplay.
4. Контакт и failures: провести live TCP/серверные fixtures по всем baseline
   version branches, включая старые setup layouts; отдельно DNS, connect,
   timeout, ban, incompatible version, verification и malformed/partial setup.
   Локальные TCP fixtures уже покрывают 4.4.3.1 layout, invalid-host DNS,
   refused connect, silent-peer contact timeout, ban, оба version rejection
   codes и game-full; прочие branches и server/platform matrix открыты.
   Сверить освобождение сокета, видимую причину и разрешённый retry/exit при
   `RETRY_LOGIN`/`ALWAYS_RETRY_LOGIN`/`SIMPLE_LOGIN`, а также Windows/POSIX
   socket errors. Успешный single-version loopback этого не заменяет.
5. Контактные credential bytes: общий SV редактор
   `src/client/sv/input/text-field.c` переводит SDL UTF-8 в Latin-1 без
   установленного wire contract. Контактная форма SV теперь удерживает
   non-ASCII SDL ввод с видимой encoding error, не изменяя private draft;
   raw CLI bytes в serializer остаются неизменными до protocol XOR.
   Установить mapping сравнением с baseline и
   реальным серверным round trip до заявления non-ASCII login; проверить
   ASCII, representable non-ASCII, неподдерживаемый символ и сохранение draft
   при encoding error. Согласовать с private raw-byte owner
   [SV-B-005](SV-B-005-vault.md) и interactive login owner
   [SV-B-006](SV-B-006-login.md); не нормализовать secret и не предполагать
   charset только по glyph/SDL text representation.

Средовые препятствия текущего cumulative gate вынесены в
[SV-V-001](../verification/SV-V-001-b002-matrix-environment.md); это не
переопределяет обязательные платформы или acceptance SV-B-002.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.connection.contact` | Complete TCP contact, version/protocol negotiation, verification and setup before exposing character selection. | [c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[nclient.c:1426](../../../src/client/nclient.c#L1426)<br>[nclient.c:18](../../../src/client/nclient.c#L18)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.connection.contact-failure` | Present DNS/socket/timeout, ban, incompatible-version and verification/setup failure distinctly; release the failed connection and allow only the baseline retry/exit transitions. | [c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[nclient.c:1426](../../../src/client/nclient.c#L1426)<br>[nclient.c:18](../../../src/client/nclient.c#L18)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.network.keepalive` | Consume server keepalive as a payload-free no-op without an acknowledgement. The independent Net_flush/Send_keepalive timer continues during nested interactions. | [nclient.c:415](../../../src/client/nclient.c#L415)<br>[nclient.c:7592](../../../src/client/nclient.c#L7592) |
| `capability.network.ping-echo` | For pong=0 reply exactly once with pong=1 and the unchanged correlation tuple/payload. For pong!=0 update lag telemetry without an echo; preserve bounded sample indexing and platform timer behavior. | [nclient.c:416](../../../src/client/nclient.c#L416) |
| `capability.network.unknown-packet` | Unknown/NULL entries send PKT_UNKNOWNPACKET, request redraw and clear receive buffer according to baseline; never guess packet length. | [nclient.c:1834](../../../src/client/nclient.c#L1834) |
| `capability.network.partial-packet` | Incomplete packets wait without publishing partial state; preserve baseline rollback and ambiguous-progress clearing. | [nclient.c:1834](../../../src/client/nclient.c#L1834) |
| `capability.network.malformed-packet` | Decode failure follows baseline clear/disconnect and releases old session state with visible failure. | [nclient.c:1834](../../../src/client/nclient.c#L1834) |
| `capability.network.keypress-stub` | Consume PKT_KEYPRESS as the current payload-free stub; do not invent injected-key behavior. | [nclient.c:433](../../../src/client/nclient.c#L433) |
| `capability.network.server-flags` | Retain negotiated server feature words and update applicable command availability. | [nclient.c:469](../../../src/client/nclient.c#L469) |
| `capability.network.pause` | Present latest state, discard pending keystrokes, await fresh acknowledgement while processing network, then flush the caller queue. | [nclient.c:401](../../../src/client/nclient.c#L401) |
| `capability.network.flush` | Submit pending visual changes and preserve disable_flush/thin_down_flush behavior without blocking network or introducing legacy Terms. | [nclient.c:384](../../../src/client/nclient.c#L384) |
| `capability.network.confirm` | Apply command confirmation byte to the waiting macro owner in order; duplicate redraw never replays it. | [nclient.c:432](../../../src/client/nclient.c#L432) |
| `capability.network.end-marker` | Consume the payload-free end marker without inventing frame counters or dropping adjacent packets. | [nclient.c:349](../../../src/client/nclient.c#L349) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Провести TCP contact/verify/setup на поддерживаемом сервере; отдельно DNS/socket/timeout/ban/version/setup failures и точные retry/exit paths.
2. Все ветви negotiation и зарегистрированных control handlers: split every field, chained sentinel, malformed/unknown packet; incomplete не изменяет model и не выдаёт side effects, malformed завершает session по policy.
3. Проверить keepalive/ping echo, pause/flush/confirm/end/server flags и no-op keypress: не изобретать input из Receive_keypress; Send queue exhaustion ставит весь packet или ждёт, не partial bytes.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.connection.contact` | Выбор/ввод/отмена endpoint; negotiated versions; DNS/socket/timeout/ban/setup failures, без ложного gameplay. Точный проверяемый результат: Complete TCP contact, version/protocol negotiation, verification and setup before exposing character selection. |
| `capability.connection.contact-failure` | Выбор/ввод/отмена endpoint; negotiated versions; DNS/socket/timeout/ban/setup failures, без ложного gameplay. Точный проверяемый результат: Present DNS/socket/timeout, ban, incompatible-version and verification/setup failure distinctly; release the failed connection and allow only the baseline retry/exit transitions. |
| `capability.network.keepalive` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Consume server keepalive as a payload-free no-op without an acknowledgement. The independent Net_flush/Send_keepalive timer continues during nested interactions. |
| `capability.network.ping-echo` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: For pong=0 reply exactly once with pong=1 and the unchanged correlation tuple/payload. For pong!=0 update lag telemetry without an echo; preserve bounded sample indexing and platform timer behavior. |
| `capability.network.unknown-packet` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Unknown/NULL entries send PKT_UNKNOWNPACKET, request redraw and clear receive buffer according to baseline; never guess packet length. |
| `capability.network.partial-packet` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Incomplete packets wait without publishing partial state; preserve baseline rollback and ambiguous-progress clearing. |
| `capability.network.malformed-packet` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Decode failure follows baseline clear/disconnect and releases old session state with visible failure. |
| `capability.network.keypress-stub` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Consume PKT_KEYPRESS as the current payload-free stub; do not invent injected-key behavior. |
| `capability.network.server-flags` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Retain negotiated server feature words and update applicable command availability. |
| `capability.network.pause` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Present latest state, discard pending keystrokes, await fresh acknowledgement while processing network, then flush the caller queue. |
| `capability.network.flush` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Submit pending visual changes and preserve disable_flush/thin_down_flush behavior without blocking network or introducing legacy Terms. |
| `capability.network.confirm` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Apply command confirmation byte to the waiting macro owner in order; duplicate redraw never replays it. |
| `capability.network.end-marker` | Split packet + sentinel; no partial effects, exactly-once ordered replies/no invented ack; error/unknown/reset; network продолжает работать во вложенном UI. Точный проверяемый результат: Consume the payload-free end marker without inventing frame counters or dropping adjacent packets. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.connection.contact`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.connection.contact-failure`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — RETRY_LOGIN, ALWAYS_RETRY_LOGIN and SIMPLE_LOGIN branches; Windows/POSIX socket errors..
- `capability.network.keepalive`, `capability.network.ping-echo`, `capability.network.keypress-stub`, `capability.network.server-flags`, `capability.network.pause`, `capability.network.flush`, `capability.network.confirm`, `capability.network.end-marker`: versions — always; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.network.unknown-packet`, `capability.network.partial-packet`, `capability.network.malformed-packet`: versions — All baseline versions supported by the cited owner; retain its version branches; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

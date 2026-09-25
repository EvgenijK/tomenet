# SV-B-025 — Полная приёмка входа в игру

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Оба согласованных пути — existing character и завершённое создание — приводят к первому authoritative игровому состоянию.

## Зависимости и граница

Завершить необходимые production части [SV-B-002](SV-B-002-contact.md), [SV-B-006](SV-B-006-login.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-015](SV-B-015-hud-core.md), [SV-B-020](SV-B-020-first-session.md), [SV-B-024](SV-B-024-birth-dna.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `existing-entry`: producer [SV-B-020](SV-B-020-first-session.md); полные owners [SV-B-025](SV-B-025-entry-complete.md). 020 реализует existing-character ветвь production entry transition;025 сохраняет единственного полного владельца, добавляет completed-birth alternative и все readiness prerequisites. Real auth/FILELua/MOTD/Net_start/map/HUD→disconnect; prior birth.complete не заставляет runtime создавать персонажа.Полный ID pending до025.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-006](SV-B-006-login.md), [SV-B-020](SV-B-020-first-session.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.session.enter-game` | After selected or created character play acknowledgement, publish coherent identity/map and load this character profile and macro layers before enabling gameplay input. | [nclient.c:1672](../../../src/client/nclient.c#L1672)<br>[nclient.c:358](../../../src/client/nclient.c#L358)<br>[c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Повторить M1 и новый birth route на real server; verify configured profile/input, FILE/Lua, MOTD, Net_start, live HP и map without premature HUD.
2. Выполнены все canonical prerequisite outcomes, включая birth.complete: это readiness stage gate, а не последовательное исполнение birth для existing character.
3. Оборвать каждый переход, проверить stale replies/macros и запрет fallback в заявляемом полном outcome.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.session.enter-game` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: After selected or created character play acknowledgement, publish coherent identity/map and load this character profile and macro layers before enabling gameplay input. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.session.enter-game`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

# SV-B-042 — Локальные pickup/destroy и slash option changes

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Разрешённые локальные settings и slash enable/disable/toggle меняют точные значения и эффекты раннего caller.

## Зависимости и граница

Завершить необходимые production части [SV-B-004](SV-B-004-save.md), [SV-B-027](SV-B-027-pickup-store.md), [SV-B-031](SV-B-031-chat.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.settings.slash-enable` | /opty /optvy exact option-name operand enables value with original verbose/redundant behavior, immediate hooks and Send_options; big_map maps approved layout owner, not competing OPT. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:8051](../../../src/client/nclient.c#L8051)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.slash-disable` | /optn /optvn disables with original recognizer/hooks/packet conditions and layout-specific big_map disposition. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:8051](../../../src/client/nclient.c#L8051)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.slash-toggle` | /optt /optvt toggles with exact original case/spacing recognition and synchronization conditions. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[nclient.c:8051](../../../src/client/nclient.c#L8051)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.local-pickup` | Exact CI /apickup toggles local auto_pickup and feedback without Client_setup/Send_options, unlike generic option command. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.local-destroy` | Exact CI /adestroy toggles local auto_destroy and feedback without Client_setup/Send_options. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.local-destroy-unmatched` | Exact CI /daunmatched toggles local destroy_all_unmatched; reports dependency without automatically enabling auto_destroy. | [c-cmd.c:412](../../../src/client/c-cmd.c#L412)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Каждая существующая local pickup/destroy/unmatched policy и slash command по source grammar; unknown slash остается forwarded owner.
2. OPT/runtime effects, own Save/reload, cancel/error/alias compatibility; не превращать изменение setting в доказательство позднего item mutation.
3. Keyboard/macro and network update while settings child open, parent focus/queue and teardown exact.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.settings.slash-enable` | Exact case/spacing/name; redundant/verbose enable, immediate hook+Send_options и layout-owned big_map. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.slash-disable` | Disable enabled/disabled/unknown option, verbose output, actual wire conditions, big_map layout ownership. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.slash-toggle` | Два toggle восстанавливают значение; exact recognizer, Send_options once, raw macro entry route. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.local-pickup` | Case-insensitive exact command flips auto_pickup+feedback; Client_setup/Send_options не меняются. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.local-destroy` | /adestroy flips auto_destroy once, no Send_options; later consumer C receives separate test. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.local-destroy-unmatched` | Toggle destroy_all_unmatched не включает auto_destroy; exact recognizer и no Client_setup packet. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.settings.slash-enable`, `capability.settings.slash-disable`, `capability.settings.slash-toggle`, `capability.settings.local-pickup`, `capability.settings.local-destroy`, `capability.settings.local-destroy-unmatched`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

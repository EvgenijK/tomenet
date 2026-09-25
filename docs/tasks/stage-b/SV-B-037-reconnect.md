# SV-B-037 — Reconnect, portal relogin и redraw

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок проходит только предусмотренные baseline relog/reconnect paths, получая новую authoritative сессию.

## Зависимости и граница

Завершить необходимые production части [SV-B-005](SV-B-005-vault.md), [SV-B-006](SV-B-006-login.md), [SV-B-036](SV-B-036-session-end.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.session.reconnect` | Retry login after permitted disconnect/client quit using a fresh session generation; reauthenticate and reselect character without retaining old world or request state. | [c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[nclient.c:344](../../../src/client/nclient.c#L344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.portal-relogin` | SERVER_PORTALS control supplies host, port, account, password, character, reason and delay; treat credentials as transient private control, reset old session and use target identity for reconnect. | [nclient.c:346](../../../src/client/nclient.c#L346)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.redraw` | Request redraw and rebuild current projections from authoritative state without repeating gameplay commands or stale cached bars. | [nclient.c:1865](../../../src/client/nclient.c#L1865) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Disconnect→reconnect и portal relogin сохраняют permitted identity/credential rules; не добавлять universal resume.
2. Old network/provider/macro/request completions не действуют после generation change; resources application scope сохраняются.
3. Redraw строится из coherent current state без повторных game commands/status bars from stale cache; retry/reject/exit each source branch.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.session.reconnect` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Retry login after permitted disconnect/client quit using a fresh session generation; reauthenticate and reselect character without retaining old world or request state. |
| `capability.session.portal-relogin` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: SERVER_PORTALS control supplies host, port, account, password, character, reason and delay; treat credentials as transient private control, reset old session and use target identity for reconnect. |
| `capability.session.redraw` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Request redraw and rebuild current projections from authoritative state without repeating gameplay commands or stale cached bars. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.session.reconnect`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — RETRY_LOGIN; ALWAYS_RETRY_LOGIN; SIMPLE_LOGIN; client quit reason and death-specific branches..
- `capability.session.portal-relogin`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — SERVER_PORTALS, RETRY_LOGIN; Windows Sleep / POSIX usleep delay*100ms..
- `capability.session.redraw`: versions — All baseline versions supported by the cited owner; retain its version branches; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

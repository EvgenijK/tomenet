# SV-B-021 — Создание аккаунта и смена пароля

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок создаёт аккаунт или меняет пароль с правильным protocol/provider result и отменой.

## Зависимости и граница

Завершить необходимые production части [SV-B-002](SV-B-002-contact.md), [SV-B-005](SV-B-005-vault.md), [SV-B-006](SV-B-006-login.md), [SV-B-020](SV-B-020-first-session.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-005](SV-B-005-vault.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.account.create` | A valid unused account name and password follow server new-account handling; account flags/validation and resulting overview remain authoritative. | [c-birth.c:66](../../../src/client/c-birth.c#L66)<br>[nserver.c:320](../../../src/server/nserver.c#L320)<br>[nclient.c:762](../../../src/client/nclient.c#L762)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.change-password` | Enter old/new/repeated private passwords, then submit old/new once; successful send immediately updates session/vault secret under the approved policy, separately from later server success messages. | [c-util.c:13174](../../../src/client/c-util.c#L13174)<br>[nclient.c:8154](../../../src/client/nclient.c#L8154)<br>[nserver.c:424](../../../src/server/nserver.c#L424)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.cancel-password-change` | Escape at old, new or repeat editor returns to account options without sending or replacing the saved credential. | [c-util.c:13174](../../../src/client/c-util.c#L13174)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.account.password-change-failure` | Keep send failure distinct from server wrong-password, length or account-write failure; report unsaved provider result without claiming that the server accepted the change. | [c-util.c:13174](../../../src/client/c-util.c#L13174)<br>[nclient.c:8154](../../../src/client/nclient.c#L8154)<br>[nserver.c:424](../../../src/server/nserver.c#L424)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Create-account server-confirmed success/rejection; cancel и retry не создают дополнительную identity или секрет.
2. Password change confirmation сравнивает raw bytes; protocol-star guard, failed send/reply, immediate vault write after accepted change send по policy, failure visible без plaintext.
3. Disconnect при private child и late provider result не меняют новую session.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.account.create` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: A valid unused account name and password follow server new-account handling; account flags/validation and resulting overview remain authoritative. |
| `capability.account.change-password` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Enter old/new/repeated private passwords, then submit old/new once; successful send immediately updates session/vault secret under the approved policy, separately from later server success messages. |
| `capability.account.cancel-password-change` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Escape at old, new or repeat editor returns to account options without sending or replacing the saved credential. |
| `capability.account.password-change-failure` | Полный результат ID: accept/cancel/reject/retry и bytes; приватность/provider failure/старый session; server_protocol <2/≥2 и RETRY_LOGIN где применимо. Точный проверяемый результат: Keep send failure distinct from server wrong-password, length or account-write failure; report unsaved provider result without claiming that the server accepted the change. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.account.create`, `capability.account.change-password`, `capability.account.cancel-password-change`, `capability.account.password-change-failure`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

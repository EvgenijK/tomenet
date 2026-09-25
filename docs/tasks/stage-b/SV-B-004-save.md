# SV-B-004 — Preview, Save, Cancel и конфликт настроек

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Изменение ранней настройки можно preview, явно сохранить или отменить; ошибка записи и внешний конфликт не теряют исходный файл.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.settings.preview` | Opening settings snapshots values; edits preview immediately. Prepare resource replacements before activation. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.save` | Explicit Save merges changed CFG/OPT records into their independent SV owners; current character default with explicit global/class/named alternatives. Use owned sibling temporary/backup/replace paths; preserve old destination on failure. | [client.c:69](../../../src/client/client.c#L69)<br>[c-util.c:15299](../../../src/client/c-util.c#L15299)<br>[client.c:187](../../../src/client/client.c#L187)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.cancel` | Cancel changes restores opening settings values, never gameplay state or already delivered effects. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.dirty-close` | Dirty close offers Save / Cancel changes / Return; Return preserves draft and active form. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.exit-unsaved` | Normal exit neither saves pending CFG/OPT changes nor introduces another prompt; other file lifecycles remain independent. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.save-failure` | Failed Save keeps edited values active and dirty, retains previous destination and reports unsaved state. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.save-conflict` | Reread destination and merge changed records; same-key external edit wins and is reported. Preserve unrelated unknown records; do not claim atomicity across uncoordinated legacy writers. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.save-global-options` | Explicit global OPT snapshot writes S/global.opt with baseline option meanings and independent owner. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.save-class-options` | Explicit class OPT snapshot writes own selected class file; saving does not create automatic class loading. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.save-named-options` | Named load/save preserves explicit destination semantics and reports failures without rewriting the source macro PRF. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Через native settings child: opening snapshot → preview → Cancel восстанавливает settings, не gameplay/effects; dirty close Save/Cancel changes/Return, Return сохраняет draft.
2. Explicit own CFG/OPT Save использует sibling temporary/backup/replace; injected stat/read/write/replace failures сохраняют destination, edited values active+dirty. Внешний same-key edit выигрывает; unrelated/unknown records остаются.
3. Global/class/named OPT snapshots и normal exit без auto-save/лишнего prompt; parse of any later field не считается его whole outcome acceptance.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.settings.preview` | Open snapshot→edit visible effect→failed replacement retains previous effective resource; gameplay state не откатывается. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.save` | CFG/OPT changed-record merge для character/global/class/named; sibling temp+replace; concurrent edit/disk fault сохраняют прежний destination. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.cancel` | Change несколько fields, game updates meanwhile, Cancel восстанавливает opening settings, не gameplay/effects. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.dirty-close` | Все3 choices при dirty; Return сохраняет draft/focus, failed Save остаётся dirty. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.exit-unsaved` | Normal exit с dirty settings не сохраняет и не добавляет prompt; history/DNA/bookmarks independent lifecycles продолжаются. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.save-failure` | Denied/open/write/replace fault: old bytes целы, edit active+dirty, visible unsaved status; без secret backups. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.save-conflict` | External same-key wins/report, disjoint edits preserved, unknown records не теряются; повтор Save не скрывает конфликт. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.save-global-options` | Explicit global snapshot в S/global.opt; shared U/global.opt неизменён, option meaning/default/load order сохранены. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.save-class-options` | Save S selected class file; следующий startup не получает нового class-OPT autoload; invalid class/disk error. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.save-named-options` | Named load/save destination, cancel, filename byte limits, source PRF immutable и failed publication. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.settings.preview`, `capability.settings.save`, `capability.settings.cancel`, `capability.settings.dirty-close`, `capability.settings.exit-unsaved`, `capability.settings.save-failure`, `capability.settings.save-conflict`, `capability.settings.save-global-options`, `capability.settings.save-class-options`, `capability.settings.save-named-options`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

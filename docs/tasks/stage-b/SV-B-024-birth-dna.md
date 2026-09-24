# SV-B-024 — Birth DNA и завершение создания

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Сохранённый DNA помогает создать нового персонажа, а completed birth передаёт правильные choices серверу.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-023](SV-B-023-birth-choices.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.files.dna-shared` | DNA stays shared U/user: normalized name/reincarnation lookup, automatic birth save and conversion preserve legacy behavior. | [c-files.c:424](../../../src/client/c-files.c#L424)<br>[c-files.c:3478](../../../src/client/c-files.c#L3478)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
| `capability.birth.restore-dna` | Load character DNA from the shared baseline filename; missing/invalid or incompatible choices cannot bypass legality checks. Preserve # selection and percent auto-reincarnation semantics. | [c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.save-dna` | Save accepted birth DNA in the shared user-resource namespace using baseline filename/format; report write failure truthfully, preserving previously valid data. | [c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.complete` | Only after all required creation steps and dedicated mode adjustments submit final identity, stats, options, geometry and graphics/resource handshake and wait for play acknowledgement. | [nclient.c:1672](../../../src/client/nclient.c#L1672)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[nclient.c:18](../../../src/client/nclient.c#L18)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. На shared U owner чтение/сохранение DNA по baseline names, missing/old/invalid data и disk failure, source isolation.
2. Lua race/class/trait data и configured options загружены до нужного create/Send_options flow; final creation success/rejection без guessed defaults.
3. Full birth end-to-end с backtracking/help, restore/save, disconnect и повторным login; это альтернативный runtime entry path существующему персонажу.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.files.dna-shared` | Normalized birth filename, missing/old DNA conversion, completed birth autosave и cancel; legacy caller видит тот же shared owner. |
| `capability.birth.restore-dna` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Load character DNA from the shared baseline filename; missing/invalid or incompatible choices cannot bypass legality checks. Preserve # selection and percent auto-reincarnation semantics. |
| `capability.birth.save-dna` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Save accepted birth DNA in the shared user-resource namespace using baseline filename/format; report write failure truthfully, preserving previously valid data. |
| `capability.birth.complete` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Only after all required creation steps and dedicated mode adjustments submit final identity, stats, options, geometry and graphics/resource handshake and wait for play acknowledgement. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.files.dna-shared`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..
- `capability.birth.restore-dna`, `capability.birth.save-dna`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..
- `capability.birth.complete`: versions — Net_start identity >4.4.5.10; graphics/font >=4.8.1.2; options >4.5.5, >4.5.8.1.0.1, >4.9.1; dimensions >4.4.9.1.0.1; glyphs >=4.8.1; feature tables >4.9.1.2.0.2.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

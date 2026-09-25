# SV-B-029 — Обзор карты и locate

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок исследует overview/sector/locate и возвращается к текущей игровой карте.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-025](SV-B-025-entry-complete.md), [SV-B-028](SV-B-028-movement.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `field-clipboard`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-030](SV-B-030-clipboard.md). Первое native поле получает настоящий platform clipboard read/error bridge и передачу bounded bytes в production editor.001 реализует общий bridge,005 подключает private-field exclusions;030 остаётся полным owner clipboard, включая остальные поля/платформенные ветви. В001 проверить real address-field paste и provider unavailable, selection replacement и итоговую capacity; в005 — private draft/history/diagnostics; в030 повторить обе интеграции на полном platform matrix. До030 не заявлять полный clipboard outcome.
- `map-chat`: producer [SV-B-029](SV-B-029-map-explore.md); полные owners [SV-B-031](SV-B-031-chat.md). 029 вводит actual native chat child с map/locate continuation.Реализовать все достижимые из этого child baseline branches: ordinary/private/channel transforms, own history, available item substitutions, local-self/forwarded slash и Escape без Send_msg.029 использует real editor/clipboard001 и histories010; это не fixed-response или test-only chat.031 расширяет и принимает полные chat outcomes во всех обязательных B callers. 029 выполняет map-specific success/cancel/transform overflow/history/provider failure и exact map selection/focus/queue return с interleaved network/resize.031 повторяет map branch; Guide/sheet/final callers добавляют integration checks033/035/036, результаты прикладываются к исходному owner031.
- `map-capture`: producer [SV-B-029](SV-B-029-map-explore.md); полные owners [SV-B-034](SV-B-034-exports.md), [SV-B-043](SV-B-043-screenshots-platform.md). 029 вводит реальный native composed-frame capture child и production encoder/file-owner/collision path для доступных map bindings: PNG с SDL3_image, BMP без него, no silent overwrite, requested filename semantics и no notification inside saved frame.035/036 используют тот же production child из sheet/final scene.034 и043 сохраняют полных owners file-collision/capture/platform outcomes. 029 проверяет real image pixels/extension и success/cancel/write/encode/collision failures с возвратом map caller.034 расширяет collision tests на все exports,035/036 добавляют реальных callers,043 проверяет все aliases/server triggers и platform configurations и повторяет029/035/036. Никакого test-only screenshot callback.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-031](SV-B-031-chat.md), [SV-B-043](SV-B-043-screenshots-platform.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-001](SV-B-001-endpoint.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-028](SV-B-028-movement.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.map.view-overview` | Request mode 0 overview or forced world mode 1; display complete map stream and own player-sector marker. | [c-cmd.c:555](../../../src/client/c-cmd.c#L555)<br>[nclient.c:385](../../../src/client/nclient.c#L385)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.map.pan-overview` | Navigation requests retain baseline map direction bitmask; 5/Space/r recenters, with server version and selector-scrolling gates. | [c-cmd.c:555](../../../src/client/c-cmd.c#L555)<br>[nclient.c:385](../../../src/client/nclient.c#L385)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.map.select-sector` | s toggles world-map selector; direction moves bounded sector selection and shows coordinates/distance without moving player. | [c-cmd.c:555](../../../src/client/c-cmd.c#L555)<br>[nclient.c:385](../../../src/client/nclient.c#L385)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.map.cancel-sector` | Escape from selection clears selector while map remains active; another Escape closes map. | [c-cmd.c:555](../../../src/client/c-cmd.c#L555)<br>[nclient.c:385](../../../src/client/nclient.c#L385)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.map.close-overview` | Escape/M exits map, clears local_map_active and restores parent with queue flush; do not resend map commands from redraw. | [c-cmd.c:555](../../../src/client/c-cmd.c#L555)<br>[nclient.c:385](../../../src/client/nclient.c#L385)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.map.locate` | Send_locate(5) opens panel browsing, direction sends panel movement without moving player. | [c-cmd.c:788](../../../src/client/c-cmd.c#L788)<br>[nclient.c:385](../../../src/client/nclient.c#L385)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.map.close-locate` | Exit locate with Escape/Space/L (normal) or W (roguelike), send Send_locate(0) and restore prior input policy. | [c-cmd.c:788](../../../src/client/c-cmd.c#L788)<br>[nclient.c:385](../../../src/client/nclient.c#L385)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Open/pan/select/cancel/close и server map updates сохраняют sector/viewport и target context.
2. Native key/macro child и early chat cancellation возвращают actual map caller; resize меняет geometry без повторной команды.
3. Normal/big, software/accelerated, no crop и restore current map after transient surface.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.map.view-overview` | Normal/big map, mode0/1, восемь битовых направлений и центр; selector bounds/two-step Esc, server 4.8.1.2 + WILDMAP gate; child chat/capture и возврат. Точный проверяемый результат: Request mode 0 overview or forced world mode 1; display complete map stream and own player-sector marker. |
| `capability.map.pan-overview` | Normal/big map, mode0/1, восемь битовых направлений и центр; selector bounds/two-step Esc, server 4.8.1.2 + WILDMAP gate; child chat/capture и возврат. Точный проверяемый результат: Navigation requests retain baseline map direction bitmask; 5/Space/r recenters, with server version and selector-scrolling gates. |
| `capability.map.select-sector` | Normal/big map, mode0/1, восемь битовых направлений и центр; selector bounds/two-step Esc, server 4.8.1.2 + WILDMAP gate; child chat/capture и возврат. Точный проверяемый результат: s toggles world-map selector; direction moves bounded sector selection and shows coordinates/distance without moving player. |
| `capability.map.cancel-sector` | Normal/big map, mode0/1, восемь битовых направлений и центр; selector bounds/two-step Esc, server 4.8.1.2 + WILDMAP gate; child chat/capture и возврат. Точный проверяемый результат: Escape from selection clears selector while map remains active; another Escape closes map. |
| `capability.map.close-overview` | Normal/big map, mode0/1, восемь битовых направлений и центр; selector bounds/two-step Esc, server 4.8.1.2 + WILDMAP gate; child chat/capture и возврат. Точный проверяемый результат: Escape/M exits map, clears local_map_active and restores parent with queue flush; do not resend map commands from redraw. |
| `capability.map.locate` | Normal/big map, mode0/1, восемь битовых направлений и центр; selector bounds/two-step Esc, server 4.8.1.2 + WILDMAP gate; child chat/capture и возврат. Точный проверяемый результат: Send_locate(5) opens panel browsing, direction sends panel movement without moving player. |
| `capability.map.close-locate` | Normal/big map, mode0/1, восемь битовых направлений и центр; selector bounds/two-step Esc, server 4.8.1.2 + WILDMAP gate; child chat/capture и возврат. Точный проверяемый результат: Exit locate with Escape/Space/L (normal) or W (roguelike), send Send_locate(0) and restore prior input policy. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.map.view-overview`, `capability.map.pan-overview`, `capability.map.select-sector`, `capability.map.cancel-sector`, `capability.map.close-overview`, `capability.map.locate`, `capability.map.close-locate`: versions — Overview selector scrolling requires WILDMAP_ALLOW_SELECTOR_SCROLLING and >=4.8.1.2; normal/big map geometry.; Mini-map player marker >=4.8.1.2 includes y_offset; >=4.8.1 uses u32 glyph, older byte glyph.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

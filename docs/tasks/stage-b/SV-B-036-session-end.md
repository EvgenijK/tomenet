# SV-B-036 — Смерть, final review и нормальный выход

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Смерть и завершение сессии показывают baseline final scenes/read-only review и освобождают текущую сессию.

## Зависимости и граница

Завершить необходимые production части [SV-B-004](SV-B-004-save.md), [SV-B-008](SV-B-008-guide.md), [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-025](SV-B-025-entry-complete.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `map-capture`: producer [SV-B-029](SV-B-029-map-explore.md); полные owners [SV-B-034](SV-B-034-exports.md), [SV-B-043](SV-B-043-screenshots-platform.md). 029 вводит реальный native composed-frame capture child и production encoder/file-owner/collision path для доступных map bindings: PNG с SDL3_image, BMP без него, no silent overwrite, requested filename semantics и no notification inside saved frame.035/036 используют тот же production child из sheet/final scene.034 и043 сохраняют полных owners file-collision/capture/platform outcomes. 029 проверяет real image pixels/extension и success/cancel/write/encode/collision failures с возвратом map caller.034 расширяет collision tests на все exports,035/036 добавляют реальных callers,043 проверяет все aliases/server triggers и platform configurations и повторяет029/035/036. Никакого test-only screenshot callback.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-009](SV-B-009-inventory.md), [SV-B-010](SV-B-010-history-profile.md), [SV-B-031](SV-B-031-chat.md), [SV-B-032](SV-B-032-message-recall.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.session.death` | Killed by, Committed suicide and Retired quit reasons enter the appropriate death/tomb or ghost/session handling, clear pending actions and preserve final review data until its owner closes. | [nclient.c:344](../../../src/client/nclient.c#L344)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.close-tomb` | Escape/q/Q/Ctrl-Q leaves the tomb through its shutdown owner and stops tomb weather/effects; redraw does not repeat disconnect. | [c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.review-final-state` | Shutdown review exposes character, inventory, equipment and message/chat history child surfaces and restores the shutdown parent on child close. | [c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.suicide` | Default-no confirmation followed by literal @ submits Send_suicide exactly once; server result controls death transition. | [c-cmd.c:9297](../../../src/client/c-cmd.c#L9297)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.cancel-suicide` | Decline or Escape first confirmation, or any second key except @, returns to gameplay without Send_suicide. | [c-cmd.c:9297](../../../src/client/c-cmd.c#L9297)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.acknowledge-final-scene` | After terminal death/retirement display final scene until literal 0; other keys do not skip this acknowledgement. Then display tomb. | [c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.open-final-review` | A nonzero key other than tomb quit/dump/screenshot commands enters final review; 0 remains at tomb. | [c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.close-final-review` | Escape/Ctrl-Q leaves final-state review and continues the disconnect/relogin path. | [c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.session.quit` | Ctrl-Q or window close follows the live quit owner, saves owned profile/history and releases network/input resources; optional save-chat child returns correctly. | [c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[c-cmd.c:274](../../../src/client/c-cmd.c#L274)<br>[c-util.c:17707](../../../src/client/c-util.c#L17707)<br>[pref.prf:51](../../../lib/user/pref.prf#L51)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Death/tomb/ack/final-review/close paths, suicide confirmation/cancel и actual server termination; surviving transition не принимает ghost powers C.
2. Final sheet/inventory/equipment/bag/history children native, return exact final parent, capture/dump/error по owners; late input не возобновляет игровую сессию.
3. Quit writes owned history/bookmarks по lifecycle, не auto-saves dirty CFG/OPT и не вызывает лишний prompt; teardown cancels pending requests/macros/provider work.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.session.death` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Killed by, Committed suicide and Retired quit reasons enter the appropriate death/tomb or ghost/session handling, clear pending actions and preserve final review data until its owner closes. |
| `capability.session.close-tomb` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Escape/q/Q/Ctrl-Q leaves the tomb through its shutdown owner and stops tomb weather/effects; redraw does not repeat disconnect. |
| `capability.session.review-final-state` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Shutdown review exposes character, inventory, equipment and message/chat history child surfaces and restores the shutdown parent on child close. |
| `capability.session.suicide` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Default-no confirmation followed by literal @ submits Send_suicide exactly once; server result controls death transition. |
| `capability.session.cancel-suicide` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Decline or Escape first confirmation, or any second key except @, returns to gameplay without Send_suicide. |
| `capability.session.acknowledge-final-scene` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: After terminal death/retirement display final scene until literal 0; other keys do not skip this acknowledgement. Then display tomb. |
| `capability.session.open-final-review` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: A nonzero key other than tomb quit/dump/screenshot commands enters final review; 0 remains at tomb. |
| `capability.session.close-final-review` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Escape/Ctrl-Q leaves final-state review and continues the disconnect/relogin path. |
| `capability.session.quit` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Ctrl-Q or window close follows the live quit owner, saves owned profile/history and releases network/input resources; optional save-chat child returns correctly. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.session.death`, `capability.session.close-tomb`, `capability.session.review-final-state`, `capability.session.suicide`, `capability.session.cancel-suicide`, `capability.session.acknowledge-final-scene`, `capability.session.open-final-review`, `capability.session.close-final-review`, `capability.session.quit`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

# SV-B-043 — Снимок экрана и optional platform ветви

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок и разрешённый server trigger создают native PNG/BMP capture; недоступные optional features дают штатный результат.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md), [SV-B-039](SV-B-039-map-effects.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `sticky-input`: producer [SV-B-001](SV-B-001-endpoint.md); полные owners [SV-B-043](SV-B-043-screenshots-platform.md). С первым SDL physical-key consumer реализовать enabled next-key modifier latch с одноразовым consume/cancel/focus reset и disabled normal modifiers.043 сохраняет полный platform/build acceptance owner. 001 проверяет production SDL adapter в enabled/disabled конфигурациях;043 повторяет ранние поля и игровые contexts на обязательных платформах.
- `map-capture`: producer [SV-B-029](SV-B-029-map-explore.md); полные owners [SV-B-034](SV-B-034-exports.md), [SV-B-043](SV-B-043-screenshots-platform.md). 029 вводит реальный native composed-frame capture child и production encoder/file-owner/collision path для доступных map bindings: PNG с SDL3_image, BMP без него, no silent overwrite, requested filename semantics и no notification inside saved frame.035/036 используют тот же production child из sheet/final scene.034 и043 сохраняют полных owners file-collision/capture/platform outcomes. 029 проверяет real image pixels/extension и success/cancel/write/encode/collision failures с возвратом map caller.034 расширяет collision tests на все exports,035/036 добавляют реальных callers,043 проверяет все aliases/server triggers и platform configurations и повторяет029/035/036. Никакого test-only screenshot callback.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-001](SV-B-001-endpoint.md), [SV-B-029](SV-B-029-map-explore.md), [SV-B-034](SV-B-034-exports.md), [SV-B-035](SV-B-035-sheet.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.screenshots.capture` | Ctrl+T/Ctrl+Shift+T and /shot /screenshot plus PNG aliases capture native composed window to PNG when SDL3_image available, BMP otherwise. Retire XHTML and screenshot_keys swapping; preserve filename mode and exact caller. | [c-files.c:2172](../../../src/client/c-files.c#L2172)<br>[main-sdl3.c:8](../../../src/client/main-sdl3.c#L8)<br>[c-cmd.c:8121](../../../src/client/c-cmd.c#L8121)<br>[c-util.c:137](../../../src/client/c-util.c#L137)<br>[nclient.c:408](../../../src/client/nclient.c#L408) |
| `capability.screenshots.server-trigger` | Preserve applicable Receive_chardump/auto-capture triggers with native PNG/BMP outcome and source gates; no XHTML output. | [c-files.c:2172](../../../src/client/c-files.c#L2172)<br>[nclient.c:408](../../../src/client/nclient.c#L408)<br>[c-util.c:137](../../../src/client/c-util.c#L137)<br>[nclient.c:408](../../../src/client/nclient.c#L408) |
| `capability.platform.optional-image` | Exercise SDL3_image enabled PNG and disabled BMP native screenshot paths with correct filename/result reporting. | [makefile.sdl3:5](../../../src/makefile.sdl3#L5) |
| `capability.platform.optional-sticky` | Preserve enabled SDL3_STICKY_KEYS next-key modifier semantics; disabled build remains distinguishable. | [makefile.sdl3:5](../../../src/makefile.sdl3#L5)<br>[main-sdl3.c:1173](../../../src/client/main-sdl3.c#L1173) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Native actual renderer capture, Ctrl+T/Ctrl+Shift+T and server-trigger semantics; filenames/collision/no overwrite по policy, no XHTML.
2. IMAGE enabled/disabled и sticky platform behavior, dependencies/package resources, failure при encode/write/provider unavailable.
3. Software+accelerated and Windows actual runtime, filename byte bounds/privacy; redraw не повторяет capture.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.screenshots.capture` | Ctrl+T/Ctrl+Shift+T and /shot /screenshot plus PNG aliases capture native composed window to PNG when SDL3_image available, BMP otherwise. Retire XHTML and screenshot_keys swapping; preserve filename mode and exact caller. Composed frame includes current overlays; no notification in capture; failure retains previous last-successful path; no XHTML or repeat on redraw. |
| `capability.screenshots.server-trigger` | Preserve applicable Receive_chardump/auto-capture triggers with native PNG/BMP outcome and source gates; no XHTML output. Composed frame includes current overlays; no notification in capture; failure retains previous last-successful path; no XHTML or repeat on redraw. |
| `capability.platform.optional-image` | Compile enabled/disabled output, capture same composed window; verify real encoding matches extension and UI PNG decoding remains available. |
| `capability.platform.optional-sticky` | Enabled next-key latch consumed exactly once, cancel/focus loss; disabled build normal simultaneous modifiers preserved. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.screenshots.capture`, `capability.screenshots.server-trigger`, `capability.platform.optional-image`, `capability.platform.optional-sticky`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

# SV-B-041 — Выбор layout, window mode и UI scale

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок явно выбирает wide/small и масштаб, сохраняя pending interaction и полный viewport.

## Зависимости и граница

Завершить необходимые production части [SV-B-003](SV-B-003-profile.md), [SV-B-004](SV-B-004-save.md), [SV-B-014](SV-B-014-map-core.md), [SV-B-038](SV-B-038-font-choices.md), [SV-B-039](SV-B-039-map-effects.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-014](SV-B-014-map-core.md), [SV-B-038](SV-B-038-font-choices.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.settings.layout` | Choose wide/small, preserve hidden right-panel state and full map fit; startup wide, once-per-launch small offer is ephemeral, legacy big_map cannot override saved layout. | [main-sdl3.c:1946](../../../src/client/main-sdl3.c#L1946)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.window-mode` | Choose fullscreen desktop mode or windowed; default fullscreen. Preserve saved windowed logical size separately; absent size uses work area subject to1024x768 minimum and WM limits. | [main-sdl3.c:1946](../../../src/client/main-sdl3.c#L1946)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.ui-scale` | Choose50..200% step5 with100 default and invalid-value feedback; apply OS DPI independently, resize text/controls/spacing without changing server viewport. | [main-sdl3.c:1946](../../../src/client/main-sdl3.c#L1946)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Wide→big66×44, small→normal66×22, no auto switching; small offer at width<=1280/height<=800 once per launch deferred until ordinary game input.
2. Min1024×768logical,125/150/200%DPI, independent text/map sizing, critical HUD visible, typed lists/details/source views follow approved layout.
3. Preview/Cancel/Save/restart и window fullscreen/decorations behavior; no universal cancel or automatic hidden game commands.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.settings.layout` | Toggle wide/small→66x44/66x22 and wire; hidden right-state сохранён; once-per-launch small offer не persists. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.window-mode` | Save windowed logical size отдельно от fullscreen; WM limits,min1024x768,return mode и no overwrite saved size. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.ui-scale` | 50..200 шаг5,100 default; invalid input retained/report,125/150/200%DPI корректные hits и server map dimensions неизменны. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.settings.layout`, `capability.settings.window-mode`, `capability.settings.ui-scale`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

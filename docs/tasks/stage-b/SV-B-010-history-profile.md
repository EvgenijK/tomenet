# SV-B-010 — Истории и полное применение session profile

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Перед игровой сценой загружаются own history/OPT и shared macro/INS/resource layers; действуют реальные клавиши и audio startup configuration.

## Зависимости и граница

Завершить необходимые production части [SV-B-001](SV-B-001-endpoint.md), [SV-B-003](SV-B-003-profile.md), [SV-B-007](SV-B-007-macros.md), [SV-B-008](SV-B-008-guide.md), [SV-B-009](SV-B-009-inventory.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `initial-audio`: producer [SV-B-003](SV-B-003-profile.md); полные owners [SV-B-012](SV-B-012-audio-start.md). Profile consumer003 вводит production initial device/pack configuration application и неблокирующий init/result bridge.010 использует именно этот путь для восьми retained startup CFG fields;012 расширяет events/music/weather и принимает полный device-failure ID. Bounds/default/requested/effective device/pack/subset реально доходят до initial open, stereo не путается с track count. No-device leaves session responsive;012/020 повторяют startup failures с actual executors, а не parser-only fixture.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-012](SV-B-012-audio-start.md), [SV-B-020](SV-B-020-first-session.md), [SV-B-031](SV-B-031-chat.md), [SV-B-036](SV-B-036-session-end.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-003](SV-B-003-profile.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.files.history-load` | Load private S/chathist-account.tmp with baseline bounds/order/dedup/relog semantics; never automatically read shared legacy input history. | [c-init.c:3393](../../../src/client/c-init.c#L3393)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
| `capability.files.history-save` | Save input history at baseline exit lifecycle to same private path, excluding credentials/archive passwords; no dependency on settings Save. | [c-init.c:3393](../../../src/client/c-init.c#L3393)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
| `capability.session.load-profile-input` | Load independent SV config/options/history and shared resource/macro files in baseline precedence; character/race/trait/class/form overrides must affect actual commands. Missing resource fallback retains requested identity under resolved policy. | [c-init.c:4349](../../../src/client/c-init.c#L4349)<br>[c-util.c:1907](../../../src/client/c-util.c#L1907)<br>[pref-sdl3.prf:21](../../../lib/user/pref-sdl3.prf#L21)<br>[pref.prf:51](../../../lib/user/pref.prf#L51)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10)<br>[c-init.c:383](../../../src/client/c-init.c#L383)<br>[c-files.c:3005](../../../src/client/c-files.c#L3005)<br>[client.c:468](../../../src/client/client.c#L468)<br>[client.c:69](../../../src/client/client.c#L69) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Load actual character profile before first Send_options; verify private own chat/history files, missing files and save failure, teardown/restart persistence.
2. Production startup выполняет весь retained handoff INS и восьми audio fields; fixture наблюдает concrete requested/effective device/pack application, не один parser.
3. Normal/roguelike commands и ordered macros после profile load используют нужный layer; profile failures не означают fake live readiness.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.files.history-load` | S/chathist-account.tmp load: ring wrap, duplicates, empty/missing/corrupt file, смена account; legacy history не читается. |
| `capability.files.history-save` | Exit/relog сохраняет только public chat draft history; проверить credential/archive exclusion, write failure и exact S owner. |
| `capability.session.load-profile-input` | Точный переход success/failure/cancel и teardown/relogin; отсутствие преждевременного gameplay, старых replies/macros/provider completions; реальные server round trips. Точный проверяемый результат: Load independent SV config/options/history and shared resource/macro files in baseline precedence; character/race/trait/class/form overrides must affect actual commands. Missing resource fallback retains requested identity under resolved policy. B читает наиболее специфичный shared INS, выполняет необходимую для чтения conversion и применяет правила на update; named/global/class SAVE и full management outcome остаются E. Read most-specific shared U/user INS перед autoinscribe update; missing/old format, preserving rule order; no source rewrite except approved conversion. B inventory update invokes production autoinscription once, unrelated shared file unchanged; E adds every explicit Save branch. Full deferred outcomes: capability.files.ins-shared. B startup projection прочитывает requested pack/subset, resolves overlay/bundled names, applies fallback/effective initial audio и reports identity. E whole config outcomes add live chooser/preview/Save/reload without semantic narrowing. Initial sound/music folder+subset loaded from S CFG before actual audio initialization; invalid/missing pack preserves requested name and exposes effective failure. No implicit CFG write during startup; verify Send_audio identity and disabled/missing sound behavior. Full deferred outcomes: capability.configuration.soundpackfolder, capability.configuration.soundpacksubset, capability.configuration.musicpackfolder, capability.configuration.musicpacksubset. B startup projection parses/defaults/applies cacheAudio,rate4000..48000,tracks4..32,buffer128..8192 при initial open. Full live preview/reconfigure/Save/reload IDs remain E. Boundary/invalid/default serialized device fields passed into actual production init, stereo output remains stereo (tracks!=speaker count). No-device startup stays responsive and connected, effective degradation visible, requested config not overwritten. Full deferred outcomes: capability.configuration.cacheaudio, capability.configuration.audiosamplerate, capability.configuration.audiochannels, capability.configuration.audiobuffer. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.files.history-load`, `capability.files.history-save`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..
- `capability.session.load-profile-input`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

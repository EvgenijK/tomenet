# SV-B-003 — Первый запуск с собственным CFG и ресурсами

Статус: частичная startup CFG/OPT и resource-owner реализация; implementation readiness и полное acceptance pending. [Evidence](../../sv-b003-evidence.md).

Текущий SV executable загружает `S/options.prf`, `S/global.opt`,
`S/global-sv.opt` до контактного экрана, не читая legacy `U/global.opt`.
Отдельный production seam применяет `S/<character>.opt` после выбора персонажа
и формирует `PKT_OPTIONS` по четырём исходным version gates. Он ещё не вызван
в реальном login flow: до B-006 нет имени персонажа и первого `Send_options`.
Alias conversions из `c-files.c` выполняются в памяти; файл не переписывается.

Общий SV resolver выбирает U overlay перед B, возвращает конкретный owner
для read/stat и raw операций над ресурсом; текстовый шрифт использует его.
Startup показывает requested и найденный source для map font, graphics и
audio pack folders. `source=found` означает только наличие файла/каталога:
эффективную карту, пак и аудиоустройство проверят соответствующие consumers.

## Пользовательский результат

Чистый и существующий профиль загружаются в независимом U/sv; requested/effective ресурсы и CLI overrides видны до входа.

## Зависимости и граница

Предыдущего B owner не требуется; используется принятый synthetic foundation A.

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `initial-audio`: producer [SV-B-003](SV-B-003-profile.md); полные owners [SV-B-012](SV-B-012-audio-start.md). Profile consumer003 вводит production initial device/pack configuration application и неблокирующий init/result bridge.010 использует именно этот путь для восьми retained startup CFG fields;012 расширяет events/music/weather и принимает полный device-failure ID. Bounds/default/requested/effective device/pack/subset реально доходят до initial open, stereo не путается с track count. No-device leaves session responsive;012/020 повторяют startup failures с actual executors, а не parser-only fixture.

Поздние обязательные проверки для primary owner этого тикета: [SV-B-010](SV-B-010-history-profile.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-019](SV-B-019-first-import.md), [SV-B-020](SV-B-020-first-session.md). До их выполнения разрешено объявить production implementation готовой для следующих задач, но полный acceptance остаётся pending; результаты поздних checks прикладываются к исходным IDs/obligations, не передавая ownership.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.settings.load` | Load independent U/sv/tomenet.cfg and own global/system/character OPT layers; never inherit legacy user CFG/OPT through bootstrap includes. Apply character options before their first send. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934)<br>[client.c:468](../../../src/client/client.c#L468)<br>[client.c:69](../../../src/client/client.c#L69) |
| `capability.settings.defaults` | Use client stock/compiled defaults with approved SV overrides: fullscreen, wide, UI100%, CascadiaMono-Regular.ttf text,16x24x.pcf map, graphics1/16x24sv, Linear tiles, Nearest PCF; FPS100,44100Hz/32 tracks/1024 buffer, volume70. Missing files initialize memory only. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.parse` | Parse line-oriented CFG, LF/CRLF, complete key tokens, comments and flags; last well-formed assignment wins. Report invalid values and use field defaults; reject scalar NUL/newline/unserializable data. Preserve compatible unknown fields; incompatible future schema uses defaults and cannot be overwritten. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.cli-overrides` | Apply baseline CLI/environment/root overrides for this launch without automatic CFG writes or relocation caused by import. | [client.c:69](../../../src/client/client.c#L69)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.settings.aliases` | Convert exact obsolete option aliases including inversions/one-to-many/discards in memory until Save; do not guess renames or rewrite legacy import sources. | [client.c:69](../../../src/client/client.c#L69)<br>[c-files.c:1047](../../../src/client/c-files.c#L1047)<br>[c-cmd.c:8310](../../../src/client/c-cmd.c#L8310)<br>[c-util.c:16934](../../../src/client/c-util.c#L16934) |
| `capability.files.resource-overlay` | Resolve U overlay before bundled B for shared resources; writes retain shared owner and cover raw stat/rename/remove/temp operations, not fopen alone. | [c-files.c:424](../../../src/client/c-files.c#L424)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
| `capability.files.transient-owner` | Backups, temporary/helper/IPC/diagnostic files belong to operation and are not imported as profile data; keep secrets out of all diagnostics. | [c-files.c:424](../../../src/client/c-files.c#L424)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
| `capability.files.report-resource-identity` | Report client version/OS/Guide length, audio pack metadata and requested/effective font/graphics identity through original versioned Send_version/Send_audio/Send_font fields. Preserve full local names; only reporting copies follow approved byte limits, never truncate stored selections. Missing features report baseline values. | [nclient.c:7502](../../../src/client/nclient.c#L7502)<br>[nclient.c:9190](../../../src/client/nclient.c#L9190)<br>[nclient.c:9204](../../../src/client/nclient.c#L9204)<br>[nserver.c:448](../../../src/server/nserver.c#L448)<br>[nserver.c:442](../../../src/server/nserver.c#L442)<br>[nserver.c:449](../../../src/server/nserver.c#L449)<br>[c-init.c:3349](../../../src/client/c-init.c#L3349)<br>[nclient.c:3340](../../../src/client/nclient.c#L3340)<br>[c-util.c:4862](../../../src/client/c-util.c#L4862) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Изолированные user root: own CFG/OPT и approved defaults; last well-formed assignment, LF/CRLF, unknown compatible fields, incompatible schema, missing/corrupt input, CLI override без implicit write.
2. Проверить overlay-before-bundled, полное локальное имя/отдельную ограниченную reporting copy, temp/backup/rename/remove в одном owner; не читать legacy plaintext credential.
3. Startup handoff восьми E audio CFG fields: parse/default/ranges, requested pack/subset, actual init/effective identity. Интеграционную проверку со звуком завершает M1; полные E live editors здесь не принимаются.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.settings.load` | Independent S load global/system/character layers до первого Send_options; shared bootstrap не импортирует legacy user OPT. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. B startup projection прочитывает requested pack/subset, resolves overlay/bundled names, applies fallback/effective initial audio и reports identity. E whole config outcomes add live chooser/preview/Save/reload without semantic narrowing. Initial sound/music folder+subset loaded from S CFG before actual audio initialization; invalid/missing pack preserves requested name and exposes effective failure. No implicit CFG write during startup; verify Send_audio identity and disabled/missing sound behavior. Full deferred outcomes: capability.configuration.soundpackfolder, capability.configuration.soundpacksubset, capability.configuration.musicpackfolder, capability.configuration.musicpacksubset. B startup projection parses/defaults/applies cacheAudio,rate4000..48000,tracks4..32,buffer128..8192 при initial open. Full live preview/reconfigure/Save/reload IDs remain E. Boundary/invalid/default serialized device fields passed into actual production init, stereo output remains stereo (tracks!=speaker count). No-device startup stays responsive and connected, effective degradation visible, requested config not overwritten. Full deferred outcomes: capability.configuration.cacheaudio, capability.configuration.audiosamplerate, capability.configuration.audiochannels, capability.configuration.audiobuffer. |
| `capability.settings.defaults` | Missing config оставляет filesystem неизменным; проверить fullscreen/wide/UI100/fonts/graphics/filter/FPS/audio defaults и invalid-value fallback. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.parse` | LF/CRLF,last-valid token, prefix collision, malformed scalar/NUL,newline,unknown records,future schema write refusal. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.cli-overrides` | CLI/environment precedence; нет implicit save или перемещения U/B после import; fingerprint остаётся U. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.settings.aliases` | Каждая source alias branch: inversion, one-to-many, discard; source не rewritten до explicit destination Save. Production SV путь; проверка последующего reload и unrelated owner isolation там, где есть запись. |
| `capability.files.resource-overlay` | Одинаковый resolved owner для read/stat/temp/rename/remove; overlay missing/invalid, fallback, shared file остаётся shared. |
| `capability.files.transient-owner` | Каждый temp/backup принадлежит resolved operation owner; relog/cancel cleanup; secret bytes не попадают в config temp или diagnostic artifact. |
| `capability.files.report-resource-identity` | Каждая source-defined versioned layout и byte limit в reporting copy; full local name не меняется; отсутствующий pack/font сообщает baseline missing values. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.settings.load`, `capability.settings.defaults`, `capability.settings.parse`, `capability.settings.cli-overrides`, `capability.settings.aliases`, `capability.files.resource-overlay`, `capability.files.transient-owner`, `capability.files.report-resource-identity`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

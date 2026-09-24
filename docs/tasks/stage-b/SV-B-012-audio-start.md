# SV-B-012 — Звук, музыка и предупреждения в живой сессии

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Входящие события дают звук/страницу/предупреждение даже когда отрисовка объединяет обновления.

## Зависимости и граница

Завершить необходимые production части [SV-B-002](SV-B-002-contact.md), [SV-B-003](SV-B-003-profile.md), [SV-B-010](SV-B-010-history-profile.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

- `initial-audio`: producer [SV-B-003](SV-B-003-profile.md); полные owners [SV-B-012](SV-B-012-audio-start.md). Profile consumer003 вводит production initial device/pack configuration application и неблокирующий init/result bridge.010 использует именно этот путь для восьми retained startup CFG fields;012 расширяет events/music/weather и принимает полный device-failure ID. Bounds/default/requested/effective device/pack/subset реально доходят до initial open, stereo не путается с track count. No-device leaves session responsive;012/020 повторяют startup failures с actual executors, а не parser-only fixture.

Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-003](SV-B-003-profile.md), [SV-B-010](SV-B-010-history-profile.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.audio.play-event` | Play original mapped sound event with baseline type/volume/position parameters; preserve USE_SOUND_2010/version branches and missing/disabled-event semantics. | [snd-sdl3.c:3](../../../src/client/snd-sdl3.c#L3)<br>[nclient.c:391](../../../src/client/nclient.c#L391)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.audio.music` | Play/change/stop music including versioned alternative/fallback tracks and loop/fade behavior; redraw cannot retrigger playback. | [snd-sdl3.c:3](../../../src/client/snd-sdl3.c#L3)<br>[nclient.c:426](../../../src/client/nclient.c#L426)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.audio.weather` | Maintain independent weather audio loop/start/stop/volume and current weather preference. | [snd-sdl3.c:3](../../../src/client/snd-sdl3.c#L3)<br>[nclient.c:422](../../../src/client/nclient.c#L422)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.audio.device-failure` | Unavailable device/mixer reports degradation without false successful activation or session disconnect; mandatory build dependencies are not optional runtime evidence. | [snd-sdl3.c:3](../../../src/client/snd-sdl3.c#L3)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043)<br>[client.c:69](../../../src/client/client.c#L69) |
| `capability.audio.ambient` | Apply signed32 ambient event ID, start/stop underlying loop under USE_SOUND_2010/use_sound without redraw replay; consume complete packet even with sound disabled. | [nclient.c:436](../../../src/client/nclient.c#L436)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.alerts.hp-warning` | Deliver each eligible low-HP/damage warning using baseline threshold, AFK/off-panel attention and preferences, including repeated eligible updates. | [nclient.c:354](../../../src/client/nclient.c#L354) |
| `capability.alerts.mana-warning` | Deliver each eligible low-mana warning under its own baseline options/thresholds. | [nclient.c:363](../../../src/client/nclient.c#L363) |
| `capability.alerts.sanity-warning` | Deliver eligible low-sanity/damage warning using version-specific fields and baseline SHOW_SANITY rules. | [nclient.c:403](../../../src/client/nclient.c#L403) |
| `capability.alerts.page` | Deliver each AFK page event with configured paging/audio fallback. | [nclient.c:411](../../../src/client/nclient.c#L411) |
| `capability.alerts.warning-beep` | Deliver each warning-beep event according to audio options and supported fallback. | [nclient.c:412](../../../src/client/nclient.c#L412) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Actual startup device/pack fields(default and bounds) доходят до audio init, stereo output не путается с track count; unavailable/no-device UI остаётся responsive, connection жив, request identity не переписывается.
2. HP50→10→50 и повторные eligible low packets: каждое предупреждение по baseline conditions, музыка/weather/ambient event ordering и suppressed/disabled build paths.
3. Attention owner вместо pixel occlusion, interleaved prompt/resize/relogin; late completion не звучит в новом session generation. E pack/device editors здесь не принимаются.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.audio.play-event` | Play original mapped sound event with baseline type/volume/position parameters; preserve USE_SOUND_2010/version branches and missing/disabled-event semantics. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.audio.music` | Play/change/stop music including versioned alternative/fallback tracks and loop/fade behavior; redraw cannot retrigger playback. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.audio.weather` | Maintain independent weather audio loop/start/stop/volume and current weather preference. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.audio.device-failure` | Unavailable device/mixer reports degradation without false successful activation or session disconnect; mandatory build dependencies are not optional runtime evidence. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. B startup projection parses/defaults/applies cacheAudio,rate4000..48000,tracks4..32,buffer128..8192 при initial open. Full live preview/reconfigure/Save/reload IDs remain E. Boundary/invalid/default serialized device fields passed into actual production init, stereo output remains stereo (tracks!=speaker count). No-device startup stays responsive and connected, effective degradation visible, requested config not overwritten. Full deferred outcomes: capability.configuration.cacheaudio, capability.configuration.audiosamplerate, capability.configuration.audiochannels, capability.configuration.audiobuffer. |
| `capability.audio.ambient` | Apply signed32 ambient event ID, start/stop underlying loop under USE_SOUND_2010/use_sound without redraw replay; consume complete packet even with sound disabled. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.alerts.hp-warning` | Eligible/ineligible и повторный одинаковый event; thresholds/AFK/offpanel/options/sound gates; ordered effect один раз после полного decode, no redraw replay. Точный проверяемый результат: Deliver each eligible low-HP/damage warning using baseline threshold, AFK/off-panel attention and preferences, including repeated eligible updates. |
| `capability.alerts.mana-warning` | Eligible/ineligible и повторный одинаковый event; thresholds/AFK/offpanel/options/sound gates; ordered effect один раз после полного decode, no redraw replay. Точный проверяемый результат: Deliver each eligible low-mana warning under its own baseline options/thresholds. |
| `capability.alerts.sanity-warning` | Eligible/ineligible и повторный одинаковый event; thresholds/AFK/offpanel/options/sound gates; ordered effect один раз после полного decode, no redraw replay. Точный проверяемый результат: Deliver eligible low-sanity/damage warning using version-specific fields and baseline SHOW_SANITY rules. |
| `capability.alerts.page` | Eligible/ineligible и повторный одинаковый event; thresholds/AFK/offpanel/options/sound gates; ordered effect один раз после полного decode, no redraw replay. Точный проверяемый результат: Deliver each AFK page event with configured paging/audio fallback. |
| `capability.alerts.warning-beep` | Eligible/ineligible и повторный одинаковый event; thresholds/AFK/offpanel/options/sound gates; ordered effect один раз после полного decode, no redraw replay. Точный проверяемый результат: Deliver each warning-beep event according to audio options and supported fallback. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.audio.play-event`, `capability.audio.music`, `capability.audio.weather`, `capability.audio.device-failure`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; USE_SOUND_2010, SOUND_SDL3, runtime use_sound/quiet_mode; SDL3_ARCHIVE for extraction; event availability and packet version branches retained..
- `capability.audio.ambient`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — USE_SOUND_2010; ambient playback additionally gated by use_sound. All supported packet versions..
- `capability.alerts.hp-warning`: versions — > 4.7.0.2.0.1; older; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.alerts.mana-warning`, `capability.alerts.page`, `capability.alerts.warning-beep`: versions — always; builds — Supported SV gameplay builds; preserve all baseline compile guards.
- `capability.alerts.sanity-warning`: versions — >= 4.8.1.3.0.0; > 4.6.1.2.0.0 and < 4.8.1.3; <= 4.6.1.2; builds — Supported SV gameplay builds; preserve all baseline compile guards.

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

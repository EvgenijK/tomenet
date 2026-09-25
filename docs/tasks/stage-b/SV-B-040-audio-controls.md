# SV-B-040 — Громкость и mute ранних audio callers

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок управляет master/music/ambient volumes и AFK mute, закрывает audio UI с правильным dirty state.

## Зависимости и граница

Завершить необходимые production части [SV-B-004](SV-B-004-save.md), [SV-B-012](SV-B-012-audio-start.md), [SV-B-020](SV-B-020-first-session.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks

Отдельных ранних subsets или поздних caller checks, кроме cumulative gate, не назначено.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.audio.master` | Preview master/category enable and master/music/effects/weather volumes in private CFG; defaults70 and switches enabled, Save explicit. | [snd-sdl3.c:3](../../../src/client/snd-sdl3.c#L3)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.audio.afk-mute` | Preserve idle/AFK mute and resume ownership without overwriting requested music/weather switches. | [snd-sdl3.c:3](../../../src/client/snd-sdl3.c#L3)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.audio.close` | Close audio settings with approved dirty-save choices and exact parent/queue restoration. | [c-util.c:17376](../../../src/client/c-util.c#L17376)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.audio.music-volume` | Apply Receive_music_vol signed volume v and alternative-track choices in all three versioned layouts; retain fallback/stop/fade semantics without redraw replay. Split/chained packets must publish only complete updates. | [nclient.c:459](../../../src/client/nclient.c#L459)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
| `capability.audio.ambient-volume` | Apply ambient/weather byte volume goals and baseline30-step progression with nonzero rounding correction at1/10-second ticks; consume complete packet even when sound disabled. | [nclient.c:435](../../../src/client/nclient.c#L435)<br>[nclient.c:1](../../../src/client/nclient.c#L1)<br>[c-util.c:18043](../../../src/client/c-util.c#L18043) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Production event/music/weather effects учитывают runtime volumes, AFK/mute и close; neither incoming update nor redraw duplicates sound.
2. CFG preview/save/cancel и dirty Return с реальным audio executor; no-device failure preserves requested settings и session.
3. Full pack installation/selection/device reconfiguration E не входят в эти B controls.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.audio.master` | Preview master/category enable and master/music/effects/weather volumes in private CFG; defaults70 and switches enabled, Save explicit. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.audio.afk-mute` | Preserve idle/AFK mute and resume ownership without overwriting requested music/weather switches. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.audio.close` | Close audio settings with approved dirty-save choices and exact parent/queue restoration. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.audio.music-volume` | Apply Receive_music_vol signed volume v and alternative-track choices in all three versioned layouts; retain fallback/stop/fade semantics without redraw replay. Split/chained packets must publish only complete updates. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |
| `capability.audio.ambient-volume` | Apply ambient/weather byte volume goals and baseline30-step progression with nonzero rounding correction at1/10-second ticks; consume complete packet even when sound disabled. Split/batched wire либо local input→effect: after redraw/recreate эффект не повторяется; missing/disabled device/pack и relog cleanup. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.audio.master`, `capability.audio.afk-mute`, `capability.audio.close`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence.; USE_SOUND_2010, SOUND_SDL3, runtime use_sound/quiet_mode; SDL3_ARCHIVE for extraction; event availability and packet version branches retained..
- `capability.audio.music-volume`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — Linux amd64 / Windows i686 SV. Preserve conditional compilation and runtime availability of the cited baseline owner; enabled and disabled paths need separate evidence..
- `capability.audio.ambient-volume`: versions — Retain all version branches of the cited owner; local operations require no server. Protocol-dependent consumers keep their existing gates and slot/byte identities.; builds — USE_SOUND_2010; ambient playback additionally gated by use_sound. All supported packet versions..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.

# C002: source contract и граница startup audio

Статус: source audit, 2026-11-05; не runtime evidence и не приёмка.
Canonical obligations не изменены. Linux обязателен; Windows отложен по
`AGENTS.md`. Сверены полный [settings policy](../../capabilities/settings-policy.md),
[snapshot](coverage.json), [spec](../../sv-stage-c002-spec.md), `CONTEXT.md`,
ADR-0002/0003/0006. Graphify query выполнен перед поиском исходников; граф
недостаточен для точных branches, выводы проверены по коду.

## CFG и defaults

Основной источник ключей — [client.c](../../../src/client/client.c):69–594;
[stock client CFG](../../../tomenet.cfg), а не `lib/config/tomenet.cfg`.
Полный historical index —
[CFG registry](../../../.scratch/single-window-sdl3-client/research/sv-config-key-draft.md).
Его draft предложения заменяются финальным settings policy.

| Семейство | Сохранённые ключи / смысл |
|---|---|
| Connection | `nick`, `name`, `meta`, `server` (включая старый host:port), `port`, `realname`, `path`, presence `fullauto` |
| Palette | presence `lighterDarkBlue`, `colormap_0`…`colormap_15`; stock palette и stock lighterDarkBlue |
| Graphics | `graphics`, `graphic_tiles`, `graphic_tiles0`…`graphic_tiles9`, `graphicsForceOutline`, `graphic_resize_type`, `disableGfxCache` |
| Shell | `fps`, `windowDecorations`, compatibility `hintBigmap` |
| Audio | `sound`, `hintSound`, `cacheAudio`, `audioSampleRate`, `audioChannels`, `audioBuffer`, `soundpackFolder`, `soundpackSubset`, `musicpackFolder`, `musicpackSubset`, `audioMaster`, `audioMusic`, `audioSound`, `audioWeather`, `audioVolumeMaster`, `audioVolumeMusic`, `audioVolumeSound`, `audioVolumeWeather` |
| Compatibility | Все десять `Term-Main`/`Term-1`…`Term-9` и их старые prefix aliases (`Mainwindow`, `Mirrorwindow`, `Recallwindow`, `Choicewindow`, `Term-4window`…`Term-9window`) с `_Title`, `_Visible`, `_X`, `_Y`, `_Columns`, `_Lines`, `_Font`; не создавать Terms и не угадывать соответствие панелям |
| Secret exception | `pass` распознаётся и отбрасывается; не применять, не удерживать как unknown, не печатать значение |

Новые SV keys и exact bounds — таблица policy: `svSchemaVersion 1`,
`svLayout wide`, `svWindowMode fullscreen`, optional windowed geometry,
`svUiScalePercent 100` (50…200 шаг5), `svTextFont CascadiaMono-Regular.ttf`,
`svMapFont 16x24x.pcf`, resource profile references,
`svGraphicsFilter linear`, `svPcfFilter nearest`; panel/block/widget metadata
не изобретать. Graphics default1, tile16x24sv, FPS100, port18348;
audio44100/32/1024, switches enabled, volumes70, packs sound/music, subset1.
Отсутствие CFG не пишет файл. Legacy hint/cache workaround records не становятся
новыми UI settings. Unsupported build/mechanism records остаются inert с diagnostic.

CFG line grammar принимает LF/CRLF, spaces/tabs и полные tokens, комментарии и
старые presence flags. Не воспроизводить prefix bug `soundpackSubset`→`sound`.
Compatible unknown records удерживаются для будущего Save; `pass` исключён.
Assignments идут по порядку; invalid occurrence сообщает ошибку и возвращает
default этого поля, последующая valid occurrence опять может заменить значение.
Future incompatible schema включает defaults, warning и overwrite prohibition.
Никаких guessed charset, NUL truncation или plaintext credential fallback.

## Roots, CLI и startup order

`U` выбирается до чтения CFG; `S=U/sv`, `B` — game library.
[client.c](../../../src/client/client.c):1328–1397 выбирает SDL base/pref path и
`TOMENET_SDL3_USER_PATH`; fingerprint остаётся U. Уже существующий SV
[launcher](../../../tests/sv/diagnostics/launcher.c):97–165 также предоставляет
`--profile-root`, `--library`, `--server`, `--port`, `--metaserver`, account и
character metadata; `TOMENET_PATH` используется как B fallback.

Baseline [CLI](../../../src/client/client.c):1397–1545 читается **слева направо
после default CFG**. `-fFILE` читает дополнительный CFG прямо в этой позиции;
поэтому `-p1234 -fother.cfg` может закончиться портом из other.cfg. `-i` —
reset уже прочитанных CFG полей, а не только skip будущего чтения. `-pPORT`,
`-FFPS`, `-PPATH`, `-NCHAR`, `-RCHAR` используют attached suffix;
`-R` также включает reincarnation. `-l` допускает attached/spaced account,
следующий positional аргумент baseline считает password; его нельзя логировать
или писать в CFG. `-nNAME` задаёт real name; baseline bare `-n` падает в CUI
branch, который не превращается в SV frontend switch. Positional server
обновляет endpoint. `-q` отключает effective audio после всего CLI parse,
`-m` пропускает MOTD, `-w` weather, `-u` lua updates, `-X` quit prompt;
`-a/-g/-G` выбирают graphics mode после parse. Backend-only debug/cache
switches сохраняют явный unsupported disposition, если механизма в SV нет.
Не объявлять все CLI flags реализованными только потому, что аргумент принят.
Ни один override не вызывает CFG/OPT Save. Import не выбирает roots повторно.
Полный branch-by-branch disposition, включая поздние consumers, SET_UID gates,
excluded CUI и обязательные ранние `-l/-R/-C/-s/-e`, находится в
[CLI disposition](cli-disposition.md). Эта сверка не объявляет полный CLI PASS
по одному факту разбора аргументов.

Baseline [initialize_main_pref_files](../../../src/client/c-init.c):212–254:
compiled option defaults → `pref.prf` → `global.opt` → `pref-<sys>.prf` →
font-specific mapping/graphics → `global-<sys>.opt`.
Policy заменяет settings owners на S (`options.prf`, `global.opt`,
`global-sv.opt`), оставляя shipped bootstrap в B и resource/macro owners shared.
`pref.prf` includes не должны протащить U/user options/global.opt.
Каждый include несёт resolved owner, `.prf` само по себе owner не определяет.

Character OPT применяется в [early pre-send point](../../../src/client/c-init.c):4409–4432,
до **первого** `Send_options` и `Net_start`, а не только после входа.
Поздний [player preferences](../../../src/client/c-init.c):308–350 вновь читает
character OPT, затем global/race/trait/class/character macros. Class OPT
autoload не существует. Полный executable preferences/mapping перенос не C002;
отсутствующий consumer нельзя выдавать за применённый только по parse result.

## Exact aliases и wire gates

[c-files.c](../../../src/client/c-files.c):1092–1214, только X:/Y: boolean options:

| Старое имя | Точная конверсия |
|---|---|
| recall_flicker | subterm_flicker |
| instant_retaliator | new_retaliator с инверсией X↔Y |
| autoloot_depth, autoloot_off | autoloot_dunonly |
| basic_players, consistent_players | basic_players_symb с исходным value; basic_players_col всегда false |
| kind_diz | add_kind_diz |
| auto_inscribe | auto_inscr_server |
| hilite_chat / hibeep_chat | highlight_chat / highbeep_chat |
| view_animated_lite / view_lite_extra / no_lite_fainting | view_animated_light / view_light_extra / no_light_fainting |
| hilite_player | highlight_player |
| colourize_prices | colourize_bignum |
| sp_huge_bar | sn_huge_bar |
| auto_insc_off | auto_inscr_off |
| stack_allow_wands | stack_allow_devices |
| view_reduce_lite | discard, не view_reduce_light |

Для `Send_options` и options внутри `Net_start` сохранять slots и strict gates
из [nclient.c](../../../src/client/nclient.c):8051 и :1703:
>4.9.1.0.0.0 —199; >4.5.8.1.0.1 —154; >4.5.5.0.0.0 —128; иначе96.
Equality попадает в старую ветку. Alias conversion только в памяти, без
переписывания источника. `big_map` policy проецируется из saved SV layout.
Initial `Net_start` viewport projection — 66×44 для effective wide layout,
66×22 для small; это wire следствие settings layout, не приёмка gameplay map
или всего C013. Character OPT `big_map` не создаёт второго источника geometry.

## Initial audio: что действительно входит в C002

Обе дополнительные obligations `settings.load.startup-audio-packs` и
`startup-audio-device` остаются обязательными. Их B/E wording историческое:
canonical allocation полных audio/configuration outcomes теперь C055/C056.
Парсинг requested CFG закрывает только входную часть этих obligations.

Baseline [init_sound](../../../src/client/c-init.c):3502 не открывает backend при
`use_sound=false`; отказ optional audio не должен разрушать сессию (ADR-0006).
[audio_and_config_init](../../../src/client/snd-sdl3.c):1842 открывает mixer,
готовит pack paths и читает sound **и** music configs; failure любой части
освобождает backend. Audio rate4000…48000, tracks4…32, output stereo;
tracks не число speakers ([open_audio](../../../src/client/snd-sdl3.c):1057).

Pack preparation — actual consumer, а не только `packname`: shared overlay
U/xtra/<requested-folder>, bundled B/xtra/<requested-folder>, cfg/default files,
subset lines `<n>|`, quotes/comments/continuations, registry event name из
[audio.lua](../../../lib/scpt/audio.lua), существующие sample/song files,
music `!initial`/`+reference` metadata.
[sound loader](../../../src/client/snd-sdl3.c):1473 считает event только при
наличии sample file; disabled `;` event всё равно counted/provided.
[music loader](../../../src/client/snd-sdl3.c):1640 считает reference directive
и разрешает references после полного чтения. Selected music subset packname
имеет приоритет над base packname независимо от порядка title records.
Нельзя вычислять provided counts числом произвольных строк CFG.

Requested folder/subset остаются в config даже при effective failure; report
effective pack identity отдельно. [Send_audio](../../../src/client/nclient.c):9190:
<4.7.3.0.0.0 — unsupported; затем PKT_AUDIO+2 network int16; начиная
4.9.2.1.0.1 добавляются четыре NUL strings name/version sound/music.
[Net_start](../../../src/client/nclient.c):1690 несёт counts при strict
>4.4.5.10.0.0; graphics/font fields начинаются inclusive4.8.1.2.0.0.
Name bytes в локальной identity не усекать; reporting copy ограничивается
wire field contract. Count identity не заменяет реальную device availability.

До этой реализации точка старта SV — `play-profile.h` counts0 и отсутствие audio owner.
Минимальный допустимый перенос: отдельный SV-local startup consumer с
явным requested/effective snapshot, initial MIX device/track allocation,
pack preparation/cache policy и destroy; snapshot явно передаётся serializer.
Нет необходимости вводить live chooser, playback событий, fade/AFK/weather
state machine, reconfigure или Save, принадлежащие полным C055/C056.
Поддержка initial settings не означает готовность этих поздних outcomes.

## Buffer adaptation и оставшаяся граница

`audioBuffer` в baseline **не применяется**: [open_mixer](../../../src/client/snd-sdl3.c):122
делает `(void)chunksize`, затем `MIX_CreateMixerDevice(default,&spec)`.
`open_audio` лишь clamp/передача аргумента, не evidence размера device buffer.
Policy/obligation требует actual initial application. Поэтому SV-local startup
не копирует этот дефект: перед `MIX_CreateMixerDevice` устанавливает
`SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES` (SDL3 ≥3.2.0), затем восстанавливает прежний
hint; higher-priority environment override не подменяется. Через
`MIX_GetMixerProperties`/`MIX_PROP_MIXER_DEVICE_NUMBER` получает device ID и
`SDL_GetAudioDeviceFormat` читает actual sample frames. Device может согласовать
другое значение: snapshot показывает requested/effective и diagnostic.

Источник API — установленные `/usr/include/SDL3/SDL_hints.h` (device buffer
hint, explicitly before open, no guarantee exact match), `SDL_audio.h`
(`SDL_GetAudioDeviceFormat`) и `SDL3_mixer/SDL_mixer.h` (mixer device property),
SDL3_mixer3.2.4. Это конкретный SV-local backend adaptation, а не waiver и не
исправление legacy. Отдельное предложение исправления legacy записано в
[списке улучшений](../../sv-improvements.md).

Полные C055/C056 не являются необходимым prerequisite для узкого initial
startup consumer. Native evidence initial device/pack/provider/serializer всё
равно обязательно; mere successful MIX init не подтверждает каждую obligation.
Изолированный reader event registry понимает literal shipped audio_sfx/audio_bgm
таблицы; при неподдержанном executable/custom выражении сообщает degradation,
не угадывая индексы. Полное Lua execution и live audio принадлежат поздним owners.

Linux checks должны вызывать production startup consumer и serializer:
enabled valid packs, disabled sound, missing pack, unavailable device,
requested/effective identity и counts/version gates; bytes CFG/OPT неизменны.
Только минимальная core проверка и startup/quit по spec; дополнительных suites
этот audit не разрешает. Source observations выше сами по себе не evidence.

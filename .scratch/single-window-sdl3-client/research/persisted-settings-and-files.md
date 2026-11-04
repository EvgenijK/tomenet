# Persisted settings and files: source inventory

## Статус и границы

Исследование для **Classify persisted settings and files**. Это факты текущего исходного кода и рекомендации для следующего HITL-решения **Define settings and migration boundary**, не утверждение новой политики хранения. Пользовательские файлы и пароли не читались; ничего в клиенте не изменялось. Инвентаризация статическая: Linux/Windows runtime, сбои диска, конкурирующие процессы и все комбинации compile flags не проверены. «Все настройки» ниже означает полный извлечённый набор имен `option_info` и синтаксис двух основных config readers; неизвестные пользовательские файлы не становятся автоматически импортируемыми.

## Установленное поведение источников

### Корни, порядок чтения, platform differences

- X11 читает `$HOME/.tomenetrc` (без HOME — `./.tomenetrc`), SDL3 — `SDL3_USER_PATH/tomenet.cfg`; явный filename заменяет default. Если файла нет, reader пытается скопировать stock config. Это уже запись во время инициализации. [client.c:129](../../../src/client/client.c#L129)
- SDL3 user root задаётся `TOMENET_SDL3_USER_PATH`, иначе `SDL_GetPrefPath(SDL3_ORG_NAME, SDL3_GAME_NAME)`; это platform API, а не жёстко заданный Linux/Windows путь. [client.c:1340](../../../src/client/client.c#L1340)
- SDL3 отдельно создаёт `scpt`, `text`, `user`, `xtra`, `game`; при создании user `scpt` копирует bundled scripts. [c-files.c:905](../../../src/client/c-files.c#L905)
- Legacy Windows с `WINDOWS_USER_HOME`, если не `DontMoveUser` и доступны HOMEDRIVE/HOMEPATH, переносит user/scpt в `\TomeNET-user`, `\TomeNET-scpt`; иначе использует lib directories. `LibPath` и `DontMoveUser` читаются из INI. [c-files.c:860](../../../src/client/c-files.c#L860), [main-win.c:5877](../../../src/client/main-win.c#L5877)
- `my_fopen` SDL3 перенаправляет только пути с prefix `ANGBAND_DIR`: сначала user override, затем bundled read-only fallback; write/append создаёт directories и копирует bundled file в user storage. При совпадении путей открывает напрямую. Это не общий write barrier: `fopen`, `rename`, `remove`, shell utilities обходят helper. [c-files.c:424](../../../src/client/c-files.c#L424)
- SDL3 `os_temp_path` выбирается TMPDIR → TMP → TEMPDIR → TEMP → user root `temp/`. Поэтому временные файлы нельзя считать стабильным user profile. [common.c:322](../../../src/common/common.c#L322)

### Конкретный файловый inventory

`user/` в таблице — логический ANGBAND_DIR_USER, с оговоренными SDL3 overrides; `xtra/<pack>/` — выбранный audio resource directory.

| Семейство | Содержимое, чтение и запись baseline | Рекомендуемый класс для обсуждения |
|---|---|---|
| `.tomenetrc`, SDL3 `tomenet.cfg`, Windows INI | Общие connection/audio/graphics settings смешаны с десятью Term layouts и credentials; writers меняют исходный config | Split by key; legacy source сохранить |
| `user/pref.prf`, `global.opt`, `pref-<sys>.prf`, `global-<sys>.opt` | Начальная последовательность загрузки, затем font prefs | Selective read-only import |
| `user/<character>.opt` | Загружается до/при входе персонажа; explicit save named/global/class | Gameplay import + отдельно presentation choices |
| `user/global.prf`, `<race>.prf`, `<trait>.prf`, `<class>.prf`, `<character>.prf`, form prefs | Последовательные macro overrides; character/form loader, arcade override | Макросы обязательны, импорт action/trigger без исполнения |
| `user/options.prf`, `window.prf`, другие `%:` includes | Любая PRF может смешивать options, window flags, mappings и executable records | Не классифицировать по extension alone |
| `user/<name>.ins`, `global.ins`, class/trait/race `.ins` | Только наиболее специфичный успешно загруженный файл; explicit save named/global/class; old versions авто-resave | Auto-inscription data import без source rewrite |
| `user/<name>.dna` | Sex/body/mode, class, race, trait, stat order; Roman suffix нормализуется, reincarnation может брать previous name; conversion может resave | Birth defaults, отдельная policy scope |
| `user/<base>-STAGE.meta`, `<base>-STAGE<n>.meta`, `<base>-STAGE<n>.prf` | Macro set/stage metadata и действия; menu создаёт, переставляет, удаляет файлы | Import как связанную группу, source immutable |
| `user/chathist-<account>.tmp` | Chat input history, newline strings в ring order, quit saves только непустой history | Personal history import opt-in policy |
| `user/bookmarks.tmp` | Guide bookmarks `line,name`, quit writes непустые entries | Bookmark import; line validity зависит от guide |
| `user/notes-<account>.txt` | Полученные private notes автоматически append; note browser читает `notes-*` | Existing personal documents read-only |
| `user/tomenet-chat_<timestamp>.txt`, `tomenet-messages_<timestamp>.txt` | Chat/all messages export on quit по save_chat/prompt | Outputs/documents; не settings |
| `user/<chosen character dump name>` | `file_character` пишет текстовый character description | Export output; не профиль |
| `user/<screenshot>.xhtml`, PNG/BMP | Screen export, native raster path или browser helpers | Export output; не импорт presentation state |
| `xtra/<soundpack>/sound.cfg`, `sound.cfg.default`; music counterparts | Event→files, pack metadata/subsets, disabled entries `;`; missing config копируется из default | Resource + user overrides, не UI layout |
| `xtra/<pack>/TomeNET-soundvol.cfg`, `TomeNET-musicvol.cfg` | Event name и volume следующей строкой; save только non-default volume, reset truncates | Audio preferences |
| Legacy `TomeNET-sound.cfg`, `TomeNET-music.cfg`, `TomeNET-nosound.cfg`, `TomeNET-nomusic.cfg` | Legacy sound backend имеет отдельные config/disabled overrides | Compatibility import candidates по backend |
| Windows `user/TomeNET-<pack>.cfg`, `TomeNET-<pack>-disabled.cfg`, `TomeNET-<pack>-volume.cfg` | Pack-specific legacy Windows config overrides | Audio compatibility import |
| `xtra/font/*`, `user/font-custom-<font>.prf`, `font-<sys>.prf` | Font assets и glyph mapping, custom first, sys fallback | Resource/font preference; Term font assignment не переносится |
| `xtra/graphics/*`, `user/graphics-<tiles>.prf`, subtiles mappings | Tileset images, masks, custom character maps | Resource/render parity import |
| SDL3 user `xtra/graphics/cache/*` | Generated image cache с temporary publish path | Regenerable cache, не migration data |
| `scpt/*.lua`, `text/*`, `game/*`, guide and checksums | Bundled/user override resources; server file transfer и manual guide update изменяют local files | Core resource/update flow отдельно от UI |
| `.bak`, `.$$$`, `tomexfer.XXXXXX`, helper `.bat`, `__tmp__`, `__temp__`, `__clipboard__`, `__ipc*`, `__tomenet.tmp`, archives | Backups/working outputs external helpers и updates | Legacy artifacts, не auto-import |
| `user/stdout.txt`, ping outputs | Diagnostics/temporary external command output | Logs/temporary, не настройки |

Evidence: startup/character order [c-init.c:234](../../../src/client/c-init.c#L234), [c-init.c:310](../../../src/client/c-init.c#L310); inscription specificity [c-init.c:361](../../../src/client/c-init.c#L361); macro save [c-util.c:5404](../../../src/client/c-util.c#L5404); macro metadata [c-util.c:5623](../../../src/client/c-util.c#L5623), [c-util.c:6773](../../../src/client/c-util.c#L6773); inscription format [c-files.c:2985](../../../src/client/c-files.c#L2985); birth format [c-files.c:3455](../../../src/client/c-files.c#L3455); chat/bookmark writers [c-init.c:3360](../../../src/client/c-init.c#L3360); private note append [nclient.c:3354](../../../src/client/nclient.c#L3354); dumps [c-files.c:2277](../../../src/client/c-files.c#L2277); screenshots [c-files.c:2731](../../../src/client/c-files.c#L2731); audio default [snd-sdl3.c:1175](../../../src/client/snd-sdl3.c#L1175), audio save [snd-sdl3.c:4109](../../../src/client/snd-sdl3.c#L4109), [snd-sdl3.c:4775](../../../src/client/snd-sdl3.c#L4775); legacy audio filenames [snd-sdl.c:755](../../../src/client/snd-sdl.c#L755), [snd-sdl.c:1081](../../../src/client/snd-sdl.c#L1081), [snd-sdl.c:1644](../../../src/client/snd-sdl.c#L1644); graphics cache [main-sdl3.c:2680](../../../src/client/main-sdl3.c#L2680); font/graphics PRF discovery [c-util.c:20139](../../../src/client/c-util.c#L20139), [c-util.c:20243](../../../src/client/c-util.c#L20243); diagnostic stdout [c-util.c:4901](../../../src/client/c-util.c#L4901).

### Migration-critical побочные эффекты

1. PRF reader не является pure data reader. `%:` recursively loads another file **до** macro-exclusive filter; `!:` и `?:` автоматически помещают macro action в input queue (`?:` suppressed только при processing bodymonster macros). `#:` emits formatted message/chat. Без специального import mode stock load может выполнить действия. [c-files.c:1075](../../../src/client/c-files.c#L1075), [c-files.c:1588](../../../src/client/c-files.c#L1588)
2. `X:`/`Y:` немедленно меняют core option value, Client_setup и вызывают `check_immediate_options(..., in_game)`; это потенциальные runtime/network/render effects. Outdated names конвертируются, а `.opt`/`options.prf` автоматически dump обратно. [c-files.c:1095](../../../src/client/c-files.c#L1095), [c-files.c:1536](../../../src/client/c-files.c#L1536), [c-files.c:1790](../../../src/client/c-files.c#L1790)
3. `.ins` conversion вызывает save; `.dna` conversion тоже вызывает save. Read-only import нельзя реализовать простым вызовом существующих loaders, даже если открытие было `r`. [c-files.c:3430](../../../src/client/c-files.c#L3430), [c-files.c:3673](../../../src/client/c-files.c#L3673)
4. SDL3 chat/bookmarks **save** через ANGBAND_DIR_USER/my_fopen (обычно user-root/user), но **load** напрямую из `os_temp_path`. Current source имеет асимметрию: нельзя обещать import «откуда baseline загружает» равным «куда baseline сохраняет». [c-init.c:3393](../../../src/client/c-init.c#L3393), [c-init.c:4485](../../../src/client/c-init.c#L4485)
5. `options_dump` сериализует все real options и W flags в один файл, делает direct rename `.bak`; SDL3 my_fopen redirection не перенаправляет rename. Macro dump отдельно строит SDL3 user path. Audio saver использует temp copy, `.bak`, remove. Нужны owners для каждой операции, не только fopen. [c-util.c:15297](../../../src/client/c-util.c#L15297), [c-util.c:5417](../../../src/client/c-util.c#L5417), [snd-sdl3.c:4180](../../../src/client/snd-sdl3.c#L4180)
6. Config writer работает через `.$$$` и replace исходного config; вызывается startup minimal creation, explicit save, credential save, pack changes, shutdown/layout save. Не переиспользовать writer как modern UI save без split. [client.c:691](../../../src/client/client.c#L691), [client.c:1031](../../../src/client/client.c#L1031), [c-init.c:3673](../../../src/client/c-init.c#L3673), [c-init.c:3480](../../../src/client/c-init.c#L3480), [main-sdl3.c:5058](../../../src/client/main-sdl3.c#L5058), [c-util.c:17115](../../../src/client/c-util.c#L17115), [c-util.c:19804](../../../src/client/c-util.c#L19804)

### Дополнительные schema/trigger детали

- Macro set `.meta` содержит пять строк: cyclic trigger (raw/human), cyclic action format, free-switch action format, comment; затем flag byte 0..3 (cyclic/free-switch). Stage `.meta`: free-switch trigger raw/human, action raw/human, comment, затем disabled marker. Это не JSON и не чистый newline text; сохраняются exact encodings. Scanner SDL3 читает user и bundled dirs с duplicate suppression, но metadata readers используют прямой `fopen` по ANGBAND_DIR_USER. [c-util.c:5623](../../../src/client/c-util.c#L5623), [c-util.c:5665](../../../src/client/c-util.c#L5665), [c-util.c:5750](../../../src/client/c-util.c#L5750)
- `.ins` текущий writer: header, comments, затем на entry match line, tag line, flags line `F|-`, `i|a|A|-`, `b|-`, `X|-` (force, ignore/pickup/destroy, bag-only, disabled). Пустые строки являются data. Loader может частично загрузить entry из повреждённого файла и считать файл успешным; import policy должна определить partial-result handling. [c-files.c:3018](../../../src/client/c-files.c#L3018), [c-files.c:3064](../../../src/client/c-files.c#L3064)
- Birth `.dna` записывается при завершении выбора рождения, а не только manual save. [c-birth.c:2344](../../../src/client/c-birth.c#L2344)
- SDL3 graphics cache — `.bmp` с tileset, output dimensions, version и fingerprints в имени; publish через уникальный соседний temporary BMP. Это производный cache, который можно заново создать, а не сохранённый пользовательский выбор. [main-sdl3.c:2666](../../../src/client/main-sdl3.c#L2666), [main-sdl3.c:2706](../../../src/client/main-sdl3.c#L2706)

### Config key sets

Полный набор, распознаваемый `read_mangrc` в данном source (compile gates остаются значимы):

- Identity/connection: `nick`, `pass`, `name`, `meta`, `server` (optional `:port`), `port`, `realname`, `path`, `fullauto`.
- Presentation/render: `fps`, `lighterDarkBlue`, `colormap_<0..BASE_PALETTE_SIZE-1>`, `graphicsForceOutline`, `graphic_resize_type`, `graphics`, `graphic_tiles`, `graphic_tiles<index>`, `disableGfxCache`, `windowDecorations`, `hintBigmap`.
- Audio: `sound`, `hintSound`, `cacheAudio`, `audioSampleRate`, `audioChannels`, `audioBuffer`, `soundpackFolder`, `musicpackFolder`, `audioMaster`, `audioMusic`, `audioSound`, `audioWeather`, `audioVolumeMaster`, `audioVolumeMusic`, `audioVolumeSound`, `audioVolumeWeather`.
- Term-specific: `Term-Main`, `Term-1`…`Term-9`, old Mainwindow/Mirrorwindow/Recallwindow/Choicewindow/Term-4window…Term-9window prefixes; suffixes `_Title`, `_Visible`, `_X`, `_Y`, `_Columns`, `_Lines`, `_Font`.

Это prefix-based parser, не строгая schema. **Особенность**: writer пишет `soundpackSubset`/`musicpackSubset`, но приведённый reader не содержит чтения этих keys; наличие записи не доказывает round-trip. [client.c:129](../../../src/client/client.c#L129), [client.c:69](../../../src/client/client.c#L69), [client.c:1132](../../../src/client/client.c#L1132)

Windows INI read-set из main-win:

- Base: `ForceIMEOff`, `ForceIMEOn`, `DisableGfxCache`, `LogFont`, `DisableNumlock`, `LighterDarkBlue`, `Colormap_<index>`, `Graphics`, `GraphicTiles`, `GraphicSubTiles<index>`, `Sound`, `HintSound`, `CacheAudio`, `SampleRate`, `MaxChannels`, `AudioBuffer`, `AudioMaster`, `SoundpackFolder`, `SoundpackSubset`, `MusicpackFolder`, `MusicpackSubset`, `AudioMusic`, `AudioSound`, `AudioWeather`, `AudioVolumeMaster`, `AudioVolumeMusic`, `AudioVolumeSound`, `AudioVolumeWeather`, `HintBigmap`, `fps`, `LibPath`, `DontMoveUser`.
- Online: `meta`, `nick`, `pass`, `port`; `server` read commented out.
- Window sections: `WindowNumber`, `Visible`, `WindowTitle`, `LogFont`, `Font`, `FontWid`, `FontHgt`, `FontWgt`, `FontAA`, `Columns`, `Rows`, `PositionX`, `PositionY`; Sound section event→wav entries legacy frontend.

Evidence: [main-win.c:1907](../../../src/client/main-win.c#L1907), [main-win.c:2003](../../../src/client/main-win.c#L2003), [main-win.c:2015](../../../src/client/main-win.c#L2015), [main-win.c:2125](../../../src/client/main-win.c#L2125), [main-win.c:5884](../../../src/client/main-win.c#L5884). Windows is compatibility context; SDL3 Windows uses CFG path, not this legacy INI UI.

### PRF schema inventory

| Opcode | Baseline meaning | Import consequence |
|---|---|---|
| `%` | Include file | Explicit dependency resolution and bounded path policy |
| `R`, `K`, `F`, `U` | Monster/item/feature/unaware glyph attributes and mappings | Resource/game visuals, preserve index and mapping offsets |
| `I` | Arbitrary graphics rectangle | Render resource parity |
| `r`, `@`, `Z` | Implementation disabled `#if 0` | Do not promise effective behavior |
| `E`, `V` | Accepted no-op | Preserve source; do not invent values |
| `A` | Action buffer | Data decode |
| `P`, `H`, `C` | Normal/hybrid/command trigger, backend key autoconversion | Preserve type/encoding and matching semantics |
| `D` | Delete macro | Ordered override semantics |
| `S` | Keymap | Current code parses all three fields from zz[0]; behavior vs intended format requires acceptance decision |
| `X`, `Y` | Named boolean option | Split gameplay/presentation through registry |
| `W` | Physical Term window flag | Legacy configuration excluded by map Notes |
| `#` | Loud formatted comment/message | Import side-effect policy |
| `!`, `?` | Execute macro action at load | Never silently treat as ordinary setting |
| `m` | SDL3 tile mask RGB | Graphics resource preference |

Full handler [c-files.c:1049](../../../src/client/c-files.c#L1049). Header/encoding decoder is `my_fgets2`, with fmt passed into `key_autoconvert`; basename `graphics-...#<index>` selects subtileset. [c-files.c:1697](../../../src/client/c-files.c#L1697)

### Credentials и server updates

Уточнение 2026-09-18: штатный SDL3 CFG хранит пароль открытым текстом, не в OS keyring и не в frob-представлении. Reader копирует `pass` без преобразования; login применяет обратимый `my_memfrob` (XOR 42 при применимом protocol), а writer повторно преобразует временную копию runtime password, возвращая исходный текст перед записью. После успешного login вызывается `store_crecedentials()`, который в SDL3 пишет общие credentials через `write_mangrc(true, true, false)`. Прежняя формулировка об обработанном пароле в config была неточной. Не считать такой файл обычным UI preference и не выводить пароль в presentation events/research. Импорт уже согласован; дальнейшая storage policy остаётся отдельным человеческим решением. [client.c:237](../../../src/client/client.c#L237), [c-init.c:3824](../../../src/client/c-init.c#L3824), [client.c:920](../../../src/client/client.c#L920), [client.c:1042](../../../src/client/client.c#L1042), [c-util.c:17959](../../../src/client/c-util.c#L17959), [nclient.c:821](../../../src/client/nclient.c#L821), [main-sdl3.c:5057](../../../src/client/main-sdl3.c#L5057). Это статическая проверка исходников, без чтения пользовательских секретов и без runtime evidence.

`Receive_file` реализует INIT/DATA/END/check/ACK transfer; local receiver отклоняет `/` prefix и `..`, принимает chunks в `tomexfer.XXXXXX`, затем пишет destination через `my_fopen(...,"wb")`. Это слабая filename проверка, не allow-list. Windows remaps `scpt/` и `user/`; Linux joins ANGBAND_DIR, SDL3 my_fopen затем применяет override. Checks идут по тому же path routing; old checksum и new MD5 branches существуют. [nclient.c:477](../../../src/client/nclient.c#L477), [files.c:128](../../../src/common/files.c#L128), [files.c:415](../../../src/common/files.c#L415), [files.c:477](../../../src/common/files.c#L477), [files.c:553](../../../src/common/files.c#L553)

Bundled server update table перечисляет конкретные `scpt/*.lua` и вызывает remote_update_lua; это source-backed отправляемый набор, **не доказательство**, что любой server payload допустим. [update.lua:1](../../../lib/scpt/update.lua#L1), [update.lua:55](../../../lib/scpt/update.lua#L55), [lua_bind.c:522](../../../src/server/lua_bind.c#L522). Уже согласованная presentation boundary оставляет allow-listed file controls в core; новую UI configuration и credentials не нужно включать в resource update namespace.

Manual installers и guide update — отдельные write flows: archive unpack меняет CWD, writes files into destination, Windows закрывает pack configs перед overwrite, helper `.bat`/temporary archives и checksum files не preference import. Исследование **не проверяло безопасность archive traversal и всех shell command quoting** и не утверждает её. [c-util.c:15410](../../../src/client/c-util.c#L15410), [c-util.c:16454](../../../src/client/c-util.c#L16454), [c-util.c:17215](../../../src/client/c-util.c#L17215), [c-files.c:3887](../../../src/client/c-files.c#L3887)

## Рекомендуемая классификация, не принятое решение

Класс назначается записи и owner operation, не extension. Подходящий registry row: `id`, `source format/key/opcode`, `source scope` (installation/backend/account/character/form/pack), `value type/default`, `core owner`, `import transforms`, `load side effects`, `modern write owner`, `legacy preservation`, `acceptance evidence`.

Кандидаты safe read-only import: gameplay named options, macro definitions/ordered deletes, auto-inscription records, birth defaults, audio overrides, graphic/font resources, bookmarks. «Safe» требует staged decode без execution, immediate apply, conversion resave и source writes; это рекомендация к import contract, а не свойство stock loaders.

Отдельная new UI configuration: single-window geometry, surface layouts и user-confirmed presentation preferences; возможны импорт palette/audio/font choices, но никакого механического преобразования десяти Term layouts. Самостоятельные files/schema/version/scope/commit protocol пока не выбраны. Existing core gameplay storage предпочтительно переиспользовать согласно Notes; persisted representation не требует дублировать runtime values.

Legacy artifacts: исходные configs/PRF/W records, unknown keys/opcodes, backups, generated caches, exports, old helper working files; сохранять источник не значит загружать его или запрещать человеку explicit export.

## Оставшиеся продуктовые решения

- One-time snapshot import или live overlay; приоритет bundled/legacy/modern и account/character/class/form; missing/unknown/corrupt/partial import, diagnostics, reimport и rollback.
- Какие presentation-related `option_info` values сохраняют смысл, какие должны стать semantic UI preferences, какие legacy Term/bar toggles лишь compatibility artifacts. Registry ниже даёт имена, но не подменяет это решение.
- Namespace и owner каждого modern write: общие gameplay/macro/ins files или отдельный profile; explicit export back to legacy; concurrency, backups, atomicity и retention.
- Credentials consent/import/storage policy; personal chat history/private notes import и сроки хранения.
- Что делать с `%`/`!`/`?`/`#` и disabled opcodes: reject, retain inert with report или explicit action после review; границы включаемых файлов и backend key conversion.
- Где действительно хранить histories/bookmarks: current SDL3 asymmetry требует осознанного выбора; guide bookmark anchoring после guide updates.
- Server file allow-list, resource overwrite/validation/update failure policy и отделение modern config от remote file destination.
- Audio/graphics resource ownership, pack-specific overrides, cache regeneration и platform import mapping.

## Полный статически извлечённый option-key registry

Ниже **имена** и источник; registry page — навигационная подсказка (исторически UI 1/4/6/7, audio 5/9, gameplay 2/3/8), **не authoritative partition**. Conditional duplicates оставлены только один раз; неактивные, hidden и compile-gated values требуют active-build inventory при реализации. Defaults/enabled для этих вариантов читаются в linked row. OPT_MAX=199 включает резерв/termination, это не 199 действующих keys. [defines.h:2482](../../../src/common/defines.h#L2482), [c-tables.c:186](../../../src/client/c-tables.c#L186)

| Key | Source |
|---|---|
| `rogue_like_commands` | [c-tables.c:189](../../../src/client/c-tables.c#L189) |
| `newbie_hints` | [c-tables.c:191](../../../src/client/c-tables.c#L191) |
| `censor_swearing` | [c-tables.c:193](../../../src/client/c-tables.c#L193) |
| `highlight_chat` | [c-tables.c:196](../../../src/client/c-tables.c#L196) |
| `highbeep_chat` | [c-tables.c:198](../../../src/client/c-tables.c#L198) |
| `page_on_privmsg` | [c-tables.c:200](../../../src/client/c-tables.c#L200) |
| `page_on_afk_privmsg` | [c-tables.c:202](../../../src/client/c-tables.c#L202) |
| `big_map` | [c-tables.c:207](../../../src/client/c-tables.c#L207) |
| `font_map_solid_walls` | [c-tables.c:214](../../../src/client/c-tables.c#L214) |
| `view_animated_light` | [c-tables.c:216](../../../src/client/c-tables.c#L216) |
| `wall_lighting` | [c-tables.c:218](../../../src/client/c-tables.c#L218) |
| `view_lamp_walls` | [c-tables.c:220](../../../src/client/c-tables.c#L220) |
| `view_shade_walls` | [c-tables.c:222](../../../src/client/c-tables.c#L222) |
| `floor_lighting` | [c-tables.c:224](../../../src/client/c-tables.c#L224) |
| `view_lamp_floor` | [c-tables.c:226](../../../src/client/c-tables.c#L226) |
| `view_shade_floor` | [c-tables.c:228](../../../src/client/c-tables.c#L228) |
| `view_light_extra` | [c-tables.c:230](../../../src/client/c-tables.c#L230) |
| `alert_hitpoint` | [c-tables.c:233](../../../src/client/c-tables.c#L233) |
| `alert_mana` | [c-tables.c:235](../../../src/client/c-tables.c#L235) |
| `alert_afk_dam` | [c-tables.c:237](../../../src/client/c-tables.c#L237) |
| `alert_offpanel_dam` | [c-tables.c:239](../../../src/client/c-tables.c#L239) |
| `exp_bar` | [c-tables.c:242](../../../src/client/c-tables.c#L242) |
| `uniques_alive` | [c-tables.c:245](../../../src/client/c-tables.c#L245) |
| `warn_unique_credit` | [c-tables.c:247](../../../src/client/c-tables.c#L247) |
| `limit_chat` | [c-tables.c:249](../../../src/client/c-tables.c#L249) |
| `no_afk_msg` | [c-tables.c:251](../../../src/client/c-tables.c#L251) |
| `overview_startup` | [c-tables.c:253](../../../src/client/c-tables.c#L253) |
| `allow_paging` | [c-tables.c:256](../../../src/client/c-tables.c#L256) |
| `ring_bell` | [c-tables.c:258](../../../src/client/c-tables.c#L258) |
| `linear_stats` | [c-tables.c:262](../../../src/client/c-tables.c#L262) |
| `exp_need` | [c-tables.c:264](../../../src/client/c-tables.c#L264) |
| `depth_in_feet` | [c-tables.c:266](../../../src/client/c-tables.c#L266) |
| `newb_suicide` | [c-tables.c:268](../../../src/client/c-tables.c#L268) |
| `show_weights` | [c-tables.c:270](../../../src/client/c-tables.c#L270) |
| `time_stamp_chat` | [c-tables.c:274](../../../src/client/c-tables.c#L274) |
| `hide_unusable_skills` | [c-tables.c:276](../../../src/client/c-tables.c#L276) |
| `short_item_names` | [c-tables.c:278](../../../src/client/c-tables.c#L278) |
| `keep_topline` | [c-tables.c:280](../../../src/client/c-tables.c#L280) |
| `target_history` | [c-tables.c:282](../../../src/client/c-tables.c#L282) |
| `taciturn_messages` | [c-tables.c:284](../../../src/client/c-tables.c#L284) |
| `always_show_lists` | [c-tables.c:286](../../../src/client/c-tables.c#L286) |
| `no_weather` | [c-tables.c:289](../../../src/client/c-tables.c#L289) |
| `player_list` | [c-tables.c:292](../../../src/client/c-tables.c#L292) |
| `player_list2` | [c-tables.c:294](../../../src/client/c-tables.c#L294) |
| `flash_player` | [c-tables.c:297](../../../src/client/c-tables.c#L297) |
| `highlight_player` | [c-tables.c:300](../../../src/client/c-tables.c#L300) |
| `basic_players_symb` | [c-tables.c:302](../../../src/client/c-tables.c#L302) |
| `subterm_flicker` | [c-tables.c:305](../../../src/client/c-tables.c#L305) |
| `no_verify_sell` | [c-tables.c:307](../../../src/client/c-tables.c#L307) |
| `no_verify_destroy` | [c-tables.c:309](../../../src/client/c-tables.c#L309) |
| `auto_afk` | [c-tables.c:312](../../../src/client/c-tables.c#L312) |
| `idle_starve_kick` | [c-tables.c:314](../../../src/client/c-tables.c#L314) |
| `safe_float` | [c-tables.c:316](../../../src/client/c-tables.c#L316) |
| `safe_macros` | [c-tables.c:318](../../../src/client/c-tables.c#L318) |
| `auto_untag` | [c-tables.c:321](../../../src/client/c-tables.c#L321) |
| `clear_inscr` | [c-tables.c:323](../../../src/client/c-tables.c#L323) |
| `auto_inscr_server` | [c-tables.c:325](../../../src/client/c-tables.c#L325) |
| `stack_force_notes` | [c-tables.c:327](../../../src/client/c-tables.c#L327) |
| `stack_force_costs` | [c-tables.c:329](../../../src/client/c-tables.c#L329) |
| `stack_allow_items` | [c-tables.c:331](../../../src/client/c-tables.c#L331) |
| `stack_allow_devices` | [c-tables.c:333](../../../src/client/c-tables.c#L333) |
| `whole_ammo_stack` | [c-tables.c:335](../../../src/client/c-tables.c#L335) |
| `always_repeat` | [c-tables.c:337](../../../src/client/c-tables.c#L337) |
| `always_pickup` | [c-tables.c:339](../../../src/client/c-tables.c#L339) |
| `use_old_target` | [c-tables.c:341](../../../src/client/c-tables.c#L341) |
| `autooff_retaliator` | [c-tables.c:343](../../../src/client/c-tables.c#L343) |
| `fail_no_melee` | [c-tables.c:345](../../../src/client/c-tables.c#L345) |
| `basic_players_col` | [c-tables.c:347](../../../src/client/c-tables.c#L347) |
| `auto_target` | [c-tables.c:349](../../../src/client/c-tables.c#L349) |
| `thin_down_flush` | [c-tables.c:351](../../../src/client/c-tables.c#L351) |
| `disable_flush` | [c-tables.c:353](../../../src/client/c-tables.c#L353) |
| `find_ignore_stairs` | [c-tables.c:356](../../../src/client/c-tables.c#L356) |
| `find_ignore_doors` | [c-tables.c:358](../../../src/client/c-tables.c#L358) |
| `find_cut` | [c-tables.c:360](../../../src/client/c-tables.c#L360) |
| `find_examine` | [c-tables.c:362](../../../src/client/c-tables.c#L362) |
| `disturb_move` | [c-tables.c:364](../../../src/client/c-tables.c#L364) |
| `disturb_near` | [c-tables.c:366](../../../src/client/c-tables.c#L366) |
| `disturb_panel` | [c-tables.c:368](../../../src/client/c-tables.c#L368) |
| `disturb_state` | [c-tables.c:370](../../../src/client/c-tables.c#L370) |
| `disturb_minor` | [c-tables.c:372](../../../src/client/c-tables.c#L372) |
| `disturb_other` | [c-tables.c:374](../../../src/client/c-tables.c#L374) |
| `view_perma_grids` | [c-tables.c:376](../../../src/client/c-tables.c#L376) |
| `view_torch_grids` | [c-tables.c:378](../../../src/client/c-tables.c#L378) |
| `view_reduce_light` | [c-tables.c:382](../../../src/client/c-tables.c#L382) |
| `view_reduce_view` | [c-tables.c:384](../../../src/client/c-tables.c#L384) |
| `easy_open` | [c-tables.c:387](../../../src/client/c-tables.c#L387) |
| `easy_disarm` | [c-tables.c:389](../../../src/client/c-tables.c#L389) |
| `easy_tunnel` | [c-tables.c:391](../../../src/client/c-tables.c#L391) |
| `audio_paging` | [c-tables.c:394](../../../src/client/c-tables.c#L394) |
| `paging_master_vol` | [c-tables.c:396](../../../src/client/c-tables.c#L396) |
| `paging_max_vol` | [c-tables.c:398](../../../src/client/c-tables.c#L398) |
| `no_ovl_close_sfx` | [c-tables.c:400](../../../src/client/c-tables.c#L400) |
| `ovl_sfx_attack` | [c-tables.c:402](../../../src/client/c-tables.c#L402) |
| `no_combat_sfx` | [c-tables.c:404](../../../src/client/c-tables.c#L404) |
| `no_magicattack_sfx` | [c-tables.c:406](../../../src/client/c-tables.c#L406) |
| `no_defense_sfx` | [c-tables.c:408](../../../src/client/c-tables.c#L408) |
| `half_sfx_attack` | [c-tables.c:410](../../../src/client/c-tables.c#L410) |
| `cut_sfx_attack` | [c-tables.c:412](../../../src/client/c-tables.c#L412) |
| `ovl_sfx_command` | [c-tables.c:414](../../../src/client/c-tables.c#L414) |
| `ovl_sfx_misc` | [c-tables.c:416](../../../src/client/c-tables.c#L416) |
| `ovl_sfx_mon_attack` | [c-tables.c:418](../../../src/client/c-tables.c#L418) |
| `ovl_sfx_mon_spell` | [c-tables.c:420](../../../src/client/c-tables.c#L420) |
| `ovl_sfx_mon_misc` | [c-tables.c:422](../../../src/client/c-tables.c#L422) |
| `no_monsterattack_sfx` | [c-tables.c:424](../../../src/client/c-tables.c#L424) |
| `positional_audio` | [c-tables.c:426](../../../src/client/c-tables.c#L426) |
| `no_house_sfx` | [c-tables.c:428](../../../src/client/c-tables.c#L428) |
| `quiet_house_sfx` | [c-tables.c:430](../../../src/client/c-tables.c#L430) |
| `mute_when_idle` | [c-tables.c:432](../../../src/client/c-tables.c#L432) |
| `alert_starvation` | [c-tables.c:434](../../../src/client/c-tables.c#L434) |
| `use_color` | [c-tables.c:439](../../../src/client/c-tables.c#L439) |
| `other_query_flag` | [c-tables.c:441](../../../src/client/c-tables.c#L441) |
| `quick_messages` | [c-tables.c:447](../../../src/client/c-tables.c#L447) |
| `carry_query_flag` | [c-tables.c:449](../../../src/client/c-tables.c#L449) |
| `show_labels` | [c-tables.c:451](../../../src/client/c-tables.c#L451) |
| `show_choices` | [c-tables.c:453](../../../src/client/c-tables.c#L453) |
| `show_details` | [c-tables.c:455](../../../src/client/c-tables.c#L455) |
| `expand_look` | [c-tables.c:457](../../../src/client/c-tables.c#L457) |
| `expand_list` | [c-tables.c:459](../../../src/client/c-tables.c#L459) |
| `avoid_other` | [c-tables.c:461](../../../src/client/c-tables.c#L461) |
| `flush_failure` | [c-tables.c:463](../../../src/client/c-tables.c#L463) |
| `flush_disturb` | [c-tables.c:465](../../../src/client/c-tables.c#L465) |
| `fresh_after` | [c-tables.c:467](../../../src/client/c-tables.c#L467) |
| `speak_unique` | [c-tables.c:470](../../../src/client/c-tables.c#L470) |
| `shuffle_music` | [c-tables.c:474](../../../src/client/c-tables.c#L474) |
| `permawalls_shade` | [c-tables.c:476](../../../src/client/c-tables.c#L476) |
| `topline_no_msg` | [c-tables.c:478](../../../src/client/c-tables.c#L478) |
| `targetinfo_msg` | [c-tables.c:480](../../../src/client/c-tables.c#L480) |
| `live_timeouts` | [c-tables.c:482](../../../src/client/c-tables.c#L482) |
| `flash_insane` | [c-tables.c:484](../../../src/client/c-tables.c#L484) |
| `last_words` | [c-tables.c:487](../../../src/client/c-tables.c#L487) |
| `disturb_see` | [c-tables.c:489](../../../src/client/c-tables.c#L489) |
| `diz_unique` | [c-tables.c:492](../../../src/client/c-tables.c#L492) |
| `diz_death` | [c-tables.c:494](../../../src/client/c-tables.c#L494) |
| `diz_death_any` | [c-tables.c:496](../../../src/client/c-tables.c#L496) |
| `diz_first` | [c-tables.c:498](../../../src/client/c-tables.c#L498) |
| `screenshot_format` | [c-tables.c:500](../../../src/client/c-tables.c#L500) |
| `palette_animation` | [c-tables.c:502](../../../src/client/c-tables.c#L502) |
| `play_all` | [c-tables.c:504](../../../src/client/c-tables.c#L504) |
| `id_selection` | [c-tables.c:506](../../../src/client/c-tables.c#L506) |
| `hp_bar` | [c-tables.c:508](../../../src/client/c-tables.c#L508) |
| `mp_bar` | [c-tables.c:510](../../../src/client/c-tables.c#L510) |
| `st_bar` | [c-tables.c:512](../../../src/client/c-tables.c#L512) |
| `find_ignore_montraps` | [c-tables.c:515](../../../src/client/c-tables.c#L515) |
| `quiet_os` | [c-tables.c:518](../../../src/client/c-tables.c#L518) |
| `disable_lightning` | [c-tables.c:520](../../../src/client/c-tables.c#L520) |
| `macros_in_stores` | [c-tables.c:522](../../../src/client/c-tables.c#L522) |
| `item_error_beep` | [c-tables.c:524](../../../src/client/c-tables.c#L524) |
| `keep_bottle` | [c-tables.c:526](../../../src/client/c-tables.c#L526) |
| `easy_disarm_montraps` | [c-tables.c:529](../../../src/client/c-tables.c#L529) |
| `no_house_magic` | [c-tables.c:531](../../../src/client/c-tables.c#L531) |
| `no_light_fainting` | [c-tables.c:533](../../../src/client/c-tables.c#L533) |
| `auto_pickup` | [c-tables.c:536](../../../src/client/c-tables.c#L536) |
| `auto_destroy` | [c-tables.c:538](../../../src/client/c-tables.c#L538) |
| `destroy_all_unmatched` | [c-tables.c:540](../../../src/client/c-tables.c#L540) |
| `mp_huge_bar` | [c-tables.c:543](../../../src/client/c-tables.c#L543) |
| `sn_huge_bar` | [c-tables.c:545](../../../src/client/c-tables.c#L545) |
| `hp_huge_bar` | [c-tables.c:547](../../../src/client/c-tables.c#L547) |
| `clone_to_stdout` | [c-tables.c:550](../../../src/client/c-tables.c#L550) |
| `clone_to_file` | [c-tables.c:552](../../../src/client/c-tables.c#L552) |
| `first_song` | [c-tables.c:554](../../../src/client/c-tables.c#L554) |
| `equip_text_colour` | [c-tables.c:556](../../../src/client/c-tables.c#L556) |
| `equip_set_colour` | [c-tables.c:558](../../../src/client/c-tables.c#L558) |
| `colourize_bignum` | [c-tables.c:560](../../../src/client/c-tables.c#L560) |
| `flash_player2` | [c-tables.c:563](../../../src/client/c-tables.c#L563) |
| `load_form_macros` | [c-tables.c:565](../../../src/client/c-tables.c#L565) |
| `auto_inscr_off` | [c-tables.c:567](../../../src/client/c-tables.c#L567) |
| `ascii_feats` | [c-tables.c:570](../../../src/client/c-tables.c#L570) |
| `ascii_items` | [c-tables.c:572](../../../src/client/c-tables.c#L572) |
| `ascii_monsters` | [c-tables.c:574](../../../src/client/c-tables.c#L574) |
| `ascii_uniques` | [c-tables.c:576](../../../src/client/c-tables.c#L576) |
| `no_flicker` | [c-tables.c:579](../../../src/client/c-tables.c#L579) |
| `stun_huge_bar` | [c-tables.c:581](../../../src/client/c-tables.c#L581) |
| `gfx_autooff_fmsw` | [c-tables.c:585](../../../src/client/c-tables.c#L585) |
| `solid_bars` | [c-tables.c:588](../../../src/client/c-tables.c#L588) |
| `autopickup_chemicals` | [c-tables.c:590](../../../src/client/c-tables.c#L590) |
| `add_kind_diz` | [c-tables.c:592](../../../src/client/c-tables.c#L592) |
| `hide_lore_paste` | [c-tables.c:594](../../../src/client/c-tables.c#L594) |
| `new_retaliator` | [c-tables.c:597](../../../src/client/c-tables.c#L597) |
| `sunburn_msg` | [c-tables.c:599](../../../src/client/c-tables.c#L599) |
| `wide_scroll_margin` | [c-tables.c:601](../../../src/client/c-tables.c#L601) |
| `gfx_palanim_repaint` | [c-tables.c:603](../../../src/client/c-tables.c#L603) |
| `gfx_hack_repaint` | [c-tables.c:605](../../../src/client/c-tables.c#L605) |
| `topline_first` | [c-tables.c:607](../../../src/client/c-tables.c#L607) |
| `ascii_weather` | [c-tables.c:609](../../../src/client/c-tables.c#L609) |
| `no2mask_weather` | [c-tables.c:611](../../../src/client/c-tables.c#L611) |
| `auto_inscr_server_ch` | [c-tables.c:613](../../../src/client/c-tables.c#L613) |
| `screenshot_keys` | [c-tables.c:616](../../../src/client/c-tables.c#L616) |
| `wild_resume_from_any` | [c-tables.c:618](../../../src/client/c-tables.c#L618) |
| `tavern_town_resume` | [c-tables.c:620](../../../src/client/c-tables.c#L620) |
| `st_huge_bar` | [c-tables.c:622](../../../src/client/c-tables.c#L622) |
| `huge_bars_gfx` | [c-tables.c:624](../../../src/client/c-tables.c#L624) |
| `autoinsc_debug` | [c-tables.c:627](../../../src/client/c-tables.c#L627) |
| `autoloot_dunonly` | [c-tables.c:629](../../../src/client/c-tables.c#L629) |
| `autoloot_dununown` | [c-tables.c:631](../../../src/client/c-tables.c#L631) |
| `flash_starvation` | [c-tables.c:633](../../../src/client/c-tables.c#L633) |
| `autoswitch_inven` | [c-tables.c:635](../../../src/client/c-tables.c#L635) |
| `show_newest` | [c-tables.c:637](../../../src/client/c-tables.c#L637) |
| `log_music` | [c-tables.c:639](../../../src/client/c-tables.c#L639) |
| `misc_no_flicker` | [c-tables.c:641](../../../src/client/c-tables.c#L641) |
| `prefer_subinven` | [c-tables.c:643](../../../src/client/c-tables.c#L643) |

Извлечено 200 уникальных имен; это lexical inventory, не подтверждённое число enabled options в сборке.

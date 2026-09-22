# Single-window client behavior baseline

## Research question

What observable behavior must a new SDL3 client preserve to claim 100% parity
with the current X11/SDL3 clients while using exactly one system `SDL_Window` and
its own semantic panels, tabs, dialogs, and overlays rather than emulating the
legacy `Term` layout?

This report audits repository HEAD `4211671279ff820575c32239272bca68cf8762f7`.
Sources are limited to tracked C code, headers, build files and documentation in
this worktree, plus the parallel read-only HTML prototype at
`/home/svechnik/Projects/github_site/tomenet_interface`. The prototype is a UX
source, not evidence of native parity: its own scope excludes C integration,
networking, `Receive_*`, `Term_keypress()` and real game-state changes
(`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:3-10`).

## Answer

The baseline is the **union of externally observable X11 and SDL3 behavior**, not
the ten legacy terminal objects. Replacing their OS-window topology is safe only
if the new client preserves the entire input language, protocol-driven state,
game and pre-game flows, informational surfaces, rendering/audio behavior,
configuration and file operations, conditional features, failure transitions,
and Linux/Windows builds.

A capability is not complete merely because its window exists. It is complete
when physical keys (normal and roguelike), macro/raw-command entry, input-context
priority, prompts, cancel/confirm behavior, outgoing intent, incoming result and
error, focus restoration, fallback rendering, and persistence all agree with the
baseline. The prototype's Definition of Done already captures most of that shape
(`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:685-692`).

## Existing seam and the single-window boundary

The SDL3 executable already shares almost everything below presentation:
`main-sdl3.c` is linked with the common terminal, commands, inventory, files,
stores, initialization, network, birth, UI, spells, skills, SDL3 audio, Lua and
graphics modules (`src/makefile.sdl3:48-69`). This is the reuse boundary: retain
those shared subsystems initially and replace the frontend incrementally.

`struct term` is currently both adapter and UI state. It owns a key queue,
dimensions, displayed/requested/saved cell buffers, and hooks for lifecycle,
events, cursor, wipe, text, tile, raw-picture and two-mask drawing
(`src/client/z-term.h:146-208`). It also holds up to four stacked saved "icky"
screens (`src/client/z-term.h:182-192`; limit at `src/common/defines.h:343-345`).
Removing `Term` as the destination UI model therefore requires either a temporary
adapter or conversion of every direct drawing/input caller to semantic state.

The current SDL3 topology is unambiguously multi-window: it creates a resizable,
high-DPI `SDL_Window` for each initialized term (`src/client/main-sdl3.c:307-340`),
initializes visible terms 1..9 and then mandatory term 0
(`src/client/main-sdl3.c:4564-4577`), routes events by `windowID`
(`src/client/main-sdl3.c:1375-1391`), and destroys/recreates auxiliary windows
when visibility changes (`src/client/main-sdl3.c:4984-5024`). The new frontend's
hard invariant should be one successful `SDL_CreateWindow` call and one live
top-level `SDL_Window`; internal views are not `Term` emulation.

## Verifiable capability inventory

The following IDs are proposed seed rows for a machine-readable manifest. Each
row cites the primary source that proves the capability family exists.

| ID | Capability family | Required observable baseline | Primary evidence |
|---|---|---|---|
| LIFE-01 | Startup/config/CLI | Defaults, config selection, server/account/password/character arguments, port/FPS/path and reincarnation overrides | `src/client/client.c:1376-1417`, `src/client/client.c:1475-1490` |
| LIFE-02 | Metaserver/connect | Server selection or explicit host, TCP contact, verification/setup/login, errors/bans/retry | `src/client/c-init.c:3731-3766`, `src/client/c-init.c:3830-3844`, `src/client/c-init.c:4114-4383` |
| LIFE-03 | Account/characters | Server flags, account response, character list/reordering, ordinary/exclusive creation, enter game | `src/client/nclient.c:762-845`, `src/client/c-init.c:4432-4451`, `src/client/nclient.c:1590-1672` |
| LIFE-04 | MOTD/play/relogin/quit | MOTD, input loop, server/client relogin, cleanup and preference save | `src/client/c-init.c:4460-4468`, `src/client/c-init.c:4530-4600`, `src/client/c-init.c:3479-3499` |
| IN-01 | Physical keyboard | Printable text, F1-F12, navigation, keypad, locks, modifiers and synthetic macro sequences | `src/client/main-sdl3.c:1035-1122`, `src/client/main-sdl3.c:1170-1306` |
| IN-02 | X11 key compatibility | X key symbols/keycodes and modifier-bearing fake sequences plus real text payload | `src/client/main-x11.c:1378-1477` |
| IN-03 | Keymaps/macros/raw keys | Normal/roguelike mapping, mutable command/hybrid/normal macros, raw fallback | `src/client/c-util.c:1892-1925`, `src/client/c-util.c:7549-7613`, `src/client/c-cmd.c:506-507` |
| CMD-01 | Movement/environment | walk/run/stay/rest/search, map/locate, stairs, tunnel, doors, bash/disarm | `src/client/c-cmd.c:291-329` |
| CMD-02 | Items | inventory/equipment, drop/gold, wear/remove/swap, destroy/stack, inscriptions, devices/consumables | `src/client/c-cmd.c:331-359` |
| CMD-03 | Combat/magic/target | fire/throw, browse, skills/abilities/ghost, hostile/friendly target, look | `src/client/c-cmd.c:361-379` |
| CMD-04 | Information/social | character, knowledge, uniques, player equipment/list, scores, help, chat and party | `src/client/c-cmd.c:380-418` |
| CMD-05 | Utilities/privileged | DM/admin, histories, quit/redraw/suicide, options, prefs, macros, auto-inscriptions, houses, screenshots, lagometer and audio | `src/client/c-cmd.c:422-506` |
| PROMPT-01 | Local prompts | item source/item, direction/target, amount, text editing, confirmation, repeat and context-local navigation | `/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:195-229`, `/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:482-496` |
| PROMPT-02 | Server prompts | key, amount, bounded number, string, confirmation and asynchronous abort | `src/client/nclient.c:7176-7253` |
| HUD-01 | Character vitals | stats, HP/MP/stamina/sanity/AC, XP/gold, title/depth/history and ailments | `src/client/nclient.c:353-380` |
| HUD-02 | Live status | target health, AFK, 15 encumbrance conditions, extra status, ping, weather, indicators, screen flash | `src/client/nclient.c:402-419`, `src/client/nclient.c:462-470`, `src/client/nclient.c:6217-6231`, `src/client/nclient.c:6321-6369` |
| DATA-01 | Inventory/equipment/bags | Slots, color, type, subtype, weight/count/value, artifact, identification, directional/trade/set data and names | `src/client/nclient.c:2317-2405`, `src/client/nclient.c:2649-2725` |
| DATA-02 | Skills/magic | skill tree/value updates, spell/power/technique requests and activation parameters | `src/client/nclient.c:381-406`, `src/client/nclient.c:3941-4012`, `src/client/nclient.c:6150-6192` |
| DATA-03 | Messages/maps | message history/chat, line/map streams, mini-map positions, target and floor data | `src/client/nclient.c:366-395`, `src/client/nclient.c:437-460` |
| SOCIAL-01 | Social state/actions | player list, party stats/info, guild/config, party/guild action requests | `src/client/nclient.c:396-400`, `src/client/nclient.c:5972-6064`, `src/client/nclient.c:7371-7395`, `src/client/nclient.c:8009-8029` |
| STORE-01 | Server-driven UI | normal/wide/special store rows, actions, animation, sell/leave and generic requests | `src/client/nclient.c:387-409`, `src/client/nclient.c:445-448`, `src/client/nclient.c:4510-5515` |
| SUBWIN-01 | Legacy information | inventory, equipment, character, non-chat/all/chat messages, clone map, lagometer, players, bonuses and bags | `src/client/c-tables.c:142-154` |
| RENDER-01 | Cell/text/cursor | cell buffers, text/wipe/cursor/redraw/clear and event/flush/delay contract | `src/client/z-term.h:195-248`, `src/client/main-sdl3.c:1690-1724` |
| RENDER-02 | Tiles/pictures | tile, two-mask and arbitrary raw pictures; partial tilesets and cache lifecycle | `src/client/z-term.h:200-207`, `src/client/main-sdl3.c:3678-3689`, `src/client/main-sdl3.c:4359-4434` |
| RENDER-03 | Palette/effects | palette updates, animation/light/weather/screen flash and target-health updates behind overlays | `src/client/nclient.c:451-462`, `src/client/nclient.c:6217-6231`, `src/client/nclient.c:6477-6519` |
| RENDER-04 | Resize/scale | pixel-to-grid resize, big-map negotiation, immediate redraw/debounce and high-DPI font reload | `src/client/main-sdl3.c:1407-1454`, `src/client/main-sdl3.c:4847-4912` |
| AUDIO-01 | Sound system | SFX, music, ambient/weather, paging, mixer, packs, jukebox, cache and live reinit | `src/client/c-init.c:3502-3548`, `src/client/client.c:415-533`, `src/client/c-util.c:16934-16970` |
| FILE-01 | Preference/config | config import/write, gameplay options, macros, auto-inscriptions, palette/graphics/audio and separate modern UI state | `src/client/client.c:68-205`, `src/client/client.c:622-691`, `src/client/c-tables.c:187-390` |
| FILE-02 | Server file transfer | init/data/end/checksum/ack/error, local writes and Lua/guide/audio reinitialization | `src/client/nclient.c:475-648` |
| FILE-03 | Screenshots/dumps | XHTML grid capture, composed SDL PNG/BMP capture and character dump | `src/client/c-files.c:2562-2762`, `src/client/main-sdl3.c:5310-5332`, `src/client/nclient.c:6234-6280` |
| FILE-04 | Clipboard | message/line copy, color stripping, URL extraction and platform-native paste behavior | `src/client/c-util.c:2073-2177`, `src/client/c-util.c:2177-2509` |
| NET-01 | Packet lifecycle | receive dispatch, version-sensitive decoding, queue/read/write buffers, flush, close and disconnect | `src/client/nclient.c:337-471`, `src/client/nclient.c:1479-1588`, `src/client/nclient.c:1904-2071` |
| NET-02 | Outgoing intents | all movement/item/target/store/social/admin/account/request/setup/audio/font packets | `src/client/nclient.c:7529-8239`, `src/client/nclient.c:8987-9204` |
| UI-01 | Single-window composition | HUD plus primary/context/dialog/target/system/technical layers, input priority and focus return | `/home/svechnik/Projects/github_site/tomenet_interface/README.md:101-125` |
| UI-02 | Full surface graph | character, magic, items, map/targets, social, knowledge, stores, settings, session and prompts | `/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:32-199` |
| PLATFORM-01 | Linux/Windows SDL3 | production/test builds for native Linux and MinGW Windows | `src/makefile.sdl3:321-346` |

This table is complete at the **capability-family level**, but not yet an atomic
row for every key, option, packet field, special-file type or store action. Those
atomic inventories are follow-up work identified below.

## Detailed findings

### Pre-game, failure, and session behavior

The native baseline starts before the map. It includes metaserver refresh and
ping, manual host/port, credential validation, account errors, server feature
flags, character rows and reordering, dedicated PvP/IDDC slots, creation, MOTD,
death/tombstone, relogin/character switching, server-requested quit and clean
shutdown. `Receive_login()` explicitly handles server rejection and retry state
before parsing server flags (`src/client/nclient.c:786-816`). Network login has a
five-second reply timeout and separate account-list and selected-character
requests (`src/client/nclient.c:1590-1635`). These transitions need system-level
overlays/dialogs; they cannot be represented as ordinary in-game tabs.

The HTML login prototype is normative only for approved presentation. It models
Server -> Account -> Character and a Name -> Sex -> Class -> Race -> Trait ->
Body -> Attributes -> Mode -> Review wizard, but its network, ping, character
creation and lists are local fixtures and are not persisted
(`/home/svechnik/Projects/github_site/tomenet_interface/README.md:43-65`). It also
documents an important pre-login data boundary: the character list has
level/name/race/class/mode/location, not glyph/appearance/winner/king/realm
(`/home/svechnik/Projects/github_site/tomenet_interface/README.md:67-78`).

### Input, macros, and local state machines

SDL3 and X11 converge on a byte-oriented key queue, including encoded navigation
keys and modifiers. Therefore GUI actions must feed the same action/command layer
as keyboard input, while physical-key parity must be tested before action
dispatch. A shortcut-only button map would miss arbitrary user macros, shipped
macro overrides and server/build-specific raw keys. The prototype explicitly
records shipped overrides for comma, semicolon, X, bracket and F-keys and notes
that user macros can replace almost any key
(`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:643-655`).

The baseline includes normal and roguelike modes. The prototype currently omits
roguelike, slash commands, login/creation, DM/admin/debug and OS-window controls
(`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:15-18`); only
OS-window controls are excluded by the single-window destination. Text input is
also more than native text fields: history traversal, beginning/end and word
movement, deletion, clipboard, chat-mode switching and searches remain gaps
(`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:482-496`).

### Protocol and semantic state

The receive dispatch table (`src/client/nclient.c:337-471`) is the lower bound on
state coverage. Many packets already provide structured semantic fields and
should update a central view model. Inventory, for example, includes slot,
color, weight/count, type/subtype, value, artifact id, direction/identification/
trade bits and display name (`src/client/nclient.c:2317-2405`); encumbrance
provides fifteen separate conditions (`src/client/nclient.c:6321-6369`).

Compatibility is version-sensitive: inventory and equipment have several wire
formats (`src/client/nclient.c:2328-2340`, `src/client/nclient.c:2660-2672`), and
character info decodes version-dependent fields and legacy mode bits
(`src/client/nclient.c:2820-2865`). Keeping the current decoders is safer than
duplicating this logic in presentation code.

Not all state is semantic. Special lines, stores and player-list rows include
formatted/color-coded data. The prototype's primary-source-backed constraint log
records that `PKT_PLAYERLIST` exposes a stable name plus formatted `playerinfo`,
not reliable columns or action capabilities; the agreed adapter keeps raw text
when parsing fails
(`/home/svechnik/Projects/github_site/tomenet_interface/SERVER_CONSTRAINTS.md:43-69`).
The permanent view model therefore needs typed values **and** a lossless raw
representation. Minor additive server changes can introduce structured variants,
but old packet forms and old clients must remain valid.

Main-window geometry is also protocol state. SDL3 converts pixels into cells,
updates `screen_wid/screen_hgt`, toggles big-map and sends dimensions back to the
server (`src/client/main-sdl3.c:4847-4906`). A fluid single-window layout still
needs a stable grid viewport whose dimensions are explicitly negotiated.

### Information surfaces and window replacement

Legacy supports ten terminals (`src/common/defines.h:335-341`) and 11 assignable
information classes (`src/client/c-tables.c:142-154`). Default auxiliary names
are Msg/Chat, Inventory, Character, Chat, Equipment, Bags and Term-7..9
(`src/client/c-tables.c:76-90`). Parity means every information class remains
available, live and configurable in the modern composition; it does not mean
preserving arbitrary `window_flag` assignment, titles or OS coordinates.

The main term carries far more than the dungeon: news, birth, tombstone, high
scores, macros, colors, visuals, options, help/file dumps, character, minimap,
messages and stores (`src/client/c-tables.c:92-104`). The prototype's complete
surface graph is the best high-level taxonomy
(`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:32-199`). Its
composition model—one primary, nested context/dialog/target, independent system
and technical layers with focus restoration—is suitable for the new frontend
(`/home/svechnik/Projects/github_site/tomenet_interface/README.md:101-113`).

### Rendering, graphics, animation, and resize

Text mode, cursor behavior and raw terminal fallback remain release requirements,
not merely migration aids. Graphics adds foreground/background two-mask
composition, arbitrary-size raw pictures, partial subtilesets, palette-sensitive
caches and live reload (`src/client/main-sdl3.c:4359-4434`). When graphics cannot
load, SDL3 either reports failure or falls back to text
(`src/client/main-sdl3.c:4547-4561`).

Fonts are PCF or monospaced TTF with per-term fallback today
(`src/client/main-sdl3.c:3715-3749`). SDL3 reloads on display-scale changes and
immediately redraws while debouncing a resize (`src/client/main-sdl3.c:1407-1454`).
The new client may replace per-term font settings with view-level typography, but
must preserve cell metrics, legible glyph fallback, high DPI and map accuracy.

Animations can update under "icky" screens: target health explicitly switches to
the base screen and back (`src/client/nclient.c:6217-6231`), while weather carries
server-controlled type, wind/intensity/speed/position and cloud geometry
(`src/client/nclient.c:6477-6519`). Thus menus do not pause online play, matching
the prototype rule that HUD/map/messages remain live beneath menus
(`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:17-30`).

### Audio, files, configuration, and side effects

Audio covers SFX, music, ambient/weather, paging, separate channel volumes,
mixing, caching, pack selection/install/reload and jukebox controls. Startup loads
Lua-defined indices and applies mixer state (`src/client/c-init.c:3502-3548`).
The build enables SDL3_mixer by default and conditionally adds libarchive for
pack extraction (`src/makefile.sdl3:33-40`, `src/makefile.sdl3:186-199`).

File transfer is gameplay infrastructure, not UI garnish: the client accepts
init/data/end/checksum/ack/error operations, writes local updates, reopens Lua and
refreshes guide metadata, with explicit access/write errors
(`src/client/nclient.c:475-648`). Guide/audio updates and pack installation need
progress/error UI without changing their underlying behavior.

Screenshots comprise two products: logical XHTML game-grid capture and real
composed-window image capture. SDL3 saves PNG with SDL3_image, else BMP
(`src/client/main-sdl3.c:5310-5332`); shared logic switches XHTML/image behavior
using modifier and option state (`src/client/c-files.c:2731-2762`). The new spec
must say whether image capture includes all panels/overlays (recommended) while
XHTML remains a game-grid export.

Clipboard is implemented for Windows, X11 and SDL3 in history/text flows,
including color stripping, double-copy URL extraction and paste behavior
(`src/client/c-util.c:2073-2177`, `src/client/c-util.c:2177-2509`). SDL3 uses
`SDL_SetClipboardText()` and `SDL_GetClipboardText()`, so the new frontend should
preserve that native SDL path rather than introduce a new platform abstraction.

Current SDL config lives under the SDL preference path as `tomenet.cfg`
(`src/client/client.c:143-158`). Existing term settings mix title, visibility,
position, dimensions and font (`src/client/client.c:87-125`), and shutdown saves
live window layout before preferences (`src/client/c-init.c:3479-3499`). The new
client should read/import compatible gameplay, macros, auto-inscriptions, audio,
palette and graphics state; ignore old auxiliary-window topology; write its UI
layout separately; and never overwrite legacy client config during import.

Options are dynamic data, not a short fixed settings page. `option_info` includes
enabled/default/page/key/description fields and already spans keyset, messages,
map visuals, item formatting, weather, rendering, safety, repeat/targeting and
disturbance behavior (`src/client/c-tables.c:187-390`). The options hub exposes ten
pages plus save/load, big-map, mixer/packs/jukebox, fonts, tilesets, palette,
account, pack install and guide update (`src/client/c-util.c:16894-16975`).

## X11 versus SDL3 discrepancy matrix

| Area | X11 baseline | SDL3 baseline | Requirement for new client |
|---|---|---|---|
| Window/event model | Explicit inner/outer X windows for terms; maps events by each window (`src/client/main-x11.c:1594-1727`) | One SDL window per visible term, routed by ID (`src/client/main-sdl3.c:1375-1391`) | Exactly one SDL window and internal hit/focus routing |
| Keyboard | `XLookupString`, X KeySym/KeyCode synthetic sequences (`src/client/main-x11.c:1392-1477`) | SDL scancode/keycode mappings designed for compatible sequences (`src/client/main-sdl3.c:1035-1306`) | Golden tests must compare logical byte streams and macro results |
| Mouse | Button/motion branches exist but are deliberately unimplemented (`src/client/main-x11.c:1737-1783`) | Current event switch handles keyboard/window events, not gameplay mouse (`src/client/main-sdl3.c:1396-1457`) | Add prototype mouse-first routes as alternate dispatch to the same intents |
| Window placement | Native X move/resize/position semantics (`src/client/main-x11.c:1845-1867`) | SDL prefers X11 over Wayland because Wayland forbids absolute top-level placement (`src/client/main-sdl3.c:4447-4453`) | Remove placement dependency with one top-level window |
| Font/DPI | X font loading and fixed cell metrics (`src/client/main-x11.c:847-910`) | TTF/PCF and high-DPI scale reload (`src/client/main-sdl3.c:3715-3749`, `src/client/main-sdl3.c:1444-1454`) | Preserve SDL3 PCF/TTF, fallback and DPI behavior |
| Graphics | XImage/Pixmap tiles, masks and caches (`src/client/main-x11.c:2380-2745`) | SDL surfaces, layers, masks, caches and runtime reload (`src/client/main-sdl3.c:4359-4434`) | Reuse SDL3 graphics semantics, render into composed viewport |
| Beep | X server beep (`src/client/main-x11.c:2085-2110`) | ASCII bell fallback when audio unavailable (`src/client/main-sdl3.c:1690-1720`) | Specify audible/visible fallback consistently |
| Clipboard | Uses external `xclip` path (`src/client/c-util.c:2257-2315`) | Uses SDL native set/get clipboard APIs (`src/client/c-util.c:2335-2381`, `src/client/c-util.c:2501-2509`) | Preserve SDL clipboard copy/paste and shared text normalization |
| Screenshot | X11 may use external ImageMagick/browser paths (`src/client/main-x11.c:607-611`, `src/client/c-cmd.c:7014-7040`) | Direct main-window PNG/BMP save (`src/client/main-sdl3.c:5310-5332`) | Capture composed single window plus retain XHTML export |

## Build/platform/conditional surface matrix

The SDL3 makefile defines native Linux and MinGW production and test targets
(`src/makefile.sdl3:321-346`). Both require `USE_SDL3`; both enable SDL3 sound;
optional dependencies alter visible capabilities:

| Condition | Surface/behavior | Evidence |
|---|---|---|
| `SOUND_SDL3` / audio unavailable | Mixer, SFX/music/weather/paging versus explicit unused-key message/fallback beep | `src/makefile.sdl3:35-36`, `src/client/c-cmd.c:468-483` |
| `SDL3_IMAGE` absent | Real screenshots are BMP rather than PNG | `src/makefile.sdl3:37`, `src/client/main-sdl3.c:5320-5326` |
| `SDL3_ARCHIVE` absent | In-client audio archive extraction unavailable | `src/makefile.sdl3:38`, `src/makefile.sdl3:186-199` |
| `SDL3_CURL_SSL` absent | Built-in guide checks/download lose curl/OpenSSL path | `src/makefile.sdl3:39`, `src/makefile.sdl3:203-231` |
| `SDL3_STICKY_KEYS` | One-shot forced Ctrl/Shift/Alt semantics | `src/client/main-sdl3.c:1059-1063`, `src/client/main-sdl3.c:1186-1218` |
| `USE_GRAPHICS` / server lacks graphics | tileset/palette UI and renderer versus text fallback/server warning | `src/client/main-sdl3.c:4547-4561`, `src/client/c-init.c:4656-4678` |
| `GRAPHICS_BG_MASK` | two-layer terrain/entity tile composition | `src/client/z-term.h:187-207` |
| `ENABLE_SUBINVEN` | bag packets, UI, moves and extra refresh flags | `src/client/nclient.c:2410-2498`, `src/common/defines.h:5678-5684` |
| `RETRY_LOGIN` | recoverable account/character switching and server relogin | `src/client/c-init.c:4438-4451`, `src/client/c-init.c:4540-4568` |
| server version/flags | packet fields, big-map, modes, stores and actions vary | `src/client/nclient.c:808-844`, `src/client/nclient.c:2328-2340` |
| admin/DM flags | privileged `]` surface available only to authorized characters | `src/client/c-cmd.c:422-433`, `src/client/nclient.c:2845-2864` |
| Linux optional GCU | `-c` terminal mode and GUI-init fallback, not part of single-window SDL mode | `src/makefile.sdl3:143-157` |

Controller support is deliberately out of scope and must not be placed in this
matrix as fog or future parity work.

## HTML prototype gap analysis

What the prototype already establishes well:

- a coherent surface graph and internal window hierarchy
  (`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:32-199`);
- non-pausing gameplay, input priority, layered system states and focus return
  (`/home/svechnik/Projects/github_site/tomenet_interface/README.md:101-125`);
- semantic panels for HUD, messages, inventory/equipment/bags, character, skills,
  abilities, several magic/technique flows, map/look, players and item actions
  (`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:223-510`);
- an unusually useful hotkey ledger that distinguishes complete, partial and
  absent visual flows (`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:7-18`).

Material gaps against native 100% parity:

- no real client core/network/protocol/state mutation by design
  (`/home/svechnik/Projects/github_site/tomenet_interface/README.md:80-89`);
- no roguelike keyset, slash commands, login hotkey inventory, DM/admin/debug or
  arbitrary macro language (`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:15-18`,
  `/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:643-655`);
- incomplete target/current-target/direction/repeat state machines
  (`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:211-229`);
- incomplete text editor/history/clipboard behavior
  (`/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:469-496`);
- missing equipment-of-player, party/guild/war, BBS, notes and most Knowledge
  (`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:492-537`);
- missing stores, homes and generic server/NPC screens
  (`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:540-550`);
- missing native settings, window-to-panel migration, audio tools, macro editor,
  auto-inscriptions, preference load, screenshots, quit/suicide and dangerous
  confirmation flows (`/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:552-591`);
- local fixtures lack network timing, partial packets, version variation,
  disconnect/relogin, file updates, graphics/audio failures and platform fallbacks.

Therefore `[x]` in HTML means approved visual interaction, not parity. The native
manifest should become canonical and automatically check the HTML ledgers, while
the HTML remains normative for approved layout/interaction decisions.

## Manifest schema and parity harness

Each atomic capability row should contain:

- stable id/name and phase (`meta`, `account`, `character`, `birth`, `play`,
  `death`, `relogin`, `shutdown`);
- input contexts; physical keys in normal/roguelike modes; macro/raw entry;
- prompt state machine, cancel/confirm, focus return, gameplay/chat blocking;
- structured fields with packet/version provenance plus lossless raw fallback;
- outgoing `Send_*`, expected result/error/abort, server/build gates;
- view class (`HUD`, `panel`, `primary`, `context`, `dialog`, `target`, `system`);
- keyboard-only and mouse-alternative actions;
- persistence/import classification and side effects/files;
- Linux/Windows and optional-library expectations;
- replay/fixture/test ids and completion status for native and HTML.

Test at four layers:

1. SDL/X key events -> identical logical key streams, keymap/macro expansion and
   prompt-local routing.
2. Recorded/versioned incoming packets -> semantic/raw state and outgoing bytes,
   including partial/unknown/error/disconnect cases.
3. State -> complete visible information/actions across every surface, renderer
   mode, overlay/background update, resize and fallback.
4. Linux and MinGW builds -> one `SDL_Window`, config isolation/import, audio
   present/absent, clipboard, XHTML/PNG/BMP capture, file updates, relogin and
   clean shutdown.

## Scope boundary

Explicitly excluded: multiple OS windows; arbitrary legacy `window_flag`
assignment; per-term title/visibility/position/size/font configuration;
controller support; and restoration/dependency on obsolete modern-client object
files.

Not excluded: all information formerly shown in auxiliary terms; keyboard-only
operation; normal and roguelike modes; shipped and arbitrary user macros; raw
keys; pre-game, death and privileged flows; protocol/build-dependent behavior;
text/raw fallback; mouse-first alternatives; and small additive server changes
that do not break existing clients.

## Remaining fog and follow-up research tickets

This ticket resolves the complete family-level baseline. An objective 100% claim
still needs atomic inventories that exceed a single research pass:

1. **Enumerate every input loop.** Generate the full normal/roguelike/macro/local
   key and prompt transition table from `c-birth.c`, `c-cmd.c`, `c-inven.c`,
   `c-store.c`, `c-util.c`, `c-xtra2.c`, `skills.c` and SDL3 audio menus.
2. **Map every packet field into semantic state.** Include version gates,
   update/clear lifetime and direct-draw handlers that currently store no model.
3. **Inventory all formatted/server-driven surfaces.** Every `SPECIAL_FILE_*`,
   store/special-store row/action/animation, guide/help/lore screen and permanent
   raw fallback.
4. **Specify renderer parity.** Encoding/control codes, cell metrics, fonts,
   palettes, tiles/subtiles/masks, arbitrary pictures, animations, lighting,
   weather, cursor and both screenshot products.
5. **Classify every persisted key and file.** Safe read-only import versus modern
   UI config, macros/auto-inscriptions, credentials, audio/graphics, bookmarks,
   histories, screenshots/dumps and server file updates.
6. **Audit all platform deltas and packaging.** X11 external helpers, SDL3 gaps,
   MinGW behavior, optional dependency degradation and CI/release artifacts.
7. **Make the HTML gap ledger machine-checkable.** Preserve its approved UX while
   distinguishing prototype-complete from native/protocol parity.

Implementation may proceed vertically while these tickets close, but the release
cannot truthfully claim 100% parity until every atomic row passes the harness.

# Surface coverage and geometry evidence — planning asset

Этот asset связывает geometry fixtures [Layout prototype](layout-prototype.html) с source inventories. Канонические пользовательские решения и итоговое подтверждение живут в [Specify surface layouts and responsive rules](issues/21-specify-surface-layouts-and-responsive-rules.md), не здесь. Prototype синтетический, не запускает native commands/network, не является atomic capability manifest/ledger или доказательством runtime parity.

## Source pointers

- [Inventory the complete behavior baseline](issues/01-inventory-behavior-baseline.md):36 family IDs, captured `87ead6ff5:docs/research/single-window-behavior-baseline.md`; historical file отсутствует в worktree. Читать `git show 87ead6ff5:docs/research/single-window-behavior-baseline.md`.
- [Atomic input contexts](../../docs/research/single-window-input-loops.md):52 scoped contexts; [remaining contexts](research/remaining-client-input-loops.md):14 и delegated packet/Lua/shutdown owners.
- [Formatted/server surfaces](research/formatted-and-server-driven-surfaces.md):all active SPECIAL_FILE categories, normal/wide/special stores, requests, local Guide/files/lore.
- [Slash grammar](research/slash-command-grammar-and-dispatch.md), [packet fields](../../docs/research/single-window-packet-state.md), [renderer inventory](research/renderer-parity.md), [persisted settings](research/persisted-settings-and-files.md).
- [Interaction semantics](issues/03-define-single-window-interaction-model.md), [input/macros](issues/04-preserve-input-and-macro-semantics.md), [text/glyph identity](issues/22-define-encoding-and-glyph-identity.md), [latency](issues/20-classify-display-urgency-and-latency-budgets.md).

## Family allocation and entry-point coverage

Все36 IDs перечислены один раз как primary ownership; capabilities могут пересекать surfaces. Entry points — native contexts/dispatch owners, а не invented buttons/hotkeys. Runtime bindings и gates берутся из input inventories. UI не расширяет chat/command availability при открытом primary.

| Families | Surface ownership | Entry points / geometry fixture |
|---|---|---|
| CMD-01 | Game map; locate/look/target child modes; environment/house actions | `input.command`, `.target`, `.house`; map font-only/mixed, targeting toggle |
| HUD-01, HUD-02 | Persistent left HUD, target/floor/map feedback, alerts | Packet/state owners; synthetic resources/status panel, safe primary rectangle |
| DATA-03 | Map/world/clone information, lag sample view, notes/documents | Knowledge dispatch, locate, `input.command.misc-index`, `.lagometer`, `.notes`; Mini-map/Lagometer/Document fixtures |
| SUBWIN-01 | Semantic information classes: inventory/equipment/bags, character/bonuses, messages/all/chat, players, map/clone, lagometer | Current widgets + respective primary pages; separate Terms/config excluded. Hidden small right panel does not remove primary entry points |
| RENDER-01, RENDER-02, RENDER-03, RENDER-04 | Fonts/glyphs, tiles/masks/pictures, palettes/effects/weather, resize/DPI/screenshot | Role settings and decoded updates; font-only/mixed map and special-store fixtures. No pixel/glyph-corpus parity claimed |
| CMD-02, DATA-01 | Inventory, Equipment, subinventory/bags; item operations and selectors | `input.command.inventory-view`, `.item-select`; Inventory fixture, child prompt, source/slot/tag/name semantics stay core-owned |
| CMD-03, DATA-02 | Abilities/Spellbook, powers, mimic/stance/technique/runecraft/breath pages; item/direction/target children | `.ability-select`, `input.spell.*`, `.target`; Abilities/Spellbook/child/target fixtures. Multi-step back/cancel follows source context |
| CMD-04 | Character/Skills plus Knowledge/Help dispatch and local content | `.character`, `.skill-tree`, `.misc-index`, `.guide`, `.local-file`, `.lore`, `.spoilers`; Character/Skills/Knowledge/Guide/Document fixtures |
| SOCIAL-01 | Social pages: players/equipment, party/team, guild, hostility/peace; name/confirm/report children | `.party`, `.guild`, server PLAYER/PLAYER_EQUIP/OTHER; Social/Document/child fixtures |
| CMD-05 | Settings/options/account, houses, gated DM/admin/tools | `input.options.*`, `.house`, `input.admin.master`; Settings/Tools/child fixtures; hidden/gated capabilities use baseline gates |
| AUDIO-01 | Audio settings/packs/mixer/jukebox; extraction/password/progress/errors | `input.audio.*`; Audio/child fixtures, baseline helper/progress outcomes |
| FILE-01, FILE-02, FILE-03, FILE-04 | Preferences/macros/autoinscriptions/resources; Guide/files/notes, clipboard/screenshots/dumps/download/transfer | `input.macro.editor`, `input.autoinscription.editor`, `.guide`, `.local-file`, `.notes`; Settings/Macros/Tools/Guide/Document/child fixtures. Persistence/encoding/file execution policy separate |
| STORE-01 | Server-entered Store primary: normal/wide rows, server actions, special Go/casino canvas | `.store`, `.stock-select`, request/item/spell children; Store/Special store/child fixtures. No fake generic enter-store action |
| LIFE-01, LIFE-02, LIFE-03, LIFE-04 | Startup/connect/auth/character overview/reorder/creation, setup MOTD, death/end/relogin/shutdown | `input.birth.*`, `input.account.*`, `input.startup.*`, `input.shutdown`; Pregame form/Session end fixtures. MOTD concrete UX delegated to separate prototype |
| PROMPT-01, PROMPT-02 | Local/key/confirm/quantity/text/item/direction prompts; KEY/AMT/NUM/STR/CFR/ABORT, sale/pickup/pause | Owning local/packet contexts; child geometry fixture. Unsupported key/field repertoire not invented |
| IN-01, IN-02, IN-03 | Input router/binding/macro pipeline across every surface | Full native scoped/delegated inventories; prototype clicks never assert macro or keyboard parity |
| NET-01, NET-02 | Decoded model, outgoing core intents, asynchronous documents/requests/system transitions | Full packet inventory; source ownership, no wire parsing/Term buffer in UI fixture |
| UI-01, UI-02 | Single primary, child chain, focus/restoration, viewport/canvas state | Every fixture; source baseline cancel/finish/resume retained, no new section recall |
| PLATFORM-01 | Linux/Windows dimensions, scaling/resource/build gates | Native acceptance matrix; browser CSS is not platform/runtime evidence |

## Reusable content geometry

- Typed lists: wrapping names, fixed semantic key/count/price visibility, vertical scrolling; header/nav/actions outside body scroll. List selection/focus survives resize of current interaction.
- Detail pages: list/tree + readable detail regions using individual HTML UX layout; no automatic columns→tabs conversion. Narrow overflowing regions use scrolling without changing action/context identity.
- Server-formatted documents: exact source lines/colors/cell columns, no automatic wrap; horizontal offset and baseline page requests/search. Unknown categories/titles use generic source-preserving viewer.
- Local Guide/lore/files: source owners stay distinct. Existing approved HTML views/layouts are normative where applicable. Original source lines/markers and baseline search/bookmark/chat-paste behaviour remain available through their source owners; source-view versus readable article layout requires explicit final confirmation if not already covered by HTML UX.
- Special store: one uniform canvas transformation shared by text/grid/raw-picture/animation anchors; complete fit, no scroll. Controls/prompts stay semantic UI outside canvas; no inference from rendered text.
- Native store reference evidence: text/tile/picture anchor `(borderX+col*refCellWidth, borderY+row*refCellHeight)`; raw extents may span fractional/non-unit cells. SDL3 source `main-sdl3.c:836–865,2136–2140,2249–2281,3444–3464`; packet/animation coordinates `nclient.c:4663–4728,4740–4821,5070–5119,5293–5404`. Fruit128×192 is8×8 reference16×24 cells, cards50×80 span50/16 ×80/24 cells (`graphics-16x24sv.prf:1536–1548`). Uniform fitting transforms anchors AND raw extents/clear regions; never squeezes pictures into a single cell. Exact resource/profile/rounding oracles belong to raster ticket.
- Pregame forms/session-ending scenes: whole-window ownership; no game HUD before active gameplay or after session teardown. Scroll long forms with persistent required controls; preserve baseline retry/skip/ack/character-flow gates.
- Setup MOTD: source-owned formatted whole-window scene with separate [Prototype the server-defined MOTD screen](issues/28-prototype-the-server-defined-motd-screen.md); current asset intentionally has no fabricated MOTD page.

## Prompt census

Quantity/key/yes-no/text/name/password/item/source/slot/tag/direction/target; character reorder(two choices), race/class/trait/stats/mode; spell/book/form/immunity/breath/runecraft; document search/regexp/goto/bookmarks; filename/dump/save-chat; raw macro trigger/record/wizard/collision; autoinscription rule/apply; tileset apply/discard; audio extraction/password/progress/error; sell/pickup/donation, house/guild/party/suicide/admin confirms; server PAUSE/ABORT. Each uses one of the child/form/viewer/target geometries above with its own baseline binding/context semantics.

## Evidence limits and remaining owners

Linked spatial captures: [Small1024×768](layout-small-evidence.png), [Wide1920×1080](layout-wide-evidence.png). Browser geometry pass:240 cases across20 surfaces,3 dimensions(1920×1080,1280×800,1024×768),2 layouts and text12/24; no frame bounds/map-fit/cell-count/default-draft failures. Subsequent fixed-column fixtures: Character/Skills/Knowledge/Settings retain2 panes, Spellbook3 panes at1024/1920 with text24. Current interaction row20 remains focused/visible and draft retained after resize; sample birth field draft/focus retained; child quantity17 retained; manual offer activates22 rows/right hidden and explicit wide restores44 rows without repeating offer. No browser console errors.

Browser geometry/draft/focus checks only. Shape rows/forms/pictures are synthetic fixtures, not complete per-capability UI implementations. Atomic manifest IDs/consumer ledger outcome/scenario evidence, full glyph corpus/raster compatibility, field-byte conversion, retention, persistence schema/defaults and native platform acceptance remain in their existing tickets. A36-family allocation cannot establish100% native capability parity.

Manual wide layout may become visually cramped in small client areas; offer/manual small layout is the agreed route. The proposed automatic minimum-window formula and396 map-width threshold were not accepted. Exact raster legibility/profile bounds belong to raster/acceptance decisions and must not be inferred from synthetic screenshots.

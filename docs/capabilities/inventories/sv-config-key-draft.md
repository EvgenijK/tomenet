# SV main CFG: source registry and schema draft

> Review precedence, 2026-09-20: this source-audit draft retains historical proposals. The current consolidated contract is [persistence contract](../persistence-contract.md), based on ticket answers through Q40. In particular, display options DO affect SV, fonts import into BOTH roles, startup is fullscreen, Lanczos maps to Linear, own CFG/OPT conversion is Save-only, and server file transfer retains original behavior WITHOUT the proposed new allow-list. Do not implement superseded proposals below.

Status: planning draft, 2026-09-20. This is not a resolved schema or implementation. Accepted outcomes come from [Specify persistence ownership and UI configuration schema — Comments](../issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#comments); exact new key spellings and classifications labelled **proposal** need review. Later Comments override earlier shared-main-config and map-zoom proposals.

## Accepted boundary

- Main destination is `<SDL3 user root>/sv/tomenet.cfg`, using current SDL3 line-oriented CFG syntax, not JSON. It owns SV connection/audio/resource selections and UI settings independently of legacy main CFG. Legacy CFG is an explicit import source only, never a normal SV read/write destination.
- Settings preview applies immediately; Cancel restores values from opening the form. Disk writes require Save; closing dirty settings offers Save / Cancel changes / Return. Normal game exit neither autosaves CFG/OPT nor adds another question. Credentials and separate approved personal-file lifecycles are exceptions with their own owners.
- UI preferences have user-root scope, not account/server/character scope. Selected resources are independent; font, tileset, audio-pack assets and common resource PRFs remain in shared locations. No resource copies arise merely from saving a selection.
- Malformed individual values use their own default with a message; unreadable UI config uses UI defaults. Unknown compatible fields survive a save. Incompatible schema version uses defaults and warning without overwriting source. Exact grammar/version/whole-file failure treatment remains draft below.
- Requested resources persist; temporary startup effective fallback does not overwrite requested selection. UI fallback: requested → bundled Cascadia Mono → bundled `16x24x.pcf`; map: requested → bundled `16x24x.pcf`, deduplicate matching steps. Report startup failures. Failure of every font fallback terminates startup with a system message; failed tileset permits font-only map. Live failure retains old working resource.
- Password bytes have **no ordinary main-CFG read or write path**. Explicit legacy credential import uses the separate approved OS-vault flow. Existing legacy password records remain untouched. A `pass` record is a recognized secret exception, not an unknown field eligible for blind preservation into an SV file.

## Source method and syntax caveats

Audited [read_mangrc and read_mangrc_aux](../../../src/client/client.c#L69), [write_mangrc](../../../src/client/client.c#L694), [colour writer](../../../src/client/client.c#L1185), [initial variables](../../../src/client/variable.c#L236), [default_set/startup](../../../src/client/client.c#L1274), and repository [client stock CFG](../../../tomenet.cfg). `lib/config/tomenet.cfg` is a **server** configuration and is not this syntax/default source. Inventory companion: [persisted settings and files](persisted-settings-and-files.md#config-key-sets).

Current reader compares prefixes, uses `atoi` and ad hoc `strtok`; flags can be enabled/disabled by presence, not Boolean value. It skips leading `#` and empty lines, does not implement a general INI/JSON grammar, and preserves neither a typed schema nor transactional parsing. `server` optionally embeds `:port`; later recognized lines win the assignments they make. Unsafe parsing is a source fact, not approved defect compatibility.

**Proposed syntax contract:** retain `key<TAB>value`, `#` comments, and existing standalone flags, but recognize complete key tokens, validate before apply, and preserve compatible unknown records. Numeric enums imported by semantic meaning, not by incidental numeric equivalence. Duplicate-key resolution, whitespace/escaping, line-length limits, schema version spelling and recovery-save mechanics still need explicit specification. Do not run the legacy startup reader/writer for migration: it can copy/create source CFG, convert Term names, and serialize credentials.

The tables classify every recognized main-CFG key/prefix plus writer-only subset keys. `R` = reader; `U` = updates existing records; `N` = emits on new-file creation; `C` = dedicated colour writer. `N comment` is a commented template, not a live default. Values below distinguish compiled/no-file defaults from stock file overrides. Unless **accepted** is written, SV retention/default/key mapping is a **proposal**, not a new product decision. Retained settings are owned by SV main CFG and explicit Save, with baseline runtime subsystem consuming them; no legacy file synchronization.

## Connection and identity

| Exact key | Legacy R/W and default | SV classification / transform |
|---|---|---|
| `nick` | R; U conditional on account+password, also recognizes `#nick`; N active or comment. `PLAYER`, or SET_UID login/TOMENET_PLAYER via default_set | Retain account metadata; remove password-dependent writer condition. Identity normalization/credential reference serialization separate pending contract |
| `pass` | R; U also `#pass`; N active obfuscated or comment; default empty | **Accepted excluded** from normal CFG read/write. Explicit credential import only; never emit secret/template or carry source record into SV |
| `name` | R; N comment; no ordinary U; `cname` empty | Retain explicit fullauto character preference, independent of last selected character unless approved |
| `meta` | R; N comment; no U; empty variable means compiled metaserver (`META_ADDRESS`, currently meta.tomenet.eu) | Retain override; absent inherits compiled service default |
| `server` | R host and optional colon-port; N comment; no U; empty means server selection | **Accepted independent connection setting**; propose host and port separate canonical records; source-order import evaluates old combined syntax |
| `port` | R; N comment, no U; 18348 | Retain independent effective-port preference; validate supported port range, preserve credential identity pairing |
| `realname` | R; N comment, no U; PLAYER or SET_UID login/TOMENET_USER | Retain baseline field-specific value; exact platform initialization/identity normalization needs registry follow-through |
| `path` | R; N comment, no U; empty initial buffer | Recognized bootstrap/path override, not UI. Proposed import exclusion pending verification of downstream path semantics; do not let imported value relocate agreed user root silently |
| `fullauto` | R presence returns skip=true; N comment; default absent/false | Retain presence semantics for compatible import; does not authorize config-password fallback |

Compiled identity sources: [variable.c](../../../src/client/variable.c#L12), [default_set](../../../src/client/client.c#L1274), [META_ADDRESS](../../../src/config.h#L225). Startup processes CLI after CFG and has environment handling; final SV precedence across CLI, explicit alternative config and imports is not yet fully specified.

## Rendering and shell

| Exact key/prefix | Legacy R/W and default | SV classification / transform |
|---|---|---|
| `fps` | R/N, no normal U; 100 | Retain frame-rate preference provisionally; allowed values and interaction with new scheduler/acceptance not inferred from `atoi` |
| `lighterDarkBlue` | R presence=true; reader resets false first; N enabled; stock enabled; no U | Retain semantic palette preference; no-file false differs from stock/generated true. Proposed SV default true to match stock; not accepted yet |
| `colormap_<i>` (`0 ≤ i < BASE_PALETTE_SIZE`, currently 16) | R, C; N commented palette. Default hex values below | Retain each explicit palette entry; keep independent palette semantics. C omits index0 unless CUSTOMIZE_COLOURS_FREELY; SV index0 editability remains a build/UX policy |
| `graphicsForceOutline` | R/U/N under SDL3+graphics+BG_MASK; -1 default, bounded -1…6 in this build | Retain requested outline setting provisionally; generated final-size outline geometry follows raster policy, not old pixel equality |
| `graphic_resize_type` | R/U/N; source SDL3 default0 Nearest, stock1 Linear; X11 default1. Values0 Nearest,1 Linear,2 Lanczos | **Accepted SV default Linear**. Propose separate new symbolic `svGraphicsFilter`; import0→Nearest,1→Linear. Lanczos conversion still requires decision; numeric2 must NOT become PixelArt accidentally |
| `graphics` | R/U/N; compiled and stock0; BG_MASK reader permits0/1/2 (none/one-mask/two-mask) | **Accepted enabled by default**. Exact encoded default for selected16x24sv must preserve resource composition mode; Boolean UI must not flatten one-mask/two-mask capability |
| `graphic_tiles` | R/U/N; `16x24sv` | **Accepted default16x24sv**; persist requested resource name/reference in own CFG, retain common assets/PRFs |
| `graphic_tiles<i>` (`0…9`, MAX_SUBFONTS=10) | R; U rewrites all after base tileset and discards prior indexed lines; N all false | Retain each selected subset state; propose false defaults matching source. Reference compatibility validated together with selected tileset |
| `disableGfxCache` | R/N under TILE_CACHE_SIZE; no U; false, stock0 | Legacy cache-specific control. Proposed inert import record/no active SV key until new cache/acceptance owner defines equivalent; do not pretend old on/off controls new prepared-asset caches |
| `windowDecorations` | R/U/N SDL3; compiled false, stock1 | Single-window-compatible preference candidate. Proposed retain and default1; user has not separately selected default/decorations scope |
| `hintBigmap` | R presence disables hint and firstrun; U appends for explicit legacy save; no active N | Legacy hint marker, **not SV layout state**. Exclude active import; accepted SV offer is once per launch and ephemeral |

Default base palette by index0…15 from [variable.c](../../../src/client/variable.c#L541): `000000 ffffff 9d9d9d ff8d00 b70000 009d44 0000ff 8d6600 666666 cdcdcd af00ff ffff00 ff3030 00ff00 00ffff c79d55`. `lighterDarkBlue` only changes original blue6 when still0000ff. `#colormap_`/`;colormap_` are rewrite recognizers in the colour writer, not additional reader directives. Unknown or build-disabled records must not become effects through broad prefix matching.

## Audio (each original key)

| Exact key | Legacy R/W and compiled / stock default | SV classification / transform |
|---|---|---|
| `sound` | R under USE_SOUND; N; no normal U; true with USE_SOUND_2010, stock1 | Retain requested audio enable; runtime availability is separate effective state |
| `hintSound` | R presence suppresses hint; N and conditional U append; absent means hint=true, generated file suppresses | Retain or retire legacy first-run hint pending UX, not silently equate to audio enable |
| `cacheAudio` | R/N, no U;1 (`no_cache_audio=false`), stock commented1 | Retain baseline audio caching preference provisionally; invert no_cache_audio correctly |
| `audioSampleRate` | R/N, no U;44100, stock commented44100 | Retain audio backend preference; supported numeric values/restart requirement need backend validation |
| `audioChannels` | R/N, no U;32, stock commented32 | Retain; valid bounds need backend validation |
| `audioBuffer` | R/N, no U;1024, stock1024 | Retain; valid bounds/restart requirement need backend validation |
| `soundpackFolder` | R/U/N; startup `sound`, stock sound | **Accepted independent selected pack**, shared underlying assets |
| `soundpackSubset` | U/N only, variable default1, stock1 | Reader mismatch: proposed explicit valid round-trip in SV, import as subset selection. Legacy `sound` prefix can consume this line as audio enable because exclusion only names soundpackFolder; do not reproduce this collision |
| `musicpackFolder` | R/U/N; startup `music`, stock music | **Accepted independent selected pack**, shared underlying assets |
| `musicpackSubset` | U/N only, variable default1, stock1 | Proposed explicit valid round-trip in SV, not evidence old reader applies it |
| `audioMaster` | R/U/N; true, stock1 | Retain independent main mixer switch |
| `audioMusic` | R/U/N; true, stock1 | Retain independent music switch |
| `audioSound` | R/U/N; true, stock1 | Retain independent effects switch |
| `audioWeather` | R/U/N; true, stock1 | Retain independent weather switch |
| `audioVolumeMaster` | R/U/N;70, stock70 | **Accepted independent volume**, propose existing default70 |
| `audioVolumeMusic` | R/U/N;70, stock70 | **Accepted independent volume**, propose existing default70 |
| `audioVolumeSound` | R/U/N;70, stock70 | **Accepted independent volume**, propose existing default70 |
| `audioVolumeWeather` | R/U/N;70, stock70 | **Accepted independent volume**, propose existing default70 |

Audio gates: all except top-level sound are USE_SOUND_2010 reader paths. Writers contain some historical compatibility inserts of sound/music folders and subset1. They are not instructions to insert defaults into an external import source. Defaults: [variable.c](../../../src/client/variable.c#L417), [startup folders](../../../src/client/client.c#L1393), [AUDIO_VOLUME_DEFAULT](../../../src/common/defines.h#L8700). Exact permitted volume/subset ranges are not established by main reader; enumerate against backend/pack definitions before schema closure.

## Every Term prefix and suffix

Recognized modern prefixes: `Term-Main`, `Term-1`, `Term-2`, `Term-3`, `Term-4`, `Term-5`, `Term-6`, `Term-7`, `Term-8`, `Term-9`. Old aliases in corresponding order: `Mainwindow`, `Mirrorwindow`, `Recallwindow`, `Choicewindow`, `Term-4window`, `Term-5window`, `Term-6window`, `Term-7window`, `Term-8window`, `Term-9window`. This cross-product table classifies every recognized suffix for each prefix, not a new multi-Term model.

| Suffix | Legacy rule / stock value | SV disposition |
|---|---|---|
| `_Title` | R/U/N only subwindows; main ignored/deleted. Stock Msg/Chat, Inventory, Character, Chat, Equipment, Bags, Term-7/8/9 | Legacy-only, never apply to SV shell |
| `_Visible` | R/U/N only subwindows; main always visible. Stock terms1…6 true,7…9 false | Legacy-only, not SV block visibility |
| `_X` | R/U/N all; skip minimized sentinel-32000 in writer. Stock0 all | Legacy window geometry; no direct import to SV window position |
| `_Y` | Same as X; stock0 all | Same |
| `_Columns` | R main forced80; main write removed/omitted. Subwindows zero→80; stock80 all | Legacy columns, never map to logical pixel width or map viewport |
| `_Lines` | R/U/N all; zero→24; stock main24, terms1…4=24,5=14,6=13,7…9=24 | Legacy rows, never automatic SV layout/big-map import; saved SV layout governs geometry |
| `_Font` | R/U/N all; stock main8x13,1/2/4=6x10,3/5…9=5x8 | Preserve resource choice as explicit role-mapping **candidate** only, not automatic Term assignment. Proposed import main font→map role if selected in preview; text role stays independently chosen; product mapping still open |

Missing-file frontend font/Term initialization is separate from stock CFG values (SDL3 compiled default font9x15 in config.h); exact platform geometry is irrelevant to SV transfer and intentionally not promoted to new defaults. Legacy reader calculates screen geometry from Term-Main lines after reading and under GLOBAL_BIG_MAP sets global_c_cfg_big_map; the SV importer must not call that apply path. SV never writes fake Term-* records.

## New SV-only fields: spelling proposals versus accepted values

All key names below are **proposed**, using the same tab-separated CFG syntax. Accepted semantics do not imply names already implemented. Effective runtime values are derived and displayed, not saved over requested values.

| Proposed key | Accepted value/behavior | Remaining detail |
|---|---|---|
| `svSchemaVersion` | Separate versioned schema and incompatible-version protection required | Propose integer1; version migration and missing-version interpretation unresolved |
| `svLayout` | wide default; small/wide user choice governs normal/big geometry, independently of legacy | Exact imported layout treatment; no legacy big-map write |
| `svUiScalePercent` |50…200, step5, default100; OS DPI separate; panels scroll instead of auto-shrink on shortage | Numeric invalid/off-step handling must use agreed default/error policy rather than silent arbitrary rounding |
| `svTextFont` | CascadiaMono-Regular.ttf v2407.24 bundled/default | Shared reference scheme, base text size at100%, explicit legacy-text visual profile identity |
| `svTextProfile` | Explicit correspondence, no universal network charset | Exact shipped profile identifier/mapping remains to enumerate |
| `svMapFont` |16x24x.pcf default and map fallback | Resource identity/reference form |
| `svMapProfile` | Numeric glyph/cell identity preserved | Exact font+PRF/profile association, precedence |
| `svGraphicsFilter` |Linear default; Nearest/Linear/PixelArt supported subject to effective fallback | Proposed symbolic values; legacy Lanczos conversion decision pending |
| `svPcfFilter` |Nearest default; independent of graphics, Nearest/Linear/PixelArt | Scope across text/map PCF roles not separately fixed; requested/effective diagnostic display required |
| `svPanelLeftWidth`, `svPanelRightWidth` | User-controlled widths; HTML UX owns concrete controls/layout | Numeric defaults/min/max and minimum center still unspecified |
| `svBlockVisible_<stable-id>` | Player may choose visible left-panel blocks | Stable ID catalogue and per-block defaults unspecified |
| `svRightWidget_<stable-id>` / order field | Chosen widgets/placement retained in wide; hidden in small | Concrete schema/order/duplicates and defaults await HTML UX registry |
| `svWindowWidth`, `svWindowHeight`, optional position/state keys | Single window minimum supported client area1024×768 logical | Startup size/maximized/fullscreen/position persistence and defaults not fixed by minimum |

**No `svMapZoom` or map-scale slider key:** user superseded former adjustable zoom with always100% maximum full-grid fit. Final output cell size/cache generation/effective viewport are derived. No persistent small-layout-offer dismissal: offer is once per application launch. No session recorder keys. Fallback chain is accepted policy, not necessarily user-editable CFG fields. Archive/resource passwords never belong in CFG or common history.

## Open issues needed before calling this registry complete

1. Approve remaining scalar defaults/gates and precise frontend applicability (fps, outline, palette/readability, decorations, hintSound, audio parameters/cache/subsets). Source defaults are not automatically new product defaults.
2. Define exact grammar/serialization (including representable strings and tab/newline policy), version migration, duplicate keys, import group validation and unsupported-build key treatment; preserve unknown compatible fields without allowing known secret passthrough.
3. Choose legacy Lanczos conversion and exact graphics mode for enabled default16x24sv; explicit font-role import mapping. Enumerate profile/resource references and base UI font size.
4. Resolve bootstrap/CLI/environment precedence and `path`; pin server host+port serialization without conflating identities, including IPv6 and platform case behavior. Do not reuse old colon tokenizer as a new identity contract.
5. Define single-window geometry and per-widget defaults/limits against HTML prototype; no fabricated widths or thresholds. Main CFG table cannot replace option/PRF/file ownership registries.
6. Exact save transaction/concurrent-SV conflict behavior, failed Save state and broken-file disposition. Old remove-then-rename writer is not atomic publish proof. Unknown-key retention and Save-only require a dedicated owner on every configuration-write path, including palette, audio-pack installation, conversion and shutdown helpers.
7. Confirm no settings auto-copy/autogeneration before Save; main-CFG independent storage implies never writing legacy path, but explicit import commit versus Save still needs one coherent transaction rule.

No user configuration files, credentials or production sources were edited. Findings are static source facts, not runtime proof of parser fixes or backend compatibility.

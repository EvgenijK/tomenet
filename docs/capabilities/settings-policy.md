# Settings, resources and platform policy snapshot

Ticket 11 planning authority, captured 2026-09-22. Current source defines baseline behavior; these approved decisions define intentional SV changes. Historical audit proposals do not override these decisions. No runtime or release acceptance is claimed.

Precedence: the final persistence contract below supersedes earlier shared-CFG proposals, map zoom, screenshot swaps/XHTML and draft transfer restrictions. Map fit is always maximal; no user map zoom. Controller support and legacy terminal topology/GCU fallback are excluded. Linux amd64 and Windows 10/11 i686 are the supported destination; Wine is intermediate evidence. See [allocation and layout](item-policy.md) and [field-byte policy](session-policy.md).


## Final persistence contract

Original `.scratch/single-window-sdl3-client/persistence-contract.md`, SHA-256 `5fcdcdf8814dfd64ee580834277228a0330a93e7a7943fb35488f3cc1c5a155b`.

# Single-window client: persistence contract

Status: approved planning contract, 2026-09-20. The user confirmed the complete contract in [Specify persistence ownership and UI configuration schema](../../.scratch/single-window-sdl3-client/issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#answer). This document consolidates the final decisions and exact paths/serialization mechanics; it supersedes conflicting historical proposals in the linked source-audit drafts. It is not an implementation or runtime parity claim.

## Ownership and paths

`U` is the existing SDL3 user root (`TOMENET_SDL3_USER_PATH`, otherwise `SDL_GetPrefPath("TomenetGame", "tomenet")`); `S = U/sv`; `B` is the selected game library, normally `lib/` beside the executable. Do not globally change `ANGBAND_DIR_USER` or `SDL3_USER_PATH` to S: that would relocate deliberately shared data and change the SDL3 handshake fingerprint. Root selection and compatible CLI/environment overrides retain baseline behavior; importing a config does not implicitly relocate U/B. The SDL3 fingerprint continues using U, not S.

| Data | Concrete destination | Load/save contract |
|---|---|---|
| Main connection/audio/resource-selection/UI CFG | `S/tomenet.cfg` | Independent of legacy CFG; explicit Save only |
| Global option snapshot | `S/global.opt` | Existing OPT syntax and global scope; explicit Save |
| System-specific option layer | `S/global-sv.opt` | Existing system-layer position; no inheritance from user `global-sdl3.opt` |
| Character option snapshot | `S/<character>.opt` | Existing naming/lifecycle, including pre-login application before option packets; explicit Save |
| Explicit class/named option snapshots | `S/<class>.opt` or user-selected SV option destination | Keep baseline named/global/class commands; class save does not introduce automatic class OPT loading |
| Dedicated option entrypoint | `S/options.prf` when present | Independent option owner even though suffix is PRF; conversion is in memory until Save |
| Macro PRFs and stage metadata | Existing shared `U/user` overlay, bundled B fallback | Existing global/race/trait/class/character/form precedence, normal permitted load-time effects; explicit macro saves update shared files |
| Autoinscriptions INS | Existing shared `U/user` | Existing most-specific-file selection, named/global/class saves and automatic format conversion |
| Birth templates DNA | Existing shared `U/user` | Existing name/reincarnation lookup, automatic birth save and format conversion |
| Input history | `S/chathist-<account>.tmp` | Existing account naming, bounded recall/dedup/relog rules; symmetric load/save path and baseline exit lifecycle |
| Guide bookmarks | `S/bookmarks.tmp` | Existing format/lifecycle; symmetric load/save, explicit old-file import |
| Received private notes | `U/user/notes-<account>.txt` | Existing automatic append/browser behavior; not a settings save |
| Message/character/screenshot exports | Existing shared `U/user` | Existing content/format semantics; generated names get a free suffix on collision, explicitly selected existing filename requires replace/cancel choice |
| Message cloning | Console and shared `U/user/stdout.txt` | Existing `clone_to_stdout`/`clone_to_file`, initially off; colour stripping, line ordering, append/flush behavior from SDL3; independent of bounded recall and new-recorder prohibition |
| Fonts/tilesets/mappings | Existing shared user resource overlay and bundled resources | Independent requested selections in S CFG; asset/mapping data remain shared |
| Audio packs, event mappings, disabled events, per-event volumes | Existing shared pack directories | Shared resource config edits; selection/master/category controls belong to S CFG |
| Derived graphics caches | Shared resource root with a distinct SV cache version/namespace | Do not import; keys cover source/profile/filter/final geometry. Never reuse legacy cache identity for incompatible output |
| Updated guide | `U/TomeNET-Guide.txt` | Viewer/checksum/update use the same user override; bundled B/G guide is fallback. Legacy may still use its installation copy |
| Credentials | Previously selected OS vault | No secret/reference-to-secret-content in ordinary CFG/OPT; precise policy remains in credential ticket |
| Server file transfers | Original client's destination mapping | Q40: preserve original validation, mapping, CHECK/INIT/DATA/END and reload behavior; no new resource-only allow-list/deny-list. Do not rebase generic transfer destinations onto S |

Absolute/custom filenames and explicit file operations retain their existing meaning. Automatic defaults and internal Save routing use the owned locations above. External import is a different operation: it never modifies its source. No feature is permitted to invoke a legacy writer against `U/tomenet.cfg` merely because it previously did so at startup, audio selection, palette change or shutdown.

The full operation inventory is [file ownership evidence](../../.scratch/single-window-sdl3-client/research/sv-file-ownership-draft.md). Its old proposals for a new transfer allow-list, guide placement or unresolved group rules are superseded by this contract and the live answers.

## Settings editing, publication and conflicts

1. Opening settings records the current values. Changes preview immediately. Resource changes must first prepare a usable replacement; a failed live load retains the previous working selection and reports the failure.
2. **Сохранить** persists the edited settings to their owned CFG/OPT destinations. An ordinary in-game option saves to the current character scope by default; the existing explicit global/class/named save actions retain their meanings. New UI settings are user-root-wide, not character/server-specific.
3. Closing a dirty form offers **Сохранить / Отменить изменения / Вернуться**. Cancel changes restores settings values from opening the form; it cannot undo gameplay actions already performed while previewing an option. Gameplay state, incoming messages and already-delivered replies are not rolled back.
4. Normal game exit does not save pending CFG/OPT changes and does not add another settings prompt. Baseline history/bookmark/notes/DNA/resource writes and approved vault writes have independent lifecycles.
5. Own CFG/OPT missing-file defaults and obsolete-key conversion remain in memory until Save. A successful explicit import has its own reviewed commit action; it is an authorized write, not an automatic startup import. Shared INS/DNA conversions remain automatic. Normal macro files do not acquire blanket auto-resave.
6. Resolve the concrete owner/path once for every read/stat/temporary/backup/replace/delete. Avoid legacy `my_fopen` redirection combined with raw rename against a different logical path. Parse and validate before publishing, use unique sibling temporaries, and retain old destination data if writing fails. A failed Save leaves the edited values active and dirty and reports that they were not saved.
7. Save rereads its destination and merges only changed records. For concurrent edits to the same value, retain the external value and report the conflict; do not silently claim the attempted value was persisted. Unrelated unknown compatible fields remain intact. This applies to multiple SV instances and still-shared writers; it is no longer a reason to share main CFG/OPT with legacy.
8. Shared legacy writers do not participate in a new locking protocol. SV can coordinate its own writers and detect changes before replacement, but cannot guarantee that a running old client will not overwrite a shared file later. This limitation was accepted. Macro-stage/import/resource linked groups must not be reported as complete if only part was published; rollback/error results identify the affected group.

Existing named save/load operations do not silently change scope. A settings Save does not rewrite a common macro PRF just because that file supplied an option value. Explicit macro saves retain unrelated recognized records in a mixed file; conversion does not replace the file with an option snapshot. Includes and execution records are preserved as records, not executed during migration.

## Main CFG schema and scalar defaults

The format is SDL3's line-oriented CFG, **not JSON**: `key` followed by spaces/tabs and a value, `#` comments, and existing standalone flags. Writer emits a tab separator and LF. Reader accepts CRLF. Existing compatible string/flag semantics remain; do not introduce a guessed encoding or apply UI Unicode conventions to opaque field bytes.

Technical version rule: `svSchemaVersion\t1`; absent version is treated as version1 for an SV file using only recognized compatible records. Unknown fields in a supported version survive Save. A newer incompatible version uses defaults with a message and is not overwritten. Duplicate recognized values are evaluated in file order, last well-formed assignment wins; an invalid occurrence is reported and uses that field's default. Match complete key tokens: `soundpackSubset` must not accidentally toggle `sound`. Reject embedded NUL/newline in a scalar and unrepresentable serialization before file publication; preserve the draft on an encoding error.

Use the exact same keys for existing compatible settings, with complete Save coverage even where the legacy writer only wrote a key on file creation. `pass` is a recognized exception: it is not read in normal startup and not emitted or preserved as an unknown field. `nick`, server/port and fullauto remain nonsecret metadata. Explicit legacy import alone can read a legacy password and send it to the vault. No secret-bearing temporary CFG or backup is created.

The [complete CFG source registry](../../.scratch/single-window-sdl3-client/research/sv-config-key-draft.md) enumerates all original read/write keys and the cross-product of ten old/new Term prefixes and suffixes. Apply these dispositions to every row:

- Connection/audio/palette/resource selections with the same meaning are retained in S CFG and save independently. Existing compiled/default initialization and the **client** stock `tomenet.cfg` provide baseline defaults; the server `lib/config/tomenet.cfg` is not a source of client defaults. Explicit decisions below override them.
- Existing values include FPS100; audio44100Hz/32 channels/buffer1024; master/music/effects/weather volumes70 and switches enabled; selected pack names `sound`/`music`, subset1. SDL3 audio bounds are rate4000…48000, buffer128…8192, effect-track count4…32 (not physical speaker channels); mixer output remains stereo. Out-of-range serialized values follow the approved invalid-value default/message rule, not unsafe indexing. Validate other values against the reused backend's supported bounds; unavailable device/capability follows baseline failure behavior rather than a false successful save/apply claim.
- Existing palette entries, `lighterDarkBlue`, outline setting, standalone flags and subtileset selections retain stock SDL3 meaning/defaults. Previously write-only subset preferences get a real read/write round-trip. Baseline frontend/build gates remain explicit; unsupported keys are retained inert with a diagnostic, not converted to an unrelated feature.
- Legacy Term geometry, titles, visibility and W assignments never instantiate SV Terms. For explicit font import, the **main** Term font maps to both text and map roles. Other Term-font assignments are not guessed into modern panels. Render-cache/workaround keys remain recognizable compatibility data where their old mechanism does not exist; do not turn them into different controls.
- CLI values override loaded preferences for the running invocation according to existing option semantics; the CFG is not rewritten just because CLI overrides were supplied. Import does not silently activate bootstrap path changes. Roots/resources are resolved by their established startup owner.

| Key / value | Contract |
|---|---|
| `svSchemaVersion 1` | Format version, no secret fields |
| `svLayout wide` | `wide` / `small`; saved SV value governs actual normal/big map geometry; startup default wide, existing once-per-launch small offer retained |
| `svWindowMode fullscreen` | `fullscreen` / `window`; fullscreen default. This means the display's desktop mode, no new resolution-switch UI. Remember saved windowed geometry separately from fullscreen output size |
| `svWindowWidth`, `svWindowHeight` | Optional last explicitly saved windowed logical client size. Never save fullscreen dimensions over it. With no saved windowed geometry, use available desktop work area subject to existing minimum1024×768; WM may constrain placement |
| `svUiScalePercent 100` | Range50…200, step5; invalid/off-step values use100 with a message; OS DPI applies separately |
| `svTextFont CascadiaMono-Regular.ttf` | Bundled exact v2407.24 asset identified by the existing candidate report; OFL/copyright included |
| `svMapFont 16x24x.pcf` | Default map font, independently changeable |
| `graphics` / `graphic_tiles 16x24sv` | Default `graphics 1` (the first standard enabled mode of the existing SDL3 toggle), selected16x24sv; retain explicit dual-mask mode2 and its existing server gate, masks/composition and resource definitions |
| `svGraphicsFilter linear` | `nearest` / `linear` / `pixelart`; default Linear. Imported legacy0/1/2 mean Nearest/Linear/Lanczos, with Lanczos→Linear reported in import result; never map numeric2 to PixelArt accidentally |
| `svPcfFilter nearest` | Independent PCF sampling selector, same modes, default Nearest; applies to PCF rendering in either role, independently of tiles |
| `svTextProfile`, `svMapProfile` | Optional explicit resource-profile references; absent means the registered profile for the selected resource/source role. No network charset inferred from font metadata |
| `svPanelLeftWidth`, `svPanelRightWidth` | Saved logical widths; exact defaults/bounds are supplied by approved HTML UX metadata, not earlier throwaway numbers |
| `svBlockVisible_<id>` | Boolean visibility preference indexed by stable semantic block ID from approved HTML/manifest |
| `svWidgetOrder_<container-id>` | Ordered stable widget IDs; duplicates invalid, unknown IDs retained inert for forward compatibility; default order/placement follows HTML UX |

There is **no map zoom key**: always100% maximum full-grid fit with accepted integer final-cell geometry. Cell raster size, effective viewport, fallback asset and cache generation are derived, not saved choices. No persistent small-offer dismissal, new recorder setting, plaintext password, or per-Term layout records are introduced.

Exact text metrics at100%, panel/block/widget catalogue and constraints, cursor appearance and control layout remain with the main HTML UX prototype by explicit user delegation. Native schema binds to the same stable IDs and approved layout version. Missing approval in the HTML ledger is not a claim of native UX acceptance; this map does not invent those visual defaults.

## Resource identity, loading and fallbacks

Keep requested resource name/reference bytes and associated profile references in S CFG; lookup uses the existing user-overlay-before-bundled hierarchy. Full local names are not shortened to fit server reporting slots; only the reporting copy follows the accepted field-boundary policy. Resource identity for caches also includes source/profile generation, not just a mutable filename. Profile data preserve proven legacy-ID correspondence, control markers and source-specific byte contracts; no universal outgoing charset is introduced. Existing PCF numeric glyph semantics and font/graphics PRF mappings remain normative.

Text font changes do not reload map definitions. Map font changes retain the established map-font→mapping reload→graphics overrides lifecycle. Explicit import of a legacy main font assigns both roles once; subsequent role changes remain independent. A missing or unsupported profile is handled as a resource failure, not a guessed mapping or an unnoticed replacement of all legacy glyphs.

| Startup failure | Effective fallback |
|---|---|
| Requested text font/profile unusable | Bundled Cascadia Mono/profile, then bundled16x24x.pcf/profile; skip duplicate steps |
| Requested map font/profile unusable | Bundled16x24x.pcf/profile |
| Selected tileset unusable | Font-only map, message; preserve requested tileset and enabled preference for next startup |
| All applicable font fallbacks fail | System error identifying the failed resource; stop startup |
| PixelArt unsupported by actual preparation/backend path | Effective Nearest, show requested/effective distinction; preserve requested PixelArt |

Live resource-change failure retains the previous working stack rather than committing fallback as the requested value. Successful current PCF is never replaced solely for aesthetics or scaling. Updated assets invalidate only affected role caches/generations. Pack config/event volumes are shared, but selecting a pack or moving a category volume is a private CFG edit with explicit Save.

## OPT registry and semantic preservation

[All 200 lexical option keys](../../.scratch/single-window-sdl3-client/research/sv-option-ownership-draft.md) are the exhaustive source index: 201 initializer rows because `big_map` has two build variants; 12 keys are inside `#if0`. Preserve active-build defaults, boolean meanings, network slot identities and gates. Hidden is not inactive: server-used `view_reduce_light`/`view_reduce_view` and client-used `other_query_flag` still have consumers.

The earlier draft's L/U classifications are **not authorizations to remove display behavior**. Q37 explicitly requires meaningful old display preferences to survive and affect SV. The following disposition overlay covers those formerly disputed groups:

| Keys / family | SV effect and ownership |
|---|---|
| `exp_bar`, `hp_bar`, `mp_bar`, `st_bar` | Select number/bar presentation of the corresponding semantic status indicator; HTML defines its modern appearance |
| `mp_huge_bar`, `sn_huge_bar`, `hp_huge_bar`, `stun_huge_bar`, `st_huge_bar`, `solid_bars`, `huge_bars_gfx` | Preserve expanded-indicator visibility and presentation preferences in the corresponding HUD elements; layout eligibility remains meaningful. Do not lose the preference merely because symbols/Term coordinates are gone |
| `keep_topline`, `topline_no_msg`, `topline_first` | Control retention/visibility/first-line selection of the corresponding immediate game-message presentation. They do not erase occurrences, full recall or mandatory delivery; they are not blanket suppression of every message surface |
| `alert_offpanel_dam` | Keep alerts for the same originating interaction contexts that formerly suspended the ordinary game view; input-context ownership replaces screen_icky, not a guess from pixel occlusion |
| `highlight_player` | Toggle self-highlight semantic indicator; confirmed HTML cursor design supplies the shape |
| `overview_startup` | Select resistance/bonuses character-page preference as baseline; not a new instruction to open an overlay at login |
| `always_show_lists` | Preserve when selectable-item/spell lists are presented during prompts; new picker layout does not discard the option |
| `player_list`, `player_list2` | Preserve normal/compact/more-compact player-list presentation; HTML expresses the levels without a legacy Term |
| `time_stamp_chat` | Preserve baseline periodic timestamp marker behavior; do not reinterpret as a timestamp attached to every message |
| `target_history`, `targetinfo_msg` | Keep existing message-production/routing conditions and source occurrence semantics; no duplicate messages just to emulate a second terminal |
| `no_flicker`, `subterm_flicker`, `misc_no_flicker` | Preserve global animation switch and semantic scopes: ordinary main game content versus auxiliary content, with messages excluded from the misc-only suppression. Content roles supply scope; don't recreate windows |
| `thin_down_flush`, `disable_flush` | Preserve existing pacing choices and counter/skip semantics in presentation scheduling. No sleep may block packet apply/urgent render deadlines; use nonblocking due times rather than redefining these options as FPS or network batching |
| `use_color` | Retain existing supported coloured/monochrome presentation behavior in applicable contexts; hiddenness is not an automatic no-op |
| `gfx_autooff_fmsw`, `no2mask_weather` | Preserve the existing solid-wall adjustment and weather-composition branch preference, subject to corresponding source/build gates |
| `clone_to_stdout`, `clone_to_file` | Original SDL3 message clone semantics, off by default; destinations specified above; not a new session recorder |
| `screenshot_keys`, `screenshot_format` | [XHTML removed and bindings unified](../../.scratch/single-window-sdl3-client/issues/09-design-parity-evidence-and-acceptance.md#screenshot-bindings-unified--confirmed-2026-09-20): Ctrl+T/Ctrl+Shift+T and /shot / /screenshot invoke native PNG/BMP capture. Retire screenshot_keys swap; importing its old value does not change the action or restore XHTML. Preserve screenshot_format filename mode and no-silent-overwrite rule |
| `big_map` | Compatibility record does not override saved SV layout. Actual normal/big geometry and any required wire projection derive from layout; preserve protocol slots and avoid a competing source of truth |
| `gfx_palanim_repaint`, `gfx_hack_repaint` | Legacy repaint-technique controls remain recognized/inert when that technique is absent. This exception does not disable palette animation or meaningful display preferences |
| `#if0` and unavailable build records | Preserve imported records inert with an explicit unsupported disposition; do not reactivate absent capabilities or shift protocol slots |

Every other active option retains its source semantics through the reused core/presentation consumer and owns its value in SV global/character OPT. Display options are not all moved to global scope: only the new UI settings have explicitly global scope. HTML determines appearance, not whether an active native option works.

Deprecated aliases follow the exact branches in `process_pref_file_aux_aux` (c-files.c:1092 onward), including inversions/one-to-many conversion and deliberately discarded aliases. No guessed rename algorithm. Alias conversion does not modify a legacy source. Unknown OPT/PRF records keep baseline warning/skip-at-load behavior, but compatible source records are retained for non-destructive save/import reporting; recognized executable records are never executed by migration.

## Preference load order and boundaries

Defaults → shipped preference/mapping bootstrap → SV global options → applicable system preferences → selected font/graphics preferences → SV system options → SV character options at the original early pre-send point → baseline global/race/trait/class/character macro sequence. Preserve the existing later character/form reload points and local command effects; no silent addition of class-OPT autoload. Settings save targets independent snapshots, not whichever shared PRF last supplied the value.

`pref.prf` bootstrap must not implicitly reintroduce old user `global.opt` or `options.prf` as a settings base. Resolve option entrypoints to the S owner, use shipped defaults as baseline data, and route explicit macro/resource sources to the shared owner. An explicitly selected legacy options file is a load/import action, not automatic synchronization. Includes carry provenance and resolved ownership; a `.prf` suffix alone does not imply macro ownership.

Shared macro option directives deliberately affect the current session. Legacy Term-window W assignments do not. Macro loading is not migration: normal permitted execution gates remain as originally agreed. A settings Save does not flatten all loaded includes/actions into a shared file. Explicit macro modifications preserve unrelated records in mixed sources.

## Import and failure granularity

Offer import on first launch only if known SDL3 or standard X11 settings are found; user may select another source manually. No whole-disk scan, no silent mixture of SDL3/X11 profiles, no automatic resource-installation discovery. Imports and reimports are explicit. If setup was skipped without saving a CFG, the next absence-of-CFG launch remains a first-run situation; detecting settings still does not authorize applying them.

Preview groups: main CFG, global/character OPT, linked macro/include/stage sets, INS, DNA, resource preferences, input history, bookmarks, notes and credentials. Compatible groups remain selected by the earlier import policy; user controls conflicts. Already shared files are not redundantly copied onto themselves. Existing destination values win by default; replacements require explicit choice. Unsupported/inert settings and Lanczos→Linear substitutions are listed.

Identity of conflicts: CFG/OPT scope+key; macro set/file+trigger/stage identity without reordering precedence; INS whole ordered file; DNA normalized baseline filename; history whole account history; bookmarks whole bookmark file; notes whole source document. Treating a document/file as a group avoids guessing how to deduplicate identical legitimate events. A linked macro/include/stage group commits completely or is skipped; missing/cyclic/out-of-source includes prevent that group, not unrelated valid groups. Import does not fetch missing resources: retain requested references and apply the approved resource-failure policy.

Parse/stage without execution or source writes. Save failure retains prior destination group; independent successful groups are reported separately. Existing vault secret wins by default and is replaced only explicitly; vault failure cannot be reported as successful credential import. Imported source remains untouched. Saving settings never rewrites a credential merely because an account/server metadata field was saved.

## Credential identity and private inputs

Reuse [credential policy](../../.scratch/single-window-sdl3-client/issues/24-define-credential-storage-policy.md#answer) and [text-field dispositions](../../.scratch/single-window-sdl3-client/issues/35-decide-text-field-boundaries-and-legacy-defects.md#answer), including session-only fallback, exact raw bytes, protocol-star rejection, no hidden legacy-password fallback and immediate vault write after accepted password-change send.

Technical serialization: namespace `tomenet-sv/v1`; collision-free length-prefixed byte tuple of baseline server-address spelling, effective port and account bytes. Do not case-fold account or resolve a hostname to an IP merely to merge records. Encode the tuple as lowercase hex for a Windows case-insensitive TargetName; preserve the same identity tuple in Linux attributes/lookup. Provider length limits are checked before storage; failure follows approved session-only behavior, never silent key truncation. Identity/reference metadata does not contain secret bytes. Credential import destination identity is shown without revealing the password.

Archive/resource passwords remain temporary private input excluded from shared history/diagnostics. Existing independent message/character/screenshot/clone outputs continue; they do not gain access to account credential buffers or reversible representations.

## Completion and evidence boundary

This contract is a planning decision, not proof of native behavior. Acceptance must cover all keys/aliases/files and both target platforms: load→apply→Save→reload, unknown/incompatible/corrupt CFG, numeric boundaries, selected/effective fallbacks, resource disappearance/reappearance, role-specific font changes, options from common macros, namespace routing including raw rename/remove, private histories, shared INS/DNA/resource updates, concurrent external edits, group import failure, original server transfers, guide override/read/update and vault identities/exclusions.

No final claim is made from source counts alone: 200 lexical names include inactive/build-specific records. The HTML layout catalogue and explicit font/profile corpus are versioned evidence inputs, not invented values. Required performance/resource budgets remain with acceptance; user UI-scale/settings decisions do not invent those budgets. The next map ticket after this one resolves acceptance, followed by migration/fallback-retirement sequencing.


## Credential policy

Original `.scratch/single-window-sdl3-client/issues/24-define-credential-storage-policy.md`, SHA-256 `849829742ffbfd8fc1b3adadd9cfd30c53ef25c85d61d06de93adfb2576ff5f4`.



Решение согласовано последовательными ответами пользователя на Q1–Q10, включая отклонение «Забыть пароль» и немедленное сохранение при смене пароля. Тикет разрешён 2026-09-18. Это planning policy; код клиента, реальное чтение секретов и runtime acceptance не выполнялись.

### Storage и ownership

SV использует защищённое системное хранилище текущего OS user: Linux amd64 — Secret Service через libsecret с binary secret values; Windows10/11 i686 MinGW32 — generic Credential Manager (`CRED_TYPE_GENERIC`, `CRED_PERSIST_LOCAL_MACHINE`, `CredReadW`/`CredWriteW`). Windows persistence означает того же пользователя на той же машине, без roaming. Собственного master password SV нет; разблокировку системного хранилища обслуживает его provider.

Это явное исключение из shared-settings policy. Старый SDL3 не читает SV secrets; SV не записывает свой пароль в `tomenet.cfg`, `.tomenetrc`, INI, PRF или UI configuration. Существующий открытый пароль legacy config может оставаться: его cleanup вне SV. При записи общих настроек legacy password record сохраняется без замены значением SV; все startup/login/conversion/manual-save/shutdown writers должны соблюдать это исключение. Иные common settings сохраняют согласованное ownership.

Системное хранилище сохраняет точные credential bytes, без Unicode normalization и преобразования по font. Runtime/protocol `my_memfrob` не является шифрованием at rest и не определяет storage format; переходы между raw secret и core representation должны быть явными. Binary libsecret/Windows blob не навязывает UTF-8/UTF-16 secret payload. Metadata, labels, names и lookup keys не содержат секрет или его обратимое представление.

API evidence и ограничения: [Secure credential storage APIs](../../.scratch/single-window-sdl3-client/research/secure-credential-storage.md). Требуется usable persistent Secret Service provider; одно наличие libsecret не доказывает защищённую конфигурацию provider. Нельзя обещать изоляцию от произвольного кода с доступом того же OS user. Существующая legacy plaintext copy этим решением не защищается.

### Identity, login и import

Record identity — отдельный SV namespace плюс server address, effective port и account. Разные aliases/DNS names/IP автоматически не объединяются; серверное разрешение адреса не является основанием отправлять пароль другой identity. Account соответствует baseline login name semantics, без blanket lowercase. Windows case-insensitive target names не должны сливать разные case-sensitive account identities; конкретная collision-free serialization/reference schema принадлежит [Specify persistence ownership and UI configuration schema](../../.scratch/single-window-sdl3-client/issues/25-specify-persistence-ownership-and-ui-configuration-schema.md).

При обычном входе пароль загружается только для выбранной identity и доступен форме ввода. Успешный login автоматически создаёт/обновляет соответствующую защищённую запись. Автовход управляется отдельно от запоминания и по умолчанию выключен; имеющаяся запись сама по себе не включает автовход. Ошибка аутентификации возвращает ручной ввод и не удаляет сохранённую запись. Успешный вход с исправленным паролем обновляет её по тому же правилу.

Legacy password читается только при явном импорте с preview/conflict policy из [Define settings and migration boundary](../../.scratch/single-window-sdl3-client/issues/05-define-settings-and-migration-boundary.md). Существующий SV secret имеет приоритет, заменить его импортом можно только явно. Импорт привязывает пароль к выбранным server/port/account и сохраняет bytes точно; preview не раскрывает содержимое секрета. Невозможность записать secret не выдаётся за успешный импорт этой группы. Внешний источник неизменен; уже общий legacy password также не удаляется.

Отсутствие SV record, её внешнее удаление, ошибка чтения или отказ разблокировки не запускают автоматический fallback к открытому паролю старого конфига. Legacy loader не должен предварительно подставлять его в login memory, обходя это правило.

### Failure и session lifecycle

При недоступном хранилище, отсутствии службы или отмене разблокировки доступен ручной вход на текущую сессию. Открытого disk fallback нет. Отсутствующая запись отличается от ошибки доступа; UI объясняет невозможность загрузки/сохранения, не объявляя ошибку «пароля нет». Ошибка запоминания не превращает успешный игровой login в ошибку: текущая сессия продолжается, пользователь видит «Пароль не сохранён». Автоматических повторяющихся unlock prompts нет.

Операции хранилища не блокируют SDL event/render loop. Их завершение связано с исходной identity и текущей login operation; поздний результат после отмены/смены аккаунта не выполняет неожиданный автовход и не попадает в другой record. Промежуточные secret buffers освобождаются/очищаются после использования, runtime copy живёт лишь столько, сколько нужно действующему login/relogin contract; дисковая diagnostic copy не создаётся.

Отдельную опцию «Забыть пароль», UI для удаления records и remember-disable switch не добавляем. Это не требует запрещать пользователю управление записями средствами ОС; отсутствие записи после такого действия обрабатывается обычным ручным входом. Автоматическое запоминание после успешного входа сохраняется.

### Немедленное сохранение при смене пароля

После локального подтверждения нового пароля и успешной постановки запроса смены на отправку (`Send_change_password == 1` в baseline) runtime password обновляется и сразу запускается запись нового пароля в защищённое хранилище соответствующего аккаунта. Сохранение не откладывается до следующего login, не ждёт server acknowledgement и не разбирает текст игрового сообщения как подтверждение. Отмена формы, несовпадение повторного ввода либо локальная ошибка отправки не являются завершённой сменой и не запускают эту замену.

Успех записи означает только сохранение нового значения локально. Если сервер отклонит запрос или соединение оборвётся до обработки, защищённая запись всё равно может содержать новый пароль; автоматического rollback по текстовому сообщению нет. При следующей ошибке входа действует ручной ввод, после успешной аутентификации запись обновляется. Если сама запись в vault не удалась, текущая сессия продолжается с явным сообщением о несохранённости; нельзя объявлять новый пароль сохранённым. Перед заменой запись намеренно не удаляется.

### Outputs и границы

Секрет, его runtime/protocol representation и содержимое auth/password-change payload не попадают в presentation events, логи, diagnostic capture, обычные exports и конфигурационные dumps. Password fields не раскрывают значение в штатном UI; import preview сообщает о наличии/конфликте, а не печатает пароль. Ограничение относится к самим секретным полям, не отменяя обычные игровые сообщения и существующие message/character/screenshot exports.

Q3 не вводит новый exporter: отдельная функция выгрузки профиля с паролями и формат зашифрованного переноса не входят в этот контракт. Исходно согласованный legacy import сохраняется. Если отдельный перенос секретов когда-либо появится в новом scope, прежнее условие явного выбора не означает разрешения включать их автоматически.

### Follow-through и evidence

- [Specify persistence ownership and UI configuration schema](../../.scratch/single-window-sdl3-client/issues/25-specify-persistence-ownership-and-ui-configuration-schema.md): metadata/identity serialization и references, per-writer исключение SV password из shared files, import group commit/error semantics. Storage policy не выбирается заново.
- [Design parity evidence and acceptance](../../.scratch/single-window-sdl3-client/issues/09-design-parity-evidence-and-acceptance.md): synthetic-secret проверки обеих платформ, successful login/change-password writes, server reject/disconnect после отправки, locked/unavailable/cancel/save-failure cases, responsive UI и late completion, independent identities/case-sensitive accounts, legacy fallback prohibition и отсутствие секретов в outputs. Особо проверить сохранность legacy password при любом common-config save.
- Platform packaging должно включать клиентские зависимости libsecret/GLib для выбранного backend; отсутствие доступного persistent provider обрабатывается по session-only policy. Windows headers/import library не заменяют реального runtime evidence; actual blob limits и exact credential bytes входят в проверки. Конкретные dependency closure и OS scenarios принадлежат acceptance, а не новому product decision.

Новых decision tickets не требуется: точные schema и acceptance вопросы уже имеют владельцев. Resource-budget fog не меняется.


## Raster policy (subject to final persistence overrides above)

Original `.scratch/single-window-sdl3-client/issues/23-choose-raster-references-and-defect-compatibility.md`, SHA-256 `73ed49a2613ec706f54a81ecd01497bb23a88fc382eb291d574cdd5eca3ff4a7`.



Уточнение пользователя 2026-09-19 в [Specify persistence ownership and UI configuration schema](../../.scratch/single-window-sdl3-client/issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#comments): карта всегда использует 100% maximum fit, пользовательский map fit zoom отменён. Все нижеследующие упоминания регулируемого zoom и его хранения заменены этим решением; остальные geometry/raster invariants сохраняются.

Пользователь подтвердил итоговое общее понимание и закрытие тикета ответом «верно» 2026-09-18. Ниже — каноническая резолюция. Противоречащие ей ранние предложения и pending statuses в Comments являются историей обсуждения, не действующим контрактом. Реализация клиента и полная runtime acceptance не выполнялись.

### Compatibility и дефекты

Новый SDL3 pipeline является canonical raster pipeline. Legacy SDL3/X11 нормативны для semantic identity и behavior, но не для точного совпадения pixels. Pixel-perfect полностью исключён, включая закреплённые environments. Проверяются glyph/tile identity, geometry, color roles, layer order, visibility, lifecycle, presence/footprint/clipping/legibility/transparency и отсутствие bleed/stale output. Captures используются для диагностики и visual review, не pixel-equality gates.

Unsafe/lossy/misindexed/stale/misreported legacy defects исправляются; сохранение quirks требует доказанной зависимости и regression evidence. Encoding defects и glyph fallback следуют [Define encoding and glyph identity](../../.scratch/single-window-sdl3-client/issues/22-define-encoding-and-glyph-identity.md), а не воспроизводят ошибочные PCF origin/default/bounds или TTF raw-byte decoding. Курсор получает новый дизайн через [Prototype cursor and cell indicators](../../.scratch/single-window-sdl3-client/issues/31-prototype-cursor-and-cell-indicators.md), не обязан повторять X11 XOR или SDL3 alpha overlay.

### Координаты, UI и карта

- Layout/hit testing используют floating-point logical UI units; актуальные window/output-pixel sizes и OS display scale задают преобразование. DPI/monitor changes пересчитывают layout/raster без утраты session/interaction state. Fixed whole-window framebuffer с постоянным растягиванием не является моделью клиента.
- User UI scale увеличивает text/controls/rows/spacing с relayout независимо от map fit zoom. Обязательный OS DPI handling сохраняется. Отдельная системная accessibility text-size setting не обязана автоматически считываться; собственный UI control одинаков по смыслу на Linux/Windows.
- Уже согласованные [surface layout rules](../../.scratch/single-window-sdl3-client/issues/21-specify-surface-layouts-and-responsive-rules.md) сохраняются: primary/form text не auto-shrink, соответствующий content scrolls; отдельно принятые panels/canvases fit. Увеличение UI может уменьшать map area, но не менять selected viewport или map-font definitions.
- Map font/profile задаёт базовые пропорции клетки. Полная сетка 66×22 либо 66×44 вписывается и центрируется без crop/scroll; fit может увеличивать или уменьшать исходные assets. User map fit zoom до 100% maximum fit только уменьшает карту относительно максимального вписывания, не меняя server dimensions.
- Final cell size — общий для всей сетки целочисленный размер в output pixels. Небольшое отклонение aspect ratio от округления допустимо; точный рациональный ratio ценой крупных ступеней не требуется. Остаток площади даёт центрирующие поля. Все слои и hit testing используют ту же geometry.

### Подготовка assets и композиция

1. Source assets сохраняются в исходном разрешении. Encoded mask keys декодируются до фильтрации; authored base, recolor coverage, background holes, outline roles и независимые subsets сохраняют реальную семантику [legacy single/two-mask composition](../../.scratch/single-window-sdl3-client/research/sdl3-tile-composition-and-scaling-apis.md). Это не произвольное наложение необработанных masks поверх готового tile.
2. Decoded layers готовятся непосредственно под final cell size и кешируются. TTF rasterize выполняется сразу под конечные метрики. Промежуточный font-sized raster с последующим постоянным scaling всей карты не применяется.
3. Steady-state layers рисуются 1:1. Optional map target имеет конечный output size и также копируется 1:1. Terrain и foreground сохраняют независимые IDs/attrs; semantic stack: background/color role → terrain → foreground game visual → effects/weather → indicators/cursor. Palette применяется runtime tint, без пересборки нейтральных layers при смене цвета.
4. Cache identity учитывает source/profile generation, mask definitions, subset, final size, filter и outline policy. Готовая generation заменяется атомарно; конкретная cache organization и бюджеты здесь не выбраны.
5. При live resize допустимо временное аппаратное масштабирование предыдущих prepared assets; после короткого debounce готовится новая generation и возвращается вывод 1:1. Это ограниченное исключение, не постоянный double scaling. Game/model updates, palette/effects, hit testing и обязательные repaint не замораживаются и соблюдают [latency budgets](../../.scratch/single-window-sdl3-client/issues/20-classify-display-urgency-and-latency-budgets.md).
6. Аппаратный SDL_Renderer допустим; raw SDL_GPU не обязателен. Backend/fallback виден в диагностике. Быстродействие не считается доказанным наличием cache; supported software paths, debounce и resource limits требуют acceptance решения.

### Filters и fonts

| Asset role | Принятая policy |
| --- | --- |
| Authored-color tile regions | Готовые SDL Nearest / Linear / PixelArt |
| Decoded tile recolor coverage | Nearest; encoded mask colors нельзя интерполировать до decoding |
| Authored outline | Nearest |
| Generated outline | Рассчитывается при подготовке final size с настроенной толщиной и индивидуальными tile bounds; затем без дополнительного steady-state scaling |
| Raw pictures | Выбранный graphics filter: исходный rectangle → final size → 1:1; anchors/extents и special-canvas transform сохраняются, обычный tile-mask pipeline не навязывается |
| PCF | FreeType напрямую для encoded-ID/charmap/metrics/default handling; отдельный от tiles filter Nearest / Linear / PixelArt, default Nearest; source glyph → final-size cache → 1:1 |
| TTF | SDL_ttf, rasterize под конечные метрики; как минимум один TTF поставляется вместе с клиентом |

Legacy custom Lanczos не переносится и не является обязательной capability. Если PixelArt реально недоступен на renderer/preparation path, применяется явный Nearest fallback; requested и effective различаются в settings/diagnostics. Само наличие enum или успешный SDL_ScaleSurface call не доказывают PixelArt support: проверенный CPU surface path заменяет его на Nearest. См. [PCF loading and scaling options](../../.scratch/single-window-sdl3-client/research/pcf-loading-and-scaling-options.md).

Сглаженная PCF coverage разрешена его выбранным filter и не меняет Nearest policy для tile recolor/outline. Успешно загруженный PCF не заменяется другим размером/семейством/TTF ради вида или масштаба. Glyph identities, metrics, общий cell transform и изоляция соседних glyphs/tiles сохраняются. Конкретный bundled TTF/profile выбирается отдельно на основе [candidate research](../../.scratch/single-window-sdl3-client/research/bundled-ttf-candidates.md).

### Пользовательские assets и failures

Собственные fonts и tilesets обязательны, включая совместимые glyph/profile/mask/subset associations и mixed tile/glyph rendering. До активации проверяется загрузка и необходимая подготовка. Ошибка live change оставляет предыдущий рабочий набор и сообщает причину; missing glyph отдельно использует уже согласованный font-default/visible one-cell fallback, не приравнивается автоматически к отказу всего ресурса.

При startup load failure показывается ошибка и автоматически применяется fallback без resource-picker dialog. Это явное исключение из запрета автоматической замены выбранного ресурса. Конкретный резервный набор, последующие отказы и persistence requested/effective selection остаются у settings owner, а не считаются молча выбранными.

### Эффекты

Для UI recreation transient special-store animation применять последующее исключение из [Define bounded working retention and optional capture](../../.scratch/single-window-sdl3-client/issues/26-define-bounded-working-retention-and-optional-capture.md#answer); остальные effect lifecycle/timing правила ниже сохраняются.

Сохраняются виды эффектов, server parameters, palette/color roles, baseline timing rules, start/stop/restore и toggles. Совпадение случайных частиц и кадров мерцания с legacy не требуется. После исчезновения временного слоя показывается клетка по последним уже полученным и применённым данным, включая изменения под эффектом и другие активные слои; старый raster snapshot не откатывает эти изменения. Дополнительный server request и full-map repaint на каждом кадре не обязательны.

TERM_SRCLITE сохраняется как цветовая анимация специальных стен/столбов; его расчёт охватывает полную map geometry без legacy повторения рисунка в нижней половине big map. Смысл, цвета и timing сохраняются. Это не изменение обычного torch lighting.

### Coverage и владельцы дальнейших решений

Все bundled assets проходят basic discovery/load/metadata/range/default/failure validation. Глубокие semantic/lifecycle/composition scenarios используют representative corpus: ordinary 9x15.pcf, full byte-ID 9x15tg, mixed 16x24tg + 16x24sv, origin fixtures 12x24/8x16 и partial-range 16x22. Проверяются numeric IDs, PRF/graphics overrides, masks/subsets, ASCII weather/raw pictures, missing/default, font switching и отдельно text-only changes. Shipped profiles обязаны покрыть применимый полный baseline glyph corpus; replacements не доказывают parity. Controlled time/RNG делает effect scenarios проверяемыми без pixel-perfect contract.

- [Prototype cursor and cell indicators](../../.scratch/single-window-sdl3-client/issues/31-prototype-cursor-and-cell-indicators.md): конкретный cursor/indicator design, strokes и contrast/scale variants.
- [Specify persistence ownership and UI configuration schema](../../.scratch/single-window-sdl3-client/issues/25-specify-persistence-ownership-and-ui-configuration-schema.md): конкретный bundled TTF/profile, UI/zoom ranges/steps/defaults, graphics default, resource fallback chain, storage/import и conversion legacy Lanczos без порчи общих legacy settings. PCF default Nearest уже принят здесь.
- [Design parity evidence and acceptance](../../.scratch/single-window-sdl3-client/issues/09-design-parity-evidence-and-acceptance.md): exact environments/full corpus, supported accelerated/software paths, memory/render/legibility limits, debounce, lifecycle/failure scenarios и реальные измерения на обеих платформах.

Исследования и bounded Linux PCF smoke — evidence отдельных фактов, не доказательство finished-client parity, Windows dependency closure или performance. Закрытие этого decision ticket не закрывает перечисленные tickets и не разрешает реализацию в рамках planning-карты.


## Encoding and glyph policy (XHTML subsequently excluded)

Original `.scratch/single-window-sdl3-client/issues/22-define-encoding-and-glyph-identity.md`, SHA-256 `3371e2160dd08b722dd6a7613349010f66a2098cc29b88e640fd715cced47b90`.



Пользователь подтвердил итоговое общее понимание и закрытие тикета 2026-09-15 ответом «принимаю». Ниже каноническая согласованная резолюция: Q1–Q3, revised outgoing-text contract, Q8–Q10 и Q11–Q13. Прежний universal ISO-8859-1 default и automatic TTF→bitmap fallback не приняты. Implementation и runtime acceptance не выполнялись.

### Evidence и обязательная база

Визуал finished client должен сохранять как минимум информацию/поведение оригинальных SDL3 и применимых X11 paths: font-only map, mixed font/tiles, layered terrain/actor composition, custom glyphs, palette/animated attrs, cursor/weather/raw pictures, font reload и font-specific visual preferences. Ни metadata font, ни удачный ASCII screenshot не доказывают этой полноты.

Source reports: [Renderer parity inventory](../../.scratch/single-window-sdl3-client/research/renderer-parity.md), [all bundled font assets](../../.scratch/single-window-sdl3-client/research/encoding-font-assets.md), [server text/glyph contract](../../.scratch/single-window-sdl3-client/research/encoding-server-contract.md), [font/map/tiles lifecycle](../../.scratch/single-window-sdl3-client/research/font-map-and-tiles-contract.md). Аудит всех 100 bundled assets и 32 BDF подтверждает отсутствие единого declared charset. Default 9x15 ISO8859-1 — repertoire конкретного font, не universal network text contract. Runtime/raster acceptance не выполнялась.

### Три разных представления

| Данные | Нормативный контракт |
|---|---|
| Wire/legacy formatted text | Исходные bytes/markers и lossless ownership сохраняются; display projection отдельно |
| Собственные UI labels/text | Unicode representation; не устанавливает Unicode repertoire server-bound fields |
| Game glyph/tile IDs | Numeric identity, attrs, layers и coordinates сохраняются независимо от Unicode scalar representation |

Unsigned byte-symbol legacy text занимает одну logical cell. Supported formatting markers имеют отдельные semantics/visible length; Unicode projection не меняет server document/store coordinates. Byte capacities wire/input files отдельно от cells и Unicode text length. Char32 storage/transfer width не означает Unicode text protocol.

Successful complete decode предшествует model apply; partial packets остаются transport-owned без partial model mutation. Lossless unknown formatted bytes не считаются invalid UTF-8 по умолчанию и не выбрасываются. Unicode decoding применяется только к явно объявленным Unicode sources/profile correspondence, без charset угадывания. Исходный content удерживается на owning snapshot/event lifetime согласно state/retention contracts.

### Independent visual roles

| Setting | Scope | Что не затрагивается изменением |
|---|---|---|
| Text font и text UI scale | HUD/messages/lists/dialogs/documents | Map grid, map-font visual definitions и outgoing text encoding |
| Map font и map scale | Terrain/monsters/items/player/font cell effects | Text-font selection и outgoing text encoding |
| Tileset / graphics mode | Tile representations по visual mappings | Не отключает byte-glyph font paths |

Все roles живут внутри одного SDL_Window; per-Term/virtual-Term модель не переносится. Existing PCF и TTF modes сохраняются независимо для text и map roles. Server documents/special-store canvases имеют свои logical grids, отдельные от карты; own UI допускает обычную text layout.

Map cell geometry задаётся map font и его масштабом как baseline. Tiles масштабируются в ту же клетку. Glyphs/tiles/cursor/weather используют согласованную geometry; raw pictures сохраняют свои anchors/extents. UI text scale отдельно от map scale. Monitor DPI/viewport/resize и concrete surface layout задаются layout ticket; отдельный font-independent tile zoom этим решением не введён.

Graphics enabled сохраняет mixed rendering: representation определяется visual definitions, не правилом «все monsters/items обязательно tiles». Byte glyphs и ASCII weather остаются font paths. Foreground/background IDs/attrs/subsets и clear/keep sentinels не теряются; original attrs и timing не сводятся к однажды вычисленным RGB.

### Font switches и server-visible visual definitions

Explicit map-font switch сохраняет baseline lifecycle: проверка загрузки font; font-specific preference layer; затем graphics overrides; штатные core-owned client setup/font notifications и repaint. Glyph definitions могут намеренно изменяться этим настроечным действием. Font load failure не заменяет успешно работающий font ошибочным ресурсом по догадке; source-backed load-before-replace behavior сохраняется.

Text-only font changes не запускают map visual definitions reload. Unicode display conversion не изменяет received game IDs. Это различает legitimate visual preference changes, передаваемые server setup, и запрещённое implicit remapping из-за text conversion. Shared PRF/resource formats/loading/owners следуют migration/persistence policy.

### Glyph lookup и fallback

- PCF glyph выбирается по encoded ID с range/bounds check и корректным origin offset. Default encoded character разрешается через encoding table, а не принимается за bitmap index. Случаи bundled 12x24/8x16 с origin 1 и 16x22 с max127 входят в обязательный corpus.
- TTF получает Unicode own UI text напрямую; legacy byte-symbols сначала проходят explicit visual-profile ID→Unicode correspondence. Printable ASCII соответствует ASCII; другие IDs требуют заданного mapping. Profile не объявляет charset network text и не меняет numeric game IDs.
- Missing glyph использует корректный font default, затем видимый one-cell replacement с сохранением исходного ID. Shipped profiles должны покрыть полный применимый baseline glyph corpus: массовая замена missing symbols не доказывает parity.
- Custom bitmap glyph fonts сохраняются PCF path. Automatic TTF→bitmap fallback не является обязательным новым механизмом этого решения.
- Solid-wall ID рисуется filled cell; tile IDs проходят graphics lookup и соответствующую композицию. Control/high bytes не считаются автоматически Unicode controls: interpretation задаётся formatted/glyph context.

Accidental TTF UTF-8 decoding raw legacy byte strings исправляется; его искажения не являются oracle. PCF origin/default/bounds defects исправляются с отдельными regression fixtures. Остальные raster/defect dispositions принадлежат raster ticket.

### Formatting, copy и readable XHTML

Supported markers сохраняют baseline transitions/reset first/swap previous/neutral и doubled-0xff literal '{'. Unknown 0xff+code скрывает introducer и оставляет code в current color. Lone trailing 0xff безопасно завершает parsing без glyph; NULL pointer arithmetic/overread не сохраняются. Raw representation не теряет markers. Cell clipping/visible count и byte length различаются и проверяются отдельно.

Copy textual content строится из той же parsed text projection с explicit correspondence, не удаляя неизвестный visible code произвольным strip-pair helper. Readable XHTML сохраняет baseline reverse mappings и glyph substitutions (#/$/.) и '~' при отсутствии доказанного text correspondence, с корректным escaping. Это intentionally readable projection, не lossless archive или raster screenshot. Native screenshot показывает final composed visual; precise screenshot lifecycle/gates обсуждаются в acceptance.

### Input и clipboard contract

Game gestures и runtime user macro definitions сохраняют exact byte/context semantics; не проходят Unicode decoding. Text commits active editor отдельно от gesture actions исключают двойную вставку/command execution. Navigation, Enter/Esc, request replies и cancel sentinels следуют input-router baseline.

Outgoing text сохраняет существующий field-specific byte contract без repertoire extension. Точный correspondence и allowed bytes/limits задаются по field, не по выбранному font. Нет universal ISO-8859-1/UTF-8 network default и blanket ASCII policy. Before core handoff проверяются representability, окончательная byte length и штатные preprocessing/escaping rules. Unknown correspondence не угадывается.

Непредставимый текст вызывает понятную encoding error с draft retained; не превращается в guessed UTF-8 и не удаляется молча. Password normalization/case conversion/font-based transcoding не применяются, imported credential bytes сохраняются точно. Mechanism хранения credentials отдельно.

Clipboard использует ту же field representability/byte-limit проверку с предусмотренными baseline newline/paste escaping rules. Signed-char high-byte dropping и partial UTF-8 slicing не считаются корректным conversion contract. Empty accepted input и approved safe chat-expansion truncation остаются: encoding errors не вводят blanket reject/reopen всех overflow submits. Field-specific editor/wire capacity mismatches требуют atomic evidence/disposition, а не расширения server format по размеру editor.

### Проверяемая матрица и остающиеся владельцы решений

| Проверка | Обязательный outcome |
|---|---|
| Text-only font/scale switch | Map font/grid/tiles/mappings и outgoing bytes сохраняются |
| Map-font switch при graphics off/on/dual-mask | Font preferences → graphics overrides → baseline notifications; UI text choice независим |
| Font-only / tiles с byte glyphs / ASCII weather | Correct glyph/tile identity, attrs и актуальная cell composition |
| PCF origins/partial tables/missing/default | Correct lookup без OOB; поддержанные glyphs не заменяются ошибочным default |
| TTF profile ASCII/high/control/custom IDs | Explicit correspondence, ID retained; full corpus coverage, не raw UTF-8 guessing |
| DPI/font reload с tiles/cursor/weather/raw pictures | Согласованная geometry, repaint без stale cached output и без утраты logical state |
| Marker valid/unknown/doubled/trailing и clipping | Утверждённые visible glyphs/colors/cell positions; raw bytes retained |
| Copy/XHTML/native screenshot | Parsed text consistency/readable substitutions отдельно от final raster capture |
| Text commit/macros/paste/account/password/request boundaries | Exact permitted bytes и replies; errors preserve draft; baseline cancel/empty/truncation сохранены |

Atomic source/field contract inventory требуется в [Enumerate source text and server-field byte contracts](../../.scratch/single-window-sdl3-client/issues/27-enumerate-source-text-and-server-field-byte-contracts.md); результаты не устанавливают новый repertoire. Это sharpened research question, не реализация. Profile/resource/settings registry и defaults/ownership — [Specify persistence ownership and UI configuration schema](../../.scratch/single-window-sdl3-client/issues/25-specify-persistence-ownership-and-ui-configuration-schema.md); конкретные reference fonts/maps/raster compatibility — [Choose raster references and defect compatibility](../../.scratch/single-window-sdl3-client/issues/23-choose-raster-references-and-defect-compatibility.md); DPI/geometry/layout — [Specify surface layouts and responsive rules](../../.scratch/single-window-sdl3-client/issues/21-specify-surface-layouts-and-responsive-rules.md); harness/configuration/platform/runtime evidence — [Design parity evidence and acceptance](../../.scratch/single-window-sdl3-client/issues/09-design-parity-evidence-and-acceptance.md).

Memory/render resource budgets и terminal fallback removal fog без изменений. Implementation, C conversion signatures, generated mapping tables, renderer build и release не выполнялись. Маршрут карты остаётся planning.


### Final confirmation — 2026-09-15

Пользователь принял итоговую резолюцию и закрытие тикета. Status изменён на resolved; context pointer добавлен в Decisions so far карты. Linked layouts/raster/persistence/acceptance и новый source/field research ticket сохраняют собственные ещё не разрешённые вопросы.

### Cross-ticket scaling amendment

[Choose raster references and defect compatibility](../../.scratch/single-window-sdl3-client/issues/23-choose-raster-references-and-defect-compatibility.md) уточняет display scale: map font/profile по-прежнему задаёт внутреннюю glyph/cell geometry и tiles используют ту же клетку, но полный map composition canvas затем получает maximum uniform fit в allocated map area и отдельный user zoom до 100% этого fit. Это не меняет glyph identity, font-specific visual definitions или independent text-font role.

В принятом там решении «Prepared assets and 1:1 composition» canvas обозначает логическую композицию: исходные assets подготавливаются сразу под конечную клетку, TTF rasterize выполняется в конечных метриках, обычный вывод идёт 1:1. Промежуточный raster в размере map font с постоянным повторным scaling отменён; прежний encoding и visual-preference lifecycle сохраняется.

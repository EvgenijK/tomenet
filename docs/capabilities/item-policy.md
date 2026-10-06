# Item-domain planning authority

Snapshot at `bbe7417e21d20470a7aeb075aa82650608f6e4ae`. Planning requirements, not native evidence.

## Acceptance sequence

Original: `.scratch/single-window-sdl3-client/migration-sequence.md`; SHA-256 `cc83546a1fefed557127a0fe1513e0dadb02c6c8a50efa9b7ee212993b8b691f`.

# Single-window client: implementation sequence

Status: approved planning contract; Q1–Q6 confirmed by the user on 2026-09-20;
Guide deferral and pregame-B amendments confirmed by the user on 2026-10-05.

Decision owner: [Sequence surface migration and retire terminal fallback](issues/29-sequence-surface-migration-and-retire-terminal-fallback.md).
Normative checks: [Acceptance contract](acceptance-contract.md). This is a plan, not implementation or runtime evidence.

## Approved framework

A–G checkpoints are cumulative. Dependencies needed by a capability (input, errors, persistence, resources, platform behavior) arrive with it, even if their broader feature family completes later. HTML completeness does not block native work. Previously accepted capabilities receive regression checks; future capabilities remain explicitly pending.

Guide is the explicit exception to ordinary prerequisite pull-forward. All permanent
Guide outcomes, Guide bookmarks and Guide update/management are accepted only in G.
At every Guide entry point in A–F the native client displays exactly
`The guide is in development` and returns to the actual caller without reading Guide
content, opening bookmark state, sending a Guide request or entering fallback. The
placeholder is temporary stage behavior, not coverage of a Guide capability.

B has a second explicit allocation boundary: it accepts the native pregame screens
through a managed protocol peer, not a live TomeNET session. The peer replaces only
the external endpoint; production transport/protocol, model, interaction, input and
UI paths remain under test. Real-server integration and the first game screen begin
in C. The former aggregate `session.enter-game` dependency must not pull profile,
FILE/Lua, game resources, map, HUD or gameplay back into B.

Linux-first; MinGW32 build from A, Wine smoke each checkpoint, Windows 10/11 VM checks at B/E/F/G and earlier for platform-specific changes. Linux software rendering starts with the first rendered slice; Windows software rendering with the first Windows runtime. Claims identify actually checked environments; Wine is not Windows acceptance.

## Stage detail

| Stage | Capabilities and required evidence | Known limits after this stage |
|---|---|---|
| A — runnable foundation | Separate executable/object paths, one SDL window, production model/command interfaces and renderer exercised by a synthetic decode→state→surface→input scenario. Prove complete/incomplete packet handling for this slice, ordered events, cancellation, resize/focus and software rendering. Create/validate full source-backed atomic manifest, native ledger and allocation to A–G; HTML snapshot/ledger may honestly report missing. | No claim of real login or gameplay; synthetic scope only. Later capabilities have named owners/stages and pending evidence. |
| B — native pregame screens | Fully working native flow for endpoint/connect, account creation/authentication, character overview/management, character creation and peer-driven MOTD. Exercise production UI/model/input/transport/protocol paths through a managed protocol peer, including success/rejection, back/cancel/retry, fragmented input, disconnect and stale-session cleanup. Include field byte rules and protected credentials required by these screens. | No live-server compatibility or gameplay claim. Post-MOTD startup, profile/FILE/Lua/game resources, first map/HUD and every gameplay/session-end flow remain pending. |
| C — live session and game actions | Connect the accepted pregame flow to real TomeNET servers; complete live negotiation and post-MOTD startup, profile/preferences/macros, FILE/Lua and resource prerequisites, then reach the first authoritative map/HUD state. Accept the former B gameplay/session scope (messages/chat, basic actions, disconnect/reconnect/exit/death and its actual dependencies) together with items/inventory/equipment, targeting/look/directions, spells/skills/ghost/mimic/runes/stances/techniques, ordinary stores and associated confirmations/requests. Preserve keysets, user macro load/play/waits, mouse intents, cancellation/retry and return to parent. Verify real-server round trips, version/wire/Lua gates, map geometry/rendering, store transactions, slot identity and multi-step macro chains. | Information/social/document/special-store and remaining integration flows stay pending unless required by an accepted C outcome. |
| D — information and server surfaces | Remaining character/knowledge/social flows, housing/utilities and applicable admin/DM flows, local and arbitrary server documents, search/page navigation, special-store canvases/animations and relevant server-driven controls. Verify lossless content, ordered replies, partial updates, fit/scroll rules, close/reset/recreation and no repeated side effects. | Remaining E settings/resource/file/audio/platform outcomes are pending; generic source-preserving documents/canvases are native surfaces, not terminal fallback. |
| E — integration completeness | Finish all non-Guide settings/import/save/cancel, visual preferences/fonts/tiles/filters/effects, audio/packs, macros editing/recording, files/exports/screenshots and clipboard/OS associations. Cover every remaining non-Guide in-scope atomic outcome, optional-feature configurations and provider/disk/resource errors. Prove independent CFG/OPT/history and shared-file ownership. All stage acceptance scenarios run with terminal fallback disabled; Guide entry points still use the native placeholder. | No missing native capability remains except the explicitly deferred Guide family and its bookmark/update data. |
| F — core acceptance | Build without terminal-fallback linkage; verify every non-Guide active ID has current evidence and no unclassified non-Guide legacy route remains. Run required target archives in isolated profiles, Linux ABI/dependency checks, Windows 10/11 runtime, optional configurations, software rendering, complete non-Guide regression and human review. | Guide remains unavailable by decision; every Guide entry point must still show the exact native placeholder. F is not final product acceptance while G is pending. |
| G — Guide and final acceptance | Replace the placeholder with the complete native Guide: local/bundled content, navigation/search/help, caller-specific and server-directed opening, bookmarks, copy/chat integration, checksum/reload/update and all related persistence/platform/error paths. Re-run complete Linux/Windows, software/accelerated, optional-build, lifecycle and human regression; verify all active IDs have current evidence and no placeholder or fallback route remains. | Only approved exclusions and explicitly documented environment limits; no pending required capability, placeholder entry or runtime check can pass final acceptance. |

Canonical active-outcome allocation after the 2026-10-05 amendments is
**A8/B54/C615/D162/E64/F1/G21**.

Every stage applies the existing acceptance layers to its scope, including short concurrency/lifecycle cases and submission gates 20/50/200 ms where applicable. No stress/soak, XHTML or global memory ceiling is added. Rendering/fonts and OS dependencies needed earlier cannot be postponed to E. Slash verbs, bindings, packet variants and local/Lua flows are allocated by their outcomes, not treated as a separate deferred umbrella feature.

## Registry and stage gates

The planned canonical files do not yet exist. A creates the schema, source-backed atomic manifest, native coverage ledger and full stage allocation from the completed inventories. Never fabricate IDs in this planning document or infer coverage from a broad family label. Each active in-scope outcome has one acceptance stage, its prerequisites, evidence requirements and a native/fallback/pending disposition. Cross-cutting scenarios reference all involved IDs. Shared primitives do not establish coverage of all callers.

A changed source or newly discovered outcome updates the registry and stage allocation before acceptance of the affected scope. If needed by an earlier claimed flow, it is not deferred merely to preserve a passed status, except for the explicit A–F Guide placeholder and B pregame/live-session boundary above. A broad existing outcome is kept in the later stage or refined into independently testable outcomes; it does not import later behavior into B merely through its old prerequisites. XHTML is explicitly excluded under the approved policy. HTML coverage and UX approval remain separate and truthful; missing HTML functionality cannot fail native acceptance.

Each checkpoint records current revision/configuration/environment, new accepted outcomes, regression results, remaining limitations and fallback inventory. Earlier accepted flows receive regression checks; changed dependencies invalidate affected evidence. Missing/failed/stale checks fail the corresponding claim. Exact test code and harness design belong to implementation.

## Native/fallback transition and retirement

A single input router owns logical input context, macro queue and pending requests. Development fallback is an allow-listed adapter for specific future capabilities inside the same SDL window; there are no permanent virtual Terms and no native reads from terminal buffers.

Routing is selected at a named flow/context entry, never as catch-all recovery from an unknown key or native error. A child flow may use fallback only when explicitly declared as pending; the parent is not claimed to cover that complete child outcome. Passing control restores the true parent and preserves the baseline queue/reply/cancel rules; no duplicated input, command or response. Networking, timers, model updates and required urgent rendering continue while fallback is active. Relog/teardown invalidates both native and fallback session context.

Each entry records its owning flow/module, replacement stage and removal check. Diagnostics contain only safe IDs/reasons/counts, never user content or secrets. Accepted flows entering fallback fail regression. Disable a route as its native replacement is accepted; remove unused adapter code incrementally. E demonstrates all scenarios with terminal fallback disabled. F excludes adapter linkage and combines build/source route checks with runtime evidence; zero runtime hits alone cannot prove zero dependency. The Guide placeholder is a native unavailable-state, not fallback. G removes that placeholder only after the complete Guide replacement is accepted.


## Approved layout decision

Original: `.scratch/single-window-sdl3-client/issues/21-specify-surface-layouts-and-responsive-rules.md`; SHA-256 `b7e85981bd1ead721540d685f02261bb83b9e323ab1d656bf9a2673334510f28`.



### Окно, единицы и масштаб

- Клиент использует один `SDL_Window`. Минимальная поддерживаемая client area — **1024×768 logical units**; OS DPI переводит logical units в surface pixels. Window decoration не входит в размер client area.
- Text font/UI scale и map font/cell scale независимы. Text-only change не меняет map setting/visual definitions; map-only change не меняет UI text. DPI/monitor change пересчитывает raster/layout без потери текущего interaction state.
- Карта всегда сохраняет полный выбранный server viewport: normal **66×22** или big **66×44**. Desired cell size — максимум; вся сетка proportionally fits с сохранением aspect ratio, без crop и map scroll. Primary open/close не меняет viewport.
- Точный glyph/raster reference, minimum legibility/profile bounds и rounding относятся к raster/acceptance owners. Предложенные в обсуждении dynamic minimum-window formula и minimum map width396 не приняты.

### Две выбираемые компоновки

**Wide layout** сохраняет left panel, map и right panel. Пользователь вручную меняет ширины panels; resize и DPI сами их не меняют и не перестраивают shell. Обычный primary может перекрывать карту и некритические части panels, но не видимый critical HUD. При явном возврате из small layout включается big_map66×44, возвращается right panel и убирается small live feed.

**Small layout** выбирается пользователем вручную. Он скрывает right panel, включает normal map66×22, visually fits left panel по высоте без outer scroll и показывает над картой общую ленту последних примерно пяти отображаемых строк всех игровых сообщений, включая chat. Это краткий live projection; полный recall остаётся Messages primary. Игрок может вручную скрывать отдельные left-panel blocks через settings. Компоновка не переключается автоматически.

При logical width≤1280 или height≤800 клиент один раз за app launch ненавязчиво предлагает small layout: «Включить малую компоновку» / «Не сейчас». Предложение не получает focus и не меняет state самостоятельно; active command, macro или server request откладывает его до ordinary game input. Manual Settings сохраняет тот же input/request contract. Persistence, defaults/panel constraints и startup precedence между SV layout preset и shared `CO_BIGMAP` принадлежат [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md).

В wide layout right panel сохраняет выбранные виджеты и размещение. Panel shell целиком не прокручивается и visually fits при нехватке высоты; списки внутри widgets могут прокручиваться. В small layout right panel hidden, но все его outcomes имеют primary entry points. Messages является primary, независимая раскрываемая feed surface не вводится.

### Primary surfaces и content rules

Одновременно открыт один primary. Он центрируется в safe area всего окна, может перекрывать карту/некритические данные и не ограничивается central map column. Resize clamps frame без закрытия interaction, сброса selection, draft, scroll/position или focus owner. После завершённого interaction раздел повторно инициализируется по baseline, без нового cross-open recall.

Existing HTML surfaces сохраняют свои подтверждённые desired sizes. Для новых surfaces применяются logical desired maxima:

| Семейство | Desired maximum |
|---|---:|
| Простые меню, typed lists, normal store |720×690 |
| List/tree + details, multi-column pages |1120×720 |
| Generic formatted document viewer |1180×760 |

Frame всегда clamps к safe area. Внутренние колонки автоматически не превращаются в tabs, primary text не уменьшается; overflowing region получает собственный scroll согласно типу содержимого.

- **Typed lists**: длинное имя wraps, semantic key/slot, count и price остаются видимыми; body scrolls vertically. Header, navigation и required actions pinned outside body. Selected row/focus остаётся видимым после resize.
- **Detail/Guide/lore**: применимый approved HTML UX даёт readable wrapped article/detail projection. Original source lines/markers и unknown content сохраняются и доступны через source-preserving no-wrap viewer; local search/bookmarks/navigation остаются у исходного owner.
- **Server-formatted documents** (`SPECIAL_FILE_*`) сохраняют original bytes, colors, rows/columns и unknown content. Automatic wrap отсутствует; horizontal scroll открывает wide lines, vertical navigation/search сохраняет штатные server page requests. Длинный документ не shrink-to-one-screen.
- **Special stores**: text/grid/raw pictures/animations/clear regions живут в одном reference canvas и проходят один uniform transform, complete fit без scroll. Text/tile anchors используют cell coordinates; raw picture extents могут занимать несколько или дробное число cells и не сжимаются в one-cell slot. Controls/prompts находятся вне canvas. Exact resource/profile/rounding oracle остаётся renderer/raster owner.
- **Map targeting** временно скрывает перекрывающий primary по принятой interaction model; HUD/live feed остаются. Finish/cancel/retry и возврат selection определяются baseline command, не layout.

### Children, focus и ввод

Quantity/text/name/password/key/confirm/item/source/slot/direction/target и packet-driven requests — children owning interaction. Child размещается поверх current primary в safe area; question wraps, editor/required controls остаются visible, long choice list scrolls. Resize сохраняет draft, selection, caret/focus и active step. Enter/Esc/answers/abort/restoration следуют конкретному baseline context; layout не вводит универсальный cancel/retry и не исполняет command повторно.

Input router/macros/chat сохраняют решения [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md). Mouse entry points разрешены как дополнительные bindings, но не заменяют keyboard/macro routes. ordinary primary или layout offer не расширяют availability chat/settings во время запроса.

### Pregame, MOTD и session end

Connect/server, account/character overview/reorder, creation, setup MOTD, gameplay и death/end/relogin — sequential whole-window scenes в baseline order. До active gameplay игровой HUD отсутствует; session teardown закрывает старые interactions. Long forms scroll с visible required controls, без automatic form-font shrinking. EOF/quit/death/relogin дают только штатные paths; universal reconnect/resume отсутствует.

Setup MOTD показывается после character selection/creation и до `Net_start`/карты. Его23×120-byte server-defined rows, colors и ASCII layout сохраняются без wrap и proportionally fit complete scene; any-key, `-m` skip и once-per-login behavior остаются baseline. Concrete UX вынесен в [Prototype the server-defined MOTD screen](28-prototype-the-server-defined-motd-screen.md), с [reference screenshot](../motd-reference.png).

### Coverage и проверка planning artifact

[Surface coverage and geometry evidence](../layout-coverage.md) распределяет все36 baseline family IDs и scoped/delegated prompts между persistent HUD/map, primary, child, source viewer, special canvas, target и whole-window form/system scenes. Это allocation для implementation spec, не runtime parity claim.

[Layout prototype](../layout-prototype.html) — throwaway geometry fixture. Browser pass:240 combinations (20 surfaces ×3 dimensions ×2 layouts × text12/24) без frame/map-fit/cell-count/default-draft failures. Отдельно проверены2/3-pane layouts при1024/1920, selected-row visibility/focus, form and child drafts, deferred offer→small22 rows/right hidden и explicit wide→44 rows. Console errors0. Captures: [small1024×768](../layout-small-evidence.png), [wide1920×1080](../layout-wide-evidence.png).

### Cross-ticket scaling amendment

[Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md) уточняет прежнюю формулировку `desired cell size — максимум`: полный viewport сначала получает maximum uniform fit в allocated map area, затем независимый user map fit zoom может только уменьшить и центрировать composition. Font/profile задаёт внутреннюю cell geometry и aspect ratio; zoom не меняет normal/big viewport или layout mode.

Последующее решение «Prepared assets and 1:1 composition» в том же тикете определяет вывод после fit/zoom: общий целочисленный final cell size, центрирующие поля от округления и подготовленные assets с композицией 1:1. Постоянно растягиваемый промежуточный raster canvas больше не является требованием layout; временное масштабирование допускается в live resize по контракту raster ticket.

Synthetic rows/forms/maps не являются capability implementation, native input/network proof или raster oracle. Atomic manifest/ledger evidence, exact field encoding, retention, persistence registry/defaults, reference raster profiles and platform acceptance остаются у существующих linked tickets. Runtime client и release artifacts не создавались.

Primary-source capture сохранён вне рабочей ветки: `prototype/single-window-surface-layouts`, commit `13b1f40fad2031cf1cf4a118c23904d56dffd303`. Commit содержит только layout prototype, coverage ledger, wide/small evidence captures и supplied MOTD reference; текущая ветка и пользовательский index не менялись.

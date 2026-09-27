# Stage A — runnable single-window foundation and complete capability registry

Label: ready-for-agent
Status: specified

## Problem Statement

The new single-window client has an approved architecture and implementation sequence, but there is no source-backed runnable SV foundation or populated atomic capability manifest to guide implementation. The current SDL3 client couples packet handling, input and presentation to legacy terminal state. Reusing its executable or displaying a mock screen would not establish the agreed semantic interface.

Developers need an independently buildable starting point that exercises real decode, state publication, native rendering and command handling. They also need a complete, auditable allocation of the behavior baseline to stages A–F, so that later work cannot lose a capability behind a broad family label or confuse an HTML mock with native implementation evidence.

## Solution

Deliver stage A of the approved implementation sequence: a separate Linux amd64 and Windows i686 SV target, a runnable synthetic scenario inside exactly one SDL window, and the complete canonical capability registry with a native coverage ledger and stage allocation.

The scenario uses production code for the migrated paths. Synthetic server bytes enter the real decoder; published semantic state drives native surfaces; native input reaches the command interface and real response serialization. The fixture substitutes the external peer and clock where necessary, not the behavior under test. This establishes the foundation for stage B without claiming a working login, game session or complete gameplay UI.

The registry covers the entire in-scope destination, including stages B–F. Runtime evidence in A covers only the foundation and its explicitly selected flows. Future capabilities remain visible and pending. The stage is complete only when both the runnable foundation and the full registry pass their respective gates.

## User Stories

1. As a developer, I want to build the SV executable separately, so that I can develop the new client without overwriting a legacy client.
2. As a developer, I want isolated Linux and MinGW object outputs, so that one configuration cannot accidentally reuse another configuration's compiled code.
3. As a developer, I want the Windows i686 target available from the first stage, so that platform constraints surface before most UI work is complete.
4. As a reviewer, I want to launch a synthetic scenario without a real account or server, so that I can inspect the foundation reproducibly.
5. As a reviewer, I want exactly one system window, so that the foundation demonstrates the intended single-window client.
6. As a reviewer, I want decoded status changes to appear in a native surface, so that the presentation path is observable end to end.
7. As a reviewer, I want incomplete packets to leave visible and semantic state unchanged, so that network fragmentation cannot expose partial updates.
8. As a reviewer, I want adjacent complete packets consumed in order, so that one handler cannot corrupt the following packet.
9. As a reviewer, I want repeated identical events to remain distinct, so that event delivery is not confused with latest-state replacement.
10. As a reviewer, I want a server-defined request to accept or cancel through native input, so that its exact response and lifecycle can be checked.
11. As a reviewer, I want status updates to continue while a request is open, so that a modal surface does not stop the session model or rendering.
12. As a reviewer, I want resize and focus changes to preserve the pending interaction, so that the foundation does not depend on a terminal screen stack.
13. As a reviewer, I want reconstruction of a surface to avoid repeating a command or event effect, so that rendering remains separate from execution.
14. As a reviewer, I want session teardown to invalidate old requests and queued input, so that a later session cannot receive an obsolete response.
15. As a developer, I want keyboard and macro processing to use the agreed input router, so that later widgets do not acquire separate command interpreters.
16. As a reviewer, I want the foundation to run with software rendering, so that correctness does not depend on accelerated graphics.
17. As a reviewer, I want logical and output coordinates handled correctly at different display scales, so that controls remain readable and input matches the visible surface.
18. As a reviewer, I want measured decode/input-to-frame-submission timing, so that responsiveness claims have evidence.
19. As a developer, I want bounded working state with explicit ownership, so that temporary fixtures do not establish an unbounded recording architecture.
20. As a developer, I want the foundation to use isolated test data and approved resource ownership, so that verification does not overwrite personal legacy settings.
21. As an implementer, I want every independently testable baseline outcome represented in the manifest, so that planning does not stop at broad capability families.
22. As an implementer, I want stable IDs for capabilities, surfaces, actions, states, bindings and input contexts, so that different consumers can refer to the same product model.
23. As an implementer, I want behavior, protocol, UX and acceptance sources distinguished, so that a mock or screenshot cannot redefine game behavior.
24. As an implementer, I want each outcome allocated to an acceptance stage with prerequisites, so that I know what must be delivered before a checkpoint can pass.
25. As an implementer, I want source inventories reconciled with the manifest, so that rare commands, version branches and platform-specific flows are not silently omitted.
26. As a reviewer, I want native implementation status separate from evidence status, so that code presence does not imply accepted behavior.
27. As an HTML prototype maintainer, I want a validated local manifest snapshot and explicit coverage gaps, so that the prototype can evolve without claiming native parity.
28. As a reviewer, I want human UX approval separate from prototype completeness, so that an existing checklist is not treated as approval evidence.
29. As an implementer, I want source changes to invalidate affected evidence, so that an old successful run cannot certify changed behavior.
30. As a developer, I want every temporary terminal fallback route assigned an owner and replacement stage, so that temporary compatibility cannot become the permanent model.
31. As a reviewer, I want accepted native flows to fail checks if they enter fallback, so that migration progress cannot silently regress.
32. As a reviewer, I want the stage report to distinguish Linux, Wine and actual Windows observations, so that platform claims match what was tested.
33. As a next-stage implementer, I want reproducible verification commands and an explicit list of remaining limitations, so that I can continue from A without reconstructing its assumptions.

## Implementation Decisions

### 1. Scope and existing code

- Follow the user's 2026-09-22 isolation policy in [AGENTS.md](../../AGENTS.md): minimize changes to legacy and common files; local production implementations in SV may duplicate baseline code to avoid dependency expansion. Preserve version-aware protocol and command/macro semantics. Record independent legacy/common improvements in [the improvements list](../../docs/sv-improvements.md). Earlier shared-implementation wording does not require modifying legacy callers.
- Existing SDL3 build definitions already distinguish Linux and MinGW objects and assemble shared client, network and Lua code. Extend that prior art with independent SV outputs. Executable names are `tomenet-sv` and `tomenet-sv.exe`; Windows remains i686 and Linux remains amd64 with the agreed Fedora41-class shipping baseline.
- Current HP handling decodes into player state and directly invokes terminal presentation. Current key requests enter a synchronous legacy prompt. These paths need separation to satisfy the new model; calling them unchanged behind a mock view is insufficient.
- Old ignored modern objects and executables are not implementation sources. Do not infer missing source interfaces from them, link them into SV, or make their restoration a prerequisite. Their cleanup is not an A deliverable.
- A is a development checkpoint. Its startable synthetic mode is explicitly identified as synthetic, uses an isolated profile and requires no real account, secret or live server. It does not publish a production-ready player package.

### 2. Production interfaces and test seams

- Use the already approved Session presentation model interface: version-aware decoded changes enter the model; native UI reads coherent read-only views and ordered events. UI actions use the separate command interface, owned together with logical input context by the input router. This preserves the interfaces agreed in the map rather than adding a second test-only model.
- Prefer suitable existing storage inside the model. Do not maintain separately mutable copies of a fact merely to give it a new interface. A read-only view need not copy all state every frame, but cannot expose partially applied changes.
- Keep transport and version decisions in the core. Keep privileged operations in their existing controlled core ownership; the model receives only appropriate presentation results. Neither native renderer nor widgets inspect protocol version predicates or terminal buffers.
- The highest integrated test seam is the runnable SV scenario: synthetic peer input and user events enter production paths; assertions observe model results, emitted response bytes, visible outcomes and successful frame submission. A controlled transport boundary is sufficient to capture replies without a real network session.
- Fixture configuration and controlled time/RNG are external inputs to the scenario. Do not create a second decoder, command interpreter or renderer for tests. Lower-level tests may isolate the model or input router through the same production interfaces when they make a failing invariant easier to diagnose.
- The registry validator is a separate data-tool interface: canonical input and consumer data enter; a validation report and process result leave. It is not an additional game-core seam.
- Concrete C signatures, module/file partitioning, fixture syntax and test runner are implementation choices constrained by these interfaces. The approved architecture and A exit criteria already establish the expected seams; this synthesis does not reopen them.

### 3. Required vertical scenario

Use a small, source-backed scenario that exercises all A obligations. The concrete starting slice is HP/status, repeated message events and a generic key request with its real reply. This choice limits the first implementation while exercising snapshots, ordered events and modal ownership; it does not migrate all gameplay or document surfaces.

1. Start an isolated synthetic session with an explicitly selected supported server version and build configuration. Open one native window with a status surface, an event/message surface and the ability to present the key request. The fixture owns initial test data, not gameplay truth.
2. Feed valid HP and message bytes through production decode/apply. Cover both applicable HP wire layouts, including the selected layout's existing value/marker semantics. Retain the original text/field identity required by the encoding and packet contracts instead of recovering it from rendered text.
3. Split fixture packets before completion. Until complete decode, publish no state mutation, event, prompt, reply or side effect from that packet. After completion, apply once in input order; an appended known packet remains correctly readable. Use malformed cases for which the existing packet contract defines failure, preserving that failure behavior.
4. Deliver two identical valid message occurrences and prove distinct delivery. Changing a latest-value status may coalesce rendering, but does not coalesce occurrences or duplicate effects during redraw.
5. Deliver a generic key request retaining its request identity and prompt. Respond through the production input router and response serializer. A valid key sends the corresponding response once; Escape sends the baseline key-request cancellation value of zero, not an invented universal cancellation code. Exercise server abort according to its owner-specific baseline contract.
6. While the request is pending, deliver another status/event update and exercise resize, focus loss/gain and view reconstruction. The request and logical input context survive; updates continue; return to the parent is correct. Focus gain itself generates no command and does not clear an accepted macro queue.
7. Exercise a representative accepted macro/input sequence through the reused processing path, respecting the request context's actual macro policy. Capture exact ordering and absence of duplicate dispatch. This is foundation evidence, not a claim that all macro edge cases or command contexts are implemented.
8. Tear down the synthetic session with a request pending, then create a new session. Old input, request identities and undelivered session-owned effects cannot act on the new session.

The scenario is automated where outcomes are machine-observable and also runnable for native visual/input review. A successful demonstration supplies prerequisite evidence to later outcomes. If an atomic capability requires live login, a complete command flow or persistence not implemented in A, its B–E acceptance allocation remains pending; do not relabel a fragment as complete.

### 4. One-window shell, resources and lifetime

- Exactly one system SDL window is used during normal A scenario execution, including the prompt and diagnostics. Native surfaces are logical children within that window, not separate or virtual legacy Terms. Renderer startup failure is reported without substituting a terminal UI and calling that success.
- Use the approved logical-coordinate/output-pixel model, minimum 1024×768 logical client area, OS display scale handling and non-destructive resize/focus behavior. Do not introduce automatic layout switching. Test-only dimensions are fixture overrides, not new product defaults.
- Apply the final persistence/default contract to the implemented shell: desktop fullscreen is the product default, UI scale defaults to 100%, and the requested UI font is the approved bundled Cascadia Mono asset/profile with its licensing and fallback policy. Resource acquisition/packaging needed to run A is implementation work. Missing assets must not silently change the product's default contract.
- Prepare the implemented text/assets for their final output size and compose them using the approved raster rules. Full map rendering, tiles, weather, effects and font/filter selectors are later-stage work unless the chosen A scenario uses them. A does not need a complete map renderer to demonstrate HP, events and a request.
- Preserve the established SDL3 user-root identity and SV-owned settings boundary. The synthetic scenario operates on isolated data. Any settings/resource support needed by A follows its approved ownership and failure rules; a full settings editor, migration UI or vault integration is not required by the selected nonsecret fixture.
- Model/event/view memory is bounded and has explicit lifetime. Replaced views and completed requests release their storage; redraw does not redispatch consumed events. Required event delivery cannot be silently evicted to relieve pressure. Implemented hard-overflow paths fail explicitly according to the retention policy.
- No session recorder, hidden capture ring or live packet/input archive is added. Prepared fixtures and bounded safe assertion/measurement results provide evidence.

### 5. Complete canonical registry and allocation

- Populate the agreed normalized collections: capabilities, surfaces, actions, states, bindings, inputContexts and relations. A capability is an independently verifiable outcome or distinct complete flow; a packet field, button or broad family is not automatically a capability.
- Stable globally unique IDs use the agreed kind/domain/name convention. Keep lifecycle values `proposed`, `active`, `deprecated`, `retired`; never reuse an ID. Split/merge history uses replacement links without cycles or automatic transfer of acceptance.
- Sources distinguish behavior, protocol, UX and acceptance. Each active capability has behavior provenance. Record repository, revision, locator and content fingerprints where necessary to distinguish the dirty working tree from its Git revision. HTML fixtures remain UX material and evidence remains evidence.
- Bindings preserve gesture, action, input context and applicable keyset/platform restrictions. Relations have validated endpoint types and the agreed relationship meanings. Schema version describes structure; manifest content identity is the SHA-256 of its actual bytes.
- Reconcile the complete baseline and all completed inventories: scoped and remaining input loops, slash dispatch, every registered versioned packet path and response, formatted/server-driven surfaces, renderer/resources, persistence/files, platform/build branches and text/byte contracts. Every inventory row is mapped to outcome/context/scenario obligations or given an explicit source-backed disposition. Numeric audit counts are cross-checks, not permanent assertions immune to source drift.
- Preserve the map's approved exclusions, including XHTML screenshots, controller support and legacy terminal topology. An excluded/dead/directional protocol item is explained in reconciliation; it is not represented as an unexplained missing required capability. Supported gates remain visible rather than removing inconvenient variants from the denominator.
- The native consumer ledger/stage allocation assigns every active in-scope outcome one acceptance stage A–F, prerequisite outcomes, applicable source/version/build/platform conditions, required evidence and native/fallback/pending disposition. Keep implementation/stage/evidence status out of the canonical manifest.
- Cross-cutting flows reference all relevant outcomes. A common primitive does not certify all callers; a synthetic cancellation result does not certify every command's cancellation semantics. Later-stage prerequisites needed by an earlier claimed flow move with that flow rather than being silently deferred.
- A exits with the full registry populated and reconciled, not a scaffold or a representative sample. A newly discovered outcome updates registry/allocation and invalidates affected acceptance before the next claim.

### 6. Validation and consumer synchronization

- Implement structural and semantic validation: unique typed IDs, valid references/relations, mandatory provenance, legal lifecycle/replacements, no replacement cycles and complete explicit native coverage/allocation. Reports identify the offending outcome/source and distinguish malformed data, missing coverage and unavailable or stale evidence.
- Native claims identify their precise scenario, expected outcomes, actual result, executable/configuration, source/fixture/resource fingerprints and environment. A source or dependency change invalidates affected evidence; if impact is unknown, invalidate the broader affected scope. No native acceptance follows from schema validity alone.
- Support the agreed local manifest-snapshot workflow for the HTML consumer. Validate the canonical source before copying it; preserve byte identity and digest; initialize new active IDs as explicit missing entries without creating evidence or approval. Publication must not leave a new snapshot paired with an incompatible old ledger after failure.
- HTML implementation values remain `missing`, `planned`, `prototype-complete`; `ux-approved` is independent and requires actual human provenance. Existing historical claims are preserved but only current, adequately scoped claims count. Prototype implementation or UX approval never supplies native parity.
- A truthful all-missing HTML ledger is valid. HTML feature completeness, new visual approvals and completion of HTML mappings are not native A gates. Consumer tooling can be verified against a temporary local consumer; report whether the actual sibling consumer was synchronized rather than implying it was.
- Read-only freshness checks report unavailable canonical data as unavailable. Local synchronization does not require Git, a network service, PRs or rewriting UX documents. Source inventory/fingerprints detect relevant additions, removals and changes even when Git HEAD is unchanged.

### 7. Fallback ownership

- The accepted A scenario uses native presentation and input throughout; entering terminal fallback fails its check. Development fallback is only for an explicitly registered future flow, with owning module/flow, replacement stage and removal criterion.
- A need not implement a terminal adapter merely to populate a migration table. An empty runtime fallback route set is valid; unimplemented B–E outcomes remain pending rather than pretending to have a working legacy route.
- If A's reuse requires a temporary adapter, route only at named flow/context entries through the single input router. Preserve parent, queue, request, cancel and reply semantics; keep network/model/timers and urgent rendering active. Do not route unknown native gestures or implementation errors to a generic fallback.
- Diagnostics expose safe route identifiers/reasons/counts without copying prompt contents, secrets or arbitrary payloads. Disable each route as its native replacement is accepted. The later E/F gates remain unchanged: E runs all scenarios with fallback disabled; F excludes fallback linkage.

## Testing Decisions

### Behavior and prior art

Good tests assert externally meaningful transitions and failures through production interfaces: coherent published state, event occurrence/order, outgoing bytes, prompt completion, visible result and input-to-result geometry. Avoid tests of private array layouts, function call counts or a second implementation of the expected decoder. Use independently specified fixture outcomes and known protocol bytes.

Existing socket-buffer parsing, packet dispatch, request response serialization and SDL3 build rules provide reuse points. The current legacy HP/key-request implementations and the completed packet/input inventories provide expected semantics. The repository search found no maintained native SV scenario/replay test source; the legacy test-client build flag and Lua engine test source are not an SV behavior harness. HTML prototypes and historical ignored modern binaries are not substitutes for native evidence.

### Required A checks

| Check | Passing evidence |
|---|---|
| Build isolation | Linux amd64 and MinGW i686 SV builds use their own outputs; relevant existing client build paths remain intact after shared changes. Record toolchain and feature switches. |
| Synthetic end-to-end path | Real decode/apply, model views/events, SDL rendering, router and response serializer execute the stated scenario without a real server or terminal fallback. |
| Packet boundaries | Every selected wire variant handles complete input and split boundaries without premature mutation; retry applies once; a following sentinel packet remains consumable; defined decode failures preserve their contract. |
| State versus events | Latest status is coherent, repeated identical messages remain separate, and view reconstruction neither loses mandatory delivery nor repeats consumed effects. |
| Request and input | Correct request identity, valid-key reply, Escape cancellation, server abort, macro policy/order and restoration; no double reply or command from redraw/focus gain. |
| Short lifecycle | Updates during a prompt, resize, focus loss/gain, minimize/restore, view reconstruction and session reset retain or discard the right state; replaced storage is released and hard overflow is explicit. |
| Native geometry | Exercise the implemented surfaces at 1024×768 logical minimum, 1920×1080 at 100%, 3840×2160 at 200%, and fractional 125%/150%. Input and visible bounds agree; pending interaction survives. Virtual geometry evidence is labelled as such. |
| Rendering platforms | Linux accelerated and explicitly forced software runs; MinGW executable smoke under Wine including software rendering. Record actual backend, not only requested flags. Wine is labelled intermediate evidence. |
| Submission timing | Instrument the frame containing the relevant model revision/outcome, from complete decode or local input. Apply urgent ≤20 ms, interactive ≤50 ms, background ≤200 ms when exercised. Keep the first outstanding deadline when updates coalesce and record violations individually. |
| Registry completeness | Every active in-scope outcome has provenance, a native row and an A–F allocation; each completed inventory row has a mapping/disposition; unresolved required rows prevent A completion. |
| Validator rejection | Negative fixtures cover duplicate/unknown IDs, bad endpoint types, missing provenance/rows/stage assignments, replacement cycles, unsupported claims and stale/missing evidence. Valid pending coverage succeeds without implying accepted implementation. |
| Local consumer integrity | Validate an all-missing consumer, addition/retirement of IDs, digest mismatch, source changes at unchanged HEAD, unavailable canonical source and interrupted synchronization; do not invent approval or lose the prior coherent snapshot. |
| Human review | Review readability, keyboard/cancel/focus behavior and perceived response on the implemented native shell, tied to the tested build. A screenshot alone does not pass a behavior flow. |

Mandatory actual Windows 10/11 VM checkpoints start at B and recur at E/F, with earlier checks when platform-specific changes require them. A does not claim actual Windows acceptance from a successful MinGW link or Wine run. Missing required tools/environments leave their checks unverified rather than converting them into a source-audit pass.

Visible-response values remain manually reviewed targets, not newly measured hard gates. No intensive-load benchmark, long soak, global memory ceiling, pixel-perfect legacy comparison or session recording is introduced. Exercise only A's implemented rendering/resource/input variants at runtime; register remaining variants and their later evidence obligations in full.

Controlled fixture time is suitable for deterministic functional assertions. Submission-budget evidence measures actual elapsed time in the running native build; advancing a fake clock or acknowledging a mock renderer does not establish that evidence.

### Completion criteria

A may be accepted when the required builds and scoped native checks pass; the complete manifest, native ledger and allocation validate; local consumer tooling demonstrates truthful coverage; and the report identifies current evidence, known limitations and any fallback inventory. Publish reproducible build/scenario/check commands and the expected outcomes with the implementation. A crash, wrong reply, lost required event, corrupt state, mandatory timing violation or unclassified required registry row prevents the corresponding gate from passing.

Successful A completion establishes a runnable foundation. It does not mark B–F outcomes accepted merely because their interfaces or registry entries exist.

## Out of Scope

- Real server selection/login/account/character lifecycle, credentials and live gameplay acceptance, which start in B. Synthetic lifecycle resets in A are required but do not replace those checks.
- Complete map/HUD, item/combat/spell/store/social/document/special-canvas implementations and their full interaction coverage. Only the selected A native surfaces and their prerequisites are implemented now.
- Full settings/import editors, all fonts/tiles/filters/effects/audio integrations, macro editing/recording and user export flows. Necessary A startup/resources/ownership are in scope; the remaining outcomes are registered and assigned later.
- Completing the HTML UI, obtaining new UX approvals or treating HTML completeness as an A dependency. Registry/snapshot validation remains in scope.
- Shipping final archives, proving complete release dependency closure, full optional-configuration/runtime matrices for later capabilities and final zero-fallback product acceptance.
- Adding a server extension, altering gameplay semantics, restoring legacy terminal topology, implementing controller support or XHTML screenshots.
- Reconstructing or cleaning up ignored modern artifacts, unrelated server changes, and modifying personal user data during checks.
- Creating implementation tickets or writing client/checker code as part of this specification task. This document is the input to ticket decomposition.

## Further Notes

### Authority and source use

The user approved the stage order, platform checkpoints, registry deadline, native/fallback handoff and stage gates in both rounds of the migration decision. This spec synthesizes those decisions. The HP/messages/key-request scenario is a concrete implementation slice under those requirements, not a new claim of product completeness.

Later explicit decisions override historical proposals. In particular: the final persistence contract replaces shared CFG/OPT writes with independent SV ownership, sets fullscreen startup, and removes map zoom; the acceptance contract removes XHTML, stress/soak and a global memory ceiling. Old research suggestions or unresolved-looking historical comments do not reinstate them.

Read the following sources when implementing the indicated branch. Their linked normative detail is part of this spec; the map remains the index for the complete inventory.

| Branch | Required source |
|---|---|
| Stage scope, gates and future allocation | [Implementation sequence](../single-window-sdl3-client/migration-sequence.md), [resolved migration decision](../single-window-sdl3-client/issues/29-sequence-surface-migration-and-retire-terminal-fallback.md#answer), [complete decision map](../single-window-sdl3-client/map.md) |
| Domain terminology | [Domain glossary](../../CONTEXT.md) |
| Model, ownership and command interface | [Presentation-state decision](../single-window-sdl3-client/issues/02-choose-presentation-state-boundary.md#answer) |
| Decode/field/response reconciliation | [Packet decision](../single-window-sdl3-client/issues/11-map-every-packet-field-to-semantic-state.md#answer), [field-level wire inventory](../../docs/research/single-window-packet-state.md) |
| Input, macro and cancel behavior | [Input semantics](../single-window-sdl3-client/issues/04-preserve-input-and-macro-semantics.md#answer), [scoped loops](../../docs/research/single-window-input-loops.md), [remaining loops](../single-window-sdl3-client/research/remaining-client-input-loops.md), [slash dispatch](../single-window-sdl3-client/research/slash-command-grammar-and-dispatch.md) |
| Registry structure and provenance | [Manifest contract](../single-window-sdl3-client/issues/07-govern-the-cross-repo-capability-manifest.md#answer), [baseline inventory owner](../single-window-sdl3-client/issues/01-inventory-behavior-baseline.md#answer) |
| Consumer claims, fingerprints and sync | [HTML ledger decision](../single-window-sdl3-client/issues/16-make-the-html-gap-ledger-machine-checkable.md#answer), [detailed ledger contract](../single-window-sdl3-client/research/html-gap-ledger.md) |
| Build identity/platform scope | [Build and platform decision](../single-window-sdl3-client/issues/08-name-and-ship-the-new-client.md#answer), [platform audit](../single-window-sdl3-client/research/platform-deltas-and-packaging.md) |
| Surface lifecycle/geometry | [Interaction model](../single-window-sdl3-client/issues/03-define-single-window-interaction-model.md), [surface layouts](../single-window-sdl3-client/issues/21-specify-surface-layouts-and-responsive-rules.md) |
| Text, glyphs and assets | [Encoding/glyph decision](../single-window-sdl3-client/issues/22-define-encoding-and-glyph-identity.md), [raster decision](../single-window-sdl3-client/issues/23-choose-raster-references-and-defect-compatibility.md#answer), [font candidates and exact asset identity](../single-window-sdl3-client/research/bundled-ttf-candidates.md), [field defect disposition](../single-window-sdl3-client/issues/35-decide-text-field-boundaries-and-legacy-defects.md#answer) |
| Settings/defaults/storage | [Final persistence contract](../single-window-sdl3-client/persistence-contract.md) |
| Working lifetime/events/overflow | [Retention contract](../single-window-sdl3-client/issues/26-define-bounded-working-retention-and-optional-capture.md#answer) |
| Timing and evidence | [Urgency semantics](../single-window-sdl3-client/issues/20-classify-display-urgency-and-latency-budgets.md), [final acceptance contract](../single-window-sdl3-client/acceptance-contract.md) |

### Current repository pointers

These are discovery pointers observed during synthesis, not mandated future module locations:

- Existing build and SDL integration: [SDL3 makefile](../../src/makefile.sdl3), [SDL3 frontend](../../src/client/main-sdl3.c), [SDL network adapter](../../src/common/net-sdl3.c).
- Existing dispatch/decoders/replies: [client network code](../../src/client/nclient.c), [socket buffer](../../src/common/sockbuf.c); legacy input policy: [client utilities](../../src/client/c-util.c), [command dispatch](../../src/client/c-cmd.c).
- The approved canonical destination is `docs/capabilities/manifest.json` with its neighboring schema; these files were absent at synthesis. Populate them during A. Keep native stage/coverage metadata in its consumer artifacts rather than adding implementation status to the canonical manifest.
- The original 36-family baseline report is retained in Git at commit `87ead6ff5` as `docs/research/single-window-behavior-baseline.md`; its resolved owner ticket links that provenance. Later inventories and decisions refine it. Recover the report read-only when reconciling the full registry; its family count is not an atomic coverage claim.
- Existing tracked test searches found Lua engine test material and legacy test-client targets, but no maintained SV scenario harness. Build the A harness around production interfaces rather than assuming historical modern replay binaries have reusable source.

The local Markdown tracker uses one implementation issue per file. Subsequent ticket decomposition should keep runnable vertical progress and explicit blockers, preserving the two A exit obligations: a verified production path and the complete registry. This spec and its `ready-for-agent` label assert readiness for that work, not completion of any runtime check.

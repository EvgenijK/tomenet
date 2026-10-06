# Single-window client: acceptance contract

Status: approved planning contract, confirmed by the user on 2026-09-20.

Decision owner: [Design parity evidence and acceptance](issues/09-design-parity-evidence-and-acceptance.md). This document consolidates the user's accepted answers; no client implementation, automated harness, Windows VM setup or runtime acceptance is claimed. Later explicit amendments in the decision ticket take precedence over historical proposals in earlier inventories.

## Scope and staged acceptance

The client is accepted in stages. Each stage identifies the capabilities being accepted, required checks, target environments and known limitations. Missing capabilities assigned to later stages do not block the current stage. Previously accepted capabilities receive regression checks. Final acceptance requires all in-scope capabilities on Linux amd64 and Windows 10/11 with the i686 executable, one system SDL_Window and no terminal fallback.

Concrete stages, their ordering, platform checkpoints and fallback retirement gates are specified in the approved [Implementation sequence](migration-sequence.md), resolved by [Sequence surface migration and retire terminal fallback](issues/29-sequence-surface-migration-and-retire-terminal-fallback.md#answer). Linux-first development and early Windows builds remain as agreed in [Name and ship the new client across platforms](issues/08-name-and-ship-the-new-client.md). A stage cannot claim a platform it did not check.

The HTML prototype supplies approved visual/layout decisions. Its implementation completeness does not block native implementation or acceptance. Native evidence must show its own behavior and approved visuals. HTML coverage/approval/freshness claims remain truthful under the [HTML ledger contract](research/html-gap-ledger.md); they do not prove native behavior and prototype-ready is not a native gate.

XHTML screenshots are explicitly excluded from SV. Record this disposition in manifest/consumer mappings rather than treating XHTML as an unimplemented required capability. This exclusion does not remove independent text copying, character/message exports, notes, bookmarks or user macro recording.

## Evidence architecture

Use complementary layers, linked to atomic capability outcomes and their applicable source/version/build gates:

| Layer | Required evidence |
|---|---|
| Registry and provenance | Active manifest IDs, behavior/protocol sources, approved UX sources, supported gates and expected outcomes; scoped input→state→output→cancel/error→persistence flows |
| Deterministic fixtures | Synthetic initial state, server input and user actions; source/fixture version, feature configuration, controlled time/RNG and explicit expected state/events/replies |
| Wire and state integration | Actual production decode/apply paths, each registered in-scope wire variant, fragmentation and batch sentinels, ordered events and version/build branches |
| Native UI scenarios | Real input routing and rendering; focus, cancellation, macros, opening/restoring surfaces, map geometry/hit testing, file and OS effects |
| Actual test server | Connection, login, character creation, gameplay commands/interactions and reconnect through real network exchange |
| Packaged runtime | The extracted intended archive and its resources/dependencies in the supported OS environments |
| Human review | Fixed gameplay/readability/focus/input/scaling/system-integration checklist plus exploratory review, tied to the tested build |

Fixtures are prepared synthetic cases, not recordings of users' sessions. No session recorder or hidden capture ring is introduced. Synthetic secrets and private payloads are used where needed. Diagnostic artifacts do not expose secrets or reversible representations.

Protocol evidence includes consumption of each complete packet and preservation of the following sentinel, incomplete-packet retry behavior and absence of premature side effects. Feature guards must not leave a registered receiver reporting success without consuming its payload; SHOW_SANITY is a known source-audit case, not permission to ship a broken supported configuration. Arbitrary combinations of shared preprocessor flags and TEST_CLIENT are not automatically supported variants.

State evidence distinguishes coalescible projections from ordered occurrences/replies. UI recreation, relog, hidden surfaces and resource replacements must not duplicate commands, sounds or other consumed effects. The agreed transient store-animation recreation exception still applies.

Evidence records the executable/build revision, build switches, fixture/scenario versions, resource/profile identifiers, OS/backend/display and expected/actual outcomes. A changed dependency invalidates affected evidence until rerun; where its impact cannot be established, invalidate the broader affected scope. Missing, failed, stale or unavailable evidence cannot establish acceptance of that scope. A source audit, successful link, screenshot file or honest ledger alone is insufficient.

## Build configurations and optional behavior

The player package contains all agreed features. Developer configurations may disable the following capabilities. On Linux and Windows check the full configuration, each capability disabled separately, and all listed capabilities disabled together. Reduced configurations require startup, network and affected-function checks; the full configuration receives full gameplay acceptance. This does not require every intermediate combination.

| Optional capability | Required disabled behavior |
|---|---|
| PNG screenshot output | Save BMP with matching extension/content. Necessary PNG decoding for UI resources remains available; this switch must not break the UI |
| Built-in audio-pack extraction | Explain manual pack installation; an already installed pack remains usable |
| Built-in Guide download/check | Open the local guide. If none is available, offer https://www.tomenet.eu/guide.php |
| Sticky modifiers | Normal simultaneous modifier/key combinations keep working |

Sticky modifiers retain their agreed baseline default; their on/off behavior is covered by affected-function checks. Diagnostic logging, sanitizers, toolchain selection and optimization settings may vary without silently redefining the supported product behavior. Terminal-mode support is not restored by this optional-feature policy.

## Reference environments

One physical computer is sufficient for the chosen acceptance route; no additional test hardware is required.

- Reference host observed during planning: Intel Core i7-14700HX, 20 cores/28 logical CPUs, nominal 32 GB RAM, NVIDIA RTX 4070 Mobile/Max-Q, Manjaro Linux and KDE Wayland. Record actual OS/kernel/driver/backend versions at each acceptance run rather than freezing incidental development versions forever.
- Linux runs directly on this host. Fedora41-class build/ABI baseline remains the shipping contract; a Manjaro run alone does not prove that ABI or package dependency closure.
- Wine provides intermediate Windows checks, labelled as Wine evidence.
- Actual Windows 10 and Windows 11 run sequentially in VMs on the same host, with 8 vCPU and 8 GiB guest RAM. Record guest version, virtual graphics/driver, VM software/configuration and host display conditions. VM selection and installation are implementation/setup work, not already completed.
- Software rendering is mandatory. Test Linux accelerated and explicitly forced software paths; test Windows software rendering and its accelerated path when available in the VM. No available acceleration means no accelerated-path claim for that VM.

These measurements establish behavior/performance on this reference configuration, not a guarantee for weaker or integrated-graphics machines. VM resource allocation is not the client's minimum system requirement.

Geometry/input matrix:

| Case | Requirement |
|---|---|
| Minimum window | 1024×768 logical units |
| Standard output | 1920×1080 at 100% |
| High-density output | 3840×2160 at 200% |
| Fractional scale | 125% and 150%, checking rounding, complete map fit and hit testing |

Use the normal/big map and approved layout/font/filter lifecycle cases within this matrix. Virtual outputs/offscreen rendering are allowed to establish geometry when a physical display mode is unavailable. Record actual logical and output dimensions and scaling; virtual geometry does not prove physical display timing.

## Timing, concurrency and memory

[Urgency and deadline semantics](issues/20-classify-display-urgency-and-latency-budgets.md) remain normative, with the accepted measurement amendment:

| Urgency | Mandatory decode/input→frame submission | Visible-response target, manually reviewed |
|---|---:|---:|
| Urgent | ≤20 ms | 50 ms |
| Interactive | ≤50 ms | 100 ms |
| Background | ≤200 ms | 250 ms |

Instrument the complete-decode/local-input origin, applicable first outstanding deadline, model revision and successful submission of the frame containing the outcome. Do not reset a deadline when newer coalescible changes arrive. Record each violation, not just averages. Server wait is separate; a submission timestamp is not a physical presentation timestamp. Exact visible-response targets are no longer mandatory measured gates; manually review responsiveness on the available display, including the Windows VM path.

Keep short functional scenarios: HP/map changes during animation, timely cancel, scroll, resize with correct latest state/hit testing, focus loss, minimize/restore and opening hidden surfaces. These are ordinary bounded scenarios, not a synthetic high-rate workload.

No intensive-load benchmark and no long-running soak is required. In particular, the proposed 10-minute update/message flood and hour-long lifecycle run were rejected. Do not report stress capacity, leak-free extended play or exact visible latency as proven by short checks.

There is no new global numeric RAM/graphics-memory ceiling for acceptance. Preserve baseline recall capacities, bounded queues/caches/views, prompt release of replaced resources and the [working-retention and overflow policy](issues/26-define-bounded-working-retention-and-optional-capture.md). Concrete internal bounds are documented by the implementation according to representation/ownership; this is not permission for unbounded archives or indefinite retention. Short lifecycle checks verify replacement/release and record memory use. Correct explicit failure under an intentionally injected hard-overflow case differs from unexpected overflow during an ordinary accepted scenario, which fails that scenario.

## Screenshots

Ctrl+T, Ctrl+Shift+T, /shot and /screenshot invoke one native screenshot action. Capture the entire composed client window in one completed frame, including map, visible panels, dialogs, tooltips and effects, without OS window decorations. The success notification appears after capture and is not part of the saved image. No legacy textual layout, XHTML exporter or redux crop is implemented.

PNG output is normal; the permitted disabled-output configuration uses BMP. Check actual file encoding against its extension. Keep the slash-command filename argument, filename mode and the [persistence contract](persistence-contract.md) collision rules: generated names acquire a free suffix, explicit existing destinations require replace/cancel. No silent overwrite and no revival of old XHTML .bak semantics.

Retire screenshot_keys as an XHTML/native binding swap; importing its previous value does not change the unified action or restore XHTML. Preserve screenshot_format filename semantics. Update last successful screenshot only after successful writing. On failure, report the error and retain access to the previous successful file. Opening the last saved image is checked through the supported OS association path.

## Required outcome families

The following references own the detailed expected outcomes; this contract selects evidence and gates rather than duplicating their state models:

| Area | Required coverage and normative owner |
|---|---|
| Input and control | [Input/macro semantics](issues/04-preserve-input-and-macro-semantics.md), scoped and remaining loop inventories, slash grammar: bindings, queue order, context cancellation/restoration, mouse intents and guarded controls |
| Packets and server surfaces | [Packet mapping](issues/11-map-every-packet-field-to-semantic-state.md), [formatted surfaces](issues/12-inventory-formatted-and-server-driven-surfaces.md): all applicable versioned paths, partial documents/canvases, replies, lossless ownership and reset/close |
| Text and bytes | [Encoding/glyph identity](issues/22-define-encoding-and-glyph-identity.md), [field registry](issues/27-enumerate-source-text-and-server-field-byte-contracts.md), [boundary dispositions](issues/35-decide-text-field-boundaries-and-legacy-defects.md): editor/storage/escaped/wire capacities, NUL/sentinel, outgoing validation, parsed copy and glyph profiles; XHTML-specific requirements superseded |
| Rendering | [Raster contract](issues/23-choose-raster-references-and-defect-compatibility.md): final-size asset preparation and 1:1 composition, palette/cache reuse, stale-generation rejection, resize exception, filters/masks/outlines, PCF/TTF discovery/custom resources, searchlight correction and current-state restoration under effects |
| Layout and lifecycle | [Surface geometry](issues/21-specify-surface-layouts-and-responsive-rules.md), [MOTD](issues/28-prototype-the-server-defined-motd-screen.md): source-preserving layouts, full-map fit, primary/children/focus and pregame/session transitions; cursor visual design remains assigned to the HTML UX effort |
| Persistence | [Persistence contract](persistence-contract.md): independent CFG/OPT/history, Save/Cancel/exit, data-only explicit import, shared macros/INS/DNA/resources, defaults/fallbacks, collision/concurrency, disk faults, source immutability and original server-file rules |
| Credentials | [Credential policy](issues/24-define-credential-storage-policy.md): Linux Secret Service and real Windows Credential Manager round-trip/identity/write timing, provider unavailable/locked/cancel/failure, late completion, import conflicts and no secret leakage |
| Retention | [Working retention](issues/26-define-bounded-working-retention-and-optional-capture.md): capacities/eviction/anchors, duplicate occurrences, consumers and explicit overflow, recreation without redispatch and bounded current document/canvas ownership |

Use semantic assertions and appropriate raster inspection, not pixel equality to legacy. Synthetic controlled clock/RNG allows reproducible effect assertions without requiring identical historical random frames. Both platform-specific behavior and shared behavior require evidence appropriate to the actual target builds.

## Package and external-integration checks

Check the extracted intended archive in an isolated environment/profile without accidentally obtaining missing libraries or resources from the developer checkout. Record ELF/PE architecture, Fedora41-class Linux ABI/required system dependencies, Windows i686 runtime libraries and dynamically loaded codecs/data. Compile/link, import-table scans or archive existence do not replace startup and feature execution from that package.

Check adjacent lib in separate and colocated legacy/SV installation layouts, intended stock contents/licenses and absence of personal user data; required PNG/archive/Guide functions in the full package; unavailable optional external providers with the accepted behavior. Exercise file/URL associations, clipboard/input, audio/resources, network connection/failure/timeout and unchanged fingerprint path input. Isolation uses test profiles/fixtures; it does not require another physical machine. Detailed ownership and secret-service fallback remain governed by their contracts.

## Pass, failure and handoff

A crash, lost mandatory event, wrong command/reply, corrupted data, unsupported behavior within the claimed scope or violation of a mandatory submission budget fails the affected check. A documented external disturbance such as host sleep makes the run invalid; record the reason and rerun. Never silently discard a failing sample or turn it into a successful average.

Human acceptance includes the agreed checklist and exploratory review on both target platforms, appropriate to the stage. Unperformed checks remain explicitly unverified. Final acceptance requires current evidence for the complete in-scope capability set, successful target-package runtime checks, mandatory software rendering, one system window and zero terminal fallback. The planned harnesses and this specification themselves do not establish that acceptance.

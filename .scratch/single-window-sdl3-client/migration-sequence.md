# Single-window client: implementation sequence

Status: approved planning contract; Q1–Q6 confirmed by the user on 2026-09-20.

Decision owner: [Sequence surface migration and retire terminal fallback](issues/29-sequence-surface-migration-and-retire-terminal-fallback.md).
Normative checks: [Acceptance contract](acceptance-contract.md). This is a plan, not implementation or runtime evidence.

## Approved framework

A–F checkpoints are cumulative. Dependencies needed by a capability (input, errors, persistence, resources, platform behavior) arrive with it, even if their broader feature family completes later. HTML completeness does not block native work. Previously accepted capabilities receive regression checks; future capabilities remain explicitly pending.

Linux-first; MinGW32 build from A, Wine smoke each checkpoint, Windows 10/11 VM checks at B/E/F and earlier for platform-specific changes. Linux software rendering starts with the first rendered slice; Windows software rendering with the first Windows runtime. Claims identify actually checked environments; Wine is not Windows acceptance.

## Stage detail

| Stage | Capabilities and required evidence | Known limits after this stage |
|---|---|---|
| A — runnable foundation | Separate executable/object paths, one SDL window, production model/command interfaces and renderer exercised by a synthetic decode→state→surface→input scenario. Prove complete/incomplete packet handling for this slice, ordered events, cancellation, resize/focus and software rendering. Create/validate full source-backed atomic manifest, native ledger and allocation to A–F; HTML snapshot/ledger may honestly report missing. | No claim of real login or gameplay; synthetic scope only. Later capabilities have named owners/stages and pending evidence. |
| B — session and game screen | Real server selection/connect/login/account and character selection/creation, MOTD, map/HUD/basic movement/chat/messages, disconnect/reconnect/exit and death/session transitions. Include credentials, byte limits, clipboard/text input, histories and minimal config/resource prerequisites; startup server file transfer/Lua reload and map weather/palette/resize negotiation arrive here when required by these flows. Verify real server round trip, normal/big map geometry, keyboard/macro routes of these flows, pending-request/session cleanup and Secret Service/Credential Manager behavior. | Item/combat selection and other C–E flows may use registered development fallback. Merely surviving a death transition does not claim ghost powers. |
| C — game actions | Items/inventory/equipment, targeting/look/directions, spells/skills/ghost/mimic/runes/stances/techniques, ordinary stores and associated confirmations/requests. Preserve keysets, user macro load/play/waits, mouse intents, cancellation/retry and return to parent. Verify relevant Lua and wire gates, store transactions, slot identity and multi-step macro chains. | Information/social/document/special-store and remaining integration flows stay pending unless required by an accepted C outcome. |
| D — information and server surfaces | Remaining character/knowledge/social flows, housing/utilities and applicable admin/DM flows, local and arbitrary server documents, search/page navigation, special-store canvases/animations and relevant server-driven controls. Verify lossless content, ordered replies, partial updates, fit/scroll rules, close/reset/recreation and no repeated side effects. | Remaining E settings/resource/file/audio/platform outcomes are pending; generic source-preserving documents/canvases are native surfaces, not terminal fallback. |
| E — integration completeness | Finish all settings/import/save/cancel, visual preferences/fonts/tiles/filters/effects, audio/packs, Guide, macros editing/recording, files/exports/screenshots and clipboard/OS associations. Cover every remaining in-scope atomic outcome, optional-feature configurations and provider/disk/resource errors. Prove independent CFG/OPT/history and shared-file ownership. All stage acceptance scenarios run with fallback disabled. | No missing native capability remains; final archive closure and final complete evidence review still await F. |
| F — final acceptance | Build without fallback linkage; verify all active in-scope IDs have current evidence, all input/packet/version/build inventories are mapped, no unclassified legacy route remains. Run required target archives in isolated profiles, Linux ABI/dependency checks, Windows 10/11 runtime, optional configurations, software rendering, complete regression and human review. | Only approved exclusions and explicitly documented environment limits; no pending required capability or runtime check can pass final acceptance. |

Every stage applies the existing acceptance layers to its scope, including short concurrency/lifecycle cases and submission gates 20/50/200 ms where applicable. No stress/soak, XHTML or global memory ceiling is added. Rendering/fonts and OS dependencies needed earlier cannot be postponed to E. Slash verbs, bindings, packet variants and local/Lua flows are allocated by their outcomes, not treated as a separate deferred umbrella feature.

## Registry and stage gates

The planned canonical files do not yet exist. A creates the schema, source-backed atomic manifest, native coverage ledger and full stage allocation from the completed inventories. Never fabricate IDs in this planning document or infer coverage from a broad family label. Each active in-scope outcome has one acceptance stage, its prerequisites, evidence requirements and a native/fallback/pending disposition. Cross-cutting scenarios reference all involved IDs. Shared primitives do not establish coverage of all callers.

A changed source or newly discovered outcome updates the registry and stage allocation before acceptance of the affected scope. If needed by an earlier claimed flow, it is not deferred merely to preserve a passed status. XHTML is explicitly excluded under the approved policy. HTML coverage and UX approval remain separate and truthful; missing HTML functionality cannot fail native acceptance.

Each checkpoint records current revision/configuration/environment, new accepted outcomes, regression results, remaining limitations and fallback inventory. Earlier accepted flows receive regression checks; changed dependencies invalidate affected evidence. Missing/failed/stale checks fail the corresponding claim. Exact test code and harness design belong to implementation.

## Native/fallback transition and retirement

A single input router owns logical input context, macro queue and pending requests. Development fallback is an allow-listed adapter for specific future capabilities inside the same SDL window; there are no permanent virtual Terms and no native reads from terminal buffers.

Routing is selected at a named flow/context entry, never as catch-all recovery from an unknown key or native error. A child flow may use fallback only when explicitly declared as pending; the parent is not claimed to cover that complete child outcome. Passing control restores the true parent and preserves the baseline queue/reply/cancel rules; no duplicated input, command or response. Networking, timers, model updates and required urgent rendering continue while fallback is active. Relog/teardown invalidates both native and fallback session context.

Each entry records its owning flow/module, replacement stage and removal check. Diagnostics contain only safe IDs/reasons/counts, never user content or secrets. Accepted flows entering fallback fail regression. Disable a route as its native replacement is accepted; remove unused adapter code incrementally. E demonstrates all scenarios with fallback disabled. F excludes adapter linkage and combines build/source route checks with runtime evidence; zero runtime hits alone cannot prove zero dependency.

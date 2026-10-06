# Single-window client: implementation sequence

Status: approved planning contract; Q1–Q6 confirmed by the user on 2026-09-20;
Guide deferral and pregame-B amendments confirmed by the user on 2026-10-05;
post-B functional-stage reallocation confirmed by the user on 2026-10-06.

Decision owner: [Sequence surface migration and retire terminal fallback](issues/29-sequence-surface-migration-and-retire-terminal-fallback.md).
The current post-B sequence is owned by
[Reallocate post-B work by functional block](issues/38-reallocate-post-b-by-functional-block.md#answer).
Normative checks: [Acceptance contract](acceptance-contract.md). This is a plan, not implementation or runtime evidence.

## Approved framework

A and B remain the fixed cumulative checkpoints described below. After B, the
former C–G allocation is replaced by the ordered one-block stages defined by
[decision 38](issues/38-reallocate-post-b-by-functional-block.md#answer) and the
[stage reallocation plan](../../docs/tasks/stage-reallocation/README.md).
Dependencies needed by a capability arrive in an earlier stage or in the same
stage only when they belong to its block. HTML completeness does not block native
work. Previously accepted capabilities receive regression checks; future
capabilities remain explicitly pending.

Guide remains in the late part of the post-B sequence. Until its block is accepted,
every earlier Guide entry point displays exactly
`The guide is in development` and returns to the actual caller without reading Guide
content, opening bookmark state, sending a Guide request or entering fallback. The
placeholder is temporary stage behavior, not coverage of a Guide capability.

B has a second explicit allocation boundary: it accepts the native pregame screens
through a managed protocol peer, not a live TomeNET session. The peer replaces only
the external endpoint; production transport/protocol, model, interaction, input and
UI paths remain under test. Real-server integration and the first game screen begin
after B in the corresponding one-block stages. The former aggregate
`session.enter-game` dependency must not pull profile, FILE/Lua, game resources,
map, HUD or gameplay back into B.

Linux-first; MinGW32 build from A and the existing B platform obligations remain
unchanged. Later platform/runtime checks are attached to the affected block stage
and cumulative verification gates rather than to the obsolete E/F/G letters.
Linux software rendering starts with the first rendered slice; Windows software
rendering with the first Windows runtime. Claims identify actually checked
environments; Wine is not Windows acceptance.

## Stage detail

| Stage | Capabilities and required evidence | Known limits after this stage |
|---|---|---|
| A — runnable foundation | Separate executable/object paths, one SDL window, production model/command interfaces and renderer exercised by a synthetic decode→state→surface→input scenario. Prove complete/incomplete packet handling for this slice, ordered events, cancellation, resize/focus and software rendering. Create/validate full source-backed atomic manifest, native ledger and allocation to A–G; HTML snapshot/ledger may honestly report missing. | No claim of real login or gameplay; synthetic scope only. Later capabilities have named owners/stages and pending evidence. |
| B — native pregame screens | Fully working native flow for endpoint/connect, account creation/authentication, character overview/management, character creation and peer-driven MOTD. Exercise production UI/model/input/transport/protocol paths through a managed protocol peer, including success/rejection, back/cancel/retry, fragmented input, disconnect and stale-session cleanup. Include field byte rules and protected credentials required by these screens. | No live-server compatibility or gameplay claim. Post-MOTD startup, profile/FILE/Lua/game resources, first map/HUD and every gameplay/session-end flow remain pending. |
| After B — functional-block sequence | The 863 outcomes formerly assigned to C–G are reallocated into ordered stages with exactly one functional block each. The prerequisite audit, registry/catalog migration and task materialization are specified in the [stage reallocation plan](../../docs/tasks/stage-reallocation/README.md). | Until the catalog and ledger migration pass validation, old C615/D162/E64/F1/G21 values are historical only and do not define a current implementation stage. |

Canonical A8/B54 allocation remains unchanged. The post-B denominator is 863;
its exact stage counts are published only by the validated stage catalog and
ledger produced by the reallocation plan.

Every stage applies the existing acceptance layers to its scope, including short concurrency/lifecycle cases and submission gates 20/50/200 ms where applicable. No stress/soak, XHTML or global memory ceiling is added. Rendering/fonts and OS dependencies needed by an earlier block cannot be postponed to an unrelated late stage. Slash verbs, bindings, packet variants and local/Lua flows are allocated by their outcomes, not treated as a separate deferred umbrella feature.

## Registry and stage gates

The planned canonical files do not yet exist. A creates the schema, source-backed atomic manifest, native coverage ledger and full stage allocation from the completed inventories. Never fabricate IDs in this planning document or infer coverage from a broad family label. Each active in-scope outcome has one acceptance stage, its prerequisites, evidence requirements and a native/fallback/pending disposition. Cross-cutting scenarios reference all involved IDs. Shared primitives do not establish coverage of all callers.

A changed source or newly discovered outcome updates the registry and stage allocation before acceptance of the affected scope. If needed by an earlier claimed flow, it is not deferred merely to preserve a passed status, except for the explicit pre-Guide placeholder and B pregame/live-session boundary above. A broad existing outcome is kept in the later stage or refined into independently testable outcomes; it does not import later behavior into B merely through its old prerequisites. XHTML is explicitly excluded under the approved policy. HTML coverage and UX approval remain separate and truthful; missing HTML functionality cannot fail native acceptance.

Each checkpoint records current revision/configuration/environment, new accepted outcomes, regression results, remaining limitations and fallback inventory. Earlier accepted flows receive regression checks; changed dependencies invalidate affected evidence. Missing/failed/stale checks fail the corresponding claim. Exact test code and harness design belong to implementation.

## Native/fallback transition and retirement

A single input router owns logical input context, macro queue and pending requests. Development fallback is an allow-listed adapter for specific future capabilities inside the same SDL window; there are no permanent virtual Terms and no native reads from terminal buffers.

Routing is selected at a named flow/context entry, never as catch-all recovery from an unknown key or native error. A child flow may use fallback only when explicitly declared as pending; the parent is not claimed to cover that complete child outcome. Passing control restores the true parent and preserves the baseline queue/reply/cancel rules; no duplicated input, command or response. Networking, timers, model updates and required urgent rendering continue while fallback is active. Relog/teardown invalidates both native and fallback session context.

Each entry records its owning flow/module, replacement stage and removal check. Diagnostics contain only safe IDs/reasons/counts, never user content or secrets. Accepted flows entering fallback fail regression. Disable a route as its native replacement is accepted; remove unused adapter code incrementally. The affected one-block stage and cumulative verification gates prove fallback retirement; the final gate excludes adapter linkage and combines build/source route checks with runtime evidence. Zero runtime hits alone cannot prove zero dependency. The Guide placeholder is a native unavailable-state, not fallback, and is removed only after the complete Guide block is accepted.

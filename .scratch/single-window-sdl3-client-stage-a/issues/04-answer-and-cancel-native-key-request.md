# 04: Answer and cancel a native key request

**What to build:** A server-defined key request appears as a native child surface. User input passes through the production input router and command interface to the real response serializer, while status updates continue.

**Blocked by:** 02: Display decoded HP through the Session presentation model.

**Status:** implemented (Linux verification complete; Windows cross dependencies unavailable)

- [x] Complete production decode preserves request identity and prompt; split input publishes no prompt, state, reply or other effect before completion, and adjacent packets remain correctly readable.
- [x] A valid key emits exactly the baseline reply bytes once for the correct request; Escape emits the key-request cancellation value zero, not a universal invented cancellation code.
- [x] Exercise server abort according to the owner-specific baseline contract and verify completion/restoration without an obsolete or duplicate reply.
- [x] The input router owns logical context and pending interaction; widgets do not become independent command interpreters or invoke a synchronous terminal prompt.
- [x] While the request remains pending, production HP updates and native rendering continue; completing or cancelling restores the correct parent context.
- [x] Redraw cannot resend a response, and completed requests release storage. Assertions capture real serializer output at a controlled transport boundary.
- [x] The scenario is both automated and manually runnable in the single system window, and entering terminal fallback fails the accepted flow.


## Implementation and verification — 2026-09-21

Production path and evidence: [docs/sv-requests.md](../../../docs/sv-requests.md).
Initial implementation commit: `5c4b069ac`; final follow-up records the review
fix and verification. Shared legacy decoder/serializer, bounded Session request,
input-router cancellation/parent restoration, SDL native input and child surface
are implemented. Server abort sends one zero reply for this owner, as baseline
requires; it sends nothing without a pending request.

All 14 available Linux regression runners passed, including sanitizer checks
(372 request fragmentation cases) and software/OpenGL native checks (40 cases /
120 submissions each). Linux SV and legacy SDL3 targets build. Parallel Spec
review: zero findings. Standards: one architecture finding fixed and follow-up
reports zero unresolved findings. Windows cross-build attempt is blocked by
missing SDL3/SDL3_ttf/FreeType development packages. No Wine/Windows, Fedora41
shipping-baseline or human UX acceptance is claimed. Macro/lifecycle continuation
remains ticket 05; this slice's native character input uses the declared ASCII
profile.

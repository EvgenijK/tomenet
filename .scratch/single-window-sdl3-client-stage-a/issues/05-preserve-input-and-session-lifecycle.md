# 05: Preserve input and session ownership through lifecycle changes

**What to build:** The combined HP, message and key-request scenario remains responsive and correct through ordinary window and session changes. Accepted macro input maintains the baseline ordering and ownership rules.

**Blocked by:** 03: Display ordered message occurrences; 04: Answer and cancel a native key request.

**Status:** implemented (Linux verification complete; Windows cross dependencies unavailable)

- [x] Run a representative accepted keyboard/macro sequence through reused production processing, respecting the request context's actual macro policy; assert exact action/reply order and absence of duplicate dispatch.
- [x] Status and message updates continue while the request is pending during resize, focus loss/gain, minimize/restore and native surface reconstruction.
- [x] Pending request identity, logical input context and correct parent restoration survive surface changes; focus gain generates no command and does not clear an accepted macro queue.
- [x] Tear down with a pending request and create a fresh synthetic session; old queued input, request identities and undelivered session-owned effects cannot act on the new session.
- [x] Replaced views and completed/invalidated requests release storage. Exercise relevant hard-overflow failures without silently dropping mandatory events.
- [x] Automated lifecycle assertions use production interfaces and are complemented by a runnable native scenario; no hidden capture ring, live packet/input archive or terminal fallback is introduced.
- [x] Document that these checks establish foundation behavior, not complete macro-context coverage or real login/session acceptance.

## Implementation and verification — 2026-09-22

Production paths, baseline sources, commands and limits:
[docs/sv-lifecycle.md](../../../docs/sv-lifecycle.md).
Implementation commit: `3f01c3eff`.
SV-local input processing implements one ASCII trigger/action macros with normal,
hybrid and inactive-command policy, nonrecursive expansion and physical backquote
cancellation. Unsupported macro wait markers are rejected. Accepted input has a
bounded request-owned queue; session generation and SDL timestamp epochs reject
old input. Window events rebuild only derived UI state.

Linux SV build and all 17 available regression runners passed. Native lifecycle
checks on software/OpenGL each verify six transitions, ten submissions and two
exact replies, with HP and distinct duplicate messages during pending interaction.
Headless sanitizer checks cover ordered macro policy, stale identity, teardown,
server abort and accepted-key/message/output overflows. Both parallel reviews
report zero findings. Legacy/common files remain unchanged.

MinGW build remains blocked by missing SDL3/SDL3_ttf/FreeType cross dependencies.
Deterministic focus events and minimized renderer readback are not compositor or
human UX acceptance. Full macro contexts, preference-file loading and real login
remain outside this foundation ticket. No live archive or fallback was added.

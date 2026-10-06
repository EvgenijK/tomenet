# 03: Display ordered message occurrences

**What to build:** A reviewer sees a native event/message surface populated by production message decoding, including distinct repeated identical messages. Updates coexist with the HP surface.

**Blocked by:** 02: Display decoded HP through the Session presentation model.

**Status:** implemented

- [x] Preserve original message text/field identity according to the approved byte and encoding contracts; rendered text is not used to reconstruct protocol data.
- [x] Two identical valid message occurrences are delivered distinctly and in order; latest-state coalescing never coalesces required events.
- [x] Complete, fragmented and adjacent packet cases demonstrate no premature publication, one application after completion and an intact following packet; cover selected version variants and defined failures.
- [x] Redraw and surface reconstruction neither repeat consumed event effects nor lose mandatory undelivered events.
- [x] Working storage is bounded with explicit session ownership and release behavior; implemented hard-overflow paths fail explicitly rather than silently evicting required delivery.
- [x] The runnable native scenario and automated assertions exercise real decode, model events and rendering with no terminal fallback or session recorder.


## Implementation and verification — 2026-09-21

Shared production message decoding now feeds ordered session-owned occurrences
and a native live feed alongside HP. Original byte fields, fragmented/adjacent
packets, clear sentinel, independent mandatory delivery, surface reconstruction,
bounded storage, explicit overflow and session release are covered.

Linux sanitizer checks, native message checks (18 cases / 54 submissions on each
of software and OpenGL), the full existing SV regression matrix, legacy HP
regression and full legacy SDL3 build passed. Standards review fixes were applied;
Standards and Spec follow-up reviews have no unresolved findings. MinGW checks
remain unavailable because cross SDL3/SDL_ttf/FreeType dependencies are absent.
Full recall, chat routing/effects and broad glyph coverage remain later-stage work.
Reproduction commands and scope are in `docs/sv-messages.md`.

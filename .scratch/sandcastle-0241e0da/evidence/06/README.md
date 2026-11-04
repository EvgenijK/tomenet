# Ticket 06: Guide callers and peer MOTD at the startup boundary

Production parent before implementation: `f7918d19fc0f2d0a2843df484d9ad7d7e7bc3348`.

Contract: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.
Produced by `tests/sv_guide_motd_checks.py`, invoking public production
`sv_endpoint_run(SvEndpointOptions)` with a controlled TCP peer and SDL gestures.
No Guide implementation, substituted birth interaction, or test-only caller.

Reproduce all ten cases:

```sh
SV_GUIDE_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/06 python3 tests/sv_guide_motd_checks.py
```

The default backend is SDL dummy/software. These are actual production native
SDL/TTF rendered frames, read from the renderer immediately before its real
`SDL_RenderPresent`, losslessly encoded as PNG. They are not callback assertions
standing in for a caller. For an interactive display review, run the same command
with `SDL_VIDEODRIVER=x11` (and an available DISPLAY), or a supported native backend.
No X11/Wayland display is available in this implementation environment; an OS
window capture, controller review and fresh final audit remain controller-owned.

`1-*` uses class-before-race; `0-*` uses race-before-class. Each `.bin` is the
complete 16,266-byte final request, compared with the existing independently
baseline-derived literal `birth-current.hex`. `.bin.reply` contains no reply,
two bytes, or the authoritative `02 03 01` reply, according to the case.
The peer checks no extra request before ack, during split replies, or after
handoff, and services an interleaved ping. `.log` records actual presentation
and public endpoint outcome in order: Guide visits → authoritative reply →
peer MOTD presentation → live-session handoff presentation. Skip omits MOTD
presentation; waiting/partial show neither MOTD nor handoff.

PNG filenames identify surface (`1` Guide, `2` peer MOTD, `3` live handoff) and
Guide visit. For class-before-race, Guide visits are class, race, trait, body,
mode; in point-buy they are class, race, trait, body, stats, mode. For the
alternate order they are race, trait, body, class, [point-buy stats], mode.
Every visit is dismissed through native SDL input. Production outcome checks
require matching open/close counts, the exact owner mask, unchanged complete
birth draft/owner across every return, and the exact placeholder text.
The final peer request independently proves selected identity and committed stats.
Sex and stat ordering have no Guide entry; stat-order `?` is exercised as a no-op.

The point-buy fixture is derived from the independent current-baseline literal:
only six committed stats after the two resource strings change from ordering
`0,1,2,3,4,5` to `11,10,10,10,10,10`. The peer advertises point-buy creation,
the player edits the first stat before Guide, then acknowledges unused points.
All options, resources, geometry and complete visual tables remain unchanged.

`red.log` replays the final corrected test against the unchanged production
parent HEAD: after authoritative acknowledgement it fails because no MOTD frame
is presented. This corroborates the initial red (the initial test fixture also
had a truncated MOTD string, corrected before the successful green).
The full ten-case green is recorded by the other logs. No gameplay surface or
`session.enter-game` is enabled; the final native frame explicitly displays the
startup boundary and the inactive gameplay state.

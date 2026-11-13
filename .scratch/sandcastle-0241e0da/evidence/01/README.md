# Slice 01 controlled-peer evidence

Contract: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.
Production seam: `sv_endpoint_run(SvEndpointOptions)`; native SDL confirmations,
real loopback TCP, production `sv_login_start_play` in `protocol/login.c`.

Capture command:

```sh
SV_BIRTH_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/01 python3 tests/sv_birth_complete_checks.py
```

Each `.bin` is the complete 16,266-byte request actually received by the peer.
Each `.bin.reply` contains the authoritative reply bytes actually sent by that
peer: waiting = empty; partial = `02 03`; ack = `02 03 7a`.
Logs include the final public endpoint phase: the first two remain
`play-pending`; only the third becomes `play-acknowledged`. No gameplay or
MOTD/live handoff is opened. A ping round trip, resize/focus events and repeated
native confirmations are exercised while waiting; no additional final request
is received.

The independent literal oracle is `tests/sv/fixtures/birth-current.hex`.
Its baseline-only provenance script is
`tests/sv/fixtures/birth-current-baseline.py`; it never reads SV serialization
code or captured bytes. Identity is male/Mage/Dwarf/Phoenix/fruit bat/Normal;
committed stat order is 0..5; 199 option slots have 88 enabled values;
viewport is 66x22; audio/graphics are disabled; resource strings are
`NO_GRAPHICS` and the effective `CascadiaMono-Regular.ttf`.

Visual tables contain all 127 unknown, 512 feature, 1280 kind and 1280 monster
entries, each an attribute plus network-order u32 character. Their zero values
are the production baseline's server-identity sentinel, not a shortened or
zero-default substitute: `initialize_main_pref_files` and
`handle_process_font_file` clear these complete arrays, and neither a Cascadia
custom font mapping nor `font-sdl3.prf` exists in the shipped resources. Option
values come from `option_info` plus shipped `options.prf`; field order/widths
come from legacy `Net_start`. The effective SV font comes from `sv_font_open`.

These artifacts establish this implementation slice only. Controller review,
fresh final audit, SV-B-023 pending native/full acceptance, downstream version,
dedicated, failure, Guide/MOTD checks, and SV-B-025 entry evidence remain owned
by their originating tickets. They are not declared passed here.

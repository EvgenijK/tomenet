# SV-B-003 startup profile implementation evidence

Status: partial implementation; full ticket readiness and acceptance remain pending.

## Verified production path

`src/tomenet-sv --endpoint` loads `U/sv/tomenet.cfg` before creating the window
or opening its text font. No CFG or `S` directory is created by this startup path.
The parser uses complete keys, handles LF/CRLF and duplicate assignments,
defaults invalid values, refuses a newer schema, and ignores the `pass` field.
The selected text font is tried from the shared user overlay before the bundled
asset, then the declared bundled fallback. The endpoint scene applies the
loaded UI scale. Command-line window mode and UI scale override the invocation
without saving it.

`tests/sv_profile_checks.py` invokes the production executable with an isolated
profile, a decoy legacy CFG, a local server list and SDL's dummy/software
renderer. It checks defaults, parsing, invalid and corrupt input, future
schema (including indentation), requested PCF, bundled font fallback,
command-line precedence, and unchanged CFG bytes.

Verified on Linux amd64 with build `linux-9dd5a1e98a672a5b25cd`, SDL3
3.4.16, SDL3_ttf 3.2.2, and FreeType 26.6.20:
`tests/sv_profile_checks.py`, `tests/sv_endpoint_checks.py`, and
`tests/sv_contact_checks.py` passed. The bundled Cascadia TTF SHA-256 is
`06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481`;
the bundled `16x24x.pcf` SHA-256 is
`fb72acdee8d41ed41d2dfc3fc769e4a629e02431376f09485d73be4c5e7758b1`.

The full Stage A runner was invoked in the sandbox at
`/tmp/sv-b003-stage-a-20260924/report.json`, then outside the sandbox at
`/tmp/sv-b003-stage-a-escalated-20260924/report.json`. The second run passed
Linux SV, SDL3 legacy, and X11 legacy builds, the headless architecture,
message, request, and lifecycle checks, and the Linux software/OpenGL native
scenarios. The overall gate remains blocked: the MinGW SDL development packages
and system Python `jsonschema` are unavailable. The registry-related checks
fail because `jsonschema` cannot be imported. Sandbox-only ptrace and display
failures did not recur outside the sandbox.

## Pending ticket obligations

This implementation does not yet load the global/system/character OPT layers,
convert their aliases, or send them at the pre-login packet point. Map font,
graphics, pack, and audio fields currently retain requested startup values;
there is no map renderer or audio executor in this early SV executable to
establish their effective state. Resource mutation owner and versioned
`Send_version`/`Send_audio`/`Send_font` reporting remain unimplemented.
Consequently none of the eight owned capability IDs or the ticket's
implementation readiness is claimed complete. The later integration and
Windows platform checks listed in the ticket also remain pending.

## Scope and provenance

Source authority: `docs/tasks/stage-b/SV-B-003-profile.md`,
`docs/capabilities/settings-policy.md`, `src/client/client.c`,
`src/client/c-files.c`, and `src/client/sv/endpoint-run.c`.
Fixtures use temporary `U` roots and the checkout `lib/` assets. No password
value is read from a CFG or emitted in diagnostics.

# SV-B-003 startup profile implementation evidence

Status: partial implementation; full ticket readiness and acceptance remain pending.

## Verified production path

`src/tomenet-sv --endpoint` loads `U/sv/tomenet.cfg` before creating the window
or opening its text font. No CFG or directory is created by this startup path.
The parser uses complete keys, handles LF/CRLF and duplicate assignments,
defaults invalid values, refuses a newer schema, and ignores the `pass` field.
The selected text font is tried from the shared user overlay before the bundled
asset, then the declared bundled fallback. The endpoint scene applies the
loaded UI scale. Command-line window mode and UI scale override the invocation
without saving it.

`tests/sv_profile_checks.py` invokes the production executable with an isolated
profile, a decoy legacy CFG, a local server list and SDL's dummy/software
renderer. It checks defaults, parsing, invalid and corrupt input, future
schema, font fallback, command-line precedence, and unchanged CFG bytes.

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
value is read from a CFG or emitted in diagnostics. Build and test results for
this work are recorded in the implementing commit message.

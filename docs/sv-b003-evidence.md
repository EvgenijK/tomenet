# SV-B-003 startup profile implementation evidence

Status: partial implementation; full ticket readiness and acceptance remain pending.

## 2026-09-25 incremental production work

The endpoint executable now loads `S/options.prf`, `S/global.opt`, and
`S/global-sv.opt` before contact. The SV-only options module holds 199 packet
slots with the 188 currently named baseline defaults, converts every explicit
`c-files.c` alias branch in memory (including inversion, one-to-many, and
discard), and exposes character-layer application plus the four versioned
`PKT_OPTIONS` lengths (97/129/155/200 bytes including the type byte). The
character layer and packet serializer are production code exercised through
their public seam; the current endpoint executable has no character selection
or first option send and therefore cannot apply them in a login yet.

The resource resolver gives U overlay priority over B for concrete paths and
keeps a single owner for stat, sibling temporary path, rename, and remove.
Writable sidecars, rename, and remove reject bundled B refs at the production
API boundary; the test checks a B ref remains readable and unchanged after
rejected remove and rename attempts.
The production text-font loader uses it. Endpoint startup reports requested
map-font, graphics, sound-pack, and music-pack names together with a found or
missing source. These are availability observations, not effective renderer,
pack-loader, or audio-device state.

Linux amd64 worktree based on `fca870549` (uncommitted B-003 changes), build
`linux-9dd5a1e98a672a5b25cd`, SDL3 3.4.16, SDL3_ttf 3.2.2, FreeType 26.6.20:
`make -C src -f makefile.sv tomenet-sv -j4`,
`python3 tests/sv_options_checks.py`,
`python3 tests/sv_profile_checks.py`,
`python3 tests/sv_endpoint_checks.py`,
`python3 tests/sv_contact_checks.py`, and `git diff --check` passed. The
options test invokes the production options/resource modules with isolated U/B
roots, checks layer order, alias branches, versioned packet bytes, refusal of
cross-owner rename, overlay fallback after remove, and unchanged OPT sources.
The profile executable test checks startup source availability and missing
pack reporting in addition to prior CFG cases. SDL video ran dummy/software;
the OPT/resource seam used temporary filesystem roots.

Source fingerprints: `src/client/c-tables.c`
`94cc27da7b272aa7d6967a39b1731cd94fe22547088c1f455c1f1a69cb4957f4`;
`src/client/c-files.c`
`3e3b0c66e859b8b9780b461f6f2c2305949ab3b17fb82a166124e01f5e94ecf5`;
SV option snapshot `src/client/sv/options-table.inc`
`154f386de394d5f7c57380f32da0da8f3123c64f32f8840dd97112f3d19f3851`.
The preexisting bundled font fingerprints below remain current.

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

Global/system OPT loading and alias conversion are implemented. Character OPT
application and versioned packet construction are production seams without a
current login caller. Map font, graphics, pack, and audio fields retain
requested startup values and report source availability; there is no map
renderer, pack loader, or audio executor in this early SV executable to
establish their effective state. Resource owner primitives exist, but no
settings/resource editor yet uses them for a full write transaction. Versioned
`Send_version`/`Send_audio`/`Send_font` reporting remains unimplemented.
Consequently none of the eight owned capability IDs or the ticket's
implementation readiness is claimed complete. The later integration and
Windows platform checks listed in the ticket also remain pending.

## Scope and provenance

Source authority: `docs/tasks/stage-b/SV-B-003-profile.md`,
`docs/capabilities/settings-policy.md`, `src/client/client.c`,
`src/client/c-files.c`, and `src/client/sv/endpoint-run.c`.
Fixtures use temporary `U` roots and the checkout `lib/` assets. No password
value is read from a CFG or emitted in diagnostics.

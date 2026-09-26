# SV-B-007 partial production evidence — 2026-09-26

Status: checked implementation slice; full B acceptance pending.

Fixed point: `a3d70bbd71239e1e8a923404a13dc811d7524956`. Branch:
`codex/sandcastle-dev-4288c03b` (implementation commit in Git history).
Platform: Linux amd64, clang 19.1.7, SDL3 3.2.10, SDL3_ttf 3.2.2,
FreeType 26.2.20, libsecret 0.21.7. Build:
`linux-8b3130721e8ad8146e3e`, `make -C src -f makefile.sv tomenet-sv`
passed. Renderer: SDL dummy software for native smoke. The focused check uses
a synthetic peer and protocol version `4.9.4.0.0.0`; it does not substitute
a behavior implementation for production SV code.

`tests/sv_macro_checks.py` compiles the production PRF parser, matcher, input
router, app, session and packet serializer under ASan/UBSan. Exact observations:

| Input | Packet bytes |
|---|---|
| `a`, then `b`, with triggers `a` and `ab` | `156,66` (longest `B`) |
| `a`, then unmatched `c` | `156,65,156,99` (short action, then pushback) |
| normal/physical up mapping | `70,8` (`PKT_WALK`, direction 8) |
| macro `:hi\r` | `46,104,105,0` (`PKT_MESSAGE`) |
| `\w05` before and after interleaved message/confirm | `156,78`, then `156,83`; no redraw replay |
| `\w99` released by server key request | `184,0,0,0,9,89` (one reply `Y`) |
| `\W0005` fresh Space / other `v` / Escape | `156,82` / `156,82,156,118` / no continuation |
| queued `!` actions `1`, then `2`, before fresh `c` | `70,2,70,1,156,99` |

The same check covers bootstrap exclusion of legacy `options.prf`, ordinary
explicit inclusion, U/B owner precedence, own OPT precedence, character/race/
trait/class/form layers, body `?` guard, aliases, `#` messages,
missing/invalid/recursive includes, prompt macro policy and generation reset.
`tests/sv_request_checks.py` verifies native Escape cancellation during xwait
and control-prefix input. Both passed.

Final suite: 28 of 31 SV scripts passed with `jsonschema==4.26.0` in a
temporary virtual environment. Two registry scripts failed on the pinned
`src/makefile.sv` source digest after this build change; the canonical registry
was left untouched. The native lifecycle scene failed because SDL dummy does
not support `SDL_MinimizeWindow`. Linux accelerated, MinGW/Wine and actual
Windows software/accelerated checks remain pending. No full capability ID is
accepted from this partial record.

## Fingerprints (SHA-256)

| File | Digest |
|---|---|
| `src/client/sv/preferences.c` | `266cb8418b27529507c0469cc62761938181c3c8b8f72b7aa1b55a1c2b3dd9ac` |
| `src/client/sv/input/macros.c` | `55b1275370b24c3ebcdc18140078a123a9110c69ff383bf0770a64cde0e43a7c` |
| `src/client/sv/input/command.c` | `605d2e05a7f4ea13383f3b712f3368934e6743c4dd06869b543f82a5e30a04ef` |
| `src/client/sv/app.c` | `3ce1142fbe26523b7e9c7c8bd8089683ae6550277a81684c028ab81e3e7330b4` |
| `src/client/sv/main.c` | `41f6a2758d98accb9bd465552f511c410e8d7c146c3f1fd172ca94b4528e952c` |
| `src/client/sv/protocol/protocol.c` | `1f851ad61551fa54457c3a8ec5a042a82d2f4e59c773c0c209bc88fe82e1361b` |
| `src/client/sv/options.c` | `235d1b1ab96711ad83cd253c8d827ee86f5cd0725fb20640e2d16652a728e0ec` |
| `src/makefile.sv` | `f3d77ac7c7b69e9b7ef1dbbe683091f208965b2bd635346dca59fd794065e9a9` |
| `tests/sv/macros.c` | `b991beaaf6e3878cf613ae0dc75599b8605defa2e969bc8bc9a3f6e31673a308` |
| `tests/sv_macro_checks.py` (fixture generator) | `e4eedce1e18bf1f96ac0834a9b8cafffff02ef6bfe35fa96fb6b4f7ec5d21fe8` |
| `lib/user/pref.prf` | `75bf31092c4426de28db8b9dc1e24c21d588f6f70c2ddebc48836df089cc6751` |
| `lib/user/pref-sdl3.prf` | `3ebe13feadd0682b5c616e694c90e3076511a2e57eea191f0c897e9f5d6b11ab` |
| `src/tomenet-sv` | `920442f6e703e2c290686db73cbd9c4657c2100bb5b058caab161cfdd7ee420a` |

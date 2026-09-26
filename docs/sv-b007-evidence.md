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

## Review follow-up — 2026-09-26

The earlier fingerprints above identify the first implementation commit; this
section records the subsequent production changes on the same branch.

- Gameplay packet selection now lives in `input/command.c`; `app.c` only
  verifies session/context and calls its stable dispatch interface.
- Macro wait registers with the generation-bound SV-B-002 confirmation waiter.
  The network decoder deposits each `PKT_CONFIRM` there; the macro frame takes
  one confirmation and ends its owner before continuing. The focused check
  verifies ownership contention, interleaved message/confirm, one continuation,
  no redraw replay and stale generation rejection.
- The persistent preference runtime records every applied PRF effect's U/B
  owner, resolved `U/user` or `B/user` path and line. It publishes the parsed
  macro/keymap profile to the same `SvApp` used by physical input. Failed `%`
  records identify the including path/line and target; a missing manual class
  file now warns. Native Ctrl+F7 named load and Ctrl+F8 class load use this
  runtime. Escape restores gameplay input without consuming queued actions;
  a warned load cannot be repeated by a second Enter without editing the name.
- The production-path focused fixture verified named and class movement
  packet bytes, character OPT and race/trait/class/character/form precedence,
  include diagnostics, native close/queue exactly once, and the existing
  wait/request cases under ASan/UBSan. `tests/sv_macro_checks.py`,
  `tests/sv_request_checks.py` (372 cases), `tests/sv_options_checks.py`,
  `make -C src -f makefile.sv tomenet-sv`, and `git diff --check` passed.
- Full `tests/sv_*.py` core sweep with `jsonschema==4.26.0` and SDL dummy
  software: **28 passed, 3 failed**. `sv_capabilities_checks.py` and
  `sv_checkpoint_checks.py` report only the existing pinned
  `src/makefile.sv` source-digest mismatch; the canonical registry remains
  unchanged. `sv_lifecycle_native.py` fails because SDL dummy rejects
  `SDL_MinimizeWindow`. Linux accelerated, MinGW/Wine and Windows runtime
  evidence remains pending.

No full B-007 capability is accepted. `endpoint-run.c` stops after
`SV_LOGIN_SELECTED` before a gameplay `SvApp` or options packet. There is no
production character/form update change in `session/session.h`; live initial
and later layer reloads depend on those seams. `ui/ui.c` is a synthetic shell,
without a map visual model, mapping table or renderer consumer. Graphical PRF
records remain diagnosed as pending and have **not** been claimed applied.
Native loader availability on the live endpoint depends on the same
endpoint-to-gameplay handoff.

Selected follow-up SHA-256: `preferences.c`
`54b8a4554cf88fe4a27d740e0ea081a50258c629d93710a244020b8309936ecd`,
`preferences-runtime.c`
`298cdce7c2cb0374dfdcf35a8b3bca7acfca180f9d18bcf826e285609efd1fc3`,
`input/native-macro-loader.c`
`22704a7e9722b9d1f347d94c634706a5fa0840c0d3842f3b02430abf512cda86`,
`input/command.c`
`85adf72a1f774e799570143b03d18a1e07bedd7a9bd77546ab0042ceae2cbacb`,
`app.c` `1a10d4b3001373c264e9938e70d93c10a67030d875d1bddba996f2229a759757`,
`main.c` `32d15650510926853f88d9ca88c0966bf4be433e60033bb43fb3d7f6d4e6cfcc`.

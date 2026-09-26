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
  verifies session/context and calls its stable dispatch interface. Macro
  matching, waiting and action dispatch live in `input/macro-executor.c`.
- Macro wait registers with the generation-bound SV-B-002 confirmation waiter.
  The network decoder deposits each `PKT_CONFIRM` there; the macro frame takes
  one confirmation and ends its owner before continuing. The focused check
  verifies ownership contention, interleaved message/confirm, one continuation,
  no redraw replay and stale generation rejection.
- The persistent preference runtime records every applied PRF effect's U/B
  owner, resolved `U/user` or `B/user` path and line. It publishes the parsed
  macro/keymap profile to the same `SvApp` used by physical input. Failed `%`
  records identify the including path/line and target; successful `%` records
  retain both that source and the target's resolved U/B owner/path. A missing manual class
  file now warns. Native Ctrl+F7 named load and Ctrl+F8 class load use this
  runtime. Escape restores gameplay input without consuming queued actions;
  a warned load cannot be repeated by a second Enter without editing the name.
- The production-path focused fixture verified named and class movement
  packet bytes, character OPT and race/trait/class/character/form precedence,
  one `global.prf` load after character OPT, zero-command and normalized
  direction `S` records, include diagnostics, native close/queue exactly once,
  incomplete-load status, and the existing
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

## Review corrections — 2026-09-26

The named review artifact `.sandcastle/reviews/codex-sandcastle-dev-4288c03b-branch.md`
was absent from this worktree. The follow-up addressed the actionable findings
quoted in the task request.

Ctrl+F7/Ctrl+F8 now create a worker snapshot of production SV options, macros,
keymaps and PRF parser state. The worker performs file I/O and parsing without
touching `SvApp` or the live preference runtime. It retains a bounded effect
journal (8,192 records); the main thread checks generation and macro idle state,
then applies profile and queued effects in parser order, at most 16 journal
records per frame. Escape/F5/teardown discard a result before commit; after
commit starts, Escape closes the loader when replay ends and F5 waits for that
end. Main-loop gameplay input dispatch waits during replay; queued server
request replies continue to dispatch while network/UI remain serviced.
An interleaved decoded key request leaves committed replay intact through all
40 fixture effects, receives its exact reply through native input, and allows
the deferred Escape close. A closed app rejects a completed but uncommitted
worker result even if generation is unchanged. Named/class success, missing
include warning, cancel before commit, multi-frame replay/close and exact
movement bytes passed through the native loader and production app under
ASan/UBSan. A worker still blocked inside filesystem
I/O can outlive a canceled UI view; its isolated state is reference counted and
it cannot publish after cancellation. The fixture did not simulate a blocking
filesystem cancellation. An effect-delivery error reports a failed/incomplete load after earlier PRF
effects remain applied, matching ordinary preference loading's sequential
semantics; the failure flag is cleared for the next load. The fixture forces
queued-action overflow in one ordinary load and verifies that a later load
succeeds.

The SDL event epoch now gates key/text input before the shell UI, loader and
gameplay adapter. The production loader test rejects old queued opener and
Enter timestamps, including after a generation restart. Repeated character
and form loads apply from the saved global profile; the global PRF effect
origin count stays constant across reloads. Character OPT is loaded before
global option effects even for global→character. `load_form_macros=true` loads
the form layer; `false` leaves the class layer in effect. The fixture checks
exact `PKT_RAW_KEY` output and queued character action bytes.

The final Linux build and `tests/sv_macro_checks.py` passed. The full
`tests/sv_*.py` sweep with `/opt/sv-venv/bin/python`, SDL dummy and software
renderer was **28/31 passed**: `sv_capabilities_checks.py` and
`sv_checkpoint_checks.py` report only the historical pinned
`src/makefile.sv` source digest mismatch; `sv_lifecycle_native.py` reaches
`SDL_MinimizeWindow`, which SDL dummy reports unsupported. The manifest source
revision remains unchanged. Live login handoff, later decoded form/character
updates, visual mapping consumer, Windows and accelerated renderer evidence
remain pending. No full SV-B-007 capability is accepted.

Selected follow-up SHA-256: `preferences.c`
`33c40bd3ae77fa0096123d1cfb2d90a9348ab3ff611bd7d21fcdfd1671fdffc1`,
`preferences-runtime.c`
`dea52d51d7fe8970501ec7d0870cac82f4f1ec5277998aefb30f18569616cfa3`,
`input/native-macro-loader.c`
`01cf65eb806a201688c2437c26897ae61f07e9cbc4d70f31437812b00eba0485`,
`input/command.c`
`e46f17d30553630453bfd79f46a8a2ebe5a0884ad478646b7a798c4dd5b567e7`,
`input/macro-executor.c`
`50ae0ca593059422a28e0459463cab58d73f292a26344bb4ed6b48ea274dcb3c`,
`app.c` `d238caf00d65ad5e97e9a37107feeadfe2e01317e29b7d46e28d88e152f32edf`,
`main.c` `11ee2c404498200b2f4b52dfe85365b87319ce029226d7c0b95378502ec4201a`,
`makefile.sv` `a5dbd2b73cd4a11b5aada5da516f5b313093159674c8948fe6cfb6e8eae04015`,
`tests/sv/macros.c`
`40fe109d33ba758c3205fc91c62fa35d7b8e9cf31164d3b83032f19b47de12be`,
`tests/sv_macro_checks.py`
`b08b85566f97958cba2799d1b3da0959837a70ce6de21414975cca670c39529c`,
`src/tomenet-sv` `d6a0ace669970c9c28344accf1017eefd3058c5cbb29ebf291a5ef02311d6dde`.

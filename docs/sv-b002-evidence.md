# SV-B-002 — scoped implementation evidence

Date: 2026-09-24. Review base: `5945b264add55a9eac70ca56ea44348330314acd`.
Status: TCP contact and the tested control subset run through production SV on Linux;
full SV-B-002 acceptance remains pending.

## Production path

`src/tomenet-sv --endpoint` selects a server, accepts account and private password
bytes through the native form or `--account`/`--password-stdin`, then uses the
SV contact parser and nonblocking TCP transport. The parser validates the contact
reply marker and negotiated version, sends verification, and retains race, class,
trait, creation flags and MOTD data only after a complete setup. The application
copies setup into the session generation before draining chained packet bytes.
The endpoint runner owns the session loop and distinguishes socket, rejection,
verification, setup and app failures. Unsupported `*` in the native password
form leaves the draft editable. Native account/password input accepts only
printable ASCII until the Unicode-to-wire mapping is established; an unsupported
SDL text event leaves the private draft intact and blocks Enter until editing
resumes. Raw CLI password bytes pass to contact unchanged before the baseline
protocol XOR.

The production decoder handles keepalive, ping echo/pong telemetry, unknown,
partial and malformed packets, keypress/end markers, server flags, pause, flush
and confirmations. Input owns pause state and a generation-bound confirmation
waiter; the native contact view acknowledges pause on a fresh key and submits
flush-triggered presentation without blocking network progress. Output writes
are whole packets and wait on a full queue.

## Observed configuration and results

- Linux 6.18.49-1-MANJARO x86_64; clang 22.1.8; SDL3 3.4.16,
  SDL3_ttf 3.2.2, FreeType 26.6.20; `SDL_VIDEODRIVER=dummy`,
  `SDL_RENDER_DRIVER=software`; isolated temporary profile and `lib/`.
- Build: `linux-9dd5a1e98a672a5b25cd`,
  `src/.sv-build/linux/9dd5a1e98a672a5b25cd/build.txt` SHA-256
  `07f643a8c76d1237aaeb716e37b5057e76ffdc9c60669a28c5133ff8d809d30e`;
  executable SHA-256 `4153c2fe517f1e7cb9f6c97ff898805b84f51eadbcf824e725cb86d6752e39cf`.
- Font SHA-256 `06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481`.
  Contact parser SHA-256 `167dcb3d7bbf7ddce6956926f816fd01ede394eb154ffce557740819e7ed4f70`;
  socket adapter `4d9a6c0d94c93f43688fe6652883af1cb1ba45a016f401f56fc2c78902c12aa3`;
  live fixture `58cbdcac62dbed47e7bd201c05a95f4c7459ad04061659965611a69ef30a2cf5`.
- `tests/sv_contact_checks.py`: pass under ASan/UBSan. It checks split fields,
  old and current setup layouts, marker/rejection/malformed failures, session
  setup ownership, control packet order, pause, flush policy, ping and send
  backpressure through production code.
- `tests/sv_contact_live_checks.py`: pass on local TCP outside the filesystem
  sandbox. It observes exact contact/verification bytes, fragmented response
  and setup, server setup handoff, ping echo, flush then unknown reply, and
  distinct ban, verification and setup failures in the native executable.
  The 2026-09-24 rework additionally passed a fragmented 4.4.3.1 setup layout,
  local refused TCP connect and a silent peer's 10-second contact timeout.
- `tests/sv_endpoint_checks.py`: pass after linking the scene fixture against
  the current production SV app and protocol. Contact field checks cover a
  refused UTF-8 non-ASCII event preserving its private draft and subsequent
  ASCII edit. `tests/sv_contact_checks.py` observes the exact verification
  bytes for a raw 0xE9 password byte under protocol 2.
- Rework Linux executable SHA-256
  `ea5cdbd0a3039ec8c6123e9fbd3fe1ffec2193df97a5e9a7fb51eaaee92d6d01`;
  runner `d987ea2e8ab113609920b40858ec2c7bd67c2a79c39e2c5b4d28e2f2bba883e1`;
  native contact input `bf3ebe2c38e0e9ae84534e2ce00a849951f371c91196be3db91d704f83793a2c`;
  live fixture `0e09339598c70647d8f15aaa71a5ea4d0f14ff602799debcc8c08a30e9b0d97e`.
- Linux SV build, `sv_arch_checks.py`, `sv_request_checks.py` (372 cases),
  `sv_lifecycle_checks.py`, `sv_message_checks.py` and `sv_endpoint_checks.py`:
  pass in their scoped runs. Sanitizer runs used `ASAN_OPTIONS=detect_leaks=0`
  because LeakSanitizer cannot operate under this sandbox's tracing.
- Native request and timing scenarios pass with SDL's dummy software renderer
  after correcting their fixture Escape events to carry a real scancode and
  to expect the production physical-key route's immediate reply.
- Cumulative `tools/run_stage_a.py` report:
  `/tmp/sv-b002-stage-a-20260924-final2/report.json`, status **blocked**.
  MinGW development dependencies are unavailable; the gate's Python lacks
  `jsonschema`; dummy SDL cannot minimize a window or run OpenGL. Source-digest
  fixtures also need regeneration for the B additions, but missing `jsonschema`
  prevents the registry checks from reaching source verification. Linux and
  legacy builds, headless architecture/message/request/lifecycle checks, and
  software HP/architecture/message/request/shell/geometry/timing checks passed.
  The software lifecycle check needs a window manager that supports minimize;
  the runtime-headless check depends on the unavailable registry validator.
- Rework cumulative report: `/tmp/sv-b002-rework-20260924/report.json`, status
  **blocked**. Linux SV and both legacy builds passed; MinGW dependencies and
  `jsonschema` are absent. The unsandboxed native scene is unavailable in this
  run (`No available video device`), while sanitizer headless checks encounter
  the sandbox's LeakSanitizer/ptrace failure. This report does not replace the
  earlier desktop or platform observations.

## Remaining acceptance work

Server flags reach the session view, but no gameplay command availability
consumer exists yet. The input-owned confirmation waiter has a production API,
but the future macro executor does not yet register or consume it. The contact
view submits its own flush-triggered frame; gameplay visual submission awaits
its renderer. A real server login/character-selection flow, broader live server
versions, DNS and retry matrix, accelerated rendering, MinGW/Wine and
actual Windows 10/11 evidence remain pending. The general
`src/client/sv/input/text-field.c` editor still implements UTF-8→Latin-1 for
other fields, but the contact form rejects non-ASCII SDL events before that
conversion. A real server wire round trip for ASCII, non-ASCII and unsupported
input is still required to establish any broader credential mapping. The
raw-byte unit check proves serializer behavior only; it does not prove server
acceptance of a non-ASCII credential. SV-B-002
owns the contact wire proof; SV-B-005 owns private raw bytes and SV-B-006 owns
the interactive login caller. Exact follow-ups are in their task files.
Environment blockers are tracked by
[SV-V-001](tasks/verification/SV-V-001-b002-matrix-environment.md); platform
acceptance remains with SV-B-002 and SV-B-075.
These outcomes are not accepted by the loopback or headless observations above.

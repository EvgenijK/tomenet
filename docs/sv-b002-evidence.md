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
form leaves the draft editable.

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

## Remaining acceptance work

Server flags reach the session view, but no gameplay command availability
consumer exists yet. The input-owned confirmation waiter has a production API,
but the future macro executor does not yet register or consume it. The contact
view submits its own flush-triggered frame; gameplay visual submission awaits
its renderer. A real server login/character-selection flow, broader live server
versions, DNS/timeout and retry matrix, accelerated rendering, MinGW/Wine and
actual Windows 10/11 evidence remain pending. The native credential form uses
the general `src/client/sv/input/text-field.c` UTF-8→Latin-1 decoder. This
assumes an outgoing byte mapping that the session policy does not establish.
Baseline comparison and a real server wire round trip for ASCII, non-ASCII and
unsupported input are required; on mapping failure the private draft must
remain editable without transmitting or storing a changed secret. SV-B-002
owns the contact wire proof; SV-B-005 owns private raw bytes and SV-B-006 owns
the interactive login caller. Exact follow-ups are in their task files.
Environment blockers are tracked by
[SV-V-001](tasks/verification/SV-V-001-b002-matrix-environment.md); platform
acceptance remains with SV-B-002 and SV-B-075.
These outcomes are not accepted by the loopback or headless observations above.

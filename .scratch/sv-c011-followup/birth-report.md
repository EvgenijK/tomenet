# C011 follow-up: native birth, visual startup and local real server

2026-11-11. **Final actual local-server check PASS**: native birth, MOTD
acknowledgement, accepted identity, complete retained C011 preparation and visible
readout. No external/public server login was performed. User acceptance is not
claimed.

## Available environment

- Actual server binary: `src/tomenet.server`, SHA-256
  `7f3d1257cbee8acbf7f432eac01044afd301ab0554b645da288ff3cf88e080ba`.
  Banner identifies **4.9.3.0.2.0**, compiled 2026-09-07; the config greeting says
  4.9.4 and is not binary-version evidence.
- Local display `DISPLAY=:1` is available with approved escalation:
  XOpenDisplay succeeded, display 1932×1208. X11 + SDL software rendering was
  used; no physical GPU/accelerated or broad visual matrix is claimed.
- `unshare -Urn` works. Bringing loopback up requires approved escalation.
  The probe runs server and client in a separate user/network namespace with
  only loopback, not the host network. The server's wildcard listeners are
  confined to that namespace. No metaserver/world-server connectivity exists.
- Accepted production birth gesture driver is available in
  `tests/sv/birth-complete.c`. The follow-up reuses its rendered-view → native
  SDL-event approach, without writing birth state or implementing behavior.

## Isolated actual-server probe

Scripts and driver:

- `local-server-probe.py`: copies installation resources into an isolated
  library; creates **empty save/data** directories and local synthetic
  account/character state only. Original save/data are never copied or used.
- `real-client.c`: public `sv_endpoint_run` with preparation output, linked
  against the existing production object closure (excluding its normal main).
- `birth-wrapper.inc`: observes the real rendered birth view and supplies SDL
  gestures. The actual server enables point-buy; native Esc confirmation is
  used for its unused-points confirmation. No private birth state is mutated.
- `real-client-build.json` records compile/link commands. Server probe metadata
  records binary/source hashes, network namespace and output paths.

The only account is synthetic `C011Probe`, with local fixture credentials; its
character is `Born`. They are created solely in the fresh isolated save root.
No real user's account or secret, public-server state or late gameplay action
is involved. Probes stop at the C011 preparation boundary or a bounded failure.

Observed attempts:

| Artifact directory | Result |
|---|---|
| `local-server/` | Actual server started; isolated loopback TCP listener verified; terminated with SIGTERM |
| `real-server/` | Client stopped before network because the minimal resource copy omitted `xtra/sv` shell textures; not a production defect |
| `real-server-02/` | Actual native birth reached point-buy; the original classic-only observation driver asserted. Driver extended to point-buy native gestures |
| `real-server-03/` | Native point-buy birth committed; bounded 35-second attempt did not receive PLAY ACK. Its monotonic-owner-mask screenshot named `birth-mode.bmp` actually shows **Creation submitted / waiting acknowledgement**, not Mode selection |
| `real-server-04/` | Native point-buy birth committed; exact post-contact outgoing/incoming bytes saved. No PLAY ACK/END/identity; no prepared payload. Root is fixing the identified SV-local pre-PLAY options issue |
| `real-server-05/` | After pre-PLAY fix, actual PLAY ACK/END received. Probe lacked native MOTD acknowledgement and expired its frame budget. Its generic stale SDL error was initially misinterpreted as a draw failure; that interpretation is withdrawn |
| `real-server-06/` | Existing skip-MOTD option: startup/runtime ready; stopped at leading EQUIP before identity, 180 deferred frames |
| `real-server-07/` | After EQUIP/INVEN support, stopped at SFX_VOLUME before identity; retained the following raw batch for offline production-decoder inspection |
| `real-server-08/` | Actual complete preparation, accepted Born, runtime ready, 666 deferred frames, no unsupported packet. Probe quit before processing queued MOTD acknowledgement; its prepared.bmp actually shows MOTD |
| `real-server-09/` | Native MOTD acknowledgement processed, final phase LIVE_HANDOFF, complete preparation/identity/runtime and 666 deferred frames. Early prepared.bmp was captured at the earlier handoff milestone and shows completed-birth waiting; it is not an accepted-identity readout screenshot |
| `real-server-10/` | After the own-C011 readout fix, preparation still completes, but the observer incorrectly also used birth READY to select prepared.bmp; this caption is not valid readout evidence |
| `real-server-11/` | Corrected observer captures the actual preparation card after MOTD acknowledgement; intermediate visual proof before the final two-line failure-status precedence correction |
| `real-server-12/` | Final production snapshot: client exit 0, native MOTD acknowledged, LIVE_HANDOFF rendered, complete/startup/identity/runtime all true, unsupported packet 0, accepted Born and 666 deferred frames. Capture explicitly requires preparation_identity, no birth view/viewport and status containing Born |

## Exact boundary and discovered defect

`real-server-04/captures/post-contact-out.bin` contains 16,468 bytes:
initial LOGIN 8, choose `Born` 6, standalone OPTIONS 200, complete PLAY 16,248,
then six keepalives. `real-server-04/play.bin` is the exact PLAY frame. It contains
199 option slots, normal resource names and all 3,199 wide glyph entries.

Incoming post-contact stream contains only 30 bytes: SERVERDETAILS (17) and the
empty LOGIN list (13). There is no PLAY ACK or END. The server records the local
name choice and finally disconnects while waiting in play negotiation.

Current server source `nserver.c:Handle_input` invokes `Receive_play` in
CONN_LOGIN. `Receive_play` recognizes LOGIN and PLAY, not standalone OPTIONS;
unexpected bytes advance only one byte per invocation. This is consistent
with the captured pre-PLAY OPTIONS causing the stall.

Legacy `Send_options` only queues bytes; `Net_start` clears its output buffer
before forming PLAY. Do not overstate this as proof that legacy can **never**
flush OPTIONS: intervening interactive `inkey` loops also contain `Net_flush`.
The necessary SV fix keeps effective character options before PLAY and avoids
the unsupported standalone pre-PLAY packet. Legacy cleanup belongs separately
in the improvements list, not this implementation.

The SV-local wire fix is implemented by the integration agent. Subsequent
actual-server probes received ACK/END and advanced through startup as expected.

## Necessary prefix completion

Actual before-identity EQUIP/INVEN frames required version-aware frame-only
retention. `%I` is a plain NUL-terminated item-name field with ONAME_LEN=160,
not `%s` (80) or `%S` (256). Modern fixed prefix is 14 bytes including packet
type; older layout gates are preserved.

Actual startup then sends SFX_VOLUME, SFX_AMBIENT, MUSIC and the keyed
UNIQUE_MONSTER list before identity. These are retained without playback or
unique/item model application. Source-proven conditional startup variants
SOUND, MUSIC_VOL, INVEN_WIDE and EQUIP_WIDE also receive frame-only support.
No SUBINVEN support was added: normal fix_subinven/window_stuff runs after
PR_BASIC/first identity.

`offline-prefix.c` calls production startup-prefix/identity/app decoders against
`real-server-07/captures/retained-tail.bin`; it does not search for guessed packet
markers or implement a second protocol decoder. `offline07.log` reaches CHAR_INFO
Born at offset 15,168 after 356 remaining complete frames. Actual 08/09 runs
then reach identity without an unknown leading packet. Deferred count is 666,
within the 2,048-frame and 256-KiB bounds.

## MOTD diagnosis correction

No explicit false MOTD draw return was observed in probe 05. The driver did not
acknowledge that surface, its frame budget expired, and the generic error path
printed a stale Parameter texture invalid error left by normal cleanup. The
independent reviewer replayed the exact 2,760-byte MOTD field through production
`sv_login_shell_motd` successfully; see `motd-report.md` and the resource-probe
log. No production font/MOTD fix was required. The driver now acknowledges the
rendered MOTD with SDL input; 09 confirms the processed phase transition.

## Visual inspection

`real-server-04/captures/{birth-sex,birth-stats,birth-mode}.png` are each
**1024×768**, captured immediately after the actual corresponding endpoint draw,
before scheduling the next native gesture. BMP originals are retained.

Inspected the Mode PNG: actual mode selection, Normal highlighted, complete
six-choice grid, readable identity/stat sidebar and footer, no overlapping
controls or clipped body content at this viewport. The animated logo is part
of the rendered startup shell.

The discovered own-C011 readout issue was fixed by the integration agent:
accepted identity at the post-MOTD boundary suppresses the completed-birth
waiting surface and is rendered in the Session preparation card. Failures keep
their own status priority. The final observer requires that semantic flag,
absence of birth views and an accepted-name status before capturing.

Final files under `real-server-12/captures/` are **1024×768**:
`birth-sex.png`, `birth-stats.png`, `birth-mode.png`, `motd.png`, and
`prepared.png`, with BMP originals. Root and this agent independently inspected
the final prepared image: Session preparation, Born, race/class/trait 0, sex 1,
mode 65536 and lives 3 are readable, with no overlapping or clipped readout.
These values are the actual server response, not the birth draft. The single
X11/software viewport is not a claim of a complete renderer/platform matrix.

## Final command, snapshot and scope

Production objects were rebuilt by root after the final failure-status
precedence fix. The scratch driver was then recompiled/relinked using the exact
commands stored in `real-client-build.json`. The actual final invocation was:

```bash
SV_REAL_OUTPUT=real-server-12 SV_REAL_CLIENT=1 \
SV_PARENT_NETNS=$(readlink /proc/self/ns/net) \
unshare -Urn python3 .scratch/sv-c011-followup/local-server-probe.py \
  > .scratch/sv-c011-followup/real-server-12-summary.log
```

This requires the same approved local namespace/display access as the preceding
probes. For a repeat, choose a **new** SV_REAL_OUTPUT directory: the script
deliberately refuses to reuse an existing isolated library. Recompile/relink the
driver from `real-client-build.json` after rebuilding production objects if code
has changed; no production behavior is implemented by that driver.

`real-server-12/command.json` records actual commands, server/client binary
hashes, 149 production source/header hashes plus the two scratch driver files,
namespace identity and capture hashes. All recorded source hashes were compared
with the final workspace and matched. Server startup is successful and client
exit code is **0**; server exit -15 is the deliberate final SIGTERM. Both probes
and actual-server processes are bounded and terminated.

The run stops at C011 accepted identity/readiness and destroys its retained
payload. It does not apply deferred item/unique/audio/gameplay packets, enter a
C014 live loop, exercise public servers, use real account data or accept the
stage on the user's behalf. Managed-peer tests and canonical registration remain
owned by root/integration evidence; this report supplies the native birth,
actual-server and one-viewport visual follow-up.

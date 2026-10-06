# SV-B-006 login implementation evidence

Status: partial production implementation; full acceptance pending.

## 2026-09-25 production slice

The interactive account editor now accepts 15 payload bytes, applies the
baseline live trim (first letter uppercased; later invalid ASCII replaced
with `_`), and leaves CLI account limits to the contact producer. Escape at
account exits startup. Escape at password returns to account and cancels the
vault lookup, wipes the private draft and sends no contact packet. Empty
password stays in the editor. The existing protocol-2 `*` guard keeps the
private draft and prevents a partial verify packet; protocol 1 retains its
separate path. SDL text and clipboard retain their current ASCII-only
conversion policy while raw CLI/vault password bytes remain unchanged.

After contact setup, the native executable sends the first `PKT_LOGIN`
(including the SDL3 baseline's deterministic six-byte MD5 fingerprint on the
versioned branch), decodes
`PKT_SERVERDETAILS`, versioned character rows and their terminator, and only
then presents a server-confirmed account overview. `PKT_QUIT` returns its
server reason; selection by an owned slot or `--character` sends the exact
selected server name and waits for the separate status byte. `--character`
applies the baseline first-letter uppercase rule. A failed status is not
reported as success. Subsequent `PKT_SFLAGS` update stored flags. The input
interaction owns choice, Q, default-character resolution and MOTD
acknowledgement; a session read view separates the decoder from the scene. The scene
shows names, levels, setup race/class titles, mode and location where sent.

The B-005 vault store starts only after the terminated overview confirms the
account, using the original selected endpoint/port/account identity. Store
failure is shown in the UI and does not change the successful auth state;
pending work is cancelled at teardown. Selection status 0 presents the setup
MOTD; a nonzero key acknowledges it before the result becomes successful.
`-m` skips the MOTD explicitly. The event queue is cleared at overview and
MOTD transitions so earlier keydowns cannot choose or acknowledge them.
No gameplay command is dispatched before the absent play handshake.

## Checks

- Linux `make -f makefile.sv tomenet-sv -j4` passed with warnings treated as
  errors for SV sources.
- MinGW i686 `make -f makefile.sv PLATFORM=mingw tomenet-sv.exe -j4` stopped
  at `check-deps`: the i686 SDL/SDL_ttf/FreeType development packages are not
  available. No Windows build or runtime behavior is claimed.
- `python3 tests/sv_login_checks.py` passed. It calls the production login
  decoder/serializer with byte-split flags, versioned rows, status, dynamic
  flags, explicit >16 overflow and quit; a second case transfers bytes
  trailing setup through the production contact→login handoff. It also runs
  the production input/view seam for Q, selection, MOTD key and `-m`, and
  checks stable, profile-specific SDL3 fingerprints.
- `python3 tests/sv_login_live_checks.py` passed with local loopback TCP. The
  native executable sent exact first and selected login packets, received a
  fragmented server overview and status 0, and handled a separate server
  `PKT_QUIT` without selecting or reporting success. The peer used a temporary
  profile and an unavailable test D-Bus address, so it did not write to the
  user's credential store.
- `python3 tests/sv_endpoint_checks.py` passed after updating account length,
  live trim and Escape transitions on the production SDL credential adapter.
- `python3 tests/sv_contact_live_checks.py` passed after its peer supplied
  the now-required login phase. Its previous pre-login ping/unknown expectation
  is retained at the production app decoder seam in `contact-control.c`;
  contact trailing-byte handoff has an explicit new check. Actual post-play
  TCP ping/unknown delivery remains a later integration obligation.
- The final `tests/sv_*checks.py` sweep ran all 23 scripts: 17 passed, including
  both live socket fixtures and all B-006-specific checks. Six registry and
  evidence scripts could not complete because this Python environment lacks
  `jsonschema`; each reported `No module named 'jsonschema'`. Installing it
  into `/tmp` was attempted and failed because the configured pip transport
  lacks SOCKS support. These are environment-limited checks, not passing
  results.
- Two-axis review against starting commit `14e3cf472`: Spec review found
  that the first-login address fingerprint needed the baseline bytes, which
  were implemented and rechecked without further concrete mismatch. Standards
  review found silent overview truncation and input coupled to SDL; both were
  corrected with explicit overflow and the headless interaction/SDL adapter
  split. Standards recheck found no remaining material violation.

## Pending obligations

This is not a completed game session. The SV executable has no versioned
`Net_start`/`PKT_PLAY` setup payload, reply handling, accepted identity packet
consumer, gameplay HUD or post-play input router. MOTD colours, exact 23×120
fit and all renderer/platform branches have not been verified. The current
overview accepts at most 16 owned rows (larger replies fail explicitly) but does not enforce all dedicated
slot/capacity/first-run policy or offer reorder/create actions. Account
information/options, all retry-login branches and clearing invalid defaults
remain pending. Selection after a real server and server-owned status/error
matrix need live evidence beyond the deterministic peer.

Disconnect cancels the local vault request, frees contact/login buffers and
stops further login sends. Full session-generation invalidation, queued
requests/macros and reconnect behavior require the later play/session path.
The B-005 password-change write and full Linux/Windows provider matrix remain
pending. Primary B-006 acceptance also awaits B-020 and B-025 integration
checks; no canonical obligation is closed by this slice.

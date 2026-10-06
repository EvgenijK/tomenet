# SV-B-005 credential implementation evidence

Status: partial production implementation; full acceptance pending for all
owned capability IDs.

## 2026-09-25 implementation

`src/client/sv/credential/vault.c` serializes the exact server spelling,
effective port and account bytes as three length-prefixed fields in the
`tomenet-sv/v1/` namespace. Lowercase hex gives an exact case-sensitive
identity on Windows. Invalid/oversized identities fail rather than truncate.
Linux lookup/store uses binary libsecret values; Windows source uses
`CredReadW`/`CredWriteW`, generic type and local-machine persistence.

The native account form starts lookup after Enter. `input/credentials.c` owns
the unfinished interaction and polls an SDL worker from the render loop.
Cancel or manual editing detaches the result; a generation
mismatch cannot copy a late secret into a new form. The worker releases the
result and wipes its buffer. Missing/invalid/unavailable records produce
explicit manual session-only entry. The found password is masked and never
submits automatically. Private text and paste use the production contact
editor, accepting unambiguous ASCII from SDL text/clipboard and rejecting an
unconfirmed non-ASCII conversion there. Raw CLI/vault high bytes pass through
unchanged; the protocol `*` guard remains. The temporary editor and
clipboard copy are wiped. Account draft capacity remains 79 bytes; password
draft capacity is 15 bytes for the current contact wire field.

## Checks

- Linux amd64 `make -C src -f makefile.sv tomenet-sv -j4` passed with
  `-Wall -Wextra -Werror`. Build key: `linux-68f156082054cf264d19`,
  clang 22.1.8, SDL3 3.4.16, SDL3_ttf 3.2.2, FreeType 26.6.20,
  libsecret 0.21.7, software scene fixture.
- `python3 tests/sv_vault_checks.py` passed using the production vault. It
  asserts exact identity separation by tuple boundary, address spelling,
  account case and port; invalid length/port; provider unavailable and stale
  generation; and binary store/restore (including NUL and `0xff`) under a
  temporary D-Bus/gnome-keyring user home with synthetic bytes. No persistent
  user credential was read or written. The same fixture restores a raw
  high-byte password through the production private account form, with no
  automatic contact submission. The fixture scans stdout/stderr for its
  synthetic secret marker.
- `python3 tests/sv_endpoint_checks.py` passed. Its native fixture exercises
  ASCII-only SDL contact draft, length limits, private history exclusion and
  the selected endpoint scene. The headless credential interaction exercises
  account→password, pending lookup cancellation, private paste, masked status,
  non-ASCII SDL rejection and protocol-star rejection; the native scene script
  also cancels a pending lookup without sending contact.
- The concluding run of all 21 `tests/sv_*checks.py` scripts with SDL dummy,
  software renderer and LeakSanitizer disabled passed 15, including native TCP
  contact, endpoint, vault, arch/lifecycle/message/request, options/profile,
  settings and runtime producer. Six failed: capabilities, checkpoint,
  evidence, HTML, reconciliation and runtime checks all reported absent Python
  `jsonschema`. The run used permissions for local TCP/D-Bus and is recorded
  at `/tmp/sv-b005-suite-report.json`; the six failures remain failed gates,
  not accepted results.
- MinGW i686 full build was attempted. It stopped at `check-deps` because
  i686 SDL3/SDL3_ttf/FreeType pkg-config packages are absent. Windows source
  and behavior have no platform-runtime evidence.

## Two-axis review against `c3d0533bc`

Standards found a noncanonical include, semantic input and pending lookup
state in application code, and duplicated text/paste editing. The include now
uses the SV root; `input/credentials.c` owns the interaction and one insert
path. Spec found that an added blanket ASCII check blocked exact raw bytes
from stdin/vault; it was removed. SDL text/clipboard still reject an
unconfirmed conversion, while raw bytes reach the existing protocol transform.
The review also identified save/change-password and later consumer/platform
checks as open obligations, as listed below.

## Pending

At the B-005 checkpoint the SV executable reached contact/setup, not accepted authentication.
It therefore performs no automatic vault save after login, no immediate
password-change write, no retry/relogin restore, and no import. Those callers
belong to B-006 and later consumers; contact ready is not treated as login
success. Provider lock/unlock cancellation, external record deletion,
record replacement, all failure codes, actual Windows10/11, accelerated
renderers and end-to-end diagnostic/export scans remain pending. No claim is
made that the local temporary Secret Service fixture proves the user's
persistent provider configuration.

Sources: `docs/tasks/stage-b/SV-B-005-vault.md`,
`docs/capabilities/session-policy.md`,
`docs/capabilities/settings-policy.md`, `CONTEXT.md`,
`docs/adr/0002-main-thread-state-ownership.md`,
`docs/adr/0005-session-scoped-work.md`.

SHA-256 source/fixture/resource fingerprints for this run:

| Input | SHA-256 |
|---|---|
| `src/client/sv/credential/vault.c` | `895bcd8e1147d59cdc7936bb7717c82382703bcbcdc74a05e1848a4e589722ed` |
| `src/client/sv/input/credentials.c` | `ad84e1e1ed8b86c40e6adc3034bb49683ee763066ba0f2edb519d516caca0234` |
| `src/client/sv/endpoint-run.c` | `39fa96f536d0e4208b280e14765b3e1ccb774020d9fa30ee5738fbe6417320fe` |
| `tests/sv/vault.c` | `bd6e41942425e7ddb18be69abb784eba3103e2b51ef92cbc73952abc073ed19b` |
| `tests/sv/credentials.c` | `da84a43ec80f74c067c684fd2381590fd103fac700cbf42b0e0df1d35bf697cb` |
| `tests/sv_vault_checks.py` | `4ce3dfe7d9c56a056a1475d31d0ccf9ce531fa9c695502d3300db56fff0aa844` |
| `tests/sv_endpoint_checks.py` | `20ddcc946782157efa042531fc03338310787fe41adac830064131943f43d06e` |
| `src/makefile.sv` | `dc94130bc7b6bdbbfabb40a876a091b0a350db4ba31451fbeee1abf75a5e5154` |
| `lib/xtra/font/CascadiaMono-Regular.ttf` | `06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481` |

## B-006 caller handoff, 2026-09-25

[SV-B-006 evidence](sv-b006-evidence.md) records the production caller that
starts `sv_vault_store` only after the server's terminated account overview.
The interactive account field now has the baseline 15-byte limit and Escape
from password returns to account. A no-service loopback fixture exercised
the confirmed-account path without writing to the user's store. Successful
provider persistence on this caller and password-change write still require
the remaining B-005 integration checks; this addendum does not close them.

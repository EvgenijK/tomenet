# SV-B-005 credential implementation evidence

Status: partial production implementation; full acceptance pending for all
owned capability IDs.

## Linux freshness review — 2026-10-28

Scope follows `docs/agents/platform-validation.md`: Windows validation is
deferred, and no Windows check was run. The historical observations below
remain historical; password-change write and later-stage relogin/import
callers do not block the narrowed Stage B scope.

Environment: Linux 6.18.49-1-MANJARO x86_64, clang 22.1.8, SDL 3.4.16,
SDL_ttf 3.2.2, FreeType 26.6.20, libsecret 0.21.7. Initial runs below reused
existing checks. The subsequently authorized minimal core-test addition is
recorded in the next subsection; production code remains unchanged.

| Existing check | Result | Boundary proved |
|---|---|---|
| `python3 -B tests/sv_vault_checks.py` | PASS, exit 0 | Production identity bounds/separation, actual isolated Secret Service binary store/lookup including NUL and `0xff`, missing record, unavailable service, stale generation, raw high-byte restore through production private field. |
| `python3 -B tests/sv_account_create_checks.py` | PASS, exit 0 | Full production endpoint/contact/login/pregame caller with controlled peer; server-confirmed save timing, exact identity/secret bytes and saved/session-only result policy. OS provider is a double in this suite. |
| `python3 -B tests/sv_account_failure_checks.py` | PASS, exit 0 | Production rejection/cancel/malformed/disconnect/retry path with loopback peer and actual vault unavailable-service boundary. |

The first vault invocation inside the sandbox could not bind the temporary
D-Bus Unix socket (`Operation not permitted`). The identical command passed
outside that socket restriction. Both account suites also used permission
for their loopback sockets. Successful logs are
`/tmp/sv-b005-linux-vault-20261028.log`,
`/tmp/sv-b021-linux-create-20261028.log` and
`/tmp/sv-b021-linux-failures-20261028.log` respectively. These `/tmp` logs are
ephemeral; the commands above reproduce them. Vault stdout contains only
three check-success lines; the existing runner scanned captured provider
stdout/stderr for its synthetic secret marker. Account suites use
SDL dummy/software; none of these runs is human native input acceptance.

The actual service is gnome-keyring running under the current Unix user in
a temporary HOME/XDG directory and private D-Bus session. This proves actual
provider API use without touching the user's persistent credential store;
it does not establish other-user isolation or acceptance of the user's
persistent desktop provider configuration.

Remaining Linux acceptance obligations:

- Actual provider-owned unlock prompt, cancellation/refusal and UI/network
  responsiveness while the prompt remains pending; late completion after
  cancellation/relog must not change the new operation.
- Invalid provider records and the remaining actual refused/error failure
  matrix. External deletion/replacement and headless locked read/write now
  have actual evidence below. Linux `vault.c` currently maps
  libsecret `GError` to `SV_VAULT_UNAVAILABLE`; B-021 double results for LOCKED,
  REFUSED and ERROR prove caller degradation, not actual provider outcomes.
- Actual successful persistence through the full server-confirmed caller.
  Existing actual-provider and full-caller checks prove their respective
  boundaries separately; they are not a combined provider/caller acceptance.
- Native private-input visual/input review and complete diagnostic/export
  secret exclusion for current B surfaces. Synthetic mask/history/clipboard
  assertions and the runner's stdout/stderr scan are narrower evidence.

Thus the refreshed core evidence does not close B-005 or B-021 native/provider
acceptance. No new test harness or suite was added.

SHA-256 fingerprints for the refreshed run:

| Input | SHA-256 |
|---|---|
| `src/client/sv/credential/vault.c` | `895bcd8e1147d59cdc7936bb7717c82382703bcbcdc74a05e1848a4e589722ed` |
| `src/client/sv/input/credentials.c` | `cddb96c935119ccb5aa276850c12e22205aa43f4aa846599cdefe3b4d75e6ce2` |
| `src/client/sv/endpoint-run.c` | `7b0c2c3edd417e69363025e3d9ecc1509968099e89b502672c061cd04937a74a` |
| `tests/sv/vault.c` | `bd6e41942425e7ddb18be69abb784eba3103e2b51ef92cbc73952abc073ed19b` |
| `tests/sv/credentials.c` | `e864efbe3635038ef4f9e01df94b700b05a864779b2bd0ffcf48a3aa1877eab6` |
| `tests/sv_vault_checks.py` | `4ce3dfe7d9c56a056a1475d31d0ccf9ce531fa9c695502d3300db56fff0aa844` |
| `tests/sv/account-create.c` | `4330ce8fab75052aa8fdd0cc92c21c4e3bc44380b352cf20f2564528b3f36092` |
| `tests/sv/account-create-failures.c` | `002b710f67777c970d4f18f4be6f8595acd3d641bba5f3fc66e6df2c2c3b9095` |
| `tests/sv_account_create_checks.py` | `e8129bc720562a687bc1e78691eb2aba6dc6c7aead52561bd9fb95b99cf84de6` |
| `tests/sv_account_failure_checks.py` | `7e563c43bf0eacfa8adff1765988acbfdfa1fe0eb649bf58ce8865a5661626e6` |
| Vault success log | `500a394f318a94ff7adaded7b968e3fe87cc6b4fa453156c000f0f8d6a16f13e` |
| Account creation success log | `581bdf7f2bb69b637ae0128d996304e2b2d5aee72b91b0419900835d6397270f` |
| Account failure success log | `75b7af053e07af688b24c53bd464fbca4543377af66281f8c793d3c816e6759d` |

### Authorized minimal replacement/deletion/locked core checks

After explicit user authorization on 2026-10-28, the existing
`tests/sv/vault.c` provider branch was extended with external libsecret
mutations. Every observation still invokes the production asynchronous
`sv_vault_lookup` / `sv_vault_store`; no separate vault behavior implementation
or new runner was introduced. `tests/sv_vault_checks.py` runs this branch last
because it locks the temporary collection, clears DISPLAY/WAYLAND_DISPLAY
to avoid desktop prompts, and sets an explicit isolated-provider guard.
Running the destructive provider branch without that guard asserts before
writing or locking a collection. The service remains confined to temporary
HOME/XDG directories and its own D-Bus session.

`python3 -B tests/sv_vault_checks.py` passed, exit 0, with ASan/UBSan:

- External binary replacement (`n`, NUL, `0xfe`) was returned byte-exactly by
  the next production lookup, rather than the previous cached value.
- External deletion was followed by production `MISSING`, zero output length.
- After production restore, `secret_service_lock_sync` reported exactly one
  locked collection. Actual production lookup returned `MISSING` (enum 2),
  zero output length and no secret bytes. Actual production store returned
  `UNAVAILABLE` (enum 3), never `SAVED`.

This is actual locked-provider read/write degradation without an interactive
prompter. libsecret/gnome-keyring reports a locked lookup as missing in this
headless configuration, while the store failure is mapped by production code
to unavailable. The run does not claim distinct `LOCKED`/`REFUSED` outcomes,
successful unlock, user cancellation/refusal, prompt responsiveness, or
full-caller persistence acceptance. The generic polling bound retains safe
cancel behavior if another provider remains pending; this run completed both
locked operations without reaching that bound.

Final stdout includes `locked lookup=2 store=3`; the wrapper's existing
synthetic-secret marker scan passed. Log:
`/tmp/sv-b005-linux-vault-expanded-20261028.log`.

| Updated input | SHA-256 |
|---|---|
| `tests/sv/vault.c` | `5acd8b0def792290f38b9d87e220b867d3c4606048ebedc46f3d30b6b0c8f25d` |
| `tests/sv_vault_checks.py` | `d09d1601aaf9d2149637abe9ec1ae8dc4ff5322e261fb6366a28c5673a282d7f` |
| Expanded vault success log | `e228f90c6dfed20f6848578bc003af9eb934569d46141d957d8392d456178429` |

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

# SV-B-024 controlled-peer audit packet

Immutable contract: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.
Current recovery gates and exact-final-HEAD receipt location: [RECOVERY.md](RECOVERY.md).
The original packet below retains author/parent-HEAD provenance; it is not
substituted for the independent recovery verdict.

Originating ticket: `docs/tasks/stage-b/SV-B-024-birth-dna.md` (unchanged).
Upstream dependency: SV-B-023, production foundation present; its full/native
acceptance remains pending at that owner. This packet does not close the parent.

## Reproduction and provenance

From the repository root, with clang, pkg-config, SDL3, SDL3_ttf, FreeType and
libsecret development packages installed:

```sh
python3 tests/sv_birth_runtime_evidence.py /tmp/sv-b024-fresh-evidence
```

Use a new/empty directory. The collector removes inherited scenario filters,
runs only the six focused B-024 endpoint suites, and records real exit codes,
source fingerprints, exact tested HEAD, commands, capture sizes and SHA-256
digests in `manifest.json`. It stops on failure and retains the failing log.
It never runs the full build/core/review workflow or marks acceptance passed.
Each suite compiles a native executable linking production SV endpoint modules
under strict warnings and ASan/UBSan. The C harness supplies native SDL gestures
and the controlled TCP peer; it does not replace birth rules, serialization or
reply ownership. These are endpoint-seam executable runs, not invocations of the
packaged `src/tomenet-sv` CLI. That build is a separate gate; its actual recovery result is in RECOVERY.md.

Retained fresh results are in [runtime/manifest.json](runtime/manifest.json).
They describe the tested production parent HEAD, not a later commit containing
this packet. No final-HEAD controller verdict is embedded or inferred. The
controller must record `git rev-parse HEAD` after integration, rerun this command
against that exact HEAD, and attach its own verdict with the contract digest.
The source fingerprints let it distinguish unchanged implementation inputs from
the changed evidence packet; they do not replace that fresh audit.

## Scenarios and applicability

All rows are current SV-B-024 obligations. A suite exit code of zero means its
native assertions passed; it does not mean controller acceptance passed.
The fresh run passed all **108 sessions** (all six suite exit codes zero).
[SCENARIOS.md](SCENARIOS.md) lists each applicable scenario, its real observed
endpoint trace and the corresponding retained log.

| Suite / artifacts | Sessions | Checked production outcome |
| --- | ---: | --- |
| `runtime/ordinary/` | 3 | Full literal current request, no early send, absent/two-byte/full authoritative ack; startup handoff only after full ack |
| `runtime/versions/` | 40 | Below/at/above all distinguishable baseline version gates, plus current version; exact full requests and actual ack captures |
| `runtime/dedicated/` | 20 | IDDC, Soloist, Hellish, unavailable trait, PvP Maia; waiting/ack in both class/race compile orders; exact dedicated requests |
| `runtime/submission/` | 18 | Quit/cancel at seven legal owners, waiting, partial, ack, new-generation retry; double confirm, resize/focus, interleaved ping and timer keepalive do not duplicate play |
| `runtime/replies/` | 17 | Reject, peer quit, disconnect, wrong reply type/packet, split ack at both bytes, pending prefixes, truncated EOF, trailing/interleaved packets, retired transport/stale input retry isolation |
| `runtime/guide-motd/` | 10 | Actual native Guide entry/return at available owners, committed draft preserved, peer MOTD/skip and point-buy in both compile orders, live handoff with gameplay inactive |

The detailed independent version oracle and applicability are in
[slice 02](../02/README.md). Resource strings apply from 4.8.1.2.0.0; older layouts
omit them. Audio/graphics disabled and zero visual tables are the native text-font
outcome, not omissions of required handshake bytes. No unlisted real-server or
alternative graphics/backend outcome is claimed. Independent baseline fixture
provenance is in `tests/sv/fixtures/birth-current-baseline.py` and
`birth-dedicated-baseline.py`; expected bytes are not extracted from SV output.

`.bin` files contain actual complete peer-received final requests, checked
byte-for-byte against those independent expectations. `.bin.reply` where present
records actual sent reply bytes (empty or partial when applicable). Reply-failure
scenarios retain request captures and owner logs; their sent failure sequences
are explicit peer literals in `tests/sv/birth-authoritative-reply.c`, not separate
binary reply captures. Runner logs show every executed scenario. Guide PNGs are
production renderer frames captured before real SDL_RenderPresent; SDL
dummy/software is the backend. No OS-window screenshot is claimed.

The actual success bytes are `02 03 7a` (PKT_SUCCESS = 122), including Guide.
Slice 06's historical README says `02 03 01`; its own binary capture also contains
`02 03 7a`. The current Guide harness (`tests/sv/guide-motd.c`) also prints
`authoritative acknowledgement = 02 03 01` in its log text. That text is a
transcription error: its actual peer reply literal uses PKT_SUCCESS = 122 and
its `.bin.reply` capture is `02 03 7a`. Use the fingerprinted binary capture
for exact authoritative bytes; neither prose string is an alternative status.
Raw transcripts and prior captures remain unchanged. Earlier slice READMEs describe the then-current `play-acknowledged`
endpoint phase; after 06, fresh complete flows reach `live-session-handoff`.
Their retained history is not rewritten or used as current runtime evidence.

## Reviewable production trace

1. Harness `main` calls public `sv_endpoint_run(SvEndpointOptions)`. Examples:
   `tests/sv/birth-complete.c` and `tests/sv/guide-motd.c`. The socket peer observes
   actual traffic while native SDL events reach the running production loop.
2. `src/client/sv/endpoint-run.c` dispatches gestures to
   `sv_native_birth_event` in `input/native-birth.c`, which invokes
   `sv_birth_command` in `session/birth.c`. Guide uses the actual native caller
   and placeholder rendering, not a test callback implementing its obligation.
3. The endpoint requests `sv_birth_committed_play` only for the current
   generation and selected creation state. Dedicated adjustments belong to that
   committed draft; `sv_login_start_play` in `protocol/login.c` serializes it with
   production defaults/font identity, then changes state to WAIT_PLAY.
4. `pump_login` sends queued bytes to the controlled TCP peer. Peer assertions
   prove no early or second final request and compare the entire captured packet.
5. `sv_login_receive` owns incremental parsing and authoritative PKT_REPLY /
   PKT_PLAY success. `sv_login_interaction_sync`, `sv_pregame_sync_login` and the
   endpoint lifecycle route pending, failure/disconnect, MOTD and handoff. Only
   the acknowledged generation can reach the next startup surface. Guide logs
   and frames establish ordered actual presentation through the startup boundary.

## Criterion ledger (author evidence, controller verdict pending)

| Criterion | Evidence / disposition |
| --- | --- |
| P1-ac1 | Ordinary/dedicated/submission: complete committed handshake once, wait for authoritative reply |
| P1-ac2 | Ordinary/versions: independent full-packet baseline literals verify defaults and identity |
| P1-ac3 | Versions/dedicated: exact bytes, version gates, stats, options, geometry, graphics/font resources; controller/fresh audit pending |
| P1-ac4 | Submission plus all peers' no-final-request assertions: legal steps, double confirm, resize/focus, interleaving |
| P1-ac5 | Replies: correct failed/disconnected/pending/handoff owners; stale native gestures and retired connection isolation |
| P1-ac6 | Guide-MOTD: actual production callers and renderer frames; controller/fresh audit pending |
| P1-ac7 | Guide-MOTD ack/skip/point: peer MOTD to live-session handoff, gameplay inactive; pending replies open neither |
| P1-ac8 | Full peer-received requests, authoritative reply captures, public owner logs; controller/fresh audit pending |
| P1-ac9 | Historical filename and SV-B-023 dependency preserved; DNA load/save remains stage C |
| P1-ac10 | Resource handshake bytes tested here; profile/resource workflows, real servers, session.enter-game and first gameplay surface remain C |
| P1-ac11 | Success ends at startup live-session handoff; Guide captures display gameplay inactive |
| P1-ac12 | **Deferred**, check P1-entry-evidence, owner SV-B-025; not executed and not passing evidence |
| P2-1 | This slice changes no legacy/shared production files; controller diff review pending |
| P2-2 | Existing SV-local implementation exercised; no shared extraction added; controller review pending |
| P2-3 | Focused runtime results in manifest; full sv-build/sv-core not run per assignment, pending orchestrator; baseline verdict pending controller |
| P2-4 | Public native endpoint → production birth/serializer/reply owner → real TCP peer; controller/fresh audit pending |
| P2-5 | No separate new improvement found in this evidence slice; existing docs/sv-improvements.md entries preserved |
| P2-6 | No new improvement entry needed; no out-of-scope improvement implemented |
| P2-7 | Intentional SV isolation/duplication retained; no consolidation defect invented |
| P2-8 | Child/evidence stay in .scratch; originating docs/tasks parent unchanged |
| P2-9 | Status/Assignee separate from Labels; no new triage performed |
| P2-10 | Read root CONTEXT, domain/tracker/labels docs, architecture and ADR-0001–0006 before work |

## Initial pending acceptance blockers (historical)

The supplied assignment reserves full gates to the orchestrator. Actual build
and core results must be attached there, using the immutable commands:

```sh
make -s -C src -f makefile.sv tomenet-sv
bash -s -- core < .sandcastle/checks.sh
```

The configured core runner exists; it has not been replaced by this collector.
Neither gate has an author result here. Controller-owned review and fresh audit
at the exact integrated final HEAD/digest are also absent. The child must remain
unresolved while these required verdicts are pending. SV-B-023 and SV-B-025 keep
their own acceptance/deferred evidence; no downstream entry workflow is launched.

Recovery completed the actual build/core gates and independent pre-resolution
review at worker HEAD. [RECOVERY.md](RECOVERY.md) records those results and the
required external fresh audit at the exact resolution commit HEAD. The initial
collector manifest's pending gate entries describe only what that collector
executed; they are not a replacement for the separate controller verdicts.

# 13: Seal the complete controlled-peer producer inputs

Type: verification
Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-8-1
Labels: bug, ready-for-agent

**What to build:** A reviewer can reproduce the established birth completion capture and prove that every consumed repository input and runtime dependency stayed unchanged. Repair the existing collector's source inventory and provenance records before final controller capture; expanding only the plan declaration does not deliver this result.

**Blocked by:** [08: Focused production coverage](./08-focused-test-coverage.md) (completed). This worker can start immediately. Transcript and tracking repairs are independent workers; final capture follows assembly of all deliveries.

**Finding IDs:** None (`findingIds: []`). This is the explicit prerequisite for the rejected binding's incomplete producer coverage, not a new ledger finding or a second repair of any ledger defect. Tickets 10, 11 and 12 exclusively own their three named findings. Preserve 01–08 and 09's delivered work with acceptance pending.

**Origin:** SV-B-024 / capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Delivery:** owner=worker; phase=parallel-work; prerequisites=[]. Deliver the collector repair on a worker branch for assembly. `birth-runtime.producedBy` names this ticket; the controller gate cannot run until this delivery is assembled.

**Production seam:** Existing public `sv_endpoint_run(SvEndpointOptions)` with native SDL input and controlled TCP peer, exercising committed birth draft, production `sv_login_start_play`, `sv_login_receive`, actual Guide caller and startup owner. This evidence-only repair connects P1-ac2/P1-ac3/P1-ac8/P2-3/P2-4 to `birth-runtime`; it changes no game behavior or serializer interface. If that seam becomes unavailable, record a blocker; do not introduce a test-only substitute.

## Worker delivery criteria

- [x] **P1-ac2, P1-ac3, P1-ac8, P2-3, P2-4:** Repair the existing collector's own inventory, rather than just its execution-plan declaration. Use the individually enumerated committed `birth-runtime.inputPaths` as the minimum sealed inventory. The planning inventory retains the prior committed input set and adds the compiler-confirmed common packet/MD5/defines/colours header closure, both primary and fallback fonts, and baseline-oracle source inputs. Independently check all six runners, the version runner's delegated ordinary runner, included native-input/controlled-peer harnesses, all consumed fixtures, every compiled source and transitive repository header for both choice orders. Resolve compiler dependency output using the actual runner flags; fail closed on a consumed repository file absent from the inventory. Reconcile any newly discovered inputs in both collector and binding before capture. List files individually, never directories. A repeat hash of the old 142-file inventory is insufficient.
- [x] **P1-ac2, P1-ac3, P1-ac8:** Include primary Cascadia Mono and fallback PCF resource bytes in before/after fingerprints, recording the effective loaded resource and basename used in the handshake. Record generated temporary profile configuration and version fixture/matrix content or fingerprints, with their producer inputs and commands. Preserve independently derived oracle provenance: legacy option defaults plus shipped preferences, baseline protocol/version gates, dedicated outcomes and complete font redefinition semantics; expected packets must never derive from SV output. Record canonical source revision/anchors and current fingerprints, distinguishing historical baseline provenance from current runtime inputs.
- [x] **P1-ac8, P2-3, P2-4:** Record tool/runtime identity before and after: Python executable/version, compiler executable/version/target and resolved flags, linker, Git, pkg-config executable and resolved cflags/libs/package versions for SDL3, SDL3_ttf, FreeType and libsecret. Record resolved loaded libraries and sanitizer runtime identity, platform/backend and relevant environment. Fingerprint consumed external dependency files or record verifiable package/build identities, including compiler system headers and linked libraries. Compare these inputs across collection; a changed/missing identity must prevent a passing unchanged-inputs claim. Keep repository file fingerprints separate from external tool/runtime records.
- [x] **P1-ac1–P1-ac8, P2-3, P2-4:** Keep the existing collector command usable with an appended fresh empty output directory. Its manifest records `tested_head`, exact `contract_digest`, complete `sources`, `artifacts`, truthful `inputs_unchanged` and suite returncode/status plus caseCount or one declared `.log` per case. Preserve unfiltered ordinary 3, versions 40, dedicated 20, submission 18, replies 17, Guide/MOTD 10 (108 total). No-send cases still emit case logs; do not require a request/reply artifact where no send is expected. Unavailable or failed gates stay pending/failed, never passed by collector success.
- [x] **P1-ac8, P2-4:** Focused checks demonstrate that omitted common headers/fonts are now sealed, declared paths match collector sources, and a controlled change to an inventory file or runtime identity is detected rather than reported unchanged. These are collector integrity checks through the real producer, with local evidence and rollback of temporary mutations. They must not implement birth behavior separately. Deliver the runner, inventory and discovery procedure without waiting for controller gates or asserting independent final acceptance.
- [x] **P1-ac1–P1-ac11, P2-1, P2-2, P2-7, P2-8:** Preserve production behavior and historical captures/receipts at their original revisions. Do not alter completed ticket history, 09's work, originating parent, SV-B-023 ownership, stage-C boundaries or recovery limits. Successful scope ends at startup live-session handoff. This prerequisite seals evidence inputs; it does not recreate the resolved/residual evidence repair.
- [x] **P2-5, P2-6, P2-9, P2-10:** Use glossary/accepted ADR vocabulary and separate Labels/Status. Record discovered legacy/shared/SV organization improvements separately with problem, affected code, proposal, status and checks, without expanding this repair; intentional isolation duplication is permitted.

## Controller acceptance obligations


Worker completion depends only on the worker delivery criteria above. The controller performs the following after assembly; these are pending obligations, not worker prerequisites or claimed passes.

- [ ] **P1-ac1–P1-ac8, P2-3, P2-4:** At final integrated HEAD and the exact contract digest, run the immutable SV build (`make -s -C src -f makefile.sv tomenet-sv`) and configured core gate (`bash -s -- core`, supplying the configured Sandcastle script on stdin). Collect fresh unfiltered runtime evidence via the manifest's declared collector into a new empty directory outside the Git tree. Verify ordinary 3, versions 40, dedicated 20, submission 18, replies 17 and Guide/MOTD 10 cases (108 total), including no-send cases. Reconcile acknowledgement transcript bytes with raw replies; inspect exact-byte/version/dedicated behavior and the actual Guide production seam. Rehash sources/artifacts independently and require unchanged inputs and exact tested HEAD/digest. Worker checks and historical receipts do not satisfy these gates.
- [ ] **P1-ac1–P1-ac11, P2-1–P2-10:** Independent controller review and fresh final audit establish behavior baseline preservation, production-path evidence and tracking correctness, including the named finding's repair. Retain actual build/core/runtime/review results and an acceptance receipt at that same HEAD/digest in a recoverable external evidence location. Any later commit requires fresh final-HEAD evidence; author self-review is not independent acceptance.
- [ ] **P1-ac9–P1-ac11, P2-1, P2-2, P2-7:** Preserve SV-B-023 dependency and the historical originating filename. Keep changes isolated to SV/tests/tracking as needed. Handshake resource fields remain mandatory; DNA load/save, profile/resource workflows, real-server integration, session.enter-game and the first gameplay surface remain stage C. Successful acceptance stops at startup screen flow/live-session handoff without gameplay presentation. Do not modify or close the parent.
- [ ] **P1-ac12:** Record P1-entry-evidence as deferred to SV-B-025 with its original reason; neither a passed check nor a blocker here. All current build/core/runtime/independent-review obligations remain mandatory.

- [ ] **P1-ac2, P1-ac3, P1-ac8, P2-3, P2-4:** Require this delivered collector repair to be assembled before `birth-runtime`. Independently approve the corrected binding's complete source-supported coverage before restoring plan-validate progress. Then independently audit dependency closure, baseline-oracle provenance, generated inputs, before/after resource fingerprints and tool/runtime identity on the exact final integrated HEAD. Retain durable external receipts; historical evidence remains historical. Do not increase or reset quota, cycle or recovery limits.

## Answer

Delivered the collector repair on `codex/sandcastle-0241e0da-wave-8-1`.
Worker implementation is resolved; every controller acceptance obligation above
remains pending, including binding approval and final integrated-HEAD capture.
No ledger finding is created or closed by this prerequisite.

The public production seam remains `sv_endpoint_run(SvEndpointOptions)` with
native SDL input and the controlled TCP peer. The agreed integrity seam is the
existing `tests/sv_birth_runtime_evidence.py` collector command and its manifest.
The observer forwards production `sv_font_open`/`sv_font_resource` unchanged and
records actual resource identity and ELF libraries. No game behavior, serializer,
legacy/shared source, historical capture, completed child history or parent was
changed. No separate birth behavior implementation exists in these checks.

The collector now seals all 162 individually declared `birth-runtime.inputPaths`
(the prior set is retained), including the compiler-confirmed common header
closure, both Cascadia Mono and fallback PCF, baseline-oracle sources, binding,
canonical provenance manifest and new collector dependencies. All six real
runners are discovered; version discovery executes its delegated ordinary
runner. Twelve actual-flag compiler dependency records cover both choice orders;
each actual compilation also rechecks its repository/system header closure.
Undeclared consumed repository inputs or external dependencies fail closed.
Both fonts have before/after source fingerprints. Production observations
identify the loaded font and, on send, the basename used in the handshake.

Generated profile configuration, all 40 version fixtures and their matrix are
retained with producer inputs/commands, consumer commands and fingerprints.
Historical canonical Git revision/anchor/blob fingerprints are distinguished
from current legacy source fingerprints. Option defaults plus shipped
preferences, protocol/version gates, dedicated outcomes and complete font
redefinition semantics remain independent oracles; expected bytes never derive
from SV output.

Separate before/after runtime records seal Python/stdlib, compiler executable,
version/target/search paths and resolved compilation flags, linker, Git,
pkg-config identities and SDL3/SDL3_ttf/FreeType/libsecret flags/package versions,
system headers, sanitizer resources, linked/loaded libraries, linker support
files, package build identities, symlink resolution, platform/backend and relevant
environment. The focused delivery capture recorded 2,734 external fingerprints;
missing or changed identities cannot produce a passing unchanged-input claim.
The default command remains unfiltered 3/40/20/18/17/10 (108); it requires case
logs/counts and does not require sends for no-send cases. Build/core/review stay
pending, and P1-entry-evidence retains its original deferral reason and SV-B-025
ownership.

TDD for the new collector integrity interface (no game behavior change):

- Red: `python3 -B tests/sv_birth_inventory_checks.py` against the original
  collector returned 1: its public command rejected `--integrity-only`, so it
  could not deliver the required focused producer integrity result.
- Green: `python3 -B tests/sv_birth_inventory_checks.py --output
  /home/agent/sv-b024-input-seal-13/delivery` returned 0. It discovered the whole
  closure, captured the ordinary three production cases, checked exact
  declared/source equality and formerly omitted headers/fonts, rejected an
  omitted `pack.h` before capture, detected a temporary `pack.h` byte change and
  a controlled retained compiler-identity change, and passed again after each
  rollback. Temporary shared-file/binding mutations were restored byte for byte.
- Additional focused production observer checks passed: no-send `cancel-sex`
  (one case log, no request), version `4.8.1.2.0.0` (one), `iddc-skipped` waiting
  and acknowledgement in both choice orders (four), and Guide/MOTD acknowledgement
  through the actual caller with race-first order (one). These are coverage and
  observer checks of existing behavior, not a claimed game-behavior TDD loop.
- Author readback independently rehashed the focused capture's 68 artifacts;
  before/after runtime records matched, sources were unchanged, and all
  originating gate statuses remained pending/deferred. `git diff --check` passed.
  This is worker verification, not independent controller acceptance.

Reproduction and discovery procedure:
[producer input seal](../evidence/13/README.md). Retained local logs and captures:
`/home/agent/sv-b024-input-seal-13/delivery.log`, `delivery/capture/manifest.json`,
`delivery/omitted-header.log`, `observer-smoke.log` and `smoke/observations/`.
Worker captures precede the delivery commit and must not be relabelled as final
integrated-HEAD evidence. No whole build/core/review workflow or final 108-case
controller gate was run. No additional legacy/shared/SV organization improvement
was discovered outside this collector repair; intentional isolation remains
permitted. Successful scope still ends at startup live-session handoff; stage C,
SV-B-023, SV-B-025, ownership and recovery limits are unchanged.

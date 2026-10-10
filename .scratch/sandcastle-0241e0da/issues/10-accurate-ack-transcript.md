# 10: Accurate controlled-peer acknowledgement transcript

Type: verification
Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-7-1
Labels: bug, ready-for-agent

**What to build:** The reviewer can trust the controlled-peer acknowledgement transcript: successful replies print the bytes actually sent, and fresh captures agree with their binary replies. Repair only the false runtime statement; preserve historical captures and all existing birth completion behavior.

**Blocked by:** [06](./06-guide-motd-handoff.md) (completed); [08](./08-focused-test-coverage.md) (completed)

**Finding IDs:** D-86f98a5241b62943

**Origin:** SV-B-024 / capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`. This ticket owns only the named open finding. It does not recreate resolved/residual findings or the earlier final-HEAD evidence repair.

**Delivery:** owner=worker; phase=parallel-work; prerequisites=[] (no worker-local setup prerequisite beyond the available completed dependencies).

**Production seam / focused TDD:** Existing public `sv_endpoint_run(SvEndpointOptions)` with native SDL input and a controlled TCP peer, through committed birth draft → production `sv_login_start_play` → socket → `sv_login_receive` and the production startup owner. The actual Guide caller is required by P1-ac6; transcript/binary agreement supports P1-ac8 and P2-4. This is an evidence repair, not a new gameplay behavior. Exercise the existing seam for the regression; do not introduce a test-only caller or serializer. If it becomes unavailable, record a worker blocker rather than inventing a substitute.

The full collector already exists and emits one `.log` per case, including no-send cases, with source/artifact fingerprints and HEAD/digest metadata. Its input inventory needs the independent delivery in ticket 13 before controller capture; this does not block the focused transcript repair. No absent collector is assumed. The focused regression belongs to this same coherent repair.

## Worker delivery criteria

- [x] **P1-ac8:** Derive the printed acknowledgement bytes from the actual peer reply array, rather than a separately maintained success literal. A complete success currently contains `02 03 7a`; partial/waiting cases must not claim a complete acknowledgement.
- [x] **P1-ac6, P1-ac8, P2-4:** Add a focused regression check comparing each successful Guide/MOTD case's transcript with its captured reply bytes, including both choice orders and ordinary, skip and point-buy completion. Demonstrate failure on the existing false transcript and success after the repair through the accepted endpoint seam; retain partial/waiting coverage.
- [x] **P1-ac1–P1-ac7, P2-1–P2-4, P2-7:** Preserve exact field order, strict/inclusive version gates, defaults, font/resource identities and redefinition tables; dedicated IDDC variants, skipped-trait normalization and PvP Maia in both choice orders; legal-step/single-send guards; incremental reply ownership, cancellation and generation isolation; actual Guide caller with preserved drafts and ack-gated peer MOTD/startup handoff. Do not change production behavior to match a transcript.
- [x] **P1-ac8, P2-3:** Keep the existing full runtime collector callable with a fresh output-directory argument. Worker focused captures prove the local repair and runner delivery only; preserve old captures under their original HEADs. Supply the controller the runner command and artifact discovery procedure, without relabelling historical evidence or requiring controller completion before worker delivery.

- [x] **P2-5, P2-6, P2-10:** Follow the root glossary and relevant accepted ADRs. Record any discovered legacy/shared/SV organization opportunities separately in the improvements register with problem, affected code, proposal, status and checks; do not expand this repair. Intentional duplication for SV isolation is not a defect.

## Controller acceptance obligations

Worker completion depends only on the worker delivery criteria above. The controller performs the following after assembly; these are pending obligations, not worker prerequisites or claimed passes.

- [ ] **P1-ac1–P1-ac8, P2-3, P2-4:** At final integrated HEAD and the exact contract digest, run the immutable SV build (`make -s -C src -f makefile.sv tomenet-sv`) and configured core gate (`bash -s -- core`, supplying the configured Sandcastle script on stdin). Collect fresh unfiltered runtime evidence via the manifest's declared collector into a new empty directory outside the Git tree. Verify ordinary 3, versions 40, dedicated 20, submission 18, replies 17 and Guide/MOTD 10 cases (108 total), including no-send cases. Reconcile acknowledgement transcript bytes with raw replies; inspect exact-byte/version/dedicated behavior and the actual Guide production seam. Rehash sources/artifacts independently and require unchanged inputs and exact tested HEAD/digest. Worker checks and historical receipts do not satisfy these gates.
- [ ] **P1-ac1–P1-ac11, P2-1–P2-10:** Independent controller review and fresh final audit establish behavior baseline preservation, production-path evidence and tracking correctness, including the named finding's repair. Retain actual build/core/runtime/review results and an acceptance receipt at that same HEAD/digest in a recoverable external evidence location. Any later commit requires fresh final-HEAD evidence; author self-review is not independent acceptance.
- [ ] **P1-ac9–P1-ac11, P2-1, P2-2, P2-7:** Preserve SV-B-023 dependency and the historical originating filename. Keep changes isolated to SV/tests/tracking as needed. Handshake resource fields remain mandatory; DNA load/save, profile/resource workflows, real-server integration, session.enter-game and the first gameplay surface remain stage C. Successful acceptance stops at startup screen flow/live-session handoff without gameplay presentation. Do not modify or close the parent.
- [ ] **P1-ac12:** Record P1-entry-evidence as deferred to SV-B-025 with its original reason; neither a passed check nor a blocker here. All current build/core/runtime/independent-review obligations remain mandatory.

## Comments

- 2026-10-10: Claimed on `codex/sandcastle-0241e0da-wave-7-1`. Agreed seam: `sv_endpoint_run(SvEndpointOptions)` with native SDL input and controlled TCP peer, exercising actual Guide callers, production play submission/reply processing and startup ownership. Focused transcript-versus-reply regression only; controller gates remain pending.

## Answer

Implemented D-86f98a5241b62943 on `codex/sandcastle-0241e0da-wave-7-1`.
The controlled peer now prints each byte from the same `reply` array sent to
its socket, after successful completion of the final fragment. It prints
`02 03 7a` for current successful replies. Waiting/partial cases do not print
an authoritative acknowledgement. No production client, legacy or shared
code, wire reply, gameplay behavior or historical capture was changed.

**Agreed public production seam:** `sv_endpoint_run(SvEndpointOptions)` with
native SDL gestures and a controlled TCP peer. Existing cases run actual Guide
callers with preserved committed/draft choices, production
`sv_login_start_play`, socket transport, `sv_login_receive`, and startup owner.
The regression compares the public peer transcript with captured `.bin.reply`
bytes, independently requiring the contract's `02 03 7a` success bytes. It
requires exactly one acknowledgement line per success and none for partial or
waiting replies (whose captures must be `02 03` or empty). Existing handshake,
no-early-send, single-send, Guide draft and ack-gated MOTD/handoff assertions
remain in place, including ordinary, skip and point-buy in both choice orders.

**TDD red:**
`SV_GUIDE_ORDERS=1 SV_GUIDE_CASES=ack SV_GUIDE_EVIDENCE=/tmp/sv-ticket10-red python3 tests/sv_guide_motd_checks.py`
exited 1 before the repair: `1-ack` transcript `02 03 01` disagreed with the
captured reply `02 03 7a`. Native endpoint completed the flow; the new evidence
assertion exposed the false statement.

**TDD green:**
`SV_GUIDE_EVIDENCE=/tmp/sv-ticket10-green python3 tests/sv_guide_motd_checks.py`
exited 0 after the minimal repair, all 10 cases under AddressSanitizer and
UndefinedBehaviorSanitizer: six successful cases (`1/0-ack`, `1/0-skip`,
`1/0-point`) and four waiting/partial cases. Local worker captures reside in
those new external directories. Each case has `<order>-<case>.log`,
`<order>-<case>.bin` (handshake), `<order>-<case>.bin.reply` (reply), and
`<order>-<case>.bin-*.png` (native SDL dummy/software frames). These are focused
working-tree checks based on HEAD `0cc25d5ef813b8f078c4aa72eab38aa3719da059`
plus this repair, not final integrated HEAD acceptance receipts.

**Controller runner delivery:**
`python3 tests/sv_birth_runtime_evidence.py /absolute/path/to/new-empty-directory`
remains callable; `python3 tests/sv_birth_runtime_evidence.py --help` exited 0.
The collector was not modified or run as a full matrix by this worker. After
assembly including ticket 13's independent input-inventory delivery, run it
with a fresh empty directory outside Git. Discover source/artifact fingerprints,
HEAD/digest and suite commands through `<output>/manifest.json`; suite summaries
are `<output>/<suite>.runner.log`, and per-case logs, binary handshakes/replies
and Guide/MOTD frames are under `<output>/<suite>/`. All historical evidence
retains its original HEAD and meaning.

Read root AGENTS.md and CONTEXT.md, domain/tracker rules, SV architecture and
accepted ADRs 0001–0006. No additional legacy/shared/SV organization opportunity
was discovered; no improvements-register expansion is needed for this repair.
The full immutable build, configured core, unfiltered 108-case capture,
independent review and fresh final-HEAD audit remain controller-owned pending
obligations, not claimed passes. The parent and SV-B-023 dependency are unchanged.
Stage C retains DNA load/save, profile/resource workflows, real-server
integration, session.enter-game and gameplay presentation. P1-entry-evidence
remains deferred to SV-B-025 for its original reason: the assigned source places
end-to-end verification downstream, while this checkpoint covers SV-B-024 birth
completion; this is neither passing evidence nor a blocker here.

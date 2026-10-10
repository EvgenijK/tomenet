# 11: Restore Wayfinder child Type metadata

Type: implementation
Status: open
Assignee: unassigned
Labels: bug, ready-for-agent

**What to build:** Readers and agents can identify the kind of each completed Wayfinder child 01–06 from its required Type metadata without losing resolution state, ownership or history.

**Blocked by:** None (can start immediately).

**Finding IDs:** D-df28653d4c8d0985

**Origin:** SV-B-024 / capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`. This ticket owns only the named open finding. It does not recreate resolved/residual findings or the earlier final-HEAD evidence repair.

**Delivery:** owner=worker; phase=parallel-work; prerequisites=[] (no worker-local setup prerequisite beyond the available completed dependencies).

**Focused verification:** Compare the six affected children before/after: only the missing Type metadata is added, and existing links and history still resolve. This ticket changes tracking metadata, so it requires no behavior seam or runtime producer.

## Worker delivery criteria

- [ ] **P2-8:** Add an appropriate `Type: implementation` near the top of each affected implementation child 01–06 alongside its existing Status and Assignee. Inspect all six children for required metadata; preserve their resolved statuses, assignees, Answers, comments, acceptance history and locations.
- [ ] **P2-8, P2-9:** Keep Type, Status, Assignee and separate Labels fields distinct. Do not reopen completed children, modify their outcomes, or change the originating parent.
- [ ] **P1-ac9, P1-ac12, P2-8:** Preserve historical references and downstream ownership; this metadata repair does not recreate any implementation or unresolved evidence finding.

- [ ] **P2-5, P2-6, P2-10:** Follow the root glossary and relevant accepted ADRs. Record any discovered legacy/shared/SV organization opportunities separately in the improvements register with problem, affected code, proposal, status and checks; do not expand this repair. Intentional duplication for SV isolation is not a defect.

## Controller acceptance obligations

Worker completion depends only on the worker delivery criteria above. The controller performs the following after assembly; these are pending obligations, not worker prerequisites or claimed passes.

- [ ] **P1-ac1–P1-ac8, P2-3, P2-4:** At final integrated HEAD and the exact contract digest, run the immutable SV build (`make -s -C src -f makefile.sv tomenet-sv`) and configured core gate (`bash -s -- core`, supplying the configured Sandcastle script on stdin). Collect fresh unfiltered runtime evidence via the manifest's declared collector into a new empty directory outside the Git tree. Verify ordinary 3, versions 40, dedicated 20, submission 18, replies 17 and Guide/MOTD 10 cases (108 total), including no-send cases. Reconcile acknowledgement transcript bytes with raw replies; inspect exact-byte/version/dedicated behavior and the actual Guide production seam. Rehash sources/artifacts independently and require unchanged inputs and exact tested HEAD/digest. Worker checks and historical receipts do not satisfy these gates.
- [ ] **P1-ac1–P1-ac11, P2-1–P2-10:** Independent controller review and fresh final audit establish behavior baseline preservation, production-path evidence and tracking correctness, including the named finding's repair. Retain actual build/core/runtime/review results and an acceptance receipt at that same HEAD/digest in a recoverable external evidence location. Any later commit requires fresh final-HEAD evidence; author self-review is not independent acceptance.
- [ ] **P1-ac9–P1-ac11, P2-1, P2-2, P2-7:** Preserve SV-B-023 dependency and the historical originating filename. Keep changes isolated to SV/tests/tracking as needed. Handshake resource fields remain mandatory; DNA load/save, profile/resource workflows, real-server integration, session.enter-game and the first gameplay surface remain stage C. Successful acceptance stops at startup screen flow/live-session handoff without gameplay presentation. Do not modify or close the parent.
- [ ] **P1-ac12:** Record P1-entry-evidence as deferred to SV-B-025 with its original reason; neither a passed check nor a blocker here. All current build/core/runtime/independent-review obligations remain mandatory.

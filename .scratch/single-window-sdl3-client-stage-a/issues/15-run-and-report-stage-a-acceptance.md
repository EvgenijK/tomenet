# 15: Run and report the complete Stage A acceptance gate

**What to build:** A reviewer can run the complete Stage A gate and see current evidence for both the production native scenario and the full registry, together with reproducible commands and explicit remaining limitations.

**Blocked by:** первоначальные prerequisites 06/12/14; оставшуюся приёмку закрывают [16](16-restore-mingw-and-run-wine-acceptance.md), [17](17-instrument-native-fallback-runtime-checks.md), [18](18-complete-scoped-stage-a-evidence.md), [19](19-review-and-close-stage-a-acceptance.md).

**Status:** completed; scoped Stage A accepted, 62/62 automated checks and current human review approved (2026-09-23)

- [x] Run the integrated production decode→model→native surfaces→input router→response serializer scenario for HP, repeated messages and key requests, including fragmented/adjacent packets, cancellation/abort, representative macro order and short lifecycle/session-reset cases.
- [x] Verify separate Linux amd64/MinGW i686 outputs, relevant legacy build paths, isolated profiles, approved startup/resources, one system window, no accepted-flow fallback and explicit storage/overflow behavior.
- [x] Collect required geometry and actual submission-timing results, Linux accelerated/software runs and Wine smoke/software evidence with actual backend/configuration identity. Missing checks remain unverified; Wine does not establish actual Windows acceptance.
- [x] Validate the complete canonical registry, native ledger, A–F allocation and inventory reconciliation; link observed scenario results to exact outcomes and current evidence fingerprints using the evidence checker.
- [x] Verify local consumer integrity tooling with truthful missing coverage; explicitly state whether the sibling consumer was synchronized, without making HTML feature completeness a native gate.
- [x] Obtain and record human native review of readability, keyboard/cancel/focus behavior and perceived response against the tested build. A screenshot alone does not pass a behavior flow; absent review remains pending.
- [x] Publish reproducible build/scenario/check commands, expected outcomes, current evidence, fallback inventory, limitations and B–F pending scope. A crash, wrong reply, lost required event, corrupt state, mandatory timing violation or unclassified required registry row prevents the corresponding gate from passing.
- [x] Do not claim live login, complete gameplay, shipping archives or actual Windows acceptance from this synthetic checkpoint; preserve later Windows checkpoints and the approved exclusions from stress/soak, recording and global memory-ceiling requirements.


## Current result and follow-up ownership

Runner/report implemented in `8ce2d70ec`; [current report](../../../docs/sv-stage-a-acceptance.md).
33/35 commands passed, all 27 regression invocations passed. This does not close
Stage A: MinGW dependencies, complete runtime/dependency evidence and human review
remain blockers. Historical checkboxes above are not a claim that nothing ran.

- 16 owns reproducible MinGW/Wine setup and current platform observations.
- 17 owns production fallback runtime checks.
- 18 owns reviewed dependencies, checker-linked evidence and scoped checkpoint status.
- 19 owns human review and the final rerun/closure of this ticket.

16 and 17 can start independently; inventory preparation in 18 can run alongside
them. Final evidence and human approval must refer to the final tested build.

## Final acceptance — ticket 20, 2026-09-23

Production startup fix and final evidence: [report](../../../docs/sv-direct3d-startup.md).
Full gate passed 62/62, all eight scoped A outcomes covered, current explicit human
review approved. Wine Direct3D maximum urgent 1.057 ms / 20 ms; all delayed controls
still reject intentional violations. Stage A is accepted; B–F and full capabilities
remain pending. Earlier results above are historical and are not erased.

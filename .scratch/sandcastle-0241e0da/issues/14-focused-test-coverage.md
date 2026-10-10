# 14: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-024](../../../docs/tasks/stage-b/SV-B-024-birth-dna.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-9-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

## Worker delivery criteria

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Controller acceptance obligations

Required gates and independent final acceptance remain controller-owned and pending until verified after assembly.

## Comments

- 2026-10-10: Claimed in isolated branch `codex/sandcastle-0241e0da-wave-9-1`; inspect existing production coverage and run focused checks only. Controller assembly gates and deferred downstream evidence remain pending.

## Answer

Completed focused verification on isolated branch
`codex/sandcastle-0241e0da-wave-9-1`. History: open → claimed 2026-10-10 →
resolved after focused checks. No other branch was edited or merged.
Read AGENTS.md, CONTEXT.md, domain/tracker/labels instructions, SV architecture,
accepted ADR-0001–0006 and the tdd skill/references; no separate GLOSSARY.md exists.
Inspected canonical SV-B-024, children 01–06, prior coverage 08, pending evidence
09, acknowledgement repair 10 and input-seal delivery 13, with their runners
and actual production callers.

**Agreed production seam:** `sv_endpoint_run(SvEndpointOptions)` with native
SDL gestures and a controlled TCP peer, explicitly established by the accepted
contract and children 01–06. Legal birth publishes through
`sv_birth_committed_play`; the endpoint invokes production `sv_login_start_play`,
sends real socket bytes, then routes incremental `sv_login_receive` results
through login/pregame ownership. Guide gestures invoke the actual native caller;
rendering wrappers forward real SDL/TTF calls. Public endpoint outcomes and
peer bytes expose the behavior. No substitute serializer, lifecycle behavior,
Guide callback or absent caller was invented.

**Coverage inspection:** existing tests cover the accepted behavior, including
10's transcript-versus-raw-reply regression and 13's complete producer seal.
No missing focused test or behavior defect was found, so no new test or
production change was necessary. This verifies already implemented behavior;
no test-first red/green loop is claimed. Independent baseline byte fixtures
and explicit protocol boundary rows remain the expected-value oracles.

| Runner in `tests/` | Cases | Result and coverage |
| --- | ---: | --- |
| `sv_birth_complete_checks.py` | 3 | passed; complete current handshake and pending/partial/authoritative replies |
| `sv_birth_version_checks.py` | 40 | passed; strict/inclusive gates, exact identity/stats/defaults/geometry/graphics/font/resource fields and full tables |
| `sv_birth_dedicated_checks.py` | 20 | passed; IDDC variants, skipped-trait normalization and PvP Maia in both choice orders |
| `sv_birth_single_submission_checks.py` | 18 | passed; legal-step guards, double/held confirm, resize/focus/network interleaving, no-send cancellation, backtracking and retired-generation input |
| `sv_birth_reply_checks.py` | 17 | passed; rejection/disconnect/malformed/truncated/split replies, ordered trailing packets, stale input/socket and correct owner |
| `sv_guide_motd_checks.py` | 10 | passed; actual Guide callers, preserved drafts, ack gating, peer MOTD continue/skip, startup handoff and accurate acknowledgement transcripts in both choice orders |

**Executed focused command:**

```sh
python3 -B tests/sv_birth_runtime_evidence.py /home/agent/sv-b024-focused-wave-9-1/runtime
```

Exit 0, all **108 sessions**, compiled with `-Wall -Wextra -Werror` and
ASan/UBSan. The collector removes inherited case filters. Native backend is
SDL dummy/software, not an OS-window capture or packaged executable invocation.
Tested HEAD: `64c51a7b090d96315ede4d550485aab5bbad891d`; immutable digest:
`0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.
HEAD and sealed inputs remained unchanged throughout capture. Only the ticket
claim was uncommitted during the checks; production/test inputs were unchanged.

A separate author Python readback exited 0: recomputed all **603 artifact** and
**162 repository source** sizes/SHA-256 values from bytes, required the exact
3/40/20/18/17/10 case counts and successful suite exits, compared before/after
runtime records (all **2734 external fingerprints**), and checked all six
successful Guide transcript lines against raw `02 03 7a` replies. Waiting/partial
cases contained no authoritative acknowledgement line and retained empty/
`02 03` replies. This is author integrity verification, not independent
controller semantic review or acceptance. `git diff --check` passed.

[Committed focused results](../evidence/14/RESULTS.json) retain commands, suite
exit codes, every case trace/log hash, tested HEAD/digest, manifest hash and
pending/deferred ownership. External raw evidence is at the command's output
path: `manifest.json` includes input/artifact seals and provenance;
`<suite>.runner.log` records suite output; `<suite>/` contains per-case logs,
requests/replies and Guide/MOTD PNG frames; discovery/observations retain actual
compiler dependencies and generated-input/resource provenance. Author readback
is at `/home/agent/sv-b024-focused-wave-9-1/author-readback.json`. Reproduce with
the same collector command targeting a new empty directory outside Git.
These receipts describe the pre-resolution tested HEAD; they are not relabelled
as evidence for this delivery commit or a future assembled HEAD.

**Remaining ownership:** full `sv-build`, configured `sv-core`, independent
controller review and fresh final audit were not run and remain pending for
the orchestrator after assembly. Ticket 09's finding and history remain intact;
this focused worker does not close its controller-owned evidence obligations.
P1-ac12 / P1-entry-evidence remains **deferred, owner SV-B-025**, because the
assigned source places end-to-end verification downstream while this checkpoint
covers SV-B-024 birth completion; it is neither passed nor a blocker here.
SV-B-023 dependency/full acceptance remain with their owner. Canonical SV-B-024
and its historical filename are unchanged; this child owns no Stage B IDs.
DNA load/save, profile/resource workflows, real-server integration,
`session.enter-game` and gameplay presentation remain stage C. Successful scope
ends at startup screen flow/live-session handoff, without gameplay presentation.

Changes only resolve this ticket, index its Answer and retain focused receipts.
No production, legacy/shared source, existing test or earlier ticket history
was changed. No additional legacy/shared/SV organization improvement was found;
existing improvement entries and intentional isolation duplication are retained.

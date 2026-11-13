# 8: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-024](../../../docs/tasks/stage-b/SV-B-024-birth-dna.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-5-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-10: Claimed for focused production coverage. Full assembly gates and
  explicit deferred acceptance remain with the orchestrator/originating owners.

## Answer

Completed focused production coverage on the assigned isolated branch
`codex/sandcastle-0241e0da-wave-5-1`; no other branch was edited or merged.
History: open → claimed 2026-10-10 → resolved after focused verification.
Read AGENTS.md, CONTEXT.md, domain/tracker/labels instructions, SV architecture,
ADR-0001–0006 and the tdd skill/references. No separate GLOSSARY.md exists.
Inspected resolved children 01–07, canonical SV-B-024 and their existing native
runners, baseline byte fixtures and production callers.

**Accepted production seam:** `sv_endpoint_run(SvEndpointOptions)` with native
SDL input and a controlled TCP peer, as already agreed in children 01–06 and
the immutable contract. The native endpoint commits legal birth through
`sv_birth_committed_play`, calls production `sv_login_start_play`, sends actual
socket bytes, and routes decoded authoritative replies through login/pregame
owners. Native Guide gestures invoke the actual caller; SDL/TTF wrappers observe
and forward real rendering. Public endpoint outcomes supplement peer evidence.
No test-only serializer, alternate behavior implementation or absent caller was
introduced. Backend is native SDL dummy/software, not an OS-window capture or
packaged executable invocation.

**Coverage inspection:** existing checks already cover the accepted behavior;
no missing focused test or production defect was found, so no new test or
behavior change was necessary. This is coverage of already implemented behavior;
there is no claimed test-first/red-green loop. Existing independent baseline
fixtures remain the exact-byte oracle. Coverage and fresh results:

| Runner in `tests/` | Sessions | Result / relevant behavior |
| --- | ---: | --- |
| `sv_birth_complete_checks.py` | 3 | passed; complete current handshake, pending/partial/authoritative reply |
| `sv_birth_version_checks.py` | 40 | passed; below/at/above protocol gates, exact identity/stats/defaults/geometry/graphics/font/resources/tables |
| `sv_birth_dedicated_checks.py` | 20 | passed; IDDC variants, skipped trait, PvP Maia, pending/ack, both choice orders |
| `sv_birth_single_submission_checks.py` | 18 | passed; no early/duplicate request, held/double confirm, resize/focus, interleaving/keepalive, backtrack/recommit, quit/cancel each owner, retry generations |
| `sv_birth_reply_checks.py` | 17 | passed; rejection/disconnect/malformed/truncated/split replies, ordered trailing packets, stale input/old socket and correct endpoint owner |
| `sv_guide_motd_checks.py` | 10 | passed; actual Guide callers, preserved point-buy draft, delayed/partial ack, peer MOTD continue/skip and startup live handoff, both choice orders |

**Executed focused command:**

```sh
python3 tests/sv_birth_runtime_evidence.py /home/agent/sv-b024-focused-wave-5-1/runtime
```

Exit 0: all **108 sessions**, strict compiler warnings and ASan/UBSan enabled.
The collector removes inherited case filters, so these counts are the complete
focused birth matrix. Tested HEAD:
`23684316119dfd56deb9251f743faaa0a2b96e6f`.
Collector confirms HEAD and fingerprinted inputs were unchanged throughout.
An independent readback verified all **347 artifact sizes/SHA-256 values** and
all six expected scenario counts (passed). [Committed results](../evidence/08/RESULTS.json)
retain each scenario's endpoint trace, suite exit codes, log fingerprints,
manifest digest, backend, tested HEAD and gate ownership. Raw peer requests,
replies, transcripts and Guide/MOTD frames are at the command's output path;
rerun into a new empty directory to reproduce. These receipts identify the
pre-resolution tested HEAD, not a fresh final-HEAD controller acceptance.

The known Guide transcript prose says `02 03 01`; actual captured successful
reply is `02 03 7a`, as already documented by ticket 07. No raw evidence is
rewritten or incorrect prose used as a wire oracle.

**Remaining ownership:** full `sv-build`, configured `sv-core`, controller
review and fresh final audit were not run by this ticket and remain pending
for orchestrator assembly. Older ticket 07 results retain their own tested HEAD;
they do not establish acceptance at this branch's final HEAD. P1-ac12 /
P1-entry-evidence remains **deferred, owner SV-B-025**, not run or passed.
SV-B-023 dependency and its native/full acceptance remain with their owner.
Canonical SV-B-024 and its historical filename are unchanged; this child claims
no Stage B capability IDs. DNA load/save, profile/resource workflows,
real-server integration, session.enter-game and gameplay presentation remain
stage C. Success remains at the startup screen flow boundary.

Changes are limited to this ticket, the effort map and focused result records.
No production, legacy/shared source, test implementation or existing ticket
history was changed. No new legacy/shared/SV organization improvement was found;
existing improvement entries and intentional isolation duplication are retained.

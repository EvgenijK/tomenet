# 11: Restore Wayfinder child Type metadata

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-024](../../../docs/tasks/stage-b/SV-B-024-birth-dna.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-0241e0da-wave-7-2
Labels: bug, ready-for-agent

**What to build:** Readers and agents can identify the kind of each completed Wayfinder child 01–06 from its required Type metadata without losing resolution state, ownership or history.

**Blocked by:** None (can start immediately).

**Finding IDs:** D-df28653d4c8d0985

**Origin:** SV-B-024 / capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`. This ticket owns only the named open finding. It does not recreate resolved/residual findings or the earlier final-HEAD evidence repair.

**Delivery:** owner=worker; phase=parallel-work; prerequisites=[] (no worker-local setup prerequisite beyond the available completed dependencies).

**Focused verification:** Compare the six affected children before/after: only the missing Type metadata is added, and existing links and history still resolve. This ticket changes tracking metadata, so it requires no behavior seam or runtime producer.

## Worker delivery criteria

- [x] **P2-8:** Add an appropriate `Type: implementation` near the top of each affected implementation child 01–06 alongside its existing Status and Assignee. Inspect all six children for required metadata; preserve their resolved statuses, assignees, Answers, comments, acceptance history and locations.
- [x] **P2-8, P2-9:** Keep Type, Status, Assignee and separate Labels fields distinct. Do not reopen completed children, modify their outcomes, or change the originating parent.
- [x] **P1-ac9, P1-ac12, P2-8:** Preserve historical references and downstream ownership; this metadata repair does not recreate any implementation or unresolved evidence finding.

- [x] **P2-5, P2-6, P2-10:** Follow the root glossary and relevant accepted ADRs. Record any discovered legacy/shared/SV organization opportunities separately in the improvements register with problem, affected code, proposal, status and checks; do not expand this repair. Intentional duplication for SV isolation is not a defect.

## Controller acceptance obligations

Worker completion depends only on the worker delivery criteria above. The controller performs the following after assembly; these are pending obligations, not worker prerequisites or claimed passes.

- [ ] **P1-ac1–P1-ac8, P2-3, P2-4:** At final integrated HEAD and the exact contract digest, run the immutable SV build (`make -s -C src -f makefile.sv tomenet-sv`) and configured core gate (`bash -s -- core`, supplying the configured Sandcastle script on stdin). Collect fresh unfiltered runtime evidence via the manifest's declared collector into a new empty directory outside the Git tree. Verify ordinary 3, versions 40, dedicated 20, submission 18, replies 17 and Guide/MOTD 10 cases (108 total), including no-send cases. Reconcile acknowledgement transcript bytes with raw replies; inspect exact-byte/version/dedicated behavior and the actual Guide production seam. Rehash sources/artifacts independently and require unchanged inputs and exact tested HEAD/digest. Worker checks and historical receipts do not satisfy these gates.
- [ ] **P1-ac1–P1-ac11, P2-1–P2-10:** Independent controller review and fresh final audit establish behavior baseline preservation, production-path evidence and tracking correctness, including the named finding's repair. Retain actual build/core/runtime/review results and an acceptance receipt at that same HEAD/digest in a recoverable external evidence location. Any later commit requires fresh final-HEAD evidence; author self-review is not independent acceptance.
- [ ] **P1-ac9–P1-ac11, P2-1, P2-2, P2-7:** Preserve SV-B-023 dependency and the historical originating filename. Keep changes isolated to SV/tests/tracking as needed. Handshake resource fields remain mandatory; DNA load/save, profile/resource workflows, real-server integration, session.enter-game and the first gameplay surface remain stage C. Successful acceptance stops at startup screen flow/live-session handoff without gameplay presentation. Do not modify or close the parent.
- [ ] **P1-ac12:** Record P1-entry-evidence as deferred to SV-B-025 with its original reason; neither a passed check nor a blocker here. All current build/core/runtime/independent-review obligations remain mandatory.

## Comments

2026-10-10: Claimed on `codex/sandcastle-0241e0da-wave-7-2` for the six-child Type metadata repair only. Children 01–06 retain their completed outcomes and owners; controller acceptance and downstream evidence remain at their originating owners.

2026-10-10: Resolved after focused six-child metadata and link verification. The claim above records the prior open → claimed transition; controller obligations remain pending.

## Answer

Implemented on `codex/sandcastle-0241e0da-wave-7-2` for finding
`D-df28653d4c8d0985` only. Added `Type: implementation` immediately before the
existing Status and Assignee in children 01–06. Type, Status, Assignee and Labels
remain distinct. All six retain their resolved statuses, original assignees,
Answers, comments, acceptance history, historical references and locations.
The canonical parent was not modified or closed; no implementation or evidence
finding was recreated.

**Focused verification:** inline `python3` checks against the pre-repair `HEAD`
passed for all six files: each contains exactly one added Type line, and removing
that line reproduces the original bytes exactly. Checked one distinct field each
for Type, Status, Assignee and Labels, resolved status, and retained Answer and
Comments sections. All six existing local Markdown links resolve. The canonical
parent diff is empty. `git diff --check` passed.

Reproduce the byte-preservation check after this repair commit (with its parent
as the pre-repair baseline):

```sh
python3 - <<'PY'
from pathlib import Path
import subprocess
files = sorted(Path('.scratch/sandcastle-0241e0da/issues').glob('0[1-6]-*.md'))
assert len(files) == 6
for path in files:
    before = subprocess.check_output(['git', 'show', f'HEAD^:{path}'])
    after = path.read_bytes()
    addition = b'Type: implementation' + bytes([10])
    assert after.count(addition) == 1
    assert after.replace(addition, b'', 1) == before, path
print('PASS: all six children differ only by the required Type line')
PY
```

**Behavior/seam:** metadata-only repair; no runtime behavior, production seam,
runtime producer or tests changed. The supplied TDD skill applies to
behavior-changing code, so no red/green loop is claimed. Read root `AGENTS.md`,
`CONTEXT.md`, tracker/triage/domain guidance and accepted ADRs 0001 and 0005;
no legacy/shared/SV organization improvement was discovered in this repair.

**Remaining owners:** immutable contract
`0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`
is unchanged. SV-B-023 dependency and historical SV-B-024 filename are retained.
Stage C retains DNA persistence, profile/resource workflows, real-server
integration, `session.enter-game` and gameplay presentation. P1-entry-evidence
remains deferred to SV-B-025 because the assigned source places end-to-end
verification in that downstream ticket; this checkpoint covers SV-B-024 birth
completion. Deferred evidence is neither a pass nor a blocker here. Full
build/core/runtime gates, independent controller review and fresh final-HEAD
acceptance remain controller-owned and were not run or claimed passed.

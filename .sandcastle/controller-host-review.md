# Controller host prerequisite review

`controller-host-review` is a saved, read-only prerequisite phase before Docker
and before worker recovery. It never runs native jobs or marks acceptance passed.
A typed controller-observed `HOST_APPROVAL_REQUIRED` selects it; provider prose
cannot select commands, profiles, resources or privileges. Completed tickets,
wave records, recovery ledgers and allowances remain unchanged.

Initial deployment uses `SANDCASTLE_HOST_REVIEW_PROFILE` and
`SANDCASTLE_HOST_REVIEW_PROFILE_SHA256`. The regular mode600 JSON file must be
inside this run's private directory. Bootstrap a saved typed checkpoint with
only these two profile variables and `npm run sandbox:resume -- <run-id>`;
do not also set `SANDCASTLE_PREFLIGHT_RESUME` or an allowance/repair mode. The
plain resume queues this phase before Docker. A profile contains:

- `id: "sv-entry-collector-v3"`, `policyPath`, `policySha256`, `sealPath`,
  `sealSha256`, `bindingDigest`, `controllerDigest`.
- `defaultRequest: {action, driver, renderer}` for historical typed worker
  failures that lack a request. Only the private reviewed profile supplies this
  default; wave/ticket/branch/base and the current committed HEAD come from the
  saved checkpoint and Git. Fresh observed requests retain their exact HEAD.
- Optional `equivalenceAction`: an already reviewed action with exactly the
  same390 sealed inputs and independently delegated metadata/file versions.
- Optional `resourceProposal`: the original closed graph path/SHA, missing
  public header identities, failed-manifest path/SHA, original policy digest,
  source proof, head/digest/nonce/action and full `originRequest`. Optional
  `originPolicyPath` + `originPolicySha256` independently pin the original
  private capture policy when the activation profile has a different policy.
  Both policies must retain the same capture recipe/inventory/binding (only the
  reviewed seal-path slot can differ). Without these fields the original
  policy defaults to the current profile policy. Origin scope
  and nonce retain the original capture; metadata HEAD renewal never relabels it.

Compute the controller digest after installing the final controller commit.
A profile cannot authorize new producer code, argv/env, arbitrary resources or
an unlisted action. Unknown or changed source profiles, frozen targets, quota
failure and rejected fingerprints remain awaiting with `HOST_REVIEW_REFUSED`.
A frozen target requires its existing explicit restoration procedure; this
phase cannot silently replace its runtime/policy identity.

The host reviewer runs with the existing quota and read-only agent runtime.
Its structured response binds source tree, contract, binding, controller,
profile, input and resource digests. A separate durable execution/result record
attests the actual dispatch, schema and response bytes. Writer execution IDs
cannot approve. The adapter verifies the held controller run lock and rechecks
all identities before installation.

Successful versions live under `controller-host-review/<fingerprint>/` with
archived prior policy, seal, checkpoint, proposal, independent review and file
hash index. Interrupted installation can reuse only these verified original
bytes. Review rejection/execution failure cannot retry the same fingerprint.
The active private profile pin is saved in `state.hostReviewProfile`, and the
installed policy in `state.activeHostPolicy`. On later resume use this saved
profile, without overriding it via bootstrap environment variables. The
controller returns to `hostReviewPending.originalPhase`; only the subsequent
normal assembled controller gate runs the complete acceptance matrix.

A newly observed public `/usr/include/*.h` addition is reviewed from its
original private failed capture before artifact enumeration. The adapter
re-extracts the closed graph/source/seal identities, rather than trusting error
prose or exporting oversized failed artifacts. Prior captures and failed
captures retain their original identities and verdicts. No quota, 300s job
limit,24MiB transport limit or8jobs/channel limit is increased.

## Controller schema repair profile upgrade

For a saved fatal schema failure at `parallel-work`, after independent review,
set `SANDCASTLE_PREFLIGHT_RESUME=1` together with
`SANDCASTLE_HOST_PROFILE_UPGRADE_PATH` and
`SANDCASTLE_HOST_PROFILE_UPGRADE_SHA256`, then use the normal resume command.
Do not set the original HOST_REVIEW_PROFILE bootstrap variables again.
The new private profile is a clone of the saved profile except its current
`controllerDigest` and new private `policyPath`/`policySha256`. The policy is a
clone of the active policy except exact committed nonconsumed controller/test
file versions in the existing inspect-only equivalence action's
`carryForward.reviewedFiles`: preserve each old array as an exact prefix and
append at most one current `{mode,sha256}` version per changed path. Controller
Markdown documentation is permitted; test paths must be `tests/sandcastle*`. Capture actions, approvals,390source seal,
resources, timeout/job limits, runtime pins and readonly delivery remain exact.
The controller validates both profile pins and archives the checkpoint under
its held lock before changing the active profile through its normal save path.
Old policies, profiles and receipts remain immutable. A new profile cannot
repair native code or grant execution privileges through this upgrade.

### Reconcile the historical ticket08 request

Run `3fd6b311`, wave8, can restore the specific controller-generated unknown
`entry-runtime-discovery` request with `SANDCASTLE_HOST_REQUEST_RECONCILE=1`
and `SANDCASTLE_HOST_REVIEW_PROFILE` plus
`SANDCASTLE_HOST_REVIEW_PROFILE_SHA256`. Use this mode alone; do not combine
it with `SANDCASTLE_PREFLIGHT_RESUME`, quota grants or worker recovery.
The controller independently reviews the new private profile before invocation.

The new profile preserves its immutable seal (390 source identities,
1051 resources), binding, default request, capture recipes, runtime pins,
timeouts, maxJobs and historical readonly deliveries. Its controller digest
must equal the loaded committed controller. Changed nonconsumed controller
files can append one independently reviewed committed version to each existing
`carryForward.reviewedFiles` array. For this incident only, two exact version
pins may be added under the inspect-only equivalence action:

- `.scratch/sandcastle-3fd6b311/evidence/08-collector-readback/README.md`
- `.scratch/sandcastle-3fd6b311/evidence/08-collector-readback/verification.json`

These pins contain `{mode:"100644",sha256:...}` from the unchanged pending
worker HEAD, not mutable metadata-root permissions. The new profile also has
`requestReconciliationReview:{reviewer,reference,head,oldRequestDigest,evidenceFiles}`.
`evidenceFiles` maps exactly those two paths to the independently reviewed
mode/hash identities; `oldRequestDigest` uses the verification module's digest
of the original normalized saved request. The old profile/policy/seal bytes
remain immutable. The independently supplied raw profile SHA binds this review
record; a provider response cannot install it.

Prepare performs the actual controller host-review proposal validation with
that profile and current branch, without launching native jobs or inference.
Apply repeats validation under the held run lock, compares the saved checkpoint,
fsyncs its private archive, preserves all counters/limits and records the old
request. It observes a fresh request selected explicitly from the reviewed
`capture-runtime-baseline` default and re-enters controller-host-review. This
is not an alias for unknown actions. The usual independent reviewer must still
approve the new proposal before worker dispatch. The worker sees a command-free
readonly `/home/agent/.sv-host-approval-status.json` with current approval status;
old header-gap reports and original producer receipts retain their historical
meaning. Acceptance still requires fresh captures after assembly.

### Explicit wave8 recovery allowance

The user-authorized three additional recovery decisions for run `3fd6b311`
apply only to `parallel-work:8:8:0:1:0`. Invoke resume with
`SANDCASTLE_EXTRA_RECOVERY_ATTEMPTS=3`,
`SANDCASTLE_RECOVERY_CHECKPOINT=parallel-work:8:8:0:1:0`, a stable
`SANDCASTLE_RECOVERY_AUTHORIZATION_ID` and
`SANDCASTLE_RECOVERY_AUTHORIZATION_EXPLANATION`. This is one continuation mode;
combine it with neither quota grants nor preflight/user-resume modes.

For changed controller code, additionally supply the independently reviewed
controller-only profile via `SANDCASTLE_RECOVERY_PROFILE_PATH` and
`SANDCASTLE_RECOVERY_PROFILE_SHA256`. The profile refresh preserves the old
390-source/1051-resource seal, capture recipes, runtime/tool pins, historical
deliveries, scopes and job limits. It may only update the loaded controller
identity and append exact reviewed nonconsumed controller file versions.

The controller validates both operations before mutation, rechecks raw saved
state and held lock, fsyncs a private checkpoint archive and applies both to one
clone with one final workflow save. `recoveryAttempts` stays at three;
`recoveryAttemptLimits` becomes six only for this checkpoint. Durable
`recoveryAllowanceHistory` records the authorization and original counter.
Both fields are protected during independent host review. Other checkpoints
retain the original three-decision bound. Changing source code or its seal is
not authorized by this allowance: it must still enter independent host review,
and may remain paused while a new fixed source profile is unavailable.

### Exact one-test source revision for worker08

The saved changed-producer `HOST_REVIEW_REFUSED` checkpoint in run `3fd6b311`,
wave8, can bootstrap an independently pinned delegation with
`SANDCASTLE_PREFLIGHT_RESUME=1`,
`SANDCASTLE_HOST_SOURCE_REVISION_PROFILE_PATH` and
`SANDCASTLE_HOST_SOURCE_REVISION_PROFILE_SHA256`. Do not combine another
continuation mode. The normal controller owns the run lock, archives the raw
checkpoint, rechecks the committed controller/profile/policy and saves once.
This bootstrap changes no source approval, recovery count, allowance or quota;
the saved controller-host-review phase must still obtain a verified durable
independent reviewer envelope before installing a new source approval.

The private profile adds `sourceRevisionDelegation` with exactly these keys:
`version,runId,wave,ticket,sourcePath,baseHead,candidateHead,oldSource,newSource,
evidenceFiles,reviewer,reference`. Version1 is restricted to this run's worker08
and `tests/sv_entry_collector_checks.py`. Old/new source identities have exactly
`{size,sha256}`. The independently reviewed `baseHead` must equal the current
approved source seal; `candidateHead` must equal the saved request and current
worker branch. The controller verifies all 390 committed source blobs, permits
only this one exact test change, and derives the full tree diff itself: the test
and existing ticket08 metadata are modified, and exactly these six regular
nonexecutable evidence blobs are added:

- `.scratch/sandcastle-3fd6b311/evidence/08-approved-inventory/README.md`
- `.scratch/sandcastle-3fd6b311/evidence/08-approved-inventory/approval-status.json`
- `.scratch/sandcastle-3fd6b311/evidence/08-approved-inventory/collector-checks.log`
- `.scratch/sandcastle-3fd6b311/evidence/08-approved-inventory/consumption-checks.log`
- `.scratch/sandcastle-3fd6b311/evidence/08-approved-inventory/input-identities.json.gz`
- `.scratch/sandcastle-3fd6b311/evidence/08-approved-inventory/verification.json`

`evidenceFiles` maps exactly those paths to `{mode:"100644",size,sha256}`.
The profile raw SHA pins the independent review reference and every old/new
identity. No mutable metadata root, second consumed change, resource addition,
new executable recipe or policy privilege is admitted by this delegation.
The bootstrap policy preserves all capture argv, binding, runtime pins, prior
approvals, 1051 external resources and limits. Only exact reviewed nonconsumed
controller versions may be appended; the existing pinned readonly worker08
delivery changes to `mode:"historical-only"` without changing its descriptor.

After independent approval, the controller publishes a new immutable version
of the policy and source seal. This seal has `identityKind:"approved-inputs"`,
`approved_source_head` equal to the newly approved worker HEAD, and
`finalAcceptance:false` plus `milestone_acceptance:false`. Its legacy
`tested_head` field aliases the expected approved input target **only for this
explicit kind**, for compatibility with the existing collector seal guard.
It is not a capture result and does not assert that this HEAD was already
executed. `source_revision`/`renewal` retain the previous seal SHA, prior approved
HEAD, exact diff, independent execution ID and original resource/compiler
origins. All old seals, policies, receipts and their actual tested HEADs remain
unchanged. Actual capture reports continue to record their job HEAD and fresh
nonce; full acceptance still requires fresh complete captures after assembly.

Historical readonly delivery verifies its pinned index and every artifact byte,
then checks the index's original 390 source identities against Git at its
original tested HEAD and a matching retained prior approval. It neither asks
for current execution approval nor claims current source equivalence. The
separate readonly approval snapshot reports the current revision's status.
A historical header-gap report or an old producer identity remains evidence of
that original attempt, not a current blocker or fresh acceptance report.

### Verified assembly provisioning correction

Run `3fd6b311` can restore only the recorded recovery13 assembly provisioning
stop with `SANDCASTLE_ASSEMBLY_HOST_REPAIR_PROOF_PATH` and
`SANDCASTLE_ASSEMBLY_HOST_REPAIR_PROOF_SHA256`, as the sole continuation mode.
This is a verified controller correction, not an attempt grant: the exhausted
`assemble:8:8:0:1:0` counter remains three and its limit remains three.

The independently pinned private proof has version1 and exactly:
`version,authorization,runId,checkpoint,recoveryId,rawStateSha256,stateFingerprint,
oldProfilePin,newProfilePin,controllerDigest,assemblyHead,bundle,focused,review`.
Authorization is `user-requested-assembly-provisioning-repair`; checkpoint and
recoveryId are the exact values above and13. `review` supplies the independent
reviewer and reference. `newProfilePin` is the independently reviewed private
controller-only profile clone; existing guards preserve its source390 seal,
1051 resources, native argv, binding, runtime pins, approvals and readonly
collector deliveries, and permit only loaded controller identity plus exact
committed nonconsumed controller version pins to change.

`bundle` contains `indexSha256,head,receipts,artifactCount,totalBytes` from
`historicalDeliveryMounts({root,state}).proof`; each receipt retains its original
nonce, renderer, receiptDigest and artifactCount. The complete historical
bundle is reverified: original37b source317, software/OpenGL receipts, every
artifact byte, original old binding and scope, paths/noextras/nosymlinks. It
never asserts current source equivalence or final acceptance. Worker and
integration provisioning both mount this bundle readonly at
`/home/agent/.sv-host-delivery` through the same validator.

Before activation, run the corrected `integrationSandboxProvider` on a fresh
isolated sandbox at `assemblyHead`, with no native execution or auth mount.
Run these exact focused commands and retain their combined unittest logs as
private regular files inside the run directory:

- `python3 -B tests/sv_entry_collector_checks.py`: six tests, no skips.
- `env PYTHONPATH=tests python3 -B tests/sv_entry_consumption_checks.py`:26 tests,
  exactly the two documented skips.

The proof's `focused` array retains both commands in that order and each
`logPath,sha256,exitCode:0,tests,skipped`. Preparation checks the actual Ran/OK
results and all390 current approved source Git mode/size/hash identities at the
exact tested assembled HEAD. Missing/changed bundles, logs, native sources,
policies, unrelated failures or an exhausted quota fail closed.

Apply repeats all verification under the held controller lock, checks raw
checkpoint/pins with CAS, fsyncs an archive of the exact original state bytes,
then applies controller pins and the assembly phase transition to one clone
with one workflow save. No quota, recovery history, completed tickets, sealed
inputs or old receipts change. A fresh integration sandbox is created with the
validated readonly mount. Revisions already merged remain skipped by ancestry;
full acceptance still requires the controller's fresh postassembly captures.

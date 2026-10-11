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

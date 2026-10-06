import { acceptContract, assertContract, digest, validateContract } from "./acceptance.mjs";

const changedContractPause = "Acceptance: committed contract changed; human decision required";
const need = (condition, message) => { if (!condition) throw new Error(`Contract command repair: ${message}`); };
const same = (left, right) => JSON.stringify(left) === JSON.stringify(right);

export function validateCommandOnlyCorrection(previous, replacement) {
  need(previous?.completionScope === "implementation" && replacement?.completionScope === "implementation", "requires implementation contracts");
  need(previous.version === replacement.version, "contract version changed");
  need(same(previous.criteria, replacement.criteria), "criteria or mandatory/deferred semantics changed");
  need(Array.isArray(previous.checks) && previous.checks.length === replacement.checks?.length, "check set changed");
  const changes = [];
  for (let index = 0; index < previous.checks.length; index++) {
    const before = previous.checks[index];
    const after = replacement.checks[index];
    const { command: beforeCommand, ...beforeFixed } = before;
    const { command: afterCommand, ...afterFixed } = after;
    need(same(beforeFixed, afterFixed), `check metadata changed at ${before.id ?? index}`);
    if (!same(beforeCommand, afterCommand)) changes.push({ id: before.id, before: beforeCommand, after: afterCommand });
  }
  need(changes.length === 1, "exactly one command check must change");
  const change = changes[0];
  need(previous.checks.find((check) => check.id === change.id)?.kind === "command", "only a focused command check may change");
  return change;
}

export function resumeCommandContractRepair(state) {
  need(state?.phase === "awaiting" && state.resumePhase === "tickets", "requires the saved post-recovery tickets checkpoint");
  need(state.pauseReason === changedContractPause, "requires the committed-contract mismatch pause");
  need(!state.wave, "cannot repair while a worker wave is active");
  assertContract(state);
  need(state.acceptance.contract.completionScope === "implementation", "is only valid for implementation scope");
  const priorRepairs = (state.contractAmendments ?? []).filter((item) => item.kind === "command-only");
  need(priorRepairs.length === 0, "command-only repair limit reached");
  state.contractCommandRepair = {
    previousDigest: state.acceptance.contractDigest,
    returnPhase: state.resumePhase,
    pauseReason: state.pauseReason,
  };
  state.phase = "contract-command-repair";
  delete state.resumePhase;
  delete state.pauseReason;
  return state;
}

export async function commandContractRepairStage(state, ops) {
  const pending = state.contractCommandRepair;
  need(state.phase === "contract-command-repair" && pending, "missing prepared checkpoint");
  assertContract(state);
  need(pending.previousDigest === state.acceptance.contractDigest, "saved contract digest changed");
  const sources = await ops.sources();
  const artifact = await ops.artifact();
  need(artifact && artifact.digest === digest(artifact.contract), "replacement artifact digest is invalid");
  validateContract(artifact.contract, sources);
  const change = validateCommandOnlyCorrection(state.acceptance.contract, artifact.contract);
  need(!pending.candidateDigest || pending.candidateDigest === artifact.digest, "replacement artifact changed during verification");
  pending.candidateDigest ??= artifact.digest;
  pending.change ??= structuredClone(change);
  need(same(pending.change, change), "saved command correction changed");
  const oldMissing = await ops.missingRunners(state.acceptance.contract);
  const newMissing = await ops.missingRunners(artifact.contract);
  need(oldMissing.length > 0 && oldMissing.every((item) => item.checkId === change.id), "previous contract is not blocked only by the corrected runner");
  need(newMissing.length === 0, "replacement command runner does not exist");
  await ops.save(state);

  if (!pending.review) {
    pending.review = await ops.verify(artifact.contract, sources, change);
    await ops.save(state);
  }
  if (!pending.nextAcceptance) {
    const candidate = structuredClone(state);
    acceptContract(candidate, artifact.contract, sources, pending.review, await ops.head());
    candidate.acceptance.commandRepair = {
      previousDigest: state.acceptance.contractDigest,
      replacementDigest: artifact.digest,
      checkId: change.id,
    };
    pending.nextAcceptance = candidate.acceptance;
    await ops.save(state);
  }

  await ops.publish(pending.nextAcceptance);
  const previous = state.acceptance;
  state.contractAmendments ??= [];
  state.contractAmendments.push({
    kind: "command-only",
    reason: pending.pauseReason,
    checkId: change.id,
    commandBefore: structuredClone(change.before),
    commandAfter: structuredClone(change.after),
    contract: structuredClone(previous.contract),
    contractDigest: previous.contractDigest,
    contractReview: structuredClone(previous.contractReview),
    sourceDigests: structuredClone(previous.sourceDigests),
    acceptedHead: previous.acceptedHead,
    replacementDigest: pending.nextAcceptance.contractDigest,
  });
  state.acceptance = pending.nextAcceptance;
  state.phase = pending.returnPhase;
  delete state.contractCommandRepair;
  await ops.save(state);
}

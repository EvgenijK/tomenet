import { assertContract, requiredChecks } from "./acceptance.mjs";

export const maxContractAmendments = 1;
const externalRunnerPause = /^Acceptance: required external check ([a-zA-Z0-9][a-zA-Z0-9_.-]{0,79}) has no runner; human decision required$/;
const need = (condition, message) => { if (!condition) throw new Error(`Contract amend RESUME: ${message}`); };

function currentMandatoryExternalIds(contract) {
  const checks = new Map(contract.checks.map((check) => [check.id, check]));
  return [...new Set(contract.criteria
    .filter((criterion) => criterion.mandatory && criterion.applicability === "current")
    .flatMap((criterion) => criterion.checkIds.filter((id) => checks.get(id)?.kind === "external")))];
}

// This is deliberately narrower than ordinary recovery. It repairs one
// historical pipeline defect, once, without resetting implementation state or
// turning a missing runner into passing evidence.
export function resumeExternalContractAmendment(state) {
  need(state?.phase === "awaiting" && state.resumePhase === "test-gate", "requires a paused test-gate checkpoint");
  const match = externalRunnerPause.exec(state.pauseReason ?? "");
  need(match, "requires the saved required-external-check runner failure");
  assertContract(state);
  need(state.acceptance.contract.completionScope === "implementation", "is only valid for implementation scope");
  const checkId = match[1];
  const check = requiredChecks(state.acceptance.contract).find((item) => item.id === checkId);
  need(check?.kind === "external", `saved check ${checkId} is not a required external check`);
  const amendments = state.contractAmendments ?? [];
  need(Array.isArray(amendments) && amendments.length < maxContractAmendments, "amendment limit reached");

  const previous = structuredClone(state.acceptance);
  const requiredExternalCheckIds = currentMandatoryExternalIds(previous.contract);
  state.contractAmendments = [...amendments, {
    reason: state.pauseReason,
    checkId,
    requiredExternalCheckIds,
    contract: structuredClone(previous.contract),
    contractDigest: previous.contractDigest,
    contractReview: structuredClone(previous.contractReview),
    sourceDigests: structuredClone(previous.sourceDigests),
    acceptedHead: previous.acceptedHead,
    evidence: structuredClone(previous.evidence),
    acceptance: previous,
  }];
  state.contractAmendment = {
    previousDigest: previous.contractDigest,
    checkId,
    requiredExternalCheckIds,
    decision: `Remove ${checkId} as the audited duplicate gate while preserving its criterion and existing runnable mappings. Replace every other mandatory/current external mapping with runnable repository evidence; do not waive or defer requirements.`,
  };
  delete state.acceptance;
  delete state.contractPending;
  delete state.assessmentPending;
  delete state.reviewFindings;
  state.phase = "contract";
  state.contractReturnPhase = "test-gate";
  delete state.resumePhase;
  delete state.pauseReason;
  return state;
}

export function validateContractAmendment(previous, next, amendment) {
  need(previous?.version === 1 && next?.version === 1, "requires versioned contracts");
  need(previous.completionScope === next.completionScope, "cannot change completion scope");
  const before = new Map(previous.criteria.map((criterion) => [criterion.id, criterion]));
  const after = new Map(next.criteria.map((criterion) => [criterion.id, criterion]));
  need(before.size === after.size && [...before.keys()].every((id) => after.has(id)), "cannot add or remove acceptance criteria");
  const priorChecks = new Map(previous.checks.map((check) => [check.id, check]));
  const nextChecks = new Map(next.checks.map((check) => [check.id, check]));
  const semanticFields = ["requirement", "mandatory", "applicability", "owner", "deferralReason"];
  for (const [id, oldCriterion] of before) {
    const newCriterion = after.get(id);
    need(semanticFields.every((field) => JSON.stringify(newCriterion[field]) === JSON.stringify(oldCriterion[field])) &&
      JSON.stringify(newCriterion.source) === JSON.stringify(oldCriterion.source), `cannot alter or downgrade criterion ${id}`);
    const oldRunnable = oldCriterion.checkIds.filter((checkId) => priorChecks.get(checkId)?.kind !== "external");
    need(oldRunnable.every((checkId) => newCriterion.checkIds.includes(checkId)), `cannot remove runnable evidence from criterion ${id}`);
  }
  const removeIds = new Set(amendment?.requiredExternalCheckIds ?? []);
  need(removeIds.size > 0, "requires recorded external mappings");
  for (const id of removeIds) {
    const stillCurrent = next.criteria.some((criterion) => criterion.mandatory && criterion.applicability === "current" && criterion.checkIds.includes(id));
    need(!stillCurrent, `mandatory/current external check ${id} remains mapped`);
    const deferredUse = next.criteria.some((criterion) => criterion.applicability === "deferred" && criterion.checkIds.includes(id));
    need(deferredUse || !nextChecks.has(id), `obsolete external check ${id} must be removed`);
  }
  need(!nextChecks.has(amendment.checkId), `audited duplicate check ${amendment.checkId} must be removed`);
  return next;
}

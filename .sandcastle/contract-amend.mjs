import { assertContract, requiredChecks } from "./acceptance.mjs";

export const maxContractAmendments = 1;
const externalRunnerPause = /^Acceptance: required external check ([a-zA-Z0-9][a-zA-Z0-9_.-]{0,79}) has no runner; human decision required$/;
const need = (condition, message) => { if (!condition) throw new Error(`Contract amend RESUME: ${message}`); };

// A contract amendment is deliberately narrower than ordinary recovery: it is
// available once, only for the historical pipeline defect that accepted an
// unrunnable implementation check. Implementation progress stays intact while
// contract evidence is rebuilt under a new independently verified digest.
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

  const previous = state.acceptance;
  state.contractAmendments = [...amendments, {
    reason: state.pauseReason,
    checkId,
    contract: structuredClone(previous.contract),
    contractDigest: previous.contractDigest,
    contractReview: structuredClone(previous.contractReview),
    sourceDigests: structuredClone(previous.sourceDigests),
    acceptedHead: previous.acceptedHead,
  }];
  state.contractAmendment = { previousDigest: previous.contractDigest, checkId };
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

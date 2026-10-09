import { assertContract, requiredChecks } from "./acceptance.mjs";

export const maxContractAmendments = 1;
export const maxSemanticContractAmendments = 3;
export const maxContractAmendmentRetryAttempts = 3;
export const maxContractAmendmentRetryAuthorizations = 1;
const externalRunnerPause = /^Acceptance: required external check ([a-zA-Z0-9][a-zA-Z0-9_.-]{0,79}) has no runner; human decision required$/;
const need = (condition, message) => { if (!condition) throw new Error(`Contract amend RESUME: ${message}`); };
const identifier = (value) => typeof value === "string" && /^[a-zA-Z0-9][a-zA-Z0-9_.-]{0,79}$/.test(value);

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
    type: "external-runner-repair",
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

// A user may explicitly change the task semantics after a contract has been
// accepted. This is intentionally separate from repair/recovery: the previous
// contract and all evidence are archived, the frozen originating spec is
// replaced by the committed amended text, and a fresh independently verified
// contract is required before planning can continue.
export function resumeSemanticContractAmendment(state, amendedSpec, authorization) {
  need(state?.phase === "awaiting" && state.resumePhase === "plan", "semantic amendment requires a paused plan checkpoint");
  need(/^User-approved requirement amendment is not represented in the immutable acceptance contract:/.test(state.pauseReason ?? ""), "semantic amendment requires the saved user-approved requirement pause");
  assertContract(state);
  need(state.acceptance.contract.completionScope === "implementation", "semantic amendment is only valid for implementation scope");
  need(typeof state.specPath === "string" && typeof state.spec === "string" && state.spec.length > 0, "semantic amendment requires a frozen originating spec");
  need(authorization && identifier(authorization.authorizationId), "semantic amendment requires a valid authorization ID");
  const authorizations = state.semanticContractAmendmentAuthorizations ?? [];
  need(Array.isArray(authorizations) && authorizations.length < maxSemanticContractAmendments, "semantic amendment limit reached");
  need(!authorizations.some((item) => item.authorizationId === authorization.authorizationId), "semantic amendment authorization ID already used");
  need(typeof amendedSpec === "string" && amendedSpec.length > 0 && amendedSpec !== state.spec, "semantic amendment requires changed spec text");
  need(typeof authorization.decision === "string" && authorization.decision.trim().length > 0, "semantic amendment requires the explicit user decision");
  need(Array.isArray(authorization.requiredQuotes) && authorization.requiredQuotes.length > 0 && new Set(authorization.requiredQuotes).size === authorization.requiredQuotes.length && authorization.requiredQuotes.every((quote) => typeof quote === "string" && quote.trim().length > 0), "semantic amendment requires unique non-empty source quotes");

  for (const criterion of state.acceptance.contract.criteria.filter((item) => item.source.path === state.specPath)) {
    need(amendedSpec.includes(criterion.source.quote), `amended spec removed represented source for ${criterion.id}`);
  }
  for (const quote of authorization.requiredQuotes) {
    need(amendedSpec.includes(quote), `amended spec is missing required quote: ${quote}`);
    need(!state.spec.includes(quote), `required quote is not new: ${quote}`);
  }

  const previous = structuredClone(state.acceptance);
  const amendments = state.contractAmendments ?? [];
  state.contractAmendments = [...amendments, {
    type: "semantic-user-requirement",
    reason: state.pauseReason,
    authorization: structuredClone(authorization),
    previousSpec: state.spec,
    amendedSpec,
    contract: structuredClone(previous.contract),
    contractDigest: previous.contractDigest,
    contractReview: structuredClone(previous.contractReview),
    sourceDigests: structuredClone(previous.sourceDigests),
    acceptedHead: previous.acceptedHead,
    evidence: structuredClone(previous.evidence),
    acceptance: previous,
  }];
  state.semanticContractAmendmentAuthorizations = [...authorizations, structuredClone(authorization)];
  state.contractAmendment = {
    type: "semantic-user-requirement",
    previousDigest: previous.contractDigest,
    authorizationId: authorization.authorizationId,
    decision: authorization.decision,
    requiredQuotes: structuredClone(authorization.requiredQuotes),
    specPath: state.specPath,
  };
  state.spec = amendedSpec;
  delete state.acceptance;
  delete state.contractPending;
  delete state.assessmentPending;
  delete state.reviewFindings;
  state.phase = "contract";
  state.contractReturnPhase = "plan";
  delete state.resumePhase;
  delete state.pauseReason;
  return state;
}

export function validateContractAmendment(previous, next, amendment) {
  need(previous?.version === 1 && next?.version === 1, "requires versioned contracts");
  need(previous.completionScope === next.completionScope, "cannot change completion scope");
  const before = new Map(previous.criteria.map((criterion) => [criterion.id, criterion]));
  const after = new Map(next.criteria.map((criterion) => [criterion.id, criterion]));
  const semantic = amendment?.type === "semantic-user-requirement";
  need([...before.keys()].every((id) => after.has(id)) && (semantic || before.size === after.size), semantic ? "cannot remove prior acceptance criteria" : "cannot add or remove acceptance criteria");
  const priorChecks = new Map(previous.checks.map((check) => [check.id, check]));
  const nextChecks = new Map(next.checks.map((check) => [check.id, check]));
  const semanticFields = ["requirement", "mandatory", "applicability", "owner", "deferralReason"];
  for (const [id, oldCriterion] of before) {
    const newCriterion = after.get(id);
    need(semanticFields.every((field) => JSON.stringify(newCriterion[field]) === JSON.stringify(oldCriterion[field])) &&
      JSON.stringify(newCriterion.source) === JSON.stringify(oldCriterion.source), `cannot alter or downgrade criterion ${id}`);
    const oldRunnable = oldCriterion.checkIds.filter((checkId) => priorChecks.get(checkId)?.kind !== "external");
    need(semantic ? JSON.stringify(newCriterion.checkIds) === JSON.stringify(oldCriterion.checkIds) : oldRunnable.every((checkId) => newCriterion.checkIds.includes(checkId)), semantic ? `cannot alter evidence mappings for prior criterion ${id}` : `cannot remove runnable evidence from criterion ${id}`);
  }
  if (semantic) {
    for (const [id, check] of priorChecks) need(JSON.stringify(nextChecks.get(id)) === JSON.stringify(check), `cannot alter or remove prior check ${id}`);
    const added = next.criteria.filter((criterion) => !before.has(criterion.id));
    need(added.length > 0, "semantic amendment requires at least one new criterion");
    need(added.every((criterion) => criterion.mandatory && criterion.applicability === "current" && criterion.source.path === amendment.specPath), "new semantic criteria must be mandatory/current and sourced from the amended spec");
    for (const quote of amendment.requiredQuotes ?? []) {
      need(added.some((criterion) => criterion.source.quote.includes(quote)), `new semantic criteria do not cite required quote: ${quote}`);
    }
    return next;
  }
  for (const oldCriterion of previous.criteria.filter((criterion) => criterion.checkIds.includes(amendment.checkId))) {
    const expected = oldCriterion.checkIds.filter((id) => id !== amendment.checkId).sort();
    const actual = after.get(oldCriterion.id).checkIds.slice().sort();
    need(JSON.stringify(actual) === JSON.stringify(expected), `audited duplicate ${amendment.checkId} must be removed without changing other mappings for ${oldCriterion.id}`);
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

export function contractAmendmentVerificationContext(state) {
  need(state?.contractAmendment && Array.isArray(state.contractAmendments) && state.contractAmendments.length > 0, "verification context requires an active archived amendment");
  const archived = state.contractAmendments.at(-1);
  need(archived.contractDigest === state.contractAmendment.previousDigest && archived.contract, "verification context archive does not match the active amendment");
  return { previousContract: structuredClone(archived.contract), amendment: structuredClone(state.contractAmendment) };
}

export function resumeContractAmendmentRetry(state, authorizationId) {
  need(typeof authorizationId === "string" && /^[a-zA-Z0-9][a-zA-Z0-9_.-]{0,79}$/.test(authorizationId), "requires a valid authorization ID");
  need(state?.phase === "awaiting" && state.resumePhase === "contract", "requires a paused contract amendment checkpoint");
  need(state.pauseReason === "Acceptance: contract feedback made no progress for 3 identical reviews; human decision required", "requires the saved identical-feedback convergence stop");
  need(state.contractAmendment && Array.isArray(state.contractAmendments) && state.contractAmendments.length === 1, "requires an active archived amendment");
  need((state.contractPending?.convergence?.identicalOpenRounds ?? 0) >= 3, "requires exhausted identical verifier feedback");
  const authorizations = state.contractAmendmentAuthorizations ?? [];
  need(Array.isArray(authorizations) && authorizations.length < maxContractAmendmentRetryAuthorizations, "amendment retry authorization limit reached");
  need(!authorizations.some((item) => item.id === authorizationId), "authorization ID already used");

  const previousPending = structuredClone(state.contractPending);
  state.contractAmendmentRetryHistory = [...(state.contractAmendmentRetryHistory ?? []), { authorizationId, pending: previousPending, pauseReason: state.pauseReason }];
  state.contractAmendmentAuthorizations = [...authorizations, { id: authorizationId, maxAttempts: maxContractAmendmentRetryAttempts }];
  state.contractPending = {
    round: 0,
    sourceManifest: structuredClone(previousPending.sourceManifest),
    feedbackLedger: structuredClone(previousPending.feedbackLedger),
  };
  state.contractAmendmentAttemptLimit = maxContractAmendmentRetryAttempts;
  state.phase = "contract";
  delete state.resumePhase;
  delete state.pauseReason;
  return state;
}

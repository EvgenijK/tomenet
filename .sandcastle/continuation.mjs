import { randomUUID } from "node:crypto";
import { taskQuotaLimit, extendTaskQuota } from "./limits.mjs";

const kinds = {
  build: { variable: "SANDCASTLE_EXTRA_BUILD_ATTEMPTS", limit: "buildLimit", phases: ["build", "build-repair", "test-gate"], counters: ["buildAttempts", "testBuildAttempts"], reason: /^(?:Build failed \d+ times; continuation requires human approval|Test-gate rebuild failed \d+ times; continuation requires human approval|Build attempt limit reached|Test-gate rebuild attempt limit reached)/ },
  test: { variable: "SANDCASTLE_EXTRA_TEST_ATTEMPTS", limit: "testLimit", phases: ["test-gate", "test-repair"], counters: ["testAttempts"], reason: /^(?:Tests failed \d+ times; continuation requires human approval|Test attempt limit reached)/ },
  review: { variable: "SANDCASTLE_EXTRA_REVIEW_ATTEMPTS", limit: "reviewLimit", phases: ["review", "plan", "plan-validate"], counters: ["reviewRound"], reason: /^(?:Acceptance: )?review round limit reached; human approval required/i },
};

function validateExtra(kind, extra) {
  const policy = kinds[kind];
  if (!policy) throw new Error(`Unknown attempt allowance: ${kind}`);
  if (!Number.isSafeInteger(extra) || extra < 1 || extra > 100) throw new Error(`${policy.variable} must be an integer from 1 to 100`);
  return policy;
}

function appendAttemptAllowance(state, kind, extra, authorizationId, explanation) {
  const policy = kinds[kind];
  const previousLimit = state[policy.limit];
  const consumedAttempts = Math.max(...policy.counters.map((field) => state[field] ?? 0));
  const limit = Math.max(previousLimit, consumedAttempts) + extra;
  if (!Number.isSafeInteger(limit)) throw new Error("Attempt allowance exceeds safe integer range");
  (state.attemptAllowanceHistory ??= []).push({ id: authorizationId, kind, extraAttempts: extra, allowanceDelta: limit - previousLimit, previousLimit, limit, counters: Object.fromEntries(policy.counters.map((field) => [field, state[field] ?? 0])), resumePhase: state.resumePhase, explanation, grantedAt: new Date().toISOString() });
  state[policy.limit] = limit;
}

function validateAuthorization(state, authorizationId, explanation) {
  if (typeof authorizationId !== "string" || !authorizationId.trim() || typeof explanation !== "string" || !explanation.trim()) throw new Error("Allowance grant requires an authorization ID and explanation");
  if ([...(state.attemptAllowanceHistory ?? []), ...(state.budget?.allowanceHistory ?? [])].some((entry) => entry.id === authorizationId)) throw new Error("Allowance grant authorization already recorded");
}

const quotaReason = /^Task reached [\d.]+% of the weekly Codex quota$/;

function activeGuards(state) {
  const guards = [];
  if (state.quotaError && !quotaReason.test(state.quotaError)) throw new Error(`Account/token/usage availability guard cannot be authorized by an allowance: ${state.quotaError}`);
  if (state.quotaError || (state.budget?.consumedPercent ?? 0) >= taskQuotaLimit(state)) guards.push({ kind: "quota", reason: "Task quota limit reached; SANDCASTLE_EXTRA_QUOTA_PERCENT is required" });
  for (const [kind, policy] of Object.entries(kinds)) {
    if (!policy.phases.includes(state.resumePhase)) continue;
    if (!Number.isSafeInteger(state[policy.limit]) || state[policy.limit] < 0 || policy.counters.some((field) => state[field] != null && (!Number.isSafeInteger(state[field]) || state[field] < 0))) throw new Error("Invalid saved attempt allowance accounting");
    const counters = kind === "build" && state.resumePhase === "test-gate" ? ["testBuildAttempts"] : policy.counters;
    if (counters.some((field) => (state[field] ?? 0) >= state[policy.limit])) guards.push({ kind, reason: `${kind} attempt limit reached; ${policy.variable} is required` });
  }
  return guards;
}

// A saved limit pause may have more than one exhausted allowance. Each explicit
// command authorizes one only; the checkpoint stays paused until all are clear.
// Work is performed on a clone so validation failures cannot partially grant it.
export function applyAllowanceGrant(state, { kind, extra, authorizationId = randomUUID(), explanation = "Explicit additional allowance authorization" }) {
  const next = structuredClone(state);
  if (next.phase !== "awaiting" || !next.resumePhase) throw new Error("Allowance grant requires a saved limit pause");
  const context = next.allowanceContinuation;
  const originalReason = context?.originalPauseReason ?? next.pauseReason ?? "";
  if (context && context.resumePhase !== next.resumePhase) throw new Error("Saved allowance checkpoint changed");
  if (!quotaReason.test(originalReason) && !Object.values(kinds).some((policy) => policy.phases.includes(next.resumePhase) && policy.reason.test(originalReason))) throw new Error("Allowance grant requires a saved quota or attempt limit pause");
  validateAuthorization(next, authorizationId, explanation);
  const guards = activeGuards(next);
  if (!guards.some((guard) => guard.kind === kind)) throw new Error(`No saved exhausted ${kind} allowance at this checkpoint`);
  if (kind === "quota") {
    // The independent guard above confirmed exhaustion even if the original
    // saved stop named the attempt limit. Narrow quota API retains its legacy
    // validation contract; this temporary reason never enters the history.
    const savedReason = next.pauseReason;
    next.pauseReason = `Task reached ${taskQuotaLimit(next)}% of the weekly Codex quota`;
    extendTaskQuota(next, extra, { authorizationId, explanation });
    next.pauseReason = savedReason;
  } else {
    validateExtra(kind, extra);
    appendAttemptAllowance(next, kind, extra, authorizationId, explanation);
  }
  const remaining = activeGuards(next);
  next.allowanceContinuation = { ...(context ?? { id: randomUUID(), originalPauseReason: originalReason, resumePhase: next.resumePhase, createdAt: new Date().toISOString() }), remainingGuards: remaining };
  if (remaining.length) {
    next.pauseReason = remaining.map((guard) => guard.reason).join("; ");
  } else {
    (next.allowanceContinuationHistory ??= []).push({ ...next.allowanceContinuation, resumedAt: new Date().toISOString() });
    delete next.allowanceContinuation;
    next.phase = next.resumePhase;
    delete next.resumePhase;
    delete next.pauseReason;
  }
  for (const key of Object.keys(state)) if (!(key in next)) delete state[key];
  Object.assign(state, next);
  return state;
}

export function rejectLegacyContinue(env = process.env) {
  if (env.SANDCASTLE_CONTINUE !== undefined) throw new Error("SANDCASTLE_CONTINUE is deprecated because it reset usage and recovery history. Use SANDCASTLE_EXTRA_QUOTA_PERCENT for a saved quota pause, SANDCASTLE_EXTRA_BUILD_ATTEMPTS / SANDCASTLE_EXTRA_TEST_ATTEMPTS / SANDCASTLE_EXTRA_REVIEW_ATTEMPTS for the matching saved attempt-limit pause, or SANDCASTLE_RESUME=1 for a user pause. No counters or allowances were changed.");
}

export function readAttemptGrant(env = process.env) {
  const requests = Object.entries(kinds).filter(([, policy]) => env[policy.variable] !== undefined);
  if (requests.length > 1) throw new Error("Grant one attempt allowance at a time; each requires its matching saved limit pause");
  if (!requests.length) return undefined;
  const [kind, policy] = requests[0];
  if (!/^[1-9]\d*$/.test(env[policy.variable])) throw new Error(`${policy.variable} must be an integer from 1 to 100`);
  const extra = Number(env[policy.variable]);
  validateExtra(kind, extra);
  return { kind, extra };
}

// Explicit grants change one allowance, never accounting, recovery attempts,
// task progress or contract state. Validation finishes before any mutation.
export function grantAttempts(state, kind, extra, { authorizationId = randomUUID(), explanation = "Explicit additional attempt authorization" } = {}) {
  const policy = validateExtra(kind, extra);
  if (state.phase !== "awaiting" || !policy.phases.includes(state.resumePhase) || !policy.reason.test(state.pauseReason ?? "") ||
    !Number.isSafeInteger(state[policy.limit]) || state[policy.limit] < 0 ||
    policy.counters.some((field) => state[field] != null && (!Number.isSafeInteger(state[field]) || state[field] < 0)) ||
    !policy.counters.some((field) => Number.isSafeInteger(state[field]) && state[field] >= state[policy.limit])) throw new Error(`${policy.variable} requires a saved matching attempt limit pause`);
  if (state.quotaError || (state.budget?.consumedPercent ?? 0) >= taskQuotaLimit(state)) throw new Error("Task quota guard still requires an explicit additional allowance; attempt grant cannot authorize quota");
  validateAuthorization(state, authorizationId, explanation);
  appendAttemptAllowance(state, kind, extra, authorizationId, explanation);
  state.phase = state.resumePhase;
  delete state.resumePhase;
  delete state.pauseReason;
  return state;
}

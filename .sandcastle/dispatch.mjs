import { preflightExecution } from "./execution.mjs";
import { pauseStatus, classifyFailure } from "./failure.mjs";
import { taskQuotaLimit } from "./limits.mjs";

export function resumeExecutionCheckpoint(state) {
  if (state.phase !== "awaiting" || !pauseStatus(state)?.fatalPreflight || !/^[a-z][a-z-]*$/.test(state.resumePhase ?? "")) {
    throw new Error("SANDCASTLE_PREFLIGHT_RESUME requires a saved execution preflight failure");
  }
  if (state.quotaError || (state.budget?.consumedPercent ?? 0) >= taskQuotaLimit(state)) throw new Error("Preflight resume cannot grant quota; authorize the remaining allowance separately");
  state.phase = state.resumePhase;
  delete state.resumePhase;
  delete state.pauseReason;
  // currentFailure remains until verification proves the correction.
  return state;
}

// Verification must precede even the quota probe: a deterministic local fault
// must not consume inference or enter an AI recovery loop.
export async function dispatchVerified(execution, { preflight = preflightExecution, record, quota, invoke }) {
  const manifest = await preflight(execution);
  await record(manifest);
  await quota();
  return invoke(manifest);
}

export async function closeExecution(sandbox, { failure, preserve }) {
  try {
    // Readiness failures must not commit the branch or dirty input they refused.
    // The provider preserves dirty worktrees when closing their runtime.
    if (!failure || !classifyFailure(failure).fatalPreflight) await preserve();
  } finally { await sandbox.close(); }
}

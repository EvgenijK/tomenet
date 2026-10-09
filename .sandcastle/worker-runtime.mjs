import { taskQuotaLimit } from "./limits.mjs";

// A bounded retry after fixing the observed old-CLI model-routing failure.
// Historical recovery attempts remain exhausted; another failure stops again.
export function resumeWorkerRuntimeUpgrade(state, { version, imageId }) {
  if (state.phase !== "awaiting" || state.resumePhase !== "parallel-work" ||
    state.pauseReason !== "Automatic recovery made no progress after three decisions at this checkpoint" ||
    state.agentPolicy?.model !== "gpt-6.1-sol" || version !== "0.161.0" || !/^sha256:[a-f0-9]{64}$/.test(imageId ?? "") ||
    !state.wave?.members?.length || state.wave.members.some((member) => member.status !== "failed" || member.head || member.result || member.integrated ||
      !member.error?.includes('"status":400') || !member.error.includes("The 'gpt-6.1-sol' model is not supported when using Codex with a ChatGPT account.")) ||
    state.workerRuntimeUpgradeResumes?.some((record) => record.waveId === state.wave.id)) {
    throw new Error("Worker runtime upgrade requires the saved unsupported-model startup failure and verified Codex 0.161.0 image");
  }
  if (state.quotaError || (state.budget?.consumedPercent ?? 0) >= taskQuotaLimit(state)) throw new Error("Worker runtime upgrade requires available quota");
  (state.workerRuntimeUpgradeResumes ??= []).push({ waveId: state.wave.id, fromVersion: "0.157.1", toVersion: version,
    imageId, at: new Date().toISOString(), previousPhase: state.phase, previousResumePhase: state.resumePhase,
    previousPauseReason: state.pauseReason, previousErrors: state.wave.members.map((member) => ({ branch: member.branch, error: member.error })) });
  state.phase = state.resumePhase;
  delete state.resumePhase;
  delete state.pauseReason;
  return state;
}

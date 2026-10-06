export function recordBuildAttempt(state, gate) {
  const field = gate === "build" ? "buildAttempts" : gate === "test-gate" ? "testBuildAttempts" : null;
  if (!field) throw new Error(`Unknown build gate: ${gate}`);
  state[field] = (state[field] ?? 0) + 1;
  state.totalBuildAttempts = (state.totalBuildAttempts ?? 0) + 1;
  return state[field];
}

export function refreshBuildBudgetsAfterBatch(state) {
  if (!state.resetBuildBudgetAfterBatch) return false;
  state.buildAttempts = 0;
  state.testBuildAttempts = 0;
  delete state.resetBuildBudgetAfterBatch;
  return true;
}

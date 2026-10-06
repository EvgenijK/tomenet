import assert from "node:assert/strict";
import test from "node:test";
import { recordBuildAttempt, refreshBuildBudgetsAfterBatch } from "../.sandcastle/build-budget.mjs";

test("project build and test-barrier rebuild have separate budgets refreshed after repair", () => {
  const state = { buildAttempts: 0, testBuildAttempts: 0, totalBuildAttempts: 0 };
  assert.equal(recordBuildAttempt(state, "build"), 1);
  assert.equal(recordBuildAttempt(state, "test-gate"), 1);
  assert.equal(recordBuildAttempt(state, "test-gate"), 2);
  assert.deepEqual([state.buildAttempts, state.testBuildAttempts, state.totalBuildAttempts], [1, 2, 3]);
  assert.equal(refreshBuildBudgetsAfterBatch(state), false);
  state.resetBuildBudgetAfterBatch = true;
  assert.equal(refreshBuildBudgetsAfterBatch(state), true);
  assert.deepEqual([state.buildAttempts, state.testBuildAttempts, state.totalBuildAttempts], [0, 0, 3]);
  assert.equal(recordBuildAttempt(state, "build"), 1);
  assert.equal(recordBuildAttempt(state, "test-gate"), 1);
  assert.equal(state.totalBuildAttempts, 5);
});

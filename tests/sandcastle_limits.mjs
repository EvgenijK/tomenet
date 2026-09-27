import assert from "node:assert/strict";
import test from "node:test";
import { updateBudget } from "../.sandcastle/limits.mjs";

test("weekly budget counts only usage after this task starts", () => {
  const state = {};
  updateBudget(state, { usedPercent: 62, resetsAt: 1000, ordinaryUsageAllowed: true });
  assert.equal(state.budget.consumedPercent, 0);
  updateBudget(state, { usedPercent: 86, resetsAt: 1000, ordinaryUsageAllowed: true });
  assert.equal(state.budget.consumedPercent, 24);
  assert.throws(() => updateBudget(state, { usedPercent: 87, resetsAt: 1000, ordinaryUsageAllowed: true }), /25%/);
});

test("budget remains conservative when the weekly window resets", () => {
  const state = {};
  updateBudget(state, { usedPercent: 80, resetsAt: 1000 });
  updateBudget(state, { usedPercent: 88, resetsAt: 1000 });
  updateBudget(state, { usedPercent: 10, resetsAt: 2000 });
  assert.equal(state.budget.consumedPercent, 18);
  assert.throws(() => updateBudget(state, { usedPercent: 17, resetsAt: 2000 }), /25%/);
});

test("explicit task token cap stops at the configured count", () => {
  const before = process.env.SANDCASTLE_TASK_TOKEN_LIMIT;
  try {
    process.env.SANDCASTLE_TASK_TOKEN_LIMIT = "100";
    const state = { budget: { consumedPercent: 0, lastUsedPercent: 1, resetsAt: 1000, tokens: 100 } };
    assert.throws(() => updateBudget(state, { usedPercent: 1, resetsAt: 1000 }), /token limit/);
  } finally {
    if (before === undefined) delete process.env.SANDCASTLE_TASK_TOKEN_LIMIT;
    else process.env.SANDCASTLE_TASK_TOKEN_LIMIT = before;
  }
});

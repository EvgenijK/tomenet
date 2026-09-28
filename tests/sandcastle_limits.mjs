import assert from "node:assert/strict";
import test from "node:test";
import { updateBudget, extendTaskQuota } from "../.sandcastle/limits.mjs";
import { recoveryRestriction } from "../.sandcastle/recovery.mjs";

test("explicit extra quota preserves consumption and cycle limits and stops at the new cap", () => {
  const state = { phase: "awaiting", resumePhase: "assemble", quotaError: "Task reached 25% of the weekly Codex quota", budget: { consumedPercent: 25, lastUsedPercent: 47, resetsAt: 1000, tokens: 123 }, buildLimit: 60, testLimit: 60, reviewLimit: 30 };
  extendTaskQuota(state, 10);
  assert.equal(state.budget.limitPercent, 35);
  assert.equal(state.budget.consumedPercent, 25);
  assert.equal(state.budget.tokens, 123);
  assert.equal(state.quotaError, undefined);
  assert.deepEqual([state.buildLimit, state.testLimit, state.reviewLimit], [60, 60, 30]);
  assert.equal(recoveryRestriction(state), undefined);
  updateBudget(state, { usedPercent: 56, resetsAt: 1000 });
  assert.equal(state.budget.consumedPercent, 34);
  assert.throws(() => updateBudget(state, { usedPercent: 57, resetsAt: 1000 }), /35%/);
  assert.match(recoveryRestriction(state), /Quota guard/);
});

test("extra quota rejects invalid grants and account stops without changing the checkpoint", () => {
  for (const extra of [0, -1, NaN, Infinity, 101]) {
    const state = { phase: "awaiting", resumePhase: "assemble", quotaError: "Task reached 25% of the weekly Codex quota", budget: { consumedPercent: 25 } };
    const before = structuredClone(state);
    assert.throws(() => extendTaskQuota(state, extra), /EXTRA_QUOTA_PERCENT/);
    assert.deepEqual(state, before);
  }
  const state = { phase: "awaiting", resumePhase: "assemble", quotaError: "Codex account usage limit reached", budget: { consumedPercent: 25 } };
  const before = structuredClone(state);
  assert.throws(() => extendTaskQuota(state, 10), /task quota pause/);
  assert.deepEqual(state, before);
});

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

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

test("one-second reset timestamp drift does not count the same weekly usage twice", () => {
  const state = { budget: { consumedPercent: 41, lastUsedPercent: 14, resetsAt: 1000, tokens: 0, limitPercent: 100 } };
  updateBudget(state, { usedPercent: 16, resetsAt: 999, ordinaryUsageAllowed: true });
  assert.equal(state.budget.consumedPercent, 43);
  updateBudget(state, { usedPercent: 16, resetsAt: 1000, ordinaryUsageAllowed: true });
  assert.equal(state.budget.consumedPercent, 43);
});

test("same-window decrease is a correction, not a new window or additional consumption", () => {
  const state = { budget: { consumedPercent: 12, lastUsedPercent: 40, resetsAt: 1000, tokens: 222, limitPercent: 100 } };
  updateBudget(state, { usedPercent: 39.5, resetsAt: 1001 });
  assert.equal(state.budget.consumedPercent, 12);
  updateBudget(state, { usedPercent: 40, resetsAt: 1000 });
  assert.equal(state.budget.consumedPercent, 12);
  updateBudget(state, { usedPercent: 41, resetsAt: 1000 });
  assert.equal(state.budget.consumedPercent, 13);
  assert.equal(state.budget.tokens, 222);
  assert.match(state.budget.usageHistory[1].explanation, /correction/);
});

test("window snapshots are append-only with explicit rollover and legacy uncertainty", () => {
  const state = { budget: { consumedPercent: 8, lastUsedPercent: 88, resetsAt: 1000, tokens: 333, limitPercent: 100 } };
  updateBudget(state, { usedPercent: 90, resetsAt: 1000 });
  const history = structuredClone(state.budget.usageHistory);
  updateBudget(state, { usedPercent: 3, resetsAt: 700000 });
  assert.equal(state.budget.consumedPercent, 13);
  assert.deepEqual(state.budget.usageHistory.slice(0, history.length), history);
  assert.notEqual(state.budget.usageHistory.at(-1).windowId, history.at(-1).windowId);
  assert.equal(state.budget.usageHistory.at(-1).deltaPercent, 3);
  assert.match(state.budget.usageHistory[0].explanation, /cannot reconstruct/i);
  const before = state.budget.consumedPercent;
  updateBudget(state, { usedPercent: 95, resetsAt: 1000 });
  assert.equal(state.budget.consumedPercent, before);
  assert.match(state.budget.usageHistory.at(-1).explanation, /stale/);
});

test("quota grant preserves deep progress and records explicit immutable grant evidence", () => {
  const state = { phase: "awaiting", resumePhase: "plan", pauseReason: "Task reached 25% of the weekly Codex quota", budget: { consumedPercent: 26, lastUsedPercent: 60, resetsAt: 1000, tokens: 200 }, completedTickets: [{ path: "a" }], acceptance: { contract: { criteria: [1, 2] } }, recoveryAttempts: { checkpoint: 3 }, buildAttempts: 12, totalBuildAttempts: 34, testAttempts: 20, reviewRound: 10 };
  const before = structuredClone(state);
  extendTaskQuota(state, 5, { authorizationId: "user-5", explanation: "User authorized five additional percentage points" });
  const record = structuredClone(state.budget.allowanceHistory[0]);
  assert.equal(record.id, "user-5");
  assert.equal(record.windowId, "weekly:1000");
  assert.equal(record.extraPercent, 5);
  assert.equal(record.deltaPercent, 6);
  assert.equal(state.budget.limitPercent, 31);
  for (const key of ["completedTickets", "acceptance", "recoveryAttempts", "buildAttempts", "totalBuildAttempts", "testAttempts", "reviewRound"]) assert.deepEqual(state[key], before[key]);
  assert.equal(state.budget.tokens, before.budget.tokens);
  extendTaskQuota(state, 2, { authorizationId: "user-2" });
  assert.deepEqual(state.budget.allowanceHistory[0], record);
  const checkpoint = structuredClone(state);
  assert.throws(() => extendTaskQuota(state, 5, { authorizationId: "user-5" }), /already recorded/);
  assert.deepEqual(state, checkpoint);
});

test("invalid account snapshot cannot corrupt existing budget", () => {
  for (const snapshot of [{ usedPercent: -1 }, { usedPercent: 101 }, { usedPercent: NaN }, { usedPercent: 10, resetsAt: "wrong" }]) {
    const state = { budget: { consumedPercent: 7, tokens: 123 } };
    const before = structuredClone(state);
    assert.throws(() => updateBudget(state, snapshot), /Invalid.*snapshot/);
    assert.deepEqual(state, before);
  }
});

test("newly identified window records an unknown gap then counts future usage from its current baseline", () => {
  const state = { budget: { consumedPercent: 10, lastUsedPercent: 80, tokens: 123, limitPercent: 100 } };
  updateBudget(state, { usedPercent: 2, resetsAt: 700000 });
  assert.equal(state.budget.consumedPercent, 10);
  assert.equal(state.budget.windowHighWaterPercent, 2);
  assert.match(state.budget.usageHistory.at(-1).explanation, /cannot reconstruct/);
  updateBudget(state, { usedPercent: 5, resetsAt: 700000 });
  assert.equal(state.budget.consumedPercent, 13);
  const increasing = { budget: { consumedPercent: 10, lastUsedPercent: 80, tokens: 123, limitPercent: 100 } };
  updateBudget(increasing, { usedPercent: 85, resetsAt: 700000 });
  assert.equal(increasing.budget.consumedPercent, 15);
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

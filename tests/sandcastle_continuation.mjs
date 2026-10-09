import assert from "node:assert/strict";
import test from "node:test";
import { grantAttempts, readAttemptGrant, rejectLegacyContinue, applyAllowanceGrant } from "../.sandcastle/continuation.mjs";
import { resumeUserPause } from "../.sandcastle/limits.mjs";

function paused(kind) {
  return { phase: "awaiting", resumePhase: { build: "build-repair", test: "test-repair", review: "review" }[kind], pauseReason: { build: "Build failed 10 times; continuation requires human approval.\nlog", test: "Tests failed 10 times; continuation requires human approval.\nlog", review: "Acceptance: review round limit reached; human approval required" }[kind], budget: { consumedPercent: 12, tokens: 123, limitPercent: 25, usageHistory: [{ id: "usage" }] }, buildLimit: 10, testLimit: 10, reviewLimit: 10, buildAttempts: 10, testBuildAttempts: 4, testAttempts: 10, totalBuildAttempts: 14, totalTestAttempts: 10, reviewRound: 10, recoveryAttempts: { saved: 3 }, completedTickets: ["ticket"], acceptance: { contract: { id: "contract" } } };
}

for (const kind of ["build", "test", "review"]) test(`${kind} attempt grant extends only the selected allowance and preserves progress`, () => {
  const state = paused(kind);
  const before = structuredClone(state);
  grantAttempts(state, kind, 3, { authorizationId: `${kind}-extra` });
  const expected = structuredClone(before);
  expected[`${kind}Limit`] += 3;
  expected.phase = expected.resumePhase;
  delete expected.pauseReason;
  delete expected.resumePhase;
  expected.attemptAllowanceHistory = state.attemptAllowanceHistory;
  assert.deepEqual(state, expected);
  assert.equal(state.attemptAllowanceHistory[0].id, `${kind}-extra`);
  assert.equal(state.attemptAllowanceHistory[0].extraAttempts, 3);
  assert.throws(() => grantAttempts(state, kind, 1), /saved.*limit pause/);
});

test("test-gate rebuild gets build allowance without resetting its counter", () => {
  const state = paused("build");
  state.resumePhase = "build-repair";
  state.pauseReason = "Test-gate rebuild failed 10 times; continuation requires human approval.";
  state.buildAttempts = 2;
  state.testBuildAttempts = 10;
  grantAttempts(state, "build", 4);
  assert.equal(state.buildLimit, 14);
  assert.equal(state.buildAttempts, 2);
  assert.equal(state.testBuildAttempts, 10);
});

test("legacy overshoot grants the requested attempts beyond usage without rewriting counters", () => {
  const state = paused("build");
  state.buildAttempts = 12;
  grantAttempts(state, "build", 3);
  assert.equal(state.buildLimit, 15);
  assert.equal(state.buildAttempts, 12);
  assert.equal(state.totalBuildAttempts, 14);
  assert.equal(state.attemptAllowanceHistory[0].extraAttempts, 3);
  assert.equal(state.attemptAllowanceHistory[0].allowanceDelta, 5);
});

test("attempt grants cannot unlock quota, user pauses, other guards, or nonmatching limits", () => {
  for (const mutate of [s => s.pauseReason = "Task reached 25% of the weekly Codex quota", s => s.pauseReason = "Paused by user", s => s.pauseReason = "Git merge conflict", s => s.buildAttempts = 3, s => s.testBuildAttempts = "invalid", s => s.budget.consumedPercent = 25, s => s.quotaError = "Weekly Codex quota is unavailable", s => s.resumePhase = "integrate"]) {
    const state = paused("build");
    mutate(state);
    const before = structuredClone(state);
    assert.throws(() => grantAttempts(state, "build", 2));
    assert.deepEqual(state, before);
  }
  const state = paused("build");
  assert.throws(() => grantAttempts(state, "test", 2), /saved.*limit pause/);
});

test("environment grants are independent and strictly validated", () => {
  assert.equal(readAttemptGrant({}), undefined);
  assert.deepEqual(readAttemptGrant({ SANDCASTLE_EXTRA_TEST_ATTEMPTS: "4" }), { kind: "test", extra: 4 });
  for (const value of ["", "0", "-1", "1.5", "abc", "101", "1e1"]) assert.throws(() => readAttemptGrant({ SANDCASTLE_EXTRA_BUILD_ATTEMPTS: value }), /EXTRA_BUILD_ATTEMPTS/);
  assert.throws(() => readAttemptGrant({ SANDCASTLE_EXTRA_BUILD_ATTEMPTS: "2", SANDCASTLE_EXTRA_REVIEW_ATTEMPTS: "2" }), /one attempt allowance/);
  for (const value of [0, -1, NaN, Infinity, 101, 1.5]) {
    const state = paused("test");
    const before = structuredClone(state);
    assert.throws(() => grantAttempts(state, "test", value));
    assert.deepEqual(state, before);
  }
});

test("legacy CONTINUE fails before mutation and names explicit alternatives", () => {
  assert.throws(() => rejectLegacyContinue({ SANDCASTLE_CONTINUE: "1" }), /deprecated.*SANDCASTLE_EXTRA_QUOTA_PERCENT.*SANDCASTLE_EXTRA_BUILD_ATTEMPTS/);
  assert.doesNotThrow(() => rejectLegacyContinue({}));
});

test("normal saved user resume grants nothing and preserves exhausted recovery", () => {
  const state = paused("build");
  state.pauseReason = "Paused by user";
  state.userRequestedPause = true;
  const before = structuredClone(state);
  resumeUserPause(state);
  const expected = structuredClone(before);
  expected.phase = expected.resumePhase;
  delete expected.resumePhase;
  delete expected.pauseReason;
  delete expected.userRequestedPause;
  assert.deepEqual(state, expected);
});

for (const first of ["quota", "build"]) test(`combined quota and build exhaustion grants ${first} first without authorizing its sibling`, () => {
  const state = paused("build");
  state.budget.consumedPercent = 25;
  const before = structuredClone(state);
  applyAllowanceGrant(state, { kind: first, extra: 5, authorizationId: first });
  assert.equal(state.phase, "awaiting");
  assert.equal(state.resumePhase, "build-repair");
  const remaining = first === "quota" ? "build" : "quota";
  assert.deepEqual(state.allowanceContinuation.remainingGuards.map(g => g.kind), [remaining]);
  assert.equal(state.buildAttempts, before.buildAttempts);
  assert.equal(state.budget.consumedPercent, 25);
  assert.equal(state.budget.tokens, 123);
  assert.deepEqual(state.recoveryAttempts, before.recoveryAttempts);
  const savedHistory = structuredClone(first === "quota" ? state.budget.allowanceHistory : state.attemptAllowanceHistory);
  applyAllowanceGrant(state, { kind: remaining, extra: 3, authorizationId: remaining });
  assert.equal(state.phase, "build-repair");
  assert.equal(state.resumePhase, undefined);
  assert.equal(state.pauseReason, undefined);
  assert.equal(state.allowanceContinuationHistory[0].originalPauseReason, before.pauseReason);
  assert.deepEqual(first === "quota" ? state.budget.allowanceHistory : state.attemptAllowanceHistory, savedHistory);
  for (const key of ["buildAttempts", "testAttempts", "totalBuildAttempts", "totalTestAttempts", "completedTickets", "acceptance", "recoveryAttempts"]) assert.deepEqual(state[key], before[key]);
});

test("quota-labelled build pause retains independent build guard after quota grant", () => {
  const state = paused("build");
  state.budget.consumedPercent = 25;
  state.pauseReason = state.quotaError = "Task reached 25% of the weekly Codex quota";
  applyAllowanceGrant(state, { kind: "quota", extra: 10 });
  assert.equal(state.phase, "awaiting");
  assert.equal(state.quotaError, undefined);
  assert.match(state.pauseReason, /SANDCASTLE_EXTRA_BUILD_ATTEMPTS/);
  assert.equal(state.buildLimit, 10);
});

test("test-gate with quota, rebuild and test exhaustion requires three independent grants", () => {
  const state = paused("test");
  state.resumePhase = "test-gate";
  state.testBuildAttempts = 10;
  state.budget.consumedPercent = 25;
  applyAllowanceGrant(state, { kind: "test", extra: 2 });
  assert.equal(state.phase, "awaiting");
  applyAllowanceGrant(state, { kind: "quota", extra: 2 });
  assert.equal(state.phase, "awaiting");
  applyAllowanceGrant(state, { kind: "build", extra: 2 });
  assert.equal(state.phase, "test-gate");
  assert.deepEqual([state.buildLimit, state.testLimit], [12, 12]);
});

test("unified grants reject unrelated pauses, account/token stops, duplicate authorization and malformed requests atomically", () => {
  for (const mutate of [s => s.pauseReason = "Git conflicts", s => s.quotaError = "Codex account usage limit reached", s => s.quotaError = "Task reached its token limit (100)", s => s.quotaError = "Weekly Codex quota is unavailable", s => s.phase = "done"]) {
    const state = paused("build");
    state.budget.consumedPercent = 25;
    mutate(state);
    const before = structuredClone(state);
    assert.throws(() => applyAllowanceGrant(state, { kind: "quota", extra: 3 }));
    assert.deepEqual(state, before);
  }
  const state = paused("build");
  state.budget.consumedPercent = 25;
  applyAllowanceGrant(state, { kind: "quota", extra: 3, authorizationId: "once" });
  for (const grant of [{ kind: "build", extra: 3, authorizationId: "once" }, { kind: "build", extra: 0 }, { kind: "test", extra: 3 }]) {
    const before = structuredClone(state);
    assert.throws(() => applyAllowanceGrant(state, grant));
    assert.deepEqual(state, before);
  }
});

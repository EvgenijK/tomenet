import assert from "node:assert/strict";
import test from "node:test";
import { classifyFailure, pauseStatus, clearCurrentFailure } from "../.sandcastle/failure.mjs";

test("failure classification is deterministic and secret-free", () => {
  for (const [message, category] of [
    ["Task reached 81% of the weekly Codex quota", "quota"],
    ["Weekly Codex quota is unavailable", "quota"],
    ["Model gpt-6.1-sol is not supported password=do-not-publish", "model"],
    ["Worker preflight failed TOKEN=do-not-publish", "startup"],
    ["schema mount /private/secret", "schema"],
    ["Acceptance: invalid criterion", "contract"],
    ["Test attempt limit reached", "test"],
    ["Main checkout is dirty Bearer do-not-publish", "git"],
    ["Something do-not-publish", "other"],
  ]) {
    const result = classifyFailure(new Error(message));
    assert.equal(result.category, category);
    assert.ok(!JSON.stringify(result).includes("do-not-publish"));
    assert.ok(!JSON.stringify(result).includes("/private"));
  }
  for (const message of ["unsupported model", "Worker preflight failed", "schema mount failure"]) {
    assert.equal(classifyFailure(message).fatalPreflight, true);
    assert.equal(classifyFailure(message).automaticRetry, false);
  }
});

test("pause guidance does not grant permission or provide universal CONTINUE", () => {
  const quota = pauseStatus({ phase: "awaiting", resumePhase: "parallel-work", pauseReason: "Task reached 81% of the weekly Codex quota" });
  assert.equal(quota.resume.requiresAuthorization, true);
  assert.deepEqual(quota.resume.environment, { SANDCASTLE_EXTRA_QUOTA_PERCENT: "<explicit-positive-allowance>" });
  const user = pauseStatus({ phase: "awaiting", resumePhase: "plan", pauseReason: "Paused by user", userRequestedPause: true });
  assert.deepEqual(user.resume.environment, { SANDCASTLE_RESUME: "1" });
  assert.ok(!JSON.stringify(quota).includes("CONTINUE"));
  assert.deepEqual(pauseStatus({ phase: "awaiting", resumePhase: "test-gate", pauseReason: "Test attempt limit reached" }).resume.environment, { SANDCASTLE_EXTRA_TEST_ATTEMPTS: "<explicit-positive-attempts>" });
  assert.equal(classifyFailure({ deterministic: true, code: "BRANCH_MISMATCH", message: "Execution preflight: branch mismatch" }).automaticRetry, false);
  const preflight = classifyFailure("Execution preflight [CLI_TOO_OLD]: private details do-not-publish");
  assert.equal(preflight.diagnosticCode, "CLI_TOO_OLD");
  assert.equal(preflight.fatalPreflight, true);
  assert.equal(pauseStatus({ phase: "done", pauseReason: "Old failure" }), undefined);
});

test("verified progress clears current pointers without erasing history", () => {
  const state = { phase: "done", pendingFailure: "Old build output", currentFailure: { code: "test" }, pauseReason: "Old pause", recoveries: [{ error: "history" }], acceptance: { assessments: ["history"] } };
  assert.throws(() => clearCurrentFailure(state, { stage: "build" }), /verified/);
  assert.equal(state.pendingFailure, "Old build output");
  clearCurrentFailure(state, { stage: "integrate", verified: true });
  assert.equal(state.pendingFailure, undefined);
  assert.equal(state.pauseReason, undefined);
  assert.deepEqual(state.recoveries, [{ error: "history" }]);
  assert.deepEqual(state.acceptance.assessments, ["history"]);
});

test("typed preflight diagnostics preserve exact safe repair guidance through pause persistence", () => {
  const failure = classifyFailure({ code: "CLI_TOO_OLD", deterministic: true, message: "raw-secret-provider-output" });
  assert.match(failure.requiredAction, /Upgrade.*CLI/);
  const paused = pauseStatus({ phase: "awaiting", resumePhase: "parallel-work", pauseReason: "Opaque worker failed status 1 raw-secret-provider-output", currentFailure: { ...failure, reason: "raw-secret-provider-output", requiredAction: "raw-secret-provider-output" } });
  assert.equal(paused.diagnosticCode, "CLI_TOO_OLD");
  assert.equal(paused.category, "startup");
  assert.equal(paused.automaticRetry, false);
  assert.deepEqual(paused.resume.environment, { SANDCASTLE_PREFLIGHT_RESUME: "1" });
  assert.equal(paused.resume.requiresVerification, true);
  assert.equal(paused.resume.requiresAuthorization, false);
  assert.ok(!JSON.stringify(paused).includes("raw-secret-provider-output"));
  const cases = { BASE_MISMATCH: "git", SCHEMA_UNREADABLE: "schema", AGENT_POLICY_INVALID: "model", CONTRACT_DIGEST_INVALID: "contract", SANDBOX_CHECKS_MISMATCH: "startup", CONTROLLER_CHANGED: "startup" };
  for (const [code, category] of Object.entries(cases)) {
    const item = classifyFailure({ code, deterministic: true });
    assert.equal(item.category, category);
    assert.equal(item.diagnosticCode, code);
    assert.equal(item.fatalPreflight, true);
  }
});

test("partial allowance continuation exposes all remaining guards without granting another", () => {
  const paused = pauseStatus({ phase: "awaiting", resumePhase: "build", pauseReason: "build attempt limit reached; SANDCASTLE_EXTRA_BUILD_ATTEMPTS required", currentFailure: classifyFailure("Task quota limit reached"), allowanceContinuation: { remainingGuards: [{ kind: "build", reason: "raw-secret-output" }, { kind: "test", reason: "raw-secret-output" }, { kind: "review", reason: "raw-secret-output" }] } });
  assert.equal(paused.reason, "Build attempt allowance is exhausted.");
  assert.deepEqual(paused.resume.environment, { SANDCASTLE_EXTRA_BUILD_ATTEMPTS: "<explicit-positive-attempts>" });
  assert.deepEqual(paused.remainingGuards.map((guard) => guard.kind), ["build", "test", "review"]);
  assert.ok(!JSON.stringify(paused).includes("raw-secret-output"));
  assert.equal(pauseStatus({ phase: "awaiting", resumePhase: "review", pauseReason: "review attempt limit reached; SANDCASTLE_EXTRA_REVIEW_ATTEMPTS required" }).resume.environment.SANDCASTLE_EXTRA_REVIEW_ATTEMPTS, "<explicit-positive-attempts>");
  assert.equal(classifyFailure("Task quota limit reached; SANDCASTLE_EXTRA_QUOTA_PERCENT required").category, "quota");
});

test("prepared delivery migration names its separate resume authorization without granting quota", () => {
  const state={phase:'awaiting',resumePhase:'assemble',verificationMigration:{version:1},pauseReason:'Historical controller-worker deadlock'};
  assert.deepEqual(pauseStatus(state).resume.environment,{SANDCASTLE_VERIFICATION_RESUME:'1'});
  assert.equal(pauseStatus(state).resume.requiresAuthorization,true);
  state.quotaError='Task reached 25% of the weekly Codex quota';
  assert.notDeepEqual(pauseStatus(state).resume.environment,{SANDCASTLE_VERIFICATION_RESUME:'1'});
});

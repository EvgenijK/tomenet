import assert from "node:assert/strict";
import test from "node:test";
import { resumeWorkerRuntimeUpgrade } from "../.sandcastle/worker-runtime.mjs";

function checkpoint() {
  return { phase: "awaiting", resumePhase: "parallel-work", pauseReason: "Automatic recovery made no progress after three decisions at this checkpoint",
    agentPolicy: { model: "gpt-6.1-sol", orchestratorEffort: "high", agentEffort: "medium" },
    budget: { consumedPercent: 82, limitPercent: 91, tokens: 123 }, completedTickets: ["previous.md"],
    recoveryAttempts: { checkpoint: 3 }, recoveries: [{ id: 15 }], reviewRound: 40, buildAttempts: 1, testAttempts: 0,
    acceptance: { contractDigest: "digest" }, wave: { id: 66, baseCommit: "base", members: [{ branch: "worker", status: "failed",
      error: 'worker agent exited: {"type":"error","status":400,"error":{"message":"The \'gpt-6.1-sol\' model is not supported when using Codex with a ChatGPT account."}}' }] } };
}
const proof = { version: "0.161.0", imageId: "sha256:" + "a".repeat(64) };
test("verified worker CLI upgrade retries the saved wave once without resetting exhausted recovery or quota", () => {
  const s = checkpoint(), before = structuredClone(s);
  resumeWorkerRuntimeUpgrade(s, proof);
  assert.equal(s.phase, "parallel-work");
  assert.equal(s.resumePhase, undefined);
  for (const key of ["budget", "completedTickets", "recoveryAttempts", "recoveries", "reviewRound", "buildAttempts", "testAttempts", "acceptance", "wave", "agentPolicy"]) assert.deepEqual(s[key], before[key]);
  assert.equal(s.workerRuntimeUpgradeResumes[0].waveId, 66);
  s.phase = before.phase; s.resumePhase = before.resumePhase; s.pauseReason = before.pauseReason;
  assert.throws(() => resumeWorkerRuntimeUpgrade(s, proof), /saved unsupported-model/);
});
test("upgrade cannot bypass unrelated failures, successful workers, quotas or unverified image versions", () => {
  for (const alter of [s => s.wave.members[0].error = "ordinary code defect", s => s.wave.members[0].head = "commit", s => s.budget.consumedPercent = 91,
    s => s.agentPolicy.model = "other", s => s.phase = "recovery"]) {
    const s = checkpoint(); alter(s); const before = structuredClone(s);
    assert.throws(() => resumeWorkerRuntimeUpgrade(s, proof)); assert.deepEqual(s, before);
  }
  for (const invalid of [{ ...proof, version: "0.157.1" }, { ...proof, imageId: "unknown" }]) assert.throws(() => resumeWorkerRuntimeUpgrade(checkpoint(), invalid));
});

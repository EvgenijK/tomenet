import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { randomBytes } from "node:crypto";
import { mkdirSync, writeFileSync, readFileSync, rmSync, existsSync } from "node:fs";
import { resolve } from "node:path";
import test from "node:test";
import { resumeUserPause } from "../.sandcastle/limits.mjs";

test("explicit user-pause resume preserves budgets, wave and reserved contract attempt", () => {
  const s = { phase: "awaiting", resumePhase: "contract", pauseReason: "Paused by user", userRequestedPause: true,
    budget: { consumedPercent: 38, limitPercent: 40, tokens: 123 }, reviewRound: 13, reviewLimit: 30, recoveryAttempts: { checkpoint: 2 },
    contractPending: { round: 1 }, wave: { id: 39, members: [{ status: "completed", integrated: false }] } };
  const before = structuredClone(s); resumeUserPause(s);
  assert.equal(s.phase, "contract"); assert.deepEqual(s.budget, before.budget); assert.deepEqual(s.wave, before.wave);
  assert.deepEqual(s.recoveryAttempts, before.recoveryAttempts); assert.deepEqual(s.contractPending, before.contractPending); assert.equal(s.reviewLimit, 30);
  assert.equal(s.userRequestedPause, undefined);
  for (const mutate of [s => { s.pauseReason = "Build failed"; }, s => { s.userRequestedPause = false; }, s => { s.budget.consumedPercent = 40; }, s => { s.quotaError = "Account usage limit"; }]) {
    const invalid = structuredClone(before); mutate(invalid); const snapshot = structuredClone(invalid);
    assert.throws(() => resumeUserPause(invalid)); assert.deepEqual(invalid, snapshot);
  }
});

test("unconfirmed CLI resume preserves the original pause and checkpoint without acquiring a lock", () => {
  const root = resolve(import.meta.dirname, "..");
  const id = randomBytes(4).toString("hex");
  const dir = resolve(root, ".sandcastle/runs", id);
  const text = JSON.stringify({ id, branch: `codex/sandcastle-test-${id}`, phase: "awaiting", resumePhase: "assemble", pauseReason: "A worker is blocked", completedTickets: [], budget: { consumedPercent: 5, tokens: 123 } });
  mkdirSync(dir, { recursive: true });
  writeFileSync(resolve(dir, "state.json"), text);
  const env = { ...process.env };
  delete env.SANDCASTLE_CONTINUE;
  delete env.SANDCASTLE_RECOVER;
  delete env.SANDCASTLE_EXTRA_QUOTA_PERCENT;
  delete env.SANDCASTLE_RESUME;
  delete env.SANDCASTLE_REPAIR_RESUME;
  delete env.SANDCASTLE_EXTRA_CONTRACT_ROUND;
  try {
    assert.throws(() => execFileSync(process.execPath, [".sandcastle/workflow.mjs", "resume", id], { cwd: root, env, stdio: ["ignore", "pipe", "pipe"] }), (error) => {
      assert.equal(error.status, 1);
      assert.match(error.stderr.toString(), /needs human confirmation/);
      return true;
    });
    assert.equal(readFileSync(resolve(dir, "state.json"), "utf8"), text);
    assert.equal(existsSync(resolve(dir, "run.lock")), false);
    assert.throws(() => execFileSync(process.execPath, [".sandcastle/workflow.mjs", "resume", id], { cwd: root, env: { ...env, SANDCASTLE_EXTRA_CONTRACT_ROUND: "1" }, stdio: ["ignore", "pipe", "pipe"] }), (error) => {
      assert.match(error.stderr.toString(), /extra contract-round grants are no longer available/);
      return true;
    });
    assert.equal(readFileSync(resolve(dir, "state.json"), "utf8"), text);
    assert.equal(existsSync(resolve(dir, "run.lock")), false);
    assert.throws(() => execFileSync(process.execPath, [".sandcastle/workflow.mjs", "resume", id], { cwd: root, env: { ...env, SANDCASTLE_REPAIR_RESUME: "1" }, stdio: ["ignore", "pipe", "pipe"] }), (error) => {
      assert.match(error.stderr.toString(), /saved exhausted legacy checkpoint/);
      return true;
    });
    assert.equal(readFileSync(resolve(dir, "state.json"), "utf8"), text);
    assert.equal(existsSync(resolve(dir, "run.lock")), false);
  } finally { rmSync(dir, { recursive: true, force: true }); }
});

import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { randomBytes } from "node:crypto";
import { mkdirSync, writeFileSync, readFileSync, rmSync, existsSync } from "node:fs";
import { resolve } from "node:path";
import test from "node:test";
import { resumeUserPause } from "../.sandcastle/limits.mjs";
import { acceptContract, baselineChecks, digest } from "../.sandcastle/acceptance.mjs";
import { maxContractAmendments, resumeExternalContractAmendment } from "../.sandcastle/contract-amend.mjs";

const head = "a".repeat(40);
const sources = { "spec.md": "Implementation behavior. Native acceptance later." };
function externalPause(overrides = {}) {
  const contract = { version: 1, completionScope: "implementation", checks: [
    ...structuredClone(baselineChecks),
    { id: "implementation-review", kind: "external", command: "", description: "Historical unrunnable review" },
    { id: "native", kind: "external", command: "", description: "Deferred native acceptance" },
  ], criteria: [
    { id: "AC-1", requirement: "Implementation behavior", mandatory: true, applicability: "current", owner: "", deferralReason: "", checkIds: ["sv-core", "implementation-review"], source: { path: "spec.md", quote: "Implementation behavior." } },
    { id: "AC-2", requirement: "Native acceptance", mandatory: true, applicability: "deferred", owner: "native owner", deferralReason: "Later platform environment", checkIds: ["native"], source: { path: "spec.md", quote: "Native acceptance later." } },
  ] };
  const state = { id: "amend", phase: "awaiting", resumePhase: "test-gate", pauseReason: "Acceptance: required external check implementation-review has no runner; human decision required",
    completedTickets: ["one.md"], reviews: [{ round: 1 }], recoveries: [{ id: 2 }], budget: { consumedPercent: 7, tokens: 99 }, buildAttempts: 4, testAttempts: 2 };
  // This fixture represents a contract accepted before the stricter validation
  // rule, so construct its immutable acceptance record directly through the
  // historical shape instead of asking today's validator to accept it.
  state.acceptance = {
    contract, contractDigest: digest(contract), sourceDigests: { "spec.md": "old-source" }, acceptedHead: head,
    contractReview: { approved: true, summary: "Historical review", missingRequirements: [] },
    policy: {}, ledger: [], repairBatches: [], evidence: [{ checkId: "sv-core", contractDigest: "old" }], assessments: [], finalAudits: [],
  };
  return Object.assign(state, overrides);
}

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

test("external-check contract amendment archives the old decision and preserves implementation progress", () => {
  const state = externalPause();
  const preserved = {
    completedTickets: structuredClone(state.completedTickets), reviews: structuredClone(state.reviews),
    recoveries: structuredClone(state.recoveries), budget: structuredClone(state.budget),
    buildAttempts: state.buildAttempts, testAttempts: state.testAttempts,
  };
  const old = structuredClone(state.acceptance);
  resumeExternalContractAmendment(state);
  assert.equal(state.phase, "contract");
  assert.equal(state.contractReturnPhase, "test-gate");
  assert.equal(state.resumePhase, undefined);
  assert.equal(state.pauseReason, undefined);
  assert.equal(state.acceptance, undefined);
  assert.equal(state.contractPending, undefined);
  assert.deepEqual(state.contractAmendments, [{
    reason: "Acceptance: required external check implementation-review has no runner; human decision required",
    checkId: "implementation-review", contract: old.contract, contractDigest: old.contractDigest,
    contractReview: old.contractReview, sourceDigests: old.sourceDigests, acceptedHead: old.acceptedHead,
  }]);
  for (const [field, value] of Object.entries(preserved)) assert.deepEqual(state[field], value);
  const amended = structuredClone(old.contract);
  amended.criteria[0].checkIds = ["sv-core"];
  acceptContract(state, amended, sources, old.contractReview, head);
  assert.deepEqual(state.acceptance.evidence, []);
  assert.notEqual(state.acceptance.contractDigest, old.contractDigest);
  assert.equal(state.contractAmendments[0].contractDigest, old.contractDigest);
});

test("external-check contract amendment is bounded to its exact saved pipeline defect", () => {
  for (const mutate of [
    (state) => { state.resumePhase = "review"; },
    (state) => { state.pauseReason = "Acceptance: required check failed"; },
    (state) => { state.acceptance.contract.completionScope = "full_acceptance"; state.acceptance.contractDigest = digest(state.acceptance.contract); },
    (state) => { state.contractAmendments = Array.from({ length: maxContractAmendments }, () => ({})); },
  ]) {
    const state = externalPause(); mutate(state); const before = structuredClone(state);
    assert.throws(() => resumeExternalContractAmendment(state), /Contract amend RESUME/);
    assert.deepEqual(state, before);
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
  delete env.SANDCASTLE_CONTRACT_AMEND_RESUME;
  delete env.SANDCASTLE_EXTRA_CONTRACT_ROUND;
  delete env.SANDCASTLE_EXTRA_CONTRACT_ATTEMPTS;
  try {
    assert.throws(() => execFileSync(process.execPath, [".sandcastle/workflow.mjs", "resume", id], { cwd: root, env, stdio: ["ignore", "pipe", "pipe"] }), (error) => {
      assert.equal(error.status, 1);
      assert.match(error.stderr.toString(), /needs human confirmation/);
      return true;
    });
    assert.equal(readFileSync(resolve(dir, "state.json"), "utf8"), text);
    assert.equal(existsSync(resolve(dir, "run.lock")), false);
    assert.throws(() => execFileSync(process.execPath, [".sandcastle/workflow.mjs", "resume", id], { cwd: root, env: { ...env, SANDCASTLE_EXTRA_CONTRACT_ATTEMPTS: "5" }, stdio: ["ignore", "pipe", "pipe"] }), (error) => {
      assert.match(error.stderr.toString(), /one rejected group/);
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

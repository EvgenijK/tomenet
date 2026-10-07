import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { randomBytes } from "node:crypto";
import { mkdirSync, writeFileSync, readFileSync, rmSync, existsSync } from "node:fs";
import { resolve } from "node:path";
import test from "node:test";
import { resumeUserPause } from "../.sandcastle/limits.mjs";
import { baselineChecks, digest, validateContract } from "../.sandcastle/acceptance.mjs";
import { maxContractAmendments, maxContractAmendmentRetryAttempts, resumeExternalContractAmendment, resumeContractAmendmentRetry, validateContractAmendment } from "../.sandcastle/contract-amend.mjs";

const head = "a".repeat(40);
function externalPause(overrides = {}) {
  const contract = { version: 1, completionScope: "implementation", checks: [
    ...structuredClone(baselineChecks),
    { id: "P1-dep-runtime", kind: "external", command: "", description: "Duplicate login overview gate" },
    { id: "runtime", kind: "external", command: "", description: "Historical unrunnable production check" },
  ], criteria: [
    { id: "P1-AC1", requirement: "Use the server overview", mandatory: true, applicability: "current", owner: "SV-B-022", deferralReason: "", checkIds: ["sv-build", "sv-core", "P1-dep-runtime"], source: { path: "spec.md", quote: "Use the server overview." } },
    { id: "P1-AC2", requirement: "Preserve production behavior", mandatory: true, applicability: "current", owner: "SV-B-022", deferralReason: "", checkIds: ["sv-core", "runtime"], source: { path: "spec.md", quote: "Preserve production behavior." } },
  ] };
  const acceptance = { contract, contractDigest: digest(contract), sourceDigests: { "spec.md": "old-source" }, acceptedHead: head,
    contractReview: { approved: true, summary: "Historical review", missingRequirements: [] }, policy: {}, ledger: [], repairBatches: [],
    evidence: [{ checkId: "sv-core", head, passed: true, logPath: "/tmp/core.json", contractDigest: digest(contract) }], assessments: [], finalAudits: [] };
  return Object.assign({ id: "amend", phase: "awaiting", resumePhase: "test-gate",
    pauseReason: "Acceptance: required external check P1-dep-runtime has no runner; human decision required",
    acceptance, completedTickets: ["one.md"], deferredTickets: [{ path: "later.md" }], reviews: [{ round: 1 }],
    recoveries: [{ id: 2 }], budget: { consumedPercent: 7, tokens: 99 }, buildAttempts: 4, testAttempts: 2 }, overrides);
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

test("external-check amendment archives contract, digest and evidence while preserving progress", () => {
  const state = externalPause();
  const preserved = Object.fromEntries(["completedTickets", "deferredTickets", "reviews", "recoveries", "budget", "buildAttempts", "testAttempts"].map((field) => [field, structuredClone(state[field])]));
  const oldAcceptance = structuredClone(state.acceptance);
  resumeExternalContractAmendment(state);
  assert.equal(state.phase, "contract");
  assert.equal(state.contractReturnPhase, "test-gate");
  assert.equal(state.acceptance, undefined);
  assert.deepEqual(state.contractAmendments[0].acceptance, oldAcceptance);
  assert.deepEqual(state.contractAmendments[0].evidence, oldAcceptance.evidence);
  assert.deepEqual(state.contractAmendment.requiredExternalCheckIds, ["P1-dep-runtime", "runtime"]);
  for (const [field, value] of Object.entries(preserved)) assert.deepEqual(state[field], value);
});

test("amendment removes P1-dep-runtime, remaps every current external check, and forbids waivers", () => {
  const state = externalPause();
  const previous = structuredClone(state.acceptance.contract);
  resumeExternalContractAmendment(state);
  const amended = structuredClone(previous);
  amended.checks = amended.checks.filter((check) => !["P1-dep-runtime", "runtime"].includes(check.id));
  amended.criteria[0].checkIds = ["sv-build", "sv-core"];
  amended.criteria[1].checkIds = ["sv-core"];
  const sources = { "spec.md": "Use the server overview. Preserve production behavior." };
  assert.doesNotThrow(() => validateContract(amended, sources));
  assert.doesNotThrow(() => validateContractAmendment(previous, amended, state.contractAmendment));
  for (const mutate of [
    (contract) => { contract.criteria[0].mandatory = false; },
    (contract) => { contract.criteria[0].applicability = "deferred"; contract.criteria[0].owner = "later"; contract.criteria[0].deferralReason = "later"; },
    (contract) => { contract.criteria.shift(); },
    (contract) => { contract.criteria[0].checkIds = ["sv-core"]; },
    (contract) => { contract.checks.push(previous.checks.find((check) => check.id === "P1-dep-runtime")); },
  ]) {
    const invalid = structuredClone(amended); mutate(invalid);
    assert.throws(() => validateContractAmendment(previous, invalid, state.contractAmendment), /Contract amend RESUME/);
  }
});

test("external-check amendment is one-shot and exact to a saved test-gate pause", () => {
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

test("identical-feedback amendment retry requires a unique bounded authorization and preserves history", () => {
  const state = externalPause();
  resumeExternalContractAmendment(state);
  const ledger = [{ id: "CF-123", message: "Missing runner", normalizedMessage: "missing runner", sourceSpanIds: [], status: "open", firstRound: 1, lastSeenRound: 4, occurrences: 3 }];
  state.phase = "awaiting";
  state.resumePhase = "contract";
  state.pauseReason = "Acceptance: contract feedback made no progress for 3 identical reviews; human decision required";
  state.contractPending = { round: 4, sourceManifest: { digest: "sources" }, feedbackLedger: ledger, convergence: { openDigest: "same", identicalOpenRounds: 3, lastRound: 4 } };
  const preserved = { budget: structuredClone(state.budget), completedTickets: structuredClone(state.completedTickets), archive: structuredClone(state.contractAmendments) };
  resumeContractAmendmentRetry(state, "svb022-amend-inventory-20261007");
  assert.equal(state.phase, "contract");
  assert.equal(state.contractAmendmentAttemptLimit, maxContractAmendmentRetryAttempts);
  assert.equal(state.contractPending.round, 0);
  assert.equal(state.contractPending.convergence, undefined);
  assert.deepEqual(state.contractPending.feedbackLedger, ledger);
  assert.equal(state.contractAmendmentRetryHistory[0].pending.round, 4);
  assert.deepEqual(state.budget, preserved.budget);
  assert.deepEqual(state.completedTickets, preserved.completedTickets);
  assert.deepEqual(state.contractAmendments, preserved.archive);
  state.phase = "awaiting"; state.resumePhase = "contract";
  state.pauseReason = "Acceptance: contract feedback made no progress for 3 identical reviews; human decision required";
  state.contractPending.convergence = { openDigest: "same", identicalOpenRounds: 3, lastRound: 3 };
  assert.throws(() => resumeContractAmendmentRetry(state, "another-id"), /authorization limit reached/);
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
  delete env.SANDCASTLE_CONTRACT_AMEND_RETRY_ID;
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

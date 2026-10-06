import assert from "node:assert/strict";
import test from "node:test";
import { acceptContract, baselineChecks, digest } from "../.sandcastle/acceptance.mjs";
import { commandContractRepairStage, resumeCommandContractRepair, validateCommandOnlyCorrection } from "../.sandcastle/contract-command-repair.mjs";

const head = "a".repeat(40);
const sources = { "spec.md": "Account creation follows the production path." };
const review = { approved: true, summary: "Independent command correction verification passed", missingRequirements: [] };
function contracts() {
  const previous = { version: 1, completionScope: "implementation", checks: [
    ...structuredClone(baselineChecks),
    { id: "account", kind: "command", command: "python3 -B tests/test_sv_account_create.py", description: "Account production checks" },
  ], criteria: [{
    id: "AC-1", requirement: "Production account creation", mandatory: true, applicability: "current",
    owner: "", deferralReason: "", checkIds: ["account"], source: { path: "spec.md", quote: "Account creation follows the production path." },
  }] };
  const replacement = structuredClone(previous);
  replacement.checks[2].command = ["python3 -B tests/sv_account_create_checks.py", "python3 -B tests/sv_account_failure_checks.py"];
  return { previous, replacement };
}
function pausedState() {
  const { previous } = contracts();
  const state = { id: "repair", phase: "awaiting", resumePhase: "tickets", pauseReason: "Acceptance: committed contract changed; human decision required",
    completedTickets: ["one.md"], tickets: ["one.md", "two.md"], reviews: [{ round: 1 }], budget: { consumedPercent: 12, tokens: 10 }, buildAttempts: 5, testAttempts: 3,
    contractAmendments: [{ contractDigest: "older" }] };
  acceptContract(state, previous, sources, review, head);
  state.acceptance.evidence.push({ checkId: "sv-core", head, passed: true, logPath: "/tmp/old", contractDigest: state.acceptance.contractDigest });
  return state;
}
function ops(replacement, overrides = {}) {
  return {
    sources: async () => sources,
    artifact: async () => ({ contract: replacement, digest: digest(replacement), verification: review }),
    missingRunners: async (contract) => contract.checks[2].command === "python3 -B tests/test_sv_account_create.py"
      ? [{ checkId: "account", command: contract.checks[2].command, runner: "tests/test_sv_account_create.py" }] : [],
    verify: async () => review,
    head: async () => head,
    publish: async () => {},
    save: async () => {},
    ...overrides,
  };
}

test("bounded command repair independently reaccepts only the command correction", async () => {
  const state = pausedState();
  const preserved = Object.fromEntries(["completedTickets", "tickets", "reviews", "budget", "buildAttempts", "testAttempts"].map((key) => [key, structuredClone(state[key])]));
  const oldDigest = state.acceptance.contractDigest;
  const { replacement } = contracts();
  let verified = 0; let published;
  resumeCommandContractRepair(state);
  await commandContractRepairStage(state, ops(replacement, {
    verify: async () => { verified++; return review; },
    publish: async (acceptance) => { published = structuredClone(acceptance); },
  }));
  assert.equal(verified, 1);
  assert.equal(state.phase, "tickets");
  assert.equal(state.acceptance.contractDigest, digest(replacement));
  assert.deepEqual(state.acceptance.evidence, []);
  assert.deepEqual(published, state.acceptance);
  assert.equal(state.contractAmendments.length, 2);
  assert.equal(state.contractAmendments[0].contractDigest, "older");
  assert.equal(state.contractAmendments[1].kind, "command-only");
  assert.equal(state.contractAmendments[1].contractDigest, oldDigest);
  assert.equal(state.contractAmendments[1].replacementDigest, digest(replacement));
  for (const [key, value] of Object.entries(preserved)) assert.deepEqual(state[key], value);
});

test("command repair rejects scope, criteria, check metadata and multi-check changes", () => {
  const { previous, replacement } = contracts();
  assert.equal(validateCommandOnlyCorrection(previous, replacement).id, "account");
  for (const mutate of [
    (contract) => { contract.completionScope = "full_acceptance"; },
    (contract) => { contract.criteria[0].mandatory = false; },
    (contract) => { contract.checks[2].description = "weaker"; },
    (contract) => { contract.checks.push({ id: "extra", kind: "command", command: "node tests/extra.mjs", description: "extra" }); },
    (contract) => { contract.checks[0].command = "changed"; },
  ]) {
    const unsafe = structuredClone(replacement); mutate(unsafe);
    assert.throws(() => validateCommandOnlyCorrection(previous, unsafe), /Contract command repair/);
  }
});

test("command repair is one-shot and requires the exact post-recovery pause", () => {
  for (const mutate of [
    (state) => { state.resumePhase = "test-gate"; },
    (state) => { state.pauseReason = "Acceptance: another pause"; },
    (state) => { state.wave = {}; },
    (state) => { state.contractAmendments.push({ kind: "command-only" }); },
  ]) {
    const state = pausedState(); mutate(state); const before = structuredClone(state);
    assert.throws(() => resumeCommandContractRepair(state), /Contract command repair/);
    assert.deepEqual(state, before);
  }
});

test("publication retry reuses the saved independent verification", async () => {
  const state = pausedState(); const { replacement } = contracts(); let verified = 0; let publishes = 0;
  resumeCommandContractRepair(state);
  const operations = ops(replacement, {
    verify: async () => { verified++; return review; },
    publish: async () => { publishes++; if (publishes === 1) throw new Error("interrupted publication"); },
  });
  await assert.rejects(commandContractRepairStage(state, operations), /interrupted publication/);
  assert.equal(state.phase, "contract-command-repair");
  assert.equal(state.contractCommandRepair.review.approved, true);
  assert.ok(state.contractCommandRepair.nextAcceptance);
  await commandContractRepairStage(state, operations);
  assert.equal(verified, 1);
  assert.equal(publishes, 2);
  assert.equal(state.phase, "tickets");
});

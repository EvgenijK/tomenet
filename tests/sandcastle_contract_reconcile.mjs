import assert from "node:assert/strict";
import test from "node:test";
import { readFileSync } from "node:fs";
import { applyContractReconciliation, assemblyCommandRules, reconcileContractDraft } from "../.sandcastle/contract-reconcile.mjs";
import { baselineChecks, contractStage, validateContract } from "../.sandcastle/acceptance.mjs";

const sources = { "spec.md": "The Guide projection is required now. Native visual evidence is later." };
const draft = () => ({ version: 1, completionScope: "implementation", checks: [
  ...structuredClone(baselineChecks), { id: "guide", kind: "command", command: "node tests/guide.mjs", description: "Production Guide" },
  { id: "native", kind: "external", command: "", description: "Native inspection" },
], criteria: [
  { id: "projection", requirement: "Guide projection", source: { path: "spec.md", quote: "The Guide projection is required now." }, mandatory: true, applicability: "deferred", owner: "Later", deferralReason: "Later", checkIds: ["native"] },
  { id: "visual", requirement: "Native visual evidence", source: { path: "spec.md", quote: "Native visual evidence is later." }, mandatory: true, applicability: "deferred", owner: "Later", deferralReason: "Later", checkIds: ["native"] },
] });
const patch = () => ({ summary: "Require production projection now and retain native evidence later", criterionEdits: [{ id: "projection", replacement: {
  ...draft().criteria[0], applicability: "current", owner: "SV-B-008", deferralReason: "", checkIds: ["guide"] } }], checkEdits: [], criteriaToAdd: [], checksToAdd: [] });

test("reconciliation output schema requires every property in every object", () => {
  const schema = JSON.parse(readFileSync(new URL("../.sandcastle/contract-reconcile-output.schema.json", import.meta.url), "utf8"));
  const inspect = (value) => {
    if (!value || typeof value !== "object") return;
    if (value.type === "object") {
      assert.equal(value.additionalProperties, false);
      assert.deepEqual([...value.required].sort(), Object.keys(value.properties).sort());
    }
    for (const child of Object.values(value)) inspect(child);
  };
  inspect(schema);
  assert.deepEqual(schema.properties.completionScope.type, ["string", "null"]);
  assert.ok(schema.properties.completionScope.enum.includes(null));
});

test("reconciliation repairs cross-part scope without deleting obligations or skipping verification", async () => {
  const pending = {}; let calls = 0;
  const revised = await reconcileContractDraft(pending, draft(), sources, async () => { calls++; return patch(); }, async () => {});
  assert.equal(revised.criteria[0].applicability, "current"); assert.equal(revised.criteria[1].applicability, "deferred");
  await reconcileContractDraft(pending, draft(), sources, async () => { calls++; throw new Error("do not rerun"); }, async () => {});
  assert.equal(calls, 1);
  await assert.rejects(reconcileContractDraft(pending, draft(), { "spec.md": sources["spec.md"] + " Changed" }, async () => {}, async () => {}), /checkpoint changed/);
});

test("reconciliation cannot remove, fabricate or downgrade mandatory criteria", () => {
  for (const edit of [
    { ...patch().criterionEdits[0], replacement: { ...patch().criterionEdits[0].replacement, mandatory: false } },
    { ...patch().criterionEdits[0], id: "unknown" },
    { ...patch().criterionEdits[0], replacement: { ...patch().criterionEdits[0].replacement, source: { path: "spec.md", quote: "invented" } } },
  ]) assert.throws(() => applyContractReconciliation(draft(), { ...patch(), criterionEdits: [edit] }, sources));
  assert.throws(() => applyContractReconciliation(draft(), { ...patch(), criteriaToAdd: [draft().criteria[0]] }, sources), /Duplicate/);
});

test("assembly can correct completion scope when parallel drafts disagree", () => {
  const mixedDraft = { ...draft(), completionScope: "full_acceptance" };
  const changed = applyContractReconciliation(mixedDraft, { ...patch(), completionScope: "implementation" }, sources);
  assert.equal(changed.completionScope, "implementation");
  assert.equal(applyContractReconciliation(draft(), { ...patch(), completionScope: null }, sources).completionScope, "implementation");
  assert.throws(() => applyContractReconciliation(mixedDraft, patch(), sources), /Invalid draft reconciliation/);
  assert.throws(() => applyContractReconciliation(mixedDraft, { ...patch(), completionScope: "invalid" }, sources), /Malformed/);
});

test("assembly returns check-specific feedback and saves only a validated correction", async () => {
  const pending = { round: 1, groupPlanDigest: "groups", groupResponses: [{}, {}, {}] };
  const beforeGroups = structuredClone(pending.groupResponses);
  const commands = ["node tests/guide.mjs && true", "node tests/guide.mjs"];
  const feedback = []; const snapshots = [];
  const revised = await reconcileContractDraft(pending, draft(), sources, async (_draft, error) => {
    feedback.push(error);
    return { ...patch(), checkEdits: [{ id: "guide", replacement: { ...draft().checks[2], command: commands.shift() } }] };
  }, async () => { snapshots.push(structuredClone(pending)); });
  assert.equal(revised.checks[2].command, "node tests/guide.mjs");
  assert.equal(feedback.length, 2);
  assert.match(feedback[1], /guide.*repository test/i);
  assert.equal(snapshots[1].reconciliation, undefined);
  assert.equal(snapshots[1].assemblyAttempts.used, 1);
  assert.equal(snapshots[1].assemblyAttempts.remaining, 2);
  assert.deepEqual(pending.groupResponses, beforeGroups);
  assert.equal(pending.assemblyAttempts.used, 2);
});

test("assembly prompt states immutable baselines and the focused forms enforced by validation", () => {
  for (const form of ["sv-build and sv-core baseline checks are immutable", "add a separate focused check", "python3 -B tests/<file>", "node tests/<file>", "bash tests/<file>", "non-empty command array", "stops on the first failure", "command=''", "shell operators", "Preserve every other already valid check"]) {
    assert.ok(assemblyCommandRules.includes(form), form);
  }
});

test("invalid assembly proposals exhaust only a persisted three-call budget", async () => {
  const pending = { round: 1, groupPlanDigest: "groups", groupResponses: [{}, {}, {}] };
  const invalid = { ...patch(), checkEdits: [{ id: "guide", replacement: { ...draft().checks[2], command: "node tests/guide.mjs | tee log" } }] };
  let calls = 0;
  await assert.rejects(reconcileContractDraft(pending, draft(), sources, async () => { calls++; return invalid; }, async () => {}), /assembly attempts exhausted.*guide/);
  assert.equal(calls, 3);
  assert.equal(pending.assemblyAttempts.used, 3);
  assert.equal(pending.reconciliation, undefined);
  await assert.rejects(reconcileContractDraft(pending, draft(), sources, async () => { calls++; return patch(); }, async () => {}), /assembly attempts exhausted/);
  assert.equal(calls, 3);
});

test("explicit extension preserves three spent calls and accepts a corrected baseline", async () => {
  const pending = { round: 2, groupPlanDigest: "groups", groupResponses: [{}, {}, {}, {}] };
  const invalid = { ...patch(), checkEdits: [{ id: "sv-core", replacement: { ...baselineChecks[1], command: "node tests/guide.mjs" } }] };
  let calls = 0;
  await assert.rejects(reconcileContractDraft(pending, draft(), sources, async () => { calls++; return invalid; }, async () => {}), /assembly attempts exhausted.*sv-core/);
  pending.assemblyAttempts.baseGranted = 3;
  pending.assemblyAttempts.extensions = [{ id: "approved-16", extra: 16, authorizedAt: "2026-10-03T00:00:00Z" }];
  pending.assemblyAttempts.granted += 16;
  pending.assemblyAttempts.remaining += 16;
  const revised = await reconcileContractDraft(pending, draft(), sources, async (_draft, error) => {
    calls++;
    assert.match(error, /sv-core/);
    return patch();
  }, async () => {});
  assert.equal(calls, 4);
  assert.equal(revised.checks.find((item) => item.id === "sv-core").command, baselineChecks[1].command);
  assert.equal(pending.assemblyAttempts.used, 4);
  assert.equal(pending.assemblyAttempts.remaining, 15);
});

test("assembly interruption consumes one reserved call and resumes at the same draft", async () => {
  const pending = { round: 1, groupPlanDigest: "groups" };
  await assert.rejects(reconcileContractDraft(pending, draft(), sources, async () => { throw new Error("Interrupted"); }, async () => {}), /Interrupted/);
  assert.equal(pending.assemblyAttempts.used, 1);
  const revised = await reconcileContractDraft(pending, draft(), sources, async () => patch(), async () => {});
  assert.equal(revised.criteria[0].applicability, "current");
  assert.equal(pending.assemblyAttempts.used, 2);
  await assert.rejects(reconcileContractDraft({ ...pending, reconciliation: undefined }, { ...draft(), completionScope: "full_acceptance" }, sources, async () => patch(), async () => {}), /checkpoint/);
});

test("corrected assembly still needs independent verification before acceptance", async () => {
  const state = { phase: "contract", contractPending: { round: 1, proposalInFlight: true }, id: "test" };
  let verifies = 0;
  await contractStage(state, {
    sources: async () => sources,
    propose: async (_sources, _feedback, pending) => reconcileContractDraft(pending, draft(), sources, async (_draft, error) => ({
      ...patch(), checkEdits: [{ id: "guide", replacement: { ...draft().checks[2], command: error ? "node tests/guide.mjs" : "node tests/guide.mjs && true" } }],
    }), async () => {}),
    verify: async (proposal) => { verifies++; validateContract(proposal, sources); return { approved: true, summary: "All source obligations covered", missingRequirements: [] }; },
    head: async () => "test-head", publish: async () => {}, save: async () => {},
  });
  assert.equal(verifies, 1);
  assert.equal(state.phase, "plan");
  assert.equal(state.acceptance.contract.checks[2].command, "node tests/guide.mjs");
});

import assert from "node:assert/strict";
import test from "node:test";
import { applyContractReconciliation, reconcileContractDraft } from "../.sandcastle/contract-reconcile.mjs";
import { baselineChecks } from "../.sandcastle/acceptance.mjs";

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
  assert.throws(() => applyContractReconciliation(mixedDraft, patch(), sources), /Invalid draft reconciliation/);
  assert.throws(() => applyContractReconciliation(mixedDraft, { ...patch(), completionScope: "invalid" }, sources), /Malformed/);
});

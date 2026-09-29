import assert from "node:assert/strict";
import test from "node:test";
import { partitionContractSources, combineContractParts, buildContractDraft } from "../.sandcastle/contract-draft.mjs";

const sources = {
  "docs/tasks/feature.md": "# Feature\n\n## Current\nCurrent behavior is required.\n\n## Later\nLater platform checks remain pending.\n",
  "AGENTS.md": "Preserve production behavior.\n",
};
const baseline = [
  { id: "sv-build", kind: "build", command: "make -s -C src -f makefile.sv tomenet-sv", description: "SV Make build" },
  { id: "sv-core", kind: "core", command: "bash -s -- core", description: "Sandcastle core checks" },
];
const response = (part, index) => ({ version: 1, completionScope: "implementation", checks: [...baseline, { id: "feature", kind: "command", command: "node tests/guide_feature.mjs", description: "Production feature" }], criteria: [{
  id: "requirement", requirement: `Part ${index} requirement`, mandatory: true, applicability: "current", deferralReason: "", owner: "SV-B-008",
  source: { path: part.sourcePath, quote: part.sourceText.trim().split("\n").find((line) => line && !line.startsWith("#")) }, checkIds: ["sv-core", "feature"],
}] });

test("draft partitions every source byte and checkpoints completed pieces across an interrupted call", async () => {
  const parts = partitionContractSources(sources, 70);
  for (const [path, content] of Object.entries(sources)) assert.equal(parts.filter((part) => part.sourcePath === path).map((part) => part.sourceText).join(""), content);
  const pending = { round: 1 }; let calls = 0; let saves = 0;
  await assert.rejects(buildContractDraft(pending, sources, async (part, index) => {
    calls++;
    if (index === 1) throw new Error("agent timeout");
    return response(part, index);
  }, async () => { saves++; }, 70), /agent timeout/);
  assert.equal(pending.parts.length, 1); assert.ok(saves >= 1);
  const result = await buildContractDraft(pending, sources, async (part, index) => { calls++; return response(part, index); }, async () => { saves++; }, 70);
  assert.equal(calls, parts.length + 1); // first part was not regenerated
  assert.equal(result.criteria.length, parts.length);
  assert.equal(result.checks.length, baseline.length + 1);
  assert.ok(result.criteria.every((criterion) => criterion.checkIds.includes("P1-feature")));
});

test("part combiner rejects fabricated quotes, inconsistent scope and changed sources", async () => {
  const parts = partitionContractSources(sources, 70);
  const answers = parts.map(response);
  assert.equal(combineContractParts(parts, answers).criteria.length, parts.length);
  const fabricated = structuredClone(answers); fabricated[0].criteria[0].source.quote = "fabricated";
  assert.throws(() => combineContractParts(parts, fabricated), /source/);
  const differentScope = structuredClone(answers); differentScope[0].completionScope = "full_acceptance";
  assert.throws(() => combineContractParts(parts, differentScope), /scope/);
  const pending = { round: 1 };
  await buildContractDraft(pending, sources, async (part, index) => response(part, index), async () => {}, 70);
  await assert.rejects(buildContractDraft(pending, { ...sources, "AGENTS.md": "Changed policy" }, async () => { throw new Error("must not run"); }, async () => {}, 70), /source.*changed/);
});

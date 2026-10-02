import assert from "node:assert/strict";
import test from "node:test";
import { planContractGroups, buildGroupedContractDraft, validateContractGroups } from "../.sandcastle/contract-groups.mjs";
import { baselineChecks } from "../.sandcastle/acceptance.mjs";

const sources = {
  "docs/tasks/feature.md": "First production obligation.",
  "docs/tasks/other.md": "Second production obligation.",
  "AGENTS.md": "Preserve the SV production path.",
};
const split = { groups: [
  { title: "Feature behavior", partIndexes: [0, 2] },
  { title: "Other behavior", partIndexes: [1] },
] };
const response = (group) => ({ version: 1, completionScope: "implementation", checks: structuredClone(baselineChecks), criteria: group.sourceSections.map((section, index) => ({
  id: `criterion-${index}`, requirement: section.sourceText, source: { path: section.sourcePath, quote: section.sourceText },
  mandatory: true, applicability: "current", owner: "", deferralReason: "", checkIds: ["sv-core"],
})) });

test("contract groups cover every source section exactly once", async () => {
  const pending = {}; let splits = 0;
  const planned = await planContractGroups(pending, sources, async () => { splits++; return split; }, async () => {});
  assert.equal(planned.parts.length, 3);
  assert.equal(splits, 1);
  await planContractGroups(pending, sources, async () => { throw new Error("split should be reused"); }, async () => {});
  assert.throws(() => validateContractGroups({ groups: [{ title: "Duplicate", partIndexes: [0, 0] }, split.groups[1]] }, planned.parts), /overlaps/);
  assert.throws(() => validateContractGroups({ groups: [{ title: "Missing", partIndexes: [0] }, split.groups[1]] }, planned.parts), /omitted/);
  await assert.rejects(planContractGroups(pending, { ...sources, "AGENTS.md": "Changed policy" }, async () => split, async () => {}), /source changed/);
});

test("parallel contract agents save successful groups and resume only the failed group", async () => {
  const pending = {};
  const { parts, plan } = await planContractGroups(pending, sources, async () => split, async () => {});
  const calls = [0, 0]; let active = 0; let maxActive = 0;
  const propose = async (group, index) => {
    calls[index]++;
    active++; maxActive = Math.max(maxActive, active);
    await new Promise((resolve) => setTimeout(resolve, 5));
    active--;
    if (index === 0 && calls[index] === 1) throw new Error("first group interrupted");
    return { ...response(group), completionScope: index === 1 ? "full_acceptance" : "implementation" };
  };
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, propose, async () => {}), /first group interrupted/);
  assert.equal(maxActive, 2);
  assert.equal(pending.groupResponses[1].criteria.length, 1);
  const contract = await buildGroupedContractDraft(pending, parts, plan, propose, async () => {});
  assert.deepEqual(calls, [2, 1]);
  assert.equal(contract.criteria.length, 3);
  assert.equal(contract.completionScope, "implementation");
  assert.ok(contract.criteria.some((criterion) => criterion.source.path === "AGENTS.md"));
});

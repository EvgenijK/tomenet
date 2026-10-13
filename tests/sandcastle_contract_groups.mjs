import assert from "node:assert/strict";
import test from "node:test";
import { planContractGroups, buildGroupedContractDraft, validateContractGroups, withContractGroupRecoveryAdvice } from "../.sandcastle/contract-groups.mjs";
import { contractSourceCatalog, verifierFeedbackDetails, feedbackForGroup } from "../.sandcastle/contract-feedback.mjs";
import { partitionContractSources, sourceSpans } from "../.sandcastle/contract-draft.mjs";
import { baselineChecks, validateContract, contractStage } from "../.sandcastle/acceptance.mjs";

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
  id: `criterion-${index}`, requirement: section.sourceText, sourceSpanId: group.sourceSpans.find((span) => span.sourcePath === section.sourcePath && span.quote === section.sourceText.trim())?.id,
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

test("assigned span IDs expand to exact sources and reject foreign, stale and free-form citations", async () => {
  const pending = {};
  const { parts, plan } = await planContractGroups(pending, sources, async () => split, async () => {});
  let assigned;
  const contract = await buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    if (index === 1) assigned = group.sourceSpans[0];
    return response(group);
  }, async () => {});
  assert.deepEqual(contract.criteria.find((item) => item.source.path === "docs/tasks/other.md").source,
    { path: assigned.sourcePath, quote: assigned.quote });
  assert.ok(pending.groupResponses[1].criteria.every((item) => item.source === undefined));
  const old = structuredClone(pending.groupResponses[0]);
  delete pending.groupResponses[1];
  const foreign = sourceSpans(parts)[0][0].id;
  const revised = sourceSpans(partitionContractSources({ ...sources, "AGENTS.md": "Changed policy." }));
  assert.notEqual(foreign, revised[0][0].id);
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    assert.equal(index, 1);
    const answer = response(group);
    answer.criteria[0].sourceSpanId = foreign;
    return answer;
  }, async () => {}), /unknown or stale sourceSpanId/);
  assert.deepEqual(pending.groupResponses[0], old);
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group) => {
    const answer = response(group);
    answer.criteria[0].sourceSpanId = "S-0000000000000000";
    return answer;
  }, async () => {}), /unknown or stale sourceSpanId/);
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group) => {
    const answer = response(group);
    answer.criteria[0].source = { path: "docs/tasks/other.md", quote: "Second production obligation." };
    return answer;
  }, async () => {}), /unknown or stale sourceSpanId/);
});

test("migration freezes only pre-existing legacy group answers", async () => {
  const pending = {};
  const { parts, plan } = await planContractGroups(pending, sources, async () => split, async () => {});
  const firstSpans = sourceSpans(parts);
  pending.groupResponses = [{ version: 1, completionScope: "implementation", checks: structuredClone(baselineChecks),
    criteria: split.groups[0].partIndexes.map((partIndex, index) => ({ id: `old-${index}`, requirement: "Legacy requirement",
      source: { path: parts[partIndex].sourcePath, quote: firstSpans[partIndex][0].quote }, mandatory: true,
      applicability: "current", owner: "", deferralReason: "", checkIds: ["sv-core"] })) }];
  const saved = structuredClone(pending.groupResponses[0]);
  const contract = await buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    assert.equal(index, 1);
    return response(group);
  }, async () => {});
  assert.deepEqual(pending.groupResponses[0], saved);
  assert.deepEqual(pending.legacyGroupIndexes, [0]);
  assert.equal(contract.criteria.length, 3);
  delete pending.groupResponses[1];
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group) => {
    const answer = response(group);
    answer.criteria[0].source = { path: group.sourceSections[0].sourcePath, quote: group.sourceSections[0].sourceText };
    return answer;
  }, async () => {}), /unknown or stale sourceSpanId/);
});

test("verifier feedback goes only to its single owner; unknown and cross-group items reach assembly only", async () => {
  const pending = {};
  const { parts, plan } = await planContractGroups(pending, sources, async () => split, async () => {});
  const spans = sourceSpans(parts);
  const local = "Cover the first production path";
  const cross = "Check behavior across both features";
  const unknown = "Check platform matrix";
  const review = { missingRequirements: [local, cross, unknown], findings: [
    { message: local, sourceSpanIds: [spans[0][0].id] },
    { message: cross, sourceSpanIds: [spans[0][0].id, spans[1][0].id] },
    { message: unknown, sourceSpanIds: ["S-0000000000000000"] },
  ] };
  pending.feedback = review.missingRequirements;
  pending.feedbackDetails = verifierFeedbackDetails(review, sources);
  await buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    assert.deepEqual(feedbackForGroup(pending, group), index === 0 ? [local] : []);
    return response(group);
  }, async () => {});
  assert.deepEqual(pending.feedback, [local, cross, unknown]);
  assert.deepEqual(pending.feedbackDetails.find((item) => item.message === unknown).sourceSpanIds, []);
  assert.equal(contractSourceCatalog(sources).length, 3);
});

test("contract recovery advice reaches only a rejected group on checkpoint replay", async () => {
  const pending = {};
  const { parts, plan } = await planContractGroups(pending, sources, async () => split, async () => {});
  const state = {};
  const calls = [];
  const propose = async (group, index) => {
    const prompt = withContractGroupRecoveryAdvice("Original contract proposal", state);
    calls.push({ index, prompt });
    const draft = response(group);
    if (index === 1 && calls.filter((call) => call.index === 1).length === 1) {
      draft.criteria[0].sourceSpanId = "S-0000000000000000";
    }
    return draft;
  };

  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, propose, async () => {}), /unknown or stale sourceSpanId/);
  const saved = structuredClone(pending.groupResponses[0]);
  assert.equal(pending.groupResponses[1], undefined);
  state.recoveryAdvice = "Cite exact text from assigned source sections.";
  state.recoveryAdvicePhase = "contract";
  const contract = await buildGroupedContractDraft(pending, parts, plan, propose, async () => {});

  assert.deepEqual(calls.map(({ index }) => index), [0, 1, 1]);
  assert.deepEqual(pending.groupResponses[0], saved);
  assert.deepEqual(calls.slice(0, 2).map(({ prompt }) => prompt), ["Original contract proposal", "Original contract proposal"]);
  assert.match(calls[2].prompt, /Recovery guidance: Cite exact text from assigned source sections\./);
  assert.equal(contract.criteria.length, 3);
  assert.equal(withContractGroupRecoveryAdvice("Independent verification", { ...state, recoveryAdvicePhase: "review" }), "Independent verification");
});

test("five authorized citation attempts retry only the rejected contract group", async () => {
  const pending = { round: 1 };
  const threeGroups = { groups: [
    { title: "Feature", partIndexes: [0] },
    { title: "Other", partIndexes: [1] },
    { title: "Policy", partIndexes: [2] },
  ] };
  const { parts, plan } = await planContractGroups(pending, sources, async () => threeGroups, async () => {});
  const initialCalls = [0, 0, 0];
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    initialCalls[index]++;
    const draft = response(group);
    if (index === 1) draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  }, async () => {}), /unknown or stale sourceSpanId/);
  assert.deepEqual(initialCalls, [1, 1, 1]);
  assert.deepEqual([0, 2].map((index) => Boolean(pending.groupResponses[index])), [true, true]);

  pending.citationRetryGrant = { round: 1, groupIndex: 1, groupPlanDigest: pending.groupPlanDigest, granted: 5, remaining: 5, used: 0 };
  const calls = [0, 0, 0];
  const contract = await buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    calls[index]++;
    const draft = response(group);
    if (calls[index] < 5) draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  }, async () => {});
  assert.deepEqual(calls, [0, 5, 0]);
  assert.equal(contract.criteria.length, 3);
  assert.equal(pending.citationRetryGrant, undefined);
});

test("citation grant stops after exactly five failed proposals without losing saved groups", async () => {
  const pending = { round: 1 };
  const { parts, plan } = await planContractGroups(pending, sources, async () => split, async () => {});
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    const draft = response(group);
    if (index === 1) draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  }, async () => {}), /unknown or stale sourceSpanId/);
  const saved = structuredClone(pending.groupResponses[0]);
  pending.citationRetryGrant = { round: 1, groupIndex: 1, groupPlanDigest: pending.groupPlanDigest, granted: 5, remaining: 5, used: 0 };
  let calls = 0;
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group) => {
    calls++;
    const draft = response(group);
    draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  }, async () => {}), /authorized contract citation attempts exhausted/);
  assert.equal(calls, 5);
  assert.deepEqual(pending.groupResponses[0], saved);
  assert.equal(pending.citationRetryGrant.remaining, 0);
  assert.equal(pending.citationRetryGrant.used, 5);
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async () => { calls++; throw new Error("unexpected sixth call"); }, async () => {}), /authorized contract citation attempts exhausted/);
  assert.equal(calls, 5);
});

test("eight authorized group attempts preserve accepted siblings and feed back validation errors", async () => {
  const pending = { round: 2 };
  const threeGroups = { groups: [
    { title: "Feature", partIndexes: [0] },
    { title: "Other", partIndexes: [1] },
    { title: "Policy", partIndexes: [2] },
  ] };
  const { parts, plan } = await planContractGroups(pending, sources, async () => threeGroups, async () => {});
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    const draft = response(group);
    if (index === 0) draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  }, async () => {}), /unknown or stale sourceSpanId/);
  const siblings = [pending.groupResponses[1], pending.groupResponses[2]].map((item) => structuredClone(item));
  pending.groupRetryGrant = { round: 2, groupIndex: 0, groupPlanDigest: pending.groupPlanDigest,
    granted: 8, remaining: 8, used: 0 };
  let calls = 0;
  const prompts = [];
  const state = { contractPending: pending };
  const contract = await buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    assert.equal(index, 0);
    calls++;
    prompts.push(withContractGroupRecoveryAdvice("Original instructions", state, index));
    const draft = response(group);
    if (calls < 3) draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  }, async () => {});
  assert.equal(calls, 3);
  assert.match(prompts[1], /unknown or stale sourceSpanId/);
  assert.deepEqual([pending.groupResponses[1], pending.groupResponses[2]], siblings);
  assert.equal(contract.criteria.length, 3);
  assert.equal(pending.groupRetryGrant, undefined);
});

test("group retry grant spends at most eight calls and saves the last validation error", async () => {
  const pending = { round: 2 };
  const { parts, plan } = await planContractGroups(pending, sources, async () => split, async () => {});
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, async (group, index) => {
    const draft = response(group);
    if (index === 1) draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  }, async () => {}), /unknown or stale sourceSpanId/);
  const sibling = structuredClone(pending.groupResponses[0]);
  pending.groupRetryGrant = { round: 2, groupIndex: 1, groupPlanDigest: pending.groupPlanDigest,
    granted: 8, remaining: 8, used: 0 };
  let calls = 0;
  const fail = async (group) => {
    calls++;
    const draft = response(group);
    draft.criteria[0].sourceSpanId = "S-0000000000000000";
    return draft;
  };
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, fail, async () => {}), /authorized contract group attempts exhausted/);
  assert.equal(calls, 8);
  assert.deepEqual(pending.groupResponses[0], sibling);
  assert.equal(pending.groupRetryGrant.used, 8);
  assert.equal(pending.groupRetryGrant.remaining, 0);
  assert.match(pending.groupRetryGrant.lastError, /unknown or stale sourceSpanId/);
  await assert.rejects(buildGroupedContractDraft(pending, parts, plan, fail, async () => {}), /authorized contract group attempts exhausted/);
  assert.equal(calls, 8);
});


test("v2 draft, reconciliation and acceptance reuse persisted IDs without embedding source text", async () => {
  const { applyContractReconciliation } = await import("../.sandcastle/contract-reconcile.mjs");
  const catalog = { version: 1, digest: "persisted-catalog", fragments: [
    { id: "S-persisted-feature", sourcePath: "docs/tasks/feature.md", quote: sources["docs/tasks/feature.md"] },
    { id: "S-persisted-other", sourcePath: "docs/tasks/other.md", quote: sources["docs/tasks/other.md"] },
    { id: "S-persisted-policy", sourcePath: "AGENTS.md", quote: sources["AGENTS.md"] },
  ] };
  const parts = partitionContractSources(sources);
  const pending = { round: 1 };
  let calls = 0;
  const propose = async (group) => {
    calls++;
    assert.ok(group.sourceSpans.every((span) => span.quote === undefined));
    return { version: 2, completionScope: "implementation", checks: structuredClone(baselineChecks),
      criteria: group.sourceSpans.map((span, index) => ({ id: `AC-${index}`, requirement: "Preserve the assigned production obligation",
        sourceSpanIds: [span.id], mandatory: true, applicability: "current", owner: "", deferralReason: "", checkIds: ["sv-core"] })) };
  };
  const draft = await buildGroupedContractDraft(pending, parts, split, propose, async () => {}, { catalog });
  assert.equal(draft.version, 2);
  assert.equal(pending.groupCitationFormat, "ids-v2");
  assert.deepEqual(new Set(draft.criteria.flatMap((criterion) => criterion.sourceSpanIds)), new Set(catalog.fragments.map((fragment) => fragment.id)));
  await buildGroupedContractDraft(pending, parts, split, async () => { throw new Error("Saved groups must be reused"); }, async () => {}, { catalog });
  assert.equal(calls, 2);
  validateContract(draft, {}, catalog); // v2 resolves only persisted records, never regenerated sources.
  assert.throws(() => validateContract(draft, sources), /persisted fragment catalog required/);
  const bad = structuredClone(draft); bad.criteria[0].sourceSpanIds = ["S-invented"];
  assert.throws(() => validateContract(bad, sources, catalog), /unknown or invalid sourceSpanIds/);
  for (const field of ["source", "sourceSpanId", "quote", "sourcePath"]) {
    const inline = structuredClone(draft); inline.criteria[0][field] = "embedded";
    assert.throws(() => validateContract(inline, sources, catalog), /fragment IDs only/);
  }
  const patch = { summary: "Keep sourced obligations", completionScope: null, criterionEdits: [], checkEdits: [], criteriaToAdd: [], checksToAdd: [] };
  const reconciled = applyContractReconciliation(draft, patch, sources, catalog);
  const state = { id: "v2", phase: "contract" };
  await contractStage(state, { sources: async () => sources, catalog: async () => catalog,
    propose: async () => reconciled, verify: async () => ({ approved: true, summary: "Source obligations covered", missingRequirements: [] }),
    head: async () => "head", save: async () => {}, publish: async (acceptance) => {
      assert.equal(acceptance.fragmentCatalogDigest, catalog.digest);
      assert.ok(acceptance.contract.criteria.every((criterion) => criterion.source === undefined));
    } });
  assert.equal(state.phase, "plan");
  assert.ok(!JSON.stringify(state.acceptance).includes('"quote"'));
  const message = "Missing persisted policy";
  assert.deepEqual(verifierFeedbackDetails({ missingRequirements: [message], findings: [{ message, sourceSpanIds: [catalog.fragments[2].id] }] }, {}, catalog),
    [{ message, sourceSpanIds: [catalog.fragments[2].id] }]);
});

test("v2 grouped draft rejects foreign fragments and old quote responses before checkpointing", async () => {
  const catalog = { version: 1, fragments: Object.entries(sources).map(([sourcePath, quote], index) => ({ id: `S-persisted-${index}`, sourcePath, quote })) };
  const parts = partitionContractSources(sources);
  for (const inline of [false, true]) {
    const pending = { round: 1 };
    await assert.rejects(buildGroupedContractDraft(pending, parts, split, async (group, index) => ({
      version: 2, completionScope: "implementation", checks: structuredClone(baselineChecks), criteria: [{
        id: "AC", requirement: "Preserve obligation", sourceSpanIds: [index === 0 ? "S-persisted-1" : "S-persisted-0"],
        ...(inline ? { source: { path: "AGENTS.md", quote: sources["AGENTS.md"] } } : {}),
        mandatory: true, applicability: "current", owner: "", deferralReason: "", checkIds: ["sv-core"] }] }), async () => {}, { catalog }), /unknown or invalid sourceSpanIds/);
    assert.equal(pending.groupResponses.filter(Boolean).length, 0);
  }
});

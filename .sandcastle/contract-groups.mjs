import { digest } from "./acceptance.mjs";
import { combineContractParts, partitionContractSources } from "./contract-draft.mjs";

export function validateContractGroups(plan, parts) {
  if (!Array.isArray(plan?.groups) || !parts.length || plan.groups.length < Math.min(2, parts.length) || plan.groups.length > Math.min(10, parts.length)) {
    throw new Error("Contract split requires two or more groups when the source has multiple sections, with at most ten groups");
  }
  const seen = new Set();
  for (const group of plan.groups) {
    if (typeof group?.title !== "string" || !group.title.trim() || !Array.isArray(group.partIndexes) || !group.partIndexes.length) throw new Error("Invalid contract group");
    for (const index of group.partIndexes) {
      if (!Number.isInteger(index) || index < 0 || index >= parts.length || seen.has(index)) throw new Error("Contract split overlaps or references an unknown section");
      seen.add(index);
    }
  }
  if (seen.size !== parts.length) throw new Error("Contract split omitted a source section");
  return plan;
}

export async function planContractGroups(pending, sources, split, save) {
  const parts = partitionContractSources(sources);
  const sourceDigest = digest(sources);
  if (pending.groupSourceDigest && pending.groupSourceDigest !== sourceDigest) throw new Error("Contract split source changed; human decision required");
  if (pending.groupPlan) {
    if (!pending.groupSourceDigest) throw new Error("Unverified contract split checkpoint");
    return { parts, plan: validateContractGroups(pending.groupPlan, parts) };
  }
  const plan = validateContractGroups(await split(parts), parts);
  pending.groupSourceDigest = sourceDigest;
  pending.groupPlan = plan;
  await save();
  return { parts, plan };
}

export async function buildGroupedContractDraft(pending, parts, plan, propose, save) {
  validateContractGroups(plan, parts);
  const grouped = plan.groups.map((group) => ({ title: group.title, sourceSections: group.partIndexes.map((index) => parts[index]) }));
  const planDigest = digest({ parts, plan });
  if (pending.groupPlanDigest && pending.groupPlanDigest !== planDigest) throw new Error("Contract group checkpoint changed; human decision required");
  pending.groupPlanDigest = planDigest;
  pending.groupResponses ??= [];
  await save();
  const outcomes = await Promise.allSettled(grouped.map(async (group, index) => {
    if (pending.groupResponses[index]) return;
    const response = await propose(group, index, grouped.length);
    combineContractParts([group], [response]);
    pending.groupResponses[index] = response;
    await save();
  }));
  const failure = outcomes.find((outcome) => outcome.status === "rejected");
  if (failure) throw failure.reason;
  return combineContractParts(grouped, pending.groupResponses, { allowMixedScopes: true });
}

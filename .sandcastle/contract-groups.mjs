import { digest } from "./acceptance.mjs";
import { combineContractParts, partitionContractSources } from "./contract-draft.mjs";
import { guidanceFor } from "./recovery-guidance.mjs";

export function withContractGroupRecoveryAdvice(prompt, state, index) {
  const guidance = guidanceFor(state, { phase: "contract", substage: "parallel", groupIndex: index });
  const pending = state.contractPending;
  const grant = pending?.citationRetryGrant;
  const citation = grant && grant.groupIndex === index
    ? `Citation retry for this group: each source.path must be one of this group's assigned sourcePath values, and each source.quote must be copied verbatim as a contiguous substring of that same assigned section's sourceText. Do not quote the task overview or referenced documents. ${pending.citationRetryError ?? "The previous answer cited text outside its assigned sections."}`
    : "";
  return [prompt, guidance, citation].filter(Boolean).join("\n\n");
}

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
  const grant = pending.citationRetryGrant;
  if (grant && (grant.round !== pending.round || grant.groupPlanDigest !== planDigest ||
    !Number.isInteger(grant.groupIndex) || grant.groupIndex < 0 || grant.groupIndex >= grouped.length ||
    pending.groupResponses?.[grant.groupIndex] || !Number.isInteger(grant.remaining) || grant.remaining < 0 ||
    !Number.isInteger(grant.used) || grant.used < 0 || grant.remaining + grant.used !== grant.granted)) {
    throw new Error("Acceptance: contract citation retry grant no longer matches its checkpoint; human decision required");
  }
  pending.groupPlanDigest = planDigest;
  pending.groupResponses ??= [];
  await save();
  const outcomes = await Promise.allSettled(grouped.map(async (group, index) => {
    if (pending.groupResponses[index]) return;
    while (true) {
      const retry = pending.citationRetryGrant?.groupIndex === index ? pending.citationRetryGrant : undefined;
      if (retry) {
        if (retry.remaining === 0) throw new Error("Acceptance: user-authorized contract citation attempts exhausted; human decision required");
        // Reserve before launching the agent so an interrupted call cannot be replayed for free.
        retry.remaining--;
        retry.used++;
        await save();
      }
      const response = await propose(group, index, grouped.length);
      try { combineContractParts([group], [response]); }
      catch (error) {
        if (!retry || error.message !== "Contract part cited a source outside its assigned section") throw error;
        pending.citationRetryError = error.message;
        await save();
        if (retry.remaining > 0) continue;
        throw new Error(`Acceptance: user-authorized contract citation attempts exhausted; ${error.message}; human decision required`);
      }
      pending.groupResponses[index] = response;
      if (retry) { delete pending.citationRetryGrant; delete pending.citationRetryError; }
      await save();
      return;
    }
  }));
  const failedIndex = outcomes.findIndex((outcome) => outcome.status === "rejected");
  if (failedIndex !== -1) {
    pending.failedGroupIndex = failedIndex;
    await save();
    throw outcomes[failedIndex].reason;
  }
  delete pending.failedGroupIndex;
  return combineContractParts(grouped, pending.groupResponses, { allowMixedScopes: true });
}

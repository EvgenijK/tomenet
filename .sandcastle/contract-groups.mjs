import { digest } from "./acceptance.mjs";
import { combineContractParts, partitionContractSources, sourceSpans } from "./contract-draft.mjs";
import { guidanceFor } from "./recovery-guidance.mjs";

export function withContractGroupRecoveryAdvice(prompt, state, index) {
  const guidance = guidanceFor(state, { phase: "contract", substage: "parallel", groupIndex: index });
  const pending = state.contractPending;
  const grant = pending?.citationRetryGrant;
  const citation = grant && grant.groupIndex === index
    ? `Citation retry for this group: use only a sourceSpanId from this group's assigned sourceSpans. The controller supplies source.path and source.quote. Do not cite the task overview or referenced documents. ${pending.citationRetryError ?? "The previous answer cited text outside its assigned sections."}`
    : "";
  const groupRetry = pending?.groupRetryGrant && pending.groupRetryGrant.groupIndex === index
    ? `This group is the only rejected checkpoint. Correct its previous validation failure without changing the assigned source scope. Each criterion needs an assigned sourceSpanId, and every criterion.checkIds value must name a check in this group's response. ${pending.groupRetryGrant.lastError ?? "The previous response did not pass group validation."}`
    : "";
  return [prompt, guidance, citation, groupRetry].filter(Boolean).join("\n\n");
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
  const spans = sourceSpans(parts);
  const grouped = plan.groups.map((group) => ({ title: group.title,
    sourceSections: group.partIndexes.map((index) => parts[index]),
    sourceSpans: group.partIndexes.flatMap((index) => spans[index]) }));
  const planDigest = digest({ parts, plan });
  if (pending.groupPlanDigest && pending.groupPlanDigest !== planDigest) throw new Error("Contract group checkpoint changed; human decision required");
  const grant = pending.citationRetryGrant;
  const groupGrant = pending.groupRetryGrant;
  if (grant && groupGrant) throw new Error("Acceptance: conflicting contract group retry grants; human decision required");
  if (grant && (grant.round !== pending.round || grant.groupPlanDigest !== planDigest ||
    !Number.isInteger(grant.groupIndex) || grant.groupIndex < 0 || grant.groupIndex >= grouped.length ||
    pending.groupResponses?.[grant.groupIndex] || !Number.isInteger(grant.remaining) || grant.remaining < 0 ||
    !Number.isInteger(grant.used) || grant.used < 0 || grant.remaining + grant.used !== grant.granted)) {
    throw new Error("Acceptance: contract citation retry grant no longer matches its checkpoint; human decision required");
  }
  if (groupGrant && (groupGrant.round !== pending.round || groupGrant.groupPlanDigest !== planDigest ||
    !Number.isInteger(groupGrant.groupIndex) || groupGrant.groupIndex < 0 || groupGrant.groupIndex >= grouped.length ||
    pending.groupResponses?.[groupGrant.groupIndex] || !Number.isInteger(groupGrant.granted) || groupGrant.granted < 1 || groupGrant.granted > 8 ||
    !Number.isInteger(groupGrant.remaining) || groupGrant.remaining < 0 ||
    !Number.isInteger(groupGrant.used) || groupGrant.used < 0 || groupGrant.remaining + groupGrant.used !== groupGrant.granted)) {
    throw new Error("Acceptance: contract group retry grant no longer matches its checkpoint; human decision required");
  }
  // Freeze the migration allowlist before launching any new agents. Existing
  // accepted answers retain their exact quote/path, while every future answer
  // must use a controller-expanded span ID. The plan digest remains unchanged.
  if (pending.groupCitationFormat === undefined) {
    pending.groupCitationFormat = "span-v1";
    pending.legacyGroupIndexes = (pending.groupResponses ?? []).flatMap((response, index) => response ? [index] : []);
  }
  if (pending.groupCitationFormat !== "span-v1" || !Array.isArray(pending.legacyGroupIndexes) ||
    pending.legacyGroupIndexes.some((index) => !Number.isInteger(index) || index < 0 || index >= grouped.length || !pending.groupResponses?.[index] ||
      !pending.groupResponses[index].criteria?.every((criterion) => criterion?.source && criterion.sourceSpanId === undefined))) {
    throw new Error("Acceptance: contract citation checkpoint format changed; human decision required");
  }
  const options = { requireSpanIds: true, legacyIndexes: pending.legacyGroupIndexes, allowMixedScopes: true };
  pending.groupPlanDigest = planDigest;
  pending.groupResponses ??= [];
  await save();
  const outcomes = await Promise.allSettled(grouped.map(async (group, index) => {
    if (pending.groupResponses[index]) return;
    while (true) {
      const retry = pending.citationRetryGrant?.groupIndex === index ? pending.citationRetryGrant
        : pending.groupRetryGrant?.groupIndex === index ? pending.groupRetryGrant : undefined;
      if (retry) {
        if (retry.remaining === 0) throw new Error(`Acceptance: user-authorized contract ${retry === grant ? "citation" : "group"} attempts exhausted; human decision required`);
        // Reserve before launching the agent so an interrupted call cannot be replayed for free.
        retry.remaining--;
        retry.used++;
        await save();
      }
      const response = await propose(group, index, grouped.length);
      try { combineContractParts([group], [response], { requireSpanIds: true, allowMixedScopes: true }); }
      catch (error) {
        if (!retry || (retry === grant && !/sourceSpanId|source quote|assigned section/.test(error.message))) throw error;
        if (retry === grant) pending.citationRetryError = error.message;
        else retry.lastError = error.message.slice(0, 1000);
        await save();
        if (retry.remaining > 0) continue;
        throw new Error(`Acceptance: user-authorized contract ${retry === grant ? "citation" : "group"} attempts exhausted; ${error.message}; human decision required`);
      }
      pending.groupResponses[index] = response;
      if (retry === grant) { delete pending.citationRetryGrant; delete pending.citationRetryError; }
      if (retry === groupGrant) delete pending.groupRetryGrant;
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
  return combineContractParts(grouped, pending.groupResponses, options);
}

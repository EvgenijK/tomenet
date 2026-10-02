import { digest, validateContract } from "./acceptance.mjs";

// A reconciler can replace or add requirements, but cannot silently remove
// requirements or downgrade a mandatory one. Full validation and independent
// source verification still follow this draft-only normalization.
export function applyContractReconciliation(draft, patch, sources) {
  if (!patch || typeof patch.summary !== "string" || !patch.summary.trim() ||
    (patch.completionScope !== undefined && !["implementation", "full_acceptance"].includes(patch.completionScope)) ||
    !["criterionEdits", "checkEdits", "criteriaToAdd", "checksToAdd"].every((key) => Array.isArray(patch[key]))) throw new Error("Malformed contract reconciliation");
  const result = structuredClone(draft);
  if (patch.completionScope) result.completionScope = patch.completionScope;
  const criteria = new Map(result.criteria.map((item) => [item.id, item]));
  const checks = new Map(result.checks.map((item) => [item.id, item]));
  const editedCriteria = new Set(); const editedChecks = new Set();
  for (const edit of patch.criterionEdits) {
    const original = criteria.get(edit?.id);
    if (!original || editedCriteria.has(edit.id) || edit.replacement?.id !== edit.id || (original.mandatory && edit.replacement.mandatory !== true)) throw new Error("Invalid or weakened contract criterion edit");
    editedCriteria.add(edit.id);
    result.criteria[result.criteria.findIndex((item) => item.id === edit.id)] = edit.replacement;
  }
  for (const edit of patch.checkEdits) {
    if (!checks.has(edit?.id) || editedChecks.has(edit.id) || edit.replacement?.id !== edit.id) throw new Error("Invalid contract check edit");
    editedChecks.add(edit.id);
    result.checks[result.checks.findIndex((item) => item.id === edit.id)] = edit.replacement;
  }
  for (const criterion of patch.criteriaToAdd) {
    if (!criterion?.id || criteria.has(criterion.id)) throw new Error("Duplicate contract criterion addition");
    criteria.set(criterion.id, criterion); result.criteria.push(criterion);
  }
  for (const check of patch.checksToAdd) {
    if (!check?.id || checks.has(check.id)) throw new Error("Duplicate contract check addition");
    checks.set(check.id, check); result.checks.push(check);
  }
  try { validateContract(result, sources); }
  catch (error) { throw new Error(`Invalid draft reconciliation: ${error.message.replace(/^Acceptance:\s*/, "")}`); }
  return result;
}

export async function reconcileContractDraft(pending, draft, sources, inspect, save) {
  const sourceDigest = digest(sources);
  const draftDigest = digest(draft);
  if (pending.reconciliation && (pending.reconciliationDraftDigest !== draftDigest || pending.reconciliationSourceDigest !== sourceDigest)) throw new Error("Contract reconciliation checkpoint changed; human decision required");
  if (!pending.reconciliation) {
    const patch = await inspect(draft);
    applyContractReconciliation(draft, patch, sources);
    pending.reconciliation = patch;
    pending.reconciliationDraftDigest = draftDigest;
    pending.reconciliationSourceDigest = sourceDigest;
    await save();
  }
  return applyContractReconciliation(draft, pending.reconciliation, sources);
}

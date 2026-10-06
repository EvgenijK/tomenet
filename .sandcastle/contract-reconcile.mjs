import { digest, validateContract } from "./acceptance.mjs";

export const assemblyCommandRules = "The sv-build and sv-core baseline checks are immutable: preserve their IDs, kinds and commands exactly, and never include them in checkEdits. If stronger regression evidence is needed, add a separate focused check instead of changing a baseline check. Preserve every other already valid check unless a requirement needs its change. New or edited focused checks must use exactly python3 -B tests/<file> [args], node tests/<file> [args], or bash tests/<file> [args]. When one required check needs multiple repository tests, use a non-empty command array; the controller runs it in order and stops on the first failure. External checks must have command=''. Never use shell operators, wrappers, or a command that does not directly invoke a repository test.";

// A reconciler can replace or add requirements, but cannot silently remove
// requirements or downgrade a mandatory one. Full validation and independent
// source verification still follow this draft-only normalization.
export function applyContractReconciliation(draft, patch, sources) {
  if (!patch || typeof patch.summary !== "string" || !patch.summary.trim() ||
    (patch.completionScope != null && !["implementation", "full_acceptance"].includes(patch.completionScope)) ||
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
    const attempts = pending.assemblyAttempts ??= { round: pending.round, groupPlanDigest: pending.groupPlanDigest,
      granted: 3, remaining: 3, used: 0, draftDigest, sourceDigest };
    const extensions = attempts.extensions ?? [];
    const validExtensions = Array.isArray(extensions) && extensions.every((item) =>
      typeof item.id === "string" && /^[a-zA-Z0-9_-]{8,80}$/.test(item.id) &&
      Number.isInteger(item.extra) && item.extra >= 1 && item.extra <= 16 &&
      typeof item.authorizedAt === "string" && item.authorizedAt.length > 0) &&
      new Set(extensions.map((item) => item.id)).size === extensions.length;
    const baseGranted = attempts.baseGranted ?? attempts.granted;
    if (attempts.round !== pending.round || attempts.groupPlanDigest !== pending.groupPlanDigest ||
      (attempts.draftDigest && attempts.draftDigest !== draftDigest) ||
      (attempts.sourceDigest && attempts.sourceDigest !== sourceDigest) ||
      !validExtensions || !Number.isInteger(baseGranted) || baseGranted < 1 || baseGranted > 5 ||
      attempts.granted !== baseGranted + extensions.reduce((sum, item) => sum + item.extra, 0) ||
      !Number.isInteger(attempts.used) || attempts.used < 0 ||
      !Number.isInteger(attempts.remaining) || attempts.remaining < 0 ||
      attempts.used + attempts.remaining !== attempts.granted) {
      throw new Error("Acceptance: contract assembly attempts no longer match the saved checkpoint; human decision required");
    }
    attempts.draftDigest ??= draftDigest;
    attempts.sourceDigest ??= sourceDigest;
    await save();
    while (!pending.reconciliation) {
      if (attempts.remaining === 0) throw new Error(`Acceptance: contract assembly attempts exhausted; ${attempts.lastError ?? "no valid correction saved"}; human decision required`);
      // Reserve before the model call, so a timeout or interrupted call cannot be replayed for free.
      attempts.remaining--;
      attempts.used++;
      await save();
      const patch = await inspect(draft, attempts.lastError);
      try { applyContractReconciliation(draft, patch, sources); }
      catch (error) {
        attempts.lastError = error.message.slice(0, 1000);
        await save();
        continue;
      }
      pending.reconciliation = patch;
      pending.reconciliationDraftDigest = draftDigest;
      pending.reconciliationSourceDigest = sourceDigest;
      delete attempts.lastError;
      await save();
    }
  }
  return applyContractReconciliation(draft, pending.reconciliation, sources);
}

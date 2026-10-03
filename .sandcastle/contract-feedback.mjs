import { partitionContractSources, sourceSpans } from "./contract-draft.mjs";

export function contractSourceCatalog(sources) {
  return sourceSpans(partitionContractSources(sources)).flat();
}

// A verifier finding is local only when every cited span has one group owner.
// Missing, unknown, or cross-group provenance stays with full-source assembly.
export function verifierFeedbackDetails(review, sources) {
  const known = new Set(contractSourceCatalog(sources).map((span) => span.id));
  const findings = Array.isArray(review?.findings) ? review.findings : [];
  return (review?.missingRequirements ?? []).map((message) => {
    const matches = findings.filter((finding) => finding?.message === message);
    const ids = matches.length === 1 && Array.isArray(matches[0].sourceSpanIds) ? matches[0].sourceSpanIds : [];
    return { message, sourceSpanIds: ids.length && ids.every((id) => known.has(id)) && new Set(ids).size === ids.length ? ids : [] };
  });
}

export function feedbackForGroup(pending, group) {
  const assigned = new Set(group.sourceSpans.map((span) => span.id));
  return (pending.feedbackDetails ?? []).filter((item) => item.sourceSpanIds?.length &&
    item.sourceSpanIds.every((id) => assigned.has(id))).map((item) => item.message);
}

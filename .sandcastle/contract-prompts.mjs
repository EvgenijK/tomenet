export const controllerExecutionFacts = Object.freeze([
  "Controller execution fact: baseline checks are immutable contract infrastructure; contract agents may reference them but must not redefine, replace, or repair them.",
  "Controller execution fact: the sv-core check is executed with .sandcastle/checks.sh supplied through standard input by the controller.",
  "Controller execution fact: a check's displayed command is an identifier for the controller runner, not necessarily the complete shell invocation. Do not infer missing execution behavior from that display string alone.",
  "Controller execution fact: controller-review is a real controller-owned gate. It passes only when every mapped criterion is evidenced as passed by both review and fresh final audit at the exact contract digest and HEAD; it has no shell command.",
]);

const commonScopeGuidance = Object.freeze([
  "Preserve every obligation supported by the assigned authoritative sources, including baseline behavior, production seams, checks, evidence, deferral owners, and repository isolation rules. Do not invent obligations or import requirements from unrelated work.",
  "Keep implementation readiness separate from full acceptance. Every mandatory current criterion in an implementation contract must map only to runnable build, core, or focused repository command checks; external checks may be current only for full_acceptance.",
  "A focused command check may reference only a repository test present in the controller-supplied runner inventory at contract time. Never invent a future test path; missing implementation evidence remains for the controller review/final-audit stages or an existing broader runner.",
  "Code inspection, diff review, repository-policy review, workflow-policy review, and production-seam review are performed by the controller-review gate during later review and fresh final audit. Map those criteria to controller-review, optionally alongside runnable implementation checks; never disguise policy evidence as sv-core or as an external blocker.",
  "Later caller, downstream integration, native-platform, or genuinely unavailable environment evidence may be a deferred external check only when its exact owner and deferral reason are recorded. Deferred external evidence is not passing evidence.",
]);

export function groupContractPromptGuidance({ usesSourceSpans = true, version = 2 } = {}) {
  return [
    version === 2
      ? "For every criterion, select sourceSpanIds only from this group's assigned sourceSpans. Store IDs only: never include source.path, source.quote, or fragment text. Use the controller-supplied fragment reader script to retrieve text; do not read or assemble fragment files yourself. Requirements spanning groups belong in full-source reconciliation."
      : usesSourceSpans
      ? "For every criterion, select sourceSpanId only from this group's assigned sourceSpans. Do not provide source.path or source.quote; the controller inserts the exact source text. Requirements spanning groups belong in full-source reconciliation."
      : "For every criterion, quote only the supplied source sections verbatim in source.path and source.quote.",
    ...commonScopeGuidance,
    "Use short IDs unique within this group. Include the immutable baseline checks exactly as supplied and add only checks required by the assigned sources.",
  ];
}

export function reconciliationContractPromptGuidance({ version = 2 } = {}) {
  return [
    ...(version === 2 ? ["All criterion additions and replacements must contain sourceSpanIds only. Retrieve referenced text with the controller-supplied fragment reader script; never inline source paths or quotes or assemble files yourself."] : []),
    "Reconcile the grouped drafts only against the fixed authoritative source set. Resolve cross-group inconsistencies and every saved verifier finding without broadening the originating task.",
    "Each retry is applied to the unchanged original grouped draft, never to a previous rejected patch. Return one complete patch that repairs all known validation failures together; partial fixes from rejected attempts are not accumulated.",
    "Repair all criterion IDs named in validation feedback in the same patch. Every criterion, including deferred criteria, must map to at least one valid check ID; add a focused or external check only when the authoritative requirement needs it.",
    "Preserve criteria already supported by authoritative sources. Do not add criteria unsupported by that source set; replace an inaccurate criterion only when saved verifier feedback justifies the correction.",
    "Keep production implementation criteria distinct from deferred native, platform, and later-caller evidence. Deferred criteria must retain an exact owner and reason.",
    ...controllerExecutionFacts,
  ];
}

export function verifierContractPromptGuidance({ version = 2 } = {}) {
  return [
    ...(version === 2 ? ["Resolve criterion sourceSpanIds with the controller-supplied fragment reader script. Do not fetch or combine fragment files yourself; keep the proposal free of inline source paths and quotes."] : []),
    "Verify the proposal only against the fixed authoritative source set supplied by the controller. Other documents may provide context but cannot introduce new contract obligations unless the controller includes them in that set.",
    "Every missing or unsupported requirement must cite its authoritative source span. Use an empty span list only for a genuinely global consistency issue, never to import a requirement from outside the supplied sources.",
    "Check that mandatory behavior is testable and that deferred evidence has an exact owner. Do not treat the absence of a native or external runner as passing evidence.",
    ...controllerExecutionFacts,
  ];
}

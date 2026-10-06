export const controllerExecutionFacts = Object.freeze([
  "Controller execution fact: baseline checks are immutable contract infrastructure; contract agents may reference them but must not redefine, replace, or repair them.",
  "Controller execution fact: the sv-core check is executed with .sandcastle/checks.sh supplied through standard input by the controller.",
  "Controller execution fact: a check's displayed command is an identifier for the controller runner, not necessarily the complete shell invocation. Do not infer missing execution behavior from that display string alone.",
  "Controller execution fact: when one required check needs multiple repository tests, its command is a non-empty array of individually valid commands; the controller runs them in order and stops on the first failure.",
]);

const commonScopeGuidance = Object.freeze([
  "Preserve every obligation supported by the assigned authoritative sources, including baseline behavior, production seams, checks, evidence, deferral owners, and repository isolation rules. Do not invent obligations or import requirements from unrelated work.",
  "Keep implementation readiness separate from full acceptance. Every mandatory current criterion in an implementation contract must map only to runnable build, core, or focused repository command checks; external checks may be current only for full_acceptance.",
  "A repository feature test expected at implementation completion is a current command check. Its command must name an existing repository runner script; implementation tickets extend that runner through the production path before the gate runs.",
  "Code inspection, diff review, policy review, and production-seam review are performed by the later review and final-audit stages. Represent their substantive requirements as criteria backed by runnable implementation checks, not as external check blockers.",
  "Later caller, downstream integration, native-platform, or genuinely unavailable environment evidence may be a deferred external check only when its exact owner and deferral reason are recorded. Deferred external evidence is not passing evidence.",
]);

export function groupContractPromptGuidance({ usesSourceSpans = true } = {}) {
  return [
    usesSourceSpans
      ? "For every criterion, select sourceSpanId only from this group's assigned sourceSpans. Do not provide source.path or source.quote; the controller inserts the exact source text. Requirements spanning groups belong in full-source reconciliation."
      : "For every criterion, quote only the supplied source sections verbatim in source.path and source.quote.",
    ...commonScopeGuidance,
    "Use short IDs unique within this group. Include the immutable baseline checks exactly as supplied and add only checks required by the assigned sources.",
  ];
}

export function reconciliationContractPromptGuidance() {
  return [
    "Reconcile the grouped drafts only against the fixed authoritative source set. Resolve cross-group inconsistencies and every saved verifier finding without broadening the originating task.",
    "Preserve criteria already supported by authoritative sources. Do not add criteria unsupported by that source set; replace an inaccurate criterion only when saved verifier feedback justifies the correction.",
    "Keep production implementation criteria distinct from deferred native, platform, and later-caller evidence. Deferred criteria must retain an exact owner and reason.",
    ...controllerExecutionFacts,
  ];
}

export function verifierContractPromptGuidance() {
  return [
    "Verify the proposal only against the fixed authoritative source set supplied by the controller. Other documents may provide context but cannot introduce new contract obligations unless the controller includes them in that set.",
    "Every missing or unsupported requirement must cite its authoritative source span. Use an empty span list only for a genuinely global consistency issue, never to import a requirement from outside the supplied sources.",
    "Check that mandatory behavior is testable and that deferred evidence has an exact owner. Do not treat the absence of a native or external runner as passing evidence.",
    ...controllerExecutionFacts,
  ];
}

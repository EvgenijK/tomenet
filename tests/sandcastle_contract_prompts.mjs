import assert from "node:assert/strict";
import test from "node:test";
import {
  controllerExecutionFacts,
  groupContractPromptGuidance,
  reconciliationContractPromptGuidance,
  verifierContractPromptGuidance,
} from "../.sandcastle/contract-prompts.mjs";

function allGuidance() {
  return [
    ...groupContractPromptGuidance({ usesSourceSpans: true }),
    ...groupContractPromptGuidance({ usesSourceSpans: false }),
    ...reconciliationContractPromptGuidance(),
    ...verifierContractPromptGuidance(),
  ].join("\n");
}

test("contract prompt guidance is neutral to originating tasks", () => {
  const guidance = allGuidance();
  assert.doesNotMatch(guidance, /Guide|SV-B-011|SV-B-008/);
  assert.match(guidance, /fixed authoritative source set/);
  assert.match(guidance, /Do not invent obligations/);
});

test("group guidance selects the controller citation format", () => {
  assert.match(groupContractPromptGuidance({ usesSourceSpans: true }).join("\n"), /select sourceSpanId/);
  assert.doesNotMatch(groupContractPromptGuidance({ usesSourceSpans: true }).join("\n"), /quote only the supplied/);
  assert.match(groupContractPromptGuidance({ usesSourceSpans: false }).join("\n"), /quote only the supplied source sections/);
});

test("verifier guidance explains immutable baseline execution", () => {
  const facts = controllerExecutionFacts.join("\n");
  const verifier = verifierContractPromptGuidance().join("\n");
  assert.match(facts, /baseline checks are immutable/);
  assert.match(facts, /sv-core/);
  assert.match(facts, /\.sandcastle\/checks\.sh supplied through standard input/);
  assert.match(facts, /not necessarily the complete shell invocation/);
  assert.match(facts, /non-empty array/);
  assert.match(facts, /stops on the first failure/);
  for (const fact of controllerExecutionFacts) assert.match(verifier, new RegExp(fact.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")));
});

test("implementation guidance requires executable evidence and leaves review to audit stages", () => {
  const guidance = allGuidance();
  assert.match(guidance, /mandatory current criterion in an implementation contract must map only to runnable build, core, or focused repository command checks/);
  assert.match(guidance, /expected at implementation completion is a current command check/);
  assert.match(guidance, /existing repository runner script/);
  assert.match(guidance, /review and final-audit stages/);
  assert.match(guidance, /external checks may be current only for full_acceptance/);
  assert.match(guidance, /deferred external check only when its exact owner and deferral reason are recorded/);
  assert.doesNotMatch(guidance, /current implementation criterion needs executable evidence or an explicit external blocker/);
});

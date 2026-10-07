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

test("reconciliation guidance requires one complete patch against the original draft", () => {
  const guidance = reconciliationContractPromptGuidance().join("\n");
  assert.match(guidance, /unchanged original grouped draft/);
  assert.match(guidance, /one complete patch/);
  assert.match(guidance, /all criterion IDs named in validation feedback/);
  assert.match(guidance, /including deferred criteria/);
  assert.match(guidance, /at least one valid check ID/);
});

test("verifier guidance explains immutable baseline execution", () => {
  const facts = controllerExecutionFacts.join("\n");
  const verifier = verifierContractPromptGuidance().join("\n");
  assert.match(facts, /baseline checks are immutable/);
  assert.match(facts, /sv-core/);
  assert.match(facts, /\.sandcastle\/checks\.sh supplied through standard input/);
  assert.match(facts, /not necessarily the complete shell invocation/);
  for (const fact of controllerExecutionFacts) assert.match(verifier, new RegExp(fact.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")));
});

test("implementation guidance never turns a mandatory current check into unrunnable external evidence", () => {
  const guidance = allGuidance();
  assert.match(guidance, /mandatory current criterion/);
  assert.match(guidance, /runnable build, core, or focused repository command checks/);
  assert.match(guidance, /Deferred external evidence is not passing evidence/);
  assert.match(guidance, /review and fresh final audit/);
  assert.match(guidance, /controller-review/);
  assert.match(guidance, /exact contract digest and HEAD/);
  assert.match(guidance, /never disguise policy evidence as sv-core/);
  assert.match(guidance, /Never invent a future test path/);
});

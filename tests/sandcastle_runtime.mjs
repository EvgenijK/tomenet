import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { sandcastleAgentSettings, sandcastleModel, sandcastleRunAgentPolicy, readOnlyAgentArgs, sandboxAgentArgs, delegatedAgentPolicy } from "../.sandcastle/runtime.mjs";

test("saved run policy controls future dispatch and rejects stale settings", () => {
  const policy = structuredClone(sandcastleRunAgentPolicy);
  assert.deepEqual(sandcastleAgentSettings("implementation Ticket88", {}, policy), { model: "gpt-6.1-sol", effort: "medium" });
  assert.deepEqual(sandcastleAgentSettings("recovery orchestrator 13", {}, policy), { model: "gpt-6.1-sol", effort: "high" });
  for (const stale of [{ ...policy, model: "gpt-6-astra" }, { ...policy, agentEffort: "high" }, { ...policy, orchestratorEffort: "medium" }]) {
    assert.throws(() => sandcastleAgentSettings("implementation Ticket88", {}, stale), /Saved Sandcastle agent policy conflicts/);
  }
  assert.deepEqual(policy, sandcastleRunAgentPolicy);
});

test("all Sandcastle dispatch roles use GPT-6.1 Sol with their requested effort", () => {
  for (const role of ["orchestrator", "orchestrator 1", "recovery orchestrator 42"]) {
    assert.deepEqual(sandcastleAgentSettings(role, {}), { model: "gpt-6.1-sol", effort: "high" });
  }
  for (const role of ["ticket planning", "implementation .scratch/run/issues/ticket.md", "assembly wave 3",
    "acceptance contract semantic amendment", "acceptance contract amendment", "acceptance contract group 2/3",
    "acceptance contract split", "acceptance contract reconciliation", "independent contract verification",
    "review 40", "fresh final acceptance auditor", "focused coverage", "future inspector", "orchestrator-like inspector"]) {
    assert.deepEqual(sandcastleAgentSettings(role, {}), { model: "gpt-6.1-sol", effort: "medium" });
  }
});

test("legacy overrides cannot silently change the model or flatten the effort split", () => {
  for (const env of [{ SANDCASTLE_MODEL: "gpt-5.6-sol" }, { SANDCASTLE_REASONING_EFFORT: "high" }]) {
    assert.throws(() => sandcastleAgentSettings("review 1", env), /conflicts.*unset SANDCASTLE_/);
  }
  const env = { SANDCASTLE_MODEL: sandcastleModel, SANDCASTLE_REASONING_EFFORT: "medium" };
  assert.equal(sandcastleAgentSettings("recovery orchestrator 1", env).effort, "high");
  assert.equal(sandcastleAgentSettings("review 1", env).effort, "medium");
});

test("host and container CLI arguments carry role settings and ignore stale user profiles", () => {
  for (const factory of [readOnlyAgentArgs, sandboxAgentArgs]) {
    for (const role of ["recovery orchestrator 1", "implementation ticket", "review 1"]) {
      const settings = sandcastleAgentSettings(role, {});
      const args = factory({ ...settings, schemaPath: "/schema.json" });
      assert.equal(args[args.indexOf("-m") + 1], "gpt-6.1-sol");
      assert.ok(args.includes(`model_reasoning_effort=${JSON.stringify(settings.effort)}`));
      assert.ok(args.includes("--ignore-user-config"));
      assert.equal(args[args.indexOf("--output-schema") + 1], "/schema.json");
    }
  }
  assert.equal(readOnlyAgentArgs({ ...sandcastleAgentSettings("review 1", {}), schemaPath: "/schema.json" }).at(-1), "-");
  assert.ok(!sandboxAgentArgs(sandcastleAgentSettings("ticket planning", {})).includes("--output-schema"));
  assert.match(delegatedAgentPolicy, /gpt-6\.1-sol with medium reasoning effort/);
});

test("production host and sandbox dispatches select by role instead of global effort", async () => {
  const workflow = await readFile(new URL("../.sandcastle/workflow.mjs", import.meta.url), "utf8");
  assert.match(workflow, /sandboxAgentArgs\(\{ \.\.\.sandcastleAgentSettings\(role, process\.env, state\.agentPolicy\)/);
  assert.match(workflow, /readOnlyAgentArgs\(\{ \.\.\.sandcastleAgentSettings\(role, process\.env, state\.agentPolicy\)/);
  assert.ok(!workflow.includes("SANDCASTLE_REASONING_EFFORT"));
});

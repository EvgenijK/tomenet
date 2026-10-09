import { basename, resolve } from "node:path";

export const sandcastleModel = "gpt-6.1-sol";

// The workflow controller is deterministic JavaScript. Its only current AI
// orchestrator is recovery; host inspectors are ordinary agents, not controllers.
export const sandcastleRunAgentPolicy = Object.freeze({ model: sandcastleModel, orchestratorEffort: "high", agentEffort: "medium" });

export function sandcastleAgentSettings(role, env = process.env, runPolicy) {
  if (runPolicy && Object.entries(sandcastleRunAgentPolicy).some(([name, expected]) => runPolicy[name] !== expected)) {
    throw new Error("Saved Sandcastle agent policy conflicts with the current runtime; apply the authorized policy before continuing.");
  }
  for (const [name, expected] of [["SANDCASTLE_MODEL", sandcastleModel], ["SANDCASTLE_REASONING_EFFORT", "medium"]]) {
    if (env[name] && env[name] !== expected) {
      throw new Error(`${name}=${env[name]} conflicts with Sandcastle's fixed agent policy; unset ${name}. All agents use ${sandcastleModel}, orchestrators high and other roles medium.`);
    }
  }
  const orchestrator = /^(?:recovery orchestrator(?: |$)|orchestrator(?: |$))/.test(role);
  const policy = runPolicy ?? sandcastleRunAgentPolicy;
  return { model: policy.model, effort: orchestrator ? policy.orchestratorEffort : policy.agentEffort };
}

export const delegatedAgentPolicy = `Any Sandcastle subagents you delegate to must use model ${sandcastleModel} with medium reasoning effort. Inherit these settings; do not select a different model or effort.`;

export function sandboxAgentArgs({ model, effort, schemaPath }) {
  return ["exec", "--json", "--ephemeral", "--ignore-user-config", "--enable", "multi_agent",
    "-s", "danger-full-access", "-c", 'approval_policy="never"', "-m", model,
    "-c", `model_reasoning_effort=${JSON.stringify(effort)}`,
    ...(schemaPath ? ["--output-schema", schemaPath] : []), "-"];
}

// Sandcastle creates parents of individual file mounts only inside this home.
const schemaDirectory = "/home/agent/.sandcastle-runtime";
export function runtimeSchemaPath(schema) {
  return `${schemaDirectory}/${basename(schema === true ? "review-output.schema.json" : schema)}`;
}
export function runtimeSchemaMounts(root) {
  return ["contract-groups-output", "contract-group-output", "contract-output", "contract-review-output", "review-output", "ticket-output"].map((name) => ({
    hostPath: resolve(root, `.sandcastle/${name}.schema.json`),
    sandboxPath: runtimeSchemaPath(`${name}.schema.json`), readonly: true,
  }));
}

export function readOnlyAgentArgs({ model, effort, schemaPath, multiAgent = false }) {
  return ["exec", "--json", "--ephemeral", "--ignore-user-config", "--ignore-rules", multiAgent ? "--enable" : "--disable", "multi_agent",
    "-s", "read-only", "-c", 'approval_policy="never"', "-m", model,
    "-c", `model_reasoning_effort=${JSON.stringify(effort)}`, "--output-schema", schemaPath, "-"];
}

// Contract and final-audit inspectors include repository reading and structured output.
// Ordinary review has no deadline; recovery receives its own longer deadline.
export const readOnlyAgentTimeoutMs = 1800000;
export const recoveryAgentTimeoutMs = 6000000;

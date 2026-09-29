import { basename, resolve } from "node:path";

// Sandcastle creates parents of individual file mounts only inside this home.
const schemaDirectory = "/home/agent/.sandcastle-runtime";
export function runtimeSchemaPath(schema) {
  return `${schemaDirectory}/${basename(schema === true ? "review-output.schema.json" : schema)}`;
}
export function runtimeSchemaMounts(root) {
  return ["contract-output", "contract-review-output", "review-output", "ticket-output"].map((name) => ({
    hostPath: resolve(root, `.sandcastle/${name}.schema.json`),
    sandboxPath: runtimeSchemaPath(`${name}.schema.json`), readonly: true,
  }));
}

export function readOnlyAgentArgs({ model, effort, schemaPath, multiAgent = false }) {
  return ["exec", "--json", "--ephemeral", "--ignore-user-config", "--ignore-rules", multiAgent ? "--enable" : "--disable", "multi_agent",
    "-s", "read-only", "-c", 'approval_policy="never"', "-m", model,
    "-c", `model_reasoning_effort=${JSON.stringify(effort)}`, "--output-schema", schemaPath, "-"];
}

// Host inspection includes repository reading and structured output generation.
// A bounded ten-minute deadline replaces the former four-minute cutoff.
export const readOnlyAgentTimeoutMs = 600000;

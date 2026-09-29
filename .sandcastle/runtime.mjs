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

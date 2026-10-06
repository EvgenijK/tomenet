import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

export async function runAcceptanceCheck(sandbox, check, execute) {
  const script = check.kind === "core"
    ? await readFile(resolve(sandbox.worktreePath, ".sandcastle/checks.sh"), "utf8")
    : undefined;
  if (!Array.isArray(check.command)) return execute(sandbox, check.command, script);
  const outputs = [];
  let result;
  for (const command of check.command) {
    result = await execute(sandbox, command, script);
    outputs.push(result.output ?? "");
    if (!result.ok) break;
  }
  return { ...result, output: outputs.join("\n") };
}

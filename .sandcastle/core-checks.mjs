import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

export async function runAcceptanceCheck(sandbox, check, execute) {
  const script = check.kind === "core"
    ? await readFile(resolve(sandbox.worktreePath, ".sandcastle/checks.sh"), "utf8")
    : undefined;
  return execute(sandbox, check.command, script);
}

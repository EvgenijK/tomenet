import { readFile } from "node:fs/promises";
import { resolve } from "node:path";
import { createHash } from "node:crypto";

// Repository-owned gate input must come from the same worktree as checked code.
export async function repositoryGateInput(worktree, check) {
  if (check.kind !== "core") return {};
  const runnerPath = ".sandcastle/checks.sh";
  const stdin = await readFile(resolve(worktree, runnerPath), "utf8");
  return { stdin, runnerPath, runnerDigest: createHash("sha256").update(stdin).digest("hex") };
}

import { access } from "node:fs/promises";
import { resolve } from "node:path";

const runnerPattern = /^(?:python3 -B|node|bash) (tests\/[a-zA-Z0-9_./-]+)(?: [a-zA-Z0-9_=./:-]+)*$/;

export function focusedCommands(check) {
  return Array.isArray(check.command) ? check.command : [check.command];
}

export function focusedRunnerPath(command) {
  return typeof command === "string" ? runnerPattern.exec(command)?.[1] : undefined;
}

export async function missingContractRunners(contract, worktree, checkAccess = access) {
  const missing = [];
  for (const check of contract.checks) {
    if (check.kind !== "command") continue;
    for (const command of focusedCommands(check)) {
      const runner = focusedRunnerPath(command);
      if (!runner) { missing.push({ checkId: check.id, command, runner: "" }); continue; }
      try { await checkAccess(resolve(worktree, runner)); }
      catch { missing.push({ checkId: check.id, command, runner }); }
    }
  }
  return missing;
}

export async function validateContractRunners(contract, worktree, checkAccess = access) {
  const [missing] = await missingContractRunners(contract, worktree, checkAccess);
  if (missing) {
    if (!missing.runner) throw new Error(`Acceptance: focused runner for check ${missing.checkId} is not statically identifiable`);
    throw new Error(`Acceptance: focused runner ${missing.runner} for check ${missing.checkId} does not exist`);
  }
  return contract;
}

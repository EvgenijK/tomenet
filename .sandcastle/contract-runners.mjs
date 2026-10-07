import { readdir, readFile } from "node:fs/promises";
import { relative, resolve } from "node:path";

const runnerPattern = /^(?:python3 -B|node|bash) (tests\/[a-zA-Z0-9_./-]+)(?: [a-zA-Z0-9_=./:-]+)*$/;

export function focusedRunnerPath(command) {
  return typeof command === "string" ? runnerPattern.exec(command)?.[1] : undefined;
}

async function walkFiles(root, directory, output) {
  for (const entry of await readdir(directory, { withFileTypes: true })) {
    const path = resolve(directory, entry.name);
    if (entry.isDirectory()) await walkFiles(root, path, output);
    else if (entry.isFile()) output.push(relative(root, path).replaceAll("\\", "/"));
  }
}

export async function contractExecutionContext(worktree, controllerRoot = worktree) {
  const files = [];
  await walkFiles(worktree, resolve(worktree, "tests"), files);
  files.sort();
  const focusedCommands = files.flatMap((path) => {
    if (path.endsWith(".py")) return [`python3 -B ${path}`];
    if (path.endsWith(".mjs") || path.endsWith(".js")) return [`node ${path}`];
    if (path.endsWith(".sh")) return [`bash ${path}`];
    return [];
  });
  const focusedRunnerPaths = focusedCommands.map((command) => focusedRunnerPath(command));
  const coreScript = await readFile(resolve(controllerRoot, ".sandcastle/checks.sh"), "utf8");
  const coreRunnerPaths = [...new Set(coreScript.match(/tests\/[a-zA-Z0-9_./-]+/g) ?? [])].sort();
  return {
    focusedRunnerPaths,
    focusedCommands,
    controllerChecks: [
      { id: "sv-build", kind: "build", evidence: "Controller build result at exact HEAD." },
      { id: "sv-core", kind: "core", evidence: "Controller executes .sandcastle/checks.sh core at exact HEAD.", expandedRunnerPaths: coreRunnerPaths },
      { id: "controller-review", kind: "review", evidence: "Controller requires each mapped criterion to pass both review and fresh final audit at the exact contract digest and HEAD." },
    ],
  };
}

export function validateContractRunnerInventory(contract, focusedRunnerPaths) {
  const available = new Set(focusedRunnerPaths);
  for (const check of contract.checks) {
    if (check.kind !== "command") continue;
    const runner = focusedRunnerPath(check.command);
    if (!runner) throw new Error(`Acceptance: focused runner for check ${check.id} is not statically identifiable`);
    if (!available.has(runner)) throw new Error(`Acceptance: focused runner ${runner} for check ${check.id} does not exist`);
  }
  return contract;
}

import { copyFile, mkdir, readFile } from "node:fs/promises";
import { randomUUID } from "node:crypto";
import { resolve } from "node:path";
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import { createSandbox } from "@ai-hero/sandcastle";
import { docker } from "@ai-hero/sandcastle/sandboxes/docker";

const execFileAsync = promisify(execFile);
const root = resolve(import.meta.dirname, "..");
const action = process.argv[2];
if (!["build", "test", "registry"].includes(action)) {
  console.error("Usage: node .sandcastle/main.mjs <build|test|registry>");
  process.exit(2);
}

const branch = `codex/sandcastle-${action}-${randomUUID().slice(0, 8)}`;
const sandbox = await createSandbox({ cwd: root, branch, sandbox: docker() });
async function run(command, stdin) {
  console.log(`$ ${command}`);
  const result = await sandbox.exec(command, { stdin, onLine: (line) => console.log(line) });
  if (result.exitCode !== 0) throw new Error(`${command} exited with status ${result.exitCode}\n${result.stderr}`);
}

try {
  console.log(`Sandcastle branch: ${sandbox.branch}`);
  await run("make -s -C src -f makefile.sv tomenet-sv");
  if (action === "build") {
    const artifacts = resolve(root, ".sandcastle/artifacts");
    await mkdir(artifacts, { recursive: true });
    await copyFile(resolve(sandbox.worktreePath, "src/tomenet-sv"), resolve(artifacts, "tomenet-sv"));
    console.log(`Built executable: ${resolve(artifacts, "tomenet-sv")}`);
  } else {
    const script = await readFile(resolve(root, ".sandcastle/checks.sh"), "utf8");
    await run(`bash -s -- ${action === "test" ? "core" : "registry"}`, script);
  }
} finally {
  const result = await sandbox.close();
  if (result.preservedWorktreePath) console.log(`Uncommitted changes preserved at ${result.preservedWorktreePath}`);
  else await execFileAsync("git", ["branch", "-d", branch], { cwd: root });
}

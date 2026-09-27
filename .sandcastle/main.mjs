import { access, chmod, copyFile, mkdir, readFile } from "node:fs/promises";
import { randomUUID } from "node:crypto";
import { homedir } from "node:os";
import { resolve } from "node:path";
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import { createSandbox, codex } from "@ai-hero/sandcastle";
import { docker } from "@ai-hero/sandcastle/sandboxes/docker";
import { reviewChanges } from "./review.mjs";

const execFileAsync = promisify(execFile);

const root = resolve(import.meta.dirname, "..");
const authDir = resolve(root, ".sandcastle/auth");
const action = process.argv[2];
const hostProxy = process.env.all_proxy || process.env.ALL_PROXY;
if (!["build", "test", "registry", "dev"].includes(action)) {
  console.error("Usage: node .sandcastle/main.mjs <build|test|registry|dev>");
  process.exit(2);
}

const branch = `codex/sandcastle-${action}-${randomUUID().slice(0, 8)}`;
const baseCommit = (await execFileAsync("git", ["rev-parse", "HEAD"], { cwd: root })).stdout.trim();
const specPath = process.env.SANDCASTLE_SPEC || process.env.SANDCASTLE_REVIEW_SPEC;
const skillsRoot = resolve(process.env.SANDCASTLE_SKILLS_ROOT || resolve(homedir(), ".agents/skills"));
const skillPaths = {
  implement: resolve(skillsRoot, "implement"),
  tdd: resolve(skillsRoot, "tdd"),
  "code-review": resolve(process.env.SANDCASTLE_CODE_REVIEW_SKILL_PATH || resolve(skillsRoot, "code-review")),
};
if (action === "dev") {
  if (!specPath) {
    throw new Error("Set SANDCASTLE_SPEC to the task or specification file for $implement and $code-review");
  }
  for (const [name, path] of Object.entries(skillPaths)) {
    try {
      await access(resolve(path, "SKILL.md"));
    } catch {
      throw new Error(`Required Sandcastle skill $${name} not found at ${path}`);
    }
  }
  await access(resolve(root, specPath));
  await mkdir(authDir, { recursive: true, mode: 0o700 });
  await chmod(authDir, 0o700);
}
const sandbox = await createSandbox({
  cwd: root,
  branch,
  sandbox: docker(action === "dev" ? {
    network: "host",
    env: hostProxy ? { all_proxy: hostProxy } : {},
    mounts: [
      ...Object.entries(skillPaths).map(([name, path]) => ({
        hostPath: path,
        sandboxPath: `/home/agent/.agents/skills/${name}`,
        readonly: true,
      })),
      { hostPath: authDir, sandboxPath: "/home/agent/.codex", readonly: false },
    ],
  } : {}),
  ...(action === "dev" ? {
    hooks: { sandbox: { onSandboxReady: [{
      command: 'if [ -n "${OPENAI_API_KEY:-}" ]; then printenv OPENAI_API_KEY | codex login --with-api-key; fi',
    }] } },
  } : {}),
});

async function ensureCodexLogin() {
  const status = await sandbox.exec("codex login status");
  if (status.exitCode === 0) return;
  throw new Error("Codex is not logged in inside Sandcastle; enable ChatGPT device-code login and run npm run sandbox:login, or set OPENAI_API_KEY in .sandcastle/.env");
}

async function exec(command, stdin) {
  console.log(`$ ${command}`);
  const result = await sandbox.exec(command, { stdin, onLine: (line) => console.log(line) });
  if (result.exitCode !== 0) {
    throw new Error(`${command} exited with status ${result.exitCode}\n${result.stderr}`);
  }
}

async function check(mode = "core") {
  const script = await readFile(resolve(root, ".sandcastle/checks.sh"), "utf8");
  await exec(`bash -s -- ${mode}`, script);
}

try {
  console.log(`Sandcastle branch: ${sandbox.branch}`);
  console.log(`Worktree: ${sandbox.worktreePath}`);
  if (action === "dev") await ensureCodexLogin();
  await exec("make -s -C src -f makefile.sv tomenet-sv");

  if (action === "build") {
    const artifacts = resolve(root, ".sandcastle/artifacts");
    await mkdir(artifacts, { recursive: true });
    await copyFile(resolve(sandbox.worktreePath, "src/tomenet-sv"), resolve(artifacts, "tomenet-sv"));
    console.log(`Built executable: ${resolve(artifacts, "tomenet-sv")}`);
  } else if (action === "test") {
    await check();
  } else if (action === "registry") {
    await check("registry");
  } else {
    const model = process.env.SANDCASTLE_MODEL || "gpt-6-sol";
    const reasoningEffort = process.env.SANDCASTLE_REASONING_EFFORT || "high";
    const agent = codex(model, { effort: reasoningEffort });
    const buildInteractiveArgs = agent.buildInteractiveArgs;
    agent.buildInteractiveArgs = (options) => {
      const [program, ...args] = buildInteractiveArgs(options);
      return [program, "--ask-for-approval", "never", "--sandbox", "danger-full-access", "-c", `model_reasoning_effort="${reasoningEffort}"`, ...args];
    };
    const taskPrompt = await readFile(resolve(root, ".sandcastle/dev-prompt.md"), "utf8");
    const specPrompt = [
      "",
      `Originating specification: ${specPath}`,
      await readFile(resolve(root, specPath), "utf8"),
    ].join("\n");
    const result = await sandbox.interactive({
      agent,
      prompt: `${taskPrompt}\nFixed point for code review: ${baseCommit}.${specPrompt}`,
      name: "TomeNET SV development",
    });
    if (result.exitCode !== 0) throw new Error(`Codex exited with status ${result.exitCode}`);
    await exec("make -s -C src -f makefile.sv tomenet-sv");
    await check();
    await reviewChanges({
      root,
      worktreePath: sandbox.worktreePath,
      baseCommit,
      branch: sandbox.branch,
      sandbox,
      model,
      reasoningEffort,
      reviewSpecPath: resolve(root, specPath),
    });
    console.log(`Inspect code review and changes on branch ${sandbox.branch}`);
  }
} finally {
  const result = await sandbox.close();
  if (result.preservedWorktreePath) {
    console.log(`Uncommitted changes preserved at ${result.preservedWorktreePath}`);
  } else if (action !== "dev") {
    await execFileAsync("git", ["branch", "-d", branch], { cwd: root });
  }
}

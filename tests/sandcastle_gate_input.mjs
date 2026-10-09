import assert from "node:assert/strict";
import { mkdtemp, mkdir, writeFile, rm } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { createHash } from "node:crypto";
import test from "node:test";
import { execFileSync } from "node:child_process";
import { repositoryGateInput } from "../.sandcastle/gate-input.mjs";
import { runRequiredCheck } from "../.sandcastle/workflow.mjs";

test("core gate uses checked worktree script, never the launch checkout", async () => {
  const dir = await mkdtemp(join(tmpdir(), "sandcastle-gate-source-"));
  try {
    const launch = join(dir, "launch"), checked = join(dir, "checked");
    for (const path of [launch, checked]) await mkdir(join(path, ".sandcastle"), { recursive: true });
    await writeFile(join(launch, ".sandcastle/checks.sh"), "exit 1 # stale runner\n");
    const script = "printf 'verified runner'\n";
    await writeFile(join(checked, ".sandcastle/checks.sh"), script);
    const input = await repositoryGateInput(checked, { kind: "core" });
    assert.equal(input.stdin, script);
    assert.equal(input.runnerPath, ".sandcastle/checks.sh");
    assert.equal(input.runnerDigest, createHash("sha256").update(script).digest("hex"));
    await writeFile(join(checked, ".sandcastle/checks.sh"), "exit 2\n");
    assert.equal((await repositoryGateInput(checked, { kind: "core" })).stdin, "exit 2\n");
  } finally { await rm(dir, { recursive: true, force: true }); }
});

test("command checks do not read a core script and missing core runner fails", async () => {
  assert.deepEqual(await repositoryGateInput("/does-not-exist", { kind: "command" }), {});
  await assert.rejects(repositoryGateInput("/does-not-exist", { kind: "core" }), { code: "ENOENT" });
});
test("workflow core gate executes and records the checked worktree runner", async () => {
  const dir = await mkdtemp(join(tmpdir(), "sandcastle-real-gate-"));
  try {
    await mkdir(join(dir, ".sandcastle"));
    const script = "printf 'production runner executed'\n";
    await writeFile(join(dir, ".sandcastle/checks.sh"), script);
    const git = (...args) => execFileSync("git", args, { cwd: dir, stdio: "pipe" }).toString().trim();
    git("init", "-q"); git("add", ".sandcastle/checks.sh");
    git("-c", "user.name=Sandcastle Test", "-c", "user.email=test@example.invalid", "commit", "-qm", "runner");
    const sandbox = { worktreePath: dir, exec: async (command, { stdin }) => {
      assert.equal(command, "sh -s");
      return { exitCode: 0, stdout: execFileSync("sh", ["-s"], { cwd: dir, input: stdin }).toString(), stderr: "" };
    } };
    const result = await runRequiredCheck(sandbox, { kind: "core", command: "sh -s" });
    assert.equal(result.ok, true); assert.equal(result.head, git("rev-parse", "HEAD"));
    assert.match(result.output, /production runner executed/);
    assert.equal(result.runnerDigest, createHash("sha256").update(script).digest("hex"));
  } finally { await rm(dir, { recursive: true, force: true }); }
});

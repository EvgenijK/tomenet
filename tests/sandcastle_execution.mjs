import assert from "node:assert/strict";
import { execFile } from "node:child_process";
import { createHash } from "node:crypto";
import { chmod, mkdir, mkdtemp, readFile, rm, symlink, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { resolve } from "node:path";
import test from "node:test";
import { promisify } from "node:util";
import { controllerExecutionIdentity, ExecutionPreflightError, preflightExecution, preflightWorker, verifyCliVersion } from "../.sandcastle/execution.mjs";

const execute = promisify(execFile);
const sha256 = (value) => createHash("sha256").update(value).digest("hex");
async function fixture(t) {
  const directory = await mkdtemp(resolve(tmpdir(), "sandcastle-execution-"));
  t.after(() => rm(directory, { recursive: true, force: true }));
  const root = resolve(directory, "repo");
  await mkdir(resolve(root, ".sandcastle"), { recursive: true });
  const git = async (...args) => (await execute("git", args, { cwd: root })).stdout.trim();
  await git("init", "-b", "integration");
  await git("config", "user.name", "Execution Tests");
  await git("config", "user.email", "execution@example.invalid");
  await writeFile(resolve(root, "package.json"), "{}\n");
  await writeFile(resolve(root, "package-lock.json"), "{}\n");
  await writeFile(resolve(root, ".sandcastle/workflow.mjs"), "// controller\n");
  await writeFile(resolve(root, ".sandcastle/checks.sh"), "#!/bin/sh\nexit 0\n");
  await writeFile(resolve(root, ".sandcastle/ticket-output.schema.json"), '{"type":"object"}\n');
  await git("add", "."); await git("commit", "-m", "Initial base");
  const oldBase = await git("rev-parse", "HEAD");
  await mkdir(resolve(root, "tickets"));
  await writeFile(resolve(root, "tickets/83.md"), "# Ticket 83\nReady for worker.\n");
  await git("add", "."); await git("commit", "-m", "Plan committed ticket");
  const baseCommit = await git("rev-parse", "HEAD");
  const worktree = resolve(directory, "worker");
  await git("worktree", "add", "-b", "worker", worktree, baseCommit);
  const bin = resolve(directory, "bin"); await mkdir(bin);
  const codex = resolve(bin, "codex");
  await writeFile(codex, "#!/bin/sh\nprintf 'codex-cli 0.161.0\\n'\n"); await chmod(codex, 0o755);
  const sandbox = { exec: async (command, options) => {
    assert.equal(command, "node -", "preflight must never perform inference");
    const env = { ...process.env, PATH: `${bin}:${process.env.PATH}` };
    delete env.NODE_TEST_CONTEXT;
    const operation = execute(process.execPath, ["-"], { cwd: worktree, env });
    operation.child.stdin.end(options.stdin);
    try { const output = await operation; return { ...output, exitCode: 0 }; }
    catch (error) { return { stdout: error.stdout, stderr: error.stderr, exitCode: error.code }; }
  } };
  const hostExec = (file, args, options) => file === "codex" ? Promise.resolve({ stdout: "codex-cli 0.161.0\n", stderr: "" }) : execute(file, args, options);
  const input = { controllerRoot: root, worktree, expectedBranch: "worker", baseCommit, ticketPath: "tickets/83.md",
    schemaPath: resolve(root, ".sandcastle/ticket-output.schema.json"), contractDigest: "a".repeat(64), model: "gpt-6.1-sol", effort: "medium", sandbox };
  const ops = { hostExec, sandboxSchemaPath: resolve(worktree, ".sandcastle/ticket-output.schema.json"), imageProvenance: async () => ({ imageId: `sha256:${"b".repeat(64)}`, repoDigests: [] }) };
  return { directory, root, worktree, oldBase, baseCommit, git, input, ops, codex };
}

const code = (expected) => error => error instanceof ExecutionPreflightError && error.deterministic === true && error.code === expected;

test("Codex policy rejects old CLI but accepts declared version and newer stable CLI", () => {
  assert.equal(verifyCliVersion("codex-cli 0.161.0\n"), "0.161.0");
  assert.equal(verifyCliVersion("codex-cli 0.162.1"), "0.162.1");
  assert.equal(verifyCliVersion("codex-cli 1.0.0"), "1.0.0");
  assert.throws(() => verifyCliVersion("codex-cli 0.157.1"), code("CLI_TOO_OLD"));
  assert.throws(() => verifyCliVersion("codex-cli unverified"), code("CLI_VERSION_INVALID"));
});

test("real worker worktree validates branch, wave ancestry and committed ticket before inference", async t => {
  const f = await fixture(t);
  const proof = await preflightWorker(f.input);
  assert.equal(proof.head, f.baseCommit);
  assert.equal(proof.ticketDigest, sha256(await readFile(resolve(f.worktree, "tickets/83.md"))));
  await assert.rejects(preflightWorker({ ...f.input, expectedBranch: "another-worker" }), code("BRANCH_MISMATCH"));
  const stale = resolve(f.directory, "stale");
  await f.git("worktree", "add", "-b", "stale", stale, f.oldBase);
  await assert.rejects(preflightWorker({ ...f.input, worktree: stale, expectedBranch: "stale" }), code("BASE_MISMATCH"));
  await assert.rejects(preflightWorker({ ...f.input, worktree: stale, expectedBranch: "stale", baseCommit: f.oldBase }), code("TICKET_UNREADABLE"));
});

test("missing committed ticket and external symlink cannot satisfy worker readiness", async t => {
  const f = await fixture(t);
  await writeFile(resolve(f.worktree, "tickets/untracked.md"), "Uncommitted\n");
  await assert.rejects(preflightWorker({ ...f.input, ticketPath: "tickets/untracked.md", allowDirty: true }), code("TICKET_NOT_COMMITTED"));
  await symlink(resolve(f.root, "tickets/83.md"), resolve(f.worktree, "tickets/external.md"));
  await assert.rejects(preflightWorker({ ...f.input, ticketPath: "tickets/external.md", allowDirty: true }), code("PATH_INVALID"));
  await assert.rejects(preflightWorker({ ...f.input, ticketPath: "../repo/tickets/83.md", allowDirty: true }), code("PATH_INVALID"));
});

test("actual no-inference sandbox probe verifies mounts and records execution identities", async t => {
  const f = await fixture(t);
  const manifest = await preflightExecution(f.input, f.ops);
  assert.equal(manifest.git.head, f.baseCommit);
  assert.equal(manifest.runtime.cliVersion, "0.161.0");
  assert.equal(manifest.runtime.imageId, `sha256:${"b".repeat(64)}`);
  assert.equal(manifest.host.cliVersion, "0.161.0");
  assert.deepEqual(manifest.agent, { model: "gpt-6.1-sol", effort: "medium" });
  assert.equal(manifest.contractDigest, "a".repeat(64));
  assert.match(manifest.controller.digest, /^[a-f0-9]{64}$/);
  assert.equal(manifest.checks.digest, sha256(await readFile(resolve(f.worktree, ".sandcastle/checks.sh"))));
  assert.equal(JSON.stringify(manifest).includes("PATH"), false);
  const again = await preflightExecution(f.input, f.ops);
  assert.equal(manifest.digest, again.digest);
  assert.equal((await controllerExecutionIdentity(f.root)).digest, manifest.controller.digest);
  await writeFile(resolve(f.root, ".sandcastle/workflow.mjs"), "// updated controller\n");
  const changed = await preflightExecution(f.input, f.ops);
  assert.notEqual(changed.controller.digest, manifest.controller.digest);
  assert.notEqual(changed.digest, manifest.digest);
  await assert.rejects(preflightExecution({ ...f.input, expectedControllerDigest: manifest.controller.digest }, f.ops), code("CONTROLLER_CHANGED"));
});

test("stale sandbox CLI and unreadable schema fail before invoking inference", async t => {
  const f = await fixture(t);
  await writeFile(f.codex, "#!/bin/sh\nprintf 'codex-cli 0.157.1\\n'\n");
  await assert.rejects(preflightExecution(f.input, f.ops), code("CLI_TOO_OLD"));
  await writeFile(f.codex, "#!/bin/sh\nprintf 'codex-cli 0.162.0\\n'\n");
  assert.equal((await preflightExecution(f.input, f.ops)).runtime.cliVersion, "0.162.0");
  await assert.rejects(preflightExecution(f.input, { ...f.ops, sandboxSchemaPath: "/does/not/exist/schema.json" }), code("SANDBOX_INPUT_UNREADABLE"));
});

test("mount and image mismatches return stable sanitized setup failures", async t => {
  const f = await fixture(t);
  const originalExec = f.input.sandbox.exec;
  for (const [field, value, expected] of [["head", "c".repeat(40), "SANDBOX_GIT_MISMATCH"],
    ["ticketDigest", "c".repeat(64), "SANDBOX_TICKET_MISMATCH"], ["schemaDigest", "c".repeat(64), "SANDBOX_SCHEMA_MISMATCH"],
    ["checksDigest", "c".repeat(64), "SANDBOX_CHECKS_MISMATCH"]]) {
    const sandbox = { exec: async (...args) => { const result = await originalExec(...args); const probe = JSON.parse(result.stdout); probe[field] = value; return { ...result, stdout: JSON.stringify(probe) }; } };
    await assert.rejects(preflightExecution({ ...f.input, sandbox }, f.ops), code(expected));
  }
  await assert.rejects(preflightExecution(f.input, { ...f.ops, imageProvenance: async () => { throw new Error("Bearer SECRET /credentials"); } }), error => {
    assert.equal(error.code, "IMAGE_UNVERIFIABLE"); assert.equal(error.message.includes("SECRET"), false); return true;
  });
});

test("host-only dispatch and intentional conflict repair use explicit context", async t => {
  const f = await fixture(t);
  await writeFile(resolve(f.worktree, "repair.txt"), "Preserved conflict repair\n");
  await assert.rejects(preflightExecution({ ...f.input, sandbox: undefined }, f.ops), code("WORKTREE_DIRTY"));
  const manifest = await preflightExecution({ ...f.input, sandbox: undefined, allowDirty: true, effort: "high" }, f.ops);
  assert.equal(manifest.git.dirty, true);
  assert.equal(manifest.runtime.kind, "host");
  assert.equal(manifest.agent.effort, "high");
  await assert.rejects(preflightExecution({ ...f.input, allowDirty: true, model: "gpt-5.6-sol" }, f.ops), code("AGENT_POLICY_INVALID"));
});

test("host networking identifies the Docker container from its hostname mount", async t => {
  const f = await fixture(t);
  const containerId = "d".repeat(64);
  const originalExec = f.input.sandbox.exec;
  const sandbox = { exec: async (command, options) => {
    const injection = `const originalRead = require('node:fs').readFileSync;
require('node:fs').readFileSync = function(path, ...args) {
  if (path === '/proc/self/mountinfo') return ${JSON.stringify(`723 713 259:7 /var/lib/docker/containers/${containerId}/hostname /etc/hostname rw,relatime - ext4 /dev/nvme1n1p2 rw\n`)};
  return originalRead.call(this, path, ...args);
};\n`;
    return originalExec(command, { ...options, stdin: injection + options.stdin });
  } };
  const inspected = [];
  const hostExec = async (file, args, options) => {
    if (file !== "docker") return f.ops.hostExec(file, args, options);
    inspected.push(args);
    if (args[0] === "inspect") {
      assert.equal(args[2], containerId);
      return { stdout: `sha256:${"b".repeat(64)}\n` };
    }
    return { stdout: "[]\n" };
  };
  const manifest = await preflightExecution({ ...f.input, sandbox }, { ...f.ops, hostExec, imageProvenance: undefined });
  assert.equal(manifest.runtime.imageId, `sha256:${"b".repeat(64)}`);
  assert.equal(inspected.length, 2);
});

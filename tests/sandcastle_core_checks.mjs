import assert from "node:assert/strict";
import { mkdir, mkdtemp, rm, writeFile } from "node:fs/promises";
import { resolve } from "node:path";
import test from "node:test";
import { baselineChecks, validateContract } from "../.sandcastle/acceptance.mjs";
import { runAcceptanceCheck } from "../.sandcastle/core-checks.mjs";

test("core checks are read from the integration worktree", async (t) => {
  const fixture = await mkdtemp("/tmp/sandcastle-core-checks-");
  t.after(() => rm(fixture, { recursive: true, force: true }));
  const controllerRoot = resolve(fixture, "controller");
  const worktreePath = resolve(fixture, "integration");
  await Promise.all([
    mkdir(resolve(controllerRoot, ".sandcastle"), { recursive: true }),
    mkdir(resolve(worktreePath, ".sandcastle"), { recursive: true }),
  ]);
  await Promise.all([
    writeFile(resolve(controllerRoot, ".sandcastle/checks.sh"), "echo stale-controller-script\n"),
    writeFile(resolve(worktreePath, ".sandcastle/checks.sh"), "echo audited-integration-script\n"),
  ]);

  let executed;
  const result = await runAcceptanceCheck(
    { worktreePath },
    { id: "sv-core", kind: "core", command: "bash -s -- core" },
    async (sandbox, command, input) => {
      executed = { sandbox, command, input };
      return { ok: true };
    },
  );

  assert.deepEqual(result, { ok: true });
  assert.equal(executed.sandbox.worktreePath, worktreePath);
  assert.equal(executed.command, "bash -s -- core");
  assert.equal(executed.input, "echo audited-integration-script\n");
});

test("a focused check runs every repository command in order and fails as one unit", async () => {
  const commands = ["python3 -B tests/sv_account_create_checks.py", "python3 -B tests/sv_account_failure_checks.py"];
  for (const failingCommand of [commands[0], commands[1], undefined]) {
    const executed = [];
    const result = await runAcceptanceCheck({ worktreePath: "/integration" },
      { id: "account", kind: "command", command: commands }, async (_sandbox, command, input) => {
        executed.push({ command, input });
        return { ok: command !== failingCommand, head: "accepted-head", output: `${command} ${command === failingCommand ? "failed" : "passed"}` };
      });
    const expected = failingCommand === commands[0] ? commands.slice(0, 1) : commands;
    assert.deepEqual(executed, expected.map((command) => ({ command, input: undefined })));
    assert.equal(result.ok, failingCommand === undefined);
  }
});

test("focused checks accept only non-empty sequences of safe repository commands", () => {
  const sources = { "spec.md": "Run production account suites." };
  const candidate = { version: 1, completionScope: "implementation", checks: [...structuredClone(baselineChecks), {
    id: "account", kind: "command", command: ["python3 -B tests/sv_account_create_checks.py", "python3 -B tests/sv_account_failure_checks.py"], description: "Account suites",
  }], criteria: [{ id: "AC-1", requirement: "Run account suites", mandatory: true, applicability: "current", deferralReason: "", owner: "", checkIds: ["account"], source: { path: "spec.md", quote: "Run production account suites." } }] };
  assert.equal(validateContract(candidate, sources), candidate);
  for (const command of [[], [...candidate.checks[2].command, "true"]]) {
    const invalid = structuredClone(candidate); invalid.checks[2].command = command;
    assert.throws(() => validateContract(invalid, sources), /focused check account/);
  }
});

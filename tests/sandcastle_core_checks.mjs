import assert from "node:assert/strict";
import { mkdir, mkdtemp, rm, writeFile } from "node:fs/promises";
import { resolve } from "node:path";
import test from "node:test";
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

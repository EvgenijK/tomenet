import assert from "node:assert/strict";
import { mkdtemp, mkdir, writeFile, rm } from "node:fs/promises";
import { tmpdir } from "node:os";
import { resolve } from "node:path";
import test from "node:test";
import { baselineChecks, controllerReviewCheck } from "../.sandcastle/acceptance.mjs";
import { contractExecutionContext, focusedRunnerPath, validateContractRunnerInventory } from "../.sandcastle/contract-runners.mjs";

const contract = (command) => ({ version: 1, completionScope: "implementation", criteria: [], checks: [
  ...structuredClone(baselineChecks),
  { id: "feature", kind: "command", command, description: "Focused production check" },
] });

test("runner inventory rejects invented and non-script repository paths before verification", async () => {
  const directory = await mkdtemp(resolve(tmpdir(), "sandcastle-runners-"));
  try {
    await mkdir(resolve(directory, "tests"));
    await mkdir(resolve(directory, ".sandcastle"));
    await writeFile(resolve(directory, "tests/real.py"), "print('ok')\n");
    await writeFile(resolve(directory, "tests/not-a-runner.c"), "int main(void) { return 0; }\n");
    await writeFile(resolve(directory, ".sandcastle/checks.sh"), "python3 -B tests/real.py\n");
    const inventory = await contractExecutionContext(directory);
    assert.deepEqual(inventory.focusedRunnerPaths, ["tests/real.py"]);
    assert.deepEqual(inventory.focusedCommands, ["python3 -B tests/real.py"]);
    assert.deepEqual(inventory.controllerChecks.at(-1), { id: controllerReviewCheck.id, kind: "review", evidence: "Controller requires each mapped criterion to pass both review and fresh final audit at the exact contract digest and HEAD." });
    assert.doesNotThrow(() => validateContractRunnerInventory(contract("python3 -B tests/real.py"), inventory.focusedRunnerPaths));
    assert.throws(() => validateContractRunnerInventory(contract("python3 -B tests/missing.py"), inventory.focusedRunnerPaths), /does not exist/);
    assert.throws(() => validateContractRunnerInventory(contract("python3 -B tests/not-a-runner.c"), inventory.focusedRunnerPaths), /does not exist/);
    assert.equal(focusedRunnerPath("node tests/example.mjs mode=one"), "tests/example.mjs");
  } finally { await rm(directory, { recursive: true, force: true }); }
});

test("verifier execution context expands sv-core to the production login evidence", async () => {
  const root = resolve(import.meta.dirname, "..");
  const inventory = await contractExecutionContext(root, root);
  const core = inventory.controllerChecks.find((check) => check.id === "sv-core");
  assert.ok(core.expandedRunnerPaths.includes("tests/sv_login_checks.py"));
  assert.ok(core.expandedRunnerPaths.includes("tests/sv_login_live_checks.py"));
  assert.ok(inventory.focusedRunnerPaths.includes("tests/sv_login_checks.py"));
});

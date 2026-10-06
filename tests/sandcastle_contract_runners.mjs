import assert from "node:assert/strict";
import test from "node:test";
import { baselineChecks } from "../.sandcastle/acceptance.mjs";
import { focusedRunnerPath, missingContractRunners, validateContractRunners } from "../.sandcastle/contract-runners.mjs";

const contract = (command) => ({ version: 1, completionScope: "implementation", criteria: [], checks: [
  ...structuredClone(baselineChecks),
  { id: "account", kind: "command", command, description: "Account production checks" },
] });

test("runner preflight catches the exact invented account script", async () => {
  const missing = contract("python3 -B tests/test_sv_account_create.py");
  await assert.rejects(
    validateContractRunners(missing, "/repo", async () => { const error = new Error("missing"); error.code = "ENOENT"; throw error; }),
    /focused runner tests\/test_sv_account_create\.py for check account does not exist/,
  );
});

test("runner preflight validates every command in a compound focused check", async () => {
  const candidate = contract(["python3 -B tests/sv_account_create_checks.py", "python3 -B tests/sv_account_failure_checks.py"]);
  const visited = [];
  assert.equal(await validateContractRunners(candidate, "/repo", async (path) => { visited.push(path); }), candidate);
  assert.deepEqual(visited, ["/repo/tests/sv_account_create_checks.py", "/repo/tests/sv_account_failure_checks.py"]);
  assert.deepEqual(await missingContractRunners(candidate, "/repo", async (path) => {
    if (path.endsWith("failure_checks.py")) throw new Error("missing");
  }), [{ checkId: "account", command: "python3 -B tests/sv_account_failure_checks.py", runner: "tests/sv_account_failure_checks.py" }]);
  assert.equal(focusedRunnerPath("node tests/feature.mjs arg=value"), "tests/feature.mjs");
});

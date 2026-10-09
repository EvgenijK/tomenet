import assert from "node:assert/strict";
import test from "node:test";
import { dispatchVerified, resumeExecutionCheckpoint, closeExecution } from "../.sandcastle/dispatch.mjs";

test("real dispatch seam stops before quota and inference on preflight failure", async () => {
  const calls = [];
  const failure = Object.assign(new Error("Missing committed ticket"), { code: "TICKET_NOT_COMMITTED", deterministic: true });
  await assert.rejects(dispatchVerified({}, {
    preflight: async () => { calls.push("preflight"); throw failure; },
    record: async () => calls.push("record"), quota: async () => calls.push("quota"), invoke: async () => calls.push("inference"),
  }), error => error === failure);
  assert.deepEqual(calls, ["preflight"]);
});
test("dispatch records actual provenance before quota and invokes once", async () => {
  const calls = [];
  const manifest = { digest: "verified" };
  const result = await dispatchVerified({ expectedBranch: "worker" }, {
    preflight: async execution => { assert.equal(execution.expectedBranch, "worker"); calls.push("preflight"); return manifest; },
    record: async value => { assert.equal(value, manifest); calls.push("record"); },
    quota: async () => calls.push("quota"), invoke: async value => { assert.equal(value, manifest); calls.push("inference"); return 42; },
  });
  assert.equal(result, 42); assert.deepEqual(calls, ["preflight", "record", "quota", "inference"]);
});
test("failed quota preserves provenance and prevents inference", async () => {
  const calls = [];
  await assert.rejects(dispatchVerified({}, {
    preflight: async () => ({}), record: async () => calls.push("record"),
    quota: async () => { calls.push("quota"); throw new Error("Task quota guard"); }, invoke: async () => calls.push("inference"),
  }), /quota guard/);
  assert.deepEqual(calls, ["record", "quota"]);
});
test("preflight resume only retries a saved execution fault without changing grants or history", () => {
  const state = { phase: "awaiting", resumePhase: "parallel-work", pauseReason: "Execution preflight [CLI_TOO_OLD]: update required",
    currentFailure: { code: "startup", diagnosticCode: "CLI_TOO_OLD", fatalPreflight: true },
    budget: { consumedPercent: 12, limitPercent: 25, tokens: 99 }, recoveryAttempts: { startup: 3 }, completedTickets: ["one.md"] };
  const before = structuredClone(state);
  resumeExecutionCheckpoint(state);
  assert.equal(state.phase, "parallel-work");
  for (const field of ["budget", "recoveryAttempts", "completedTickets", "currentFailure"]) assert.deepEqual(state[field], before[field]);
  for (const change of [s => { s.pauseReason = "Tests failed"; delete s.currentFailure; }, s => { s.phase = "test-gate"; }, s => { s.budget.consumedPercent = 25; }]) {
    const invalid = structuredClone(before); change(invalid); const snapshot = structuredClone(invalid);
    assert.throws(() => resumeExecutionCheckpoint(invalid)); assert.deepEqual(invalid, snapshot);
  }
});
test("preflight teardown closes runtime without committing rejected branch or dirty inputs", async () => {
  const calls = [];
  await closeExecution({ close: async () => calls.push("close") }, {
    failure: Object.assign(new Error("Execution preflight: wrong branch"), { code: "BRANCH_MISMATCH", deterministic: true }),
    preserve: async () => calls.push("commit"),
  });
  assert.deepEqual(calls, ["close"]);
  await closeExecution({ close: async () => calls.push("close") }, { failure: new Error("SV test gate failed"), preserve: async () => calls.push("commit") });
  assert.deepEqual(calls, ["close", "commit", "close"]);
});

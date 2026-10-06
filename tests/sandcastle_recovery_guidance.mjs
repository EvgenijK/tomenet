import assert from "node:assert/strict";
import test from "node:test";
import { beginRecovery, previewRecovery } from "../.sandcastle/recovery.mjs";
import { guidanceFor, guidanceForAgent } from "../.sandcastle/recovery-guidance.mjs";

const decision = (action, summary = "Correct the failed checkpoint") => ({ action, summary, ticketPaths: [], dependency: "" });
const base = (phase) => ({ id: "test", phase, completedTickets: [], reviewRound: 0, buildAttempts: 0, testAttempts: 0,
  budget: { consumedPercent: 0, limitPercent: 25 }, contractPending: { step: "split" } });

test("contract recovery is scoped to the failed substage and group, not independent verification", () => {
  const split = base("contract");
  beginRecovery(split, "Invalid split");
  const splitNext = previewRecovery(split, decision("retry", "Cover every source section")).next;
  assert.match(guidanceFor(splitNext, { phase: "contract", substage: "split" }), /Cover every source section/);
  assert.equal(guidanceFor(splitNext, { phase: "contract", substage: "assemble" }), "");
  assert.equal(guidanceFor(splitNext, { phase: "contract", substage: "verify" }), "");

  const reconciliation = base("contract"); reconciliation.contractPending.step = "assemble";
  beginRecovery(reconciliation, "Malformed reconciliation patch");
  const assembledNext = previewRecovery(reconciliation, decision("retry", "Return the focused patch only")).next;
  assert.match(guidanceFor(assembledNext, { phase: "contract", substage: "assemble" }), /Return the focused patch only/);
  assert.equal(guidanceFor(assembledNext, { phase: "contract", substage: "verify" }), "");

  const group = base("contract"); group.contractPending = { step: "parallel", failedGroupIndex: 1 };
  beginRecovery(group, "Invalid group 2 source quote");
  const groupNext = previewRecovery(group, decision("retry", "Cite only assigned text")).next;
  assert.equal(guidanceFor(groupNext, { phase: "contract", substage: "parallel", groupIndex: 0 }), "");
  assert.match(guidanceFor(groupNext, { phase: "contract", substage: "parallel", groupIndex: 1 }), /Cite only assigned text/);
  assert.equal(guidanceFor(groupNext, { phase: "contract", substage: "verify" }), "");
});

test("invalid manifest routes validation details to planner; unchanged retry is rejected", () => {
  const state = base("plan-validate");
  beginRecovery(state, "Ticket batch has unresolved dependency");
  assert.throws(() => previewRecovery(state, decision("retry")), /requires repair by the planner/);
  const next = previewRecovery(state, decision("repair", "Regenerate manifest dependencies")).next;
  assert.equal(next.phase, "plan");
  assert.match(guidanceForAgent(next, "ticket planning"), /Ticket batch has unresolved dependency/);
  assert.equal(guidanceForAgent(next, "implementation .scratch/issues/1.md"), "");
  const transient = base("plan-validate");
  const error = Object.assign(new Error("busy read"), { code: "EBUSY" });
  beginRecovery(transient, error);
  assert.equal(previewRecovery(transient, decision("retry")).next.phase, "plan-validate");
});

test("build and test repair guidance reaches the planner instead of the failed gate", () => {
  for (const phase of ["build", "build-repair", "test-gate", "test-repair"]) {
    const state = base(phase);
    beginRecovery(state, `${phase} output failed`);
    const next = previewRecovery(state, decision("repair", `Repair ${phase} failure`)).next;
    assert.equal(next.recoveryAdviceTarget.phase, "plan");
    assert.match(guidanceForAgent(next, "ticket planning"), new RegExp(`Repair ${phase} failure`));
    assert.equal(guidanceForAgent(next, "implementation issues/1.md"), "");
  }
});

test("read-only review retries receive only technical instructions, not prior conclusions", () => {
  for (const phase of ["review", "final-audit"]) {
    const state = base(phase);
    beginRecovery(state, "Prior verdict: accept high-severity flaw");
    const next = previewRecovery(state, decision("retry", "Prior reviewer says ready")).next;
    const hint = guidanceFor(next, { phase });
    assert.match(hint, /Technical retry instruction/);
    assert.doesNotMatch(hint, /Prior verdict|Prior reviewer|ready|accept high-severity/);
  }
});

test("parallel recovery guidance is delivered only to failed or blocked members", () => {
  const state = base("parallel-work");
  state.wave = { id: 1, members: [
    { ticket: { path: "issues/1.md" }, status: "completed" },
    { ticket: { path: "issues/2.md" }, status: "failed" },
    { ticket: { path: "issues/3.md" }, status: "blocked" },
  ] };
  beginRecovery(state, "Two workers failed");
  const next = previewRecovery(state, decision("retry", "Retry failed worker")).next;
  assert.equal(guidanceForAgent(next, "implementation issues/1.md"), "");
  assert.match(guidanceForAgent(next, "implementation issues/2.md"), /Retry failed worker/);
  assert.match(guidanceForAgent(next, "implementation issues/3.md"), /Retry failed worker/);
  assert.equal(guidanceForAgent(next, "assembly wave 1"), "");
});

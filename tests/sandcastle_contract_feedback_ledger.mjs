import assert from "node:assert/strict";
import test from "node:test";

import { baselineChecks, contractStage } from "../.sandcastle/acceptance.mjs";

const head = "a".repeat(40);
const sources = { "spec.md": "Preserve cancellation. Keep protocol semantics." };
const proposal = () => ({
  version: 1,
  completionScope: "implementation",
  checks: structuredClone(baselineChecks),
  criteria: [{
    id: "AC-1",
    requirement: "Preserve cancellation",
    source: { path: "spec.md", quote: "Preserve cancellation." },
    mandatory: true,
    applicability: "current",
    checkIds: ["sv-core"],
    owner: "",
    deferralReason: "",
  }],
});
const noOp = async () => {};
const reject = (...messages) => ({
  approved: false,
  summary: messages.join("; "),
  missingRequirements: messages,
  findings: messages.map((message) => ({ message, sourceSpanIds: [] })),
});

test("contract feedback keeps stable cumulative history until independent approval", async () => {
  const first = "Cover cancellation behavior";
  const second = "Cover protocol semantics";
  const reviews = [reject(first), reject(second), reject(first, second)];
  const received = [];
  const state = { id: "ledger", phase: "contract" };
  const ops = {
    sources: async () => sources,
    head: async () => head,
    save: noOp,
    publish: noOp,
    propose: async (_sources, feedback) => {
      received.push([...feedback]);
      return proposal();
    },
    verify: async () => reviews.shift(),
  };

  await contractStage(state, ops);
  const firstId = state.contractPending.feedbackLedger[0].id;
  assert.deepEqual(received[0], []);
  assert.equal(state.contractPending.feedbackLedger[0].status, "open");

  await contractStage(state, ops);
  assert.deepEqual(received[1], [first]);
  assert.equal(state.contractPending.feedbackLedger.find((item) => item.id === firstId).status, "open");
  assert.equal(state.contractPending.feedbackLedger.length, 2);

  await contractStage(state, ops);
  assert.deepEqual(received[2], [first, second]);
  const repeated = state.contractPending.feedbackLedger.find((item) => item.message === first);
  assert.equal(repeated.id, firstId);
  assert.equal(repeated.status, "open");
  assert.equal(repeated.occurrences, 2);
  assert.deepEqual(state.contractPending.feedback, [first, second]);

  await contractStage(state, { ...ops, verify: async () => ({
    approved: true,
    summary: "All source obligations are represented",
    missingRequirements: [],
    findings: [],
  }) });
  assert.deepEqual(received[3], [first, second]);
  assert.equal(state.phase, "plan");
  assert.ok(state.acceptance.contractFeedbackLedger.every((item) => item.status === "addressed"));
});

test("three identical rejected finding sets stop early and survive checkpoint resume", async () => {
  const message = "Cover cancellation behavior";
  let state = { id: "stalled", phase: "contract" };
  const ops = {
    sources: async () => sources,
    head: async () => head,
    save: noOp,
    publish: noOp,
    propose: async () => proposal(),
    verify: async () => reject(message),
  };

  await contractStage(state, ops);
  state = structuredClone(state);
  await contractStage(state, ops);
  state = structuredClone(state);
  await assert.rejects(contractStage(state, ops), /contract feedback made no progress.*human decision required/i);
  assert.equal(state.contractPending.round, 3);
  assert.equal(state.contractPending.feedbackLedger.length, 1);
  assert.equal(state.contractPending.feedbackLedger[0].occurrences, 3);
  assert.equal(state.contractPending.convergence.identicalOpenRounds, 3);

  await assert.rejects(contractStage(state, ops), /contract feedback made no progress.*human decision required/i);
  assert.equal(state.contractPending.round, 3);
});

import assert from "node:assert/strict";
import test from "node:test";
import { validateTicketBatch, recordTicketResult, validateTicketCompletion } from "../.sandcastle/tickets.mjs";

const path = (id) => `.scratch/sandcastle-test/issues/${id}.md`;
const state = () => ({ id: "test", completedTickets: [path("11")], ticketIndex: 0 });

test("review batch may depend on completed tickets in earlier batches", () => {
  const tickets = [{ path: path("22"), blockedBy: [path("11")] }, { path: path("23"), blockedBy: [path("22")] }];
  assert.deepEqual(validateTicketBatch({ tickets }, state()), tickets);
});

test("unknown and forward dependencies remain invalid", () => {
  for (const dependency of [path("missing"), path("23")]) {
    assert.throws(() => validateTicketBatch({ tickets: [{ path: path("22"), blockedBy: [dependency] }, { path: path("23"), blockedBy: [] }] }, state()), /Invalid blockers/);
  }
});

test("blocked agent result preserves the frontier and completed set", () => {
  const s = state();
  const result = { status: "blocked", summary: "Production birth caller does not exist yet" };
  recordTicketResult(s, { path: path("17") }, result);
  assert.deepEqual(s.completedTickets, [path("11")]);
  assert.equal(s.ticketIndex, 0);
  assert.deepEqual(s.blockedTicket, { path: path("17"), ...result });
});

test("completed agent result advances exactly once", () => {
  const s = state();
  recordTicketResult(s, { path: path("22") }, { status: "completed", summary: "Implemented and focused checks passed" });
  assert.deepEqual(s.completedTickets, [path("11"), path("22")]);
  assert.equal(s.ticketIndex, 1);
});

test("missing or malformed outcome cannot silently complete a ticket", () => {
  for (const result of [undefined, {}, { status: "ready-for-agent", summary: "No changes" }, { status: "completed", summary: "" }]) {
    const s = state();
    assert.throws(() => recordTicketResult(s, { path: path("22") }, result), /Invalid implementation result/);
    assert.deepEqual(s.completedTickets, [path("11")]);
    assert.equal(s.ticketIndex, 0);
  }
});

test("a blocked previous ticket cannot satisfy a new batch dependency", () => {
  const s = state();
  recordTicketResult(s, { path: path("17") }, { status: "blocked", summary: "Missing caller" });
  assert.throws(() => validateTicketBatch({ tickets: [{ path: path("29"), blockedBy: [path("17")] }] }, s), /Invalid blockers/);
});

test("duplicate and self-dependent tickets are rejected", () => {
  const ticket = { path: path("22"), blockedBy: [] };
  assert.throws(() => validateTicketBatch({ tickets: [ticket, ticket] }, state()), /Duplicate/);
  assert.throws(() => validateTicketBatch({ tickets: [{ ...ticket, blockedBy: [ticket.path] }] }, state()), /Invalid blockers/);
});

test("completion also requires resolved Markdown and a persisted Answer", () => {
  for (const content of ["Status: open\n## Answer\nBlocked", "Status: resolved\nNo answer", "**Status:** ready-for-agent\n## Answer\nMissing caller"]) {
    assert.throws(() => validateTicketCompletion(content, path("17")), /requires resolved/);
  }
  validateTicketCompletion("Status: resolved\n\n## Answer\nImplemented and verified", path("22"));
  validateTicketCompletion("**Status:** resolved\n\n## Answer\nImplemented and verified", path("22"));
});

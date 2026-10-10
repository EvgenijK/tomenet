import assert from "node:assert/strict";
import test from "node:test";
import { focusedTestTicketContents, validateTicketBatch, recordTicketResult, validateTicketCompletion } from "../.sandcastle/tickets.mjs";

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

const canonicalImplementation = [
  "Type: implementation",
  "Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.",
  "Status: resolved",
  "Labels: enhancement, ready-for-agent",
  "",
  "## Answer",
  "Implemented and verified",
].join("\n");

test("completed implementation requires a canonical non-owning Tracker role", () => {
  const withoutRole = canonicalImplementation.replace(/^Tracker role:.*\n/m, "");
  assert.throws(() => validateTicketCompletion(withoutRole, path("22")), /canonical Tracker role/);

  const mismatchedRole = canonicalImplementation.replace("[SV-B-023]", "[SV-B-099]");
  assert.throws(() => validateTicketCompletion(mismatchedRole, path("22")), /canonical Tracker role/);
});

test("focused coverage completion cannot bypass metadata checks by losing Type", () => {
  const focusedWithoutTypeOrRole = canonicalImplementation
    .replace(/^Type:.*\n/m, "")
    .replace(/^Tracker role:.*\n/m, "");
  assert.throws(() => validateTicketCompletion(focusedWithoutTypeOrRole, path("22-focused-test-coverage")), /canonical Tracker role/);
});

test("completed implementation requires exactly one standard triage-state label", () => {
  const withoutTriageState = canonicalImplementation.replace("Labels: enhancement, ready-for-agent", "Labels: enhancement");
  assert.throws(() => validateTicketCompletion(withoutTriageState, path("22")), /exactly one standard triage-state label/);

  const conflictingStates = canonicalImplementation.replace("Labels: enhancement, ready-for-agent", "Labels: enhancement, needs-triage, ready-for-agent");
  assert.throws(() => validateTicketCompletion(conflictingStates, path("22")), /exactly one standard triage-state label/);
});

test("completed implementation accepts canonical tracker and triage metadata", () => {
  validateTicketCompletion(canonicalImplementation, path("22"));
});

test("focused coverage ticket is non-owning canonical history with a triage state", () => {
  const content = focusedTestTicketContents({ specPath: "docs/tasks/stage-b/SV-B-023-birth-choices.md" }, 64);
  assert.match(content, /^# 64: Verify focused production coverage$/m);
  assert.match(content, /^Tracker role: Wayfinder planning\/execution history for canonical \[SV-B-023\]\(\.\.\/\.\.\/\.\.\/docs\/tasks\/stage-b\/SV-B-023-birth-choices\.md\); this child does not own Stage B capability IDs\.$/m);
  assert.match(content, /^Status: open$/m);
  assert.match(content, /^Labels: enhancement, ready-for-agent$/m);
});

test("worker delivery cannot wait for post-assembly controller acceptance", () => {
  const manifest = { tickets: [{ path: path("09-final-head-runtime-evidence"), blockedBy: [path("11")],
    delivery: { owner: "worker", phase: "parallel-work", prerequisites: [{ owner: "controller", phase: "final-audit" }] } }] };
  assert.throws(() => validateTicketBatch(manifest, state()), /PLAN_STAGE_CYCLE/);
});

test("typed worker completion requires completed local delivery predicates but permits pending controller obligations", () => {
  const path = '.scratch/sandcastle-test/issues/09-runtime-delivery.md';
  const delivery = {owner:'worker',phase:'parallel-work',prerequisites:[]};
  const pending = 'Type: verification\nStatus: resolved\n## Worker delivery criteria\n- [ ] Collector committed and focused checks passed.\n## Controller acceptance obligations\n- [ ] Independent final-HEAD runtime audit.\n## Answer\nDelivery record.';
  assert.throws(()=>validateTicketCompletion(pending,path,delivery),/DELIVERY_PREDICATES/);
  assert.doesNotThrow(()=>validateTicketCompletion(pending.replace('[ ] Collector','[x] Collector'),path,delivery));
});

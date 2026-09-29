import assert from "node:assert/strict";
import test from "node:test";
import { mkdtemp, mkdir, readFile, writeFile, rm } from "node:fs/promises";
import { tmpdir } from "node:os";
import { resolve } from "node:path";
import { execFileSync } from "node:child_process";
import { beginRecovery, previewRecovery, recoveryRestriction, runRecovery, documentRecovery } from "../.sandcastle/recovery.mjs";
import { selectReadyTickets, runWave } from "../.sandcastle/parallel.mjs";

const path = (number) => `.scratch/sandcastle-test/issues/${number}-ticket.md`;
function state() {
  return { id: "test", branch: "codex/run", specPath: "docs/tasks/owner.md", phase: "parallel-work", completedTickets: [path(1)],
    tickets: [{ path: path(1), blockedBy: [] }, { path: path(2), blockedBy: [] }, { path: path(3), blockedBy: [path(2)] }, { path: path(4), blockedBy: [] }],
    ticketIndex: 1, reviewRound: 0, reviewLimit: 5, buildAttempts: 2, buildLimit: 10, testAttempts: 1, testLimit: 10,
    budget: { consumedPercent: 7, tokens: 123 },
    wave: { id: 1, after: "tickets", baseCommit: "a".repeat(40), members: [
      { ticket: { path: path(1), blockedBy: [] }, status: "completed", integrated: true, head: "b".repeat(40) },
      { ticket: { path: path(2), blockedBy: [] }, status: "blocked", branch: "codex/worker", head: "c".repeat(40), result: { status: "blocked", summary: "Absent real entry handoff" } },
    ] },
  };
}
const decision = (action = "defer") => ({ action, summary: "Real game entry is not implemented yet", ticketPaths: action === "defer" ? [path(2)] : [], dependency: action === "defer" ? "docs/tasks/dependency.md" : "" });
const noOp = async () => {};

test("quota mentioned in restoration guidance is not an exhausted usage guard", () => {
  const s = state(); s.phase = "contract";
  assert.equal(recoveryRestriction(s, "Controller mount is fixed; preserve quota and cycle limits during recovery"), undefined);
  assert.doesNotThrow(() => beginRecovery(s, "Recovery agent requested a stop: move schema paths; preserve quota and cycle limits"));
  for (const error of ["Task reached 25% of the weekly Codex quota", "Weekly Codex quota is unavailable; cannot enforce the budget", "Codex account usage limit reached", "Task reached its token limit (123)", "Build failed 10 times; continuation requires human approval."]) assert.ok(recoveryRestriction(state(), error));
});

test("deferral retains successful siblings, pending acceptance and dependent closure while independent work resumes", () => {
  const s = state(); beginRecovery(s, "Worker blocked");
  const before = structuredClone(s);
  const { next, affected } = previewRecovery(s, decision());
  assert.deepEqual(s, before);
  assert.deepEqual(affected.map((ticket) => ticket.path), [path(2), path(3)]);
  assert.deepEqual(next.completedTickets, [path(1)]);
  assert.deepEqual(selectReadyTickets(next, 3).map((ticket) => ticket.path), [path(4)]);
  assert.equal(next.phase, "tickets"); assert.equal(next.wave, undefined);
  assert.equal(next.waveHistory[0].members[1].head, "c".repeat(40));
  assert.deepEqual(next.budget, before.budget);
  assert.equal(next.buildLimit, before.buildLimit);
  assert.match(next.scopeNotes, /acceptance/);
});

test("deferral of the last blocked implementation ticket reaches build without claiming its completion", () => {
  const s = state(); s.tickets = s.tickets.slice(0, 2); beginRecovery(s, "Worker blocked");
  const { next } = previewRecovery(s, decision());
  assert.equal(next.tickets.every((ticket) => next.completedTickets.includes(ticket.path)), true);
  assert.equal(next.deferredTickets[0].path, path(2));
  assert.equal(next.completedTickets.includes(path(2)), false);
});

test("deferring one blocker preserves other failed members and retries only unfinished work", async () => {
  const s = state(); s.wave.members.push({ ticket: { path: path(4), blockedBy: [] }, status: "failed", branch: "codex/other", error: "Transient execution failure" });
  beginRecovery(s, "Wave incomplete"); const { next } = previewRecovery(s, decision());
  assert.equal(next.phase, "parallel-work");
  const seen = [];
  await runWave(next, { save: noOp, run: async (member) => { seen.push(member.ticket.path); return { result: { status: "completed", summary: "Verified" }, head: "d".repeat(40) }; } });
  assert.deepEqual(seen, [path(4)]);
  assert.equal(next.wave.members[0].integrated, true);
});

test("deferral cannot lose unmerged success, complete a blocker, or skip required test coverage", () => {
  for (const mutate of [s => { s.wave.members[0].integrated = false; }, s => { s.wave.after = "test-gate"; }, s => { s.wave.members[1].status = "running"; }, s => { s.completedTickets.push(path(2)); }]) {
    const s = state(); mutate(s); beginRecovery(s, "Incident");
    assert.throws(() => previewRecovery(s, decision()));
  }
});

test("malformed actions and external dependency paths cannot change scheduler state", () => {
  const s = state(); beginRecovery(s, "Incident"); const before = structuredClone(s);
  for (const d of [{ ...decision(), action: "done" }, { ...decision(), dependency: "docs/tasks/../../auth.md" }, { ...decision(), ticketPaths: [path(99)] }, { ...decision(), ticketPaths: [path(2), path(2)] }, { ...decision("retry"), ticketPaths: [path(2)] }]) assert.throws(() => previewRecovery(s, d));
  assert.deepEqual(s, before);
});

test("quota and cycle guard stops never ask a recovery agent or extend budgets", () => {
  for (const mutate of [s => { s.quotaError = "limit"; }, s => { s.budget.consumedPercent = 25; }, s => { s.phase = "build-repair"; s.buildAttempts = 10; }, s => { s.phase = "test-repair"; s.testAttempts = 10; }, s => { s.reviewRound = 5; s.reviewFindings = [{}]; }]) {
    const s = state(); mutate(s); const before = structuredClone(s);
    assert.ok(recoveryRestriction(s)); assert.throws(() => beginRecovery(s, "Failure")); assert.deepEqual(s, before);
  }
});

test("usage guard reached by the decision agent prevents applying its proposed deferral", async () => {
  const s = state(); beginRecovery(s, "Worker blocked"); const completed = [...s.completedTickets]; let published = false;
  await assert.rejects(runRecovery(s, { decide: async () => { s.quotaError = "Account allowance exhausted"; return decision(); }, save: noOp, document: async () => { published = true; }, event: noOp }), /Quota guard/);
  assert.equal(published, false); assert.deepEqual(s.completedTickets, completed); assert.equal(s.deferredTickets, undefined);
});

test("three automatic retries without progress stop; verified progress permits a fresh checkpoint", () => {
  let s = state();
  for (let i = 0; i < 3; i++) { beginRecovery(s, "Repeated failure"); s = previewRecovery(s, decision("retry")).next; }
  assert.throws(() => beginRecovery(s, "Different error text at same checkpoint"), /no progress/);
  s.completedTickets.push(path(4)); assert.doesNotThrow(() => beginRecovery(s, "Failure after progress"));
});

test("code repair goes through existing build/test or planning paths and preserves counters", () => {
  for (const [phase, expected] of [["build-repair", "build-repair"], ["test-repair", "test-repair"], ["plan-validate", "plan"], ["parallel-work", "parallel-work"], ["assemble", "assemble"]]) {
    const s = state(); s.phase = phase; beginRecovery(s, "Concrete failure");
    const { next } = previewRecovery(s, decision("repair"));
    assert.equal(next.phase, expected); assert.equal(next.buildAttempts, 2); assert.equal(next.testAttempts, 1); assert.equal(next.pendingFailure, "Concrete failure");
  }
  const s = state(); s.phase = "integrate"; beginRecovery(s, "Integration failure"); assert.throws(() => previewRecovery(s, decision("repair")), /require/);
});

test("saved decision survives interrupted publication and is replayed without another AI call", async () => {
  const s = state(); beginRecovery(s, "Worker blocked"); let calls = 0; let saved;
  await assert.rejects(runRecovery(s, { decide: async () => { calls++; return decision(); }, save: async () => { saved = structuredClone(s); }, document: async () => { throw new Error("Interrupted"); }, event: noOp }), /Interrupted/);
  assert.equal(saved.phase, "recovery"); assert.equal(saved.recovery.decision.action, "defer");
  await runRecovery(saved, { decide: async () => { calls++; throw new Error("Do not rerun AI"); }, save: noOp, document: noOp, event: noOp });
  assert.equal(calls, 1); assert.equal(saved.recoveries.length, 1); assert.equal(saved.phase, "tickets");
});

test("stop keeps the original checkpoint and every successful revision", async () => {
  const s = state(); beginRecovery(s, "Ambiguous failure");
  assert.equal(await runRecovery(s, { decide: async () => decision("stop"), save: noOp, document: noOp, event: noOp }), false);
  assert.equal(s.phase, "parallel-work"); assert.equal(s.wave.members[0].head, "b".repeat(40)); assert.equal(s.recoveries[0].action, "stop");
});

test("actual recovery publisher commits pending metadata once and preserves worker Git ancestry", async () => {
  const dir = await mkdtemp(resolve(tmpdir(), "sandcastle-recovery-"));
  const tree = resolve(dir, "repo"); const runs = resolve(dir, "runs");
  await mkdir(tree); await mkdir(resolve(runs, "test"), { recursive: true });
  const git = async (cwd, ...args) => execFileSync("git", args, { cwd, encoding: "utf8", stdio: ["ignore", "pipe", "pipe"] }).trim();
  try {
    await git(tree, "init", "-b", "main"); await git(tree, "config", "user.name", "Recovery test"); await git(tree, "config", "user.email", "recovery@example.invalid");
    await mkdir(resolve(tree, ".scratch/sandcastle-test/issues"), { recursive: true }); await mkdir(resolve(tree, "docs/tasks"), { recursive: true });
    for (const n of [1, 2, 3, 4]) await writeFile(resolve(tree, path(n)), `# Ticket ${n}\nStatus: open\nAssignee: unassigned\nLabels: enhancement, ready-for-agent\n\n- [ ] Actual behavior\n`);
    for (const [name, text] of [["docs/tasks/owner.md", "Original owner: acceptance pending\n"], ["docs/tasks/dependency.md", "Status: specified\n"], [".scratch/sandcastle-test/map.md", "Original decisions\n"]]) await writeFile(resolve(tree, name), text);
    const manifest = ".scratch/sandcastle-test/batch-0.json"; const s = state(); const originalManifest = JSON.stringify({ tickets: s.tickets });
    await writeFile(resolve(tree, manifest), originalManifest);
    await git(tree, "add", "."); await git(tree, "commit", "-m", "Preserved successful worker");
    const head = await git(tree, "rev-parse", "HEAD"); beginRecovery(s, "Absent dependency"); const d = decision(); const preview = previewRecovery(s, d);
    const ensureCommit = async (cwd, message) => { if (await git(cwd, "status", "--porcelain")) { await git(cwd, "add", "."); await git(cwd, "commit", "-m", message); } };
    const ops = { worktree: tree, runsDir: runs, git, ensureCommit, manifest };
    await documentRecovery(s, s.recovery, d, preview, ops);
    const after = await git(tree, "rev-parse", "HEAD"); assert.notEqual(after, head);
    await git(tree, "merge-base", "--is-ancestor", head, after);
    const ticket = await readFile(resolve(tree, path(2)), "utf8"); assert.match(ticket, /Status: open/); assert.match(ticket, /needs-info/); assert.match(ticket, /acceptance are not completed/); assert.match(ticket, /- \[ \]/);
    assert.match(ticket, /Labels: enhancement, needs-info/);
    assert.deepEqual(JSON.parse(await readFile(resolve(tree, manifest))).tickets.map((t) => t.path), [path(1), path(4)]);
    assert.equal(await readFile(resolve(tree, manifest.replace(".json", "-before-recovery-1.json")), "utf8"), originalManifest);
    await documentRecovery(s, s.recovery, d, preview, ops);
    assert.equal(await git(tree, "rev-parse", "HEAD"), after); assert.equal(await git(tree, "status", "--porcelain"), "");
    assert.equal((await readFile(resolve(tree, path(2)), "utf8")).match(/Sandcastle recovery 1/g).length, 1);
    const missing = { ...d, dependency: "docs/tasks/absent.md" };
    await assert.rejects(documentRecovery(s, s.recovery, missing, previewRecovery(s, missing), ops), /ENOENT/);
    assert.equal(await git(tree, "status", "--porcelain"), "");
  } finally { await rm(dir, { recursive: true, force: true }); }
});

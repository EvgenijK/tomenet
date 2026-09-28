import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { mkdtempSync, writeFileSync, readFileSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";
import test from "node:test";
import { selectReadyTickets, createWave, runWave, collectWave } from "../.sandcastle/parallel.mjs";

const ticket = (id, blockedBy = []) => ({ path: `.scratch/sandcastle-test/issues/${id}.md`, blockedBy });
const revision = "a".repeat(40);
const completedResult = { status: "completed", summary: "Implemented and verified" };
const blockedResult = { status: "blocked", summary: "Required caller is unavailable" };
const state = (tickets) => ({ id: "test", branch: "codex/sandcastle-test", tickets, completedTickets: [], ticketIndex: 0, phase: "tickets" });
const deferred = () => {
  let resolve;
  const promise = new Promise((done) => { resolve = done; });
  return { promise, resolve };
};
const workerSuccess = async () => ({ result: completedResult, head: revision });
const callbacks = (overrides = {}) => ({ save: async () => {}, run: workerSuccess, assemble: async () => completedResult, verify: async () => {}, ...overrides });

test("frontier is derived from dependencies and completed set, independent of ticketIndex", () => {
  const a = ticket("a"), b = ticket("b", [a.path]), c = ticket("c"), d = ticket("d", [b.path]);
  const s = state([a, b, c, d]);
  s.ticketIndex = 99;
  assert.deepEqual(selectReadyTickets(s, 2), [a, c]);
  assert.deepEqual(selectReadyTickets(s, 1), [a]);
  s.completedTickets.push(a.path);
  assert.deepEqual(selectReadyTickets(s, 2), [b, c]);
  s.completedTickets.push(b.path, c.path, d.path);
  assert.deepEqual(selectReadyTickets(s, 2), []);
});

test("wave captures isolated branches at one base and refuses an unresolved dead end", () => {
  const s = state([ticket("a"), ticket("b")]);
  const wave = createWave(s, { baseCommit: "base-sha", limit: 2, after: "review-fix" });
  assert.equal(s.wave, wave);
  assert.equal(wave.baseCommit, "base-sha");
  assert.equal(wave.after, "review-fix");
  assert.deepEqual(wave.members.map((m) => m.ticket), s.tickets);
  assert.deepEqual(wave.members.map((m) => m.status), ["pending", "pending"]);
  assert.equal(new Set(wave.members.map((m) => m.branch)).size, 2);
  assert.ok(wave.members.every((m) => typeof m.branch === "string" && m.branch));
  const dead = state([ticket("blocked", [ticket("missing").path])]);
  assert.throws(() => createWave(dead, { baseCommit: "base-sha", limit: 2 }));
});

test("workers start concurrently and only successful assembly plus verification completes tickets", async () => {
  const s = state([ticket("a"), ticket("b")]);
  createWave(s, { baseCommit: "base", limit: 2 });
  const entered = deferred(), release = deferred(), started = [], snapshots = [];
  const ops = callbacks({
    save: async (current) => { snapshots.push(structuredClone(current)); },
    run: async (member) => {
      started.push(member.ticket.path);
      if (started.length === 2) entered.resolve();
      await release.promise;
      return { result: completedResult, head: revision };
    },
  });
  const running = runWave(s, ops);
  await entered.promise;
  assert.equal(started.length, 2);
  assert.deepEqual(s.completedTickets, []);
  release.resolve();
  await running;
  assert.equal(s.phase, "assemble");
  assert.deepEqual(s.completedTickets, []);
  assert.ok(snapshots.length > 0);
  assert.ok(snapshots.every((snapshot) => snapshot.completedTickets.length === 0));
  const verified = [];
  await collectWave(s, callbacks({ verify: async (member) => { verified.push(member.ticket.path); } }));
  assert.deepEqual(verified.sort(), s.tickets.map((t) => t.path).sort());
  assert.deepEqual(s.completedTickets.sort(), s.tickets.map((t) => t.path).sort());
  assert.equal(s.ticketIndex, 2);
  assert.equal(s.phase, "tickets");
  assert.equal(s.wave, undefined);
  assert.equal(s.waveHistory.length, 1);
});

test("dependent tickets enter the next wave only after prerequisite is collected", async () => {
  const a = ticket("a"), b = ticket("b", [a.path]), c = ticket("c");
  const s = state([a, b, c]);
  createWave(s, { baseCommit: "base", limit: 2 });
  await runWave(s, callbacks());
  assert.deepEqual(selectReadyTickets(s, 2), [a, c]);
  await collectWave(s, callbacks());
  const second = createWave(s, { baseCommit: "merged", limit: 2 });
  assert.deepEqual(second.members.map((m) => m.ticket), [b]);
  assert.equal(second.baseCommit, "merged");
});

test("failed worker retains the successful sibling and retry runs only the failed member", async () => {
  const s = state([ticket("a"), ticket("b")]);
  createWave(s, { baseCommit: "base", limit: 2 });
  const calls = [];
  await runWave(s, callbacks({ run: async (member) => {
    calls.push(member.ticket.path);
    if (member.ticket.path === s.tickets[1].path) throw new Error("Worker process failed");
    return workerSuccess();
  } }));
  await assert.rejects(collectWave(s, callbacks()));
  assert.equal(s.phase, "parallel-work");
  assert.deepEqual(s.completedTickets, [s.tickets[0].path]);
  assert.ok(s.wave);
  await runWave(s, callbacks({ run: async (member) => { calls.push(member.ticket.path); return workerSuccess(); } }));
  assert.equal(calls.filter((path) => path === s.tickets[0].path).length, 1);
  assert.equal(calls.filter((path) => path === s.tickets[1].path).length, 2);
  await collectWave(s, callbacks());
  assert.equal(s.completedTickets.length, 2);
  assert.equal(new Set(s.completedTickets).size, 2);
});

test("blocked worker is retained with its reason while a merged sibling completes", async () => {
  const s = state([ticket("a"), ticket("b")]);
  createWave(s, { baseCommit: "base", limit: 2 });
  await runWave(s, callbacks({ run: async (member) => ({ result: member.ticket.path === s.tickets[1].path ? blockedResult : completedResult, head: revision }) }));
  await assert.rejects(collectWave(s, callbacks()));
  assert.equal(s.phase, "parallel-work");
  assert.deepEqual(s.completedTickets, [s.tickets[0].path]);
  const blocked = s.wave.members.find((member) => member.ticket.path === s.tickets[1].path);
  assert.equal(blocked.result.status, "blocked");
  assert.match(blocked.result.summary, /caller/);
});

test("blocked assembly cannot complete any worker", async () => {
  const s = state([ticket("a"), ticket("b")]);
  createWave(s, { baseCommit: "base", limit: 2 });
  await runWave(s, callbacks());
  let verified = 0;
  await assert.rejects(collectWave(s, callbacks({ assemble: async () => blockedResult, verify: async () => { verified++; } })));
  assert.deepEqual(s.completedTickets, []);
  assert.equal(verified, 0);
  assert.ok(s.wave);
});

test("verification failure leaves all completion records untouched and supports collection retry", async () => {
  const s = state([ticket("a"), ticket("b")]);
  createWave(s, { baseCommit: "base", limit: 2 });
  await runWave(s, callbacks());
  await assert.rejects(collectWave(s, callbacks({ verify: async (member) => {
    if (member.ticket.path === s.tickets[1].path) throw new Error("Commit is not an ancestor of assembled HEAD");
  } })), /ancestor/);
  assert.deepEqual(s.completedTickets, []);
  assert.equal(s.ticketIndex, 0);
  assert.ok(s.wave.members.every((member) => !member.integrated));
  await collectWave(s, callbacks());
  assert.equal(s.completedTickets.length, 2);
});

test("assembly requires a valid completed outcome", async () => {
  for (const outcome of [undefined, {}, { status: "completed", summary: "" }, { status: "ready", summary: "No assembly" }]) {
    const s = state([ticket("a")]);
    createWave(s, { baseCommit: "base", limit: 1 });
    await runWave(s, callbacks());
    await assert.rejects(collectWave(s, callbacks({ assemble: async () => outcome })));
    assert.deepEqual(s.completedTickets, []);
  }
});

test("malformed worker outcome or uncommitted revision cannot satisfy dependencies", async () => {
  for (const output of [
    { result: { status: "completed", summary: "" }, head: revision },
    { result: { status: "ready", summary: "No implementation" }, head: revision },
    { result: completedResult, head: "HEAD" },
  ]) {
    const s = state([ticket("a")]);
    createWave(s, { baseCommit: "base", limit: 1 });
    await runWave(s, callbacks({ run: async () => output }));
    assert.equal(s.wave.members[0].status, "failed");
    assert.ok(s.wave.members[0].error);
    await assert.rejects(collectWave(s, callbacks()));
    assert.deepEqual(s.completedTickets, []);
  }
});

test("worker persistence failure surfaces before moving to assembly", async () => {
  const s = state([ticket("a"), ticket("b")]);
  createWave(s, { baseCommit: "base", limit: 2 });
  let saves = 0;
  await assert.rejects(runWave(s, callbacks({ save: async () => {
    if (++saves === 1) throw new Error("State filesystem is unavailable");
  } })), /filesystem/);
  assert.equal(s.phase, "parallel-work");
  assert.deepEqual(s.completedTickets, []);
});

test("real conflicting Git branches are resolved by assembler and verified by ancestry", async () => {
  const dir = mkdtempSync(join(tmpdir(), "sandcastle-parallel-git-"));
  const git = (...args) => execFileSync("git", args, { cwd: dir, encoding: "utf8", stdio: ["ignore", "pipe", "pipe"] }).trim();
  try {
    git("init", "-b", "integration");
    git("config", "user.name", "Sandcastle test");
    git("config", "user.email", "sandcastle-test@example.invalid");
    writeFileSync(join(dir, "shared.txt"), "base\n");
    git("add", "shared.txt");
    git("commit", "-m", "Base");
    const s = state([ticket("a"), ticket("b")]);
    const wave = createWave(s, { baseCommit: git("rev-parse", "HEAD"), limit: 2 });
    // Git checkout is sequential in this fixture; independent worker execution is
    // exercised above. Here runWave receives actual isolated branch commits.
    const heads = new Map();
    for (const member of wave.members) {
      git("checkout", "-b", member.branch, wave.baseCommit);
      writeFileSync(join(dir, "shared.txt"), `${member.ticket.path}\n`);
      git("add", "shared.txt");
      git("commit", "-m", `Implement ${member.ticket.path}`);
      heads.set(member.branch, git("rev-parse", "HEAD"));
    }
    git("checkout", "integration");
    await runWave(s, callbacks({ run: async (member) => ({ result: completedResult, head: heads.get(member.branch) }) }));
    let conflicts = 0;
    await collectWave(s, callbacks({
      assemble: async (current) => {
        for (const member of current.members) {
          try { git("merge", "--no-ff", "--no-edit", member.branch); }
          catch (error) {
            assert.match(git("diff", "--name-only", "--diff-filter=U"), /shared\.txt/);
            conflicts++;
            writeFileSync(join(dir, "shared.txt"), `${s.tickets.map((t) => t.path).join("\n")}\n`);
            git("add", "shared.txt");
            git("commit", "-m", "Resolve parallel worker conflict preserving both changes");
          }
        }
        return { status: "completed", summary: "Merged workers and resolved shared file" };
      },
      verify: async (member) => {
        git("merge-base", "--is-ancestor", member.head, "HEAD");
        assert.ok(readFileSync(join(dir, "shared.txt"), "utf8").includes(member.ticket.path));
      },
    }));
    assert.equal(conflicts, 1);
    assert.equal(s.completedTickets.length, 2);
    assert.equal(s.wave, undefined);
  } finally { rmSync(dir, { recursive: true, force: true }); }
});

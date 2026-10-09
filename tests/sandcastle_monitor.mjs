import assert from "node:assert/strict";
import test from "node:test";
import { mkdtemp, mkdir, copyFile, open, writeFile, readFile, rm } from "node:fs/promises";
import { resolve } from "node:path";
import { spawn } from "node:child_process";
import { appendStageEvent } from "../.sandcastle/events.mjs";
import { readMonitor, acknowledgeMonitor, monitorMain } from "../.sandcastle/monitor.mjs";
import { classifyFailure } from "../.sandcastle/failure.mjs";

async function fixture(t) {
  const dir = await mkdtemp("/tmp/sandcastle-monitor-");
  t.after(() => rm(dir, { recursive: true, force: true }));
  const state = { id: "test-run", phase: "build" };
  const save = () => writeFile(resolve(dir, state.id, "state.json"), JSON.stringify(state));
  await appendStageEvent(dir, state, { stage: "worker", status: "running", summary: "Worker running" });
  await save();
  return { dir, state, save, event: async (stage, status) => appendStageEvent(dir, state, { stage, status, summary: "Safe stage outcome" }) };
}

test("processed and notified checkpoints survive restarts without losing pending notifications", async (t) => {
  const { dir, state, event } = await fixture(t);
  await event("build", "completed");
  await acknowledgeMonitor(dir, state.id, { processedCursor: 2 });
  const replay = await monitorMain(["read", state.id], dir);
  assert.equal(replay.status.events.length, 0);
  assert.deepEqual(replay.notifications.map((item) => item.seq), [2]);
  await acknowledgeMonitor(dir, state.id, { notifiedCursor: 2 });
  assert.equal((await readMonitor(dir, state.id)).decision, "DONT_NOTIFY");
  assert.equal(JSON.parse(await readFile(resolve(dir, state.id, "monitor.json"))).notifiedCursor, 2);
  await assert.rejects(acknowledgeMonitor(dir, state.id, { processedCursor: 1 }), /monotonic/);
  await assert.rejects(acknowledgeMonitor(dir, state.id, { notifiedCursor: 3 }), /monotonic/);
});

test("acknowledged unchanged pauses are quiet; changed cause and terminal completion notify", async (t) => {
  const { dir, state, save, event } = await fixture(t);
  state.phase = "awaiting"; state.resumePhase = "build"; state.pauseReason = "Task reached 81% of the weekly Codex quota SECRET=never-publish";
  await save(); await event("workflow", "paused");
  assert.equal((await readMonitor(dir, state.id)).notifications[0].pause.category, "quota");
  await acknowledgeMonitor(dir, state.id, { processedCursor: 2, notifiedCursor: 2 });
  await event("workflow", "paused");
  assert.equal((await readMonitor(dir, state.id)).decision, "DONT_NOTIFY");
  await acknowledgeMonitor(dir, state.id, { processedCursor: 3 });
  state.pauseReason = "Main checkout dirty SECRET=never-publish";
  await save(); await event("workflow", "paused");
  const changed = await readMonitor(dir, state.id);
  assert.equal(changed.notifications.at(-1).pause.category, "git");
  assert.ok(!JSON.stringify(changed).includes("never-publish"));
  await acknowledgeMonitor(dir, state.id, { processedCursor: 4, notifiedCursor: 4 });
  state.phase = "done"; await save(); await event("integrate", "completed");
  const done = await readMonitor(dir, state.id);
  assert.equal(done.terminal, true);
  assert.deepEqual(done.notifications.map((item) => item.seq), [5]);
});

test("cross-run and malformed checkpoints fail closed; read never creates or mutates", async (t) => {
  const { dir, state } = await fixture(t);
  await assert.rejects(readMonitor(dir, "../escape"), /run ID/);
  await writeFile(resolve(dir, state.id, "monitor.json"), JSON.stringify({ version: 1, runId: "other", processedCursor: 0, notifiedCursor: 0 }));
  await assert.rejects(readMonitor(dir, state.id), /cross-run/);
  await writeFile(resolve(dir, state.id, "monitor.json"), "partial");
  await assert.rejects(readMonitor(dir, state.id));
  await assert.rejects(acknowledgeMonitor(dir, "absent", { processedCursor: 0 }), /does not exist/);
  await assert.rejects(monitorMain(["ack", state.id, "--processed", "1", "--processed", "1"], dir), /options/);
});

test("concurrent independent acknowledgements never roll back delivered cursors", async (t) => {
  const { dir, state, event } = await fixture(t);
  await event("build", "completed");
  const results = await Promise.allSettled([
    acknowledgeMonitor(dir, state.id, { processedCursor: 2, notifiedCursor: 2 }),
    acknowledgeMonitor(dir, state.id, { processedCursor: 1, notifiedCursor: 1 }),
  ]);
  assert.equal(results.filter((item) => item.status === "fulfilled").length >= 1, true);
  const report = await readMonitor(dir, state.id);
  assert.equal(report.checkpoint.processedCursor, 2);
  assert.equal(report.checkpoint.notifiedCursor, 2);
});

test("acknowledging an earlier pause does not hide a changed reason arriving after read", async (t) => {
  const { dir, state, save, event } = await fixture(t);
  state.phase = "awaiting"; state.resumePhase = "parallel-work";
  state.currentFailure = classifyFailure({ code: "CLI_TOO_OLD" });
  await save(); await event("workflow", "paused");
  const first = await readMonitor(dir, state.id);
  assert.equal(first.notifications.at(-1).pause.diagnosticCode, "CLI_TOO_OLD");
  // Another controller outcome arrives while the notification is delivered.
  state.currentFailure = classifyFailure({ code: "HOST_CLI_UNAVAILABLE" });
  await save(); await event("workflow", "paused");
  await acknowledgeMonitor(dir, state.id, { processedCursor: first.status.cursor, notifiedCursor: first.status.cursor });
  const newer = await readMonitor(dir, state.id);
  assert.equal(newer.decision, "NOTIFY");
  assert.equal(newer.notifications.at(-1).seq, 3);
  assert.equal(newer.notifications.at(-1).pause.diagnosticCode, "HOST_CLI_UNAVAILABLE");
  await acknowledgeMonitor(dir, state.id, { processedCursor: 3, notifiedCursor: 3 });
  assert.equal((await readMonitor(dir, state.id)).decision, "DONT_NOTIFY");
});

test("ack remains attached to event guidance when state is saved before the next pause event", async (t) => {
  const { dir, state, save, event } = await fixture(t);
  state.phase = "awaiting"; state.resumePhase = "parallel-work";
  state.currentFailure = classifyFailure({ code: "CLI_TOO_OLD" });
  await save(); await event("workflow", "paused");
  state.currentFailure = classifyFailure({ code: "HOST_CLI_UNAVAILABLE" });
  await save(); // Controller state writes precede its durable event append.
  assert.equal((await readMonitor(dir, state.id)).notifications.at(-1).pause.diagnosticCode, "CLI_TOO_OLD");
  await acknowledgeMonitor(dir, state.id, { processedCursor: 2, notifiedCursor: 2 });
  await event("workflow", "paused");
  assert.equal((await readMonitor(dir, state.id)).decision, "NOTIFY");
});

test("partial allowance pauses distinguish remaining build and test guards at the same checkpoint", async (t) => {
  const { dir, state, save, event } = await fixture(t);
  state.phase = "awaiting"; state.resumePhase = "build";
  state.allowanceContinuation = { remainingGuards: [{ kind: "build" }, { kind: "test" }] };
  await save(); await event("workflow", "paused");
  await acknowledgeMonitor(dir, state.id, { processedCursor: 2, notifiedCursor: 2 });
  state.allowanceContinuation.remainingGuards = [{ kind: "test" }];
  await save(); await event("workflow", "paused");
  const report = await readMonitor(dir, state.id);
  assert.equal(report.decision, "NOTIFY");
  assert.deepEqual(report.notifications.at(-1).pause.remainingGuards.map((guard) => guard.kind), ["test"]);
});

test("a paused state without a journal can acknowledge its initial notification", async (t) => {
  const { dir } = await fixture(t);
  await mkdir(resolve(dir, "state-only"));
  await writeFile(resolve(dir, "state-only", "state.json"), JSON.stringify({ id: "state-only", phase: "awaiting", resumePhase: "plan", pauseReason: "Paused by user", userRequestedPause: true }));
  assert.equal((await readMonitor(dir, "state-only")).decision, "NOTIFY");
  await acknowledgeMonitor(dir, "state-only", { processedCursor: 0, notifiedCursor: 0 });
  assert.equal((await readMonitor(dir, "state-only")).decision, "DONT_NOTIFY");
});

test("explicit monitor CLI reads and acknowledges the durable per-run checkpoint", async (t) => {
  const { dir } = await fixture(t);
  const scripts = resolve(dir, "cli");
  await mkdir(scripts);
  for (const name of ["events.mjs", "failure.mjs", "monitor.mjs"]) await copyFile(resolve(import.meta.dirname, "../.sandcastle", name), resolve(scripts, name));
  const runs = resolve(scripts, "runs");
  await appendStageEvent(runs, { id: "cli-run", phase: "build" }, { stage: "build", status: "completed", summary: "Build completed" });
  const invoke = async (args) => {
    const path = resolve(dir, "cli-output.json");
    const output = await open(path, "w");
    try {
      const child = spawn(process.execPath, [resolve(scripts, "monitor.mjs"), ...args], { stdio: ["ignore", output.fd, "ignore"] });
      assert.equal(await new Promise((done, reject) => { child.on("error", reject); child.on("exit", done); }), 0);
    } finally { await output.close(); }
    return JSON.parse(await readFile(path, "utf8"));
  };
  assert.equal((await invoke(["read", "cli-run"])).decision, "NOTIFY");
  assert.equal((await invoke(["ack", "cli-run", "--processed", "1", "--notified", "1"])).notifiedCursor, 1);
  assert.equal((await invoke(["read", "cli-run"])).decision, "DONT_NOTIFY");
});

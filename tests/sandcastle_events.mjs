import assert from "node:assert/strict";
import test from "node:test";
import { appendFile, chmod, copyFile, mkdir, mkdtemp, open, readFile, rm, writeFile } from "node:fs/promises";
import { execFile, spawn } from "node:child_process";
import { promisify } from "node:util";
import { resolve } from "node:path";
import { appendStageEvent, readRunStatus, stageOutcome } from "../.sandcastle/events.mjs";

const execute = promisify(execFile);
const modulePath = resolve(import.meta.dirname, "../.sandcastle/events.mjs");
const state = { id: "test-run", branch: "codex/test", phase: "tickets", completedTickets: ["one"], tickets: [{ path: "two" }], ticketIndex: 0 };
const details = (summary = "Build completed") => ({ stage: "build", status: "completed", summary });

test("stage reports distinguish failed gates, review findings and coverage scheduling", () => {
  assert.equal(stageOutcome({ phase: "build-repair" }, "build").status, "failed");
  assert.equal(stageOutcome({ phase: "test-repair" }, "test-gate").status, "failed");
  assert.equal(stageOutcome({ phase: "test" }, "build").status, "completed");
  const review = stageOutcome({ phase: "plan", reviewRound: 3, reviewFindings: [1, 2] }, "review");
  assert.equal(review.status, "needs-fixes");
  assert.match(review.summary, /2 findings/);
  assert.equal(stageOutcome({ phase: "parallel-work" }, "test").status, "scheduled");
  assert.equal(stageOutcome({ phase: "report" }, "review").status, "completed");
});

test("live wave status exposes worker outcomes without their private prompts or results", async (t) => {
  const dir = await fixture(t);
  const current = { ...state, phase: "assemble", wave: { id: 1, after: "tickets", baseCommit: "a".repeat(40), members: [
    { ticket: { path: "one", prompt: "private-test-prompt" }, branch: "codex/one", status: "completed", result: { summary: "private-test-prompt" } },
    { ticket: { path: "two" }, branch: "codex/two", status: "blocked", error: "private-test-prompt" },
  ] } };
  const outcome = stageOutcome(current, "parallel-work");
  assert.equal(outcome.status, "partial");
  assert.match(outcome.summary, /1\/2/);
  await appendStageEvent(dir, current, { stage: "parallel-work", ...outcome });
  await writeFile(resolve(dir, state.id, "state.json"), JSON.stringify(current));
  const report = await readRunStatus(dir, state.id);
  assert.deepEqual(report.wave.members.map((member) => member.status), ["completed", "blocked"]);
  assert.ok(!JSON.stringify(report).includes("private-test-prompt"));
});
async function fixture(t) {
  const dir = await mkdtemp("/tmp/sandcastle-events-");
  t.after(() => rm(dir, { recursive: true, force: true }));
  return dir;
}
function hookEnvironment(t, path, timeout = "5000") {
  const previous = { hook: process.env.SANDCASTLE_STATUS_HOOK, timeout: process.env.SANDCASTLE_STATUS_HOOK_TIMEOUT_MS };
  process.env.SANDCASTLE_STATUS_HOOK = path;
  process.env.SANDCASTLE_STATUS_HOOK_TIMEOUT_MS = timeout;
  t.after(() => {
    for (const [key, value] of [["SANDCASTLE_STATUS_HOOK", previous.hook], ["SANDCASTLE_STATUS_HOOK_TIMEOUT_MS", previous.timeout]]) {
      if (value === undefined) delete process.env[key]; else process.env[key] = value;
    }
  });
}

test("concurrent stage endings are serialized with a durable latest snapshot", async (t) => {
  const dir = await fixture(t);
  const events = await Promise.all(Array.from({ length: 12 }, (_, index) => appendStageEvent(dir, state, details(`Stage ${index}`))));
  assert.deepEqual(events.map((event) => event.seq), Array.from({ length: 12 }, (_, index) => index + 1));
  assert.deepEqual(JSON.parse(await readFile(resolve(dir, state.id, "events.latest.json"))), events.at(-1));
  const status = await readRunStatus(dir, state.id, { after: 9 });
  assert.equal(status.cursor, 12);
  assert.deepEqual(status.events.map((event) => event.seq), [10, 11, 12]);
  assert.equal((await readRunStatus(dir, state.id, { after: 12 })).events.length, 0);
});

test("restart and independent processes continue the journal sequence", async (t) => {
  const dir = await fixture(t);
  await appendStageEvent(dir, state, details());
  const code = `import { appendStageEvent } from ${JSON.stringify(modulePath)}; await appendStageEvent(${JSON.stringify(dir)}, ${JSON.stringify(state)}, ${JSON.stringify(details("Child finished"))});`;
  await Promise.all(Array.from({ length: 4 }, () => execute(process.execPath, ["--input-type=module", "-e", code])));
  const result = await appendStageEvent(dir, state, details("After restart"));
  assert.equal(result.seq, 6);
  assert.deepEqual((await readRunStatus(dir, state.id)).events.map((event) => event.seq), [1, 2, 3, 4, 5, 6]);
});

test("an interrupted trailing append is ignored then repaired without losing prior events", async (t) => {
  const dir = await fixture(t);
  await appendStageEvent(dir, state, details("First"));
  await appendFile(resolve(dir, state.id, "events.jsonl"), '{"seq":2,"summary":"partial');
  assert.equal((await readRunStatus(dir, state.id)).cursor, 1);
  assert.equal((await appendStageEvent(dir, state, details("Recovered"))).seq, 2);
  assert.deepEqual((await readRunStatus(dir, state.id)).events.map((event) => event.summary), ["First", "Recovered"]);
});

test("reports whitelist fields instead of copying specs, prompts or credentials", async (t) => {
  const dir = await fixture(t);
  const secret = "credential-should-never-appear";
  const unsafeState = { ...state, spec: secret, prompt: secret, env: { TOKEN: secret }, credentials: secret, pauseReason: secret };
  const event = await appendStageEvent(dir, unsafeState, { ...details(), spec: secret, prompts: secret, env: secret, head: "a".repeat(40), tickets: ["docs/tasks/task.md"] });
  await writeFile(resolve(dir, state.id, "state.json"), JSON.stringify(unsafeState));
  const report = await readRunStatus(dir, state.id);
  assert.ok(!JSON.stringify(report).includes(secret));
  assert.ok(!(await readFile(resolve(dir, state.id, "events.jsonl"), "utf8")).includes(secret));
  assert.deepEqual(event.tickets, ["docs/tasks/task.md"]);
  assert.equal(report.phase, "tickets");
  assert.equal(report.progress.completed, 1);
});

test("hook receives only an immutable event path; failure is recorded without interrupting stages", async (t) => {
  const dir = await fixture(t);
  const hook = resolve(dir, "hook.sh");
  await writeFile(hook, `#!/bin/sh\n[ "$#" = 1 ] || exit 8\ncat "$1" > "${dir}/received.json"\necho 'credential-should-never-appear' >&2\nexit 7\n`);
  await chmod(hook, 0o700);
  hookEnvironment(t, hook);
  const event = await appendStageEvent(dir, state, details());
  assert.deepEqual(JSON.parse(await readFile(resolve(dir, "received.json"))), event);
  const report = await readRunStatus(dir, state.id);
  assert.deepEqual(report.events[0].delivery, { seq: 1, status: "failed", reason: "execution-failed", exitCode: 7 });
  assert.ok(!JSON.stringify(report).includes("credential-should-never-appear"));
  assert.equal((await appendStageEvent(dir, state, details("Next stage"))).seq, 2);
});

test("hook timeout is finite and a relative executable is rejected safely", async (t) => {
  const dir = await fixture(t);
  const hook = resolve(dir, "slow-hook.sh");
  await writeFile(hook, "#!/bin/sh\nexec sleep 10\n");
  await chmod(hook, 0o700);
  hookEnvironment(t, hook, "30");
  const before = Date.now();
  await appendStageEvent(dir, state, details());
  assert.ok(Date.now() - before < 2000);
  assert.equal((await readRunStatus(dir, state.id)).events[0].delivery.reason, "timeout-or-output-limit");
  process.env.SANDCASTLE_STATUS_HOOK = "relative-hook";
  await appendStageEvent(dir, state, details());
  assert.equal((await readRunStatus(dir, state.id)).events[1].delivery.reason, "hook-must-be-absolute");
});

test("successful hook delivery does not inherit workflow credentials", async (t) => {
  const dir = await fixture(t);
  const hook = resolve(dir, "successful-hook.sh");
  await writeFile(hook, '#!/bin/sh\n[ "$#" = 1 ] && [ -z "$SANDCASTLE_TEST_TOKEN" ] && [ -f "$1" ]\n');
  await chmod(hook, 0o700);
  hookEnvironment(t, hook);
  const previous = process.env.SANDCASTLE_TEST_TOKEN;
  process.env.SANDCASTLE_TEST_TOKEN = "workflow-credential";
  t.after(() => { if (previous === undefined) delete process.env.SANDCASTLE_TEST_TOKEN; else process.env.SANDCASTLE_TEST_TOKEN = previous; });
  await appendStageEvent(dir, state, details());
  assert.equal((await readRunStatus(dir, state.id)).events[0].delivery.status, "delivered");
});

test("CLI reads multiple cursor events and excludes sensitive state", async (t) => {
  const dir = await fixture(t);
  const scripts = resolve(dir, ".sandcastle");
  await mkdir(scripts);
  await copyFile(modulePath, resolve(scripts, "events.mjs"));
  await copyFile(resolve(import.meta.dirname, "../.sandcastle/status.mjs"), resolve(scripts, "status.mjs"));
  const runs = resolve(scripts, "runs");
  for (let index = 0; index < 3; index++) await appendStageEvent(runs, state, details(`Stage ${index}`));
  await writeFile(resolve(runs, state.id, "state.json"), JSON.stringify({ ...state, spec: "secret spec" }));
  // A real descriptor also verifies the CLI when this host's sandbox suppresses
  // nested Node pipe output. Its return code and JSON remain the same interface.
  const outputPath = resolve(dir, "status-output.json");
  const output = await open(outputPath, "w");
  try {
    const child = spawn(process.execPath, [resolve(scripts, "status.mjs"), state.id, "--after", "1"], { stdio: ["ignore", output.fd, "ignore"] });
    const exitCode = await new Promise((done, reject) => { child.on("error", reject); child.on("exit", done); });
    assert.equal(exitCode, 0);
  } finally { await output.close(); }
  const stdout = await readFile(outputPath, "utf8");
  const report = JSON.parse(stdout);
  assert.deepEqual(report.events.map((event) => event.seq), [2, 3]);
  assert.equal(report.cursor, 3);
  assert.ok(!stdout.includes("secret spec"));
  await assert.rejects(execute(process.execPath, [resolve(scripts, "status.mjs"), state.id, "--after", "-1"]));
});

test("invalid IDs, cursors and event fields cannot escape the run or corrupt its journal", async (t) => {
  const dir = await fixture(t);
  await assert.rejects(appendStageEvent(dir, { id: "../elsewhere" }, details()), /Invalid Sandcastle run/);
  await assert.rejects(readRunStatus(dir, state.id, { after: -1 }), /Invalid event cursor/);
  await assert.rejects(appendStageEvent(dir, state, { ...details(), head: "not-a-hash" }), /Invalid event HEAD/);
  assert.equal((await appendStageEvent(dir, state, details())).seq, 1);
});

import { mkdir, open, readFile, rename, unlink } from "node:fs/promises";
import { isAbsolute, resolve } from "node:path";
import { execFile } from "node:child_process";

const queues = new Map();
const delay = (ms) => new Promise((done) => setTimeout(done, ms));

function runDirectory(runsDir, id) {
  if (typeof id !== "string" || !/^[a-zA-Z0-9][a-zA-Z0-9_-]{0,127}$/.test(id)) throw new Error("Invalid Sandcastle run ID");
  return resolve(runsDir, id);
}
function label(value, name) {
  if (typeof value !== "string" || !/^[a-zA-Z0-9][a-zA-Z0-9 _./:-]{0,127}$/.test(value)) throw new Error(`Invalid event ${name}`);
  return value;
}
function summary(value) {
  if (typeof value !== "string" || !value.trim()) throw new Error("Event summary is required");
  // Only caller-supplied summaries belong here. Never copy prompts, specs,
  // environment, agent output, or raw command errors into this channel.
  return value.replace(/[\x00-\x1f\x7f]/g, " ").slice(0, 2000);
}
function count(value) { return Number.isSafeInteger(value) && value >= 0 ? value : 0; }
function compactState(state) {
  return {
    id: state.id,
    ...(typeof state.branch === "string" ? { branch: state.branch } : {}),
    ...(typeof state.phase === "string" ? { phase: state.phase } : {}),
    ...(typeof state.resumePhase === "string" ? { resumePhase: state.resumePhase } : {}),
    ...(state.recovery ? { recovery: { id: count(state.recovery.id), checkpoint: state.recovery.phase, attempt: count(state.recovery.attempt) } } : {}),
    ...(state.wave ? { wave: {
      id: count(state.wave.id), after: state.wave.after,
      members: (state.wave.members ?? []).map((member) => ({ ticket: member.ticket?.path, branch: member.branch, status: member.status, integrated: member.integrated === true })),
    } } : {}),
    progress: {
      completed: Array.isArray(state.completedTickets) ? state.completedTickets.length : 0,
      ticketIndex: count(state.ticketIndex),
      batchTickets: Array.isArray(state.tickets) ? state.tickets.length : 0,
      reviewRound: count(state.reviewRound),
      buildAttempts: count(state.totalBuildAttempts),
      testAttempts: count(state.totalTestAttempts),
    },
  };
}

export function stageOutcome(state, stage) {
  const failedGate = (stage === "build" || stage === "test-gate") && ["build-repair", "test-repair"].includes(state.phase);
  if (failedGate) return { status: "failed", summary: `${stage}: gate failed; next stage ${state.phase}.` };
  if (stage === "review" && state.phase === "plan") return { status: "needs-fixes", summary: `Review ${state.reviewRound}: ${(state.reviewFindings ?? []).length} findings; preparing repair tickets.` };
  if (stage === "parallel-work") {
    const members = state.wave?.members ?? [];
    const successful = members.filter((member) => member.status === "completed").length;
    return { status: successful === members.length ? "completed" : "partial", summary: `Workers finished: ${successful}/${members.length} completed; assembly follows before dependencies become ready.` };
  }
  if (stage === "test") return { status: "scheduled", summary: "Focused test coverage scheduled in an isolated worker; test gates are still pending." };
  if (stage === "tickets" && state.phase === "parallel-work") return { status: "scheduled", summary: `Scheduled ${state.wave.members.length} ready tickets in wave ${state.wave.id}.` };
  return { status: "completed", summary: `${stage}: finished; next stage ${state.phase}.` };
}
async function optionalRead(path) {
  try { return await readFile(path, "utf8"); }
  catch (error) { if (error.code === "ENOENT") return ""; throw error; }
}
function parseJournal(content) {
  const end = content.lastIndexOf("\n") + 1;
  const events = content.slice(0, end).split("\n").filter(Boolean).map((line) => JSON.parse(line));
  for (let index = 0; index < events.length; index++) {
    if (events[index].seq !== index + 1) throw new Error("Invalid Sandcastle event sequence");
  }
  return { events, validBytes: Buffer.byteLength(content.slice(0, end)) };
}
async function durableWrite(path, content) {
  const file = await open(path, "w", 0o600);
  try { await file.writeFile(content); await file.sync(); }
  finally { await file.close(); }
}
async function durableAppend(path, content, validBytes) {
  const file = await open(path, "a+", 0o600);
  try {
    if (validBytes !== undefined) await file.truncate(validBytes);
    await file.writeFile(content);
    await file.sync();
  } finally { await file.close(); }
}
async function acquireLock(dir) {
  const path = resolve(dir, "events.lock");
  const deadline = Date.now() + 35000;
  while (true) {
    try {
      const file = await open(path, "wx", 0o600);
      await file.writeFile(`${process.pid}\n`);
      await file.close();
      return async () => unlink(path);
    } catch (error) {
      if (error.code !== "EEXIST") throw error;
      const pid = Number((await optionalRead(path)).trim());
      if (Number.isSafeInteger(pid) && pid > 0) {
        try { process.kill(pid, 0); }
        catch (error) { if (error.code === "ESRCH") { await unlink(path).catch((e) => { if (e.code !== "ENOENT") throw e; }); continue; } }
      }
      if (Date.now() >= deadline) throw new Error("Timed out acquiring Sandcastle event lock");
      await delay(20);
    }
  }
}
async function deliver(path) {
  const executable = process.env.SANDCASTLE_STATUS_HOOK;
  if (!executable) return undefined;
  if (!isAbsolute(executable)) return { status: "failed", reason: "hook-must-be-absolute" };
  const requested = Number(process.env.SANDCASTLE_STATUS_HOOK_TIMEOUT_MS);
  const timeout = Number.isFinite(requested) && requested > 0 ? Math.min(requested, 30000) : 5000;
  return new Promise((done) => {
    execFile(executable, [path], { timeout, killSignal: "SIGKILL", maxBuffer: 1024, env: { PATH: process.env.PATH ?? "/usr/bin:/bin" } }, (error) => {
      if (!error) return done({ status: "delivered" });
      done({ status: "failed", reason: error.killed ? "timeout-or-output-limit" : "execution-failed", ...(Number.isInteger(error.code) ? { exitCode: error.code } : {}) });
    });
  });
}

// Per-process queue plus an on-disk lock serialize independent workers. The
// fsynced journal is authoritative; latest is a convenient replaceable snapshot.
export async function appendStageEvent(runsDir, state, details) {
  const dir = runDirectory(runsDir, state.id);
  const preceding = queues.get(dir) ?? Promise.resolve();
  const operation = preceding.catch(() => {}).then(async () => {
    await mkdir(dir, { recursive: true });
    const unlock = await acquireLock(dir);
    try {
      const path = resolve(dir, "events.jsonl");
      const { events, validBytes } = parseJournal(await optionalRead(path));
      const event = {
        seq: events.length + 1, time: new Date().toISOString(), runId: state.id,
        ...(typeof state.branch === "string" ? { branch: state.branch } : {}),
        stage: label(details.stage, "stage"), status: label(details.status, "status"),
        summary: summary(details.summary), progress: compactState(state).progress,
      };
      if (details.tickets !== undefined) {
        if (!Array.isArray(details.tickets) || details.tickets.some((ticket) => typeof ticket !== "string" || /[\x00-\x1f]/.test(ticket))) throw new Error("Invalid event tickets");
        event.tickets = details.tickets.map((ticket) => ticket.slice(0, 1024));
      }
      if (details.head !== undefined) {
        if (!/^[0-9a-f]{7,64}$/.test(details.head)) throw new Error("Invalid event HEAD");
        event.head = details.head;
      }
      const serialized = JSON.stringify(event) + "\n";
      await durableAppend(path, serialized, validBytes);
      await durableWrite(resolve(dir, "events.latest.json.tmp"), serialized);
      await rename(resolve(dir, "events.latest.json.tmp"), resolve(dir, "events.latest.json"));
      const directory = await open(dir, "r");
      try { await directory.sync(); } finally { await directory.close(); }
      if (process.env.SANDCASTLE_STATUS_HOOK) {
        let delivery;
        try {
          const eventPath = resolve(dir, `event-${String(event.seq).padStart(6, "0")}.json`);
          await durableWrite(eventPath, serialized);
          delivery = await deliver(eventPath);
        } catch { delivery = { status: "failed", reason: "delivery-setup-failed" }; }
        // Notification availability cannot stop implementation. If the disk
        // cannot store its outcome, the durable stage event still remains.
        await durableAppend(resolve(dir, "deliveries.jsonl"), JSON.stringify({ seq: event.seq, ...delivery }) + "\n").catch(() => {});
      }
      return event;
    } finally { await unlock(); }
  });
  queues.set(dir, operation);
  try { return await operation; }
  finally { if (queues.get(dir) === operation) queues.delete(dir); }
}

export async function readRunStatus(runsDir, id, { after = 0 } = {}) {
  if (!Number.isSafeInteger(after) || after < 0) throw new Error("Invalid event cursor");
  const dir = runDirectory(runsDir, id);
  const [stateText, journal, deliveryText] = await Promise.all([
    optionalRead(resolve(dir, "state.json")), optionalRead(resolve(dir, "events.jsonl")), optionalRead(resolve(dir, "deliveries.jsonl")),
  ]);
  if (!stateText && !journal) throw new Error(`Sandcastle run ${id} does not exist`);
  const { events } = parseJournal(journal);
  const deliveries = new Map(deliveryText.slice(0, deliveryText.lastIndexOf("\n") + 1).split("\n").filter(Boolean).map((line) => { const item = JSON.parse(line); return [item.seq, item]; }));
  return {
    ...compactState(stateText ? JSON.parse(stateText) : { id }),
    cursor: events.at(-1)?.seq ?? 0,
    events: events.filter((event) => event.seq > after).map((event) => deliveries.has(event.seq) ? { ...event, delivery: deliveries.get(event.seq) } : event),
  };
}

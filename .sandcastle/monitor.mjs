import { mkdir, open, readFile, rename, unlink } from "node:fs/promises";
import { resolve } from "node:path";
import { createHash } from "node:crypto";
import { readRunStatus } from "./events.mjs";

const validId = (id) => typeof id === "string" && /^[a-zA-Z0-9][a-zA-Z0-9_-]{0,127}$/.test(id);
const validCursor = (value) => Number.isSafeInteger(value) && value >= 0;
function directory(runsDir, id) {
  if (!validId(id)) throw new Error("Invalid Sandcastle run ID");
  return resolve(runsDir, id);
}
function initial(runId) { return { version: 1, runId, processedCursor: 0, notifiedCursor: 0 }; }
function validate(checkpoint, id) {
  if (checkpoint.version !== 1 || checkpoint.runId !== id || !validCursor(checkpoint.processedCursor) || !validCursor(checkpoint.notifiedCursor) || checkpoint.notifiedCursor > checkpoint.processedCursor ||
    (checkpoint.lastPauseSignature !== undefined && !/^(?:[a-f0-9]{64}|[a-z_-]+:[a-z-]*(?::[A-Z_]*)?)$/.test(checkpoint.lastPauseSignature))) throw new Error("Invalid or cross-run monitor checkpoint");
  // Never copy arbitrary fields from a checkpoint into public status.
  return { ...initial(id), processedCursor: checkpoint.processedCursor, notifiedCursor: checkpoint.notifiedCursor,
    ...(checkpoint.lastPauseSignature ? { lastPauseSignature: checkpoint.lastPauseSignature } : {}) };
}
async function checkpointAt(dir, id) {
  try { return validate(JSON.parse(await readFile(resolve(dir, "monitor.json"), "utf8")), id); }
  catch (error) { if (error.code === "ENOENT") return initial(id); throw error; }
}
const pauseSignature = (status) => status.pause ? createHash("sha256").update(JSON.stringify({
  code: status.pause.code, checkpoint: status.pause.resume.checkpoint ?? "", diagnostic: status.pause.diagnosticCode ?? "",
  guards: (status.pause.remainingGuards ?? []).map((guard) => guard.kind),
})).digest("hex") : undefined;
const significant = (event) => ["completed", "failed", "needs-fixes", "ready", "ready_with_notes", "blocked", "done"].includes(event.status);

// Reading does not acknowledge delivery. A monitor restart can replay an
// unacknowledged outcome even when it already acknowledged processing it.
export async function readMonitor(runsDir, id) {
  const dir = directory(runsDir, id);
  const checkpoint = await checkpointAt(dir, id);
  const status = await readRunStatus(runsDir, id, { after: Math.min(checkpoint.processedCursor, checkpoint.notifiedCursor) });
  if (checkpoint.processedCursor > status.cursor) throw new Error("Monitor cursor is ahead of the run journal");
  const outcomes = status.events.filter((event) => event.seq > checkpoint.notifiedCursor && significant(event));
  // Pause notifications contain structured current guidance, not raw errors.
  // Repeated copies of an acknowledged unchanged pause remain quiet.
  const pausedEvent = status.events.filter((event) => event.seq > checkpoint.notifiedCursor && event.status === "paused").at(-1);
  const pause = pausedEvent?.pause ?? status.pause;
  const signature = pauseSignature({ pause });
  if (status.pause && signature !== checkpoint.lastPauseSignature && (pausedEvent || checkpoint.notifiedCursor === 0)) outcomes.push({ seq: pausedEvent?.seq ?? status.cursor, stage: "workflow", status: "paused", pause });
  outcomes.sort((a, b) => a.seq - b.seq);
  return { runId: id, checkpoint, status: { ...status, events: status.events.filter((event) => event.seq > checkpoint.processedCursor) }, notifications: outcomes, decision: outcomes.length ? "NOTIFY" : "DONT_NOTIFY", terminal: status.phase === "done" };
}

async function lock(dir) {
  const path = resolve(dir, "monitor.lock");
  const deadline = Date.now() + 10000;
  while (true) {
    try {
      const file = await open(path, "wx", 0o600);
      try { await file.writeFile(String(process.pid)); await file.sync(); } finally { await file.close(); }
      return () => unlink(path);
    } catch (error) {
      if (error.code !== "EEXIST") throw error;
      let pid;
      try { pid = Number(await readFile(path, "utf8")); } catch (error) { if (error.code === "ENOENT") continue; throw error; }
      if (Number.isSafeInteger(pid) && pid > 0) {
        try { process.kill(pid, 0); }
        catch (error) { if (error.code === "ESRCH") { await unlink(path).catch((e) => { if (e.code !== "ENOENT") throw e; }); continue; } }
      }
      if (Date.now() >= deadline) throw new Error("Timed out acquiring monitor checkpoint lock");
      await new Promise((done) => setTimeout(done, 10));
    }
  }
}

export async function acknowledgeMonitor(runsDir, id, { processedCursor, notifiedCursor } = {}) {
  if ((processedCursor !== undefined && !validCursor(processedCursor)) || (notifiedCursor !== undefined && !validCursor(notifiedCursor)) || (processedCursor === undefined && notifiedCursor === undefined)) throw new Error("Explicit valid monitor cursors are required");
  const dir = directory(runsDir, id);
  // Validate the run before creating any checkpoint or lock.
  await readRunStatus(runsDir, id);
  await mkdir(dir, { recursive: true });
  const unlock = await lock(dir);
  try {
    const previous = await checkpointAt(dir, id);
    const status = await readRunStatus(runsDir, id);
    const next = { ...previous, processedCursor: processedCursor ?? previous.processedCursor, notifiedCursor: notifiedCursor ?? previous.notifiedCursor };
    if (next.processedCursor < previous.processedCursor || next.notifiedCursor < previous.notifiedCursor || next.processedCursor > status.cursor || next.notifiedCursor > next.processedCursor) throw new Error("Monitor acknowledgement must be monotonic and within the processed journal");
    const pausedEvent = status.events.filter((event) => event.status === "paused").at(-1);
    // Only bind the current reason to acknowledgement of its own event. A
    // later pause can arrive between read and ack; acknowledging the earlier
    // notification must not silently acknowledge that new reason as well.
    if (status.pause && next.notifiedCursor > previous.notifiedCursor && pausedEvent && pausedEvent.seq <= next.notifiedCursor && !status.events.some((event) => event.seq > pausedEvent.seq)) next.lastPauseSignature = pauseSignature(pausedEvent.pause ? { pause: pausedEvent.pause } : status);
    if (status.pause && status.cursor === 0 && notifiedCursor === 0) next.lastPauseSignature = pauseSignature(status);
    if (!status.pause && next.notifiedCursor >= status.cursor) delete next.lastPauseSignature;
    const tmp = resolve(dir, "monitor.json.tmp");
    const file = await open(tmp, "w", 0o600);
    try { await file.writeFile(JSON.stringify(next) + "\n"); await file.sync(); } finally { await file.close(); }
    await rename(tmp, resolve(dir, "monitor.json"));
    const handle = await open(dir, "r");
    try { await handle.sync(); } finally { await handle.close(); }
    return next;
  } finally { await unlock(); }
}

export async function monitorMain(args = process.argv.slice(2), runsDir = resolve(import.meta.dirname, "runs")) {
  const [operation, id, ...options] = args;
  if (!validId(id) || !["read", "ack"].includes(operation)) throw new Error("Usage: sandbox:monitor read <run-id> | ack <run-id> [--processed seq] [--notified seq]");
  if (operation === "read") {
    if (options.length) throw new Error("Monitor read accepts no options");
    return readMonitor(runsDir, id);
  }
  const cursors = {};
  for (let i = 0; i < options.length; i += 2) {
    const key = options[i] === "--processed" ? "processedCursor" : options[i] === "--notified" ? "notifiedCursor" : undefined;
    if (!key || cursors[key] !== undefined || !/^\d+$/.test(options[i + 1] ?? "")) throw new Error("Invalid monitor acknowledgement options");
    cursors[key] = Number(options[i + 1]);
  }
  return acknowledgeMonitor(runsDir, id, cursors);
}
if (process.argv[1] && resolve(process.argv[1]) === resolve(import.meta.filename)) {
  try { console.log(JSON.stringify(await monitorMain(), null, 2)); }
  catch { console.error("Sandcastle monitor request failed; inspect the run ID, journal and acknowledgement cursors."); process.exitCode = 1; }
}

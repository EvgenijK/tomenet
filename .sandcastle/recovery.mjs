import { access, mkdir, readFile, writeFile } from "node:fs/promises";
import { resolve } from "node:path";
import { validateTicketBatch } from "./tickets.mjs";
import { taskQuotaLimit } from "./limits.mjs";

// The AI chooses an action; this controller alone changes scheduler state.
// Quota, gate limits and successful commit verification are never overridden.
export function recoveryRestriction(state, error = "") {
  if (state.quotaError || (state.budget?.consumedPercent ?? 0) >= taskQuotaLimit(state)) return "Quota guard requires human continuation";
  if (/quota|account usage limit|token limit|human approval/i.test(error)) return "Usage or cycle guard requires human continuation";
  if (["build", "build-repair", "test-gate"].includes(state.phase) && state.buildAttempts >= state.buildLimit) return "Build attempt limit reached";
  if (["test-gate", "test-repair"].includes(state.phase) && state.testAttempts >= state.testLimit) return "Test attempt limit reached";
  if (state.reviewRound >= state.reviewLimit && state.reviewFindings?.length) return "Review round limit reached";
  return undefined;
}

export function beginRecovery(state, error, stage = state.phase) {
  const message = String(error?.message ?? error);
  const restriction = recoveryRestriction(state, message);
  if (restriction) throw new Error(restriction);
  const key = [state.phase, state.wave?.id ?? 0, state.completedTickets.length, state.reviewRound, state.buildAttempts, state.testAttempts].join(":");
  const attempts = (state.recoveryAttempts ??= {});
  if ((attempts[key] ?? 0) >= 3) throw new Error("Automatic recovery made no progress after three decisions at this checkpoint");
  attempts[key] = (attempts[key] ?? 0) + 1;
  const id = state.recoverySequence = (state.recoverySequence ?? 0) + 1;
  state.recovery = { id, key, phase: state.phase, stage, error: message.slice(-8000), attempt: attempts[key] };
  state.phase = "recovery";
  return state.recovery;
}

export function recoveryContext(state) {
  return {
    incident: state.recovery, branch: state.branch, specPath: state.specPath,
    scopeNotes: state.scopeNotes, task: state.spec?.slice(0, 12000),
    tickets: state.tickets, completedTickets: state.completedTickets,
    deferredTickets: state.deferredTickets,
    wave: state.wave, pendingFailure: state.pendingFailure?.slice(-8000),
    previousDecisions: (state.recoveries ?? []).slice(-3),
  };
}

function validateDecision(decision) {
  if (!decision || Object.keys(decision).some((key) => !["action", "summary", "ticketPaths", "dependency"].includes(key)) ||
    !["retry", "repair", "defer", "stop"].includes(decision.action) ||
    typeof decision.summary !== "string" || !decision.summary.trim() || decision.summary.length > 4000 ||
    !Array.isArray(decision.ticketPaths) || decision.ticketPaths.some((path) => typeof path !== "string") ||
    new Set(decision.ticketPaths).size !== decision.ticketPaths.length || typeof decision.dependency !== "string") throw new Error("Invalid recovery decision");
  if (decision.action !== "defer" && (decision.ticketPaths.length || decision.dependency)) throw new Error("Only deferral may name tickets and a dependency");
}

export function previewRecovery(state, decision) {
  validateDecision(decision);
  const incident = state.recovery;
  if (!incident || state.phase !== "recovery") throw new Error("Missing recovery checkpoint");
  const restriction = recoveryRestriction({ ...state, phase: incident.phase });
  if (restriction) throw new Error(restriction);
  const next = structuredClone(state);
  const affected = [];
  next.phase = incident.phase;
  if (["retry", "repair"].includes(decision.action)) {
    next.recoveryAdvice = decision.summary;
    next.recoveryAdvicePhase = incident.phase;
  }
  if (decision.action === "repair") {
    if (["build", "build-repair"].includes(incident.phase)) next.phase = "build-repair";
    else if (["test-gate", "test-repair"].includes(incident.phase)) next.phase = "test-repair";
    else if (["plan", "plan-validate"].includes(incident.phase)) next.phase = "plan";
    else if (["parallel-work", "assemble"].includes(incident.phase)) next.phase = incident.phase;
    else throw new Error("Repairs require an existing worker/assembly, failed build/test gate, or planning checkpoint");
    next.pendingFailure ??= incident.error;
    next.recoveryAdvice = decision.summary;
  } else if (decision.action === "defer") {
    if (!decision.ticketPaths.length || !/^docs\/tasks\/[a-zA-Z0-9_./-]+\.md$/.test(decision.dependency) || decision.dependency.includes("..")) throw new Error("Deferral requires tickets and a real external dependency document");
    if (!["tickets", "parallel-work", "assemble"].includes(incident.phase) || (state.wave && state.wave.after !== "tickets")) throw new Error("Required testing and acceptance gates cannot be deferred");
    if (state.wave?.members.some((member) => member.status === "completed" && !member.integrated)) throw new Error("Integrate successful siblings before deferring a blocked worker");
    const blocked = new Set(state.wave?.members.filter((member) => ["blocked", "failed"].includes(member.status)).map((member) => member.ticket.path) ?? []);
    if (state.blockedTicket) blocked.add(state.blockedTicket.path);
    for (const path of decision.ticketPaths) {
      if (!blocked.has(path) || !state.tickets.some((ticket) => ticket.path === path) || state.completedTickets.includes(path)) throw new Error(`Cannot defer an unblocked, unknown or completed ticket: ${path}`);
    }
    const deferred = new Set(decision.ticketPaths);
    // A deferred prerequisite cannot accidentally unlock its downstream work.
    let changed;
    do {
      changed = false;
      for (const ticket of state.tickets) {
        if (!state.completedTickets.includes(ticket.path) && !deferred.has(ticket.path) && ticket.blockedBy.some((path) => deferred.has(path))) {
          deferred.add(ticket.path); changed = true;
        }
      }
    } while (changed);
    affected.push(...state.tickets.filter((ticket) => deferred.has(ticket.path)));
    next.deferredTickets ??= [];
    for (const ticket of affected) {
      const reason = `${decision.summary} Return after ${decision.dependency}; implementation and acceptance remain pending.`;
      if (!next.deferredTickets.some((item) => item.path === ticket.path)) next.deferredTickets.push({ path: ticket.path, reason, dependency: decision.dependency, recoveryId: incident.id });
    }
    next.tickets = state.tickets.filter((ticket) => !deferred.has(ticket.path));
    if (next.tickets.length) validateTicketBatch({ tickets: next.tickets }, next);
    if (next.wave) {
      // Retain unfinished branch outcomes, including any non-deferred failures,
      // instead of dropping the rest of a partially completed wave.
      next.wave.deferredTickets = next.deferredTickets.filter((item) => deferred.has(item.path));
      next.wave.members = next.wave.members.filter((member) => !deferred.has(member.ticket.path));
      if (next.wave.members.every((member) => member.integrated)) {
        (next.waveHistory ??= []).push({ ...state.wave, deferredTickets: next.wave.deferredTickets, status: "partial-deferred" });
        delete next.wave;
      }
    }
    next.phase = next.wave ? "parallel-work" : "tickets";
    delete next.blockedTicket;
    next.ticketIndex = next.tickets.filter((ticket) => next.completedTickets.includes(ticket.path)).length;
    next.scopeNotes = (next.scopeNotes ?? "") + `\nRecovery ${incident.id}: explicitly defer ${affected.map((ticket) => ticket.path).join(", ")} until ${decision.dependency}. ${decision.summary} Preserve original ownership and all pending acceptance. Do not implement this missing external dependency or recreate the same deferred caller as a repair ticket; continue reviewing applicable existing behavior.`;
  }
  const record = { id: incident.id, phase: incident.phase, ...decision, affectedTickets: affected.map((ticket) => ticket.path), recordPath: `.scratch/sandcastle-${state.id}/recovery/${incident.id}.md` };
  (next.recoveries ??= []).push(record);
  delete next.recovery;
  return { next, affected, record };
}

export async function runRecovery(state, ops) {
  const decision = state.recovery.decision ?? await ops.decide(recoveryContext(state));
  const preview = previewRecovery(state, decision);
  // Save the accepted decision before Git/file operations. Restart replays it
  // idempotently without consuming another agent call or repeating side effects.
  state.recovery.decision = decision;
  await ops.save(state);
  await ops.document(state.recovery, decision, preview);
  for (const key of Object.keys(state)) delete state[key];
  Object.assign(state, preview.next);
  await ops.save(state);
  await ops.event(preview.record);
  return decision.action !== "stop";
}

export function recoveryRecord(record) {
  return [`# Sandcastle recovery ${record.id}`, "", `Checkpoint: ${record.phase}`, `Action: ${record.action}`, "", record.summary, "",
    ...(record.dependency ? [`Return after: ${record.dependency}`, "", "Deferred implementation and acceptance remain pending.", ""] : []),
    ...record.affectedTickets.map((path) => `- ${path}`), ""].join("\n");
}

export async function documentRecovery(state, incident, decision, { next, affected, record }, { worktree, runsDir, git, ensureCommit, manifest: manifestPath }) {
  await writeFile(resolve(runsDir, state.id, `recovery-${incident.id}.json`), JSON.stringify(record, null, 2) + "\n");
  if (decision.action !== "defer") return;
  if (await git(worktree, "ls-files", "-u")) throw new Error("Resolve the interrupted merge before deferring tickets");
  if (decision.dependency === state.specPath) throw new Error("A ticket cannot defer to its own original owner");
  await access(resolve(worktree, decision.dependency));
  const marker = `Sandcastle recovery ${incident.id}`;
  for (const ticket of affected) {
    const path = resolve(worktree, ticket.path);
    let content = await readFile(path, "utf8");
    if (content.includes(marker)) continue;
    content = content.replace(/^\*{0,2}Status:\*{0,2}.*$/m, "Status: open").replace(/^\*{0,2}Assignee:\*{0,2}.*$/m, "Assignee: unassigned");
    const roles = new Set(["needs-triage", "needs-info", "ready-for-agent", "ready-for-human", "wontfix"]);
    const labels = (content.match(/^Labels:\s*(.*)$/m)?.[1] ?? "bug").split(/,\s*/).filter((label) => label && !roles.has(label));
    const labelLine = `Labels: ${[...labels, "needs-info"].join(", ")}`;
    content = /^Labels:.*$/m.test(content) ? content.replace(/^Labels:.*$/m, labelLine) : content.replace(/^Assignee:.*$/m, `$&\n${labelLine}`);
    content += `\n${content.includes("## Comments") ? "" : "## Comments\n\n"}- ${marker}: deferred until ${decision.dependency}. ${decision.summary} Ticket remains open; implementation and acceptance are not completed. Retained worker evidence: ${state.wave?.members.find((member) => member.ticket.path === ticket.path)?.head ?? "saved checkpoint"}.\n`;
    await writeFile(path, content);
  }
  const manifest = resolve(worktree, manifestPath);
  const snapshot = manifest.replace(/\.json$/, `-before-recovery-${incident.id}.json`);
  try { await access(snapshot); }
  catch (error) { if (error.code !== "ENOENT") throw error; await writeFile(snapshot, await readFile(manifest, "utf8")); }
  await writeFile(manifest, JSON.stringify({ tickets: next.tickets }, null, 2) + "\n");
  const note = `\n## ${marker}\n\n${decision.summary}\n\nDeferred: ${affected.map((ticket) => ticket.path).join(", ")}. Return after ${decision.dependency}; original ownership and all pending acceptance remain unchanged.\n`;
  for (const path of [`.scratch/sandcastle-${state.id}/map.md`, ...(state.specPath?.startsWith("docs/tasks/") && !state.specPath.includes("..") ? [state.specPath] : [])]) {
    const full = resolve(worktree, path);
    let content;
    try { content = await readFile(full, "utf8"); }
    catch (error) { if (error.code === "ENOENT") continue; throw error; }
    if (!content.includes(`## ${marker}`)) await writeFile(full, content + note);
  }
  await mkdir(resolve(worktree, `.scratch/sandcastle-${state.id}`, "recovery"), { recursive: true });
  await writeFile(resolve(worktree, record.recordPath), recoveryRecord(record));
  await ensureCommit(worktree, `Sandcastle: recovery ${incident.id} defers missing dependencies`);
}

import { recordTicketResult, validateTicketBatch } from "./tickets.mjs";

export function selectReadyTickets(state, limit) {
  if (!Number.isInteger(limit) || limit < 1 || limit > 10) throw new Error("Parallelism must be an integer from 1 to 10");
  const completed = new Set(state.completedTickets);
  return state.tickets.filter((ticket) => !completed.has(ticket.path) && ticket.blockedBy.every((path) => completed.has(path))).slice(0, limit);
}

export function createWave(state, { baseCommit, limit, after = "tickets", tickets = selectReadyTickets(state, limit), usedBranches = [] }) {
  if (state.wave) throw new Error("Finish the existing wave before creating another");
  if (!tickets.length) throw new Error("No ready tickets; inspect unresolved dependencies");
  if (!Array.isArray(usedBranches) || !usedBranches.every((branch) => typeof branch === "string" && branch)) throw new Error("Used worker branches must be a string array");
  validateTicketBatch({ tickets }, state);
  if (tickets.some((ticket) => state.completedTickets.includes(ticket.path) || ticket.blockedBy.some((path) => !state.completedTickets.includes(path)))) throw new Error("Wave contains completed or blocked tickets");
  const occupied = new Set(usedBranches);
  let id = (state.waveSequence ?? 0) + 1;
  while (tickets.some((_, index) => occupied.has(`codex/sandcastle-${state.id}-wave-${id}-${index + 1}`))) id++;
  state.waveSequence = id;
  state.wave = {
    id, baseCommit, after,
    members: tickets.map((ticket, index) => ({ ticket, branch: `codex/sandcastle-${state.id}-wave-${id}-${index + 1}`, status: "pending" })),
  };
  state.phase = "parallel-work";
  return state.wave;
}

export function resumeWorkerProvisioning(state, { usedBranches }) {
  const need = (condition, message) => { if (!condition) throw new Error(`Worker provisioning RESUME: ${message}`); };
  need(state?.phase === "awaiting" && state.resumePhase === "parallel-work", "requires a paused parallel-work checkpoint");
  const modern = state.wave?.members.some(member => member.failure?.deterministic);
  if (!modern) need(state.pauseReason === "Automatic recovery made no progress after three decisions at this checkpoint", "requires the saved no-progress recovery stop");
  need(state.wave && Array.isArray(state.wave.members) && state.wave.members.length > 0, "requires a saved worker wave");
  const unfinished = state.wave.members.filter(member => !member.integrated);
  const provisioningCodes = ["TICKET_UNREADABLE", "TICKET_NOT_COMMITTED", "BASE_MISMATCH", "BRANCH_MISMATCH"];
  need(unfinished.length > 0 && unfinished.every((member) => member.status === "failed" && !member.head && !member.result && !member.integrated &&
    (modern ? member.failure?.deterministic && provisioningCodes.includes(member.failure.code) : /^ENOENT: no such file or directory, open /.test(member.error ?? ""))), "requires only deterministic missing-ticket provisioning failures");
  need(state.wave.members.every(member => !member.integrated || state.completedTickets.includes(member.ticket.path)), "successful siblings must be integrated before reprovisioning");
  const oldWave = structuredClone(state.wave);
  const history = state.workerProvisioningResumes ?? [];
  need(!history.some((item) => item.oldWave.id === oldWave.id), "saved wave was already reprovisioned");
  const candidate = structuredClone(state);
  delete candidate.wave;
  createWave(candidate, {
    baseCommit: oldWave.baseCommit,
    after: oldWave.after,
    tickets: unfinished.map((member) => member.ticket),
    limit: unfinished.length,
    usedBranches,
  });
  state.waveSequence = candidate.waveSequence;
  state.wave = candidate.wave;
  state.phase = candidate.phase;
  state.workerProvisioningResumes = [...history, { oldWave, newWaveId: candidate.wave.id }];
  delete state.resumePhase;
  delete state.pauseReason;
  delete state.blockedTicket;
  delete state.recovery;
  return state;
}

function validResult(result) {
  if (!result || !["completed", "blocked"].includes(result.status) || typeof result.summary !== "string" || !result.summary.trim()) throw new Error("Invalid wave agent result");
}

export async function runWave(state, ops) {
  const wave = state.wave;
  if (!wave) throw new Error("Missing saved wave");
  const outcomes = await Promise.allSettled(wave.members.map(async (member) => {
    if (member.integrated || member.status === "completed") return;
    member.status = "running";
    delete member.error;
    delete member.failure;
    await ops.save(state);
    try {
      const output = await ops.run(member, wave);
      validResult(output.result);
      if (output.result.status === "completed" && !/^[a-f0-9]{40,64}$/.test(output.head ?? "")) throw new Error("Completed worker must provide a committed Git revision");
      member.result = output.result;
      member.head = output.head;
      member.status = output.result.status;
    } catch (error) {
      member.status = "failed";
      member.error = error.message;
      if (error.deterministic) member.failure = { code: error.code, deterministic: true };
    }
    await ops.save(state);
  }));
  const rejected = outcomes.find((outcome) => outcome.status === "rejected");
  if (rejected) throw rejected.reason;
  state.phase = "assemble";
  await ops.save(state);
}

export async function collectWave(state, ops) {
  const wave = state.wave;
  if (!wave) throw new Error("Missing saved wave");
  const completed = wave.members.filter((member) => member.status === "completed" && !member.integrated);
  if (completed.length) {
    const result = await ops.assemble(wave);
    validResult(result);
    wave.assembly = result;
    await ops.save(state);
    if (result.status === "blocked") throw new Error(`Assembly blocked: ${result.summary}`);
    // Verify the complete wave before publishing any new dependency readiness.
    for (const member of completed) await ops.verify(member, wave);
    for (const member of completed) {
      if (!member.integrated) {
        recordTicketResult(state, member.ticket, member.result);
        member.integrated = true;
        await ops.save(state);
      }
    }
  }
  state.ticketIndex = state.tickets.filter((ticket) => state.completedTickets.includes(ticket.path)).length;
  const failures = wave.members.filter((member) => member.status !== "completed");
  if (failures.length) {
    state.phase = "parallel-work";
    const failed = failures.find((member) => member.failure?.deterministic) ?? failures[0];
    state.blockedTicket = { path: failed.ticket.path, status: "blocked", summary: failed.error ?? failed.result?.summary ?? "Worker did not complete" };
    await ops.save(state);
    const error = new Error(`Wave ${wave.id} incomplete: ${state.blockedTicket.path}: ${state.blockedTicket.summary}`);
    if (failed.failure?.deterministic) Object.assign(error, failed.failure);
    throw error;
  }
  delete state.blockedTicket;
  (state.waveHistory ??= []).push(wave);
  delete state.wave;
  state.phase = wave.after;
  await ops.save(state);
}

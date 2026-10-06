import { recordTicketResult, validateTicketBatch } from "./tickets.mjs";

export function selectReadyTickets(state, limit) {
  if (!Number.isInteger(limit) || limit < 1 || limit > 10) throw new Error("Parallelism must be an integer from 1 to 10");
  const completed = new Set(state.completedTickets);
  return state.tickets.filter((ticket) => !completed.has(ticket.path) && ticket.blockedBy.every((path) => completed.has(path))).slice(0, limit);
}

export function createWave(state, { baseCommit, limit, after = "tickets", tickets = selectReadyTickets(state, limit) }) {
  if (state.wave) throw new Error("Finish the existing wave before creating another");
  if (!tickets.length) throw new Error("No ready tickets; inspect unresolved dependencies");
  validateTicketBatch({ tickets }, state);
  if (tickets.some((ticket) => state.completedTickets.includes(ticket.path) || ticket.blockedBy.some((path) => !state.completedTickets.includes(path)))) throw new Error("Wave contains completed or blocked tickets");
  const id = (state.waveSequence ?? 0) + 1;
  state.waveSequence = id;
  state.wave = {
    id, baseCommit, after,
    members: tickets.map((ticket, index) => ({ ticket, branch: `codex/sandcastle-${state.id}-wave-${id}-${index + 1}`, status: "pending" })),
  };
  state.phase = "parallel-work";
  return state.wave;
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
  const completed = wave.members.filter((member) => member.status === "completed");
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
    state.blockedTicket = { path: failures[0].ticket.path, status: "blocked", summary: failures[0].error ?? failures[0].result?.summary ?? "Worker did not complete" };
    await ops.save(state);
    throw new Error(`Wave ${wave.id} incomplete: ${state.blockedTicket.path}: ${state.blockedTicket.summary}`);
  }
  delete state.blockedTicket;
  (state.waveHistory ??= []).push(wave);
  delete state.wave;
  state.phase = wave.after;
  await ops.save(state);
}

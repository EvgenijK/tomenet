import { isAbsolute } from "node:path";

export function validateTicketBatch(parsed, state) {
  if (!Array.isArray(parsed.tickets) || parsed.tickets.length < 1 || parsed.tickets.length > 30) throw new Error("Ticket manifest must contain 1–30 tickets");
  const seen = new Set();
  const completed = new Set(state.completedTickets ?? []);
  for (const ticket of parsed.tickets) {
    if (typeof ticket.path !== "string" || isAbsolute(ticket.path) || ticket.path.includes("..") || !ticket.path.startsWith(`.scratch/sandcastle-${state.id}/issues/`)) throw new Error(`Invalid ticket path: ${ticket.path}`);
    if (seen.has(ticket.path)) throw new Error(`Duplicate ticket path: ${ticket.path}`);
    if (!Array.isArray(ticket.blockedBy) || ticket.blockedBy.some((path) => path === ticket.path || (!seen.has(path) && !completed.has(path)))) throw new Error(`Invalid blockers for ${ticket.path}`);
    seen.add(ticket.path);
  }
  return parsed.tickets;
}

export function recordTicketResult(state, ticket, result) {
  if (!result || !["completed", "blocked"].includes(result.status) || typeof result.summary !== "string" || !result.summary.trim()) throw new Error(`Invalid implementation result for ${ticket.path}`);
  if (result.status === "blocked") {
    state.blockedTicket = { path: ticket.path, ...result };
    return;
  }
  delete state.blockedTicket;
  if (state.completedTickets.includes(ticket.path)) throw new Error(`Ticket already completed: ${ticket.path}`);
  state.completedTickets.push(ticket.path);
  state.ticketIndex++;
}

export function validateTicketCompletion(content, path) {
  if (!/^\*{0,2}Status:\*{0,2}\s*resolved\s*$/mi.test(content) || !/^## Answer\s*$/m.test(content)) throw new Error(`Completed result requires resolved ticket and Answer: ${path}`);
}

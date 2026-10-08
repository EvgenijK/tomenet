import { basename, isAbsolute } from "node:path";

export function focusedTestTicketContents(state, number) {
  const specPath = state?.specPath;
  if (typeof specPath !== "string" || !specPath || isAbsolute(specPath) || specPath.split("/").includes("..")) throw new Error(`Invalid canonical spec path: ${specPath}`);
  if (!Number.isInteger(number) || number < 1) throw new Error(`Invalid focused ticket number: ${number}`);
  const specName = basename(specPath, ".md");
  const canonicalId = specName.match(/^[A-Z]+-[A-Z]+-\d+/)?.[0] ?? specName;
  const stage = specPath.match(/(?:^|\/)stage-([^/]+)(?:\/|$)/i)?.[1]?.toUpperCase();
  const ownership = stage ? `Stage ${stage}` : "canonical task";
  return [
    `# ${number}: Verify focused production coverage`, "", "Type: implementation",
    `Tracker role: Wayfinder planning/execution history for canonical [${canonicalId}](../../../${specPath}); this child does not own ${ownership} capability IDs.`,
    "Status: open", "Assignee: unassigned", "Labels: enhancement, ready-for-agent", "",
    "Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.", "",
    "- [ ] Relevant production paths are exercised and results recorded under Answer.",
    "- [ ] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.", "",
  ].join("\n");
}

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

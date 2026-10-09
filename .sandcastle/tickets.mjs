import { basename, isAbsolute } from "node:path";

const TRIAGE_STATE_LABELS = new Set(["needs-triage", "needs-info", "ready-for-agent", "ready-for-human", "wontfix"]);

function fieldValue(content, field) {
  return content.match(new RegExp(`^\\*{0,2}${field}:\\*{0,2}\\s*(.+?)\\s*$`, "mi"))?.[1];
}

function hasCanonicalTrackerRole(content) {
  const role = fieldValue(content, "Tracker role");
  const match = role?.match(/^Wayfinder planning\/execution history for canonical \[([^\]]+)\]\(\.\.\/\.\.\/\.\.\/([^\)]+)\); this child does not own (.+) capability IDs\.$/);
  if (!match) return false;

  const [, recordedId, specPath, recordedOwnership] = match;
  if (isAbsolute(specPath) || specPath.split("/").includes("..") || !specPath.endsWith(".md")) return false;
  const specName = basename(specPath, ".md");
  const canonicalId = specName.match(/^[A-Z]+-[A-Z]+-\d+/)?.[0] ?? specName;
  const stage = specPath.match(/(?:^|\/)stage-([^/]+)(?:\/|$)/i)?.[1]?.toUpperCase();
  const ownership = stage ? `Stage ${stage}` : "canonical task";
  return recordedId === canonicalId && recordedOwnership === ownership;
}

function hasSingleTriageStateLabel(content) {
  const labels = fieldValue(content, "Labels")?.split(",").map((label) => label.trim()).filter(Boolean) ?? [];
  return labels.filter((label) => TRIAGE_STATE_LABELS.has(label)).length === 1;
}

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
  const metadata = content.split(/^## Answer\s*$/m, 1)[0];
  const isFocusedCoverage = /(?:^|\/)\d+-focused-test-coverage\.md$/.test(path);
  if (fieldValue(metadata, "Type")?.toLowerCase() !== "implementation" && !isFocusedCoverage) return;
  if (!hasCanonicalTrackerRole(metadata)) throw new Error(`Completed implementation ticket requires canonical Tracker role: ${path}`);
  if (!hasSingleTriageStateLabel(metadata)) throw new Error(`Completed implementation ticket requires exactly one standard triage-state label: ${path}`);
}

import { createHash } from "node:crypto";
import { taskQuotaLimit } from "./limits.mjs";

// Model opinions never override these controller-owned safety rules.
export const acceptancePolicy = Object.freeze({ maxContractRounds: 2, maxDefectRepairs: 3, maxFinalAudits: 2 });
function contractRoundLimit(state) {
  need(state.contractRoundLimit === undefined || [3, 4].includes(state.contractRoundLimit), "invalid contract round grant");
  return state.contractRoundLimit ?? acceptancePolicy.maxContractRounds;
}

// One user-approved revision per stopped checkpoint, with an absolute fourth-
// round ceiling. Earlier verifier feedback and all non-contract limits survive.
// No quota, repair or review counter changes; the previous feedback is retained.
export function grantContractRound(state) {
  const pending = state.contractPending;
  const previousLimit = contractRoundLimit(state);
  need(state.phase === "awaiting" && state.resumePhase === "contract" &&
    state.pauseReason === "Acceptance: contract verification limit reached; human decision required" &&
    !state.acceptance && previousLimit < 4 &&
    pending?.round === previousLimit && Array.isArray(pending.feedback) && pending.feedback.length > 0 &&
    !pending.proposal && !pending.review && !(pending.parts?.length), "additional round requires the rejected contract at the current limit and explicit authorization");
  need(!state.quotaError && (state.budget?.consumedPercent ?? 0) < taskQuotaLimit(state), "quota still requires a separate allowance");
  const grant = { previousLimit, newLimit: previousLimit + 1, grantedAt: new Date().toISOString() };
  state.contractRoundGrants ??= state.contractRoundGrant ? [state.contractRoundGrant] : [];
  state.contractRoundGrants.push(grant);
  state.contractRoundLimit = grant.newLimit;
  state.contractRoundGrant = grant;
  state.phase = "contract";
  delete state.resumePhase;
  delete state.pauseReason;
  return state;
}
export const baselineChecks = [
  { id: "sv-build", kind: "build", command: "make -s -C src -f makefile.sv tomenet-sv", description: "SV Make build" },
  { id: "sv-core", kind: "core", command: "bash -s -- core", description: "Sandcastle core checks" },
];
export const digest = (value) => createHash("sha256").update(typeof value === "string" ? value : JSON.stringify(value)).digest("hex");
const text = (value) => typeof value === "string" && value.trim().length > 0;
const identifier = (value) => typeof value === "string" && /^[a-zA-Z0-9][a-zA-Z0-9_.-]{0,79}$/.test(value);
const path = (value) => typeof value === "string" && /^[a-zA-Z0-9_./-]+$/.test(value) && !value.startsWith("/") && !value.split("/").includes("..");
const normalize = (value) => value.normalize("NFKC").toLowerCase().replace(/\s+/g, " ").trim();
const need = (condition, message) => { if (!condition) throw new Error(`Acceptance: ${message}`); };

export function validateContract(contract, sources) {
  need(contract?.version === 1 && ["implementation", "full_acceptance"].includes(contract.completionScope), "invalid contract scope/version");
  need(Array.isArray(contract.criteria) && contract.criteria.length > 0 && contract.criteria.length <= 100, "numbered criteria required");
  need(Array.isArray(contract.checks) && contract.checks.length > 0 && contract.checks.length <= 100, "fixed checks required");
  const checks = new Map();
  for (const check of contract.checks) {
    need(identifier(check.id) && !checks.has(check.id) && text(check.description), "invalid/duplicate check");
    need(["build", "core", "command", "external"].includes(check.kind), "unknown check kind");
    if (["build", "core"].includes(check.kind)) {
      const baseline = baselineChecks.find((item) => item.id === check.id);
      need(baseline && baseline.kind === check.kind && baseline.command === check.command, "baseline command cannot be weakened");
    } else if (check.kind === "command") {
      // Focused repository checks, never arbitrary shell programs or remote actions.
      need(typeof check.command === "string" && /^(?:python3 -B|node|bash) tests\/[a-zA-Z0-9_./-]+(?: [a-zA-Z0-9_=./:-]+)*$/.test(check.command) && !check.command.includes(".."), "focused check must invoke a repository test without shell operators");
    } else need(check.command === "", "external checks have no automatic runner");
    checks.set(check.id, check);
  }
  for (const baseline of baselineChecks) {
    const check = checks.get(baseline.id);
    need(check?.kind === baseline.kind && check.command === baseline.command, `missing or weakened mandatory ${baseline.id}`);
  }
  const ids = new Set();
  for (const criterion of contract.criteria) {
    need(identifier(criterion.id) && !ids.has(criterion.id) && text(criterion.requirement), "invalid/duplicate criterion");
    ids.add(criterion.id);
    need(typeof criterion.mandatory === "boolean" && ["current", "deferred"].includes(criterion.applicability), "criterion needs explicit applicability");
    need(text(criterion.source?.quote) && typeof sources[criterion.source.path] === "string" && sources[criterion.source.path].includes(criterion.source.quote), `unverifiable source for ${criterion.id}`);
    need(Array.isArray(criterion.checkIds) && criterion.checkIds.length > 0 && new Set(criterion.checkIds).size === criterion.checkIds.length && criterion.checkIds.every((id) => checks.has(id)), `invalid check mapping for ${criterion.id}`);
    need(typeof criterion.owner === "string" && typeof criterion.deferralReason === "string", "owner/reason fields required");
    if (criterion.applicability === "deferred") {
      need(contract.completionScope === "implementation" && text(criterion.owner) && text(criterion.deferralReason), "full acceptance cannot defer required criteria");
    } else need(criterion.deferralReason === "", "current criteria cannot carry a deferral");
  }
  need(contract.criteria.some((item) => item.mandatory && item.applicability === "current"), "at least one current mandatory criterion required");
  return contract;
}

export function requiredChecks(contract) {
  const ids = new Set(baselineChecks.map((item) => item.id));
  for (const criterion of contract.criteria) if (criterion.mandatory && criterion.applicability === "current") for (const id of criterion.checkIds) ids.add(id);
  return contract.checks.filter((check) => ids.has(check.id));
}

export function acceptContract(state, contract, sources, review, head) {
  validateContract(contract, sources);
  need(review?.approved === true && text(review.summary) && Array.isArray(review.missingRequirements) && review.missingRequirements.length === 0, "independent contract verification did not pass");
  state.acceptance = {
    contract, contractDigest: digest(contract), sourceDigests: Object.fromEntries(Object.entries(sources).map(([key, value]) => [key, digest(value)])),
    acceptedHead: head, contractReview: review, policy: { ...acceptancePolicy }, ledger: [], repairBatches: [], evidence: [], assessments: [], finalAudits: [],
    ...(state.contractMigration ? { migration: state.contractMigration } : {}),
  };
}

export function assertContract(state) {
  need(state.acceptance && digest(state.acceptance.contract) === state.acceptance.contractDigest, "accepted contract changed; stop for a new explicit scope decision");
}

export function prepareAcceptanceMigration(state) {
  if (state.acceptance || ["awaiting", "done", "contract", "recovery"].includes(state.phase)) return false;
  state.contractReturnPhase = ["report", "integrate"].includes(state.phase) || (state.phase === "plan" && state.reviewFindings) ? "build" : state.phase;
  state.contractMigration = "This run began before acceptance contracts. The contract is validated before further implementation, not retroactively before existing commits. Old reviews remain evidence for identity reconciliation; repair counts start with mapped, integrated batches after migration. All final checks and a fresh audit are required.";
  delete state.reviewFindings;
  state.phase = "contract";
  return true;
}

export function recordCheck(state, { checkId, head, passed, logPath }) {
  assertContract(state);
  need(state.acceptance.contract.checks.some((check) => check.id === checkId) && text(head) && typeof passed === "boolean" && text(logPath), "invalid controller check evidence");
  state.acceptance.evidence.push({ checkId, head, passed, logPath, contractDigest: state.acceptance.contractDigest });
}

function eligibleResidual(finding, contract) {
  return finding.severity === "low" && ["cosmetic", "maintainability"].includes(finding.category) && finding.criterionIds.every((id) => {
    const criterion = contract.criteria.find((item) => item.id === id);
    return !criterion.mandatory && criterion.applicability === "current";
  });
}

function validateAssessment(assessment, contract) {
  need(text(assessment?.summary) && Array.isArray(assessment.criteria) && Array.isArray(assessment.preserved) && assessment.preserved.every(text), "malformed review assessment");
  const criteria = new Map(contract.criteria.map((item) => [item.id, item]));
  const seen = new Set();
  for (const item of assessment.criteria) {
    need(criteria.has(item.id) && !seen.has(item.id) && ["passed", "failed", "pending"].includes(item.status) && text(item.evidence), "review needs one evidenced status per criterion");
    need(criteria.get(item.id).applicability !== "deferred" || item.status === "pending", "deferred acceptance cannot be claimed passed");
    seen.add(item.id);
  }
  need(seen.size === criteria.size, "review omitted contract criteria");
  need(Array.isArray(assessment.findings) && Array.isArray(assessment.resolved) && Array.isArray(assessment.acceptedResiduals), "review needs findings, resolutions and residual decisions");
  for (const finding of assessment.findings) {
    need(typeof finding.id === "string" && identifier(finding.defectKey) && text(finding.area), "finding needs a stable defect key and production area");
    need(["critical", "high", "medium", "low"].includes(finding.severity) && ["behavior", "regression", "security", "data-loss", "protocol", "scope", "evidence", "cosmetic", "maintainability"].includes(finding.category), "invalid defect classification");
    need(Array.isArray(finding.criterionIds) && finding.criterionIds.length > 0 && new Set(finding.criterionIds).size === finding.criterionIds.length && finding.criterionIds.every((id) => criteria.has(id)), "finding must reference contract criteria");
    need(path(finding.file) && Number.isInteger(finding.line) && finding.line >= 1 && text(finding.problem) && text(finding.fix) && text(finding.evidence), "finding needs a production location and evidence");
  }
  for (const resolution of assessment.resolved) need(identifier(resolution.id) && text(resolution.evidence), "resolution requires evidence");
  for (const residual of assessment.acceptedResiduals) need(identifier(residual.id) && ["reason", "risk", "owner", "returnCondition"].every((field) => text(residual[field])), "residual requires reason, risk, owner and return condition");
}

// Identity excludes file and line. Reviewers reconcile paraphrases with saved IDs;
// exact key/area matches are also folded within a round, across axes and restarts.
export function applyAssessment(state, assessment, { head, audit = false } = {}) {
  assertContract(state);
  const original = state.acceptance;
  validateAssessment(assessment, original.contract);
  const next = structuredClone(original);
  const touched = new Set();
  const ranks = { low: 0, medium: 1, high: 2, critical: 3 };
  for (const finding of assessment.findings) {
    const identity = `${normalize(finding.area)}:${normalize(finding.defectKey)}`;
    let entry = finding.id ? next.ledger.find((item) => item.id === finding.id) : next.ledger.find((item) => item.identity === identity);
    if (finding.id) need(entry, `unknown finding ID ${finding.id}`);
    if (entry) {
      need(entry.identity === identity, `identity changed for ${entry.id}`);
      // A previously waived/resolved issue may reopen only with new concrete evidence.
      if (["resolved", "residual"].includes(entry.status) && entry.evidence === finding.evidence && entry.category === finding.category && ranks[finding.severity] <= ranks[entry.severity] && finding.criterionIds.every((id) => entry.criterionIds.includes(id)) && !audit) { touched.add(entry.id); continue; }
      const previous = { ...entry };
      Object.assign(entry, finding, { id: previous.id, repairs: previous.repairs, identity, status: "open" });
      entry.criterionIds = [...new Set([...previous.criterionIds, ...finding.criterionIds])];
      if (ranks[previous.severity] > ranks[entry.severity]) entry.severity = previous.severity;
      if (!eligibleResidual(previous, next.contract)) entry.category = previous.category;
      delete entry.residual;
    } else {
      entry = { ...finding, id: `D-${digest(identity).slice(0, 16)}`, identity, repairs: 0, status: "open", firstHead: head };
      next.ledger.push(entry);
    }
    entry.lastHead = head;
    touched.add(entry.id);
  }
  const resolvedIds = new Set();
  for (const resolution of assessment.resolved) {
    const entry = next.ledger.find((item) => item.id === resolution.id);
    need(entry && !touched.has(entry.id) && !resolvedIds.has(entry.id), "unknown, duplicate or simultaneously open resolution");
    entry.status = "resolved"; entry.resolution = { head, evidence: resolution.evidence }; delete entry.residual;
    resolvedIds.add(entry.id);
  }
  const residualIds = new Set();
  for (const residual of assessment.acceptedResiduals) {
    const candidates = next.ledger.filter((item) => item.id === residual.id || (touched.has(item.id) && item.defectKey === residual.id));
    const entry = candidates.length === 1 ? candidates[0] : undefined;
    need(entry && !resolvedIds.has(entry.id) && !residualIds.has(entry.id) && eligibleResidual(entry, next.contract), "only optional low cosmetic/maintainability defects may remain");
    entry.status = "residual";
    entry.residual = { ...residual, id: entry.id, head, backlogPath: `.scratch/sandcastle-${state.id}/residuals/${entry.id}.md` };
    residualIds.add(entry.id);
  }
  // Silence is not a fix. Every old open issue stays open until evidenced resolution.
  if (audit) for (const entry of next.ledger) if (entry.status === "residual" && !residualIds.has(entry.id)) { entry.status = "open"; delete entry.residual; }
  const record = { ...assessment, head, contractDigest: next.contractDigest };
  next.assessments.push(record);
  if (audit) next.finalAudits.push(record);
  state.acceptance = next;
  return closureGate(state, head, { final: audit });
}

export function closureGate(state, head, { final = true } = {}) {
  assertContract(state);
  const a = state.acceptance;
  const assessment = (final ? a.finalAudits : a.assessments).at(-1);
  const reasons = [];
  if (!assessment || assessment.head !== head || assessment.contractDigest !== a.contractDigest) reasons.push("Fresh assessment of the final HEAD is missing");
  for (const check of requiredChecks(a.contract)) {
    const evidence = a.evidence.filter((item) => item.checkId === check.id && item.head === head && item.contractDigest === a.contractDigest).at(-1);
    if (!evidence?.passed) reasons.push(`Required check ${check.id} did not pass at final HEAD`);
  }
  for (const criterion of a.contract.criteria) if (criterion.mandatory && criterion.applicability === "current" && assessment?.criteria.find((item) => item.id === criterion.id)?.status !== "passed") reasons.push(`Mandatory criterion ${criterion.id} is not passed`);
  for (const result of assessment?.criteria ?? []) if (result.status === "failed" && !a.ledger.some((item) => item.status !== "resolved" && item.criterionIds.includes(result.id))) reasons.push(`Failed criterion ${result.id} has no registered defect`);
  const open = a.ledger.filter((item) => item.status === "open");
  for (const entry of open) reasons.push(`Open defect ${entry.id}`);
  for (const entry of a.ledger.filter((item) => item.status === "residual")) if (!eligibleResidual(entry, a.contract) || (final && entry.residual.head !== head)) reasons.push(`Unaccepted residual ${entry.id}`);
  if (state.wave || (state.tickets ?? []).some((ticket) => !(state.completedTickets ?? []).includes(ticket.path))) reasons.push("Worker commits/tickets still await integration");
  const ready = reasons.length === 0;
  return { verdict: ready ? a.ledger.some((item) => item.status === "residual") ? "ready_with_notes" : "ready" : "blocked", head, reasons, scope: a.contract.completionScope };
}

export function repairFindings(state) {
  assertContract(state);
  const open = state.acceptance.ledger.filter((item) => item.status === "open");
  const exhausted = open.filter((item) => item.repairs >= acceptancePolicy.maxDefectRepairs);
  need(exhausted.length === 0, `defect repair limit reached: ${exhausted.map((item) => item.id).join(", ")}; human decision required, blockers remain open`);
  return open;
}

export function assertAuditedChanges(state, changedPaths) {
  const outcome = closureGate(state, state.acceptance?.finalVerdict?.head);
  need(outcome.verdict !== "blocked", "integration/report gate blocked; human decision required");
  const allowed = new Set([
    `docs/tasks/sandcastle/${state.id}-report.md`,
    ...(state.recoveries ?? []).map((item) => item.recordPath),
    ...state.acceptance.ledger.filter((item) => item.status === "residual").map((item) => item.residual.backlogPath),
  ]);
  need(changedPaths.every((item) => allowed.has(item)), "code changed after final audit; human decision required");
  return outcome;
}

export function registerRepairBatch(state, tickets, key) {
  if (!state.reviewFindings?.length) return;
  const open = repairFindings(state);
  const wanted = new Set(open.map((item) => item.id));
  const mapped = new Set();
  for (const ticket of tickets) {
    need(!state.completedTickets.includes(ticket.path), "a new repair cannot reuse an already completed ticket");
    need(Array.isArray(ticket.findingIds) && ticket.findingIds.length > 0 && new Set(ticket.findingIds).size === ticket.findingIds.length && ticket.findingIds.every((id) => wanted.has(id)), "repair ticket must name open ledger IDs");
    for (const id of ticket.findingIds) mapped.add(id);
  }
  need(mapped.size === wanted.size, "repair batch omitted open defects");
  if (!state.acceptance.repairBatches.some((item) => item.key === key)) state.acceptance.repairBatches.push({ key, tickets: structuredClone(tickets), counted: [] });
}

export function countIntegratedRepairs(state) {
  if (!state.acceptance) return;
  for (const batch of state.acceptance.repairBatches) {
    const ids = new Set(batch.tickets.flatMap((ticket) => ticket.findingIds));
    for (const id of ids) {
      if (batch.counted.includes(id) || !batch.tickets.filter((ticket) => ticket.findingIds.includes(id)).every((ticket) => state.completedTickets.includes(ticket.path))) continue;
      const entry = state.acceptance.ledger.find((item) => item.id === id);
      need(entry.repairs < acceptancePolicy.maxDefectRepairs, `defect repair limit reached for ${id}`);
      entry.repairs++;
      batch.counted.push(id);
    }
  }
}

export function acceptanceReport(state) {
  assertContract(state);
  const a = state.acceptance;
  const g = a.finalVerdict;
  return [
    "## Acceptance", "", `Verdict: ${g?.verdict ?? "blocked"}; scope: ${a.contract.completionScope}`, `Contract SHA256: ${a.contractDigest}`, `Contract: .scratch/sandcastle-${state.id}/acceptance-contract.json`, `Audited code HEAD: ${g?.head ?? "not audited"}`, `Fresh final audits: ${a.finalAudits.length}/${acceptancePolicy.maxFinalAudits}`, "",
    `Coverage: ${a.finalAudits.at(-1)?.criteria.filter((item) => item.status === "passed").length ?? 0} passed, ${a.finalAudits.at(-1)?.criteria.filter((item) => item.status === "failed").length ?? 0} failed, ${a.finalAudits.at(-1)?.criteria.filter((item) => item.status === "pending").length ?? 0} pending.`, "",
    "## Criteria and coverage", "", ...a.contract.criteria.map((criterion) => {
      const result = a.finalAudits.at(-1)?.criteria.find((item) => item.id === criterion.id);
      return `- ${criterion.id}: ${result?.status ?? "pending"} — ${criterion.requirement}. Source: ${criterion.source.path}. Evidence: ${result?.evidence ?? "not checked"}${criterion.applicability === "deferred" ? `; pending at ${criterion.owner}: ${criterion.deferralReason}` : ""}`;
    }), "", "## Controller checks", "", ...a.contract.checks.map((check) => {
      const evidence = a.evidence.filter((item) => item.head === g?.head && item.checkId === check.id).at(-1);
      return `- ${check.id}: ${evidence ? evidence.passed ? "passed" : "failed" : "not run"}; ${evidence?.logPath ?? check.description}`;
    }), "",
    "## Residual backlog (open, not fixed)", "", ...a.ledger.filter((item) => item.status === "residual").map((item) => `- ${item.id}: ${item.problem}. Risk: ${item.residual.risk}. Accepted because: ${item.residual.reason}. Owner: ${item.residual.owner}. Return when: ${item.residual.returnCondition}. ${item.residual.backlogPath}`), "",
    "## Defect history", "", ...a.ledger.map((item) => `- ${item.id}: ${item.status}; integrated repair cycles: ${item.repairs}/${acceptancePolicy.maxDefectRepairs}; ${item.problem}`), "",
    "## Preserved behavior", "", ...(a.finalAudits.at(-1)?.preserved ?? []).map((item) => `- ${item}`), "",
    ...(a.migration ? ["## Legacy run migration", "", a.migration, ""] : []),
  ].join("\n");
}

// Checkpoints contain the raw agent response before validation/publication. A
// restart replays it instead of paying for another opinion or repair cycle.
export async function contractStage(state, ops) {
  const pending = state.contractPending ??= { round: 0 };
  const sources = await ops.sources();
  if (!pending.proposal) {
    // Legacy interrupted proposals reserved a round before calling the model.
    // An incomplete response is resumed in that round, never counted as a
    // rejected contract. Validation/verification rejections carry feedback.
    if (pending.proposalInFlight === undefined && pending.round > 0 && !pending.feedback && !pending.review) pending.proposalInFlight = true;
    if (!pending.proposalInFlight) {
      need(pending.round < contractRoundLimit(state), "contract verification limit reached; human decision required");
      pending.round++;
      pending.proposalInFlight = true;
    }
    await ops.save(state);
    pending.proposal = await ops.propose(sources, pending.feedback ?? [], pending);
    delete pending.proposalInFlight;
    await ops.save(state);
  }
  try { validateContract(pending.proposal, sources); }
  catch (error) {
    pending.feedback = [error.message]; delete pending.proposal; delete pending.parts; delete pending.partSourceDigest;
    delete pending.reconciliation; delete pending.reconciliationDraftDigest; delete pending.reconciliationSourceDigest;
    await ops.save(state);
    need(pending.round < contractRoundLimit(state), "contract validation limit reached; human decision required");
    return;
  }
  if (!pending.review) {
    pending.review = await ops.verify(pending.proposal, sources);
    await ops.save(state);
  }
  if (pending.review.approved !== true || pending.review.missingRequirements?.length) {
    pending.feedback = pending.review.missingRequirements?.length ? pending.review.missingRequirements : [pending.review.summary];
    delete pending.proposal; delete pending.review; delete pending.parts; delete pending.partSourceDigest;
    delete pending.reconciliation; delete pending.reconciliationDraftDigest; delete pending.reconciliationSourceDigest;
    await ops.save(state);
    need(pending.round < contractRoundLimit(state), "contract verification limit reached; human decision required");
    return;
  }
  const candidate = structuredClone(state);
  acceptContract(candidate, pending.proposal, sources, pending.review, await ops.head());
  await ops.publish(candidate.acceptance);
  state.acceptance = candidate.acceptance;
  state.phase = state.contractReturnPhase ?? "plan";
  delete state.contractReturnPhase;
  delete state.contractPending;
  await ops.save(state);
}

export async function assessmentStage(state, ops, { audit = false } = {}) {
  assertContract(state);
  const head = await ops.head();
  const a = state.acceptance;
  if (!state.assessmentPending) {
    if (audit) need(a.finalAudits.length < acceptancePolicy.maxFinalAudits, "final audit limit reached; human decision required");
    else need(state.reviewRound < state.reviewLimit, "review round limit reached; human approval required");
    state.assessmentPending = { head, audit };
    if (!audit) state.reviewRound++;
    await ops.save(state);
  }
  const pending = state.assessmentPending;
  need(pending.head === head && pending.audit === audit, "assessment checkpoint HEAD changed");
  if (!pending.response) {
    pending.response = await ops.inspect({ head, audit });
    await ops.save(state);
  }
  // Applying a saved response must be atomic and idempotent if publication fails.
  if (!pending.applied) {
    const outcome = applyAssessment(state, pending.response, { head, audit });
    pending.applied = true; pending.outcome = outcome;
    await ops.save(state);
  }
  await ops.publish(pending.response, { head, audit });
  const open = state.acceptance.ledger.filter((item) => item.status === "open");
  const outcome = pending.outcome;
  state.acceptance.lastVerdict = outcome;
  if (outcome.verdict === "blocked" && open.length === 0) {
    state.acceptance.blockedReason = outcome.reasons.join("; ");
    await ops.save(state);
    throw new Error(`Acceptance: ${state.acceptance.blockedReason}; human decision required`);
  }
  if (open.length) {
    if (audit) need(state.acceptance.finalAudits.length < acceptancePolicy.maxFinalAudits, "final audit limit reached; blockers remain open; human decision required");
    const findings = repairFindings(state);
    if (!audit) need(state.reviewRound < state.reviewLimit, "review round limit reached; human approval required");
    state.reviewFindings = findings;
    state.phase = "plan";
  } else {
    delete state.reviewFindings;
    state.phase = audit ? "report" : "final-audit";
    if (audit) state.acceptance.finalVerdict = outcome;
  }
  delete state.assessmentPending;
  await ops.save(state);
}

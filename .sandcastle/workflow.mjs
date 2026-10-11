import { randomUUID } from "node:crypto";
import { execFile } from "node:child_process";
import { access, mkdir, open, readFile, readdir, rename, unlink, writeFile } from "node:fs/promises";
import { homedir } from "node:os";
import { resolve, isAbsolute } from "node:path";
import { promisify } from "node:util";
import { createSandbox } from "@ai-hero/sandcastle";
import { docker } from "@ai-hero/sandcastle/sandboxes/docker";
import { readWeeklyUsage, updateBudget, resumeUserPause } from "./limits.mjs";
import { focusedTestTicketContents, validateTicketBatch, validateTicketCompletion } from "./tickets.mjs";
import { createWave, resumeWorkerProvisioning, runWave, collectWave } from "./parallel.mjs";
import { appendStageEvent, stageOutcome } from "./events.mjs";
import { beginRecovery, recoveryRestriction, runRecovery, documentRecovery, recoveryRecord, resumeImprovedContract, resumeContractTimeoutUpgrade, resumeContractCitationAttempts, resumeContractGroupAttempts, resumeContractAssemblyAttempts, extendContractAssemblyAttempts, recoveryCategory } from "./recovery.mjs";
import { baselineChecks, controllerReviewCheck, assertContract, prepareAcceptanceMigration, contractStage, assessmentStage, requiredChecks, recordCheck, registerRepairBatch, countIntegratedRepairs, assertAuditedChanges, acceptanceReport } from "./acceptance.mjs";
import { buildContractDraft } from "./contract-draft.mjs";
import { contractSourceCatalog, feedbackForGroup } from "./contract-feedback.mjs";
import { planContractGroups, buildGroupedContractDraft, withContractGroupRecoveryAdvice } from "./contract-groups.mjs";
import { assemblyCommandRules, reconcileContractDraft } from "./contract-reconcile.mjs";
import { groupContractPromptGuidance, reconciliationContractPromptGuidance, verifierContractPromptGuidance } from "./contract-prompts.mjs";
import { resolveContractSources } from "./contract-sources.mjs";
import { runtimeSchemaPath, runtimeSchemaMounts, readOnlyAgentArgs, sandboxAgentArgs, sandcastleAgentSettings, delegatedAgentPolicy, readOnlyAgentTimeoutMs, recoveryAgentTimeoutMs } from "./runtime.mjs";
import { recordBuildAttempt, refreshBuildBudgetsAfterBatch } from "./build-budget.mjs";
import { guidanceFor, guidanceForAgent } from "./recovery-guidance.mjs";
import { contractAmendmentVerificationContext, resumeExternalContractAmendment, resumeSemanticContractAmendment, resumeContractAmendmentRetry, validateContractAmendment } from "./contract-amend.mjs";
import { contractExecutionContext, validateContractRunnerInventory } from "./contract-runners.mjs";
import { parseAgentOutput, agentFailureDetail } from "./agent-output.mjs";
import { resumeWorkerRuntimeUpgrade } from "./worker-runtime.mjs";
import { rejectLegacyContinue, readAttemptGrant, applyAllowanceGrant } from "./continuation.mjs";
import { classifyFailure, pauseStatus, clearCurrentFailure } from "./failure.mjs";
import { controllerExecutionIdentity, preflightExecution, preflightWorker } from "./execution.mjs";
import { dispatchVerified, resumeExecutionCheckpoint, closeExecution } from "./dispatch.mjs";
import { repositoryGateInput } from "./gate-input.mjs";
import { sandcastleRunAgentPolicy } from "./runtime.mjs";
import { digest, verifyEvidence, verificationError } from "./verification.mjs";
import { validateExecutionPlan, acceptExecutionPlan, assertExecutionPlan, requiredArtifactChecks } from "./verification-plan.mjs";
import { assertIntegrationBase, freezeVerificationTarget, assertVerificationTarget, releaseVerificationTarget, executeArtifactCheck, verifyRequiredArtifacts, toolchainIdentity } from "./verification-gates.mjs";
import { isDeliveryReady, dependencyReadyTickets, assemblyContributions } from "./delivery.mjs";
import { resumeRepairPlanCheckpoint } from './repair-plan.mjs';
import { resumeVerificationMigration } from "./verification-migration.mjs";
import { startLinuxHostChannel } from './linux-host.mjs';
import { validateHostApprovalResume, applyHostApprovalResume } from './host-approval-resume.mjs';
import { workerHostDelivery, workerHostFailure } from './worker-host.mjs';

const execFileAsync = promisify(execFile);
const root = resolve(import.meta.dirname, "..");
const authDir = resolve(root, ".sandcastle/auth");
const runsDir = resolve(root, ".sandcastle/runs");
const command = process.argv[2];
const runId = process.argv[3];
const targetBranch = "modern_interface";
const { model } = sandcastleAgentSettings("orchestrator");
const skillsRoot = resolve(process.env.SANDCASTLE_SKILLS_ROOT || resolve(homedir(), ".agents/skills"));
const skillPaths = Object.fromEntries(["to-tickets", "code-review", "tdd", "resolving-merge-conflicts"].map((name) => [name, resolve(skillsRoot, name)]));
const quote = (value) => `'${String(value).replaceAll("'", "'\\''")}'`;
let saveQueue = Promise.resolve();
let quotaQueue = Promise.resolve();
let provisionQueue = Promise.resolve();
let loadedControllerDigest;

async function git(cwd, ...args) {
  return (await execFileAsync("git", args, { cwd, maxBuffer: 4 * 1024 * 1024 })).stdout.trim();
}
async function workerBranchNames(state) {
  const output = await git(root, "for-each-ref", "--format=%(refname:short)", `refs/heads/codex/sandcastle-${state.id}-wave-*`);
  return output.split("\n").filter(Boolean);
}
async function save(state) {
  state.updatedAt = new Date().toISOString();
  const contents = JSON.stringify(state, null, 2) + "\n";
  const next = saveQueue.then(async () => {
    const dir = resolve(runsDir, state.id);
    await mkdir(dir, { recursive: true });
    await writeFile(resolve(dir, "state.json.tmp"), contents);
    await rename(resolve(dir, "state.json.tmp"), resolve(dir, "state.json"));
  });
  saveQueue = next.catch(() => {});
  await next;
}
async function quotaCheckpoint(state, timeoutMs = 20000) {
  const next = quotaQueue.then(async () => {
    if (state.quotaError) throw new Error(state.quotaError);
    updateBudget(state, await readWeeklyUsage(authDir, model, timeoutMs));
    await save(state);
  });
  quotaQueue = next.catch(() => {});
  await next;
}
function taskDir(state) { return `.scratch/sandcastle-${state.id}`; }
function manifestPath(state) { return `${taskDir(state)}/batch-${state.reviewRound}.json`; }
async function acquireLock(state) {
  const path = resolve(runsDir, state.id, "run.lock");
  for (let attempt = 0; attempt < 2; attempt++) {
    try {
      const handle = await open(path, "wx");
      await handle.writeFile(`${process.pid}\n`);
      await handle.close();
      return path;
    } catch (error) {
      if (error.code !== "EEXIST") throw error;
      const pid = Number((await readFile(path, "utf8")).trim());
      try { process.kill(pid, 0); throw new Error(`Sandcastle run ${state.id} is already active as process ${pid}`); }
      catch (check) { if (check.code !== "ESRCH") throw check; }
      await unlink(path);
    }
  }
  throw new Error(`Could not acquire lock for Sandcastle run ${state.id}`);
}
function skillMounts() {
  return Object.entries(skillPaths).map(([name, path]) => ({ hostPath: path, sandboxPath: `/home/agent/.agents/skills/${name}`, readonly: true }));
}
async function ensureCommit(worktree, message) {
  if (await git(worktree, "ls-files", "-u")) throw new Error(`Unresolved merge preserved at ${worktree}; assembly must resolve it before committing`);
  if (!(await git(worktree, "status", "--porcelain"))) return;
  await git(worktree, "add", "-A");
  await git(worktree, "commit", "-m", message);
}
async function recoverWorktree(state) {
  const listing = await git(root, "worktree", "list", "--porcelain");
  const entry = listing.split("\n\n").find((block) => block.includes(`branch refs/heads/${state.branch}`));
  if (!entry) return;
  const path = entry.match(/^worktree (.+)$/m)?.[1];
  if (!path || !path.startsWith(resolve(root, ".sandcastle/worktrees") + "/")) {
    throw new Error(`Branch ${state.branch} is checked out outside Sandcastle; resolve that worktree before resuming`);
  }
  // Sandcastle reuses managed dirty worktrees. Preserve an interrupted merge
  // for the assembly agent instead of committing conflict markers or resetting.
  if (await git(path, "ls-files", "-u")) return;
  await ensureCommit(path, `Sandcastle: preserve interrupted task ${state.id}`);
  await git(root, "worktree", "remove", path);
}
async function recoveryWorktree(state) {
  const listing = await git(root, "worktree", "list", "--porcelain");
  const entry = listing.split("\n\n").find((block) => block.includes(`branch refs/heads/${state.branch}`));
  if (entry) {
    const path = entry.match(/^worktree (.+)$/m)?.[1];
    if (!path?.startsWith(resolve(root, ".sandcastle/worktrees") + "/")) throw new Error("Recovery can modify only the managed integration worktree");
    return path;
  }
  const path = resolve(root, ".sandcastle/worktrees", state.branch.replaceAll("/", "-"));
  await git(root, "worktree", "add", path, state.branch);
  return path;
}
async function recordExecution(state, role, manifest) {
  const id = randomUUID();
  const path = resolve(runsDir, state.id, `execution-${id}.json`);
  const record = { id, role, at: new Date().toISOString(), ...manifest };
  await writeFile(path, JSON.stringify(record, null, 2) + "\n");
  (state.executionHistory ??= []).push({ id, role, at: record.at, digest: manifest.digest, path });
  await save(state);
}
function executionContext(state, role, worktree, extra = {}) {
  return { controllerRoot: root, expectedControllerDigest: loadedControllerDigest,
    worktree, expectedBranch: state.branch, baseCommit: state.baseCommit,
    contractDigest: state.acceptance?.contractDigest,
    ...sandcastleAgentSettings(role, process.env, state.agentPolicy), ...extra };
}
async function agent(sandbox, state, role, prompt, schema = false, readOnly = false, timeoutMs = readOnlyAgentTimeoutMs, workerContext = {}) {
  // Docker cannot provide the nested namespaces needed by Bubblewrap. Keep
  // inspectors read-only on the host, against this exact integration worktree.
  if (readOnly) return hostAgent(state, {
    cwd: sandbox.worktreePath, role, prompt,
    schemaPath: resolve(root, ".sandcastle", schema === true ? "review-output.schema.json" : schema.replace(/^\.sandcastle\//, "")),
    multiAgent: role.startsWith("review "), timeoutMs,
  });
  const settings = sandboxAgentArgs({ ...sandcastleAgentSettings(role, process.env, state.agentPolicy),
    schemaPath: schema ? runtimeSchemaPath(schema) : undefined }).map(quote).join(" ");
  console.log(`Agent: ${role}`);
  const captured = { message: "", tokens: 0 };
  const schemaPath = schema ? resolve(root, ".sandcastle", schema === true ? "review-output.schema.json" : schema.replace(/^\.sandcastle\//, "")) : undefined;
  const result = await dispatchVerified(executionContext(state, role, sandbox.worktreePath, {
    sandbox, schemaPath, allowDirty: role.startsWith("assembly "), ...workerContext,
  }), {
    preflight: execution => preflightExecution(execution, { sandboxSchemaPath: schema ? runtimeSchemaPath(schema) : undefined }),
    record: manifest => recordExecution(state, role, manifest), quota: () => quotaCheckpoint(state),
    invoke: () => sandbox.exec(`codex ${settings}`, {
      stdin: [prompt, delegatedAgentPolicy, guidanceForAgent(state, role)].filter(Boolean).join("\n\n"),
      onLine: (line) => parseAgentOutput(line, captured),
    }),
  });
  state.budget.tokens += captured.tokens;
  await save(state);
  if (result.exitCode !== 0) {
    throw new Error(`${role} agent exited with status ${result.exitCode}: ${agentFailureDetail(captured, result.stderr)}`);
  }
  try { await quotaCheckpoint(state); }
  catch (error) { state.quotaError = error.message; }
  await save(state);
  return captured.message;
}
export async function gate(sandbox, command, input) {
  const head = await git(sandbox.worktreePath, "rev-parse", "HEAD");
  if (await git(sandbox.worktreePath, "status", "--porcelain")) throw new Error("Acceptance: checks require a clean committed HEAD; human decision required");
  console.log(`Gate: ${command}`);
  const result = await sandbox.exec(command, { stdin: input, onLine: (line) => console.log(line) });
  if (await git(sandbox.worktreePath, "rev-parse", "HEAD") !== head || await git(sandbox.worktreePath, "status", "--porcelain")) throw new Error("Acceptance: checks modified committed work; evidence is invalid; human decision required");
  return { ok: result.exitCode === 0, head, output: `${result.stdout}\n${result.stderr}`.slice(-12000) };
}
export async function runRequiredCheck(sandbox, check) {
  const input = await repositoryGateInput(sandbox.worktreePath, check);
  const result = await gate(sandbox, check.command, input.stdin);
  return { ...result, runnerPath: input.runnerPath, runnerDigest: input.runnerDigest };
}
async function recoveryAgent(state, context) {
  // Host read-only CLI remains available when a development container fails.
  // It may diagnose Git/tickets, but cannot modify code, state, auth or limits.
  const prompt = [
    "Act as the Sandcastle recovery orchestrator. Diagnose the saved incident and choose a concrete recovery action. The user authorizes autonomous recovery and deferral of absent external dependencies while independent work continues.",
    "You are read-only. Inspect AGENTS.md, the original spec, ticket files and relevant Git revisions (use git show <branch>:<path> for current integration/worker evidence). Do not edit files, commit, merge, reset, run builds or write scheduler state. Do not read credentials/auth files or print environment variables. Return only schema JSON.",
    "retry: transient execution/tool failure; rerun the saved checkpoint, preserving successful workers. repair: a failing existing worker or unfinished assembly, build/test gate or invalid planning manifest; send focused repair guidance to its existing agent/pipeline, always through worker branches and assembly for code changes. defer: blocked implementation ticket genuinely requires an absent external owner; identify its repository-relative docs/tasks dependency document and preserve all acceptance obligations. stop: recovery needs a user decision or cannot safely proceed.",
    "For defer, name only blocked/failed active tickets in ticketPaths; the controller also defers downstream dependents. Successful siblings must be integrated first. Required tests, builds and acceptance gates cannot be skipped. Do not defer a code defect just to pass a gate. Do not widen the originating task into an absent later-stage flow. Explicitly deferred callers stay pending; they must not repeatedly reappear in repair batches.",
    `For retry/repair/stop use ticketPaths=[] and dependency=''. For defer use a real docs/tasks/...md dependency; read that document to confirm the missing prerequisite. Explain the concrete cause and restoration condition in summary. Existing quota/cycle limits cannot be increased or reset. At most three recovery decisions are allowed for the same cause at an unchanged checkpoint. A contract agent timeout at the ${readOnlyAgentTimeoutMs / 1000}-second ceiling cannot be retried unchanged; choose stop and explain the required bounded runtime repair.`,
    JSON.stringify(context, null, 2),
  ].join("\n\n");
  return JSON.parse(await hostAgent(state, { cwd: root, role: `recovery orchestrator ${state.recovery.id}`, prompt, schemaPath: resolve(root, ".sandcastle/recovery-output.schema.json"), timeoutMs: recoveryAgentTimeoutMs }));
}
async function hostAgent(state, { cwd, role, prompt, schemaPath, multiAgent = false, timeoutMs = readOnlyAgentTimeoutMs }) {
  console.log(`Agent: ${role} (host read-only)`);
  const captured = { message: "", tokens: 0 };
  const startedAt = Date.now();
  let result;
  try { result = await dispatchVerified(executionContext(state, role, cwd, {
    schemaPath, ...(role.startsWith("recovery orchestrator ") ? { expectedBranch: targetBranch, allowDirty: true } : {}),
  }), {
    record: manifest => recordExecution(state, role, manifest), quota: () => quotaCheckpoint(state),
    invoke: () => {
      const operation = execFileAsync("codex", readOnlyAgentArgs({ ...sandcastleAgentSettings(role, process.env, state.agentPolicy), schemaPath, multiAgent }),
        { cwd, env: { ...process.env, CODEX_HOME: authDir }, ...(timeoutMs ? { timeout: timeoutMs, killSignal: "SIGKILL" } : {}), maxBuffer: 8 * 1024 * 1024 });
      operation.child.stdin.on("error", () => {});
      operation.child.stdin.end([prompt, delegatedAgentPolicy].join("\n\n"));
      return operation;
    },
  }); }
  catch (error) {
    if (error.deterministic || classifyFailure(error).category === "quota") throw error;
    for (const line of (error.stdout ?? "").split("\n")) parseAgentOutput(line, captured);
    state.budget.tokens += captured.tokens;
    const elapsedMs = Date.now() - startedAt;
    (state.agentHistory ??= []).push({ role, status: error.killed ? "timeout" : "failed", elapsedMs, timeoutMs, at: new Date().toISOString() });
    await save(state);
    const failure = new Error(`${role} agent failed: ${error.killed ? `timeout after ${timeoutMs} ms` : error.code}: ${agentFailureDetail(captured, error.stderr)}`);
    if (error.killed) failure.code = "AGENT_TIMEOUT";
    failure.elapsedMs = elapsedMs;
    failure.timeoutMs = timeoutMs;
    throw failure;
  }
  for (const line of result.stdout.split("\n")) parseAgentOutput(line, captured);
  state.budget.tokens += captured.tokens;
  (state.agentHistory ??= []).push({ role, status: "completed", elapsedMs: Date.now() - startedAt, timeoutMs, at: new Date().toISOString() });
  await save(state);
  try { await quotaCheckpoint(state); }
  catch (error) { state.quotaError = error.message; }
  await save(state);
  return captured.message;
}
async function recover(sandbox, state) {
  const result = await runRecovery(state, {
    decide: (context) => recoveryAgent(state, context), save,
    document: async (incident, decision, preview) => {
      const worktree = decision.action === "defer" ? sandbox?.worktreePath ?? await recoveryWorktree(state) : undefined;
      await documentRecovery(state, incident, decision, preview, { worktree, runsDir, git, ensureCommit, manifest: manifestPath(state) });
    },
    event: (record) => appendStageEvent(runsDir, state, { stage: "recovery", status: record.action, summary: `Recovery ${record.id}: ${record.action} for ${record.category}; checkpoint ${record.phase}; ${record.affectedTickets.length} tickets deferred.`, tickets: record.affectedTickets }),
  });
  if (!result) throw new Error(`Recovery agent requested a stop: ${state.recoveries.at(-1).summary}`);
  return sandbox;
}
function contractContext(state) {
  assertContract(state);
  return `Immutable acceptance contract (${state.acceptance.contractDigest}):\n${JSON.stringify(state.acceptance.contract, null, 2)}`;
}
async function inspect(sandbox, state, role, prompt, schema, timeoutMs = readOnlyAgentTimeoutMs) {
  const head = await git(sandbox.worktreePath, "rev-parse", "HEAD");
  if (await git(sandbox.worktreePath, "status", "--porcelain")) throw new Error("Acceptance inspection requires a clean committed worktree");
  const response = await agent(sandbox, state, role, prompt, schema, true, timeoutMs);
  if (await git(sandbox.worktreePath, "rev-parse", "HEAD") !== head || await git(sandbox.worktreePath, "status", "--porcelain")) throw new Error("Read-only acceptance inspection changed the worktree");
  return JSON.parse(response);
}
async function contractSubstage(state, pending, name, run) {
  pending.step = name;
  await save(state);
  await appendStageEvent(runsDir, state, { stage: `contract-${name}`, status: "running", summary: `Contract ${name} started.` });
  try {
    const result = await run();
    await appendStageEvent(runsDir, state, { stage: `contract-${name}`, status: "completed", summary: `Contract ${name} completed.` });
    return result;
  } catch (error) {
    await appendStageEvent(runsDir, state, { stage: `contract-${name}`, status: "failed", summary: `Contract ${name} failed; saved checkpoint retained.` });
    throw error;
  }
}
async function contract(sandbox, state) {
  const amendment = state.contractAmendment ? structuredClone(state.contractAmendment) : undefined;
  let sourceResolution;
  let executionContext;
  await contractStage(state, {
    save, head: async () => { if (state.verificationVersion === 1) await assertVerificationTarget(state, sandbox.worktreePath); return git(sandbox.worktreePath, "rev-parse", "HEAD"); },
    sources: async () => {
      sourceResolution = await resolveContractSources({
        root,
        worktree: sandbox.worktreePath,
        specPath: state.specPath,
        spec: state.spec,
      });
      const pending = state.contractPending;
      if (pending.sourceManifest && pending.sourceManifest.digest !== sourceResolution.manifest.digest) {
        throw new Error("Acceptance contract source manifest changed during drafting; human decision required");
      }
      if (!pending.sourceManifest) {
        pending.sourceManifest = sourceResolution.manifest;
        await save(state);
      }
      executionContext ??= await contractExecutionContext(sandbox.worktreePath);
      return sourceResolution.sources;
    },
    propose: async (sources, feedback, pending) => {
      if (state.contractAmendment) {
        const archived = state.contractAmendments.at(-1);
        if (state.contractAmendment.type === "semantic-user-requirement") {
          return inspect(sandbox, state, "acceptance contract semantic amendment", [
            "Amend the previously accepted implementation contract only for the explicit user-authorized semantic decision. Return the complete amended contract as schema JSON. Preserve every prior criterion ID, source, requirement, mandatory flag, applicability, owner, deferral reason, and check mapping exactly; preserve every prior check exactly. Do not remove, weaken, defer, or reinterpret any prior obligation and do not claim pending evidence passed.",
            "Add the smallest mandatory/current criterion set that fully represents the new decision and cites the required new quotes from the amended originating spec. Map new implementation behavior only to existing runnable evidence from runnerInventory, or to controller-review for inspection-only obligations. Do not invent commands or alter completion scope.",
            ...groupContractPromptGuidance({ usesSourceSpans: false }),
            JSON.stringify({ sources, previousContract: archived.contract, amendment: state.contractAmendment, controllerReviewCheck, runnerInventory: executionContext }, null, 2),
            feedback.length ? `Saved verifier feedback is cumulative. Repair every still-applicable item without changing prior criteria:\n${JSON.stringify(feedback)}` : "",
          ].join("\n\n"), ".sandcastle/contract-output.schema.json");
        }
        return inspect(sandbox, state, "acceptance contract amendment", [
          "Amend the previously accepted implementation contract only to repair its unrunnable external-check mappings. Return the complete amended contract as schema JSON. Preserve every criterion ID, source, requirement, mandatory flag, applicability, owner, deferral reason, and every existing runnable mapping. Do not add or remove criteria, change completion scope, waive a requirement, claim pending evidence passed, or move a current obligation to deferred.",
          `The audited duplicate gate ${state.contractAmendment.checkId} must be removed without replacement from its criterion; retain that criterion's other runnable mappings exactly. P1-AC1 therefore remains mapped only to sv-build and sv-core. The expanded sv-core inventory below is controller fact and includes tests/sv_login_checks.py plus tests/sv_login_live_checks.py, which exercise the SV-B-006 production overview dependency. Do not add another P1-AC1 check.`,
          "Remove every other external check used by a mandatory/current criterion. For implementation behavior, select only an existing focused command from runnerInventory, or sv-core when its expanded inventory actually covers the obligation. For repository policy, scope inspection, workflow compliance, prerequisite-reading, and other review-only obligations, use the exact controllerReviewCheck. Never invent a test path and never use sv-core as a label for policy review. External checks may remain only for genuinely deferred criteria with the unchanged exact owner and reason.",
          ...groupContractPromptGuidance({ usesSourceSpans: false }),
          JSON.stringify({ sources, previousContract: archived.contract, amendment: state.contractAmendment, controllerReviewCheck, runnerInventory: executionContext }, null, 2),
          feedback.length ? `Saved verifier feedback is historical input. Repair every still-applicable item using the authoritative runner inventory and controller gates above:\n${JSON.stringify(feedback)}` : "",
        ].join("\n\n"), ".sandcastle/contract-output.schema.json");
      }
      const proposePart = (part, index, count) => inspect(sandbox, state, `acceptance contract group ${index + 1}/${count}`, withContractGroupRecoveryAdvice([
        `Draft numbered acceptance criteria for ONLY assigned major subtask ${index + 1}/${count}. This is a persisted checkpoint, not the full contract. Informational documents may clarify ownership or terminology but cannot introduce obligations outside the fixed authoritative source set. Do not edit files. Return schema JSON.`,
        ...groupContractPromptGuidance({ usesSourceSpans: Boolean(part.sourceSpans) }),
        "Assigned independent verifier feedback is cumulative and binding. Address every supplied item without dropping earlier corrections.",
        "Feature-specific checks may use only an existing command in runnerInventory, the exact controller-review gate for policy/scope inspection, or deferred external checks with command=''. Never invent a future test path and do not use shell operators. The controller prefixes IDs and merges all saved parts before independent verification.",
        JSON.stringify({ originatingTask: { path: state.specPath }, assignedSubtask: part, baselineChecks,
          feedback: part.sourceSpans ? feedbackForGroup(pending, part) : feedback,
          sourceManifest: sourceResolution.manifest,
          controllerReviewCheck,
          runnerInventory: executionContext,
          informationalPaths: Object.keys(sourceResolution.informationalSources) }, null, 2),
      ].join("\n\n"), state, index), part.sourceSpans ? ".sandcastle/contract-group-output.schema.json" : ".sandcastle/contract-output.schema.json");
      let draft;
      if (pending.parts || pending.partSourceDigest) {
        // Preserve checkpoints from runs started with the former sequential draft.
        draft = await buildContractDraft(pending, sources, proposePart, () => save(state));
      } else {
        const { parts, plan } = await contractSubstage(state, pending, "split", () => planContractGroups(pending, sources, (sections) => inspect(sandbox, state, "acceptance contract split", [
          "Split the original task and AGENTS.md into major, coherent subtasks for independent contract agents. You are read-only. Group the numbered source sections by requirement area; every section index must appear exactly once. Use at least two groups when there are at least two sections, and at most ten groups. Keep related requirements together without omitting any source. Return only schema JSON.",
          JSON.stringify({ originatingTask: state.specPath, sections: sections.map((section, index) => ({ index, ...section })), feedback }, null, 2),
          guidanceFor(state, { phase: "contract", substage: "split" }),
        ].filter(Boolean).join("\n\n"), ".sandcastle/contract-groups-output.schema.json"), () => save(state)));
        draft = await contractSubstage(state, pending, "parallel", () => buildGroupedContractDraft(pending, parts, plan, proposePart, () => save(state)));
      }
      return contractSubstage(state, pending, "assemble", () => reconcileContractDraft(pending, draft, sources, (proposal, lastError) => inspect(sandbox, state, "acceptance contract reconciliation", [
        "Assemble the parallel subtask contracts into ONE internally consistent contract before independent verification. This is a draft correction, not acceptance. Use only the fixed authoritative source set for obligations; informational documents may clarify ownership but cannot expand scope. Do not edit files. Return only a focused patch using the output schema: set completionScope to null when unchanged or to the corrected scope, replace existing criteria/checks or add missing ones, never remove criteria or downgrade mandatory requirements.",
        assemblyCommandRules,
        ...reconciliationContractPromptGuidance(),
        JSON.stringify({ sources, proposal, verifierFeedback: feedback, sourceManifest: sourceResolution.manifest, controllerReviewCheck, runnerInventory: executionContext,
          informationalPaths: Object.keys(sourceResolution.informationalSources) }, null, 2),
        lastError ? `Previous reconciliation patch was rejected. Repair this exact validation error without dropping obligations: ${lastError}` : "",
        guidanceFor(state, { phase: "contract", substage: "assemble" }),
      ].filter(Boolean).join("\n\n"), ".sandcastle/contract-reconcile-output.schema.json"), () => save(state)));
    },
    validateProposal: (proposal) => {
      validateContractRunnerInventory(proposal, executionContext.focusedRunnerPaths);
      return state.contractAmendment
        ? validateContractAmendment(state.contractAmendments.at(-1).contract, proposal, state.contractAmendment)
        : proposal;
    },
    verify: (proposal, sources) => contractSubstage(state, state.contractPending, "verify", () => inspect(sandbox, state, "independent contract verification", [
      "Independently compare the proposed acceptance contract against every obligation in the fixed authoritative source set. You did not create this contract. Do not edit files. Return approved=true only if requirements, production-path checks, scope boundaries and deferred owners are accurately represented. Scoring is not a gate. List omissions and unsupported deferrals in missingRequirements; each finding must use the identical message and authoritative sourceSpanIds.",
      state.contractAmendment?.type === "semantic-user-requirement"
        ? `This is a user-authorized semantic amendment of digest ${state.contractAmendment.previousDigest}. Verify that every prior criterion and check remains exactly preserved, the explicit decision and every required quote are represented by new mandatory/current criteria, no unrelated requirement is added, and no pending evidence is claimed passed.`
        : state.contractAmendment ? `This is a bounded amendment of digest ${state.contractAmendment.previousDigest}. Verify that every previous criterion remains semantically unchanged and mandatory, ${state.contractAmendment.checkId} is removed as the audited duplicate, all other mandatory/current external mappings are replaced by runnable evidence, and no pending evidence is claimed passed. Treat runnerInventory and controller checks as authoritative execution facts: sv-core expands to the listed exact repository tests; controller-review is the deterministic review plus fresh-final-audit gate at the exact digest and HEAD.` : "",
      ...verifierContractPromptGuidance(),
      JSON.stringify({ sources, sourceSpans: contractSourceCatalog(sources), proposal,
        ...(state.contractAmendment ? contractAmendmentVerificationContext(state) : {}),
        sourceManifest: sourceResolution.manifest, runnerInventory: executionContext,
        informationalPaths: Object.keys(sourceResolution.informationalSources) }, null, 2),
    ].join("\n\n"), ".sandcastle/contract-review-output.schema.json")),
    publish: async (accepted) => {
      await mkdir(resolve(sandbox.worktreePath, taskDir(state)), { recursive: true });
      await writeFile(resolve(sandbox.worktreePath, taskDir(state), "acceptance-contract.json"), JSON.stringify({ contract: accepted.contract, digest: accepted.contractDigest, verification: accepted.contractReview, migration: accepted.migration }, null, 2) + "\n");
      await ensureCommit(sandbox.worktreePath, "Sandcastle: accept independently verified contract");
    },
  });
  if (amendment && state.phase !== "contract" && state.acceptance) {
    const archive = state.contractAmendments.at(-1);
    archive.amendedContractDigest = state.acceptance.contractDigest;
    state.lastContractAmendment = { ...amendment, amendedContractDigest: state.acceptance.contractDigest };
    delete state.contractAmendment;
    delete state.contractAmendmentAttemptLimit;
    await save(state);
    await appendStageEvent(runsDir, state, { stage: "contract-amend", status: "completed", summary: amendment.type === "semantic-user-requirement"
      ? `User-authorized semantic amendment ${amendment.authorizationId} independently verified; prior criteria preserved.`
      : `Contract amendment independently verified; ${amendment.requiredExternalCheckIds.length} unrunnable mandatory/current external mappings resolved.` });
  }
}
async function plan(sandbox, state, reason = "") {
  assertContract(state);
  releaseVerificationTarget(state);
  const skill = await readFile(resolve(skillPaths["to-tickets"], "SKILL.md"), "utf8");
  const manifest = manifestPath(state);
  const prompt = [
    "Use the following to-tickets skill to split this work into small, independently verifiable vertical tickets. The user has already approved automatic decomposition and asked for questions only at critical failures or limits; skip the skill's quiz/approval step.",
    skill,
    `Read AGENTS.md, CONTEXT.md, relevant ADRs, and docs/agents/issue-tracker.md. Write numbered ticket files under ${taskDir(state)}/issues/ and a JSON manifest at ${manifest}.`,
    'Manifest format: {"tickets":[{"path":".scratch/.../issues/01-name.md","blockedBy":[],"findingIds":[]}]}. List tickets in dependency order; blockedBy may contain earlier paths in this batch or completed tickets from prior batches. Never depend on unresolved or deferred tickets. Each ticket must have acceptance criteria referencing contract IDs. When repairing review findings, every ticket must name its ledger findingIds and every open finding must be covered once as one coherent repair (multiple dependent tickets are allowed). Do not recreate a resolved/residual finding or repair the same defect through unrelated tickets.',
    'Manifest also requires executionPlan: {version:1,contractDigest,criteria:[{id,evidence:"inspection"|"artifact"|"deferred",artifactCheckIds:[]}],artifactChecks:[{id,owner:"controller",phase:"test-gate",target:"integrated-head",command:["python3","-B","tests/collector.py"],outputArgument:true,manifestPath:"manifest.json",syntheticOnly:true,caseArtifactSuffix:".log",inputPaths:["tests/collector.py"],expectedSuites:{ordinary:3},producedBy:"optional runner delivery ticket path"}]}. Cover every immutable criterion exactly once; runtime evidence requires an artifact producer, not inspection-only. Collector receives an appended fresh output-directory argument and must record tested_head, contract_digest, inputs_unchanged, sources, artifacts and suites with returncode/status/caseCount (or one declared caseArtifactSuffix (.log/.bin) per case, including cases with no network send). A newly needed collector is a worker delivery prerequisite, followed by its controller gate after assembly; never claim an absent collector already exists.',
    'Each ticket requires delivery:{owner:"worker",phase:"parallel-work",prerequisites:[]}. Each prerequisite is worker-local with owner=worker and phase=parallel-work. Markdown MUST use exact headings ## Worker delivery criteria and ## Controller acceptance obligations, distinguishing Worker delivery criteria (implementation/runner delivery and focused checks) from Controller acceptance obligations (required final integrated-HEAD build/core/runtime/independent review). Worker completion must never wait for controller obligations. Preserve all mandatory/current obligations in executionPlan; do not mislabel them deferred.',
    "For each behavior-changing ticket, name the public SV production seam for TDD in the ticket and link it to an accepted contract criterion or check. Use only a seam supported by the originating task and contract; flag an absent seam as a blocker instead of inventing a test-only interface.",
    `Completed tickets available as dependencies:\n${JSON.stringify(state.completedTickets)}`,
    state.scopeNotes || "Respect the originating spec's implementation and full acceptance boundaries. Generated tickets cannot expand the authoritative scope. Record deferred checks without claiming they passed.",
    "Commit the tickets and manifest. Ask no routine questions.",
    `Originating task (${state.specPath}):\n${state.spec}`,
    contractContext(state),
    state.acceptance.assessments.at(-1) ? `Preserve verified behavior:\n${JSON.stringify(state.acceptance.assessments.at(-1).preserved)}` : "",
    state.planReviewHistory?.at(-1) && (state.planReviewHistory.at(-1).review.approved===false || state.planReviewHistory.at(-1).review.missingRequirements.length>0) ? `Previous rejected binding (correct its concrete requirements; do not retry unchanged):\n${JSON.stringify(state.planReviewHistory.at(-1))}` : '',
    'For artifact input coverage inspect the entire runner and invoked scripts, compiled source dependencies (including transitively included headers), repository runtime resources and tool/runtime identity. List individual committed input/resource files, not directory entries. If an existing collector needs broader inputs or output fields, create an explicit worker delivery prerequisite via producedBy and fix its source manifest before controller capture; do not only change the plan declaration. Preserve the established suite matrix and no-send cases.',
    reason,
  ].join("\n\n");
  await agent(sandbox, state, "ticket planning", prompt);
  await ensureCommit(sandbox.worktreePath, `Sandcastle: plan ticket batch ${state.reviewRound}`);
  state.phase = "plan-validate";
  await save(state);
  await acceptPlan(sandbox, state);
}
async function acceptPlan(sandbox, state) {
  const manifest = manifestPath(state);
  const parsed = JSON.parse(await readFile(resolve(sandbox.worktreePath, manifest), "utf8"));
  validateTicketBatch(parsed, { ...state, verificationVersion: 1 });
  validateExecutionPlan(parsed.executionPlan, state.acceptance.contract, state.acceptance.contractDigest, parsed.tickets);
  const bindingReview = await inspect(sandbox, state, "independent execution-plan verifier", [
    "Independently verify this execution binding against every original contract obligation and all ticket Markdown. Read the actual files. Inspect ownership, stage reachability, worker delivery versus independent final acceptance, real collector invocation/output format and coverage. Runtime/capture/evidence requirements must use controller-owned artifact producers after assembly. Generated tickets cannot defer, weaken or expand requirements. Inspect available runners or explicit delivery prerequisites. Return approved only for a reachable complete plan; runtime backend/expected case matrix/input coverage must be justified from source. Do not edit files. Return contract-review schema JSON.",
    contractContext(state), JSON.stringify(parsed),
  ].join("\n\n"), ".sandcastle/contract-review-output.schema.json");
  const reviewRecord={at:new Date().toISOString(),head:await git(sandbox.worktreePath,'rev-parse','HEAD'),manifest,bindingDigest:digest(parsed.executionPlan),contractDigest:state.acceptance.contractDigest,review:structuredClone(bindingReview)};
  (state.planReviewHistory??=[]).push(reviewRecord);
  await writeFile(resolve(runsDir,state.id,`plan-review-${state.planReviewHistory.length}.json`),JSON.stringify(reviewRecord,null,2)+'\n');
  await save(state);
  acceptExecutionPlan(state, parsed.executionPlan, bindingReview, parsed.tickets);
  registerRepairBatch(state, parsed.tickets, manifest);
  for (const ticket of parsed.tickets) {
    await access(resolve(sandbox.worktreePath, ticket.path));
  }
  state.tickets = parsed.tickets;
  state.ticketIndex = 0;
  state.batchCount++;
  if (state.batchCount > 1) state.resetBuildBudgetAfterBatch = true;
  if (!state.repairKind) state.testAttempts = 0;
  delete state.repairKind;
  delete state.pendingFailure;
  delete state.recoveryAdvice;
  delete state.recoveryAdviceTarget;
  state.phase = "tickets";
  await save(state);
}
async function startTicketWave(sandbox, state) {
  if (state.tickets.every((ticket) => dependencyReadyTickets(state).has(ticket.path))) {
    countIntegratedRepairs(state);
    refreshBuildBudgetsAfterBatch(state);
    delete state.reviewFindings;
    state.phase = "build";
    await save(state);
    return;
  }
  createWave(state, { baseCommit: await git(sandbox.worktreePath, "rev-parse", "HEAD"), limit: state.parallelism, usedBranches: await workerBranchNames(state) });
  await save(state);
}
function sandboxProvider(evidenceInbox, hostMounts = []) {
  const proxy = process.env.all_proxy || process.env.ALL_PROXY;
  return docker({ network: "host", env: proxy ? { all_proxy: proxy } : {}, mounts: [
    ...skillMounts(), { hostPath: authDir, sandboxPath: "/home/agent/.codex", readonly: false },
    ...runtimeSchemaMounts(root),
    ...hostMounts,
    ...(evidenceInbox ? [{ hostPath: evidenceInbox, sandboxPath: "/home/agent/.sandcastle-evidence-out", readonly: false }] : []),
  ] });
}
async function openWorker(state, member, wave) {
  // Git worktree registration is serialized; agent work itself is concurrent.
  const next = provisionQueue.then(async () => {
    await recoverWorktree({ id: state.id, branch: member.branch });
    let hostChannel;
    try {
      const mounts=[];
      const delivery=workerHostDelivery(state,member,wave);
      if (process.env.SANDCASTLE_LINUX_HOST_POLICY) {
        const scope={runId:state.id,wave:wave.id,ticket:member.ticket.path,branch:member.branch,baseCommit:wave.baseCommit};
        hostChannel=await startLinuxHostChannel({root,policyPath:process.env.SANDCASTLE_LINUX_HOST_POLICY,scope,directory:resolve(runsDir,state.id,'host-channels',randomUUID())});
        if(delivery)mounts.push({hostPath:delivery.deliveryDirectory,sandboxPath:'/home/agent/.sv-host-delivery',readonly:true});
        const scopePath=resolve(runsDir,state.id,`host-scope-${randomUUID()}.json`);
        await writeFile(scopePath,JSON.stringify({...scope,contractDigest:state.acceptance.contractDigest}),{mode:0o600});
        mounts.push(hostChannel.mount,{hostPath:scopePath,sandboxPath:'/home/agent/.sv-host-scope.json',readonly:true},
          {hostPath:resolve(root,'.sandcastle/linux-host-client.mjs'),sandboxPath:'/home/agent/.sv-host-client.mjs',readonly:true});
      }
      const worker=await createSandbox({ cwd: root, branch: member.branch, baseBranch: wave.baseCommit, sandbox: sandboxProvider(undefined,mounts) });
      worker.linuxHostChannel=hostChannel;worker.linuxHostDelivery=delivery;return worker;
    } catch(error) { if(hostChannel)await hostChannel.close();throw error; }
  });
  provisionQueue = next.then(() => {}, () => {});
  return next;
}
async function implementWorker(worker, state, member) {
  const ticket = member.ticket;
  const content = await readFile(resolve(worker.worktreePath, ticket.path), "utf8");
  const tddSkill = await readFile(resolve(skillPaths.tdd, "SKILL.md"), "utf8");
  const prompt = [
    `Implement exactly ${ticket.path} in your isolated branch ${member.branch}. Other agents work in separate branches; do not edit or merge those branches.`, content,
    "Read AGENTS.md, CONTEXT.md and relevant ADRs. Claim the ticket before work and preserve its history. Keep legacy and shared edits minimal. Exercise the SV production path. Add meaningful tests for changed behavior. Commit your changes. Do not run the whole build/test/review workflow: the orchestrator runs those gates after all tickets. Ask only on critical failure.",
    "Use the following $tdd skill for behavior-changing code. Identify the public SV production seam from the accepted contract and ticket before writing a test. At that seam, run a failing test before the minimal implementation, then rerun it to show green. Work in vertical slices. Record the seam and red/green commands and results in the ticket Answer. If no agreed production seam exists for a behavior change, do not invent a test-only implementation; return blocked with the missing seam as the reason. For a focused testing ticket that only adds coverage of already implemented behavior, record the checks without claiming a test-first loop; use TDD if its test exposes behavior that must be fixed.",
    tddSkill,
    state.scopeNotes || "Keep deferred acceptance checks at their originating owners; do not invent absent callers or broaden the task.",
    contractContext(state),
    ...(worker.linuxHostDelivery ? [`Controller independently verified the corrected delivery matrix at source HEAD ${worker.linuxHostDelivery.head}. Read /home/agent/.sv-host-delivery/index.json and the readonly software/OpenGL reports and sealed artifacts. Verify your production/driver/producer input hashes against these reports; preserve this exact tested source identity when only committing ticket bookkeeping. The verified Linux video driver is ${worker.linuxHostDelivery.driver}; select this driver for any newly approved capture. Do not repeat the matrix or request unapproved new source solely because history metadata changes. Complete only substantiated worker delivery checkboxes and retain controller/full acceptance obligations pending. Any production/driver/runner change requires a fresh independently approved test.`] : []),
    ...(worker.linuxHostChannel ? ['Controller bootstrap is available through node /home/agent/.sv-host-client.mjs /home/agent/.sv-host-scope.json preflight|readiness|interaction software|opengl ABSENT_OUTPUT_DIR [wayland|x11]. The optional driver defaults to wayland; use x11 for the independently verified XWayland delivery. It uses your committed Git HEAD, private scoped requests and real Linux host execution. Docker desktop variables/sockets need not be present. The controller automatically carries delivery source approval across metadata paths and exact file versions delegated by its private reviewed policy; returned receipts record this proof. Do not rerun the interaction matrix solely to inspect a dependency or after bookkeeping changes when verified delivery is mounted. Implement ticket08 collectors and their focused checks before requesting approval of their new consumed-input closures. A changed production/runner inventory requires a new host policy approval, not arbitrary host execution. When a host policy rejects an action/source, return blocked with the typed Execution preflight [HOST_APPROVAL_REQUIRED] diagnostic; this controller-owned prerequisite is not another worker code repair. Ticket07 implements driver/capture; ticket08 implements complete collectors. Readiness is supplemental and never full interaction acceptance.'] : []),
    `Keep accepted behavior intact: ${JSON.stringify(state.acceptance.assessments.at(-1)?.preserved ?? [])}`,
    'Return only JSON matching the output schema. status=completed requires implemented behavior, focused verification, a committed result and the ticket marked Status: resolved with an Answer. If a dependency or required environment is missing, return status=blocked with the concrete reason; never claim completion.',
    ...(worker.linuxHostChannel ? ['When committed collector/source delivery needs controller-owned host approval, return status=blocked with hostApprovalRequest:{action:"entry-runtime-discovery" or the concrete scoped action required,driver:"x11",renderer:"software"}. The controller independently checks that request against its private host policy without running the job, and pauses directly on a verified HOST_APPROVAL_REQUIRED rather than asking another worker to self-approve. Readonly historical dependency evidence remains usable at its original tested HEAD while implementing new collectors; it does not authorize current host execution.'] : []),
  ].join("\n\n");
  const response = await agent(worker, state, `implementation ${ticket.path}`, prompt, ".sandcastle/ticket-output.schema.json", false, readOnlyAgentTimeoutMs,
    { expectedBranch: member.branch, baseCommit: state.wave.baseCommit, ticketPath: ticket.path });
  const result = JSON.parse(response);
  await ensureCommit(worker.worktreePath, `Sandcastle: preserve ${ticket.path}`);
  const head=await git(worker.worktreePath,'rev-parse','HEAD');
  const hostFailure=await workerHostFailure(worker.linuxHostChannel,result,{version:1,runId:state.id,wave:state.wave.id,ticket:ticket.path,branch:member.branch,baseCommit:state.wave.baseCommit,head,contractDigest:state.acceptance.contractDigest,nonce:randomUUID().replaceAll('-','')});
  if (result.status === "completed") validateTicketCompletion(await readFile(resolve(worker.worktreePath, ticket.path), "utf8"), ticket.path, ticket.delivery);
  await appendStageEvent(runsDir, state, { stage: "ticket", status: result.status, summary: `${ticket.path}: ${result.status}; changes await assembly.`, tickets: [ticket.path] });
  return { result, head,...(hostFailure?{failure:hostFailure}:{}) };
}
async function runWorkers(state) {
  await runWave(state, {
    save,
    run: async (member, wave) => {
      let worker, failure;
      try {
        worker = await openWorker(state, member, wave);
        await preflightWorker({ worktree: worker.worktreePath, expectedBranch: member.branch, baseCommit: wave.baseCommit, ticketPath: member.ticket.path });
        return await implementWorker(worker, state, member);
      } catch (error) {
        failure = error;
        await appendStageEvent(runsDir, state, { stage: "ticket", status: "failed", summary: `${member.ticket.path}: worker failed; its branch is retained.`, tickets: [member.ticket.path] });
        throw error;
      } finally {
        if (worker) {
          try { if(worker.linuxHostChannel)await worker.linuxHostChannel.close(); }
          finally { await closeExecution(worker, { failure, preserve: () => ensureCommit(worker.worktreePath, `Sandcastle: preserve wave ${wave.id} worker`) }); }
        }
      }
    },
  });
}
async function assemble(sandbox, state) {
  try { await collectWave(state, {
    save,
    assemble: async (wave) => {
      const contributions = assemblyContributions(state,wave);
      const prompt = [
        `Assemble wave ${wave.id} on the current integration branch ${state.branch}. Read AGENTS.md, CONTEXT.md and each contributing ticket.`,
        `Common base: ${wave.baseCommit}. Merge these committed worker revisions using Git, in the listed order:\n${JSON.stringify(contributions, null, 2)}`,
        "First inspect git status and any in-progress merge. If a conflict exists, use $resolving-merge-conflicts: inspect both intents and their tickets, resolve every hunk, run focused checks, and finish the merge. Never abort, reset, discard a worker's intended behavior, or mark conflict-containing files as resolved without fixing them.",
        "Validated delivery receipts may retain their historical claimed Markdown and blocked result; their controller acceptance is pending. Only the exact listed delivery receipt revisions are allowed alongside completed revisions. For each revision use git merge-base --is-ancestor to skip revisions already integrated, otherwise git merge --no-ff the exact revision. Preserve commit ancestry for all successful workers. Do not merge failed or blocked worker branches unless the exact listed revision has a controller-validated delivery receipt; that exception does not establish acceptance.",
        "After merging, inspect the combined change, fix only integration errors and run appropriate focused checks. Ordinary completed tickets must remain resolved with their Answer; controller-validated delivery keeps historical acceptance pending. Commit all corrections and leave a clean worktree without unmerged files. Do not run the full workflow; the orchestrator runs its gates next.",
        state.scopeNotes || "Preserve the original implementation/acceptance boundary.",
        contractContext(state),
        'Return only JSON matching the schema: completed if all listed revisions are integrated and focused checks passed, otherwise blocked with a concrete reason.',
      ].join("\n\n");
      const response = await agent(sandbox, state, `assembly wave ${wave.id}`, prompt, ".sandcastle/ticket-output.schema.json");
      return JSON.parse(response);
    },
    verify: async (member) => {
      await git(sandbox.worktreePath, "merge-base", "--is-ancestor", member.head, "HEAD");
      if (await git(sandbox.worktreePath, "ls-files", "-u")) throw new Error("Assembly left unmerged files");
      if (await git(sandbox.worktreePath, "status", "--porcelain")) throw new Error("Assembly left uncommitted changes");
      const content = await readFile(resolve(sandbox.worktreePath, member.ticket.path), "utf8");
      if (isDeliveryReady(state, member)) {
        if (digest(content) !== member.deliveryReceipt.ticketDigest) throw verificationError("DELIVERY_INVALID", "assembly changed the validated delivery ticket");
      } else validateTicketCompletion(content, member.ticket.path, member.ticket.delivery);
    },
  }); } finally {
    countIntegratedRepairs(state);
    await save(state);
  }
}
async function build(sandbox, state) {
  recordBuildAttempt(state, "build");
  await save(state);
  const result = await gate(sandbox, "make -s -C src -f makefile.sv tomenet-sv");
  await checkEvidence(sandbox, state, "sv-build", result);
  if (result.ok) { state.phase = "test"; await save(state); return; }
  state.pendingFailure = result.output;
  state.phase = "build-repair";
  await save(state);
  if (state.buildAttempts >= state.buildLimit) throw new Error(`Build failed ${state.buildAttempts} times; continuation requires human approval.\n${result.output}`);
  throw new Error("SV build gate failed; recovery must choose the next action");
}
async function repairBuild(sandbox, state) {
  state.repairKind = "build";
  await save(state);
  await plan(sandbox, state, `Split the failed SV build into focused repair tickets. All code repairs must go through worker branches and assembly; preserve the original scope. Build output:\n${state.pendingFailure}`);
}
async function test(sandbox, state) {
  // Testing may add code, so it uses the same isolated-worker/assembly path.
  const issues = resolve(sandbox.worktreePath, taskDir(state), "issues");
  const entries = await readdir(issues);
  const number = Math.max(0, ...entries.map((name) => Number(name.match(/^(\d+)-/)?.[1] ?? 0))) + 1;
  const path = `${taskDir(state)}/issues/${String(number).padStart(2, "0")}-focused-test-coverage.md`;
  await writeFile(resolve(sandbox.worktreePath, path), focusedTestTicketContents(state, number));
  await ensureCommit(sandbox.worktreePath, "Sandcastle: plan focused test coverage");
  createWave(state, { baseCommit: await git(sandbox.worktreePath, "rev-parse", "HEAD"), tickets: [{ path, blockedBy: [], ...(state.verificationVersion === 1 ? { delivery: { owner: "worker", phase: "parallel-work", prerequisites: [] } } : {}) }], limit: 1, after: "test-gate", usedBranches: await workerBranchNames(state) });
  await save(state);
}
async function verificationRuntime(sandbox, state) {
  const provenance = await preflightExecution(executionContext(state, "verification runtime", sandbox.worktreePath, { sandbox }));
  const linuxHost=process.env.SANDCASTLE_LINUX_HOST_POLICY ? await startLinuxHostChannel({root,policyPath:process.env.SANDCASTLE_LINUX_HOST_POLICY,scope:{runId:state.id},inspectOnly:true}) : undefined;
  return { ...provenance.runtime, toolchain: await toolchainIdentity(sandbox),...(linuxHost?{linuxHost:linuxHost.identity}:{}) };
}
async function freezeTarget(sandbox, state) {
  assertExecutionPlan(state);
  // Fail before runtime probing/build/artifact attempts; reconciliation is not
  // an implicit part of verification migration or accepted feature scope.
  await assertIntegrationBase(root, sandbox.worktreePath, targetBranch);
  if(process.env.SANDCASTLE_LINUX_HOST_POLICY) {
    const policy=JSON.parse(await readFile(process.env.SANDCASTLE_LINUX_HOST_POLICY,'utf8'));
    const head=await git(sandbox.worktreePath,'rev-parse','HEAD');
    for(const checkId of policy.hostCheckIds??[]) {
      const binding=requiredArtifactChecks(state).find(item=>item.id===checkId);
      if(!binding)throw new Error('Linux host: declared host check absent from accepted plan');
      const entry=Object.entries(policy.actions).find(([,action])=>action.checkId===checkId&&action.controllerOnly===true);
      if(!entry)throw Object.assign(new Error(`Execution preflight [HOST_APPROVAL_REQUIRED]: ${checkId} needs independent collector/source approval before freeze; preserve counters and approve delivered assembled inputs`),{code:'HOST_APPROVAL_REQUIRED',deterministic:true});
      const scope={runId:state.id,wave:0,ticket:binding.producedBy,branch:state.branch,baseCommit:state.baseCommit};
      const inspected=await startLinuxHostChannel({root,policyPath:process.env.SANDCASTLE_LINUX_HOST_POLICY,scope,inspectOnly:true});
      await inspected.authorize({version:1,...scope,head,contractDigest:state.acceptance.contractDigest,nonce:'0'.repeat(32),action:entry[0],driver:'wayland',renderer:'software'},binding);
    }
  }
  await freezeVerificationTarget(state, sandbox.worktreePath, { controllerDigest: loadedControllerDigest, runtime: await verificationRuntime(sandbox, state), publication: { root, branch: targetBranch } });
  await save(state);
}
async function testGate(sandbox, state) {
  if (state.verificationVersion === 1) await freezeTarget(sandbox, state);
  recordBuildAttempt(state, "test-gate");
  await save(state);
  const rebuilt = await gate(sandbox, "make -s -C src -f makefile.sv tomenet-sv");
  await checkEvidence(sandbox, state, "sv-build", rebuilt);
  if (!rebuilt.ok) {
    state.pendingFailure = rebuilt.output;
    state.phase = "build-repair";
    await save(state);
    if (state.testBuildAttempts >= state.buildLimit) throw new Error(`Test-gate rebuild failed ${state.testBuildAttempts} times; continuation requires human approval.\n${rebuilt.output}`);
    throw new Error("SV rebuild gate failed; recovery must choose the next action");
  }
  state.testAttempts++;
  state.totalTestAttempts++;
  await save(state);
  let result;
  for (const check of requiredChecks(state.acceptance.contract).filter((item) => item.id !== "sv-build")) {
    if (check.kind === "external") throw new Error(`Acceptance: required external check ${check.id} has no runner; human decision required`);
    if (check.kind === "review") continue;
    result = await runRequiredCheck(sandbox, check);
    await checkEvidence(sandbox, state, check.id, result);
    if (!result.ok) { result.output = `${check.id}: ${result.output}`; break; }
  }
  if (result.ok) {
    if (state.verificationVersion === 1) {
      for (const binding of requiredArtifactChecks(state)) {
        const receipt = await executeArtifactCheck(sandbox, state, binding, { runDirectory: resolve(runsDir, state.id), save,
          hostArtifact: process.env.SANDCASTLE_LINUX_HOST_POLICY ? async ({binding,target,inbox}) => {
            const policy=JSON.parse(await readFile(process.env.SANDCASTLE_LINUX_HOST_POLICY,'utf8'));
            if(!policy.hostCheckIds?.includes(binding.id))return null;
            const entry=Object.entries(policy.actions).find(([,action])=>action.checkId===binding.id&&action.controllerOnly===true);
            if(!entry)throw new Error('Linux host: final collector needs independent source/binding approval');
            const scope={runId:state.id,wave:0,ticket:binding.producedBy,branch:state.branch,baseCommit:state.baseCommit};
            const channel=await startLinuxHostChannel({root,policyPath:process.env.SANDCASTLE_LINUX_HOST_POLICY,scope,directory:resolve(runsDir,state.id,'host-channels',randomUUID())});
            try {return await channel.request({version:1,...scope,head:target.head,contractDigest:target.contractDigest,nonce:randomUUID().replaceAll('-',''),action:entry[0],driver:'wayland',renderer:'software'},{binding,output:inbox});}
            finally {await channel.close();}
          } : undefined,
          assertRuntime: async target => { if (digest(await verificationRuntime(sandbox, state)) !== digest(target.runtime)) throw verificationError("VERIFICATION_RUNTIME_CHANGED", "check runtime changed"); } });
        (state.artifactEvidence ??= []).push(receipt); await save(state);
        if (!receipt.passed) throw verificationError("EVIDENCE_FAILED", `artifact gate ${binding.id} failed`);
      }
      await verifyRequiredArtifacts(state, sandbox.worktreePath); await save(state);
    }
    state.phase = "review"; await save(state); return;
  }
  state.pendingFailure = result.output;
  state.phase = "test-repair";
  await save(state);
  if (state.testAttempts >= state.testLimit) throw new Error(`Tests failed ${state.testAttempts} times; continuation requires human approval.\n${result.output}`);
  throw new Error("SV test gate failed; recovery must choose the next action");
}
async function repairTest(sandbox, state) {
  state.repairKind = "test";
  await save(state);
  await plan(sandbox, state, `Split the failing SV tests into focused repair tickets. Exercise the production path, keep the originating scope and route every code repair through workers and assembly. Test output:\n${state.pendingFailure}`);
}
async function checkEvidence(sandbox, state, checkId, result) {
  const head = await git(sandbox.worktreePath, "rev-parse", "HEAD");
  if (head !== result.head) throw new Error("Acceptance: HEAD changed before saving check evidence; human decision required");
  const logPath = resolve(runsDir, state.id, `check-${checkId}-${state.totalBuildAttempts}-${state.totalTestAttempts}.json`);
  await writeFile(logPath, JSON.stringify({ head, checkId, passed: result.ok, output: result.output, runnerPath: result.runnerPath, runnerDigest: result.runnerDigest }, null, 2) + "\n");
  recordCheck(state, { checkId, head, passed: result.ok, logPath });
  await save(state);
}
async function review(sandbox, state, audit = false) {
  if (state.verificationVersion === 1) { await assertVerificationTarget(state, sandbox.worktreePath); await verifyRequiredArtifacts(state, sandbox.worktreePath); }
  await assessmentStage(state, {
    save, head: async () => { if (state.verificationVersion === 1) await assertVerificationTarget(state, sandbox.worktreePath); return git(sandbox.worktreePath, "rev-parse", "HEAD"); },
    inspect: ({ head, validationError }) => inspect(sandbox, state, audit ? "fresh final acceptance auditor" : `review ${state.reviewRound}`, [
      audit
        ? `You are a fresh independent final auditor. You did not implement, plan or review this change. Inspect the final committed production result at ${head} and baseline ${state.baseCommit} against the original task and accepted contract. Do not read earlier review/recovery reports, ticket Answers or previous reviewers' conclusions. Check every criterion, controller evidence, preserved baseline behavior and candidate residual risk independently. Do not edit files. Return only schema JSON.`
        : `$code-review Review the committed diff since ${state.baseCommit}. Run Standards and Spec axes in parallel as the skill directs. Reconcile both axes into a deduplicated result. Read AGENTS.md; do not edit files. Return only schema JSON.`,
      "The original task and accepted contract are authoritative. Generated tickets cannot broaden scope or weaken requirements. Explicit deferred caller/native/platform acceptance stays pending with its owner; headless success does not prove full acceptance. Give one evidenced status per contract criterion and list verified behavior to preserve. Any failed current criterion needs a finding.",
      "Each defect has a stable defectKey and production area identifying its root problem, not its line/file/title. Reuse exact saved id/defectKey/area when a known issue is paraphrased or moves files. New findings use id=''. Never create another ID for the same underlying defect. Deduplicate across axes; retain the strongest severity/impact. resolved requires a saved ledger ID and concrete evidence at THIS HEAD. Silence does not resolve an issue. Reopen a previously resolved/accepted issue only with new concrete evidence.",
      "critical/high/medium, behavior, regression, protocol, security, data-loss, scope violations, false evidence and unmet mandatory requirements block closure. Only low cosmetic/maintainability findings linked exclusively to optional current criteria may be accepted in acceptedResiduals, with reason, risk, owner and returnCondition. Use its saved ledger ID, or defectKey for a new finding in this response. Residuals remain open backlog entries, not fixed issues. Independently confirm EVERY candidate residual in a final audit. A three-repair ceiling cannot make a blocker acceptable.",
      "After three integrated repairs of a defect, make a final explicit disposition: accept it only if eligible for a documented residual, otherwise leave it open for a human decision. Never request a fourth repair. Optional low polish should not prevent readiness once mandatory behavior and checks pass.",
      contractContext(state),
      `Original task:\n${state.spec}`,
      `Controller evidence at ${head}:\n${JSON.stringify(state.acceptance.evidence.filter((item) => item.head === head), null, 2)}`,
      state.verificationVersion === 1 ? `Durable controller artifact receipts (read the sealed manifest and artifacts independently; hashing does not establish semantics):\n${JSON.stringify(state.verifiedArtifactReferences)}` : "",
      // Final audit receives facts and candidate risks, not previous review verdicts.
      audit
        ? `Candidate residuals requiring independent assessment (these proposals are not an acceptance verdict):\n${JSON.stringify(state.acceptance.ledger.filter((item) => item.status === "residual").map(({ id, defectKey, area, criterionIds, file, line, problem, residual }) => ({ id, defectKey, area, criterionIds, file, line, problem, residual })), null, 2)}`
        : `Defect register:\n${JSON.stringify(state.acceptance.ledger, null, 2)}`,
      !audit && state.acceptance.migration && state.acceptance.assessments.length === 0 ? `Legacy reviews for initial identity reconciliation:\n${JSON.stringify(state.reviews.map((item) => ({ head: item.head, findings: item.findings })), null, 2)}` : "",
      guidanceFor(state, { phase: audit ? "final-audit" : "review" }),
      validationError ? `Technical correction: the preceding response did not pass controller validation: ${validationError}. Recheck independently and return a complete schema-conforming response; do not infer a desired verdict.` : "",
    ].filter(Boolean).join("\n\n"), ".sandcastle/review-output.schema.json", audit ? readOnlyAgentTimeoutMs : 0),
    publish: async (parsed, { head }) => {
      if (state.verificationVersion === 1) { await assertVerificationTarget(state, sandbox.worktreePath); await verifyRequiredArtifacts(state, sandbox.worktreePath); }
      const path = resolve(runsDir, state.id, audit ? `final-audit-${state.acceptance.finalAudits.length}.json` : `review-${state.reviewRound}.json`);
      await writeFile(path, JSON.stringify({ head, ...parsed }, null, 2) + "\n");
      if (!audit && !state.reviews.some((item) => item.path === path)) state.reviews.push({ path, head, ...parsed });
    },
  }, { audit });
}
async function report(sandbox, state) {
  const head = state.acceptance.finalVerdict.head;
  const outcome = await verifiedTree(sandbox, state);
  state.acceptance.finalVerdict = outcome;
  const reportPath = `docs/tasks/sandcastle/${state.id}-report.md`;
  const contents = [
    `# Sandcastle task ${state.id}`, "", `Specification: ${state.specPath}`, `Base: ${state.baseCommit}`, `Verified code HEAD: ${head}`, "",
    "## Originating request", "", state.spec, "",
    `Ticket batches: ${state.batchCount}`, `Build attempts: ${state.totalBuildAttempts}`, `Test attempts: ${state.totalTestAttempts}`, `Code review rounds: ${state.reviewRound}`, "",
    `Parallelism: ${state.parallelism}`, `Assembled waves: ${(state.waveHistory ?? []).length}`, "",
    "## Completed tickets", "", ...state.completedTickets.map((path) => `- ${path}`), "",
    "## Integrated deliveries with controller-owned acceptance", "", ...(state.deliveredTickets ?? []).map(path => `- ${path}: delivery integrated; historical worker status retained. Run acceptance is established only by the final verdict below.`), "",
    "## Deferred checks (not completed)", "", ...(state.deferredTickets ?? []).map((ticket) => `- ${ticket.path}: ${ticket.reason}`), "",
    "## Automatic recovery decisions", "", ...(state.recoveries ?? []).map((record) => `- ${record.action} at ${record.phase}: ${record.summary} Record: ${record.recordPath}`), "",
    "## Reviews", "", ...state.reviews.flatMap((item, index) => [
      `### Round ${index + 1}`, "", `Reviewed HEAD: ${item.head}`, `Summary: ${item.summary}`, "",
      ...(item.findings.length ? item.findings.map((finding) => `- ${finding.severity}: ${finding.file}:${finding.line} — ${finding.problem} Fix: ${finding.fix}`) : ["- No findings"]), "",
    ]),
    acceptanceReport(state), "",
    "Headless gates do not establish full ticket acceptance. Later caller integrations, display-backed human review and platform checks remain pending unless separately evidenced.", "",
  ].join("\n");
  await mkdir(resolve(sandbox.worktreePath, "docs/tasks/sandcastle"), { recursive: true });
  await mkdir(resolve(sandbox.worktreePath, taskDir(state), "recovery"), { recursive: true });
  for (const record of state.recoveries ?? []) await writeFile(resolve(sandbox.worktreePath, record.recordPath), recoveryRecord(record));
  for (const item of state.acceptance.ledger.filter((entry) => entry.status === "residual")) {
    await mkdir(resolve(sandbox.worktreePath, taskDir(state), "residuals"), { recursive: true });
    await writeFile(resolve(sandbox.worktreePath, item.residual.backlogPath), [
      `# ${item.id}: ${item.problem}`, "", "Type: implementation", "Status: open", "Assignee: unassigned", "Labels: enhancement, ready-for-human", "",
      `Owner: ${item.residual.owner}`, `Criteria: ${item.criterionIds.join(", ")}`, `Risk: ${item.residual.risk}`, `Accepted because: ${item.residual.reason}`, `Return when: ${item.residual.returnCondition}`, `Verified HEAD: ${head}`, "", "- [ ] Residual issue is resolved and verified through the production path.", "",
    ].join("\n"));
  }
  await writeFile(resolve(sandbox.worktreePath, reportPath), contents);
  await ensureCommit(sandbox.worktreePath, `Sandcastle: report task ${state.id}`);
  state.phase = "integrate";
  await save(state);
}
async function integrate(sandbox, state) {
  if (state.verificationVersion === 1) await assertIntegrationBase(root, sandbox.worktreePath, targetBranch);
  await verifiedTree(sandbox, state);
  if (await git(root, "status", "--porcelain")) throw new Error("Main checkout has uncommitted changes; cannot fast-forward modern_interface safely");
  if (await git(root, "branch", "--show-current") !== targetBranch) throw new Error(`Main checkout must be on ${targetBranch} for integration`);
  await git(root, "merge", "--ff-only", state.branch);
  state.phase = "done";
  await save(state);
  console.log(`Task committed to ${targetBranch}: ${await git(root, "rev-parse", "HEAD")}`);
}
export async function verifiedTree(sandbox, state) {
  const verified = state.acceptance?.finalVerdict?.head;
  assertAuditedChanges(state, []);
  await git(sandbox.worktreePath, "merge-base", "--is-ancestor", verified, "HEAD");
  const changed = (await git(sandbox.worktreePath, "diff", "--name-only", verified, "HEAD")).split("\n").filter(Boolean);
  // A temporary unaudited code change followed by a revert is still a code
  // descendant, not a permitted report-only publication.
  for (const revision of (await git(sandbox.worktreePath, "rev-list", `${verified}..HEAD`)).split("\n").filter(Boolean)) {
    changed.push(...(await git(sandbox.worktreePath, "diff-tree", "--no-commit-id", "--name-only", "-r", "-m", revision)).split("\n").filter(Boolean));
  }
  const outcome = assertAuditedChanges(state, changed);
  if (state.verificationVersion === 1) {
    // Publication descendants have already passed the strict path/ancestry gate.
    for (const reference of state.verifiedArtifactReferences ?? []) await verifyEvidence(reference, state.verificationTarget);
    if (state.verifiedArtifactReferences?.length !== requiredArtifactChecks(state).length) throw verificationError("EVIDENCE_MISSING", "publication lost artifact evidence");
  }
  return outcome;
}

export async function main() {
rejectLegacyContinue();
const attemptGrant = readAttemptGrant();
const modeKeys = ["SANDCASTLE_HOST_APPROVAL_RESUME_JSON", "SANDCASTLE_PLAN_REPAIR_RESUME", "SANDCASTLE_VERIFICATION_RESUME", "SANDCASTLE_PREFLIGHT_RESUME", "SANDCASTLE_EXTRA_QUOTA_PERCENT", "SANDCASTLE_EXTRA_BUILD_ATTEMPTS", "SANDCASTLE_EXTRA_TEST_ATTEMPTS", "SANDCASTLE_EXTRA_REVIEW_ATTEMPTS",
  "SANDCASTLE_RESUME", "SANDCASTLE_RECOVER", "SANDCASTLE_REPAIR_RESUME", "SANDCASTLE_TIMEOUT_UPGRADE_RESUME", "SANDCASTLE_CONTRACT_AMEND_RESUME",
  "SANDCASTLE_CONTRACT_AMEND_RETRY_ID", "SANDCASTLE_SEMANTIC_AMENDMENT_JSON", "SANDCASTLE_REPROVISION_WAVE", "SANDCASTLE_WORKER_CLI_UPGRADE_RESUME",
  "SANDCASTLE_EXTRA_CONTRACT_ATTEMPTS", "SANDCASTLE_GROUP_RETRY_ATTEMPTS", "SANDCASTLE_ASSEMBLY_RETRY_ATTEMPTS", "SANDCASTLE_EXTRA_ASSEMBLY_ATTEMPTS"];
const requestedModes = modeKeys.filter(key => process.env[key] !== undefined);
const booleanModes = ["SANDCASTLE_PLAN_REPAIR_RESUME", "SANDCASTLE_VERIFICATION_RESUME", "SANDCASTLE_PREFLIGHT_RESUME", "SANDCASTLE_RESUME", "SANDCASTLE_RECOVER", "SANDCASTLE_REPAIR_RESUME", "SANDCASTLE_TIMEOUT_UPGRADE_RESUME",
  "SANDCASTLE_CONTRACT_AMEND_RESUME", "SANDCASTLE_REPROVISION_WAVE", "SANDCASTLE_WORKER_CLI_UPGRADE_RESUME"];
for (const key of booleanModes) if (process.env[key] !== undefined && process.env[key] !== "1") throw new Error(`${key} must equal 1 when explicitly requested; no checkpoint was changed`);
if (requestedModes.length > 1) throw new Error("Continuation modes cannot be combined; authorize and validate one saved checkpoint operation at a time");
if (!["start", "resume"].includes(command)) throw new Error("Usage: npm run sandbox:dev (SANDCASTLE_SPEC=path) or npm run sandbox:resume -- <run-id>");
if (command === "start" && requestedModes.length) throw new Error("Continuation grants require a saved run; no allowances were changed");
loadedControllerDigest = (await controllerExecutionIdentity(root)).digest;
let state, checkpointAtRead;
if (command === "start") {
  const specPath = process.env.SANDCASTLE_SPEC || "inline task";
  if (!process.env.SANDCASTLE_TASK && (!process.env.SANDCASTLE_SPEC || isAbsolute(specPath) || specPath.includes(".."))) throw new Error("Set SANDCASTLE_SPEC to a repository-relative spec path or SANDCASTLE_TASK to task text");
  if (await git(root, "branch", "--show-current") !== targetBranch) throw new Error(`Run from ${targetBranch}`);
  if (await git(root, "status", "--porcelain")) throw new Error("Commit or stash local changes before starting Sandcastle; worktrees start from committed state");
  const spec = process.env.SANDCASTLE_TASK || await readFile(resolve(root, specPath), "utf8");
  const id = randomUUID().slice(0, 8);
  state = { id, branch: `codex/sandcastle-dev-${id}`, baseCommit: await git(root, "rev-parse", "HEAD"), specPath, spec, agentPolicy: { ...sandcastleRunAgentPolicy }, phase: "contract", reviewRound: 0, reviews: [], batchCount: 0, completedTickets: [], buildAttempts: 0, testBuildAttempts: 0, testAttempts: 0, totalBuildAttempts: 0, totalTestAttempts: 0, buildLimit: 10, testLimit: 10, reviewLimit: 10 };
  await save(state);
} else {
  if (!/^[a-f0-9]{8}$/.test(runId ?? "")) throw new Error("Pass the eight-character Sandcastle run ID");
  state = JSON.parse(await readFile(resolve(runsDir, runId, "state.json"), "utf8"));
  checkpointAtRead = JSON.stringify(state);
  if (state.phase === "done") { console.log(`Task ${runId} is already complete`); process.exit(0); }
}
const hostApprovalResumePath=process.env.SANDCASTLE_HOST_APPROVAL_RESUME_JSON;
let hostApprovalProof;
if(hostApprovalResumePath){hostApprovalProof=JSON.parse(await readFile(resolve(hostApprovalResumePath),'utf8'));await validateHostApprovalResume(root,state,hostApprovalProof,{controllerDigest:loadedControllerDigest,policyPath:process.env.SANDCASTLE_LINUX_HOST_POLICY});}
const planRepairResume = process.env.SANDCASTLE_PLAN_REPAIR_RESUME === '1';
if(planRepairResume)resumeRepairPlanCheckpoint(structuredClone(state));
const verificationResume = process.env.SANDCASTLE_VERIFICATION_RESUME === "1";
if (verificationResume) resumeVerificationMigration(structuredClone(state));
const extraQuota = process.env.SANDCASTLE_EXTRA_QUOTA_PERCENT;
const userResume = process.env.SANDCASTLE_RESUME === "1";
const preflightResume = process.env.SANDCASTLE_PREFLIGHT_RESUME === "1";
if (preflightResume) resumeExecutionCheckpoint(structuredClone(state));
if (process.env.SANDCASTLE_RECOVER === "1" && pauseStatus(state)?.fatalPreflight) throw new Error("Deterministic execution faults require correcting preflight and SANDCASTLE_PREFLIGHT_RESUME=1, not an AI recovery retry");
const improvedContractResume = process.env.SANDCASTLE_REPAIR_RESUME === "1";
const timeoutUpgradeResume = process.env.SANDCASTLE_TIMEOUT_UPGRADE_RESUME === "1";
const contractAmendResume = process.env.SANDCASTLE_CONTRACT_AMEND_RESUME === "1";
const contractAmendRetryId = process.env.SANDCASTLE_CONTRACT_AMEND_RETRY_ID;
const contractAmendRetry = contractAmendRetryId !== undefined;
const semanticAmendmentJson = process.env.SANDCASTLE_SEMANTIC_AMENDMENT_JSON;
const semanticAmendResume = semanticAmendmentJson !== undefined;
const workerProvisioningResume = process.env.SANDCASTLE_REPROVISION_WAVE === "1";
const workerRuntimeResume = process.env.SANDCASTLE_WORKER_CLI_UPGRADE_RESUME === "1";
let workerRuntimeProof;
if (workerRuntimeResume) {
  const incompatible = ["SANDCASTLE_RESUME", "SANDCASTLE_REPAIR_RESUME", "SANDCASTLE_TIMEOUT_UPGRADE_RESUME", "SANDCASTLE_CONTRACT_AMEND_RESUME",
    "SANDCASTLE_CONTRACT_AMEND_RETRY_ID", "SANDCASTLE_SEMANTIC_AMENDMENT_JSON", "SANDCASTLE_REPROVISION_WAVE", "SANDCASTLE_EXTRA_QUOTA_PERCENT",
    "SANDCASTLE_EXTRA_CONTRACT_ATTEMPTS", "SANDCASTLE_GROUP_RETRY_ATTEMPTS", "SANDCASTLE_ASSEMBLY_RETRY_ATTEMPTS", "SANDCASTLE_EXTRA_ASSEMBLY_ATTEMPTS",
    "SANDCASTLE_CONTINUE", "SANDCASTLE_RECOVER"];
  if (incompatible.some((key) => process.env[key] !== undefined)) throw new Error("Worker CLI upgrade cannot be combined with continuation modes");
  const image = "sandcastle:tomenet_modern_client";
  const { stdout: version } = await execFileAsync("docker", ["run", "--rm", "--entrypoint", "codex", image, "--version"], { timeout: 30000 });
  const { stdout: imageId } = await execFileAsync("docker", ["image", "inspect", "--format={{.Id}}", image], { timeout: 30000 });
  workerRuntimeProof = { version: version.trim().replace(/^codex-cli /, ""), imageId: imageId.trim() };
  resumeWorkerRuntimeUpgrade(structuredClone(state), workerRuntimeProof);
}
let semanticAmendment;
let semanticAmendedSpec;
if (semanticAmendResume) {
  try { semanticAmendment = JSON.parse(semanticAmendmentJson); }
  catch { throw new Error("SANDCASTLE_SEMANTIC_AMENDMENT_JSON must be valid JSON"); }
  semanticAmendedSpec = await git(root, "show", `${state.branch}:${state.specPath}`);
}
const extraContractAttempts = process.env.SANDCASTLE_EXTRA_CONTRACT_ATTEMPTS;
const extraGroupAttempts = process.env.SANDCASTLE_GROUP_RETRY_ATTEMPTS;
const extraAssemblyAttempts = process.env.SANDCASTLE_ASSEMBLY_RETRY_ATTEMPTS;
const assemblyExtension = process.env.SANDCASTLE_EXTRA_ASSEMBLY_ATTEMPTS;
const assemblyExtensionId = process.env.SANDCASTLE_ASSEMBLY_EXTENSION_ID;
if (attemptGrant) applyAllowanceGrant(structuredClone(state), attemptGrant);
let reprovisionBranches;
if (workerProvisioningResume) {
  if (userResume || improvedContractResume || timeoutUpgradeResume || contractAmendResume || contractAmendRetry || semanticAmendResume || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Worker provisioning RESUME cannot be combined with other continuation modes");
  reprovisionBranches = await workerBranchNames(state);
  resumeWorkerProvisioning(structuredClone(state), { usedBranches: reprovisionBranches });
}
if (semanticAmendResume && (userResume || improvedContractResume || timeoutUpgradeResume || contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1")) throw new Error("Semantic contract amendment cannot be combined with other continuation modes");
if (process.env.SANDCASTLE_EXTRA_CONTRACT_ROUND !== undefined) throw new Error("Contract rounds are capped at 12; extra contract-round grants are no longer available");
if (assemblyExtensionId !== undefined && assemblyExtension === undefined) throw new Error("Assembly extension ID requires an attempt count");
if (assemblyExtension !== undefined) {
  if (userResume || improvedContractResume || contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract assembly extension cannot be combined with other continuation modes");
  extendContractAssemblyAttempts(structuredClone(state), Number(assemblyExtension), assemblyExtensionId);
}
if (extraGroupAttempts !== undefined) {
  if (userResume || improvedContractResume || contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract group grant cannot be combined with other continuation modes");
  resumeContractGroupAttempts(structuredClone(state), Number(extraGroupAttempts));
}
if (extraAssemblyAttempts !== undefined) {
  if (userResume || improvedContractResume || contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract assembly grant cannot be combined with other continuation modes");
  resumeContractAssemblyAttempts(structuredClone(state), Number(extraAssemblyAttempts));
}
if (extraContractAttempts !== undefined) {
  if (userResume || improvedContractResume || contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract citation grant cannot be combined with other continuation modes");
  resumeContractCitationAttempts(structuredClone(state), Number(extraContractAttempts));
}
if (improvedContractResume) {
  if (userResume || contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract repair RESUME cannot be combined with other continuation modes");
  resumeImprovedContract(structuredClone(state));
}
if (timeoutUpgradeResume) {
  if (userResume || improvedContractResume || contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract timeout upgrade RESUME cannot be combined with other continuation modes");
  resumeContractTimeoutUpgrade(structuredClone(state));
}
if (userResume) {
  if (contractAmendResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("User pause RESUME cannot be combined with other continuation modes");
  resumeUserPause(structuredClone(state));
}
if (contractAmendResume) {
  if (userResume || improvedContractResume || timeoutUpgradeResume || contractAmendRetry || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract amend RESUME cannot be combined with other continuation modes");
  resumeExternalContractAmendment(structuredClone(state));
}
if (contractAmendRetry) {
  if (userResume || improvedContractResume || timeoutUpgradeResume || contractAmendResume || extraQuota !== undefined || extraContractAttempts !== undefined || extraGroupAttempts !== undefined || extraAssemblyAttempts !== undefined || assemblyExtension !== undefined || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Contract amend retry cannot be combined with other continuation modes");
  resumeContractAmendmentRetry(structuredClone(state), contractAmendRetryId);
}
if (semanticAmendResume) resumeSemanticContractAmendment(structuredClone(state), semanticAmendedSpec, semanticAmendment);
if (extraQuota !== undefined) {
  // Validate before acquiring the lock or changing the persisted checkpoint.
  applyAllowanceGrant(structuredClone(state), { kind: "quota", extra: Number(extraQuota) });
  if (contractAmendResume || contractAmendRetry || process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Extra quota continuation cannot be combined with CONTINUE, RECOVER or contract amendment");
}
if (state.phase === "awaiting" && !requestedModes.length) {
  const pause = pauseStatus(state);
  throw new Error(`Run ${runId} needs human confirmation. ${pause.reason} ${pause.requiredAction} Consult sandbox:status for the matching continuation mode; no allowances were changed.`);
}
state.parallelism = Number(process.env.SANDCASTLE_PARALLELISM || state.parallelism || 3);
if (!Number.isInteger(state.parallelism) || state.parallelism < 1 || state.parallelism > 10) throw new Error("SANDCASTLE_PARALLELISM must be an integer from 1 to 10");
state.reviewLimit ??= 10;
const lockPath = await acquireLock(state);
try {
  if (checkpointAtRead && JSON.stringify(JSON.parse(await readFile(resolve(runsDir, state.id, "state.json"), "utf8"))) !== checkpointAtRead) {
    throw new Error("Saved run changed before continuation lock; re-read status and retry the explicitly authorized operation");
  }
} catch (error) { await unlink(lockPath); throw error; }
let sandbox;
try {
  if (state.phase === "awaiting") {
    if(hostApprovalResumePath) {
      const verified=await validateHostApprovalResume(root,state,hostApprovalProof,{controllerDigest:loadedControllerDigest,policyPath:process.env.SANDCASTLE_LINUX_HOST_POLICY});
      await applyHostApprovalResume(root,state,verified);
    } else if(planRepairResume) {
      resumeRepairPlanCheckpoint(state);
    } else if (verificationResume) {
      resumeVerificationMigration(state);
    } else if (preflightResume) {
      resumeExecutionCheckpoint(state);
    } else if (attemptGrant) {
      applyAllowanceGrant(state, attemptGrant);
    } else if (workerRuntimeResume) {
      resumeWorkerRuntimeUpgrade(state, workerRuntimeProof);
    } else if (workerProvisioningResume) {
      resumeWorkerProvisioning(state, { usedBranches: reprovisionBranches });
    } else if (assemblyExtension !== undefined) {
      extendContractAssemblyAttempts(state, Number(assemblyExtension), assemblyExtensionId);
    } else if (extraGroupAttempts !== undefined) {
      resumeContractGroupAttempts(state, Number(extraGroupAttempts));
    } else if (extraAssemblyAttempts !== undefined) {
      resumeContractAssemblyAttempts(state, Number(extraAssemblyAttempts));
    } else if (extraContractAttempts !== undefined) {
      resumeContractCitationAttempts(state, Number(extraContractAttempts));
    } else if (improvedContractResume) {
      resumeImprovedContract(state);
    } else if (timeoutUpgradeResume) {
      resumeContractTimeoutUpgrade(state);
    } else if (userResume) {
      resumeUserPause(state);
    } else if (contractAmendResume) {
      resumeExternalContractAmendment(state);
    } else if (contractAmendRetry) {
      resumeContractAmendmentRetry(state, contractAmendRetryId);
    } else if (semanticAmendResume) {
      resumeSemanticContractAmendment(state, semanticAmendedSpec, semanticAmendment);
    } else if (extraQuota !== undefined) {
      applyAllowanceGrant(state, { kind: "quota", extra: Number(extraQuota) });
    } else {
      const phase = state.resumePhase;
      const reason = state.pauseReason;
      state.phase = phase;
      if (phase !== "recovery" || !state.recovery) beginRecovery(state, reason, phase);
    }
    if (state.phase === "awaiting") {
      state.currentFailure = classifyFailure(state.pauseReason, { phase: state.resumePhase });
      await save(state);
      await appendStageEvent(runsDir, state, { stage: "allowance", status: "paused", summary: "Authorized allowance recorded; another independent checkpoint allowance remains exhausted." });
      console.error(`Sandcastle remains paused: ${pauseStatus(state).requiredAction}`);
      return;
    }
    delete state.pauseReason;
    delete state.resumePhase;
    await save(state);
    if (workerRuntimeResume) await appendStageEvent(runsDir, state, { stage: "runtime-upgrade", status: "retry", summary: "Verified worker Codex CLI upgraded from 0.157.1 to 0.161.0 for requested GPT-6.1 Sol; saved wave, quota and exhausted recovery counters retained." });
    if (timeoutUpgradeResume) {
      const upgrade = state.timeoutUpgradeResumes.at(-1);
      await appendStageEvent(runsDir, state, { stage: "recovery", status: "retry", summary: `User-authorized contract timeout upgrade ${upgrade.oldTimeoutMs / 1000}s → ${upgrade.newTimeoutMs / 1000}s; saved group checkpoint retained.` });
    }
    if (contractAmendRetry) {
      await appendStageEvent(runsDir, state, { stage: "contract-amend", status: "retry", summary: `User-authorized bounded amendment retry ${contractAmendRetryId}; at most 3 fresh proposal/verify attempts; implementation progress retained.` });
    }
    if (semanticAmendResume) {
      await appendStageEvent(runsDir, state, { stage: "contract-amend", status: "running", summary: `User-authorized semantic amendment ${semanticAmendment.authorizationId}; previous acceptance archived and prior criteria must be preserved.` });
    }
    if (workerProvisioningResume) {
      const reprovision = state.workerProvisioningResumes.at(-1);
      await appendStageEvent(runsDir, state, { stage: "parallel-work", status: "retry", summary: `Verified stale worker branch collision; wave ${reprovision.oldWave.id} reprovisioned as wave ${reprovision.newWaveId}; ticket, base commit, quota and attempt counters retained.` });
    }
  }
  for (const path of Object.values(skillPaths)) await access(resolve(path, "SKILL.md"));
  await access(resolve(authDir, "auth.json"));
  console.log(`Sandcastle run: ${state.id}, branch: ${state.branch}, phase: ${state.phase}`);
  await appendStageEvent(runsDir, state, { stage: "workflow", status: "running", summary: `Workflow started at ${state.phase}; parallelism ${state.parallelism}.` });
  while (state.phase !== "done") {
    if (prepareAcceptanceMigration(state)) await save(state);
    const stage = state.phase;
    try {
      if (state.phase !== "recovery" && !sandbox) {
        if (command === "resume") await recoverWorktree(state);
        const inbox = resolve(runsDir, state.id, "evidence-inbox");
        await mkdir(inbox, { recursive: true, mode: 0o700 });
        sandbox = await createSandbox({ cwd: root, branch: state.branch, baseBranch: targetBranch, sandbox: sandboxProvider(inbox) });
        console.log(`Worktree: ${sandbox.worktreePath}`);
        await dispatchVerified(executionContext(state, "orchestrator", sandbox.worktreePath, {
          sandbox, allowDirty: state.phase === "assemble",
        }), { record: manifest => recordExecution(state, "workflow preflight", manifest),
          quota: () => quotaCheckpoint(state, 600000), invoke: async () => {} });
      }
      if (state.acceptance) {
        assertContract(state);
        if (sandbox) {
          const artifact = JSON.parse(await readFile(resolve(sandbox.worktreePath, taskDir(state), "acceptance-contract.json"), "utf8"));
          if (JSON.stringify(artifact.contract) !== JSON.stringify(state.acceptance.contract) || artifact.digest !== state.acceptance.contractDigest) throw new Error("Acceptance: committed contract changed; human decision required");
        }
      }
      if (state.phase === "recovery") sandbox = await recover(sandbox, state);
      else if (state.phase === "contract") await contract(sandbox, state);
      else if (state.phase === "plan") await plan(sandbox, state, state.reviewFindings ? `Fix these review findings in small tickets:\n${JSON.stringify(state.reviewFindings, null, 2)}` : "");
      else if (state.phase === "plan-validate") await acceptPlan(sandbox, state);
      else if (state.phase === "tickets") await startTicketWave(sandbox, state);
      else if (state.phase === "parallel-work") await runWorkers(state);
      else if (state.phase === "assemble") await assemble(sandbox, state);
      else if (state.phase === "build") await build(sandbox, state);
      else if (state.phase === "build-repair") await repairBuild(sandbox, state);
      else if (state.phase === "test") await test(sandbox, state);
      else if (state.phase === "test-gate") await testGate(sandbox, state);
      else if (state.phase === "test-repair") await repairTest(sandbox, state);
      else if (state.phase === "review") await review(sandbox, state);
      else if (state.phase === "final-audit") await review(sandbox, state, true);
      else if (state.phase === "report") await report(sandbox, state);
      else if (state.phase === "integrate") await integrate(sandbox, state);
      else throw new Error(`Unknown workflow phase: ${state.phase}`);
      const targetFinished = stage !== "parallel-work" ||
        (state.recoveryAdviceTarget?.ticketPaths ?? []).every((path) => state.wave?.members.find((member) => member.ticket.path === path)?.status === "completed");
      if (targetFinished && (state.recoveryAdvicePhase === stage || state.recoveryAdviceTarget?.phase === stage)) {
        delete state.recoveryAdvice;
        delete state.recoveryAdvicePhase;
        delete state.recoveryAdviceTarget;
        await save(state);
      }
      if (stage !== "recovery") {
        const outcome = stageOutcome(state, stage);
        if (["build", "test-gate", "review", "final-audit", "integrate"].includes(stage) && ["completed", "ready", "ready_with_notes"].includes(outcome.status)) clearCurrentFailure(state, { stage, verified: true });
        await save(state);
        await appendStageEvent(runsDir, state, { stage, ...outcome, tickets: state.wave?.members.map((member) => member.ticket.path) });
      }
      if (state.quotaError) throw new Error(state.quotaError);
    } catch (error) {
      if (state.phase === "recovery" || stage === "recovery") throw error;
      const category = recoveryCategory(error);
      state.currentFailure = classifyFailure(error, { phase: state.phase });
      (state.failureHistory ??= []).push({ stage, at: new Date().toISOString(), ...state.currentFailure });
      const timing = Number.isFinite(error.elapsedMs) && Number.isFinite(error.timeoutMs) ? ` after ${Math.round(error.elapsedMs / 1000)}s (limit ${Math.round(error.timeoutMs / 1000)}s)` : "";
      await appendStageEvent(runsDir, state, { stage, status: "failed", summary: `${stage}: ${category}${timing}; saved checkpoint retained.` });
      if (state.currentFailure.fatalPreflight || recoveryRestriction(state, error.message)) throw error;
      if (sandbox) {
        // Keep conflicts for the assembly agent; do not commit conflict markers.
        if (!(await git(sandbox.worktreePath, "ls-files", "-u"))) await ensureCommit(sandbox.worktreePath, `Sandcastle: preserve recovery checkpoint ${state.id}`);
      }
      beginRecovery(state, error, stage);
      await save(state);
      await appendStageEvent(runsDir, state, { stage: "recovery", status: "running", summary: `Recovery ${state.recovery.id}: diagnosing ${category} at ${stage}, attempt ${state.recovery.attempt}/3.` });
    }
  }
} catch (error) {
  if (sandbox && !classifyFailure(error).fatalPreflight) {
    try { await ensureCommit(sandbox.worktreePath, `Sandcastle: preserve interrupted task ${state.id}`); }
    catch (commitError) { console.error(`Could not preserve worktree changes: ${commitError.message}`); }
  }
  state.resumePhase = state.phase;
  state.phase = "awaiting";
  state.pauseReason = error.message;
  state.currentFailure = classifyFailure(error, { phase: state.resumePhase });
  await save(state);
  await appendStageEvent(runsDir, state, { stage: "workflow", status: "paused", summary: `Workflow stopped at ${state.resumePhase}; details preserved in state.json.` });
  const pause = pauseStatus(state);
  console.error(`Sandcastle paused [${pause.code}]: ${pause.reason}`);
  console.error(`Required action: ${pause.requiredAction}`);
  if (pause.resume.environment) console.error(`Resume authorization: ${JSON.stringify(pause.resume.environment)}; npm run sandbox:resume -- ${state.id}`);
  process.exitCode = 1;
} finally {
  try { if (sandbox) await sandbox.close(); }
  finally { await unlink(lockPath); }
}
}

if (process.argv[1] && resolve(process.argv[1]) === resolve(import.meta.filename)) await main();

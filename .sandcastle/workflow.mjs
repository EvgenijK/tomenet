import { randomUUID } from "node:crypto";
import { execFile } from "node:child_process";
import { access, mkdir, open, readFile, readdir, rename, unlink, writeFile } from "node:fs/promises";
import { homedir } from "node:os";
import { resolve, isAbsolute } from "node:path";
import { promisify } from "node:util";
import { createSandbox } from "@ai-hero/sandcastle";
import { docker } from "@ai-hero/sandcastle/sandboxes/docker";
import { readWeeklyUsage, updateBudget, extendTaskQuota } from "./limits.mjs";
import { validateTicketBatch, validateTicketCompletion } from "./tickets.mjs";
import { createWave, runWave, collectWave } from "./parallel.mjs";
import { appendStageEvent, stageOutcome } from "./events.mjs";
import { beginRecovery, recoveryRestriction, runRecovery, documentRecovery, recoveryRecord } from "./recovery.mjs";
import { baselineChecks, assertContract, prepareAcceptanceMigration, contractStage, assessmentStage, requiredChecks, recordCheck, registerRepairBatch, countIntegratedRepairs, assertAuditedChanges, acceptanceReport } from "./acceptance.mjs";
import { runtimeSchemaPath, runtimeSchemaMounts } from "./runtime.mjs";

const execFileAsync = promisify(execFile);
const root = resolve(import.meta.dirname, "..");
const authDir = resolve(root, ".sandcastle/auth");
const runsDir = resolve(root, ".sandcastle/runs");
const command = process.argv[2];
const runId = process.argv[3];
const targetBranch = "modern_interface";
const model = process.env.SANDCASTLE_MODEL || "gpt-6-sol";
const effort = process.env.SANDCASTLE_REASONING_EFFORT || "high";
const skillsRoot = resolve(process.env.SANDCASTLE_SKILLS_ROOT || resolve(homedir(), ".agents/skills"));
const skillPaths = Object.fromEntries(["to-tickets", "code-review", "tdd", "resolving-merge-conflicts"].map((name) => [name, resolve(skillsRoot, name)]));
const quote = (value) => `'${String(value).replaceAll("'", "'\\''")}'`;
let saveQueue = Promise.resolve();
let quotaQueue = Promise.resolve();
let provisionQueue = Promise.resolve();

async function git(cwd, ...args) {
  return (await execFileAsync("git", args, { cwd, maxBuffer: 4 * 1024 * 1024 })).stdout.trim();
}
async function save(state) {
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
async function quotaCheckpoint(state) {
  const next = quotaQueue.then(async () => {
    if (state.quotaError) throw new Error(state.quotaError);
    updateBudget(state, await readWeeklyUsage(authDir, model));
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
function parseAgentOutput(line, result) {
  let event;
  try { event = JSON.parse(line); } catch { return; }
  if (event.type === "item.completed" && event.item?.type === "agent_message") {
    result.message = event.item.text;
    console.log(event.item.text.slice(0, 1500));
  }
  if (event.type === "turn.completed") {
    const usage = event.usage ?? {};
    result.tokens += (usage.input_tokens || 0) + (usage.output_tokens || 0);
  }
}
async function agent(sandbox, state, role, prompt, schema = false, readOnly = false) {
  await quotaCheckpoint(state);
  const settings = [
    "--json", "--ephemeral", "--enable multi_agent",
    `-s ${readOnly ? "read-only" : "danger-full-access"}`, "-c 'approval_policy=\"never\"'",
    `-m ${quote(model)}`, `-c ${quote(`model_reasoning_effort=${JSON.stringify(effort)}`)}`,
    ...(schema ? [`--output-schema ${quote(runtimeSchemaPath(schema))}`] : []),
    "-",
  ].join(" ");
  console.log(`Agent: ${role}`);
  const captured = { message: "", tokens: 0 };
  const result = await sandbox.exec(`codex exec ${settings}`, {
    stdin: [prompt, !readOnly && state.recoveryAdvice ? `Recovery guidance: ${state.recoveryAdvice}` : ""].filter(Boolean).join("\n\n"),
    onLine: (line) => parseAgentOutput(line, captured),
  });
  state.budget.tokens += captured.tokens;
  await save(state);
  if (result.exitCode !== 0) {
    throw new Error(`${role} agent exited with status ${result.exitCode}: ${result.stderr.slice(-2000)}`);
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
async function recoveryAgent(state, context) {
  // Host read-only CLI remains available when a development container fails.
  // It may diagnose Git/tickets, but cannot modify code, state, auth or limits.
  await quotaCheckpoint(state);
  console.log(`Agent: recovery orchestrator ${state.recovery.id}`);
  const prompt = [
    "Act as the Sandcastle recovery orchestrator. Diagnose the saved incident and choose a concrete recovery action. The user authorizes autonomous recovery and deferral of absent external dependencies while independent work continues.",
    "You are read-only. Inspect AGENTS.md, the original spec, ticket files and relevant Git revisions (use git show <branch>:<path> for current integration/worker evidence). Do not edit files, commit, merge, reset, run builds or write scheduler state. Do not read credentials/auth files or print environment variables. Return only schema JSON.",
    "retry: transient execution/tool failure; rerun the saved checkpoint, preserving successful workers. repair: a failing existing worker or unfinished assembly, build/test gate or invalid planning manifest; send focused repair guidance to its existing agent/pipeline, always through worker branches and assembly for code changes. defer: blocked implementation ticket genuinely requires an absent external owner; identify its repository-relative docs/tasks dependency document and preserve all acceptance obligations. stop: recovery needs a user decision or cannot safely proceed.",
    "For defer, name only blocked/failed active tickets in ticketPaths; the controller also defers downstream dependents. Successful siblings must be integrated first. Required tests, builds and acceptance gates cannot be skipped. Do not defer a code defect just to pass a gate. Do not widen the originating task into an absent later-stage flow. Explicitly deferred callers stay pending; they must not repeatedly reappear in repair batches.",
    "For retry/repair/stop use ticketPaths=[] and dependency=''. For defer use a real docs/tasks/...md dependency; read that document to confirm the missing prerequisite. Explain the concrete cause and restoration condition in summary. Existing quota/cycle limits cannot be increased or reset. At most three recovery decisions are allowed at an unchanged checkpoint.",
    JSON.stringify(context, null, 2),
  ].join("\n\n");
  const captured = { message: "", tokens: 0 };
  const operation = execFileAsync("codex", [
    "exec", "--json", "--ephemeral", "--ignore-user-config", "--ignore-rules", "--disable", "multi_agent",
    "-s", "read-only", "-c", 'approval_policy="never"', "-m", model,
    "-c", `model_reasoning_effort=${JSON.stringify(effort)}`,
    "--output-schema", resolve(root, ".sandcastle/recovery-output.schema.json"), "-",
  ], { cwd: root, env: { ...process.env, CODEX_HOME: authDir }, timeout: 240000, killSignal: "SIGKILL", maxBuffer: 8 * 1024 * 1024 });
  operation.child.stdin.on("error", () => {});
  operation.child.stdin.end(prompt);
  let result;
  try { result = await operation; }
  catch (error) {
    for (const line of (error.stdout ?? "").split("\n")) parseAgentOutput(line, captured);
    state.budget.tokens += captured.tokens;
    await save(state);
    throw new Error(`Recovery agent failed: ${error.killed ? "timeout" : error.code}`);
  }
  for (const line of result.stdout.split("\n")) parseAgentOutput(line, captured);
  state.budget.tokens += captured.tokens;
  await save(state);
  try { await quotaCheckpoint(state); }
  catch (error) { state.quotaError = error.message; }
  await save(state);
  return JSON.parse(captured.message);
}
async function recover(sandbox, state) {
  const result = await runRecovery(state, {
    decide: (context) => recoveryAgent(state, context), save,
    document: async (incident, decision, preview) => {
      const worktree = decision.action === "defer" ? sandbox?.worktreePath ?? await recoveryWorktree(state) : undefined;
      await documentRecovery(state, incident, decision, preview, { worktree, runsDir, git, ensureCommit, manifest: manifestPath(state) });
    },
    event: (record) => appendStageEvent(runsDir, state, { stage: "recovery", status: record.action, summary: `Recovery ${record.id}: ${record.action}; checkpoint ${record.phase}; ${record.affectedTickets.length} tickets deferred.`, tickets: record.affectedTickets }),
  });
  if (!result) throw new Error(`Recovery agent requested a stop: ${state.recoveries.at(-1).summary}`);
  return sandbox;
}
function contractContext(state) {
  assertContract(state);
  return `Immutable acceptance contract (${state.acceptance.contractDigest}):\n${JSON.stringify(state.acceptance.contract, null, 2)}`;
}
async function inspect(sandbox, state, role, prompt, schema) {
  const head = await git(sandbox.worktreePath, "rev-parse", "HEAD");
  if (await git(sandbox.worktreePath, "status", "--porcelain")) throw new Error("Acceptance inspection requires a clean committed worktree");
  const response = await agent(sandbox, state, role, prompt, schema, true);
  if (await git(sandbox.worktreePath, "rev-parse", "HEAD") !== head || await git(sandbox.worktreePath, "status", "--porcelain")) throw new Error("Read-only acceptance inspection changed the worktree");
  return JSON.parse(response);
}
async function contract(sandbox, state) {
  await contractStage(state, {
    save, head: () => git(sandbox.worktreePath, "rev-parse", "HEAD"),
    sources: async () => ({ [state.specPath]: state.spec, "AGENTS.md": await readFile(resolve(sandbox.worktreePath, "AGENTS.md"), "utf8") }),
    propose: (sources, feedback) => inspect(sandbox, state, "acceptance contract", [
      "Derive a numbered, verifiable acceptance contract BEFORE implementation. Read the originating spec's referenced documents and relevant ADRs to understand it, but cite literal quotes from the supplied authoritative sources. Do not edit files. Return schema JSON.",
      "Separate implementation readiness from full native/platform acceptance exactly as the originating task allows. Use completionScope=implementation only when the task explicitly permits pending later integrations/acceptance. Mark deferred criteria pending with an explicit source-supported reason and real owning task/path; never weaken a currently required criterion. Preserve baseline protocol/game semantics and AGENTS.md isolation rules as mandatory criteria.",
      "Every criterion needs stable id, requirement, mandatory, applicability (current/deferred), source {path,quote}, owner, deferralReason (empty for current), and checkIds. Only explicitly optional polish may be nonmandatory; all originating requirements are mandatory. Add optional cosmetic/maintainability criteria if applicable, without downgrading behavior. Include source requirements that cannot yet be tested as external checks; do not conceal them.",
      "Include the exact baseline checks below. Add focused production test runners for THIS FEATURE to the fixed set (including existing feature suites and planned missing tests). Command checks may invoke only python3 -B tests/<file> [args], node tests/<file> [args], or bash tests/<file> [args], without shell operators. External checks have command=''. Current mandatory criteria need executable evidence; missing environment must block closure. The contract and fixed check set cannot be weakened in repair rounds.",
      JSON.stringify({ sources, baselineChecks, feedback }, null, 2),
    ].join("\n\n"), ".sandcastle/contract-output.schema.json"),
    verify: (proposal, sources) => inspect(sandbox, state, "independent contract verification", [
      "Independently compare the proposed acceptance contract against EVERY obligation in the originating spec and AGENTS.md. Read relevant referenced documents and ADRs. You did not create this contract. Do not edit files. Return approved=true only if all requirements, production-path checks, scope boundaries and deferred owners are accurately represented. Scoring is not a gate. List omissions/unsupported deferrals in missingRequirements. Contract criteria must be testable; mandatory behavior cannot be relabeled optional polish. Full acceptance cannot be inferred from headless checks.",
      JSON.stringify({ sources, proposal }, null, 2),
    ].join("\n\n"), ".sandcastle/contract-review-output.schema.json"),
    publish: async (accepted) => {
      await mkdir(resolve(sandbox.worktreePath, taskDir(state)), { recursive: true });
      await writeFile(resolve(sandbox.worktreePath, taskDir(state), "acceptance-contract.json"), JSON.stringify({ contract: accepted.contract, digest: accepted.contractDigest, verification: accepted.contractReview, migration: accepted.migration }, null, 2) + "\n");
      await ensureCommit(sandbox.worktreePath, "Sandcastle: accept independently verified contract");
    },
  });
}
async function plan(sandbox, state, reason = "") {
  assertContract(state);
  const skill = await readFile(resolve(skillPaths["to-tickets"], "SKILL.md"), "utf8");
  const manifest = manifestPath(state);
  const prompt = [
    "Use the following to-tickets skill to split this work into small, independently verifiable vertical tickets. The user has already approved automatic decomposition and asked for questions only at critical failures or limits; skip the skill's quiz/approval step.",
    skill,
    `Read AGENTS.md, CONTEXT.md, relevant ADRs, and docs/agents/issue-tracker.md. Write numbered ticket files under ${taskDir(state)}/issues/ and a JSON manifest at ${manifest}.`,
    'Manifest format: {"tickets":[{"path":".scratch/.../issues/01-name.md","blockedBy":[],"findingIds":[]}]}. List tickets in dependency order; blockedBy may contain earlier paths in this batch or completed tickets from prior batches. Never depend on unresolved or deferred tickets. Each ticket must have acceptance criteria referencing contract IDs. When repairing review findings, every ticket must name its ledger findingIds and every open finding must be covered once as one coherent repair (multiple dependent tickets are allowed). Do not recreate a resolved/residual finding or repair the same defect through unrelated tickets.',
    `Completed tickets available as dependencies:\n${JSON.stringify(state.completedTickets)}`,
    state.scopeNotes || "Respect the originating spec's implementation and full acceptance boundaries. Generated tickets cannot expand the authoritative scope. Record deferred checks without claiming they passed.",
    "Commit the tickets and manifest. Ask no routine questions.",
    `Originating task (${state.specPath}):\n${state.spec}`,
    contractContext(state),
    state.acceptance.assessments.at(-1) ? `Preserve verified behavior:\n${JSON.stringify(state.acceptance.assessments.at(-1).preserved)}` : "",
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
  validateTicketBatch(parsed, state);
  registerRepairBatch(state, parsed.tickets, manifest);
  for (const ticket of parsed.tickets) {
    await access(resolve(sandbox.worktreePath, ticket.path));
  }
  state.tickets = parsed.tickets;
  state.ticketIndex = 0;
  state.batchCount++;
  if (!state.repairKind) { state.buildAttempts = 0; state.testAttempts = 0; }
  delete state.repairKind;
  delete state.pendingFailure;
  delete state.recoveryAdvice;
  state.phase = "tickets";
  await save(state);
}
async function startTicketWave(sandbox, state) {
  if (state.tickets.every((ticket) => state.completedTickets.includes(ticket.path))) {
    countIntegratedRepairs(state);
    delete state.reviewFindings;
    state.phase = "build";
    await save(state);
    return;
  }
  createWave(state, { baseCommit: await git(sandbox.worktreePath, "rev-parse", "HEAD"), limit: state.parallelism });
  await save(state);
}
function sandboxProvider() {
  const proxy = process.env.all_proxy || process.env.ALL_PROXY;
  return docker({ network: "host", env: proxy ? { all_proxy: proxy } : {}, mounts: [
    ...skillMounts(), { hostPath: authDir, sandboxPath: "/home/agent/.codex", readonly: false },
    ...runtimeSchemaMounts(root),
  ] });
}
async function openWorker(state, member, wave) {
  // Git worktree registration is serialized; agent work itself is concurrent.
  const next = provisionQueue.then(async () => {
    await recoverWorktree({ id: state.id, branch: member.branch });
    return createSandbox({ cwd: root, branch: member.branch, baseBranch: wave.baseCommit, sandbox: sandboxProvider() });
  });
  provisionQueue = next.then(() => {}, () => {});
  return next;
}
async function implementWorker(worker, state, member) {
  const ticket = member.ticket;
  const content = await readFile(resolve(worker.worktreePath, ticket.path), "utf8");
  const prompt = [
    `Implement exactly ${ticket.path} in your isolated branch ${member.branch}. Other agents work in separate branches; do not edit or merge those branches.`, content,
    "Read AGENTS.md, CONTEXT.md and relevant ADRs. Claim the ticket before work and preserve its history. Keep legacy and shared edits minimal. Exercise the SV production path. Add meaningful tests for changed behavior. Commit your changes. Do not run the whole build/test/review workflow: the orchestrator runs those gates after all tickets. Ask only on critical failure.",
    state.scopeNotes || "Keep deferred acceptance checks at their originating owners; do not invent absent callers or broaden the task.",
    contractContext(state),
    `Keep accepted behavior intact: ${JSON.stringify(state.acceptance.assessments.at(-1)?.preserved ?? [])}`,
    'Return only JSON matching the output schema. status=completed requires implemented behavior, focused verification, a committed result and the ticket marked Status: resolved with an Answer. If a dependency or required environment is missing, return status=blocked with the concrete reason; never claim completion.',
  ].join("\n\n");
  const response = await agent(worker, state, `implementation ${ticket.path}`, prompt, ".sandcastle/ticket-output.schema.json");
  const result = JSON.parse(response);
  await ensureCommit(worker.worktreePath, `Sandcastle: preserve ${ticket.path}`);
  if (result.status === "completed") validateTicketCompletion(await readFile(resolve(worker.worktreePath, ticket.path), "utf8"), ticket.path);
  await appendStageEvent(runsDir, state, { stage: "ticket", status: result.status, summary: `${ticket.path}: ${result.status}; changes await assembly.`, tickets: [ticket.path] });
  return { result, head: await git(worker.worktreePath, "rev-parse", "HEAD") };
}
async function runWorkers(state) {
  await runWave(state, {
    save,
    run: async (member, wave) => {
      let worker;
      try {
        worker = await openWorker(state, member, wave);
        return await implementWorker(worker, state, member);
      } catch (error) {
        await appendStageEvent(runsDir, state, { stage: "ticket", status: "failed", summary: `${member.ticket.path}: worker failed; its branch is retained.`, tickets: [member.ticket.path] });
        throw error;
      } finally {
        if (worker) {
          try { await ensureCommit(worker.worktreePath, `Sandcastle: preserve wave ${wave.id} worker`); }
          finally { await worker.close(); }
        }
      }
    },
  });
}
async function assemble(sandbox, state) {
  try { await collectWave(state, {
    save,
    assemble: async (wave) => {
      const members = wave.members.filter((member) => member.status === "completed");
      const prompt = [
        `Assemble wave ${wave.id} on the current integration branch ${state.branch}. Read AGENTS.md, CONTEXT.md and each contributing ticket.`,
        `Common base: ${wave.baseCommit}. Merge these committed worker revisions using Git, in the listed order:\n${JSON.stringify(members.map(({ ticket, branch, head }) => ({ ticket: ticket.path, branch, head })), null, 2)}`,
        "First inspect git status and any in-progress merge. If a conflict exists, use $resolving-merge-conflicts: inspect both intents and their tickets, resolve every hunk, run focused checks, and finish the merge. Never abort, reset, discard a worker's intended behavior, or mark conflict-containing files as resolved without fixing them.",
        "For each revision use git merge-base --is-ancestor to skip revisions already integrated, otherwise git merge --no-ff the exact revision. Preserve commit ancestry for all successful workers. Do not merge failed or blocked worker branches.",
        "After merging, inspect the combined change, fix only integration errors and run appropriate focused checks. Every contributing ticket must remain resolved with its Answer. Commit all corrections and leave a clean worktree without unmerged files. Do not run the full workflow; the orchestrator runs its gates next.",
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
      validateTicketCompletion(await readFile(resolve(sandbox.worktreePath, member.ticket.path), "utf8"), member.ticket.path);
    },
  }); } finally {
    countIntegratedRepairs(state);
    await save(state);
  }
}
async function build(sandbox, state) {
  state.buildAttempts++;
  state.totalBuildAttempts++;
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
  await writeFile(resolve(sandbox.worktreePath, path), [
    `# ${number}: Verify focused production coverage`, "", "Type: implementation", "Status: open", "Assignee: unassigned", "Labels: enhancement", "",
    "Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.", "",
    "- [ ] Relevant production paths are exercised and results recorded under Answer.",
    "- [ ] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.", "",
  ].join("\n"));
  await ensureCommit(sandbox.worktreePath, "Sandcastle: plan focused test coverage");
  createWave(state, { baseCommit: await git(sandbox.worktreePath, "rev-parse", "HEAD"), tickets: [{ path, blockedBy: [] }], limit: 1, after: "test-gate" });
  await save(state);
}
async function testGate(sandbox, state) {
  state.buildAttempts++;
  state.totalBuildAttempts++;
  await save(state);
  const rebuilt = await gate(sandbox, "make -s -C src -f makefile.sv tomenet-sv");
  await checkEvidence(sandbox, state, "sv-build", rebuilt);
  if (!rebuilt.ok) {
    state.pendingFailure = rebuilt.output;
    state.phase = "build-repair";
    await save(state);
    if (state.buildAttempts >= state.buildLimit) throw new Error(`Build failed ${state.buildAttempts} times; continuation requires human approval.\n${rebuilt.output}`);
    throw new Error("SV rebuild gate failed; recovery must choose the next action");
  }
  state.testAttempts++;
  state.totalTestAttempts++;
  await save(state);
  let result;
  for (const check of requiredChecks(state.acceptance.contract).filter((item) => item.id !== "sv-build")) {
    if (check.kind === "external") throw new Error(`Acceptance: required external check ${check.id} has no runner; human decision required`);
    const script = check.kind === "core" ? await readFile(resolve(root, ".sandcastle/checks.sh"), "utf8") : undefined;
    result = await gate(sandbox, check.command, script);
    await checkEvidence(sandbox, state, check.id, result);
    if (!result.ok) { result.output = `${check.id}: ${result.output}`; break; }
  }
  if (result.ok) { state.phase = "review"; await save(state); return; }
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
  await writeFile(logPath, JSON.stringify({ head, checkId, passed: result.ok, output: result.output }, null, 2) + "\n");
  recordCheck(state, { checkId, head, passed: result.ok, logPath });
  await save(state);
}
async function review(sandbox, state, audit = false) {
  await assessmentStage(state, {
    save, head: () => git(sandbox.worktreePath, "rev-parse", "HEAD"),
    inspect: ({ head }) => inspect(sandbox, state, audit ? "fresh final acceptance auditor" : `review ${state.reviewRound}`, [
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
      // Final audit receives facts and candidate risks, not previous review verdicts.
      audit
        ? `Candidate residuals requiring independent assessment (these proposals are not an acceptance verdict):\n${JSON.stringify(state.acceptance.ledger.filter((item) => item.status === "residual").map(({ id, defectKey, area, criterionIds, file, line, problem, residual }) => ({ id, defectKey, area, criterionIds, file, line, problem, residual })), null, 2)}`
        : `Defect register:\n${JSON.stringify(state.acceptance.ledger, null, 2)}`,
      !audit && state.acceptance.migration && state.acceptance.assessments.length === 0 ? `Legacy reviews for initial identity reconciliation:\n${JSON.stringify(state.reviews.map((item) => ({ head: item.head, findings: item.findings })), null, 2)}` : "",
    ].filter(Boolean).join("\n\n"), ".sandcastle/review-output.schema.json"),
    publish: async (parsed, { head }) => {
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
  return assertAuditedChanges(state, changed);
}

export async function main() {
if (!["start", "resume"].includes(command)) throw new Error("Usage: npm run sandbox:dev (SANDCASTLE_SPEC=path) or npm run sandbox:resume -- <run-id>");
let state;
if (command === "start") {
  const specPath = process.env.SANDCASTLE_SPEC || "inline task";
  if (!process.env.SANDCASTLE_TASK && (!process.env.SANDCASTLE_SPEC || isAbsolute(specPath) || specPath.includes(".."))) throw new Error("Set SANDCASTLE_SPEC to a repository-relative spec path or SANDCASTLE_TASK to task text");
  if (await git(root, "branch", "--show-current") !== targetBranch) throw new Error(`Run from ${targetBranch}`);
  if (await git(root, "status", "--porcelain")) throw new Error("Commit or stash local changes before starting Sandcastle; worktrees start from committed state");
  const spec = process.env.SANDCASTLE_TASK || await readFile(resolve(root, specPath), "utf8");
  const id = randomUUID().slice(0, 8);
  state = { id, branch: `codex/sandcastle-dev-${id}`, baseCommit: await git(root, "rev-parse", "HEAD"), specPath, spec, phase: "contract", reviewRound: 0, reviews: [], batchCount: 0, completedTickets: [], buildAttempts: 0, testAttempts: 0, totalBuildAttempts: 0, totalTestAttempts: 0, buildLimit: 10, testLimit: 10, reviewLimit: 5 };
  await save(state);
} else {
  if (!/^[a-f0-9]{8}$/.test(runId ?? "")) throw new Error("Pass the eight-character Sandcastle run ID");
  state = JSON.parse(await readFile(resolve(runsDir, runId, "state.json"), "utf8"));
  if (state.phase === "done") { console.log(`Task ${runId} is already complete`); process.exit(0); }
}
const extraQuota = process.env.SANDCASTLE_EXTRA_QUOTA_PERCENT;
if (extraQuota !== undefined) {
  // Validate before acquiring the lock or changing the persisted checkpoint.
  extendTaskQuota(structuredClone(state), Number(extraQuota));
  if (process.env.SANDCASTLE_CONTINUE === "1" || process.env.SANDCASTLE_RECOVER === "1") throw new Error("Extra quota continuation cannot be combined with CONTINUE or RECOVER");
}
if (state.phase === "awaiting" && process.env.SANDCASTLE_CONTINUE !== "1" && process.env.SANDCASTLE_RECOVER !== "1" && extraQuota === undefined) throw new Error(`Run ${runId} needs human confirmation. Set SANDCASTLE_CONTINUE=1 to continue, or SANDCASTLE_RECOVER=1 to diagnose without resetting limits. Reason: ${state.pauseReason}`);
state.parallelism = Number(process.env.SANDCASTLE_PARALLELISM || state.parallelism || 3);
if (!Number.isInteger(state.parallelism) || state.parallelism < 1 || state.parallelism > 8) throw new Error("SANDCASTLE_PARALLELISM must be an integer from 1 to 8");
const lockPath = await acquireLock(state);
let sandbox;
try {
  if (state.phase === "awaiting") {
    if (extraQuota !== undefined) {
      extendTaskQuota(state, Number(extraQuota));
      state.phase = state.resumePhase;
    } else if (process.env.SANDCASTLE_CONTINUE !== "1") {
      const phase = state.resumePhase;
      const reason = state.pauseReason;
      state.phase = phase;
      if (phase !== "recovery" || !state.recovery) beginRecovery(state, reason, phase);
    } else {
      state.buildLimit += 10;
      state.testLimit += 10;
      state.reviewLimit += 5;
      state.recoveryAttempts = {};
      if (state.budget) {
        state.budget.consumedPercent = 0;
        state.budget.tokens = 0;
        state.budget.limitPercent = 25;
      }
      delete state.quotaError;
      state.phase = state.resumePhase;
    }
    delete state.pauseReason;
    delete state.resumePhase;
    await save(state);
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
        await quotaCheckpoint(state);
        if (command === "resume") await recoverWorktree(state);
        sandbox = await createSandbox({ cwd: root, branch: state.branch, baseBranch: targetBranch, sandbox: sandboxProvider() });
        console.log(`Worktree: ${sandbox.worktreePath}`);
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
      if (state.recoveryAdvicePhase === stage) {
        delete state.recoveryAdvice;
        delete state.recoveryAdvicePhase;
        await save(state);
      }
      if (stage !== "recovery") await appendStageEvent(runsDir, state, { stage, ...stageOutcome(state, stage), tickets: state.wave?.members.map((member) => member.ticket.path) });
      if (state.quotaError) throw new Error(state.quotaError);
    } catch (error) {
      if (state.phase === "recovery" || stage === "recovery") throw error;
      await appendStageEvent(runsDir, state, { stage, status: "failed", summary: `${stage}: stopped before completion; examining the saved checkpoint.` });
      if (recoveryRestriction(state, error.message)) throw error;
      if (sandbox) {
        // Keep conflicts for the assembly agent; do not commit conflict markers.
        if (!(await git(sandbox.worktreePath, "ls-files", "-u"))) await ensureCommit(sandbox.worktreePath, `Sandcastle: preserve recovery checkpoint ${state.id}`);
      }
      beginRecovery(state, error, stage);
      await save(state);
      await appendStageEvent(runsDir, state, { stage: "recovery", status: "running", summary: `Recovery ${state.recovery.id}: diagnosing failure at ${stage}.` });
    }
  }
} catch (error) {
  if (sandbox) {
    try { await ensureCommit(sandbox.worktreePath, `Sandcastle: preserve interrupted task ${state.id}`); }
    catch (commitError) { console.error(`Could not preserve worktree changes: ${commitError.message}`); }
  }
  state.resumePhase = state.phase;
  state.phase = "awaiting";
  state.pauseReason = error.message;
  await save(state);
  await appendStageEvent(runsDir, state, { stage: "workflow", status: "paused", summary: `Workflow stopped at ${state.resumePhase}; details preserved in state.json.` });
  console.error(`Sandcastle paused: ${error.message}`);
  console.error(`Resume with: SANDCASTLE_CONTINUE=1 npm run sandbox:resume -- ${state.id}`);
  process.exitCode = 1;
} finally {
  try { if (sandbox) await sandbox.close(); }
  finally { await unlink(lockPath); }
}
}

if (process.argv[1] && resolve(process.argv[1]) === resolve(import.meta.filename)) await main();

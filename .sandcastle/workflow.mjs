import { randomUUID } from "node:crypto";
import { execFile } from "node:child_process";
import { access, mkdir, open, readFile, rename, unlink, writeFile } from "node:fs/promises";
import { homedir } from "node:os";
import { resolve, isAbsolute } from "node:path";
import { promisify } from "node:util";
import { createSandbox } from "@ai-hero/sandcastle";
import { docker } from "@ai-hero/sandcastle/sandboxes/docker";
import { readWeeklyUsage, updateBudget } from "./limits.mjs";

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
const skillPaths = Object.fromEntries(["to-tickets", "code-review", "tdd"].map((name) => [name, resolve(skillsRoot, name)]));
const quote = (value) => `'${String(value).replaceAll("'", "'\\''")}'`;

async function git(cwd, ...args) {
  return (await execFileAsync("git", args, { cwd, maxBuffer: 4 * 1024 * 1024 })).stdout.trim();
}
async function save(state) {
  const dir = resolve(runsDir, state.id);
  await mkdir(dir, { recursive: true });
  await writeFile(resolve(dir, "state.json.tmp"), JSON.stringify(state, null, 2) + "\n");
  await rename(resolve(dir, "state.json.tmp"), resolve(dir, "state.json"));
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
  await ensureCommit(path, `Sandcastle: preserve interrupted task ${state.id}`);
  await git(root, "worktree", "remove", path);
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
async function agent(sandbox, state, role, prompt, schema = false) {
  updateBudget(state, await readWeeklyUsage(authDir, model));
  await save(state);
  const settings = [
    "--json", "--ephemeral", "--enable multi_agent",
    "-s danger-full-access", "-c 'approval_policy=\"never\"'",
    `-m ${quote(model)}`, `-c ${quote(`model_reasoning_effort=${JSON.stringify(effort)}`)}`,
    ...(schema ? ["--output-schema .sandcastle/review-output.schema.json"] : []),
    "-",
  ].join(" ");
  console.log(`Agent: ${role}`);
  const captured = { message: "", tokens: 0 };
  const result = await sandbox.exec(`codex exec ${settings}`, {
    stdin: prompt,
    onLine: (line) => parseAgentOutput(line, captured),
  });
  state.budget.tokens += captured.tokens;
  await save(state);
  if (result.exitCode !== 0) {
    throw new Error(`${role} agent exited with status ${result.exitCode}: ${result.stderr.slice(-2000)}`);
  }
  try { updateBudget(state, await readWeeklyUsage(authDir, model)); }
  catch (error) { state.quotaError = error.message; }
  await save(state);
  return captured.message;
}
async function gate(sandbox, command, input) {
  console.log(`Gate: ${command}`);
  const result = await sandbox.exec(command, { stdin: input, onLine: (line) => console.log(line) });
  return { ok: result.exitCode === 0, output: `${result.stdout}\n${result.stderr}`.slice(-12000) };
}
async function plan(sandbox, state, reason = "") {
  const skill = await readFile(resolve(skillPaths["to-tickets"], "SKILL.md"), "utf8");
  const manifest = manifestPath(state);
  const prompt = [
    "Use the following to-tickets skill to split this work into small, independently verifiable vertical tickets. The user has already approved automatic decomposition and asked for questions only at critical failures or limits; skip the skill's quiz/approval step.",
    skill,
    `Read AGENTS.md, CONTEXT.md, relevant ADRs, and docs/agents/issue-tracker.md. Write numbered ticket files under ${taskDir(state)}/issues/ and a JSON manifest at ${manifest}.`,
    'Manifest format: {"tickets":[{"path":".scratch/.../issues/01-name.md","blockedBy":[]}]}. List tickets in dependency order; blockedBy contains earlier ticket path strings. Each ticket must have acceptance criteria.',
    "Commit the tickets and manifest. Ask no routine questions.",
    reason || `Originating task (${state.specPath}):\n${state.spec}`,
  ].join("\n\n");
  await agent(sandbox, state, "ticket planning", prompt);
  await ensureCommit(sandbox.worktreePath, `Sandcastle: plan ticket batch ${state.reviewRound}`);
  const parsed = JSON.parse(await readFile(resolve(sandbox.worktreePath, manifest), "utf8"));
  if (!Array.isArray(parsed.tickets) || parsed.tickets.length < 1 || parsed.tickets.length > 30) throw new Error("Ticket manifest must contain 1–30 tickets");
  const seen = new Set();
  for (const ticket of parsed.tickets) {
    if (typeof ticket.path !== "string" || isAbsolute(ticket.path) || ticket.path.includes("..") || !ticket.path.startsWith(`${taskDir(state)}/issues/`)) throw new Error(`Invalid ticket path: ${ticket.path}`);
    if (!Array.isArray(ticket.blockedBy) || ticket.blockedBy.some((path) => !seen.has(path))) throw new Error(`Invalid blockers for ${ticket.path}`);
    await access(resolve(sandbox.worktreePath, ticket.path));
    seen.add(ticket.path);
  }
  state.tickets = parsed.tickets;
  state.ticketIndex = 0;
  state.batchCount++;
  state.phase = "tickets";
  await save(state);
}
async function implementTicket(sandbox, state) {
  const ticket = state.tickets[state.ticketIndex];
  const content = await readFile(resolve(sandbox.worktreePath, ticket.path), "utf8");
  const prompt = [
    `Implement exactly ticket ${state.ticketIndex + 1}/${state.tickets.length}: ${ticket.path}.`, content,
    "Read AGENTS.md, CONTEXT.md and relevant ADRs. Keep legacy and shared edits minimal. Exercise the SV production path. Add meaningful tests for changed behavior. Commit your changes. Do not run the whole build/test/review workflow: the orchestrator runs those gates after all tickets. Ask only on critical failure.",
  ].join("\n\n");
  await agent(sandbox, state, `implementation ${state.ticketIndex + 1}`, prompt);
  await ensureCommit(sandbox.worktreePath, `Sandcastle: complete ${ticket.path}`);
  state.completedTickets.push(ticket.path);
  state.ticketIndex++;
  if (state.ticketIndex >= state.tickets.length) { state.phase = "build"; state.buildAttempts = 0; state.testAttempts = 0; }
  await save(state);
}
async function build(sandbox, state) {
  state.buildAttempts++;
  state.totalBuildAttempts++;
  await save(state);
  const result = await gate(sandbox, "make -s -C src -f makefile.sv tomenet-sv");
  if (result.ok) { state.phase = "test"; await save(state); return; }
  state.pendingFailure = result.output;
  state.phase = "build-repair";
  await save(state);
  if (state.buildAttempts >= state.buildLimit) throw new Error(`Build failed ${state.buildAttempts} times; continuation requires human approval.\n${result.output}`);
}
async function repairBuild(sandbox, state) {
  await agent(sandbox, state, "build repair", `Fix the failed SV build on this branch. Read AGENTS.md. Build output:\n${state.pendingFailure}\nCommit the correction; the orchestrator will rebuild. Ask only on critical failure.`);
  await ensureCommit(sandbox.worktreePath, "Sandcastle: repair SV build");
  delete state.pendingFailure;
  state.phase = "build";
  await save(state);
}
async function test(sandbox, state) {
  await agent(sandbox, state, "testing", "Test the completed SV change through production paths. Inspect the tickets and add focused missing tests if needed. Run relevant focused checks. Commit any new test changes. The orchestrator runs the full headless gate next. Ask only on critical failure.");
  await ensureCommit(sandbox.worktreePath, "Sandcastle: add focused test coverage");
  state.buildAttempts++;
  state.totalBuildAttempts++;
  await save(state);
  const rebuilt = await gate(sandbox, "make -s -C src -f makefile.sv tomenet-sv");
  if (!rebuilt.ok) {
    state.pendingFailure = rebuilt.output;
    state.phase = "build-repair";
    await save(state);
    if (state.buildAttempts >= state.buildLimit) throw new Error(`Build failed ${state.buildAttempts} times; continuation requires human approval.\n${rebuilt.output}`);
    return;
  }
  state.testAttempts++;
  state.totalTestAttempts++;
  await save(state);
  const script = await readFile(resolve(root, ".sandcastle/checks.sh"), "utf8");
  const result = await gate(sandbox, "bash -s -- core", script);
  if (result.ok) { state.phase = "review"; await save(state); return; }
  state.pendingFailure = result.output;
  state.phase = "test-repair";
  await save(state);
  if (state.testAttempts >= state.testLimit) throw new Error(`Tests failed ${state.testAttempts} times; continuation requires human approval.\n${result.output}`);
}
async function repairTest(sandbox, state) {
  await agent(sandbox, state, "test repair", `Fix the failing SV tests through the production path. Read AGENTS.md. Test output:\n${state.pendingFailure}\nCommit the correction; the orchestrator will retest. Ask only on critical failure.`);
  await ensureCommit(sandbox.worktreePath, "Sandcastle: repair SV tests");
  delete state.pendingFailure;
  state.phase = "build";
  state.buildAttempts = 0;
  await save(state);
}
async function review(sandbox, state) {
  state.reviewRound++;
  await save(state);
  const prompt = [
    `$code-review Review the committed diff since ${state.baseCommit}. Run Standards and Spec axes as the skill directs, using parallel sub-agents. Read AGENTS.md.`,
    "Use the complete originating task and all generated tickets as the specification. Examine all code, tests and behavior at current HEAD. Do not change files.",
    "Return only the structured JSON requested by the output schema. Include every actionable issue in findings. Use an empty findings array only when the change is ready.",
    `Originating task:\n${state.spec}`,
  ].join("\n\n");
  const response = await agent(sandbox, state, `review ${state.reviewRound}`, prompt, true);
  const parsed = JSON.parse(response);
  const report = resolve(runsDir, state.id, `review-${state.reviewRound}.json`);
  await writeFile(report, JSON.stringify({ head: await git(sandbox.worktreePath, "rev-parse", "HEAD"), ...parsed }, null, 2) + "\n");
  state.reviews.push({ path: report, head: await git(sandbox.worktreePath, "rev-parse", "HEAD"), summary: parsed.summary, findings: parsed.findings });
  if (parsed.findings.length === 0) state.phase = "report";
  else { state.phase = "plan"; state.reviewFindings = parsed.findings; }
  await save(state);
  if (parsed.findings.length > 0 && state.reviewRound >= state.reviewLimit) throw new Error(`Review found ${parsed.findings.length} issues on round ${state.reviewRound}; continuation requires human approval. See ${report}`);
}
async function report(sandbox, state) {
  const head = await git(sandbox.worktreePath, "rev-parse", "HEAD");
  const reportPath = `docs/tasks/sandcastle/${state.id}-report.md`;
  const contents = [
    `# Sandcastle task ${state.id}`, "", `Specification: ${state.specPath}`, `Base: ${state.baseCommit}`, `Verified code HEAD: ${head}`, "",
    "## Originating request", "", state.spec, "",
    `Ticket batches: ${state.batchCount}`, `Build attempts: ${state.totalBuildAttempts}`, `Test attempts: ${state.totalTestAttempts}`, `Code review rounds: ${state.reviewRound}`, "",
    "## Completed tickets", "", ...state.completedTickets.map((path) => `- ${path}`), "",
    "## Reviews", "", ...state.reviews.flatMap((item, index) => [
      `### Round ${index + 1}`, "", `Reviewed HEAD: ${item.head}`, `Summary: ${item.summary}`, "",
      ...(item.findings.length ? item.findings.map((finding) => `- ${finding.severity}: ${finding.file}:${finding.line} — ${finding.problem} Fix: ${finding.fix}`) : ["- No findings"]), "",
    ]),
    "## Gates", "", "- SV Make build: passed", "- Sandcastle core checks: passed", "- Final code review: no findings", "",
  ].join("\n");
  await mkdir(resolve(sandbox.worktreePath, "docs/tasks/sandcastle"), { recursive: true });
  await writeFile(resolve(sandbox.worktreePath, reportPath), contents);
  await ensureCommit(sandbox.worktreePath, `Sandcastle: report task ${state.id}`);
  state.phase = "integrate";
  await save(state);
}
async function integrate(sandbox, state) {
  if (await git(root, "status", "--porcelain")) throw new Error("Main checkout has uncommitted changes; cannot fast-forward modern_interface safely");
  if (await git(root, "branch", "--show-current") !== targetBranch) throw new Error(`Main checkout must be on ${targetBranch} for integration`);
  await git(root, "merge", "--ff-only", state.branch);
  state.phase = "done";
  await save(state);
  console.log(`Task committed to ${targetBranch}: ${await git(root, "rev-parse", "HEAD")}`);
}

if (!["start", "resume"].includes(command)) throw new Error("Usage: npm run sandbox:dev (SANDCASTLE_SPEC=path) or npm run sandbox:resume -- <run-id>");
let state;
if (command === "start") {
  const specPath = process.env.SANDCASTLE_SPEC || "inline task";
  if (!process.env.SANDCASTLE_TASK && (!process.env.SANDCASTLE_SPEC || isAbsolute(specPath) || specPath.includes(".."))) throw new Error("Set SANDCASTLE_SPEC to a repository-relative spec path or SANDCASTLE_TASK to task text");
  if (await git(root, "branch", "--show-current") !== targetBranch) throw new Error(`Run from ${targetBranch}`);
  if (await git(root, "status", "--porcelain")) throw new Error("Commit or stash local changes before starting Sandcastle; worktrees start from committed state");
  const spec = process.env.SANDCASTLE_TASK || await readFile(resolve(root, specPath), "utf8");
  const id = randomUUID().slice(0, 8);
  state = { id, branch: `codex/sandcastle-dev-${id}`, baseCommit: await git(root, "rev-parse", "HEAD"), specPath, spec, phase: "plan", reviewRound: 0, reviews: [], batchCount: 0, completedTickets: [], buildAttempts: 0, testAttempts: 0, totalBuildAttempts: 0, totalTestAttempts: 0, buildLimit: 10, testLimit: 10, reviewLimit: 5 };
  await save(state);
} else {
  if (!/^[a-f0-9]{8}$/.test(runId ?? "")) throw new Error("Pass the eight-character Sandcastle run ID");
  state = JSON.parse(await readFile(resolve(runsDir, runId, "state.json"), "utf8"));
  if (state.phase === "done") { console.log(`Task ${runId} is already complete`); process.exit(0); }
  if (state.phase === "awaiting") {
    if (process.env.SANDCASTLE_CONTINUE !== "1") throw new Error(`Run ${runId} needs human confirmation. Set SANDCASTLE_CONTINUE=1 to continue, or leave it stopped. Reason: ${state.pauseReason}`);
    state.buildLimit += 10;
    state.testLimit += 10;
    state.reviewLimit += 5;
    if (state.budget) {
      state.budget.consumedPercent = 0;
      state.budget.tokens = 0;
    }
    delete state.quotaError;
    state.phase = state.resumePhase;
    delete state.pauseReason;
    delete state.resumePhase;
    await save(state);
  }
}
const proxy = process.env.all_proxy || process.env.ALL_PROXY;
const lockPath = await acquireLock(state);
let sandbox;
try {
  for (const path of Object.values(skillPaths)) await access(resolve(path, "SKILL.md"));
  await access(resolve(authDir, "auth.json"));
  console.log(`Sandcastle run: ${state.id}, branch: ${state.branch}, phase: ${state.phase}`);
  updateBudget(state, await readWeeklyUsage(authDir, model));
  await save(state);
  if (command === "resume") await recoverWorktree(state);
  sandbox = await createSandbox({
    cwd: root, branch: state.branch, baseBranch: targetBranch,
    sandbox: docker({ network: "host", env: proxy ? { all_proxy: proxy } : {}, mounts: [
      ...skillMounts(), { hostPath: authDir, sandboxPath: "/home/agent/.codex", readonly: false },
    ] }),
  });
  console.log(`Worktree: ${sandbox.worktreePath}`);
  while (state.phase !== "done") {
    if (state.phase === "plan") await plan(sandbox, state, state.reviewFindings ? `Fix these review findings in small tickets:\n${JSON.stringify(state.reviewFindings, null, 2)}` : "");
    else if (state.phase === "tickets") await implementTicket(sandbox, state);
    else if (state.phase === "build") await build(sandbox, state);
    else if (state.phase === "build-repair") await repairBuild(sandbox, state);
    else if (state.phase === "test") await test(sandbox, state);
    else if (state.phase === "test-repair") await repairTest(sandbox, state);
    else if (state.phase === "review") await review(sandbox, state);
    else if (state.phase === "report") await report(sandbox, state);
    else if (state.phase === "integrate") await integrate(sandbox, state);
    else throw new Error(`Unknown workflow phase: ${state.phase}`);
    if (state.quotaError) throw new Error(state.quotaError);
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
  console.error(`Sandcastle paused: ${error.message}`);
  console.error(`Resume with: SANDCASTLE_CONTINUE=1 npm run sandbox:resume -- ${state.id}`);
  process.exitCode = 1;
} finally {
  try { if (sandbox) await sandbox.close(); }
  finally { await unlink(lockPath); }
}

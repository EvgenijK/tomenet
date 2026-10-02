import assert from "node:assert/strict";
import test from "node:test";
import { mkdtemp, mkdir, writeFile, readFile, rm } from "node:fs/promises";
import { tmpdir } from "node:os";
import { resolve } from "node:path";
import { execFileSync } from "node:child_process";
import { baselineChecks, validateContract, acceptContract, assertContract, prepareAcceptanceMigration, recordCheck, applyAssessment, closureGate, repairFindings, registerRepairBatch, countIntegratedRepairs, contractStage, assessmentStage, acceptanceReport } from "../.sandcastle/acceptance.mjs";
import { gate, verifiedTree } from "../.sandcastle/workflow.mjs";
import { runtimeSchemaPath, runtimeSchemaMounts, readOnlyAgentArgs } from "../.sandcastle/runtime.mjs";
import { docker } from "@ai-hero/sandcastle/sandboxes/docker";
import { createWave, runWave, collectWave } from "../.sandcastle/parallel.mjs";
import { beginRecovery, recoveryRestriction, previewRecovery } from "../.sandcastle/recovery.mjs";

const head = "a".repeat(40);
const sources = { "spec.md": "Preserve cancellation. Optional spacing. Platform acceptance belongs to SV-B075." };
const proposal = () => ({ version: 1, completionScope: "implementation", checks: [...structuredClone(baselineChecks), { id: "feature", kind: "command", command: "python3 -B tests/feature_checks.py", description: "Production cancellation" }, { id: "native", kind: "external", command: "", description: "Native platform acceptance" }], criteria: [
  { id: "AC-1", requirement: "Cancel returns without changing state", source: { path: "spec.md", quote: "Preserve cancellation." }, mandatory: true, applicability: "current", checkIds: ["feature"], owner: "", deferralReason: "" },
  { id: "AC-2", requirement: "Consistent optional spacing", source: { path: "spec.md", quote: "Optional spacing." }, mandatory: false, applicability: "current", checkIds: ["sv-core"], owner: "", deferralReason: "" },
  { id: "AC-3", requirement: "Native acceptance", source: { path: "spec.md", quote: "Platform acceptance belongs to SV-B075." }, mandatory: true, applicability: "deferred", checkIds: ["native"], owner: "docs/tasks/SV-B075.md", deferralReason: "Later platform owner" },
] });
const approval = { approved: true, summary: "Every source obligation checked independently", missingRequirements: [] };
const noOp = async () => {};
function state() {
  const s = { id: "test", phase: "review", reviewRound: 0, reviewLimit: 10, completedTickets: [], tickets: [] };
  acceptContract(s, proposal(), sources, approval, head);
  for (const checkId of ["sv-build", "sv-core", "feature"]) recordCheck(s, { checkId, head, passed: true, logPath: `/tmp/${checkId}.json` });
  return s;
}
const finding = (overrides = {}) => ({ id: "", defectKey: "cancel-mutates-state", area: "Guide input cancellation", criterionIds: ["AC-1"], severity: "medium", category: "behavior", file: "src/client/guide.c", line: 10, problem: "Cancel mutates state", fix: "Restore prior state", evidence: "Production cancellation loses cursor", ...overrides });
const assessment = (findings = [], overrides = {}) => ({ summary: "Inspected production result", criteria: [{ id: "AC-1", status: "passed", evidence: "feature check at HEAD plus code inspection" }, { id: "AC-2", status: "passed", evidence: "Visual code inspection" }, { id: "AC-3", status: "pending", evidence: "Owned by SV-B075; not run" }], preserved: ["Cancellation produces no protocol packet"], findings, resolved: [], acceptedResiduals: [], ...overrides });
const residual = (id) => ({ id, reason: "Optional whitespace only", risk: "No behavior or readability impact", owner: "docs/tasks/polish.md", returnCondition: "Next visual polish pass" });

test("real Sandcastle provider accepts all runtime schema file mounts without rebuilding the image", () => {
  const mounts = runtimeSchemaMounts(resolve(import.meta.dirname, ".."));
  assert.doesNotThrow(() => docker({ mounts }));
  for (const schema of [true, ".sandcastle/contract-groups-output.schema.json", ".sandcastle/contract-output.schema.json", ".sandcastle/contract-review-output.schema.json", ".sandcastle/ticket-output.schema.json"]) {
    assert.ok(mounts.some((mount) => mount.sandboxPath === runtimeSchemaPath(schema) && mount.readonly));
  }
  assert.throws(() => docker({ mounts: [{ ...mounts[0], sandboxPath: "/opt/sandcastle-runtime/contract-output.schema.json" }] }), /outside the sandbox home directory/);
});

test("host inspection keeps the CLI read-only and enables parallel agents only for ordinary reviews", () => {
  const config = { model: "gpt-6-sol", effort: "high", schemaPath: "/repo/.sandcastle/review-output.schema.json" };
  const args = readOnlyAgentArgs(config);
  assert.equal(args[args.indexOf("-s") + 1], "read-only");
  assert.equal(args[args.indexOf("--output-schema") + 1], config.schemaPath);
  assert.ok(args.includes("--ignore-user-config") && args.includes("--ignore-rules") && args.includes("--ephemeral"));
  assert.ok(args.includes("--disable")); assert.ok(!args.includes("danger-full-access"));
  assert.ok(readOnlyAgentArgs({ ...config, multiAgent: true }).includes("--enable"));
});

test("interrupted contract proposal resumes its reserved round instead of exhausting the ceiling", async () => {
  const s = { id: "test", phase: "contract" };
  const ops = { sources: async () => sources, head: async () => head, save: noOp, publish: noOp,
    propose: async () => { throw new Error("Interrupted proposal"); }, verify: async () => ({ approved: false, summary: "Missing coverage", missingRequirements: ["Add feature evidence"] }) };
  await assert.rejects(contractStage(s, ops), /Interrupted/); assert.equal(s.contractPending.round, 1);
  await contractStage(s, { ...ops, propose: async () => proposal() });
  assert.equal(s.phase, "contract"); assert.equal(s.contractPending.round, 1);
  await contractStage(s, { ...ops, propose: async () => proposal(), verify: async () => approval });
  assert.equal(s.phase, "plan");
});

test("legacy contract grants do not lower the new twelve-round ceiling", async () => {
  const s = { id: "test", phase: "contract", contractRoundLimit: 4, contractPending: { round: 4, feedback: ["Add feature evidence"] } };
  let receivedFeedback;
  await contractStage(s, { sources: async () => sources, head: async () => head, save: noOp, publish: noOp,
    propose: async (_, feedback) => { receivedFeedback = feedback; return proposal(); }, verify: async () => approval });
  assert.deepEqual(receivedFeedback, ["Add feature evidence"]);
  assert.equal(s.phase, "plan");
});

test("contract rejects missing baseline, fabricated sources, invalid mappings and unsupported full closure", () => {
  validateContract(proposal(), sources);
  for (const mutate of [c => c.checks.shift(), c => { c.criteria[0].source.quote = "Fabricated"; }, c => { c.criteria[0].checkIds = ["missing"]; }, c => { c.completionScope = "full_acceptance"; }, c => { c.checks[0].command = "true"; }, c => { c.checks[1].kind = "command"; c.checks[1].command = "node tests/unrelated.mjs"; }, c => { c.checks[2].command = "python3 -B tests/feature_checks.py; true"; }, c => { c.checks[2].command = "python3 -B tests/../../private.py"; }]) {
    const c = proposal(); mutate(c); assert.throws(() => validateContract(c, sources));
  }
  assert.throws(() => acceptContract({}, proposal(), sources, { ...approval, missingRequirements: ["Cancellation omitted"] }, head));
});

test("contract is immutable and silent mutation cannot weaken the closure gate", () => {
  const s = state(); s.acceptance.contract.criteria[0].mandatory = false;
  assert.throws(() => assertContract(s), /changed/); assert.throws(() => closureGate(s, head), /changed/);
});

test("mandatory feature checks, final HEAD and an independent final assessment are all required", () => {
  const s = state(); applyAssessment(s, assessment(), { head });
  assert.equal(closureGate(s, head).verdict, "blocked");
  applyAssessment(s, assessment(), { head, audit: true });
  assert.equal(closureGate(s, head).verdict, "ready");
  assert.equal(closureGate(s, "b".repeat(40)).verdict, "blocked");
  recordCheck(s, { checkId: "feature", head, passed: false, logPath: "/tmp/failure.json" });
  assert.match(closureGate(s, head).reasons.join(" "), /feature/);
  assert.equal(closureGate(s, head).verdict, "blocked");
});

test("a required external current check blocks closure; headless does not establish platform acceptance", () => {
  const s = state(); s.acceptance = undefined;
  const c = proposal(); c.completionScope = "full_acceptance"; c.criteria[2].applicability = "current"; c.criteria[2].deferralReason = "";
  acceptContract(s, c, sources, approval, head);
  for (const checkId of ["sv-build", "sv-core", "feature"]) recordCheck(s, { checkId, head, passed: true, logPath: "/tmp/evidence.json" });
  const a = assessment(); a.criteria[2].status = "passed";
  assert.match(applyAssessment(s, a, { head, audit: true }).reasons.join(" "), /native/);
  const deferred = state(); const claims = assessment(); claims.criteria[2].status = "passed";
  assert.throws(() => applyAssessment(deferred, claims, { head }), /deferred/);
});

test("cross-axis duplicates and a moved/paraphrased issue share one ledger ID and strongest impact", () => {
  const s = state(); applyAssessment(s, assessment([finding(), finding({ line: 40, problem: "Escape resets the position", severity: "high" })]), { head });
  assert.equal(s.acceptance.ledger.length, 1); const id = s.acceptance.ledger[0].id;
  applyAssessment(s, assessment([finding({ id, file: "src/client/router.c", line: 100, problem: "Esc changes cursor", evidence: "Reproduced at new route" })]), { head: "b".repeat(40) });
  assert.equal(s.acceptance.ledger.length, 1); assert.equal(s.acceptance.ledger[0].id, id); assert.equal(s.acceptance.ledger[0].severity, "high");
  const before = structuredClone(s);
  assert.throws(() => applyAssessment(s, assessment([finding({ id, defectKey: "renamed-to-hide-history" })]), { head }), /identity changed/);
  assert.deepEqual(s, before);
});

test("omission does not resolve a defect; an evidenced resolution can later reopen on new evidence", () => {
  const s = state(); applyAssessment(s, assessment([finding()]), { head }); const id = s.acceptance.ledger[0].id;
  assert.equal(applyAssessment(s, assessment(), { head }).verdict, "blocked");
  applyAssessment(s, assessment([], { resolved: [{ id, evidence: "Actual production cancellation now preserves state" }] }), { head });
  assert.equal(s.acceptance.ledger[0].status, "resolved");
  applyAssessment(s, assessment([finding({ id })]), { head }); assert.equal(s.acceptance.ledger[0].status, "resolved");
  applyAssessment(s, assessment([finding({ id, evidence: "New regression: cancel while reload is in flight" })]), { head });
  assert.equal(s.acceptance.ledger[0].status, "open");
});

test("only optional low polish can close with notes and final auditor must confirm its risk", () => {
  const s = state(); const f = finding({ defectKey: "optional-spacing", criterionIds: ["AC-2"], severity: "low", category: "cosmetic" });
  const a = assessment([f], { acceptedResiduals: [residual(f.defectKey)] });
  assert.equal(applyAssessment(s, a, { head }).verdict, "ready_with_notes");
  const id = s.acceptance.ledger[0].id;
  const final = assessment([], { acceptedResiduals: [residual(id)] });
  assert.equal(applyAssessment(s, final, { head, audit: true }).verdict, "ready_with_notes");
  assert.match(s.acceptance.ledger[0].residual.backlogPath, /residuals\/D-/);
  assert.equal(applyAssessment(s, assessment(), { head, audit: true }).verdict, "blocked");
});

test("severity labels cannot waive mandatory failures, regressions, scope, security or medium defects", () => {
  for (const f of [finding({ severity: "low", category: "cosmetic" }), finding({ severity: "medium", category: "maintainability", criterionIds: ["AC-2"] }), ...["behavior", "regression", "security", "data-loss", "protocol", "scope", "evidence"].map(category => finding({ severity: "low", category, criterionIds: ["AC-2"] }))]) {
    const s = state(); const before = structuredClone(s);
    assert.throws(() => applyAssessment(s, assessment([f], { acceptedResiduals: [residual(f.defectKey)] }), { head }), /optional low/);
    assert.deepEqual(s, before);
  }
  const s = state(); const a = assessment(); a.criteria[0].status = "failed";
  assert.equal(applyAssessment(s, a, { head, audit: true }).verdict, "blocked");
});

test("three repairs count only assembled mapped commits and survive replay/new wording", async () => {
  const s = state(); applyAssessment(s, assessment([finding()]), { head }); const id = s.acceptance.ledger[0].id;
  for (let round = 1; round <= 3; round++) {
    s.reviewFindings = repairFindings(s);
    const tickets = [{ path: `.scratch/sandcastle-test/issues/${round}-a.md`, blockedBy: [], findingIds: [id] }, { path: `.scratch/sandcastle-test/issues/${round}-b.md`, blockedBy: [], findingIds: [id] }];
    registerRepairBatch(s, tickets, `batch-${round}`); s.tickets = tickets;
    createWave(s, { baseCommit: head, limit: 2 });
    const ops = { save: noOp, run: async () => ({ result: { status: "completed", summary: "Committed repair" }, head }), assemble: async () => ({ status: "completed", summary: "Merged and checked" }), verify: noOp };
    await runWave(s, ops); countIntegratedRepairs(s); assert.equal(s.acceptance.ledger[0].repairs, round - 1);
    await collectWave(s, ops); countIntegratedRepairs(s); countIntegratedRepairs(s);
    assert.equal(s.acceptance.ledger[0].repairs, round);
    applyAssessment(s, assessment([finding({ id, line: round * 20, problem: "Same defect after repair", evidence: `Reproduced round ${round}` })]), { head });
  }
  assert.throws(() => repairFindings(s), /repair limit/);
  assert.equal(s.acceptance.ledger[0].status, "open");
  assert.equal(recoveryRestriction(s, "Acceptance: defect repair limit reached"), "Acceptance gate requires a human decision; recovery cannot weaken it");
});

test("repair manifests cannot invent IDs, omit a defect or defer a review repair", () => {
  const s = state(); applyAssessment(s, assessment([finding()]), { head }); s.reviewFindings = repairFindings(s);
  const id = s.acceptance.ledger[0].id;
  for (const findingIds of [[], ["unknown"], [id, id]]) assert.throws(() => registerRepairBatch(s, [{ path: "ticket", findingIds }], "batch"));
  s.completedTickets.push("old-ticket");
  assert.throws(() => registerRepairBatch(s, [{ path: "old-ticket", findingIds: [id] }], "batch"), /already completed/);
  s.phase = "parallel-work"; s.tickets = [{ path: ".scratch/sandcastle-test/issues/1.md", blockedBy: [], findingIds: [id] }];
  s.wave = { id: 1, after: "tickets", members: [{ ticket: s.tickets[0], status: "blocked" }] };
  beginRecovery(s, "Absent environment");
  assert.throws(() => previewRecovery(s, { action: "defer", summary: "Missing prerequisite", ticketPaths: [s.tickets[0].path], dependency: "docs/tasks/owner.md" }), /cannot be deferred/);
});

test("contract phase blocks implementation until an independent check passes, with twelve attempts", async () => {
  const s = { id: "test", phase: "contract" }; let proposals = 0, reviews = 0;
  const ops = { sources: async () => sources, head: async () => head, save: noOp, publish: noOp, propose: async () => { proposals++; return proposal(); }, verify: async () => { reviews++; return reviews === 1 ? { approved: false, summary: "Missing scope detail", missingRequirements: ["Clarify platform owner"] } : approval; } };
  await contractStage(s, ops); assert.equal(s.phase, "contract"); assert.equal(s.acceptance, undefined);
  await contractStage(s, ops); assert.equal(s.phase, "plan"); assert.equal(proposals, 2); assert.equal(reviews, 2);
  const blocked = { id: "test", phase: "contract" };
  const reject = { ...ops, verify: async () => ({ approved: false, summary: "Still missing", missingRequirements: ["Mandatory requirement"] }) };
  for (let round = 0; round < 11; round++) await contractStage(blocked, reject);
  await assert.rejects(contractStage(blocked, reject), /contract verification limit/);
  assert.equal(blocked.acceptance, undefined); assert.equal(blocked.phase, "contract");
});

test("contract publication interruption replays saved decisions without another model call", async () => {
  const s = { id: "test", phase: "contract", contractReturnPhase: "assemble", contractMigration: "Legacy implementation is not retroactively accepted" }; let calls = 0;
  const ops = { sources: async () => sources, head: async () => head, save: noOp, propose: async () => { calls++; return proposal(); }, verify: async () => { calls++; return approval; }, publish: async () => { throw new Error("Disk unavailable"); } };
  await assert.rejects(contractStage(s, ops), /Disk/); assert.equal(s.acceptance, undefined);
  await contractStage(s, { ...ops, publish: noOp }); assert.equal(calls, 2); assert.equal(s.phase, "assemble"); assert.ok(s.acceptance.migration);
});

test("review/final audit phases replay publication and stop after one final repair repeat", async () => {
  const s = state(); let calls = 0;
  const ops = { head: async () => head, save: noOp, inspect: async () => { calls++; return assessment(); }, publish: async () => { throw new Error("Report disk full"); } };
  await assert.rejects(assessmentStage(s, ops), /disk full/);
  await assessmentStage(s, { ...ops, publish: noOp }); assert.equal(calls, 1); assert.equal(s.reviewRound, 1); assert.equal(s.phase, "final-audit");
  const f = finding({ evidence: "Fresh auditor discovers cancellation bug" });
  await assessmentStage(s, { ...ops, inspect: async () => assessment([f]), publish: noOp }, { audit: true }); assert.equal(s.phase, "plan");
  await assert.rejects(assessmentStage(s, { ...ops, inspect: async () => assessment([f]), publish: noOp }, { audit: true }), /final audit limit/);
  assert.equal(s.acceptance.finalAudits.length, 2); assert.equal(s.acceptance.ledger[0].status, "open"); assert.equal(s.acceptance.finalVerdict, undefined);
});

test("real repository feature evidence plus final audit yields truthful ready-with-notes report", async (t) => {
  const dir = await mkdtemp(resolve(tmpdir(), "sandcastle-acceptance-")); t.after(() => rm(dir, { recursive: true, force: true }));
  const git = (...args) => execFileSync("git", args, { cwd: dir, encoding: "utf8", stdio: ["ignore", "pipe", "pipe"] }).trim();
  git("init", "-b", "integration"); git("config", "user.name", "Acceptance test"); git("config", "user.email", "acceptance@example.invalid");
  await mkdir(resolve(dir, "tests"));
  const c = proposal(); c.checks[2].command = "node tests/feature_checks.mjs";
  await writeFile(resolve(dir, "contract.json"), JSON.stringify(c));
  await writeFile(resolve(dir, "tests/feature_checks.mjs"), `import { readFileSync } from 'node:fs';\nimport { validateContract } from ${JSON.stringify(new URL("../.sandcastle/acceptance.mjs", import.meta.url).href)};\nvalidateContract(JSON.parse(readFileSync('contract.json')), ${JSON.stringify(sources)});\n`);
  git("add", "."); git("commit", "-m", "Feature production fixture"); const revision = git("rev-parse", "HEAD");
  const s = state(); acceptContract(s, c, sources, approval, revision);
  const sandbox = { worktreePath: dir, exec: async () => ({ exitCode: 0, stdout: execFileSync(process.execPath, ["tests/feature_checks.mjs"], { cwd: dir, encoding: "utf8", stdio: ["ignore", "pipe", "pipe"] }), stderr: "" }) };
  const result = await gate(sandbox, c.checks[2].command);
  assert.equal(result.head, revision); assert.equal(result.ok, true);
  const output = result.output;
  const logPath = resolve(dir, ".git/feature.json"); await writeFile(logPath, JSON.stringify({ head: revision, passed: true, output }));
  assert.equal(JSON.parse(await readFile(logPath)).head, revision);
  for (const checkId of ["sv-build", "sv-core", "feature"]) recordCheck(s, { checkId, head: revision, passed: true, logPath });
  const f = finding({ defectKey: "optional-spacing", criterionIds: ["AC-2"], severity: "low", category: "cosmetic" });
  const ops = { save: noOp, head: async () => git("rev-parse", "HEAD"), inspect: async () => assessment([f], { acceptedResiduals: [residual(f.defectKey)] }), publish: noOp };
  await assessmentStage(s, ops); assert.equal(s.phase, "final-audit");
  await assessmentStage(s, ops, { audit: true }); assert.equal(s.phase, "report");
  const report = acceptanceReport(s); assert.match(report, /ready_with_notes/); assert.match(report, /AC-3: pending/); assert.match(report, /Residual backlog \(open, not fixed\)/); assert.doesNotMatch(report, /Final code review: no findings/);
  await mkdir(resolve(dir, "docs/tasks/sandcastle"), { recursive: true });
  await writeFile(resolve(dir, "docs/tasks/sandcastle/test-report.md"), report);
  git("add", "."); git("commit", "-m", "Publish report after audit");
  assert.equal((await verifiedTree(sandbox, s)).verdict, "ready_with_notes");
  await writeFile(resolve(dir, "contract.json"), "{}\n"); git("add", "."); git("commit", "-m", "Unverified code change");
  await assert.rejects(verifiedTree(sandbox, s), /code changed after final audit/);
  await writeFile(resolve(dir, "contract.json"), JSON.stringify(c)); git("add", "."); git("commit", "-m", "Restore audited code");
  await assert.rejects(gate({ ...sandbox, exec: async () => {
    await writeFile(resolve(dir, "contract.json"), "{}\n");
    return { exitCode: 0, stdout: "pretend success", stderr: "" };
  } }, c.checks[2].command), /modified committed work/);
});

test("legacy migration preserves a completed unmerged wave and never resumes a paused run", () => {
  const s = { id: "test", phase: "awaiting", resumePhase: "assemble", completedTickets: ["one"], budget: { consumedPercent: 35, limitPercent: 35 }, reviewRound: 13, reviews: [{ findings: [{ problem: "Old finding" }] }], wave: { members: [{ status: "completed", integrated: false, head }] } };
  const before = structuredClone(s); assert.equal(prepareAcceptanceMigration(s), false); assert.deepEqual(s, before);
  s.phase = "assemble"; const wave = structuredClone(s.wave);
  assert.equal(prepareAcceptanceMigration(s), true); assert.equal(s.phase, "contract"); assert.equal(s.contractReturnPhase, "assemble");
  assert.deepEqual(s.wave, wave); assert.deepEqual(s.budget, before.budget); assert.deepEqual(s.reviews, before.reviews); assert.equal(s.reviewRound, 13);
  for (const phase of ["report", "integrate"]) {
    const legacy = { phase }; prepareAcceptanceMigration(legacy); assert.equal(legacy.contractReturnPhase, "build");
  }
});

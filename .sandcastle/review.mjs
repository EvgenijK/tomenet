import { execFile } from "node:child_process";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import { resolve } from "node:path";
import { promisify } from "node:util";

const execFileAsync = promisify(execFile);

function shellQuote(value) {
  return `'${String(value).replaceAll("'", "'\\''")}'`;
}

async function git(worktreePath, ...args) {
  const { stdout } = await execFileAsync("git", args, { cwd: worktreePath });
  return stdout.trim();
}

export async function reviewChanges({ root, worktreePath, baseCommit, branch, sandbox, model, reasoningEffort, reviewSpecPath }) {
  const commitCount = Number(await git(worktreePath, "rev-list", "--count", `${baseCommit}..HEAD`));
  const uncommitted = await git(worktreePath, "status", "--porcelain");
  const scopes = [
    ...(commitCount > 0 ? [{ name: "branch", args: `--base ${shellQuote(baseCommit)}`, useSkill: Boolean(reviewSpecPath) }] : []),
    ...(uncommitted ? [{ name: "uncommitted", args: "--uncommitted" }] : []),
  ];

  if (scopes.length === 0) {
    console.log("Code review skipped: no changes in the Sandcastle worktree.");
    return [];
  }

  const reportsDir = resolve(root, ".sandcastle/reviews");
  await mkdir(reportsDir, { recursive: true });
  const reports = [];
  const settings = [
    `-c ${shellQuote('approval_policy="never"')}`,
    `-c ${shellQuote('sandbox_mode="danger-full-access"')}`,
    `-c ${shellQuote(`model=${JSON.stringify(model)}`)}`,
    `-c ${shellQuote(`model_reasoning_effort=${JSON.stringify(reasoningEffort)}`)}`,
  ].join(" ");

  for (const scope of scopes) {
    let result;
    if (scope.useSkill) {
      const spec = await readFile(reviewSpecPath, "utf8");
      const prompt = [
        `$code-review Review the changes since ${baseCommit}.`,
        `The originating specification was supplied from ${reviewSpecPath} on the host. Its complete content follows; use this content even if the host path is unavailable in the container.`,
        "Use it for the Spec axis, and run the Standards and Spec sub-agents in parallel as the skill requires.",
        "Report the two axes separately. Do not modify files.",
        "",
        spec,
      ].join("\n");
      console.log(`$ codex exec with $code-review (base ${baseCommit})`);
      result = await sandbox.exec(`codex exec --enable multi_agent ${settings} -`, { stdin: prompt });
    } else {
      console.log(`$ codex review ${scope.args}`);
      result = await sandbox.exec(`codex review ${settings} ${scope.args}`);
    }
    const reportPath = resolve(reportsDir, `${branch.replaceAll("/", "-")}-${scope.name}.md`);
    const report = [
      `# Sandcastle code review: ${scope.name}`,
      "",
      `Branch: ${branch}`,
      `Base commit: ${baseCommit}`,
      `Reviewer: ${scope.useSkill ? "$code-review" : "codex review"}`,
      `Exit code: ${result.exitCode}`,
      "",
      "## Review output",
      "",
      result.stdout.trim() || "(No review output on stdout.)",
      ...(result.stderr.trim() ? ["", "## CLI stderr", "", "```", result.stderr.trim(), "```"] : []),
      "",
    ].join("\n");
    await writeFile(reportPath, report);
    console.log(`Code review report: ${reportPath}`);
    reports.push(reportPath);
    if (result.exitCode !== 0) {
      throw new Error(`codex review ${scope.name} failed with status ${result.exitCode}; see ${reportPath}`);
    }
  }

  return reports;
}

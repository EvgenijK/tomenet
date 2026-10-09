import { createHash } from "node:crypto";
import { execFile } from "node:child_process";
import { readFile, readdir, realpath } from "node:fs/promises";
import { isAbsolute, relative, resolve, sep } from "node:path";
import { promisify } from "node:util";

const execFileAsync = promisify(execFile);
const sha256 = (value) => createHash("sha256").update(value).digest("hex");
const defaultHostExec = (file, args, options = {}) => execFileAsync(file, args, { timeout: 30000, maxBuffer: 1024 * 1024, ...options });
export const executionCliPolicy = Object.freeze({ minimumVersion: "0.161.0" });

// Only these stable diagnostics are persisted. Provider stderr, environment and
// credentials must never become execution provenance or recovery prompts.
export class ExecutionPreflightError extends Error {
  constructor(code, message) {
    super(`Execution preflight [${code}]: ${message}`);
    this.name = "ExecutionPreflightError";
    this.code = code;
    this.deterministic = true;
  }
}
const fail = (code, message) => { throw new ExecutionPreflightError(code, message); };
async function checked(operation, code, message) {
  try { return await operation(); }
  catch (error) { if (error instanceof ExecutionPreflightError) throw error; fail(code, message); }
}

export function verifyCliVersion(output, minimumVersion = executionCliPolicy.minimumVersion) {
  const version = /^codex-cli (\d+\.\d+\.\d+)(?:\s*)$/.exec(String(output).trim())?.[1];
  const minimum = /^(\d+)\.(\d+)\.(\d+)$/.exec(minimumVersion)?.slice(1).map(Number);
  if (!minimum) fail("CLI_POLICY_INVALID", "CLI minimum version must be a stable semantic version");
  if (!version) fail("CLI_VERSION_INVALID", "Codex did not return a stable CLI version");
  const actual = version.split(".").map(Number);
  const firstDifference = actual.findIndex((value, index) => value !== minimum[index]);
  if (firstDifference !== -1 && actual[firstDifference] < minimum[firstDifference]) {
    fail("CLI_TOO_OLD", `Codex ${version} is below required ${minimumVersion}; update the runtime before retrying`);
  }
  return version;
}

function repositoryPath(path) {
  if (typeof path !== "string" || !path || isAbsolute(path) || path.split(/[\\/]/).some((part) => !part || part === "." || part === "..")) {
    fail("PATH_INVALID", "Ticket and check paths must be repository-relative paths");
  }
  return path;
}

async function containedFile(worktree, path, code) {
  repositoryPath(path);
  return checked(async () => {
    const root = await realpath(worktree);
    const actual = await realpath(resolve(root, path));
    if (!actual.startsWith(root + sep)) fail("PATH_INVALID", "A repository input resolves outside its worktree");
    return readFile(actual);
  }, code, "Required committed input is not readable in the worktree");
}

// A reused worker branch is accepted only if it descends from the current wave
// base, contains the committed ticket and has no uncommitted changes. There is
// no checkout/reset here: failures leave the original work recoverable.
export async function preflightWorker({ worktree, expectedBranch, baseCommit, ticketPath, allowDirty = false }, ops = {}) {
  const hostExec = ops.hostExec ?? defaultHostExec;
  const git = async (...args) => (await hostExec("git", args, { cwd: worktree })).stdout.trim();
  return checked(async () => {
    if (typeof expectedBranch !== "string" || !expectedBranch || expectedBranch.startsWith("-")) fail("BRANCH_INVALID", "Expected worker branch is required");
    const branch = await git("branch", "--show-current");
    if (branch !== expectedBranch) fail("BRANCH_MISMATCH", "Worktree is not on the expected branch");
    const head = await git("rev-parse", "HEAD");
    if (baseCommit) {
      if (!/^[a-f0-9]{40,64}$/.test(baseCommit)) fail("BASE_INVALID", "Wave base must be a full committed revision");
      await checked(() => git("merge-base", "--is-ancestor", baseCommit, head), "BASE_MISMATCH", "Worker branch does not contain the current wave base; provision a fresh branch");
    }
    const dirty = Boolean(await git("status", "--porcelain"));
    if (dirty && !allowDirty) fail("WORKTREE_DIRTY", "Commit or preserve worktree changes before dispatch");
    let ticketDigest;
    if (ticketPath) {
      const contents = await containedFile(worktree, ticketPath, "TICKET_UNREADABLE");
      const blob = await checked(() => hostExec("git", ["show", `${head}:${ticketPath}`], { cwd: worktree, encoding: "buffer" }), "TICKET_NOT_COMMITTED", "Worker HEAD does not contain its ticket; provision a fresh branch");
      if (!Buffer.from(blob.stdout).equals(contents)) fail("TICKET_CHANGED", "Worker ticket differs from its committed revision");
      ticketDigest = sha256(contents);
    }
    return { branch, head, dirty, ...(baseCommit ? { baseCommit } : {}), ...(ticketPath ? { ticketPath, ticketDigest } : {}) };
  }, "GIT_UNAVAILABLE", "Cannot verify the execution worktree with Git");
}

export async function controllerExecutionIdentity(root) {
  const paths = (await readdir(resolve(root, ".sandcastle"))).filter((name) => /\.(?:mjs|json|sh)$/.test(name) || name === "Dockerfile").sort();
  const files = [];
  for (const name of paths) {
    const path = `.sandcastle/${name}`;
    files.push({ path, digest: sha256(await readFile(resolve(root, path))) });
  }
  for (const path of ["package.json", "package-lock.json"]) {
    files.push({ path, digest: sha256(await readFile(resolve(root, path))) });
  }
  return { files, digest: sha256(JSON.stringify(files)) };
}

// This command only reads local files/Git metadata and prints identities. It
// deliberately does not use codex exec, login, models or any inference endpoint.
function sandboxProbe(input) {
  return `const fs = require('node:fs');
const cp = require('node:child_process');
const crypto = require('node:crypto');
const input = ${JSON.stringify(input)};
const hash = value => crypto.createHash('sha256').update(value).digest('hex');
const run = (file,args) => { const r=cp.spawnSync(file,args,{encoding:'utf8'}); if(r.status!==0) throw Error('probe'); return r.stdout.trim(); };
try {
  const output = { branch: run('git',['branch','--show-current']), head: run('git',['rev-parse','HEAD']), hostname: require('node:os').hostname(), cli: run('codex',['--version']) };
  // Host networking can inherit the host's UTS hostname. Docker's hostname
  // bind mount still identifies the actual container independently of UTS.
  if(fs.existsSync('/proc/self/mountinfo')) {
    const mount = fs.readFileSync('/proc/self/mountinfo','utf8').split('\\n').map(line => line.split(' ')).find(fields => fields[4] === '/etc/hostname');
    output.containerId = mount?.[3]?.match(new RegExp('/containers/([a-f0-9]{64})/hostname$'))?.[1];
  }
  if(input.ticketPath) output.ticketDigest = hash(fs.readFileSync(input.ticketPath));
  if(input.schemaPath) { const content = fs.readFileSync(input.schemaPath); JSON.parse(content); output.schemaDigest = hash(content); }
  if(input.checksPath) output.checksDigest = hash(fs.readFileSync(input.checksPath));
  console.log(JSON.stringify(output));
} catch { process.exitCode=1; }
`;
}

async function dockerProvenance(hostname, hostExec) {
  if (!/^[a-f0-9]{12,64}$/.test(hostname ?? "")) fail("IMAGE_UNVERIFIABLE", "Sandbox does not identify its running Docker container");
  const imageId = (await hostExec("docker", ["inspect", "--format={{.Image}}", hostname])).stdout.trim();
  if (!/^sha256:[a-f0-9]{64}$/.test(imageId)) fail("IMAGE_UNVERIFIABLE", "Running container image identity is not verifiable");
  const repoDigests = JSON.parse((await hostExec("docker", ["image", "inspect", "--format={{json .RepoDigests}}", imageId])).stdout);
  return { imageId, repoDigests: Array.isArray(repoDigests) ? repoDigests.filter((value) => typeof value === "string") : [] };
}

// Call before each inference dispatch; no stale success cache is used. The
// manifest digest permits grouping identical controller/runtime identities in
// durable run history, while every dispatch freshly verifies HEAD and mounts.
export async function preflightExecution({ controllerRoot, worktree, expectedBranch, baseCommit, ticketPath,
  schemaPath, checksPath = ".sandcastle/checks.sh", contractDigest, model, effort, sandbox,
  minimumCliVersion = executionCliPolicy.minimumVersion, allowDirty = false, expectedControllerDigest }, ops = {}) {
  const hostExec = ops.hostExec ?? defaultHostExec;
  return checked(async () => {
    if (model !== "gpt-6.1-sol" || !["medium", "high"].includes(effort)) fail("AGENT_POLICY_INVALID", "Dispatch requires the declared GPT-6.1 Sol model and medium/high effort");
    if (contractDigest !== undefined && !/^[a-f0-9]{64}$/.test(contractDigest)) fail("CONTRACT_DIGEST_INVALID", "Contract identity must be a SHA-256 digest");
    const git = await preflightWorker({ worktree, expectedBranch, baseCommit, ticketPath, allowDirty }, ops);
    const controller = await controllerExecutionIdentity(controllerRoot);
    if (expectedControllerDigest && controller.digest !== expectedControllerDigest) fail("CONTROLLER_CHANGED", "Controller files changed since startup; restart the controller before dispatch");
    const hostVersion = verifyCliVersion((await checked(() => hostExec("codex", ["--version"]), "HOST_CLI_UNAVAILABLE", "Host Codex executable is not available")).stdout, minimumCliVersion);
    const checksDigest = sha256(await containedFile(worktree, checksPath, "CHECKS_UNREADABLE"));
    let schemaDigest;
    if (schemaPath) {
      const content = await checked(() => readFile(schemaPath), "SCHEMA_UNREADABLE", "Dispatch schema is not readable");
      await checked(() => JSON.parse(content), "SCHEMA_INVALID", "Dispatch schema is not valid JSON");
      schemaDigest = sha256(content);
    }
    let runtime = { kind: "host", cliVersion: hostVersion };
    if (sandbox) {
      const result = await checked(() => sandbox.exec("node -", { stdin: sandboxProbe({ ticketPath, schemaPath: ops.sandboxSchemaPath ?? schemaPath, checksPath }) }), "SANDBOX_PROBE_FAILED", "No-inference sandbox probe could not execute");
      if (result.exitCode !== 0) fail("SANDBOX_INPUT_UNREADABLE", "Sandbox cannot read its Git identity, ticket, schema or checks, or execute Codex");
      const probe = await checked(() => JSON.parse(result.stdout), "SANDBOX_PROBE_INVALID", "Sandbox probe did not return valid execution metadata");
      if (probe.branch !== git.branch || probe.head !== git.head) fail("SANDBOX_GIT_MISMATCH", "Mounted sandbox worktree is not the validated host branch and HEAD");
      if (ticketPath && probe.ticketDigest !== git.ticketDigest) fail("SANDBOX_TICKET_MISMATCH", "Mounted sandbox ticket differs from the committed worker ticket");
      if (schemaDigest && probe.schemaDigest !== schemaDigest) fail("SANDBOX_SCHEMA_MISMATCH", "Mounted dispatch schema differs from the current controller schema");
      if (probe.checksDigest !== checksDigest) fail("SANDBOX_CHECKS_MISMATCH", "Mounted checks differ from the validated worktree checks");
      const cliVersion = verifyCliVersion(probe.cli, minimumCliVersion);
      const image = await checked(() => ops.imageProvenance ? ops.imageProvenance(probe) : dockerProvenance(probe.containerId ?? probe.hostname, hostExec), "IMAGE_UNVERIFIABLE", "Cannot inspect the actual running container image");
      if (!/^sha256:[a-f0-9]{64}$/.test(image?.imageId ?? "")) fail("IMAGE_UNVERIFIABLE", "Actual sandbox image must have a content-addressed identity");
      runtime = { kind: "sandbox", cliVersion, imageId: image.imageId, repoDigests: image.repoDigests ?? [] };
    }
    const identity = { version: 1, controller, git, contractDigest: contractDigest ?? null,
      checks: { path: checksPath, digest: checksDigest }, ...(schemaDigest ? { schema: { path: relative(controllerRoot, schemaPath), digest: schemaDigest } } : {}),
      host: { cliVersion: hostVersion }, runtime, agent: { model, effort }, cliPolicy: { minimumVersion: minimumCliVersion } };
    return { ...identity, digest: sha256(JSON.stringify(identity)) };
  }, "EXECUTION_INPUT_UNREADABLE", "Cannot read required controller or execution inputs");
}

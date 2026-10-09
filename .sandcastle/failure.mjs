// Status is a public channel: classify private diagnostics, never echo them.
const definitions = {
  quota: ["quota", "Task quota allowance is exhausted.", "Grant a specific additional quota allowance to continue."],
  account_usage: ["quota", "Account usage is unavailable or limited.", "Restore account usage or wait for the account window to reset."],
  user_pause: ["user", "Workflow is paused by the user.", "Resume the saved checkpoint when ready."],
  model: ["model", "The selected model or reasoning effort is incompatible with the worker runtime.", "Repair and verify the worker CLI, image and model policy before resuming."],
  startup: ["startup", "Worker startup or execution preflight failed.", "Repair the execution environment and verify preflight before resuming."],
  schema: ["schema", "An agent response or output schema failed validation.", "Inspect the saved validation artifact and repair the response or schema at this checkpoint."],
  contract: ["contract", "The acceptance contract requires a correction or decision.", "Inspect the saved contract finding; apply a validated amendment or a specific attempt grant."],
  test: ["test", "A build or test gate failed or exhausted its attempts.", "Repair the failing gate; an exhausted limit requires a specific attempt grant."],
  git: ["git", "Git integration or worker ownership prevents progress.", "Preserve local changes and resolve the saved Git or worktree condition before resuming."],
  attempts: ["attempts", "The saved checkpoint exhausted its attempt allowance.", "Inspect the checkpoint and grant a bounded number of attempts for that operation."],
  other: ["other", "Workflow requires attention at the saved checkpoint.", "Inspect the private checkpoint artifacts to determine the required repair or decision."],
};

// Keep these identifiers aligned with execution.mjs. Messages and actions are
// controller-owned constants, never provider output or checkpoint content.
const preflightDefinitions = {
  CLI_POLICY_INVALID: ["startup", "The minimum Codex CLI policy is invalid.", "Set a valid minimum CLI version and verify preflight."],
  CLI_VERSION_INVALID: ["startup", "Codex did not return a supported CLI version format.", "Repair the Codex executable and verify its version before resuming."],
  CLI_TOO_OLD: ["startup", "Codex is older than the minimum execution policy permits.", "Upgrade the worker runtime CLI and verify its version before resuming."],
  HOST_CLI_UNAVAILABLE: ["startup", "The host Codex executable is unavailable.", "Restore the host Codex installation and verify preflight."],
  PATH_INVALID: ["startup", "A required execution input is outside the repository boundary.", "Correct the repository-relative input path and verify preflight."],
  BRANCH_INVALID: ["git", "The expected worker branch is invalid.", "Correct the saved worker branch identity and verify preflight."],
  BRANCH_MISMATCH: ["git", "The worker worktree is on a different branch.", "Restore the registered worker branch without discarding changes, then verify preflight."],
  BASE_INVALID: ["git", "The wave base is not a valid committed revision.", "Restore the saved wave base identity and verify preflight."],
  BASE_MISMATCH: ["git", "The worker branch does not contain the current wave base.", "Provision a fresh registered worker from the saved wave base and verify preflight."],
  WORKTREE_DIRTY: ["git", "The execution worktree contains uncommitted changes.", "Commit or safely preserve those changes before dispatch, then verify preflight."],
  TICKET_UNREADABLE: ["git", "The worker ticket cannot be read from its worktree.", "Restore the committed ticket in a registered worker and verify preflight."],
  TICKET_NOT_COMMITTED: ["git", "The worker HEAD does not contain its ticket.", "Provision a fresh registered worker containing the committed ticket."],
  TICKET_CHANGED: ["git", "The worker ticket differs from its committed version.", "Preserve and reconcile the ticket changes before dispatch, then verify preflight."],
  GIT_UNAVAILABLE: ["git", "Git cannot verify the execution worktree.", "Restore Git access to the registered worktree and verify preflight."],
  AGENT_POLICY_INVALID: ["model", "The model or reasoning effort does not match the execution policy.", "Correct the saved dispatch policy and verify the worker runtime."],
  CONTRACT_DIGEST_INVALID: ["contract", "The accepted contract identity is not a valid SHA-256 digest.", "Restore the accepted contract identity through validated contract handling."],
  CHECKS_UNREADABLE: ["startup", "The repository test runner cannot be read from the execution worktree.", "Restore the committed runner at the checked HEAD and verify preflight."],
  SCHEMA_UNREADABLE: ["schema", "The dispatch output schema cannot be read.", "Restore the current controller schema and its mount, then verify preflight."],
  SCHEMA_INVALID: ["schema", "The dispatch output schema is not valid JSON.", "Repair the controller schema and verify preflight before any agent dispatch."],
  SANDBOX_PROBE_FAILED: ["startup", "The sandbox execution probe failed to start.", "Repair the sandbox launcher and verify the no-inference probe."],
  SANDBOX_INPUT_UNREADABLE: ["startup", "The sandbox cannot read required inputs or execute Codex.", "Repair sandbox mounts and the CLI installation, then verify the no-inference probe."],
  SANDBOX_PROBE_INVALID: ["startup", "The sandbox execution probe returned invalid metadata.", "Repair the probe runtime and verify its structured output."],
  SANDBOX_GIT_MISMATCH: ["git", "The sandbox branch or HEAD differs from the validated host worktree.", "Mount the registered worker worktree and verify branch and HEAD in both environments."],
  SANDBOX_TICKET_MISMATCH: ["git", "The mounted ticket differs from the committed worker ticket.", "Correct the sandbox ticket mount and verify its digest before dispatch."],
  SANDBOX_SCHEMA_MISMATCH: ["schema", "The mounted output schema differs from the controller schema.", "Correct the schema mount and verify its digest before dispatch."],
  SANDBOX_CHECKS_MISMATCH: ["startup", "The mounted test runner differs from the execution worktree runner.", "Correct the runner mount and verify its digest at the checked HEAD."],
  IMAGE_UNVERIFIABLE: ["startup", "The running sandbox image identity cannot be verified.", "Restore container inspection access and verify the actual content-addressed image."],
  EXECUTION_INPUT_UNREADABLE: ["startup", "Required controller or execution inputs cannot be read.", "Restore access to the execution inputs and verify preflight."],
  CONTROLLER_CHANGED: ["startup", "Controller files changed after this workflow process started.", "Restart the controller with the intended files, then verify execution preflight."],
};

export function classifyFailure(error, { phase } = {}) {
  const message = String(error?.message ?? error ?? "");
  const code = String(error?.code ?? /Execution preflight \[([A-Z_]+)\]/.exec(message)?.[1] ?? "");
  if (preflightDefinitions[code]) {
    const [category, reason, requiredAction] = preflightDefinitions[code];
    return { code: category, category, diagnosticCode: code, reason, requiredAction, fatalPreflight: true, automaticRetry: false };
  }
  let kind = "other";
  if (/^Task reached .*quota|^Task reached its token limit|task quota.*(?:guard|exhaust|allowance|limit)/i.test(message)) kind = "quota";
  else if (/weekly.*quota.*unavailable|account usage|usage probe|usage read|rate limit/i.test(message)) kind = "account_usage";
  else if (message === "Paused by user") kind = "user_pause";
  else if (/model.*(?:not found|not supported|unsupported|unavailable|incompatible)|unsupported.*model|reasoning.*(?:unsupported|not supported)|worker.*CLI.*upgrade/i.test(message) || ["MODEL_INCOMPATIBLE", "AGENT_POLICY_INVALID"].includes(code)) kind = "model";
  else if (/schema mount|processFileMountParents|file mounts outside|schema.*(?:invalid|validation|malformed)|(?:invalid|malformed).*schema|JSON.*(?:parse|invalid)|response.*(?:parse|validat)/i.test(message) || ["SCHEMA_INVALID", "SCHEMA_UNREADABLE", "SANDBOX_SCHEMA_MISMATCH"].includes(code)) kind = "schema";
  else if (["BRANCH_INVALID", "BRANCH_MISMATCH", "BASE_INVALID", "BASE_MISMATCH", "WORKTREE_DIRTY", "GIT_UNAVAILABLE", "SANDBOX_GIT_MISMATCH", "TICKET_NOT_COMMITTED", "TICKET_CHANGED"].includes(code)) kind = "git";
  else if (/preflight|startup|spawn .*ENOENT|docker|image.*(?:digest|mismatch)|controller.*(?:skew|mismatch)|runtime.*(?:mismatch|incompatible)|agent.*(?:HTTP 400|authentication|unauthorized)|authentication|login required/i.test(message) || ["PREFLIGHT_FAILED", "ENOENT", "AUTH_REQUIRED"].includes(code)) kind = "startup";
  else if (/dirty|clean.*(?:checkout|working tree)|worktree|branch.*(?:collision|occupied)|merge conflict|unmerged|git.*(?:fail|conflict)|integration.*(?:ref|branch)/i.test(message)) kind = "git";
  else if (/acceptance:|contract|criterion|criteria|semantic amendment/i.test(message)) kind = "contract";
  else if (/build.*(?:fail|limit|exhaust)|test.*(?:fail|limit|exhaust)|gate.*fail/i.test(message) || ["build-repair", "test-repair"].includes(phase)) kind = "test";
  else if (/attempt.*(?:exhaust|limit)|recovery made no progress|review round limit|cycle.*(?:limit|exhaust)/i.test(message)) kind = "attempts";
  const [category, reason, requiredAction] = definitions[kind];
  const fatalPreflight = error?.deterministic === true || /Execution preflight \[/.test(message) || ["model", "startup"].includes(kind) || /schema mount|processFileMountParents|file mounts outside/i.test(message);
  const diagnosticCode = ["CLI_TOO_OLD", "CLI_VERSION_INVALID", "HOST_CLI_UNAVAILABLE", "SCHEMA_INVALID", "SCHEMA_UNREADABLE", "SANDBOX_SCHEMA_MISMATCH", "BRANCH_MISMATCH", "BASE_MISMATCH", "WORKTREE_DIRTY", "SANDBOX_GIT_MISMATCH", "TICKET_NOT_COMMITTED", "TICKET_CHANGED", "AGENT_POLICY_INVALID", "IMAGE_UNVERIFIABLE", "SANDBOX_PROBE_FAILED", "CHECKS_UNREADABLE", "SANDBOX_CHECKS_MISMATCH", "AUTH_REQUIRED"].includes(code) ? code : undefined;
  return { code: kind, category, reason, requiredAction, ...(diagnosticCode ? { diagnosticCode } : {}), fatalPreflight, automaticRetry: !fatalPreflight && !["quota", "account_usage", "user_pause", "attempts", "contract", "git"].includes(kind) };
}

export function pauseStatus(state) {
  if (state.phase !== "awaiting") return undefined;
  const saved = state.currentFailure;
  const guardPolicy = {
    quota: { code: "quota", reason: "Task quota allowance is exhausted.", environment: { SANDCASTLE_EXTRA_QUOTA_PERCENT: "<explicit-positive-allowance>" } },
    build: { code: "test", reason: "Build attempt allowance is exhausted.", environment: { SANDCASTLE_EXTRA_BUILD_ATTEMPTS: "<explicit-positive-attempts>" } },
    test: { code: "test", reason: "Test attempt allowance is exhausted.", environment: { SANDCASTLE_EXTRA_TEST_ATTEMPTS: "<explicit-positive-attempts>" } },
    review: { code: "attempts", reason: "Review attempt allowance is exhausted.", environment: { SANDCASTLE_EXTRA_REVIEW_ATTEMPTS: "<explicit-positive-attempts>" } },
  };
  const remainingGuards = (state.allowanceContinuation?.remainingGuards ?? []).map((guard) => {
    const policy = guardPolicy[guard.kind];
    return policy ? { kind: guard.kind, reason: policy.reason, requiredAction: "Authorize this specific allowance separately; each grant preserves the other guards.", environment: policy.environment } : { kind: "other", reason: definitions.other[1], requiredAction: definitions.other[2] };
  });
  let failure;
  // Rebuild public strings from stable identifiers rather than trusting saved
  // reason/action text. New checkpoints retain exact typed preflight guidance;
  // legacy checkpoints still classify their private reason without echoing it.
  if (remainingGuards.length) {
    const policy = guardPolicy[remainingGuards[0].kind];
    const code = policy?.code ?? "other";
    const [category, reason, requiredAction] = definitions[code];
    failure = { code, category, reason: policy?.reason ?? reason, requiredAction, fatalPreflight: false, automaticRetry: false };
  } else if (state.quotaError) failure = classifyFailure(state.quotaError, { phase: state.resumePhase });
  else if (saved && preflightDefinitions[saved.diagnosticCode]) failure = classifyFailure({ code: saved.diagnosticCode });
  else if (saved && definitions[saved.code]) {
    const [category, reason, requiredAction] = definitions[saved.code];
    failure = { code: saved.code, category, reason, requiredAction, fatalPreflight: saved.fatalPreflight === true, automaticRetry: saved.automaticRetry === true && saved.fatalPreflight !== true };
  } else failure = classifyFailure(state.pauseReason || state.recovery?.error, { phase: state.resumePhase });
  const resume = { checkpoint: safePhase(state.resumePhase), requiresVerification: !["quota", "user_pause"].includes(failure.code), requiresAuthorization: ["quota", "attempts", "contract"].includes(failure.code) };
  if (failure.code === "quota" && !/token limit/i.test(state.quotaError || state.pauseReason || "")) resume.environment = { SANDCASTLE_EXTRA_QUOTA_PERCENT: "<explicit-positive-allowance>" };
  if (failure.code === "user_pause" && state.userRequestedPause === true) resume.environment = { SANDCASTLE_RESUME: "1" };
  const reason = state.pauseReason ?? "";
  if (/^(?:Build failed \d+ times; continuation requires human approval|Test-gate rebuild failed \d+ times; continuation requires human approval|Build attempt limit reached|Test-gate rebuild attempt limit reached)/i.test(reason)) resume.environment = { SANDCASTLE_EXTRA_BUILD_ATTEMPTS: "<explicit-positive-attempts>" };
  if (/^(?:Tests failed \d+ times; continuation requires human approval|Test attempt limit reached)/i.test(reason)) resume.environment = { SANDCASTLE_EXTRA_TEST_ATTEMPTS: "<explicit-positive-attempts>" };
  if (/^(?:Acceptance: )?review round limit reached; human approval required/i.test(reason)) resume.environment = { SANDCASTLE_EXTRA_REVIEW_ATTEMPTS: "<explicit-positive-attempts>" };
  if (/^review attempt limit reached/i.test(reason)) resume.environment = { SANDCASTLE_EXTRA_REVIEW_ATTEMPTS: "<explicit-positive-attempts>" };
  if (resume.environment && failure.code !== "user_pause") resume.requiresAuthorization = true;
  if (failure.fatalPreflight) {
    resume.environment = { SANDCASTLE_PREFLIGHT_RESUME: "1" };
    resume.requiresVerification = true;
    resume.requiresAuthorization = false;
  }
  if (remainingGuards.length) {
    if (remainingGuards[0].environment) resume.environment = remainingGuards[0].environment;
    else delete resume.environment;
    resume.requiresAuthorization = true;
  }
  return { ...failure, resume, ...(remainingGuards.length ? { remainingGuards } : {}) };
}

function safePhase(value) { return typeof value === "string" && /^[a-z][a-z-]{0,40}$/.test(value) ? value : undefined; }

// The controller calls this only after verified progress. Historical artifacts
// and recovery ledgers remain intact; terminal status must not show old output
// as a current failure.
export function clearCurrentFailure(state, { stage, verified = false } = {}) {
  if (!verified || !safePhase(stage)) throw new Error("Clearing current failure requires a verified stage outcome");
  delete state.pendingFailure;
  delete state.currentFailure;
  if (state.phase !== "awaiting") delete state.pauseReason;
  return state;
}

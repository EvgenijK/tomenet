import { spawn } from "node:child_process";

export function taskQuotaLimit(state) {
  const limit = state.budget?.limitPercent ?? 25;
  if (!Number.isFinite(limit) || limit <= 0) throw new Error("Invalid task quota allowance");
  return limit;
}

// An explicit resume of a user pause grants no extra budget or repair cycles.
export function resumeUserPause(state) {
  if (state.phase !== "awaiting" || state.userRequestedPause !== true || state.pauseReason !== "Paused by user" || !state.resumePhase) throw new Error("RESUME requires a saved user-requested pause");
  if (state.quotaError || (state.budget?.consumedPercent ?? 0) >= taskQuotaLimit(state)) throw new Error("Task quota guard still requires an explicit additional allowance");
  state.phase = state.resumePhase;
  delete state.resumePhase;
  delete state.pauseReason;
  delete state.userRequestedPause;
  return state;
}

// Only the explicit continuation CLI calls this; recovery never grants quota.
export function extendTaskQuota(state, extraPercent) {
  if (!Number.isFinite(extraPercent) || extraPercent <= 0 || extraPercent > 100) throw new Error("SANDCASTLE_EXTRA_QUOTA_PERCENT must be greater than 0 and at most 100");
  if (state.phase !== "awaiting" || !state.resumePhase || !state.budget || !/^Task reached [\d.]+% of the weekly Codex quota$/.test(state.quotaError ?? state.pauseReason ?? "")) throw new Error("Extra quota continuation requires a task quota pause");
  const previousLimit = taskQuotaLimit(state);
  const limitPercent = Math.max(previousLimit, state.budget.consumedPercent) + extraPercent;
  (state.budget.allowanceHistory ??= []).push({ extraPercent, previousLimit, consumedPercent: state.budget.consumedPercent, limitPercent, grantedAt: new Date().toISOString() });
  state.budget.limitPercent = limitPercent;
  delete state.quotaError;
}

// Read the same account quota snapshot used by the Codex app. No credential is
// copied into a report or sent to the development container by this probe.
export async function readWeeklyUsage(authDir, model, timeoutMs = 20000) {
  const child = spawn("codex", ["app-server", "--stdio"], {
    env: { ...process.env, CODEX_HOME: authDir },
    stdio: ["pipe", "pipe", "ignore"],
  });
  let buffer = "";
  let initialized = false;
  const result = await new Promise((resolve, reject) => {
    let settled = false;
    const timer = setTimeout(() => finish(new Error("Codex usage probe timed out")), timeoutMs);
    const finish = (error, value) => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      child.stdin.destroy();
      child.stdout.destroy();
      child.kill("SIGKILL");
      error ? reject(error) : resolve(value);
    };
    child.on("error", (error) => finish(error));
    child.on("exit", (code) => finish(new Error(`Codex usage probe exited: ${code}`)));
    child.stdout.on("data", (chunk) => {
      buffer += chunk;
      let newline;
      while ((newline = buffer.indexOf("\n")) >= 0) {
        const line = buffer.slice(0, newline);
        buffer = buffer.slice(newline + 1);
        let message;
        try { message = JSON.parse(line); } catch { continue; }
        if (message.id === 1 && !initialized) {
          if (message.error) return finish(new Error(`Codex initialize failed: ${message.error.message}`));
          initialized = true;
          child.stdin.write(JSON.stringify({ method: "initialized", params: {} }) + "\n");
          child.stdin.write(JSON.stringify({ id: 2, method: "account/rateLimits/read", params: { excludeResetCreditDetails: true } }) + "\n");
        } else if (message.id === 2) {
          if (message.error) return finish(new Error(`Codex usage read failed: ${message.error.message}`));
          return finish(null, message.result);
        }
      }
    });
    child.stdin.write(JSON.stringify({ id: 1, method: "initialize", params: { clientInfo: { name: "sandcastle-workflow", version: "1" } } }) + "\n");
  });
  const buckets = result?.rateLimitsByLimitId ?? {};
  const bucket = Object.entries(buckets).find(([id, value]) => id === model || value.normalModelSlug === model)?.[1]
    ?? buckets.codex ?? result?.rateLimits;
  const windows = [bucket?.primary, bucket?.secondary];
  const weekly = windows.find((window) => window?.windowDurationMins >= 7 * 24 * 60);
  if (!weekly || !Number.isFinite(weekly.usedPercent)) {
    throw new Error("Weekly Codex quota is unavailable; cannot enforce the 25% task budget");
  }
  return { usedPercent: weekly.usedPercent, resetsAt: weekly.resetsAt, ordinaryUsageAllowed: result.ordinaryUsageAllowed };
}

export function updateBudget(state, snapshot) {
  const budget = state.budget ??= { consumedPercent: 0, lastUsedPercent: snapshot.usedPercent, resetsAt: snapshot.resetsAt, tokens: 0 };
  if (budget.resetsAt !== snapshot.resetsAt || snapshot.usedPercent < budget.lastUsedPercent) {
    budget.consumedPercent += snapshot.usedPercent;
  } else {
    budget.consumedPercent += Math.max(0, snapshot.usedPercent - budget.lastUsedPercent);
  }
  budget.lastUsedPercent = snapshot.usedPercent;
  budget.resetsAt = snapshot.resetsAt;
  if (snapshot.ordinaryUsageAllowed === false) throw new Error("Codex account usage limit reached");
  const limit = taskQuotaLimit(state);
  if (budget.consumedPercent >= limit) throw new Error(`Task reached ${limit}% of the weekly Codex quota`);
  const tokenLimit = Number(process.env.SANDCASTLE_TASK_TOKEN_LIMIT || 0);
  if (tokenLimit > 0 && budget.tokens >= tokenLimit) throw new Error(`Task reached its token limit (${tokenLimit})`);
  return budget;
}

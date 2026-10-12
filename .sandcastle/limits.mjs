import { spawn } from "node:child_process";
import { randomUUID } from "node:crypto";

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
export function extendTaskQuota(state, extraPercent, { authorizationId = randomUUID(), explanation = "Explicit additional task quota authorization" } = {}) {
  if (!Number.isFinite(extraPercent) || extraPercent <= 0 || extraPercent > 100) throw new Error("SANDCASTLE_EXTRA_QUOTA_PERCENT must be greater than 0 and at most 100");
  if (state.phase !== "awaiting" || !state.resumePhase || !state.budget || !/^Task reached [\d.]+% of the weekly Codex quota$/.test(state.quotaError ?? state.pauseReason ?? "")) throw new Error("Extra quota continuation requires a task quota pause");
  const previousLimit = taskQuotaLimit(state);
  recordTaskQuotaAllowance(state, Math.max(previousLimit,state.budget.consumedPercent)+extraPercent, {authorizationId,explanation,extraPercent});
  delete state.quotaError;
}

// Accounting only: guarded continuation callers establish the authorization.
// A prospective ceiling is absolute and never depends on consumption growth.
export function recordTaskQuotaAllowance(state, limitPercent, {authorizationId,explanation,extraPercent}) {
  const previousLimit=taskQuotaLimit(state);
  if(!Number.isFinite(limitPercent)||limitPercent<=previousLimit||!Number.isFinite(extraPercent)||extraPercent<=0)throw new Error("Invalid absolute task quota allowance");
  if (typeof authorizationId !== "string" || !authorizationId.trim() || typeof explanation !== "string" || !explanation.trim()) throw new Error("Quota grant requires an authorization ID and explanation");
  if (state.budget.allowanceHistory?.some((entry) => entry.id === authorizationId)) throw new Error("Quota grant authorization already recorded");
  if (!Number.isFinite(state.budget.consumedPercent) || state.budget.consumedPercent < 0) throw new Error("Invalid task quota consumption");
  (state.budget.allowanceHistory ??= []).push({ id: authorizationId, windowId: state.budget.windowId ?? windowIdentity(state.budget.resetsAt), deltaPercent: limitPercent - previousLimit, extraPercent, previousLimit, consumedPercent: state.budget.consumedPercent, limitPercent, explanation, snapshot: { usedPercent: state.budget.lastUsedPercent ?? null, resetsAt: state.budget.resetsAt ?? null, tokens: state.budget.tokens ?? null }, grantedAt: new Date().toISOString() });
  state.budget.limitPercent = limitPercent;
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

function windowIdentity(resetsAt) {
  return Number.isFinite(resetsAt) ? `weekly:${resetsAt}` : "weekly:unknown";
}

function usageRecord(budget, snapshot, deltaPercent, explanation, windowId = budget.windowId) {
  (budget.usageHistory ??= []).push({ id: randomUUID(), sequence: budget.usageHistory.length + 1, windowId, snapshot: { usedPercent: snapshot.usedPercent ?? null, resetsAt: snapshot.resetsAt ?? null, ordinaryUsageAllowed: snapshot.ordinaryUsageAllowed ?? null, tokens: budget.tokens ?? null }, deltaPercent, consumedPercent: budget.consumedPercent, explanation, observedAt: new Date().toISOString() });
}

// Consumption is a lifetime accumulator. Window samples are account-wide, so
// they provide a conservative allowance guard, not per-task billing attribution.
export function updateBudget(state, snapshot) {
  if (!snapshot || !Number.isFinite(snapshot.usedPercent) || snapshot.usedPercent < 0 || snapshot.usedPercent > 100 ||
    (snapshot.resetsAt != null && !Number.isFinite(snapshot.resetsAt))) throw new Error("Invalid Codex usage snapshot");
  const existing = state.budget;
  if (existing && (!Number.isFinite(existing.consumedPercent) || existing.consumedPercent < 0 ||
    (existing.lastUsedPercent != null && (!Number.isFinite(existing.lastUsedPercent) || existing.lastUsedPercent < 0 || existing.lastUsedPercent > 100)) ||
    (existing.windowHighWaterPercent != null && (!Number.isFinite(existing.windowHighWaterPercent) || existing.windowHighWaterPercent < 0 || existing.windowHighWaterPercent > 100)))) throw new Error("Invalid saved task quota accounting");
  const budget = state.budget ??= { consumedPercent: 0, lastUsedPercent: snapshot.usedPercent, resetsAt: snapshot.resetsAt, tokens: 0 };
  if (!budget.windowId) {
    budget.windowId = windowIdentity(budget.resetsAt);
  }
  budget.windowResetAt ??= budget.resetsAt ?? null;
  budget.windowHighWaterPercent ??= budget.lastUsedPercent ?? snapshot.usedPercent;
  if (!budget.usageHistory) {
    usageRecord(budget, { usedPercent: budget.lastUsedPercent, resetsAt: budget.resetsAt }, 0, existing
      ? "Legacy accounting baseline imported; existing consumption retained. Cannot reconstruct consumption lost to past unrecorded resets."
      : "Initial task baseline; account usage before this snapshot is excluded.");
  }
  const knownWindow = Number.isFinite(budget.windowResetAt) && Number.isFinite(snapshot.resetsAt);
  const resetDelta = knownWindow ? snapshot.resetsAt - budget.windowResetAt : 0;
  let deltaPercent = 0;
  let explanation;
  if (knownWindow && resetDelta <= -300) {
    // An older response must not reopen an already accounted window.
    usageRecord(budget, snapshot, 0, "Ignored stale earlier-window snapshot", windowIdentity(snapshot.resetsAt));
  } else {
    if (knownWindow && resetDelta >= 300) {
      budget.windowId = windowIdentity(snapshot.resetsAt);
      budget.windowResetAt = snapshot.resetsAt;
      deltaPercent = snapshot.usedPercent;
      budget.windowHighWaterPercent = snapshot.usedPercent;
      explanation = "Confirmed account window rollover; charged observed new-window usage conservatively. Unobserved usage between snapshots cannot reconstruct missing history.";
    } else {
      const newlyIdentifiedWindow = !Number.isFinite(budget.windowResetAt) && Number.isFinite(snapshot.resetsAt);
      deltaPercent = Math.max(0, snapshot.usedPercent - budget.windowHighWaterPercent);
      explanation = newlyIdentifiedWindow
        ? "Window identity became available; charged observed positive growth and established the current snapshot baseline"
        : snapshot.usedPercent < budget.windowHighWaterPercent
        ? "Same-window usage correction; high-water baseline retained, no rollover inferred."
        : "Same-window usage increase above recorded high-water baseline";
      budget.windowHighWaterPercent = newlyIdentifiedWindow ? snapshot.usedPercent : Math.max(budget.windowHighWaterPercent, snapshot.usedPercent);
      if (!knownWindow && budget.resetsAt !== snapshot.resetsAt) {
        explanation += "; window identity unavailable, missing rollover history cannot reconstruct consumption";
        if (!Number.isFinite(budget.windowResetAt) && Number.isFinite(snapshot.resetsAt)) {
          budget.windowResetAt = snapshot.resetsAt;
          budget.windowId = windowIdentity(snapshot.resetsAt);
        }
      }
    }
    budget.consumedPercent += deltaPercent;
    budget.lastUsedPercent = snapshot.usedPercent;
    budget.resetsAt = snapshot.resetsAt;
    usageRecord(budget, snapshot, deltaPercent, explanation);
  }
  if (snapshot.ordinaryUsageAllowed === false) throw new Error("Codex account usage limit reached");
  const limit = taskQuotaLimit(state);
  if (budget.consumedPercent >= limit) throw new Error(`Task reached ${limit}% of the weekly Codex quota`);
  const tokenLimit = Number(process.env.SANDCASTLE_TASK_TOKEN_LIMIT || 0);
  if (tokenLimit > 0 && budget.tokens >= tokenLimit) throw new Error(`Task reached its token limit (${tokenLimit})`);
  return budget;
}

import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { randomBytes } from "node:crypto";
import { mkdirSync, writeFileSync, readFileSync, rmSync, existsSync } from "node:fs";
import { resolve } from "node:path";
import test from "node:test";

test("unconfirmed CLI resume preserves the original pause and checkpoint without acquiring a lock", () => {
  const root = resolve(import.meta.dirname, "..");
  const id = randomBytes(4).toString("hex");
  const dir = resolve(root, ".sandcastle/runs", id);
  const text = JSON.stringify({ id, branch: `codex/sandcastle-test-${id}`, phase: "awaiting", resumePhase: "assemble", pauseReason: "A worker is blocked", completedTickets: [], budget: { consumedPercent: 5, tokens: 123 } });
  mkdirSync(dir, { recursive: true });
  writeFileSync(resolve(dir, "state.json"), text);
  const env = { ...process.env };
  delete env.SANDCASTLE_CONTINUE;
  delete env.SANDCASTLE_RECOVER;
  delete env.SANDCASTLE_EXTRA_QUOTA_PERCENT;
  try {
    assert.throws(() => execFileSync(process.execPath, [".sandcastle/workflow.mjs", "resume", id], { cwd: root, env, stdio: ["ignore", "pipe", "pipe"] }), (error) => {
      assert.equal(error.status, 1);
      assert.match(error.stderr.toString(), /needs human confirmation/);
      return true;
    });
    assert.equal(readFileSync(resolve(dir, "state.json"), "utf8"), text);
    assert.equal(existsSync(resolve(dir, "run.lock")), false);
  } finally { rmSync(dir, { recursive: true, force: true }); }
});

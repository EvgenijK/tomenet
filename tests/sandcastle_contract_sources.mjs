import assert from "node:assert/strict";
import test from "node:test";
import { mkdtemp, mkdir, rm, symlink, writeFile } from "node:fs/promises";
import { join } from "node:path";
import { tmpdir } from "node:os";
import { resolveContractSources } from "../.sandcastle/contract-sources.mjs";
import { partitionContractSources, sourceSpans } from "../.sandcastle/contract-draft.mjs";

async function fixture(t) {
  const root = await mkdtemp(join(tmpdir(), "sandcastle-contract-sources-"));
  t.after(() => rm(root, { recursive: true, force: true }));
  await mkdir(join(root, "docs/tasks"), { recursive: true });
  await mkdir(join(root, "docs/policy"), { recursive: true });
  await writeFile(join(root, "AGENTS.md"), "Keep production behavior.\n");
  await writeFile(join(root, "docs/tasks/dependency.md"), "Dependency details.\n[Do not follow](../policy/transitive.md)\n");
  await writeFile(join(root, "docs/tasks/completion.md"), "Later completion owner.\n");
  await writeFile(join(root, "docs/policy/canonical.md"), "Canonical requirement.\n");
  await writeFile(join(root, "docs/policy/transitive.md"), "Transitive requirement must not enter.\n");
  await writeFile(join(root, "docs/unrelated.md"), "Unrelated requirement.\n");
  const specPath = "docs/tasks/feature.md";
  const spec = [
    "# Feature",
    "",
    "## Dependencies",
    "Depends on [dependency](dependency.md).",
    "",
    "## Canonical sources",
    "Apply [policy](../policy/canonical.md#rule).",
    "",
    "## Definition of Done",
    "Later work belongs to [completion](completion.md).",
    "",
  ].join("\n");
  await writeFile(join(root, specPath), spec);
  return { root, specPath, spec };
}

test("builds a deterministic one-hop source set with normative and informational roles", async (t) => {
  const { root, specPath, spec } = await fixture(t);
  const first = await resolveContractSources({ root, specPath, spec });
  const second = await resolveContractSources({ worktree: root, specPath, spec, agents: "Keep production behavior.\n" });

  assert.deepEqual(first, second);
  assert.deepEqual(Object.keys(first.sources), [specPath, "AGENTS.md", "docs/policy/canonical.md"]);
  assert.deepEqual(Object.keys(first.informationalSources), ["docs/tasks/completion.md", "docs/tasks/dependency.md"]);
  assert.equal(first.informationalSources["docs/tasks/dependency.md"], "Dependency details.\n[Do not follow](../policy/transitive.md)\n");
  assert.ok(!first.manifest.entries.some(({ path }) => path.endsWith("transitive.md") || path.endsWith("unrelated.md")));
  assert.match(first.manifest.digest, /^[0-9a-f]{64}$/);
  assert.ok(first.manifest.entries.every(({ sha256 }) => /^[0-9a-f]{64}$/.test(sha256)));
  const spans = sourceSpans(partitionContractSources(first.sources, 40)).flat();
  assert.ok(spans.some(({ sourcePath, quote }) => sourcePath === specPath && quote === "# Feature"));
  assert.ok(spans.some(({ sourcePath, quote }) => sourcePath === "docs/policy/canonical.md" && quote === "Canonical requirement."));
  assert.ok(!spans.some(({ sourcePath }) => sourcePath === "docs/tasks/dependency.md"));
});

test("deduplicates repeated links and promotes a document cited by normative context", async (t) => {
  const { root, specPath } = await fixture(t);
  const spec = [
    "# Feature",
    "## Dependencies",
    "Depends on [shared](dependency.md).",
    "## Primary requirements",
    "The [same source](dependency.md#accepted) is authoritative.",
  ].join("\n");
  const result = await resolveContractSources({ root, specPath, spec });
  assert.equal(result.sources["docs/tasks/dependency.md"].startsWith("Dependency details."), true);
  assert.ok(!("docs/tasks/dependency.md" in result.informationalSources));
  const entry = result.manifest.entries.find(({ path }) => path === "docs/tasks/dependency.md");
  assert.deepEqual(entry.origins, ["ticket-link:informational", "ticket-link:normative"]);
});

test("rejects traversal and absolute local Markdown links", async (t) => {
  const { root, specPath } = await fixture(t);
  await assert.rejects(resolveContractSources({ root, specPath, spec: "[escape](../../../outside.md)" }), /outside the repository/);
  await assert.rejects(resolveContractSources({ root, specPath, spec: "[absolute](/tmp/outside.md)" }), /Absolute local Markdown link/);
  await assert.rejects(resolveContractSources({ root, specPath, spec: "[file](file:///tmp/outside.md)" }), /Absolute local Markdown link/);
  await assert.rejects(resolveContractSources({ root, specPath, spec: "[windows](C:\\outside.md)" }), /Absolute local Markdown link/);
  await assert.rejects(resolveContractSources({ root, specPath: "../feature.md", spec: "text" }), /traverse outside/);
});

test("rejects a repository-local path whose symlink target escapes the checkout", async (t) => {
  const { root, specPath } = await fixture(t);
  const outside = await mkdtemp(join(tmpdir(), "sandcastle-contract-outside-"));
  t.after(() => rm(outside, { recursive: true, force: true }));
  await writeFile(join(outside, "policy.md"), "outside\n");
  await symlink(join(outside, "policy.md"), join(root, "docs/tasks/escaped.md"));
  await assert.rejects(resolveContractSources({ root, specPath, spec: "## Canonical sources\n[escape](escaped.md)\n" }), /outside the repository/);
});

test("ignores remote, fragment, image and non-Markdown links", async (t) => {
  const { root, specPath } = await fixture(t);
  const spec = "[web](https://example.test/remote.md) [anchor](#here) ![image](missing.md) [json](missing.json)";
  const result = await resolveContractSources({ root, specPath, spec });
  assert.deepEqual(Object.keys(result.sources), [specPath, "AGENTS.md"]);
  assert.deepEqual(result.informationalSources, {});
});

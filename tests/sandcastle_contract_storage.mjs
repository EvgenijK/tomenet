import assert from "node:assert/strict";
import test from "node:test";
import { createHash } from "node:crypto";
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import { mkdtemp, readFile, readdir, rm, stat, symlink, unlink, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { resolve } from "node:path";
import { ensureFragmentCatalog, loadFragmentCatalog, readContractArtifact, writeContractArtifact } from "../.sandcastle/contract-storage.mjs";
import { readContractFragments } from "../.sandcastle/contract-fragments.mjs";

const exec = promisify(execFile);
const digest = (value) => createHash("sha256").update(JSON.stringify(value)).digest("hex");
const sources = { "docs/task.md": "# Task\n\n  Preserve production semantics.  \n", "AGENTS.md": "Keep SV isolated.\n" };
const fixture = async (t) => {
  const root = await mkdtemp(resolve(tmpdir(), "sandcastle-contract-storage-"));
  t.after(() => rm(root, { recursive: true, force: true }));
  return { root, directory: resolve(root, "source-fragments") };
};

test("catalog persists one exact-ID file per fragment and reuses identical files on resume", async (t) => {
  const { directory } = await fixture(t);
  const first = await ensureFragmentCatalog({ directory, sources });
  assert.equal(first.fragments.length, 3);
  assert.deepEqual(new Set(await readdir(directory)), new Set(["index.json", ...first.fragments.map((fragment) => fragment.id)]));
  const oldFiles = await Promise.all(first.fragments.map(async (fragment) => ({ text: await readFile(resolve(directory, fragment.id), "utf8"), stat: await stat(resolve(directory, fragment.id)) })));
  const resumed = await ensureFragmentCatalog({ directory, sources });
  assert.deepEqual(resumed, first);
  for (const [index, fragment] of first.fragments.entries()) {
    assert.equal(await readFile(resolve(directory, fragment.id), "utf8"), oldFiles[index].text);
    assert.equal((await stat(resolve(directory, fragment.id))).mtimeMs, oldFiles[index].stat.mtimeMs);
  }
  assert.equal(first.fragments.find((fragment) => fragment.quote.startsWith("Preserve")).quote, "Preserve production semantics.");
});

test("explicit append retains old IDs/files, rejects source changes and historical text replacement", async (t) => {
  const { directory } = await fixture(t);
  const first = await ensureFragmentCatalog({ directory, sources });
  const previous = new Map(await Promise.all(first.fragments.map(async (fragment) => [fragment.id, await readFile(resolve(directory, fragment.id), "utf8")])));
  const added = { ...sources, "docs/task.md": `${sources["docs/task.md"]}Verify input cancellation.\n`, "docs/new.md": "Handle reconnect.\n" };
  await assert.rejects(ensureFragmentCatalog({ directory, sources: added }), /explicit requirement amendment/);
  const next = await ensureFragmentCatalog({ directory, sources: added, allowAppend: true });
  assert.equal(next.fragments.length, first.fragments.length + 2);
  assert.deepEqual(next.fragments.slice(0, first.fragments.length), first.fragments);
  for (const [id, value] of previous) assert.equal(await readFile(resolve(directory, id), "utf8"), value);
  assert.notEqual(next.digest, first.digest);
  await assert.rejects(ensureFragmentCatalog({ directory, sources: { ...added, "AGENTS.md": "Changed policy." }, allowAppend: true }), /removes or replaces fragment/);
  await assert.rejects(ensureFragmentCatalog({ directory, sources, allowAppend: true }), /removes or replaces fragment/);
  assert.deepEqual(await loadFragmentCatalog(directory), next);
});

test("catalog rejects edited, missing, extra and symlinked fragment files", async (t) => {
  const { directory, root } = await fixture(t);
  const catalog = await ensureFragmentCatalog({ directory, sources });
  const fragment = catalog.fragments[0];
  const path = resolve(directory, fragment.id);
  const original = await readFile(path, "utf8");
  await writeFile(path, JSON.stringify({ ...fragment, quote: "invented requirement" }));
  await assert.rejects(ensureFragmentCatalog({ directory, sources }), /fragment integrity/);
  await writeFile(path, original);
  await unlink(path);
  await assert.rejects(ensureFragmentCatalog({ directory, sources }), /ENOENT/);
  await writeFile(path, original);
  await writeFile(resolve(directory, "S-unindexed"), original);
  await assert.rejects(loadFragmentCatalog(directory), /unindexed/);
  await unlink(resolve(directory, "S-unindexed"));
  const external = resolve(root, "copy");
  await writeFile(external, original);
  await unlink(path);
  await symlink(external, path);
  await assert.rejects(loadFragmentCatalog(directory), /plain file/);
});

test("existing catalog with missing index is not silently rebuilt", async (t) => {
  const { directory } = await fixture(t);
  await ensureFragmentCatalog({ directory, sources });
  await unlink(resolve(directory, "index.json"));
  await assert.rejects(ensureFragmentCatalog({ directory, sources }), /ENOENT/);
});

function accepted(fragmentId) {
  const contract = {
    version: 2, completionScope: "implementation",
    criteria: [{ id: "AC-1", requirement: "Preserve production semantics", sourceSpanIds: [fragmentId], mandatory: true, applicability: "current", owner: "", deferralReason: "", checkIds: ["sv-core"] }],
    checks: [{ id: "sv-core", kind: "core", command: "bash -s -- core", description: "Sandcastle core checks" }],
  };
  return { contract, digest: digest(contract), verification: { approved: true, summary: "Checked", missingRequirements: [] }, migration: "Migrated run" };
}

test("split artifact stores only IDs in root, one criterion/check per file and exact roundtrip digest", async (t) => {
  const { root, directory } = await fixture(t);
  const catalog = await ensureFragmentCatalog({ directory, sources });
  const original = accepted(catalog.fragments[0].id);
  await writeContractArtifact(root, original);
  const index = JSON.parse(await readFile(resolve(root, "acceptance-contract.json"), "utf8"));
  assert.deepEqual(index.contract.criteria, ["AC-1"]);
  assert.deepEqual(index.contract.checks, ["sv-core"]);
  assert.deepEqual(await readdir(resolve(root, "criteria")), ["AC-1"]);
  assert.deepEqual(await readdir(resolve(root, "checks")), ["sv-core"]);
  const criterion = JSON.parse(await readFile(resolve(root, "criteria", "AC-1"), "utf8"));
  assert.deepEqual(criterion.sourceSpanIds, [catalog.fragments[0].id]);
  assert.equal(criterion.source, undefined);
  assert.deepEqual(await readContractArtifact(root), original);
  await writeFile(resolve(root, "criteria", "AC-1"), JSON.stringify({ ...criterion, requirement: "silently weakened" }));
  await assert.rejects(readContractArtifact(root), /integrity failed/);
  await writeFile(resolve(root, "criteria", "AC-1"), JSON.stringify(criterion));
  await writeContractArtifact(root, original);
  await unlink(resolve(root, "checks", "sv-core"));
  await assert.rejects(readContractArtifact(root), /ENOENT/);
});

test("split writer rejects quotations and unsafe IDs, historical v1 artifacts still read unchanged", async (t) => {
  const { root } = await fixture(t);
  const original = accepted("S-known");
  original.contract.criteria[0].source = { path: "task.md", quote: "Legacy" };
  original.digest = digest(original.contract);
  await assert.rejects(writeContractArtifact(root, original), /without quotations/);
  delete original.contract.criteria[0].source;
  original.contract.criteria[0].id = "../outside";
  original.digest = digest(original.contract);
  await assert.rejects(writeContractArtifact(root, original), /unsafe file ID/);
  const legacy = { contract: { version: 1, criteria: [{ id: "C1", source: { path: "task.md", quote: "Legacy" } }], checks: [] }, digest: "historical", verification: { approved: true } };
  await writeFile(resolve(root, "acceptance-contract.json"), JSON.stringify(legacy));
  assert.deepEqual(await readContractArtifact(root), legacy);
  legacy.digest = digest(legacy.contract);
  await writeContractArtifact(root, legacy);
  assert.deepEqual(await readContractArtifact(root), legacy);
});

test("interrupted authorized append resumes its journal without replacing historical files", async (t) => {
  const { root, directory } = await fixture(t);
  const first = await ensureFragmentCatalog({ directory, sources });
  const previousIndex = JSON.parse(await readFile(resolve(directory, "index.json"), "utf8"));
  const amended = { ...sources, "docs/task.md": `${sources["docs/task.md"]}New requirement.\nAnother requirement.\n` };
  const otherDirectory = resolve(root, "completed-catalog");
  await ensureFragmentCatalog({ directory: otherDirectory, sources });
  const completed = await ensureFragmentCatalog({ directory: otherDirectory, sources: amended, allowAppend: true });
  const nextIndex = JSON.parse(await readFile(resolve(otherDirectory, "index.json"), "utf8"));
  const additions = completed.fragments.filter((fragment) => !first.fragments.some((old) => old.id === fragment.id));
  const unchangedFile = resolve(directory, first.fragments[0].id);
  const oldStat = await stat(unchangedFile);
  await writeFile(resolve(directory, "append.json"), JSON.stringify({ version: 1, previousDigest: first.digest, previousIndex, additions, nextIndex }));
  await writeFile(resolve(directory, additions[0].id), JSON.stringify(additions[0]));
  const beforeNames = await readdir(directory);
  const beforeBytes = new Map(await Promise.all(beforeNames.map(async (name) => [name, await readFile(resolve(directory, name), "utf8")])));
  await assert.rejects(ensureFragmentCatalog({ directory, sources, allowAppend: true, expectedDigest: first.digest }), /supplied amended sources/);
  assert.deepEqual(await readdir(directory), beforeNames);
  for (const [name, contents] of beforeBytes) assert.equal(await readFile(resolve(directory, name), "utf8"), contents);
  await assert.rejects(ensureFragmentCatalog({ directory, sources: amended, expectedDigest: first.digest }), /explicit amendment authorization/);
  assert.equal((await readdir(directory)).includes("append.json"), true);
  assert.deepEqual(await ensureFragmentCatalog({ directory, sources: amended, allowAppend: true, expectedDigest: first.digest }), completed);
  assert.equal((await stat(unchangedFile)).mtimeMs, oldStat.mtimeMs);
  assert.equal((await readdir(directory)).includes("append.json"), false);
  // The final index can also be durable before journal cleanup.
  await writeFile(resolve(directory, "append.json"), JSON.stringify({ version: 1, previousDigest: first.digest, previousIndex, additions, nextIndex }));
  assert.deepEqual(await ensureFragmentCatalog({ directory, sources: amended, allowAppend: true, expectedDigest: first.digest }), completed);
});

test("saved digest allows only authorized interrupted/completed append and forbids catalog recreation", async (t) => {
  const { directory } = await fixture(t);
  await assert.rejects(ensureFragmentCatalog({ directory, sources, expectedDigest: "saved-but-missing" }), /refusing recreation/);
  const first = await ensureFragmentCatalog({ directory, sources });
  const amended = { ...sources, "docs/task.md": `${sources["docs/task.md"]}New requirement.\n` };
  const next = await ensureFragmentCatalog({ directory, sources: amended, allowAppend: true, expectedDigest: first.digest });
  // State saving can lag behind a completed catalog append and journal cleanup.
  assert.deepEqual(await ensureFragmentCatalog({ directory, sources: amended, allowAppend: true, expectedDigest: first.digest }), next);
  const beforeNames = await readdir(directory);
  const beforeBytes = new Map(await Promise.all(beforeNames.map(async (name) => [name, await readFile(resolve(directory, name), "utf8")])));
  const anotherAmendment = { ...amended, "docs/task.md": `${amended["docs/task.md"]}Unrelated further requirement.\n` };
  await assert.rejects(ensureFragmentCatalog({ directory, sources: anotherAmendment, allowAppend: true, expectedDigest: first.digest }), /completed append differs/);
  assert.deepEqual(await readdir(directory), beforeNames);
  for (const [name, contents] of beforeBytes) assert.equal(await readFile(resolve(directory, name), "utf8"), contents);
  await assert.rejects(ensureFragmentCatalog({ directory, sources: amended, expectedDigest: first.digest }), /expected saved digest/);
  await assert.rejects(ensureFragmentCatalog({ directory, sources: amended, allowAppend: true, expectedDigest: "unrelated-digest" }), /expected saved digest/);
  assert.deepEqual(await ensureFragmentCatalog({ directory, sources: amended, expectedDigest: next.digest }), next);
  const receiptPath = resolve(directory, "append-receipt.json");
  const receipt = JSON.parse(await readFile(receiptPath, "utf8"));
  receipt.previousIndex.fragments.reverse();
  await writeFile(receiptPath, JSON.stringify(receipt));
  await assert.rejects(ensureFragmentCatalog({ directory, sources: amended, allowAppend: true, expectedDigest: first.digest }), /historical catalog/);
});

test("split publication rejects stray files and retires old checks only for an explicit amendment", async (t) => {
  const { root } = await fixture(t);
  const original = accepted("S-known");
  await writeContractArtifact(root, original);
  await writeFile(resolve(root, "criteria", "AC-stray"), "{}");
  await assert.rejects(readContractArtifact(root), /unindexed criteria/);
  await assert.rejects(writeContractArtifact(root, original), /unindexed criteria/);
  await unlink(resolve(root, "criteria", "AC-stray"));
  const next = structuredClone(original);
  next.contract.checks[0].id = "controller-review";
  next.contract.criteria[0].checkIds = ["controller-review"];
  next.digest = digest(next.contract);
  await assert.rejects(writeContractArtifact(root, next), /without an explicit amendment/);
  await writeContractArtifact(root, next, { allowAmendment: true });
  assert.deepEqual(await readdir(resolve(root, "checks")), ["controller-review"]);
  assert.deepEqual(await readContractArtifact(root), next);
});

test("fragment CLI returns catalog-owned text and rejects traversal, unknown IDs and corrupt catalogs", async (t) => {
  const { directory } = await fixture(t);
  const catalog = await ensureFragmentCatalog({ directory, sources });
  const id = catalog.fragments[0].id;
  const cli = resolve(".sandcastle/contract-fragments.mjs");
  const result = await exec(process.execPath, [cli, directory, id]);
  assert.deepEqual(JSON.parse(result.stdout), [catalog.fragments[0]]);
  assert.deepEqual(await readContractFragments(directory, [id, id]), [catalog.fragments[0], catalog.fragments[0]]);
  await assert.rejects(exec(process.execPath, [cli, directory, "../outside"]), /unsafe file ID/);
  await assert.rejects(exec(process.execPath, [cli, directory, "S-unknown"]), /Unknown fragment ID/);
  await unlink(resolve(directory, id));
  await assert.rejects(exec(process.execPath, [cli, directory, id]), /ENOENT/);
});

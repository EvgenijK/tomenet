import { createHash, randomUUID } from "node:crypto";
import { lstat, mkdir, readFile, readdir, rename, rm, unlink, writeFile } from "node:fs/promises";
import { resolve } from "node:path";

const hash = (value) => createHash("sha256").update(typeof value === "string" ? value : JSON.stringify(value)).digest("hex");
const json = (value) => `${JSON.stringify(value, null, 2)}\n`;
const fail = (message) => { throw new Error(`Acceptance storage: ${message}`); };

export function assertFragmentId(id) {
  if (typeof id !== "string" || !/^[a-zA-Z0-9][a-zA-Z0-9_.-]{0,79}$/.test(id)) fail(`unsafe file ID ${id}`);
  return id;
}

async function plainDirectory(directory) {
  const stat = await lstat(directory);
  if (!stat.isDirectory() || stat.isSymbolicLink()) fail(`not a plain directory: ${directory}`);
}

async function readJson(path) {
  const stat = await lstat(path);
  if (!stat.isFile() || stat.isSymbolicLink()) fail(`not a plain file: ${path}`);
  return JSON.parse(await readFile(path, "utf8"));
}

async function atomicJson(path, value) {
  // The public filename changes only after its complete contents are written.
  const temporary = `${path}.tmp-${randomUUID()}`;
  await writeFile(temporary, json(value), { flag: "wx" });
  await rename(temporary, path);
}

function fragmentId(sourcePath, quote) {
  return `S-${hash(`${sourcePath}\0${quote}`)}`;
}

function sourceDigests(sources) {
  if (!sources || typeof sources !== "object" || Array.isArray(sources) || !Object.keys(sources).length) fail("non-empty source map required");
  const entries = Object.entries(sources).sort(([a], [b]) => a.localeCompare(b));
  for (const [path, content] of entries) {
    if (!path || path.includes("\0") || typeof content !== "string" || !content.trim()) fail(`invalid source ${path}`);
  }
  return Object.fromEntries(entries.map(([path, content]) => [path, hash(content)]));
}

function newFragments(sources) {
  const fragments = new Map();
  for (const [sourcePath, content] of Object.entries(sources)) {
    for (const line of content.split("\n")) {
      const quote = line.trim();
      if (!quote) continue;
      const id = fragmentId(sourcePath, quote);
      fragments.set(id, { id, sourcePath, quote });
    }
  }
  return [...fragments.values()];
}

function indexFor(fragments, digests) {
  const index = { version: 1, fragments: fragments.map((fragment) => ({ id: fragment.id, sha256: hash(fragment) })), sourceDigests: digests };
  return { ...index, digest: hash(index) };
}

/** Load the saved catalog; missing, edited, extra and symlinked files fail closed. */
export async function loadFragmentCatalog(directory) {
  return readCatalog(directory);
}

function verifyAppendReceipt(index, receipt) {
  const previous = receipt?.previousIndex;
  if (receipt?.version !== 1 || !previous || receipt.nextDigest !== index.digest) fail("invalid fragment append receipt");
  const { digest, ...metadata } = previous;
  if (previous.version !== 1 || hash(metadata) !== digest || !Array.isArray(previous.fragments) ||
      previous.fragments.length > index.fragments.length || previous.fragments.some((entry, offset) => JSON.stringify(entry) !== JSON.stringify(index.fragments[offset])) ||
      Object.keys(previous.sourceDigests ?? {}).some((path) => !Object.hasOwn(index.sourceDigests, path))) fail("append receipt does not preserve historical catalog");
  return previous.digest;
}

async function readCatalog(directory, permittedExtras = [], { ignoreReceipt = false } = {}) {
  await plainDirectory(directory);
  const index = await readJson(resolve(directory, "index.json"));
  const { digest, ...metadata } = index;
  if (index.version !== 1 || !Array.isArray(index.fragments) || !index.fragments.length ||
      !index.sourceDigests || typeof index.sourceDigests !== "object" || Array.isArray(index.sourceDigests) || hash(metadata) !== digest) fail("fragment catalog index integrity failed");
  const expected = new Set(["index.json"]);
  const fragments = [];
  for (const entry of index.fragments) {
    assertFragmentId(entry?.id);
    if (entry.id === "index.json" || expected.has(entry.id)) fail("duplicate fragment ID");
    expected.add(entry.id);
    const fragment = await readJson(resolve(directory, entry.id));
    if (fragment.id !== entry.id || typeof fragment.sourcePath !== "string" || typeof fragment.quote !== "string" ||
        !fragment.quote.trim() || fragmentId(fragment.sourcePath, fragment.quote) !== entry.id || hash(fragment) !== entry.sha256 ||
        !Object.hasOwn(index.sourceDigests, fragment.sourcePath)) fail(`fragment integrity failed for ${entry.id}`);
    fragments.push(fragment);
  }
  const receiptPath = resolve(directory, "append-receipt.json");
  if (await present(receiptPath)) {
    expected.add("append-receipt.json");
    if (!ignoreReceipt) verifyAppendReceipt(index, await readJson(receiptPath));
  }
  const extras = new Set(permittedExtras);
  const names = await readdir(directory);
  if (names.some((name) => !expected.has(name) && !extras.has(name))) fail("fragment catalog contains unindexed files");
  return { version: 1, fragments, digest, sourceDigests: index.sourceDigests };
}

async function present(path) {
  try { await lstat(path); return true; }
  catch (error) { if (error.code !== "ENOENT") throw error; return false; }
}

async function finishAppend(directory, expectedDigest, sources, digests) {
  const journalPath = resolve(directory, "append.json");
  if (!await present(journalPath)) return;
  const journal = await readJson(journalPath);
  if (journal.version !== 1 || !Array.isArray(journal.additions) || !journal.nextIndex || !journal.previousIndex ||
      journal.previousIndex.digest !== journal.previousDigest || typeof journal.previousDigest !== "string") fail("invalid fragment append journal");
  if (expectedDigest !== undefined && ![journal.previousDigest, journal.nextIndex.digest].includes(expectedDigest)) fail("append journal differs from expected catalog digest");
  if (hash(journal.nextIndex.sourceDigests) !== hash(digests)) fail("append journal differs from supplied amended sources");
  const lines = new Map(Object.entries(sources).map(([path, content]) => [path, new Set(content.split("\n").map((line) => line.trim()))]));
  const receipt = { version: 1, previousIndex: journal.previousIndex, nextDigest: journal.nextIndex.digest };
  verifyAppendReceipt(journal.nextIndex, receipt);
  const additions = journal.additions;
  const ids = new Set();
  for (const fragment of additions) {
    assertFragmentId(fragment?.id);
    if (ids.has(fragment.id) || fragmentId(fragment.sourcePath, fragment.quote) !== fragment.id ||
        typeof fragment.sourcePath !== "string" || typeof fragment.quote !== "string" || !fragment.quote.trim()) fail("invalid journal fragment");
    if (!lines.get(fragment.sourcePath)?.has(fragment.quote)) fail("append journal fragment is outside supplied amended sources");
    ids.add(fragment.id);
  }
  const catalog = await readCatalog(directory, ["append.json", ...ids], { ignoreReceipt: true });
  if (catalog.fragments.some((fragment) => !lines.get(fragment.sourcePath)?.has(fragment.quote))) fail("append journal removes historical source fragments");
  if (catalog.digest === journal.nextIndex.digest) {
    // The committed index survived, but cleanup was interrupted.
    if (JSON.stringify(indexFor(catalog.fragments, catalog.sourceDigests)) !== JSON.stringify(journal.nextIndex)) fail("committed append journal mismatch");
    await atomicJson(resolve(directory, "append-receipt.json"), receipt);
    await unlink(journalPath);
    return;
  }
  if (catalog.digest !== journal.previousDigest || additions.some((fragment) => catalog.fragments.some((old) => old.id === fragment.id))) fail("append journal does not match catalog");
  if (JSON.stringify(indexFor(catalog.fragments, catalog.sourceDigests)) !== JSON.stringify(journal.previousIndex)) fail("append journal previous catalog integrity failed");
  const nextIndex = indexFor([...catalog.fragments, ...additions], journal.nextIndex.sourceDigests);
  if (JSON.stringify(nextIndex) !== JSON.stringify(journal.nextIndex)) fail("append journal integrity failed");
  for (const fragment of additions) {
    const path = resolve(directory, fragment.id);
    if (await present(path)) {
      if (hash(await readJson(path)) !== hash(fragment)) fail(`interrupted append fragment changed ${fragment.id}`);
    } else await writeFile(path, json(fragment), { flag: "wx" });
  }
  await atomicJson(resolve(directory, "append-receipt.json"), receipt);
  await atomicJson(resolve(directory, "index.json"), nextIndex);
  await unlink(journalPath);
}

/** Create once, reuse without rebuilding, append only for an explicit requirement change. */
export async function ensureFragmentCatalog({ directory, sources, allowAppend = false, expectedDigest }) {
  const digests = sourceDigests(sources);
  const exists = await present(directory);
  if (!exists) {
    if (expectedDigest !== undefined) fail("saved fragment catalog is missing; refusing recreation");
    const fragments = newFragments(sources);
    // Only a complete catalog becomes visible, including on an interrupted first run.
    const staging = `${directory}.initial-${randomUUID()}`;
    await mkdir(staging, { recursive: true });
    await plainDirectory(staging);
    try {
      for (const fragment of fragments) await writeFile(resolve(staging, fragment.id), json(fragment), { flag: "wx" });
      await writeFile(resolve(staging, "index.json"), json(indexFor(fragments, digests)), { flag: "wx" });
      await rename(staging, directory);
    } finally { await rm(staging, { recursive: true, force: true }); }
    return loadFragmentCatalog(directory);
  }
  if (await present(resolve(directory, "append.json"))) {
    if (!allowAppend) fail("unfinished fragment append requires explicit amendment authorization");
    await finishAppend(directory, expectedDigest, sources, digests);
  }
  const catalog = await loadFragmentCatalog(directory);
  if (expectedDigest !== undefined && catalog.digest !== expectedDigest) {
    const receiptPath = resolve(directory, "append-receipt.json");
    if (!allowAppend || !await present(receiptPath) || verifyAppendReceipt(await readJson(resolve(directory, "index.json")), await readJson(receiptPath)) !== expectedDigest) fail("fragment catalog differs from expected saved digest");
    if (hash(catalog.sourceDigests) !== hash(digests)) fail("completed append differs from supplied amended sources");
  }
  if (hash(catalog.sourceDigests) === hash(digests)) return catalog;
  if (!allowAppend) fail("fragment sources changed; explicit requirement amendment required");
  // Existing source lines must survive the amendment. IDs and files are never replaced.
  const lines = new Map(Object.entries(sources).map(([path, content]) => [path, new Set(content.split("\n").map((line) => line.trim()))]));
  for (const fragment of catalog.fragments) {
    if (!lines.get(fragment.sourcePath)?.has(fragment.quote)) fail(`amendment removes or replaces fragment ${fragment.id}`);
  }
  const ids = new Set(catalog.fragments.map((fragment) => fragment.id));
  const additions = newFragments(sources).filter((fragment) => !ids.has(fragment.id));
  await atomicJson(resolve(directory, "append.json"), { version: 1, previousDigest: catalog.digest,
    previousIndex: indexFor(catalog.fragments, catalog.sourceDigests), additions,
    nextIndex: indexFor([...catalog.fragments, ...additions], digests) });
  await finishAppend(directory, catalog.digest, sources, digests);
  return loadFragmentCatalog(directory);
}

function artifactItems(items, label) {
  if (!Array.isArray(items) || !items.length) fail(`missing ${label}`);
  const ids = new Set();
  for (const item of items) {
    assertFragmentId(item?.id);
    if (ids.has(item.id)) fail(`duplicate ${label} ID ${item.id}`);
    ids.add(item.id);
  }
  return items.map((item) => item.id);
}

function assertCriterionReferences(criterion) {
  if (criterion.source !== undefined || criterion.sourceSpanId !== undefined || criterion.quote !== undefined ||
      !Array.isArray(criterion.sourceSpanIds) || !criterion.sourceSpanIds.length ||
      new Set(criterion.sourceSpanIds).size !== criterion.sourceSpanIds.length) fail(`criterion ${criterion.id} requires fragment IDs without quotations`);
  criterion.sourceSpanIds.forEach(assertFragmentId);
}

/** Publish split v2 artifacts; v1 is retained only as a historical reader format. */
export async function writeContractArtifact(taskDirectory, accepted, { allowAmendment = false } = {}) {
  const { contract, digest, verification, migration } = accepted;
  if (![1, 2].includes(contract?.version) || hash(contract) !== digest) fail("invalid contract version/digest");
  await mkdir(taskDirectory, { recursive: true });
  if (contract.version === 1) {
    await atomicJson(resolve(taskDirectory, "acceptance-contract.json"), { contract, digest, verification,
      ...(migration !== undefined ? { migration } : {}) });
    return;
  }
  const criterionIds = artifactItems(contract.criteria, "criteria");
  const checkIds = artifactItems(contract.checks, "checks");
  contract.criteria.forEach(assertCriterionReferences);
  let previous;
  if (await present(resolve(taskDirectory, "acceptance-contract.json"))) {
    previous = await readContractArtifact(taskDirectory);
    if (previous.digest !== digest && !allowAmendment) fail("contract artifact changed without an explicit amendment");
  }
  for (const [name, items] of [["criteria", contract.criteria], ["checks", contract.checks]]) {
    const directory = resolve(taskDirectory, name);
    await mkdir(directory, { recursive: true });
    await plainDirectory(directory);
    const knownIds = new Set([...(previous?.contract?.version === 2 ? previous.contract[name].map((item) => item.id) : []), ...items.map((item) => item.id)]);
    if ((await readdir(directory)).some((name) => !knownIds.has(name))) fail(`unindexed ${name} file`);
    for (const item of items) await atomicJson(resolve(directory, item.id), item);
    // Only an explicitly authorized replacement can retire a previously listed file.
    const wanted = new Set(items.map((item) => item.id));
    for (const old of previous?.contract?.version === 2 ? previous.contract[name] : []) {
      if (!wanted.has(old.id)) await unlink(resolve(directory, old.id));
    }
  }
  const artifact = { contract: { ...contract, criteria: criterionIds, checks: checkIds }, digest, verification,
    ...(migration !== undefined ? { migration } : {}) };
  await atomicJson(resolve(taskDirectory, "acceptance-contract.json"), artifact);
  return artifact;
}

/** Reconstruct exact contract object order so its acceptance digest remains unchanged. */
export async function readContractArtifact(taskDirectory) {
  const artifact = await readJson(resolve(taskDirectory, "acceptance-contract.json"));
  if (artifact.contract?.version === 1) return artifact;
  if (artifact.contract?.version !== 2) fail("unsupported contract artifact version");
  const contract = { ...artifact.contract };
  for (const name of ["criteria", "checks"]) {
    const ids = contract[name];
    if (!Array.isArray(ids) || !ids.length || new Set(ids).size !== ids.length) fail(`invalid ${name} file references`);
    await plainDirectory(resolve(taskDirectory, name));
    contract[name] = [];
    for (const id of ids) {
      assertFragmentId(id);
      const item = await readJson(resolve(taskDirectory, name, id));
      if (item.id !== id) fail(`${name} filename/ID mismatch ${id}`);
      if (name === "criteria") assertCriterionReferences(item);
      contract[name].push(item);
    }
    const expected = new Set(ids);
    if ((await readdir(resolve(taskDirectory, name))).some((id) => !expected.has(id))) fail(`unindexed ${name} file`);
  }
  if (hash(contract) !== artifact.digest) fail("split contract artifact integrity failed");
  return { ...artifact, contract };
}

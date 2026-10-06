import { createHash } from "node:crypto";

const sourceDigest = (sources) => createHash("sha256").update(JSON.stringify(sources)).digest("hex");

// IDs include the source revision, path, absolute character offset and text. They
// survive regrouping, but cannot be reused for changed source text.
export function sourceSpans(parts) {
  const offsets = new Map();
  const revision = sourceDigest(parts);
  return parts.map((part) => {
    const start = offsets.get(part.sourcePath) ?? 0;
    offsets.set(part.sourcePath, start + part.sourceText.length);
    const spans = [];
    let position = 0;
    for (const line of part.sourceText.match(/[^\n]*(?:\n|$)/g) ?? []) {
      if (!line) continue;
      const quote = line.trim();
      if (quote) {
        const fingerprint = createHash("sha256").update(`${revision}\0${part.sourcePath}\0${start + position}\0${quote}`).digest("hex").slice(0, 16);
        spans.push({ id: `S-${fingerprint}`, sourcePath: part.sourcePath, quote });
      }
      position += line.length;
    }
    return spans;
  });
}

// Keep heading blocks intact so a quoted requirement and its context reach
// the same proposer. The size is a target, not a reason to cut a table row.
export function partitionContractSources(sources, targetChars = 6500) {
  if (!Number.isInteger(targetChars) || targetChars < 1) throw new Error("Invalid contract partition size");
  const parts = [];
  for (const [sourcePath, content] of Object.entries(sources)) {
    if (typeof content !== "string" || !content) throw new Error(`Empty contract source: ${sourcePath}`);
    const starts = [0, ...[...content.matchAll(/^## /gm)].map((match) => match.index).filter((index) => index > 0), content.length];
    let current = "";
    for (let index = 0; index < starts.length - 1; index++) {
      const section = content.slice(starts[index], starts[index + 1]);
      if (current && current.length + section.length > targetChars) { parts.push({ sourcePath, sourceText: current }); current = ""; }
      current += section;
    }
    if (current) parts.push({ sourcePath, sourceText: current });
  }
  return parts;
}

export function combineContractParts(parts, responses, { allowMixedScopes = false, requireSpanIds = false, legacyIndexes = [] } = {}) {
  if (!Array.isArray(responses) || responses.length !== parts.length || !parts.length) throw new Error("Incomplete contract parts");
  const scopes = new Set(responses.map((response) => response?.completionScope));
  if ([...scopes].some((scope) => !["implementation", "full_acceptance"].includes(scope)) || scopes.size !== 1 && !allowMixedScopes) throw new Error("Inconsistent contract completion scope");
  const contract = { version: 1, completionScope: scopes.has("implementation") ? "implementation" : "full_acceptance", criteria: [], checks: [] };
  const baseline = new Map();
  const commandIds = new Map();
  for (let index = 0; index < parts.length; index++) {
    const response = responses[index];
    if (response?.version !== 1 || !Array.isArray(response.criteria) || !Array.isArray(response.checks)) throw new Error("Malformed contract part");
    const mapped = new Map();
    for (const check of response.checks) {
      if (typeof check?.id !== "string") throw new Error("Malformed contract check");
      if (["sv-build", "sv-core"].includes(check.id)) {
        const previous = baseline.get(check.id);
        if (previous && (previous.kind !== check.kind || previous.command !== check.command)) throw new Error(`Conflicting baseline check ${check.id}`);
        if (!previous) { baseline.set(check.id, check); contract.checks.push(check); }
        mapped.set(check.id, check.id);
      } else {
        const commandKey = check.kind === "command" ? JSON.stringify(check.command) : undefined;
        const id = commandKey && commandIds.has(commandKey) ? commandIds.get(commandKey) : `P${index + 1}-${check.id}`;
        mapped.set(check.id, id);
        if (!commandKey || !commandIds.has(commandKey)) contract.checks.push({ ...check, id });
        if (commandKey) commandIds.set(commandKey, id);
      }
    }
    for (const criterion of response.criteria) {
      const sections = parts[index].sourceSections ?? [parts[index]];
      let source;
      if (requireSpanIds && !legacyIndexes.includes(index)) {
        const span = parts[index].sourceSpans?.find((item) => item.id === criterion?.sourceSpanId);
        if (!span || criterion.source !== undefined) throw new Error(`Contract criterion ${criterion?.id ?? "<missing>"} has an unknown or stale sourceSpanId in group ${index + 1}; use an assigned source span ID (${(parts[index].sourceSpans ?? []).slice(0, 8).map((item) => item.id).join(", ")}${(parts[index].sourceSpans?.length ?? 0) > 8 ? ", ..." : ""})`);
        source = { path: span.sourcePath, quote: span.quote };
      } else {
        const quote = criterion?.source?.quote;
        if (typeof quote !== "string" || !quote.trim()) throw new Error(`Contract criterion ${criterion?.id ?? "<missing>"} has an empty source quote in group ${index + 1}`);
        if (!sections.some((section) => section.sourcePath === criterion.source.path && section.sourceText.includes(quote))) throw new Error(`Contract criterion ${criterion.id} cited a source outside its assigned section in group ${index + 1}`);
        source = criterion.source;
      }
      if (!Array.isArray(criterion.checkIds) || criterion.checkIds.some((id) => !mapped.has(id))) throw new Error(`Contract criterion ${criterion?.id ?? "<missing>"} has an unmapped check in group ${index + 1}`);
      const { sourceSpanId: _sourceSpanId, ...fields } = criterion;
      contract.criteria.push({ ...fields, source, id: `P${index + 1}-${criterion.id}`, checkIds: criterion.checkIds.map((id) => mapped.get(id)) });
    }
  }
  return contract;
}

export async function buildContractDraft(pending, sources, proposePart, save, targetChars = 6500) {
  const parts = partitionContractSources(sources, targetChars);
  const digest = sourceDigest(sources);
  if (pending.partSourceDigest && pending.partSourceDigest !== digest) throw new Error("Acceptance contract source changed during draft; human decision required");
  if (pending.parts && !pending.partSourceDigest) throw new Error("Unverified contract part checkpoint");
  pending.partSourceDigest = digest;
  pending.parts ??= [];
  if (pending.parts.length > parts.length) throw new Error("Contract part checkpoint exceeds source count");
  await save();
  for (let index = pending.parts.length; index < parts.length; index++) {
    const response = await proposePart(parts[index], index, parts.length);
    combineContractParts([parts[index]], [response]); // Reject invented quotes before saving the checkpoint.
    pending.parts.push(response);
    await save();
  }
  return combineContractParts(parts, pending.parts);
}

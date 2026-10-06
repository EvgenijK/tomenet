import { createHash } from "node:crypto";
import { readFile, realpath } from "node:fs/promises";
import { dirname, isAbsolute, relative, resolve, sep } from "node:path";

const digest = (value) => createHash("sha256").update(value).digest("hex");
const compare = (left, right) => left < right ? -1 : left > right ? 1 : 0;
const markdownPath = (value) => /\.md$/i.test(value);
const externalTarget = (value) => /^[a-z][a-z0-9+.-]*:/i.test(value) || value.startsWith("//");
const windowsAbsolute = (value) => /^[a-z]:[\\/]/i.test(value) || value.startsWith("\\\\");
const normativeContext = /(?:canonical|normative|authoritative|primary|обязательн|норматив|первичн|основан)/i;
const informationalContext = /(?:dependenc|prerequis|blocked|завис|блокир|definition of done|comments?|answer)/i;

function repositoryPath(value, label) {
  if (typeof value !== "string" || !value.trim()) throw new Error(`${label} must be a non-empty repository-relative path`);
  const path = value.replaceAll("\\", "/");
  if (path.includes("\0") || path.startsWith("/") || windowsAbsolute(value)) throw new Error(`${label} must be repository-relative: ${value}`);
  const segments = path.split("/");
  if (segments.some((segment) => segment === "..")) throw new Error(`${label} cannot traverse outside the repository: ${value}`);
  const normalized = segments.filter((segment) => segment && segment !== ".").join("/");
  if (!normalized) throw new Error(`${label} must name a file`);
  return normalized;
}

function ensureInside(root, candidate, label) {
  const path = relative(root, candidate);
  if (path === "" || path === ".." || path.startsWith(`..${sep}`) || isAbsolute(path)) {
    throw new Error(`${label} resolves outside the repository`);
  }
}

function linkedRepositoryPath(checkout, ticketPath, target, label) {
  const candidate = resolve(checkout, dirname(ticketPath), target.replaceAll("\\", "/"));
  ensureInside(checkout, candidate, label);
  return relative(checkout, candidate).split(sep).join("/");
}

function headingAt(source, offset) {
  let heading = "";
  for (const match of source.slice(0, offset).matchAll(/^#{1,6}\s+(.+)$/gm)) heading = match[1].trim();
  return heading;
}

function lineAt(source, offset) {
  const start = source.lastIndexOf("\n", offset - 1) + 1;
  const end = source.indexOf("\n", offset);
  return source.slice(start, end < 0 ? source.length : end).trim();
}

function linkedTargets(source) {
  const matches = [];
  const inline = /(?<!!)\[[^\]\n]+\]\(\s*(?:<([^>\n]+)>|([^\s)]+))(?:\s+(?:"[^"]*"|'[^']*'|\([^)]*\)))?\s*\)/g;
  const definition = /^\s{0,3}\[[^\]\n]+\]:\s*(?:<([^>\n]+)>|(\S+))/gm;
  for (const pattern of [inline, definition]) {
    for (const match of source.matchAll(pattern)) {
      const rawTarget = match[1] ?? match[2];
      matches.push({ rawTarget, offset: match.index, context: `${headingAt(source, match.index)}\n${lineAt(source, match.index)}` });
    }
  }
  return matches.sort((left, right) => left.offset - right.offset || compare(left.rawTarget, right.rawTarget));
}

function localMarkdownTarget(rawTarget) {
  if (!rawTarget || rawTarget.startsWith("#")) return null;
  if (/^file:/i.test(rawTarget) || windowsAbsolute(rawTarget)) throw new Error(`Absolute local Markdown link is not allowed: ${rawTarget}`);
  if (externalTarget(rawTarget)) return null;
  let target;
  try { target = decodeURIComponent(rawTarget); }
  catch { throw new Error(`Invalid encoded Markdown link: ${rawTarget}`); }
  const path = target.split("#", 1)[0].split("?", 1)[0];
  if (!markdownPath(path)) return null;
  if (path.startsWith("/") || windowsAbsolute(path)) throw new Error(`Absolute local Markdown link is not allowed: ${rawTarget}`);
  return path;
}

function roleFor(context) {
  if (normativeContext.test(context)) return "normative";
  if (informationalContext.test(context)) return "informational";
  return "informational";
}

async function safeRead(checkout, repositoryRelativePath) {
  const candidate = resolve(checkout, repositoryRelativePath);
  ensureInside(checkout, candidate, repositoryRelativePath);
  const actual = await realpath(candidate);
  ensureInside(checkout, actual, repositoryRelativePath);
  return readFile(actual, "utf8");
}

/**
 * Build the immutable, one-hop source set used to draft and verify a contract.
 *
 * `sources` contains only normative material and can be passed directly to the
 * existing partition/sourceSpan pipeline. Linked dependency/background files
 * remain available in `informationalSources` and the content-addressed manifest,
 * but cannot silently add acceptance obligations.
 */
export async function resolveContractSources({ root, worktree, specPath, spec, agents, agentsPath = "AGENTS.md" }) {
  const checkout = resolve(worktree ?? root ?? "");
  if (!root && !worktree) throw new Error("Contract sources require root or worktree");
  const actualCheckout = await realpath(checkout);
  const ticketPath = repositoryPath(specPath, "specPath");
  const policyPath = repositoryPath(agentsPath, "agentsPath");
  if (ticketPath === policyPath) throw new Error("Contract ticket and agent policy must be different files");
  if (!markdownPath(ticketPath) || !markdownPath(policyPath)) throw new Error("Contract ticket and agent policy must be Markdown files");
  const ticket = spec ?? await safeRead(actualCheckout, ticketPath);
  const policy = agents ?? await safeRead(actualCheckout, policyPath);
  if (typeof ticket !== "string" || !ticket) throw new Error("Contract spec must be non-empty text");
  if (typeof policy !== "string" || !policy) throw new Error("AGENTS.md must be non-empty text");

  const entries = new Map([
    [ticketPath, { path: ticketPath, role: "normative", content: ticket, origins: ["originating-ticket"] }],
    [policyPath, { path: policyPath, role: "normative", content: policy, origins: ["repository-policy"] }],
  ]);
  for (const link of linkedTargets(ticket)) {
    const target = localMarkdownTarget(link.rawTarget);
    if (!target) continue;
    const joined = linkedRepositoryPath(actualCheckout, ticketPath, target, `Markdown link ${link.rawTarget}`);
    const role = roleFor(link.context);
    const previous = entries.get(joined);
    if (previous) {
      previous.role = previous.role === "normative" || role === "normative" ? "normative" : "informational";
      previous.origins.push(`ticket-link:${role}`);
      continue;
    }
    entries.set(joined, { path: joined, role, content: await safeRead(actualCheckout, joined), origins: [`ticket-link:${role}`] });
  }

  const sourceRank = (path) => path === ticketPath ? 0 : path === policyPath ? 1 : 2;
  const ordered = [...entries.values()].sort((left, right) => sourceRank(left.path) - sourceRank(right.path) || compare(left.path, right.path));
  const manifestEntries = ordered.map(({ path, role, content, origins }) => ({
    path,
    role,
    sha256: digest(content),
    origins: [...new Set(origins)].sort(compare),
  }));
  const manifest = {
    version: 1,
    traversal: "originating-ticket-links-only",
    specPath: ticketPath,
    entries: manifestEntries,
  };
  manifest.digest = digest(JSON.stringify(manifest));
  return {
    sources: Object.fromEntries(ordered.filter((entry) => entry.role === "normative").map((entry) => [entry.path, entry.content])),
    informationalSources: Object.fromEntries(ordered.filter((entry) => entry.role === "informational").map((entry) => [entry.path, entry.content])),
    manifest,
  };
}

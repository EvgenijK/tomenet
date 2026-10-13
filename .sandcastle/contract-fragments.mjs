#!/usr/bin/env node
import { pathToFileURL } from "node:url";
import { assertFragmentId, loadFragmentCatalog } from "./contract-storage.mjs";

export async function readContractFragments(directory, ids) {
  if (!Array.isArray(ids) || !ids.length) throw new Error("At least one fragment ID is required");
  ids.forEach(assertFragmentId);
  const catalog = await loadFragmentCatalog(directory);
  const fragments = new Map(catalog.fragments.map((fragment) => [fragment.id, fragment]));
  return ids.map((id) => {
    if (!fragments.has(id)) throw new Error(`Unknown fragment ID ${id}`);
    return fragments.get(id);
  });
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const [directory, ...ids] = process.argv.slice(2);
  try {
    if (!directory || !ids.length) throw new Error("Usage: node .sandcastle/contract-fragments.mjs <source-fragments-directory> <ID> [ID...]");
    process.stdout.write(`${JSON.stringify(await readContractFragments(directory, ids), null, 2)}\n`);
  } catch (error) {
    process.stderr.write(`${error.message}\n`);
    process.exitCode = 1;
  }
}

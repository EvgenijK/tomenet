import { resolve } from "node:path";
import { readRunStatus } from "./events.mjs";

const [id, flag, sequence, ...extra] = process.argv.slice(2);
if (!id || (flag !== undefined && (flag !== "--after" || sequence === undefined)) || extra.length) {
  console.error("Usage: node .sandcastle/status.mjs <run-id> [--after sequence]");
  process.exitCode = 2;
} else {
  try {
    const after = sequence === undefined ? 0 : /^\d+$/.test(sequence) ? Number(sequence) : NaN;
    console.log(JSON.stringify(await readRunStatus(resolve(import.meta.dirname, "runs"), id, { after }), null, 2));
  } catch (error) {
    console.error(error.message);
    process.exitCode = 1;
  }
}

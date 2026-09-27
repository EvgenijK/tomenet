import { spawn } from "node:child_process";
import { chmod, mkdir } from "node:fs/promises";
import { resolve } from "node:path";

const root = resolve(import.meta.dirname, "..");
const authDir = resolve(root, ".sandcastle/auth");
await mkdir(authDir, { recursive: true, mode: 0o700 });
await chmod(authDir, 0o700);

const mode = process.argv[2] ?? "device";
if (process.argv.length > 3 || !["browser", "device", "status"].includes(mode)) {
  console.error("Usage: npm run sandbox:login [-- device|browser|status]");
  process.exit(2);
}
const statusArgs = [
  "run", "--rm", "-i",
  "--user", `${process.getuid()}:${process.getgid()}`,
  "--env", "HOME=/home/agent",
  "--volume", `${authDir}:/home/agent/.codex:rw`,
  "--entrypoint", "codex",
  "sandcastle:tomenet_modern_client",
  "login", "status",
];
const executable = mode === "status" ? "docker" : "codex";
const args = mode === "status" ? statusArgs : ["login", ...(mode === "device" ? ["--device-auth"] : [])];
const env = mode === "status" ? process.env : { ...process.env, CODEX_HOME: authDir };

const child = spawn(executable, args, { stdio: "inherit", env });
child.on("error", (error) => {
  console.error(`Could not launch ${executable}: ${error.message}`);
  process.exitCode = 1;
});
child.on("exit", (code, signal) => {
  process.exitCode = code ?? (signal ? 1 : 0);
});

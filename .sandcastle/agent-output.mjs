function sanitizeDiagnostic(value) {
  return String(value ?? "")
    .replace(/\b(?:Bearer|Basic)\s+[A-Za-z0-9._~+/=-]+/gi, "Bearer <REDACTED>")
    .replace(/\b(?:api[_-]?key|access[_-]?token|refresh[_-]?token|password|secret)\s*[=:]\s*(?:"[^"]*"|'[^']*'|[^\s,;&]+)/gi, "<REDACTED>")
    .replace(/\bsk-[A-Za-z0-9_-]+/g, "<REDACTED>")
    .replace(/(https?:\/\/)[^\s/@]+:[^\s/@]+@/gi, "$1<REDACTED>@")
    .replace(/[\x00-\x1f\x7f]/g, " ").slice(0, 2000);
}

export function agentFailureDetail(captured, stderr) {
  return captured.error || sanitizeDiagnostic(stderr) || "no diagnostic in JSON stdout or stderr";
}

export function parseAgentOutput(line, result, log = console.log) {
  let event;
  try { event = JSON.parse(line); } catch { return; }
  if (event.type === "item.completed" && event.item?.type === "agent_message") {
    result.message = event.item.text;
    log(event.item.text.slice(0, 1500));
  }
  if (event.type === "error" || event.type === "turn.failed" || event.item?.type === "error") {
    const detail = event.error?.message ?? event.message ?? event.item?.message;
    if (typeof detail === "string" && detail) result.error = sanitizeDiagnostic(detail);
  }
  if (event.type === "turn.completed") {
    const usage = event.usage ?? {};
    result.tokens += (usage.input_tokens || 0) + (usage.output_tokens || 0);
  }
}
